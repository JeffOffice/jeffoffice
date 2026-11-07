#include "tst_write.h"


void WriteTests::modifyingAStyleRestylesTheTextKeepsWhatTheTextSaysAndIsOneStep()
{
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Heading one</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Heading two</w:t></w:r></w:p>"
                            "<w:p><w:r><w:t>Body</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(12);
    c.setPosition(23, QTextCursor::KeepAnchor);
    setTextColor(*doc, c, QColor(255, 0, 0));
    const double normalSize = styCharAt(*doc, 25).fontPointSize();
    wml::Style h1 = styStyle(*doc, QStringLiteral("Heading1"));
    h1.rPr.size = 48;
    h1.rPr.italic = true;
    h1.pPr.jc = QStringLiteral("center");
    const quint64 before = doc->nextStepSerial();
    QString problem;
    QVERIFY2(modifyStyle(*doc, c, QStringLiteral("Heading1"), h1, &problem), qPrintable(problem));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    auto center = [&](int at) { return doc->story()->findBlock(at).blockFormat().alignment() & Qt::AlignHorizontal_Mask; };
    QCOMPARE(styCharAt(*doc, 3).fontPointSize(), 24.0);
    QVERIFY(styCharAt(*doc, 3).fontItalic());
    QCOMPARE(center(3), Qt::AlignHCenter);
    // What the text says itself stays: the second heading's red; it follows the style in the rest.
    QCOMPARE(styCharAt(*doc, 14).fontPointSize(), 24.0);
    QVERIFY(styCharAt(*doc, 14).fontItalic());
    QCOMPARE(styCharAt(*doc, 14).foreground().color(), QColor(255, 0, 0));
    QVERIFY(styCharAt(*doc, 3).foreground().color() != QColor(255, 0, 0));
    // The body is not changed.
    QCOMPARE(styCharAt(*doc, 25).fontPointSize(), normalSize);
    QVERIFY(!styCharAt(*doc, 25).fontItalic());
    QVERIFY(center(25) != Qt::AlignHCenter);
    // One Undo is the style as it was, and the text with it; one Redo again.
    QVERIFY(doc->undo());
    QCOMPARE(styStyle(*doc, QStringLiteral("Heading1")).rPr.size, std::optional<int>(34));
    QCOMPARE(styCharAt(*doc, 3).fontPointSize(), 17.0);
    QVERIFY(!styCharAt(*doc, 3).fontItalic());
    QVERIFY(center(3) != Qt::AlignHCenter);
    QCOMPARE(styCharAt(*doc, 14).foreground().color(), QColor(255, 0, 0));
    QVERIFY(doc->redo());
    QCOMPARE(styStyle(*doc, QStringLiteral("Heading1")).rPr.size, std::optional<int>(48));
    QCOMPARE(styCharAt(*doc, 3).fontPointSize(), 24.0);
    // Saved: the style says the new look, the paragraphs say only what they said themselves; opened again, the text looks the same.
    const QByteArray saved = doc->toDocx();
    const QByteArray styleXmlOut = styleXml(saved), bodyOut = documentXml(saved);
    QVERIFY(styleXmlOut.contains("<w:sz w:val=\"48\"/>"));
    QVERIFY(!bodyOut.contains("<w:sz w:val=\"48\"/>"));
    QVERIFY(bodyOut.contains("FF0000"));
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(styCharAt(*back, 3).fontPointSize(), 24.0);
    QVERIFY(styCharAt(*back, 3).fontItalic());
    QCOMPARE(styCharAt(*back, 14).foreground().color(), QColor(255, 0, 0));
    QCOMPARE(back->story()->findBlock(3).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    // A character style restyles the runs that have it, in any paragraph.
    auto chars = open(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">a </w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t>b</w:t></w:r></w:p>", kStyles));
    QVERIFY(chars);
    wml::Style strong = styStyle(*chars, QStringLiteral("Strong"));
    strong.rPr.italic = true;
    QTextCursor sc(chars->story());
    QVERIFY(modifyStyle(*chars, sc, QStringLiteral("Strong"), strong));
    QVERIFY(styCharAt(*chars, 2).fontItalic());
    QVERIFY(!styCharAt(*chars, 0).fontItalic());
    QVERIFY(chars->undo());
    QVERIFY(!styCharAt(*chars, 2).fontItalic());
}


void WriteTests::modifyingAStyleWithNoChangeLeavesTheFileByteForByteAndUndoReturnsToIt()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("Odd"), QStringLiteral("Odd"),
                                                       "<w:aliases w:val=\"Strange\"/><w:basedOn w:val=\"Normal\"/><w:autoRedefine/><w:locked/><w:rsid w:val=\"00112233\"/><w:pPr><w:keepNext/></w:pPr><w:rPr><w:i/></w:rPr>");
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Odd\"/></w:pPr><w:r><w:t>Text</w:t></w:r></w:p>";
    const QByteArray zip = makeDocx(body, styles);
    const QByteArray original = styleXml(zip);
    auto doc = open(zip);
    QVERIFY(doc);
    QTextCursor c(doc->story());
    for (const QString &id : {QStringLiteral("Odd"), QStringLiteral("Heading1"), QStringLiteral("Normal"), QStringLiteral("Strong")}) {
        QVERIFY(modifyStyle(*doc, c, id, styStyle(*doc, id)));
        QVERIFY(!doc->canUndo());
        QVERIFY(!doc->isModified());
    }
    QCOMPARE(styleXml(doc->toDocx()), original);
    // A change and its undo come back to the same bytes; the change says what the style kept.
    wml::Style odd = styStyle(*doc, QStringLiteral("Odd"));
    odd.rPr.bold = true;
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("Odd"), odd));
    QVERIFY(doc->isModified());
    const QByteArray changed = styleXml(doc->toDocx());
    QVERIFY(changed != original);
    for (const char *kept : {"<w:aliases w:val=\"Strange\"/>", "<w:autoRedefine/>", "<w:locked/>", "00112233", "<w:keepNext/>"}) QVERIFY2(changed.contains(kept), kept);
    QVERIFY(doc->undo());
    QCOMPARE(styleXml(doc->toDocx()), original);
    QVERIFY(!doc->isModified());
    QVERIFY(doc->redo());
    QCOMPARE(styleXml(doc->toDocx()), changed);
}


void WriteTests::aStylesNameBasisAndNextAreCheckedAndSavedTheSame()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("Pull"), QStringLiteral("Pull quote"), "<w:basedOn w:val=\"Normal\"/><w:qFormat/>") +
                              styCustomStyle(QStringLiteral("Mark"), QStringLiteral("Mark"), "", "character");
    auto doc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r></w:p>", styles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    wml::Style pull = styStyle(*doc, QStringLiteral("Pull"));
    pull.name = QStringLiteral("Callout");
    pull.basedOn = QStringLiteral("Heading1");
    pull.next = QStringLiteral("Pull");
    QString problem;
    QVERIFY2(modifyStyle(*doc, c, QStringLiteral("Pull"), pull, &problem), qPrintable(problem));
    auto back = open(doc->toDocx());
    QVERIFY(back);
    const wml::Style saved = styStyle(*back, QStringLiteral("Pull"));
    QCOMPARE(saved.name, QStringLiteral("Callout"));
    QCOMPARE(saved.basedOn, QStringLiteral("Heading1"));
    QCOMPARE(saved.next, QStringLiteral("Pull"));
    // What is refused changes nothing and says why.
    const quint64 serial = doc->nextStepSerial();
    auto refused = [&](const wml::Style &def, const QString &id = QStringLiteral("Pull")) {
        QString why;
        const bool done = modifyStyle(*doc, c, id, def, &why);
        return !done && !why.isEmpty() && doc->nextStepSerial() == serial;
    };
    wml::Style s = styStyle(*doc, QStringLiteral("Pull"));
    s.name = QStringLiteral("heading 1");
    QVERIFY(refused(s));
    s.name = QStringLiteral("  ");
    QVERIFY(refused(s));
    s.name = QString(kMostStyleNameLength + 1, QLatin1Char('n'));
    QVERIFY(refused(s));
    s = styStyle(*doc, QStringLiteral("Pull"));
    s.basedOn = QStringLiteral("Pull");
    QVERIFY(refused(s));
    s.basedOn = QStringLiteral("Nowhere");
    QVERIFY(refused(s));
    s.basedOn = QStringLiteral("Mark");   // (a character style)
    QVERIFY(refused(s));
    wml::Style loop = styStyle(*doc, QStringLiteral("Heading1"));
    loop.basedOn = QStringLiteral("Pull");   // (Pull is based on Heading1: a loop)
    QVERIFY(refused(loop, QStringLiteral("Heading1")));
    // A name that differs only in case from the style's own is its own.
    s = styStyle(*doc, QStringLiteral("Pull"));
    s.name = QStringLiteral("CALLOUT");
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("Pull"), s));
    QCOMPARE(styStyle(*doc, QStringLiteral("Pull")).name, QStringLiteral("CALLOUT"));
    // A built-in style keeps its name.
    wml::Style h1 = styStyle(*doc, QStringLiteral("Heading1"));
    h1.name = QStringLiteral("Mine");
    h1.rPr.italic = true;
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("Heading1"), h1));
    QCOMPARE(styStyle(*doc, QStringLiteral("Heading1")).name, QStringLiteral("heading 1"));
    // Making a style with a taken name, or one of the built-in names, is refused as well.
    wml::Style fresh;
    fresh.name = QStringLiteral("callout");
    QVERIFY(createStyle(*doc, c, fresh, &problem).isEmpty());
    QVERIFY(!problem.isEmpty());
    fresh.name = QStringLiteral("Subtitle");
    QVERIFY(createStyle(*doc, c, fresh, &problem).isEmpty());
    fresh.name = QStringLiteral("Brand new");
    fresh.basedOn = QStringLiteral("Pull");
    fresh.next = QStringLiteral("Nowhere");
    const QString id = createStyle(*doc, c, fresh, &problem);
    QCOMPARE(id, QStringLiteral("Brandnew"));
    QVERIFY(styStyle(*doc, id).next.isEmpty());   // (a following style the document lacks is none)
    QCOMPARE(createStyle(*doc, c, [&] { wml::Style t = fresh; t.name = QStringLiteral("Brand-new"); return t; }(), &problem), QStringLiteral("Brandnew2"));
}


QByteArray WriteTests::styManyStyles(int n, QByteArray *chain)
{
    QByteArray styles = kStyles;
    for (int i = 0; i < n; ++i)
        styles += styCustomStyle(QStringLiteral("S%1").arg(i), QStringLiteral("Style %1").arg(i),
                                 "<w:basedOn w:val=\"" + (i == 0 ? QByteArray("Normal") : QByteArray("S") + QByteArray::number(i - 1)) + "\"/>" + (i == 0 ? QByteArray("<w:rPr><w:sz w:val=\"20\"/></w:rPr>") : QByteArray()));
    if (chain) *chain = styles;
    return styles;
}


void WriteTests::aDocumentOfTenThousandStylesIsListedChangedAndUndoneWithinBounds()
{
    QByteArray styles;
    styManyStyles(10000, &styles);
    // A name of a hundred thousand characters is in the file (and in the list, cut off).
    styles += styCustomStyle(QStringLiteral("Long"), QString(100000, QLatin1Char('L')), "<w:basedOn w:val=\"Normal\"/>");
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"S5\"/></w:pPr><w:r><w:t>Near the root</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:pStyle w:val=\"S9999\"/></w:pPr><w:r><w:t>Far from the root</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:pStyle w:val=\"Long\"/></w:pPr><w:r><w:t>Long name</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, styles));
    QVERIFY(doc);
    const QVector<ListedStyle> all = listStyles(*doc, StyleFilter::All);
    QVERIFY(all.size() > 0 && all.size() <= kMostStylesListed);
    for (const ListedStyle &s : all) QVERIFY(s.name.size() <= kMostStyleNameLength);
    const QStringList inUse = styIds(listStyles(*doc, StyleFilter::InUse));
    QVERIFY(inUse.contains(QStringLiteral("S5")) && inUse.contains(QStringLiteral("S9999")) && inUse.contains(QStringLiteral("Long")));
    QVERIFY(listStyles(*doc, StyleFilter::Recommended).size() <= kMostStylesListed);
    // The root of a chain of ten thousand: the paragraphs that can see it follow it, and one Undo is the chain as it was.
    QTextCursor c(doc->story());
    wml::Style root = styStyle(*doc, QStringLiteral("S0"));
    root.rPr.size = 30;
    const double was = styCharAt(*doc, 2).fontPointSize();
    QCOMPARE(was, 10.0);
    const quint64 before = doc->nextStepSerial();
    QString problem;
    QVERIFY2(modifyStyle(*doc, c, QStringLiteral("S0"), root, &problem), qPrintable(problem));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QCOMPARE(styCharAt(*doc, 2).fontPointSize(), 15.0);
    QVERIFY(doc->undo());
    QCOMPARE(styCharAt(*doc, 2).fontPointSize(), 10.0);
    QVERIFY(doc->redo());
    QCOMPARE(styCharAt(*doc, 2).fontPointSize(), 15.0);
    // Names: 255 characters are a name, 256 are not, whether made or given; the long name of the file can be kept and changed.
    wml::Style fresh;
    fresh.name = QString(kMostStyleNameLength + 1, QLatin1Char('x'));
    QVERIFY(createStyle(*doc, c, fresh, &problem).isEmpty());
    QVERIFY(!problem.isEmpty());
    fresh.name = QString(kMostStyleNameLength, QLatin1Char('x'));
    const QString made = createStyle(*doc, c, fresh, &problem);
    QVERIFY2(!made.isEmpty(), qPrintable(problem));
    QVERIFY(made.size() <= 40);
    wml::Style longOne = styStyle(*doc, QStringLiteral("Long"));
    longOne.rPr.bold = true;
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("Long"), longOne, &problem));
    longOne.name = QString(kMostStyleNameLength + 1, QLatin1Char('y'));
    QVERIFY(!modifyStyle(*doc, c, QStringLiteral("Long"), longOne, &problem));
    // A document that has as many styles as can be made takes no more.
    QByteArray many;
    styManyStyles(kMostStylesMade, &many);
    auto full = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r></w:p>", many));
    QVERIFY(full);
    QTextCursor fc(full->story());
    wml::Style one;
    one.name = QStringLiteral("One too many");
    QVERIFY(createStyle(*full, fc, one, &problem).isEmpty());
    QVERIFY(!problem.isEmpty());
    QVERIFY(listStyles(*full, StyleFilter::All).size() <= kMostStylesListed);
}


void WriteTests::styleChainsThatNameThemselvesAreFollowedNoFurtherThanOnce()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("A"), QStringLiteral("A"), "<w:basedOn w:val=\"B\"/><w:next w:val=\"B\"/><w:rPr><w:b/></w:rPr>") +
                              styCustomStyle(QStringLiteral("B"), QStringLiteral("B"), "<w:basedOn w:val=\"A\"/><w:next w:val=\"A\"/>") +
                              styCustomStyle(QStringLiteral("C"), QStringLiteral("C"), "<w:basedOn w:val=\"Normal\"/>");
    auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"A\"/></w:pPr><w:r><w:t>Looped</w:t></w:r></w:p>", styles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    QString problem;
    // A change to a style in a loop of the file is made, and ends.
    wml::Style a = styStyle(*doc, QStringLiteral("A"));
    a.rPr.italic = true;
    QVERIFY2(modifyStyle(*doc, c, QStringLiteral("A"), a, &problem), qPrintable(problem));
    QVERIFY(styCharAt(*doc, 2).fontItalic());
    QVERIFY(doc->undo());
    QVERIFY(!styCharAt(*doc, 2).fontItalic());
    // A change that would make a loop is refused; one that keeps the loop of the file as it is, is not.
    wml::Style cc = styStyle(*doc, QStringLiteral("C"));
    wml::Style aa = styStyle(*doc, QStringLiteral("A"));
    cc.basedOn = QStringLiteral("B");
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("C"), cc));   // (C on B on A on B: C is not in the loop)
    aa.basedOn = QStringLiteral("C");
    QVERIFY(!modifyStyle(*doc, c, QStringLiteral("A"), aa, &problem));   // (A on C on B on A)
    QVERIFY(!problem.isEmpty());
    // A following style that names itself, or the style before it, is only followed once (Enter at the end of a paragraph).
    c.setPosition(6);
    insertParagraph(*doc, c);
    QCOMPARE(styBlockStyle(*doc, c.position()), QStringLiteral("B"));
    insertParagraph(*doc, c);
    QCOMPARE(styBlockStyle(*doc, c.position()), QStringLiteral("A"));
    // The list of styles, and giving a style of a loop to a paragraph, end.
    QVERIFY(!listStyles(*doc, StyleFilter::All).isEmpty());
    applyParagraphStyle(*doc, c, QStringLiteral("B"));
    QCOMPARE(styBlockStyle(*doc, c.position()), QStringLiteral("B"));
}


void WriteTests::whatTheStepsOfTheHistoryHoldOfTheStylesIsFreedWithTheSteps()
{
    auto doc = textDoc({QStringLiteral("Text")});
    QTextCursor c(doc->story());
    QCOMPARE(doc->styleStepsHeldForTest(), 0);
    for (const char *name : {"One", "Two", "Three"}) {
        wml::Style s;
        s.name = QString::fromLatin1(name);
        QVERIFY(!createStyle(*doc, c, s).isEmpty());
    }
    QCOMPARE(doc->styleStepsHeldForTest(), 3);
    QVERIFY(doc->undo());
    QCOMPARE(doc->styleStepsHeldForTest(), 3);   // (the undone step can be redone)
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Three")));
    QVERIFY(doc->redo());
    QVERIFY(doc->styleSheet().find(QStringLiteral("Three")));
    QVERIFY(doc->undo());
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Two")));
    // A new edit ends what could be redone, and what the two undone steps held with it; the first style is still held for its Undo.
    c.setPosition(4);
    insertText(*doc, c, QStringLiteral("!"));
    QCOMPARE(doc->styleStepsHeldForTest(), 1);
    QVERIFY(doc->styleSheet().find(QStringLiteral("One")));
    QVERIFY(doc->undo());   // (the text)
    QVERIFY(doc->undo());   // (One)
    QVERIFY(!doc->styleSheet().find(QStringLiteral("One")));
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
}


void WriteTests::aStyleThatUpdatesItselfFollowsTheFormattingOfItsParagraphsInTheSameStep()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("Auto"), QStringLiteral("Auto"), "<w:basedOn w:val=\"Normal\"/><w:autoRedefine/>");
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Auto\"/></w:pPr><w:r><w:t>First</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"Auto\"/></w:pPr><w:r><w:t>Second</w:t></w:r></w:p><w:p><w:r><w:t>Other</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, styles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(2);
    const quint64 before = doc->nextStepSerial();
    setAlignment(*doc, c, Qt::AlignHCenter);
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QCOMPARE(styStyle(*doc, QStringLiteral("Auto")).pPr.jc, QStringLiteral("center"));
    // The other paragraph of the style follows it, and a paragraph of another style does not.
    QCOMPARE(doc->story()->findBlock(8).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    QVERIFY((doc->story()->findBlock(16).blockFormat().alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignHCenter);
    QVERIFY(doc->undo());
    QVERIFY(styStyle(*doc, QStringLiteral("Auto")).pPr.jc.isEmpty());
    QVERIFY((doc->story()->findBlock(8).blockFormat().alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignHCenter);
    QVERIFY((doc->story()->findBlock(2).blockFormat().alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignHCenter);
    QVERIFY(!doc->canUndo());
    // Characters too.
    QVERIFY(doc->redo());
    c.setPosition(0);
    c.setPosition(5, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    QCOMPARE(styStyle(*doc, QStringLiteral("Auto")).rPr.bold, std::optional<bool>(true));
    QVERIFY(styCharAt(*doc, 8).fontWeight() >= QFont::DemiBold);
    // A style that does not update itself is left alone.
    c.setPosition(16);
    setAlignment(*doc, c, Qt::AlignRight);
    QVERIFY(styStyle(*doc, QStringLiteral("Normal")).pPr.jc.isEmpty());
}


void WriteTests::updatingAStyleToMatchTheSelectionIsOneStep()
{
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Heading one</w:t></w:r></w:p><w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Heading two</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.setPosition(7, QTextCursor::KeepAnchor);
    setTextColor(*doc, c, QColor(0, 128, 0));
    setFontSize(*doc, c, 30);
    setAlignment(*doc, c, Qt::AlignRight);
    const quint64 before = doc->nextStepSerial();
    QString problem;
    QVERIFY2(updateStyleToSelection(*doc, c, &problem), qPrintable(problem));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    const wml::Style h1 = styStyle(*doc, QStringLiteral("Heading1"));
    QCOMPARE(h1.rPr.size, std::optional<int>(60));
    QVERIFY(h1.rPr.color && h1.rPr.color->val.toUpper() == QLatin1String("008000"));
    QCOMPARE(h1.pPr.jc, QStringLiteral("right"));
    QVERIFY(h1.rPr.bold == std::optional<bool>(true));   // (what the style said before stays)
    QCOMPARE(styCharAt(*doc, 14).fontPointSize(), 30.0);   // (the other heading follows)
    QCOMPARE(doc->story()->findBlock(14).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignRight);
    QVERIFY(doc->undo());
    QCOMPARE(styStyle(*doc, QStringLiteral("Heading1")).rPr.size, std::optional<int>(34));
    QCOMPARE(styCharAt(*doc, 14).fontPointSize(), 17.0);
}


void WriteTests::theFormatPainterCopiesCharactersAndParagraphsAndPastesInOneStep()
{
    auto doc = textDoc({QStringLiteral("Alpha beta gamma"), QStringLiteral("Second paragraph"), QStringLiteral("Third paragraph")});
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.setPosition(5, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    setTextColor(*doc, c, QColor(255, 0, 0));
    setFontSize(*doc, c, 20);
    const FormatBrush brush = copyFormat(*doc, c);
    QVERIFY(brush.valid);
    QVERIFY(!brush.hasParagraph);   // (words, not a paragraph mark)
    QTextCursor d(doc->story());
    d.setPosition(11);
    d.setPosition(16, QTextCursor::KeepAnchor);
    const quint64 before = doc->nextStepSerial();
    QVERIFY(pasteFormat(*doc, d, brush));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QVERIFY(styCharAt(*doc, 12).fontWeight() >= QFont::DemiBold);
    QCOMPARE(styCharAt(*doc, 12).fontPointSize(), 20.0);
    QCOMPARE(styCharAt(*doc, 12).foreground().color(), QColor(255, 0, 0));
    QVERIFY(styCharAt(*doc, 7).fontWeight() < QFont::DemiBold);   // ("beta" is as it was)
    QVERIFY(doc->undo());
    QVERIFY(styCharAt(*doc, 12).fontWeight() < QFont::DemiBold);
    QVERIFY(doc->redo());
    // Saved, and opened again, it is the same.
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QVERIFY(styCharAt(*back, 12).fontWeight() >= QFont::DemiBold);
    QCOMPARE(styCharAt(*back, 12).fontPointSize(), 20.0);
    QCOMPARE(styCharAt(*back, 12).foreground().color(), QColor(255, 0, 0));
    // The words brush on a collapsed caret takes the word the caret is in.
    QTextCursor caret(doc->story());
    caret.setPosition(8);
    QVERIFY(pasteFormat(*doc, caret, brush));
    QVERIFY(styCharAt(*doc, 7).fontWeight() >= QFont::DemiBold);
    QVERIFY(styCharAt(*doc, 11).fontWeight() >= QFont::DemiBold);   // (the word "beta" is 6 to 10; the space is not part of it, "gamma" was painted already)
    QVERIFY(doc->undo());
    // A paragraph: selected with its mark, its alignment, indent, and spacing come too, and the paragraphs painted are only those selected.
    QTextCursor p1(doc->story());
    p1.setPosition(0);
    p1.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    setAlignment(*doc, p1, Qt::AlignHCenter);
    changeIndent(*doc, p1, 36);
    QTextCursor src(doc->story());
    src.setPosition(0);
    src.setPosition(doc->story()->findBlock(0).length(), QTextCursor::KeepAnchor);   // (the mark is in the selection)
    const FormatBrush para = copyFormat(*doc, src);
    QVERIFY(para.valid && para.hasParagraph);
    QTextCursor dest(doc->story());
    const int second = doc->story()->findBlock(20).position(), third = doc->story()->findBlock(second + 20).position();
    dest.setPosition(second);
    dest.setPosition(third, QTextCursor::KeepAnchor);
    const quint64 serial = doc->nextStepSerial();
    QVERIFY(pasteFormat(*doc, dest, para));
    QCOMPARE(doc->nextStepSerial(), serial + 1);
    const QTextBlockFormat secondFormat = doc->story()->findBlock(second).blockFormat(), thirdFormat = doc->story()->findBlock(third).blockFormat();
    QCOMPARE(secondFormat.alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
    QCOMPARE(secondFormat.leftMargin(), 36.0);
    QVERIFY((thirdFormat.alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignHCenter);
    QCOMPARE(thirdFormat.leftMargin(), 0.0);
    // The same brush on words only changes the characters.
    QTextCursor words(doc->story());
    words.setPosition(third);
    words.setPosition(third + 5, QTextCursor::KeepAnchor);
    QVERIFY(pasteFormat(*doc, words, para));
    QVERIFY((doc->story()->findBlock(third).blockFormat().alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignHCenter);
    // A brush with nothing in it pastes nothing.
    FormatBrush none;
    QVERIFY(!pasteFormat(*doc, words, none));
    QVERIFY(doc->undo());
    QVERIFY(doc->undo());
    QCOMPARE(doc->story()->findBlock(second).blockFormat().leftMargin(), 0.0);
    auto saved = open(doc->toDocx());
    QVERIFY(saved);
    QCOMPARE(saved->story()->findBlock(0).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
}


void WriteTests::goToFindsPagesSectionsLinesBookmarksFootnotesAndHeadings()
{
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"Alpha\"/><w:r><w:t>Intro</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>";
    body += "<w:p><w:bookmarkStart w:id=\"3\" w:name=\"_Hidden\"/><w:r><w:t>Hidden bookmark</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p>";
    for (int i = 0; i < 70; ++i) body += "<w:p><w:r><w:t>Paragraph number " + QByteArray::number(i) + " fills the page with some words in it so that there are several pages of text here.</w:t></w:r></w:p>";
    body += "<w:p><w:bookmarkStart w:id=\"2\" w:name=\"Middle\"/><w:r><w:t>Middle</w:t></w:r><w:bookmarkEnd w:id=\"2\"/></w:p>";
    body += "<w:p><w:pPr>" + sectionXml(12240, 15840, 1440, 1440, 1440, 1440) + "</w:pPr><w:r><w:t>End of one</w:t></w:r></w:p>";
    body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Second part</w:t></w:r></w:p><w:p><w:r><w:t>Closing words.</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sectionXml(12240, 15840, 1440, 1440, 1440, 1440)));
    QVERIFY(doc);
    const quint64 serial = doc->nextStepSerial();
    QString problem;
    // Bookmarks, without the hidden ones, by name without regard to case.
    QStringList names;
    for (const GoToItem &i : goToItems(*doc, GoToKind::Bookmark)) names << i.label;
    QCOMPARE(names, (QStringList{QStringLiteral("Alpha"), QStringLiteral("Middle")}));
    QCOMPARE(goToPosition(*doc, GoToKind::Bookmark, QStringLiteral("alpha"), 0), 0);
    const int middle = goToPosition(*doc, GoToKind::Bookmark, QStringLiteral("MIDDLE"), 0);
    QVERIFY(middle > 0);
    QVERIFY(doc->story()->findBlock(middle).text().contains(QStringLiteral("Middle")));
    QCOMPARE(goToPosition(*doc, GoToKind::Bookmark, QStringLiteral("_Hidden"), 0, &problem), -1);
    QVERIFY(!problem.isEmpty());
    // Headings, by number, relative to the caret, and by a part of their text.
    QCOMPARE(goToItems(*doc, GoToKind::Heading).size(), 2);
    const int second = goToPosition(*doc, GoToKind::Heading, QStringLiteral("2"), 0);
    QCOMPARE(doc->story()->findBlock(second).text(), QStringLiteral("Second part"));
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("second"), 0), second);
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("+1"), 0), second);
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("-1"), second + 3), 0);
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("3"), 0, &problem), -1);
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("nothing like it"), 0), -1);
    // Sections.
    QCOMPARE(goToPosition(*doc, GoToKind::Section, QStringLiteral("1"), 40), 0);
    const int section2 = goToPosition(*doc, GoToKind::Section, QStringLiteral("2"), 0);
    QCOMPARE(section2, second);
    QCOMPARE(goToPosition(*doc, GoToKind::Section, QStringLiteral("-1"), second + 1), 0);
    QCOMPARE(goToPosition(*doc, GoToKind::Section, QStringLiteral("3"), 0), -1);
    QCOMPARE(goToItems(*doc, GoToKind::Section).size(), 2);
    // Pages: the first position of each; relative; and none past the last.
    flow::Paginator &pager = doc->pager();
    const int pages = pager.pageCount();
    QVERIFY(pages >= 3);
    QCOMPARE(goToPosition(*doc, GoToKind::Page, QStringLiteral("1"), 500), 0);
    QCOMPARE(goToPosition(*doc, GoToKind::Page, QStringLiteral("2"), 0), pager.page(1).start.pos);
    QVERIFY(pager.page(1).start.pos > 0);
    QCOMPARE(goToPosition(*doc, GoToKind::Page, QStringLiteral("+1"), 0), pager.page(1).start.pos);
    QCOMPARE(goToPosition(*doc, GoToKind::Page, QStringLiteral("-1"), pager.page(2).start.pos), pager.page(1).start.pos);
    QCOMPARE(goToPosition(*doc, GoToKind::Page, QString::number(pages), 0), pager.page(pages - 1).start.pos);
    for (const char *bad : {"0", "-9", "99999", "abc", "", "99999999999999999999", "+2147483647"}) {
        problem.clear();
        QCOMPARE(goToPosition(*doc, GoToKind::Page, QString::fromLatin1(bad), 0, &problem), -1);
        QVERIFY2(!problem.isEmpty(), bad);
    }
    // Lines: counted from the top of the document.
    QCOMPARE(goToPosition(*doc, GoToKind::Line, QStringLiteral("1"), 0), 0);
    const QVector<StoryLayout::LineBrief> first = pager.lines(0);
    QVERIFY(first.size() > 3);
    QCOMPARE(goToPosition(*doc, GoToKind::Line, QStringLiteral("3"), 0), first[2].pos);
    QCOMPARE(goToPosition(*doc, GoToKind::Line, QString::number(first.size() + 1), 0), pager.lines(1)[0].pos);
    QCOMPARE(goToPosition(*doc, GoToKind::Line, QStringLiteral("9999999"), 0), -1);
    QCOMPARE(doc->nextStepSerial(), serial);   // (nothing above changed the document)
    // Footnotes: by number in the order of the text, whichever order they were made in.
    QTextCursor c(doc->story());
    c.setPosition(300);
    QVERIFY(insertFootnote(*doc, c));
    QTextCursor c2(doc->story());
    c2.setPosition(100);
    QVERIFY(insertFootnote(*doc, c2));
    QVector<int> refs;
    for (int id : doc->citedFootnotes()) refs += doc->footnoteReferences(id);
    std::sort(refs.begin(), refs.end());
    QCOMPARE(refs.size(), 2);
    QCOMPARE(goToPosition(*doc, GoToKind::Footnote, QStringLiteral("1"), 0), refs[0]);
    QCOMPARE(goToPosition(*doc, GoToKind::Footnote, QStringLiteral("2"), 0), refs[1]);
    QCOMPARE(goToPosition(*doc, GoToKind::Footnote, QStringLiteral("+1"), refs[0] + 1), refs[1]);
    QCOMPARE(goToPosition(*doc, GoToKind::Footnote, QStringLiteral("3"), 0), -1);
    QCOMPARE(goToItems(*doc, GoToKind::Footnote).size(), 2);
}


void WriteTests::goToIsHeldToWhatAHostileDocumentCanHold()
{
    QByteArray body;
    for (int i = 0; i < 6000; ++i)
        body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"" + QByteArray::number(i + 1) + "\" w:name=\"b" + QByteArray::number(i) + "\"/><w:r><w:t>H" + QByteArray::number(i) +
                "</w:t></w:r><w:bookmarkEnd w:id=\"" + QByteArray::number(i + 1) + "\"/></w:p>";
    body += "<w:p><w:bookmarkStart w:id=\"90000\" w:name=\"" + QByteArray(5000, 'z') + "\"/><w:r><w:t>Long bookmark name</w:t></w:r><w:bookmarkEnd w:id=\"90000\"/></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    QCOMPARE(goToItems(*doc, GoToKind::Heading).size(), kMostGoToItems);
    QCOMPARE(goToItems(*doc, GoToKind::Bookmark).size(), kMostGoToItems);
    for (const GoToItem &i : goToItems(*doc, GoToKind::Heading)) QVERIFY(i.label.size() <= 120);
    QVERIFY(goToPosition(*doc, GoToKind::Bookmark, QStringLiteral("b5999"), 0) > 0);   // (a bookmark can be gone to though it is not listed)
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("5000"), 0) > 0, true);
    QCOMPARE(goToPosition(*doc, GoToKind::Heading, QStringLiteral("5001"), 0), -1);
    QCOMPARE(goToPosition(*doc, GoToKind::Footnote, QStringLiteral("1"), 0), -1);
    for (GoToKind kind : {GoToKind::Page, GoToKind::Section, GoToKind::Line, GoToKind::Footnote, GoToKind::Heading})
        for (const char *bad : {"2147483648", "-2147483649", "+99999999999", "10000001", "0x10", "1e9"}) QCOMPARE(goToPosition(*doc, kind, QString::fromLatin1(bad), 0), -1);
}


void WriteTests::draggingSelectedTextMovesOrCopiesItInOneStep()
{
    auto doc = textDoc({QStringLiteral("One two three four five"), QStringLiteral("Next")});
    QTextCursor c(doc->story());
    c.setPosition(4);
    c.setPosition(8, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    c.setPosition(4);
    c.setPosition(8, QTextCursor::KeepAnchor);
    const quint64 before = doc->nextStepSerial();
    QVERIFY(moveSelectedText(*doc, c, 19, false));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QCOMPARE(plain(*doc), QStringLiteral("One three four two five\nNext"));
    QCOMPARE(c.selectedText(), QStringLiteral("two "));
    QCOMPARE(c.selectionStart(), 15);
    QVERIFY(styCharAt(*doc, 16).fontWeight() >= QFont::DemiBold);   // (it is bold where it went)
    QVERIFY(styCharAt(*doc, 5).fontWeight() < QFont::DemiBold);
    QVERIFY(doc->undo());
    QCOMPARE(plain(*doc), QStringLiteral("One two three four five\nNext"));
    QVERIFY(styCharAt(*doc, 5).fontWeight() >= QFont::DemiBold);
    QVERIFY(doc->redo());
    QCOMPARE(plain(*doc), QStringLiteral("One three four two five\nNext"));
    // Dropped on itself it does nothing; dropped before it (with the Ctrl key: copied), it goes there and the original stays.
    c.setPosition(15);
    c.setPosition(19, QTextCursor::KeepAnchor);
    const quint64 serial = doc->nextStepSerial();
    QVERIFY(!moveSelectedText(*doc, c, 17, false));
    QVERIFY(!moveSelectedText(*doc, c, 15, false));
    QVERIFY(!moveSelectedText(*doc, c, 19, true));
    QCOMPARE(doc->nextStepSerial(), serial);
    QVERIFY(moveSelectedText(*doc, c, 0, true));
    QCOMPARE(doc->nextStepSerial(), serial + 1);
    QCOMPARE(plain(*doc), QStringLiteral("two One three four two five\nNext"));
    QCOMPARE(c.selectedText(), QStringLiteral("two "));
    QVERIFY(styCharAt(*doc, 1).fontWeight() >= QFont::DemiBold);
    QVERIFY(styCharAt(*doc, 20).fontWeight() >= QFont::DemiBold);
    // Into another paragraph, and a selection of two paragraphs.
    QTextCursor two(doc->story());
    two.setPosition(0);
    two.setPosition(28, QTextCursor::KeepAnchor);
    QVERIFY(moveSelectedText(*doc, two, doc->story()->characterCount() - 1, false));
    QCOMPARE(plain(*doc), QStringLiteral("Nexttwo One three four two five\n"));
    QVERIFY(doc->undo());
    QVERIFY(doc->undo());
    QCOMPARE(plain(*doc), QStringLiteral("One three four two five\nNext"));
    // Nowhere to put it: no selection, a place past the text, and a place before it.
    QTextCursor none(doc->story());
    none.setPosition(2);
    QVERIFY(!moveSelectedText(*doc, none, 5, false));
    two.setPosition(0);
    two.setPosition(3, QTextCursor::KeepAnchor);
    QVERIFY(!moveSelectedText(*doc, two, 9999, false));
    QVERIFY(!moveSelectedText(*doc, two, -3, false));
    // Saved, and opened again, it is the same.
    QVERIFY(moveSelectedText(*doc, two, 10, false));
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QCOMPARE(plain(*back), plain(*doc));
}


// ---- the review of wave D

void WriteTests::aBuiltInStyleTheDocumentLacksIsAddedByARealChangeAloneAndOneUndoTakesItAway()
{
    const QByteArray zip = makeDocx("<w:p><w:r><w:t>Text</w:t></w:r></w:p>", kStyles);
    const QByteArray original = styleXml(zip);
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Title")));
    QTextCursor c(doc->story());
    // Looking at it, and a change that comes to the style as it is, add nothing and mark nothing.
    const std::optional<wml::Style> title = styleOfDocument(*doc, QStringLiteral("Title"));
    QVERIFY(title);
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("Title"), *title));
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Title")));
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
    QCOMPARE(styleXml(doc->toDocx()), original);
    // A real change adds it (and the built-in styles it is based on), in the one step.
    wml::Style changed = *title;
    changed.rPr.italic = true;
    const quint64 before = doc->nextStepSerial();
    QString problem;
    QVERIFY2(modifyStyle(*doc, c, QStringLiteral("Title"), changed, &problem), qPrintable(problem));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QVERIFY(doc->styleSheet().find(QStringLiteral("Title")));
    QVERIFY(styleXml(doc->toDocx()).contains("w:styleId=\"Title\""));
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Title")));
    QVERIFY(!doc->isModified());
    QCOMPARE(styleXml(doc->toDocx()), original);
    QVERIFY(doc->redo());
    QVERIFY(doc->styleSheet().find(QStringLiteral("Title")));
    QVERIFY(doc->undo());
    // A new style based on a built-in style the document lacks takes it along, and one Undo takes both.
    wml::Style fresh;
    fresh.name = QStringLiteral("Cover");
    fresh.basedOn = QStringLiteral("Subtitle");
    QVERIFY(!createStyle(*doc, c, fresh, &problem).isEmpty());
    QVERIFY(doc->styleSheet().find(QStringLiteral("Subtitle")));
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Subtitle")) && !doc->styleSheet().find(QStringLiteral("Cover")));
    QCOMPARE(styleXml(doc->toDocx()), original);
    // Update Style to Match Selection with nothing more said by the selection changes nothing.
    QVERIFY(updateStyleToSelection(*doc, c));
    QVERIFY(!doc->canUndo());
}


void WriteTests::theStyleSheetIsNotCopiedForEachRunOfAnAbsentBuiltInStyleWhenAStyleIsChanged()
{
    // Thirty thousand styles, a character style with two thousand more based on it, and paragraphs of a built-in style the document lacks (Title) whose runs have those.
    QByteArray styles = kStyles;
    for (int i = 0; i < 30000; ++i) styles += styCustomStyle(QStringLiteral("U%1").arg(i), QStringLiteral("Unused %1").arg(i), "<w:basedOn w:val=\"Normal\"/>");
    styles += styCustomStyle(QStringLiteral("Root"), QStringLiteral("Root"), "<w:rPr><w:sz w:val=\"20\"/></w:rPr>", "character");
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Title\"/></w:pPr>";
    for (int i = 0; i < 2000; ++i) {
        styles += styCustomStyle(QStringLiteral("K%1").arg(i), QStringLiteral("Kid %1").arg(i), "<w:basedOn w:val=\"Root\"/>", "character");
        body += "<w:r><w:rPr><w:rStyle w:val=\"K" + QByteArray::number(i) + "\"/></w:rPr><w:t>x</w:t></w:r>";
    }
    body += "</w:p>";
    auto doc = open(makeDocx(body, styles));
    QVERIFY(doc);
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Title")));
    QTextCursor c(doc->story());
    wml::Style root = styStyle(*doc, QStringLiteral("Root"));
    root.rPr.size = 30;
    const qint64 copies = doc->sheetCopiesForTest();
    QString problem;
    QVERIFY2(modifyStyle(*doc, c, QStringLiteral("Root"), root, &problem), qPrintable(problem));
    // (the sheet is copied to look at the built-in style once for each look worked out, not once for each of the two thousand)
    QVERIFY2(doc->sheetCopiesForTest() - copies <= 6, qPrintable(QString::number(doc->sheetCopiesForTest() - copies)));
    QCOMPARE(styCharAt(*doc, 5).fontPointSize(), 15.0);
    QVERIFY(doc->undo());
    QCOMPARE(styCharAt(*doc, 5).fontPointSize(), 10.0);
}


void WriteTests::aStyleThatUpdatesItselfFollowsOneParagraphsFormattingByHandAndNothingElse()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("Auto"), QStringLiteral("Auto"), "<w:basedOn w:val=\"Normal\"/><w:autoRedefine/>");
    const QByteArray para = "<w:p><w:pPr><w:pStyle w:val=\"Auto\"/></w:pPr><w:r><w:t>Paragraph</w:t></w:r></w:p>";
    auto doc = open(makeDocx(para + para + para, styles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    auto rest = [&] { return styStyle(*doc, QStringLiteral("Auto")); };
    // Formatting two paragraphs at once is not one paragraph's look: the style stays.
    c.setPosition(0);
    c.setPosition(15, QTextCursor::KeepAnchor);
    quint64 before = doc->nextStepSerial();
    toggleBold(*doc, c);
    setAlignment(*doc, c, Qt::AlignHCenter);
    QCOMPARE(doc->nextStepSerial(), before + 2);
    QVERIFY(!rest().rPr.bold && rest().pPr.jc.isEmpty());
    // Neither is a paint, or Clear All Formatting.
    FormatBrush brush;
    brush.valid = true;
    brush.chars.setFontItalic(true);
    QTextCursor one(doc->story());
    one.setPosition(0);
    one.setPosition(5, QTextCursor::KeepAnchor);
    QVERIFY(pasteFormat(*doc, one, brush));
    QVERIFY(!rest().rPr.italic);
    clearFormatting(*doc, one);
    QVERIFY(!rest().rPr.bold && !rest().rPr.italic);
    // What a person does to one paragraph's look still makes the style follow it, in the step of the command.
    before = doc->nextStepSerial();
    one.setPosition(20);
    one.setPosition(25, QTextCursor::KeepAnchor);
    toggleBold(*doc, one);
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QCOMPARE(rest().rPr.bold, std::optional<bool>(true));
    // The step holds the style as it was and as it is, the bytes it was read as only once (the new look is not kept as bytes).
    QVERIFY(doc->styleStepsHeldForTest() >= 1);
}


void WriteTests::theSectionsAreFoundInOnePassOverTheParagraphs()
{
    QByteArray body;
    for (int i = 0; i < 3000; ++i) body += "<w:p><w:pPr>" + sectionXml(12240, 15840, 1440, 1440, 1440, 1440) + "</w:pPr><w:r><w:t>S" + QByteArray::number(i) + "</w:t></w:r></w:p>";
    body += "<w:p><w:r><w:t>Last</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sectionXml(12240, 15840, 1440, 1440, 1440, 1440)));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 3001);
    const qint64 before = goToBlocksVisitedForTest();
    const QVector<GoToItem> items = goToItems(*doc, GoToKind::Section);
    QCOMPARE(items.size(), 3001);
    QCOMPARE(items[1].position, doc->story()->findBlockByNumber(1).position());
    QCOMPARE(items[3000].position, doc->story()->findBlockByNumber(3000).position());
    QVERIFY2(goToBlocksVisitedForTest() - before <= 3100, qPrintable(QString::number(goToBlocksVisitedForTest() - before)));   // (not 3001 times 3001)
    const qint64 one = goToBlocksVisitedForTest();
    QCOMPARE(goToPosition(*doc, GoToKind::Section, QStringLiteral("2000"), 0), doc->story()->findBlockByNumber(1999).position());
    QVERIFY(goToBlocksVisitedForTest() - one <= 2100);
}


void WriteTests::theListOfStylesAlwaysHoldsTheOnesTheTextUsesAndTheCaretsWhateverTheCut()
{
    QByteArray styles;
    styManyStyles(9000, &styles);   // (more than the list holds, in the order of the file)
    auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"S8999\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"S8500\"/></w:rPr><w:t>Used late</w:t></w:r></w:p>", styles));
    QVERIFY(doc);
    for (StyleFilter filter : {StyleFilter::All, StyleFilter::Recommended, StyleFilter::InUse}) {
        const QVector<ListedStyle> list = listStyles(*doc, filter, QStringLiteral("S8998"));
        QVERIFY(list.size() <= kMostStylesListed);
        const QStringList ids = styIds(list);
        QVERIFY(ids.contains(QStringLiteral("S8999")));
        QVERIFY(ids.contains(QStringLiteral("Normal")));
        if (filter != StyleFilter::InUse) QVERIFY2(ids.contains(QStringLiteral("S8998")), "the caret's style");
    }
    QVERIFY(styIds(listStyles(*doc, StyleFilter::All, QStringLiteral("S8998"))).size() == kMostStylesListed);
}


void WriteTests::aSelectionHoldingADeletedParagraphIsMovedToWhereItWasDropped()
{
    const QByteArray del = "<w:del w:id=\"%1\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"/>";
    QByteArray body = "<w:p><w:r><w:t>Alpha</w:t></w:r></w:p>";
    body += "<w:p><w:pPr><w:rPr>" + QByteArray(del).replace("%1", "1") + "</w:rPr></w:pPr><w:del w:id=\"2\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:delText>gone</w:delText></w:r></w:del></w:p>";
    body += "<w:p><w:r><w:t>Beta</w:t></w:r></w:p><w:p><w:r><w:t>Omega</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const QTextBlock gone = doc->story()->findBlockByNumber(1), beta = doc->story()->findBlockByNumber(2), omega = doc->story()->findBlockByNumber(3);
    QCOMPARE(beta.text(), QStringLiteral("Beta"));
    QTextCursor c(doc->story());
    c.setPosition(gone.position());
    c.setPosition(beta.position() + 4, QTextCursor::KeepAnchor);
    const int toEnd = omega.position() + 5;
    QVERIFY(moveSelectedText(*doc, c, toEnd, false));
    // "Omega" is whole and what was moved comes after it.
    const QString text = plain(*doc);
    QVERIFY2(text.contains(QStringLiteral("Omega")), qPrintable(text));
    QVERIFY2(text.indexOf(QStringLiteral("Beta")) > text.indexOf(QStringLiteral("Omega")), qPrintable(text));
    QVERIFY(doc->undo());
    QCOMPARE(doc->story()->findBlockByNumber(2).text(), QStringLiteral("Beta"));
    QCOMPARE(doc->story()->findBlockByNumber(3).text(), QStringLiteral("Omega"));
}


void WriteTests::aSelectionHoldingAFootnoteReferenceIsMovedToWhereItWasDropped()
{
    auto doc = textDoc({QStringLiteral("One two three four five six")});
    QTextCursor note(doc->story());
    note.setPosition(7);
    QVERIFY(insertFootnote(*doc, note));
    QTextCursor c(doc->story());
    c.setPosition(4);
    c.setPosition(14, QTextCursor::KeepAnchor);
    const QString moving = c.selectedText();
    const int to = doc->story()->characterCount() - 1;
    QVERIFY(moveSelectedText(*doc, c, to, false));
    QString text = plain(*doc), words = moving;
    text.remove(QChar::ObjectReplacementCharacter);
    words.remove(QChar::ObjectReplacementCharacter);
    const QString base = QStringLiteral("One two three four five six");
    QCOMPARE(text, base.left(4) + base.mid(4 + words.size()) + words);
    QCOMPARE(doc->citedFootnotes().size(), 1);
    QCOMPARE(doc->footnoteReferences(doc->citedFootnotes().first()).size(), 1);
    QVERIFY(doc->undo());
    QCOMPARE(doc->citedFootnotes().size(), 1);
}


void WriteTests::aStyleChangeNoStepTookIsPutBackAndDoesNotJoinTheNextStep()
{
    auto doc = textDoc({QStringLiteral("Text")});
    QTextCursor c(doc->story());
    doc->story()->setUndoRedoEnabled(false);   // (a story that keeps no history: no step can take the change)
    wml::Style s;
    s.name = QStringLiteral("Lost");
    QString problem;
    QVERIFY(createStyle(*doc, c, s, &problem).isEmpty());
    QVERIFY(!problem.isEmpty());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Lost")));
    QVERIFY(!doc->isModified());
    doc->story()->setUndoRedoEnabled(true);
    c.setPosition(4);
    insertText(*doc, c, QStringLiteral("!"));
    QCOMPARE(doc->styleStepsHeldForTest(), 0);
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Lost")));
}


void WriteTests::aStyleKeepsAnUnknownChildAndAttributeThroughModifyAndSave()
{
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:customStyle=\"1\" w:styleId=\"Odd\" w:rsidR=\"00AB12CD\"><w:name w:val=\"Odd\"/><w:basedOn w:val=\"Normal\"/><w:w14x w:val=\"keep me\"/><w:qFormat/><w:pPr><w:keepNext/></w:pPr></w:style>";
    auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Odd\"/></w:pPr><w:r><w:t>Text</w:t></w:r></w:p>", styles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    wml::Style odd = styStyle(*doc, QStringLiteral("Odd"));
    odd.rPr.italic = true;
    QVERIFY(modifyStyle(*doc, c, QStringLiteral("Odd"), odd));
    const QByteArray saved = styleXml(doc->toDocx());
    QVERIFY2(saved.contains("<w:w14x w:val=\"keep me\"/>"), saved.right(900));
    QVERIFY2(saved.contains("w:rsidR=\"00AB12CD\""), saved.right(900));
    auto back = open(doc->toDocx());
    QVERIFY(back);
    QVERIFY(styleXml(back->toDocx()).contains("w:rsidR=\"00AB12CD\""));
}


// ---- follow-ups of wave D

void WriteTests::applyingABuiltInStyleTheDocumentLacksIsOneStepWithItsStyleAndUndoRestoresTheFile()
{
    const QByteArray plainStyles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
    const QByteArray zip = makeDocx("<w:p><w:r><w:t>Some words</w:t></w:r></w:p><w:p><w:r><w:t>More</w:t></w:r></w:p>", plainStyles);
    const QByteArray original = styleXml(zip);
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Heading1")) && !doc->styleSheet().find(QStringLiteral("Strong")));
    QTextCursor c(doc->story());
    c.setPosition(2);
    const quint64 before = doc->nextStepSerial();
    applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
    QCOMPARE(doc->nextStepSerial(), before + 1);
    QVERIFY(doc->styleSheet().find(QStringLiteral("Heading1")));
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Heading1")));
    QVERIFY(!doc->isModified());
    QCOMPARE(styleXml(doc->toDocx()), original);
    QVERIFY(doc->redo());
    QVERIFY(doc->styleSheet().find(QStringLiteral("Heading1")));
    QCOMPARE(styBlockStyle(*doc, 2), QStringLiteral("Heading1"));
    QVERIFY(doc->undo());
    // A character style, and the style that follows a heading when Enter is pressed at its end, are in their steps as well.
    c.setPosition(0);
    c.setPosition(4, QTextCursor::KeepAnchor);
    const quint64 serial = doc->nextStepSerial();
    applyCharacterStyle(*doc, c, QStringLiteral("Strong"));
    QCOMPARE(doc->nextStepSerial(), serial + 1);
    QVERIFY(doc->styleSheet().find(QStringLiteral("Strong")));
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("Strong")));
    QCOMPARE(styleXml(doc->toDocx()), original);
}


void WriteTests::insertingAFootnoteIntoADocumentThatLacksTheFootnoteStylesPutsThemInTheStepAndUndoTakesThemOut()
{
    const QByteArray plainStyles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
    const QByteArray zip = makeDocx("<w:p><w:r><w:t>Some words here</w:t></w:r></w:p>", plainStyles);
    const QByteArray original = styleXml(zip);
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(!doc->styleSheet().find(QStringLiteral("FootnoteText")));
    QTextCursor c(doc->story());
    c.setPosition(4);
    QVERIFY(insertFootnote(*doc, c));
    QVERIFY(doc->styleSheet().find(QStringLiteral("FootnoteText")) && doc->styleSheet().find(QStringLiteral("FootnoteReference")));
    QVERIFY(doc->undo());
    QVERIFY(!doc->styleSheet().find(QStringLiteral("FootnoteText")) && !doc->styleSheet().find(QStringLiteral("FootnoteReference")));
    QCOMPARE(styleXml(doc->toDocx()), original);
    QVERIFY(doc->redo());
    QVERIFY(doc->styleSheet().find(QStringLiteral("FootnoteText")));
    QVERIFY(styleXml(doc->toDocx()).contains("FootnoteText"));
}


void WriteTests::aSectionBreakOnTheLastParagraphIsOneSectionAtItsFirstParagraph()
{
    QByteArray body = "<w:p><w:r><w:t>One</w:t></w:r></w:p>";
    body += "<w:p><w:pPr>" + sectionXml(12240, 15840, 1440, 1440, 1440, 1440) + "</w:pPr><w:r><w:t>Two</w:t></w:r></w:p>";
    body += "<w:p><w:r><w:t>Three</w:t></w:r></w:p>";
    body += "<w:p><w:pPr>" + sectionXml(12240, 15840, 1440, 1440, 1440, 1440) + "</w:pPr><w:r><w:t>Last</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sectionXml(12240, 15840, 1440, 1440, 1440, 1440)));
    QVERIFY(doc);
    const QVector<GoToItem> items = goToItems(*doc, GoToKind::Section);
    QCOMPARE(items.size(), doc->sectionCount());
    QVERIFY(items.size() >= 2);
    QCOMPARE(items[0].position, 0);
    QCOMPARE(items[1].position, doc->story()->findBlockByNumber(2).position());
    // (no two sections start at one place)
    for (int i = 1; i < items.size(); ++i) QVERIFY2(items[i].position > items[i - 1].position, qPrintable(QString::number(i)));
    // and each is found by its number, at the same place
    for (int i = 0; i < items.size(); ++i) QCOMPARE(goToPosition(*doc, GoToKind::Section, QString::number(i + 1), 0), items[i].position);
}


// ---- wave D in the window: the Styles pane, the style dialogs, Copy Formatting, Go To, and dragging text

std::unique_ptr<WriteWindow> WriteTests::styWindow(const QByteArray &body, const QByteArray &styles)
{
    auto w = std::make_unique<WriteWindow>();
    w->resize(1000, 700);
    w->setDocument(open(makeDocx(body, styles)), QString());
    w->show();
    w->refreshNow();
    return w;
}

QPoint WriteTests::styPointOf(PageView *view, int position)
{
    QTextCursor c(view->document()->story());
    c.setPosition(position);
    view->showCursor(c);
    QRect r;
    if (!view->caretRect(&r)) return QPoint();
    return QPoint(r.center().x() + 1, r.center().y());
}


void WriteTests::theStylesPaneShowsTheStylesFiltersThemAndAppliesOneWithAClick()
{
    const QByteArray styles = kStyles + styCustomStyle(QStringLiteral("Stealth"), QStringLiteral("Stealth note"), "<w:basedOn w:val=\"Normal\"/><w:semiHidden/>");
    auto w = styWindow("<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Heading words</w:t></w:r></w:p><w:p><w:r><w:t>Body words</w:t></w:r></w:p>", styles);
    QVERIFY(!w->stylesPane());
    QAction *command = w->act(QStringLiteral("style.pane"));
    QVERIFY(command);
    QCOMPARE(command->shortcut(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_S));
    command->trigger();
    StylesPane *pane = w->stylesPane();
    QVERIFY(pane);
    QVERIFY(pane->isVisible());
    // The caret is in the heading: its style is the one marked, and the list can be of the styles in use, all, or those recommended.
    QCOMPARE(pane->currentKey(), QStringLiteral("p:Heading1"));
    pane->setFilter(write::StyleFilter::InUse);
    QVERIFY(pane->rowOf(QStringLiteral("p:Heading1")) >= 0 && pane->rowOf(QStringLiteral("p:Normal")) >= 0);
    QCOMPARE(pane->rowOf(QStringLiteral("p:Stealth")), -1);
    QCOMPARE(pane->list()->count(), 2);
    QVERIFY(pane->list()->item(pane->rowOf(QStringLiteral("p:Heading1")))->font().bold());
    QVERIFY(!pane->list()->item(pane->rowOf(QStringLiteral("p:Normal")))->font().bold());
    pane->setFilter(write::StyleFilter::All);
    QVERIFY(pane->rowOf(QStringLiteral("p:Stealth")) >= 0 && pane->rowOf(QStringLiteral("c:Strong")) >= 0 && pane->rowOf(QStringLiteral("p:Title")) >= 0);
    pane->setFilter(write::StyleFilter::Recommended);
    QCOMPARE(pane->rowOf(QStringLiteral("p:Stealth")), -1);
    QVERIFY(pane->rowOf(QStringLiteral("p:Title")) >= 0);
    // A click on a style gives it to the selected paragraph, as one step.
    QTextCursor c(w->document()->story());
    c.setPosition(17);
    w->view()->setCursor(c);
    w->refreshNow();
    QCOMPARE(pane->currentKey(), QStringLiteral("p:Normal"));
    const int row = pane->rowOf(QStringLiteral("p:Heading1"));
    QVERIFY(row >= 0);
    const quint64 serial = w->document()->nextStepSerial();
    QTest::mouseClick(pane->list()->viewport(), Qt::LeftButton, Qt::NoModifier, pane->list()->visualItemRect(pane->list()->item(row)).center());
    QCOMPARE(styBlockStyle(*w->document(), 17), QStringLiteral("Heading1"));
    QCOMPARE(w->document()->nextStepSerial(), serial + 1);
    QCOMPARE(pane->currentKey(), QStringLiteral("p:Heading1"));
    QTRY_VERIFY(w->act(QStringLiteral("edit.undo"))->isEnabled());
    w->act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(styBlockStyle(*w->document(), 17), QStringLiteral("Normal"));
    // A character style goes to the characters selected.
    c.setPosition(17);
    c.setPosition(21, QTextCursor::KeepAnchor);
    w->view()->setCursor(c);
    pane->setFilter(write::StyleFilter::All);
    pane->apply(pane->rowOf(QStringLiteral("c:Strong")));
    QCOMPARE(styCharAt(*w->document(), 18).stringProperty(tp::CharStyle), QStringLiteral("Strong"));
    QVERIFY(styIds(listStyles(*w->document(), StyleFilter::InUse)).contains(QStringLiteral("Strong")));
    // The pane follows the document: a style made shows in it.
    QTextCursor sel(w->document()->story());
    sel.setPosition(0);
    sel.setPosition(5, QTextCursor::KeepAnchor);
    QVERIFY(!createStyle(*w->document(), sel, [] { wml::Style s; s.name = QStringLiteral("Zebra"); return s; }()).isEmpty());
    w->refreshNow();
    QVERIFY(pane->rowOf(QStringLiteral("p:Zebra")) >= 0);
    QVERIFY(w->document()->undo());
    w->refreshNow();
    QCOMPARE(pane->rowOf(QStringLiteral("p:Zebra")), -1);
}


void WriteTests::theCreateStyleDialogMakesTheStyleOfTheSelectionAndOneUndoTakesItBack()
{
    auto w = styWindow("<w:p><w:r><w:t>Alpha beta</w:t></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r></w:p>");
    selectRange(*w, 0, 5);
    const quint64 serial = w->document()->nextStepSerial();
    withDialog([&] { w->act(QStringLiteral("style.new"))->trigger(); }, QStringLiteral("styleDialog"), [](QDialog *d) {
        QCOMPARE(d->windowTitle(), QStringLiteral("Create New Style from Formatting"));
        auto *name = d->findChild<QLineEdit *>(QStringLiteral("styleName"));
        QVERIFY(name && name->isEnabled());
        name->setText(QStringLiteral("Callout"));
        auto *size = d->findChild<QComboBox *>(QStringLiteral("styleSize"));
        QVERIFY(size);
        size->setEditText(QStringLiteral("20"));
        d->findChild<QCheckBox *>(QStringLiteral("styleBold"))->setChecked(true);
        auto *align = d->findChild<QComboBox *>(QStringLiteral("styleAlign"));
        align->setCurrentIndex(align->findData(QStringLiteral("center")));
        QVERIFY(d->findChild<QComboBox *>(QStringLiteral("styleBasedOn"))->currentData().toString() == QLatin1String("Normal"));
    });
    WriteDoc &doc = *w->document();
    QCOMPARE(doc.nextStepSerial(), serial + 1);
    const wml::Style made = styStyle(doc, QStringLiteral("Callout"));
    QCOMPARE(made.name, QStringLiteral("Callout"));
    QCOMPARE(made.rPr.size, std::optional<int>(40));
    QCOMPARE(made.rPr.bold, std::optional<bool>(true));
    QCOMPARE(made.pPr.jc, QStringLiteral("center"));
    QVERIFY(made.customStyle);
    QCOMPARE(styBlockStyle(doc, 2), QStringLiteral("Callout"));
    QCOMPARE(styCharAt(doc, 2).fontPointSize(), 20.0);
    QCOMPARE(styBlockStyle(doc, 11), QStringLiteral("Normal"));
    w->act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(!doc.styleSheet().find(QStringLiteral("Callout")));
    QCOMPARE(styBlockStyle(doc, 2), QStringLiteral("Normal"));
    w->refreshNow();
    w->act(QStringLiteral("edit.redo"))->trigger();
    QVERIFY(doc.styleSheet().find(QStringLiteral("Callout")));
    // The style is in the file.
    auto back = open(doc.toDocx());
    QVERIFY(back);
    QCOMPARE(styStyle(*back, QStringLiteral("Callout")).rPr.size, std::optional<int>(40));
    QCOMPARE(styBlockStyle(*back, 2), QStringLiteral("Callout"));
    // A name another style has keeps the dialog open and says why; Cancel leaves everything as it was.
    const quint64 now = doc.nextStepSerial();
    withDialog([&] { w->act(QStringLiteral("style.new"))->trigger(); }, QStringLiteral("styleDialog"), [](QDialog *d) {
        d->findChild<QLineEdit *>(QStringLiteral("styleName"))->setText(QStringLiteral("heading 1"));
        d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        QVERIFY(d->isVisible());
        QVERIFY(!d->findChild<QLabel *>(QStringLiteral("styleProblem"))->text().isEmpty());
    }, false);
    QCOMPARE(doc.nextStepSerial(), now);
}


void WriteTests::theModifyStyleDialogChangesTheStyleAndWithNoChangeLeavesTheFileAsItWas()
{
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Heading words</w:t></w:r></w:p><w:p><w:r><w:t>Body words</w:t></w:r></w:p>";
    const QByteArray zip = makeDocx(body, kStyles);
    auto w = std::make_unique<WriteWindow>();
    w->resize(1000, 700);
    w->setDocument(open(zip), QString());
    w->show();
    w->refreshNow();
    const QByteArray original = styleXml(zip);
    // OK with nothing changed: no step, the file not marked as changed, styles.xml the same bytes.
    withDialog([&] { w->act(QStringLiteral("style.modify"))->trigger(); }, QStringLiteral("styleDialog"), [](QDialog *d) {
        QCOMPARE(d->windowTitle(), QStringLiteral("Modify Style"));
        QVERIFY(!d->findChild<QLineEdit *>(QStringLiteral("styleName"))->isEnabled());   // (a built-in style keeps its name)
        QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("styleName"))->text(), QStringLiteral("heading 1"));
        QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("styleSize"))->currentText(), QStringLiteral("17"));
        QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("styleBold"))->isChecked());
    });
    QVERIFY(!w->document()->canUndo());
    QVERIFY(!w->isModified());
    QCOMPARE(styleXml(w->document()->toDocx()), original);
    // A change: the style, the text that has it, and one step.
    const quint64 serial = w->document()->nextStepSerial();
    withDialog([&] { w->act(QStringLiteral("style.modify"))->trigger(); }, QStringLiteral("styleDialog"), [](QDialog *d) {
        d->findChild<QComboBox *>(QStringLiteral("styleSize"))->setEditText(QStringLiteral("30"));
        d->findChild<QCheckBox *>(QStringLiteral("styleItalic"))->setChecked(true);
        d->findChild<QCheckBox *>(QStringLiteral("styleAutomatic"))->setChecked(true);
    });
    WriteDoc &doc = *w->document();
    QCOMPARE(doc.nextStepSerial(), serial + 1);
    QCOMPARE(styStyle(doc, QStringLiteral("Heading1")).rPr.size, std::optional<int>(60));
    QVERIFY(styStyle(doc, QStringLiteral("Heading1")).autoRedefine);
    QCOMPARE(styCharAt(doc, 3).fontPointSize(), 30.0);
    QVERIFY(styCharAt(doc, 3).fontItalic());
    QVERIFY(w->isModified());
    w->act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(styleXml(doc.toDocx()), original);
    QVERIFY(!w->isModified());
    // The command for the style of the Styles pane's chosen row, and Update to Match Selection (the caret in the body, a size given by hand).
    QTextCursor c(doc.story());
    c.setPosition(18);
    c.setPosition(22, QTextCursor::KeepAnchor);
    w->view()->setCursor(c);
    setFontSize(doc, w->view()->cursor(), 14);
    w->refreshNow();
    const quint64 before = doc.nextStepSerial();
    w->act(QStringLiteral("style.update"))->trigger();
    QCOMPARE(doc.nextStepSerial(), before + 1);
    QCOMPARE(styStyle(doc, QStringLiteral("Normal")).rPr.size, std::optional<int>(28));
}


// A release that never came (the mouse let go outside the window) does not leave the drag on: a move with no button held ends it, and a later move with the button down selects nothing.
void WriteTests::aDragOfTheMouseInTextEndsWhenNoButtonIsHeld()
{
    auto w = styWindow("<w:p><w:r><w:t>Alpha beta gamma delta epsilon</w:t></w:r></w:p>");
    PageView *view = w->view();
    w->refreshNow();
    const QPoint from = styPointOf(view, 11), to = styPointOf(view, 16);
    auto move = [&](const QPoint &p, Qt::MouseButtons held) {
        QMouseEvent ev(QEvent::MouseMove, QPointF(p), QPointF(view->viewport()->mapToGlobal(p)), Qt::NoButton, held, Qt::NoModifier);
        QCoreApplication::sendEvent(view->viewport(), &ev);
    };
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, from);
    move(from + QPoint(1, 0), Qt::NoButton);                                        // (the release was lost)
    move(to, Qt::LeftButton);
    QVERIFY2(!view->cursor().hasSelection(), qPrintable(view->cursor().selectedText()));
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, to);
}


void WriteTests::theFormatPainterPaintsOnceOrStaysOnUntilEscapeAndEachPaintIsOneStep()
{
    auto w = styWindow("<w:p><w:r><w:t>Alpha beta gamma delta epsilon</w:t></w:r></w:p>");
    PageView *view = w->view();
    QAction *painter = w->act(QStringLiteral("edit.formatPainter"));
    QVERIFY(painter);
    QCOMPARE(w->painterMode(), WriteWindow::PainterMode::Off);
    selectRange(*w, 0, 5);
    toggleBold(*w->document(), view->cursor());
    selectRange(*w, 0, 5);
    w->refreshNow();
    // One use: the next selection made with the mouse is painted, then the painter is off.
    painter->trigger();
    QCOMPARE(w->painterMode(), WriteWindow::PainterMode::Once);
    QVERIFY(painter->isChecked());
    const QPoint from = styPointOf(view, 11), to = styPointOf(view, 16);
    const quint64 serial = w->document()->nextStepSerial();
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, from);
    QTest::mouseMove(view->viewport(), to);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QCOMPARE(view->cursor().selectedText(), QStringLiteral("gamma"));
    QCOMPARE(w->document()->nextStepSerial(), serial + 1);
    QVERIFY(styCharAt(*w->document(), 12).fontWeight() >= QFont::DemiBold);
    QVERIFY(styCharAt(*w->document(), 7).fontWeight() < QFont::DemiBold);
    QCOMPARE(w->painterMode(), WriteWindow::PainterMode::Off);
    QVERIFY(!painter->isChecked());
    // Not painting again: the next selection changes nothing.
    const QPoint d1 = styPointOf(view, 17), d2 = styPointOf(view, 22);
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, d1);
    QTest::mouseMove(view->viewport(), d2);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, d2);
    QCOMPARE(w->document()->nextStepSerial(), serial + 1);
    // A double click on the button keeps it on: every selection is painted, each as a step, until Esc.
    selectRange(*w, 0, 5);
    auto *button = [&]() -> QToolButton * {
        for (QToolButton *b : w->ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() == painter) return b;
        return nullptr;
    }();
    QVERIFY(button);
    QTest::mouseDClick(button, Qt::LeftButton);
    QCOMPARE(w->painterMode(), WriteWindow::PainterMode::Locked);
    QVERIFY(painter->isChecked());
    const quint64 locked = w->document()->nextStepSerial();
    for (const std::pair<int, int> &range : {std::pair<int, int>(6, 10), std::pair<int, int>(23, 30)}) {
        const QPoint a = styPointOf(view, range.first), b = styPointOf(view, range.second);
        QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, a);
        QTest::mouseMove(view->viewport(), b);
        QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, b);
        QCOMPARE(w->painterMode(), WriteWindow::PainterMode::Locked);
    }
    QCOMPARE(w->document()->nextStepSerial(), locked + 2);
    QVERIFY(styCharAt(*w->document(), 7).fontWeight() >= QFont::DemiBold);
    QVERIFY(styCharAt(*w->document(), 25).fontWeight() >= QFont::DemiBold);
    QTest::keyClick(view, Qt::Key_Escape);
    QCOMPARE(w->painterMode(), WriteWindow::PainterMode::Off);
    QVERIFY(!painter->isChecked());
    // Each paint is one Undo.
    w->act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(styCharAt(*w->document(), 25).fontWeight() < QFont::DemiBold);
    QVERIFY(styCharAt(*w->document(), 7).fontWeight() >= QFont::DemiBold);
    // The keys: Copy Format and Paste Format.
    QCOMPARE(w->act(QStringLiteral("edit.copyFormat"))->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    QCOMPARE(w->act(QStringLiteral("edit.pasteFormat"))->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    selectRange(*w, 0, 5);
    w->act(QStringLiteral("edit.copyFormat"))->trigger();
    selectRange(*w, 31, 38);
    w->act(QStringLiteral("edit.pasteFormat"))->trigger();
    QVERIFY(styCharAt(*w->document(), 33).fontWeight() >= QFont::DemiBold);
}


void WriteTests::goToOpensWithCtrlGAndPutsTheCaretAtThePlaceNamed()
{
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>First heading</w:t></w:r></w:p>";
    for (int i = 0; i < 6; ++i) body += "<w:p><w:r><w:t>Some text of paragraph " + QByteArray::number(i) + ".</w:t></w:r></w:p>";
    body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"Target\"/><w:r><w:t>Second heading</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p><w:p><w:r><w:t>Last.</w:t></w:r></w:p>";
    auto w = styWindow(body);
    QAction *goTo = w->act(QStringLiteral("edit.goTo"));
    QVERIFY(goTo);
    QCOMPARE(goTo->shortcut().toString(QKeySequence::NativeText), QKeySequence(Qt::CTRL | Qt::Key_G).toString(QKeySequence::NativeText));
    const int second = w->document()->story()->findBlock(w->document()->story()->toPlainText().indexOf(QStringLiteral("Second heading"))).position();
    withDialog([&] { goTo->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
        auto *kinds = d->findChild<QListWidget *>(QStringLiteral("goToKinds"));
        QCOMPARE(kinds->count(), 12);
        kinds->setCurrentRow(11);   // Heading
        auto *choices = d->findChild<QListWidget *>(QStringLiteral("goToChoices"));
        QCOMPARE(choices->count(), 2);
        choices->setCurrentRow(1);
        QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->text(), QStringLiteral("Second heading"));
    });
    QCOMPARE(w->view()->cursor().position(), second);
    // A bookmark by name, and a place that is not there keeps the dialog open and says so.
    selectRange(*w, 0, 0);
    withDialog([&] { goTo->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
        d->findChild<QListWidget *>(QStringLiteral("goToKinds"))->setCurrentRow(3);   // Bookmark
        d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("target"));
    });
    QCOMPARE(w->view()->cursor().position(), second);
    selectRange(*w, 3, 3);
    withDialog([&] { goTo->trigger(); }, QStringLiteral("goToDialog"), [](QDialog *d) {
        d->findChild<QListWidget *>(QStringLiteral("goToKinds"))->setCurrentRow(0);   // Page
        d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("40"));
        d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        QVERIFY(d->isVisible());
        QVERIFY(!d->findChild<QLabel *>(QStringLiteral("goToProblem"))->text().isEmpty());
    }, false);
    QCOMPARE(w->view()->cursor().position(), 3);
    QVERIFY(!w->document()->canUndo());   // (going to a place changes nothing)
}


void WriteTests::dragAndDropMovesSelectedTextOrCopiesItWithCtrlInOneStep()
{
    auto w = styWindow("<w:p><w:r><w:t>One two three four five</w:t></w:r></w:p>");
    PageView *view = w->view();
    WriteDoc &doc = *w->document();
    auto drop = [&](QMimeData *mime, int position, Qt::KeyboardModifiers modifiers) {
        const QPoint at = styPointOf(view, position);
        // (A drop goes to the widget that accepted the drag when it came in.)
        QDragEnterEvent enter(at, Qt::CopyAction | Qt::MoveAction, mime, Qt::LeftButton, modifiers);
        QApplication::sendEvent(view->viewport(), &enter);
        if (!enter.isAccepted()) return false;
        QDropEvent e(QPointF(at), Qt::CopyAction | Qt::MoveAction, mime, Qt::LeftButton, modifiers);
        QApplication::sendEvent(view->viewport(), &e);
        return e.isAccepted();
    };
    selectRange(*w, 4, 8);
    std::unique_ptr<QMimeData> mime(view->prepareTextDrag());
    QVERIFY(mime);
    QCOMPARE(mime->text(), QStringLiteral("two "));
    const quint64 serial = doc.nextStepSerial();
    QVERIFY(drop(mime.get(), 19, Qt::NoModifier));
    QCOMPARE(plain(doc), QStringLiteral("One three four two five"));
    QCOMPARE(view->cursor().selectedText(), QStringLiteral("two "));
    QCOMPARE(doc.nextStepSerial(), serial + 1);
    w->refreshNow();
    w->act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(plain(doc), QStringLiteral("One two three four five"));
    w->refreshNow();
    w->act(QStringLiteral("edit.redo"))->trigger();
    QCOMPARE(plain(doc), QStringLiteral("One three four two five"));
    // With the Ctrl key it is copied; dropped on the selection itself it does nothing.
    selectRange(*w, 15, 19);
    mime.reset(view->prepareTextDrag());
    const quint64 next = doc.nextStepSerial();
    drop(mime.get(), 17, Qt::NoModifier);
    QCOMPARE(doc.nextStepSerial(), next);
    QVERIFY(drop(mime.get(), 0, Qt::ControlModifier));
    QCOMPARE(plain(doc), QStringLiteral("two One three four two five"));
    QCOMPARE(doc.nextStepSerial(), next + 1);
    // Text from outside the document is put where it is dropped.
    QMimeData outside;
    outside.setText(QStringLiteral("NEW "));
    QVERIFY(drop(&outside, 4, Qt::NoModifier));
    QCOMPARE(plain(doc), QStringLiteral("two NEW One three four two five"));
    QCOMPARE(doc.nextStepSerial(), next + 2);
}


void WriteTests::aClickInsideTheSelectionPutsTheCaretThereAndWithTheDragOffAPressStartsANewSelection()
{
    auto w = styWindow("<w:p><w:r><w:t>One two three four five</w:t></w:r></w:p>");
    PageView *view = w->view();
    const QPoint inside = styPointOf(view, 6), from = styPointOf(view, 8), to = styPointOf(view, 14);
    // (Where a click puts the caret is the layout's to say: a click with nothing selected is the measure of a click inside the selection.)
    selectRange(*w, 0, 0);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, inside);
    const int expected = view->cursor().position();
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, to);   // (far from the first: not a double click with it)
    selectRange(*w, 0, 13);
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, inside);
    QVERIFY(!view->cursor().hasSelection());
    QCOMPARE(view->cursor().position(), expected);
    // The press that begins a drag of the text is held until the pointer moves; with the drag off (as the Copy Formatting has it) the press begins a new selection.
    view->setTextDragAllowed(false);
    selectRange(*w, 0, 13);
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, from);
    QTest::mouseMove(view->viewport(), to);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QVERIFY(view->cursor().hasSelection());
    QVERIFY(view->cursor().selectionStart() < view->cursor().selectionEnd());
    QVERIFY(view->cursor().selectionStart() > 0 && view->cursor().selectionEnd() < 13 + 5);   // (a new selection from the press to the release, not the old one)
    QVERIFY(!w->document()->canUndo());   // (nothing was moved)
}


void WriteTests::theStylesPaneOfADocumentWithTenThousandStylesListsAtMostWhatItHolds()
{
    QByteArray styles;
    styManyStyles(10000, &styles);
    auto w = styWindow("<w:p><w:pPr><w:pStyle w:val=\"S7\"/></w:pPr><w:r><w:t>Text</w:t></w:r></w:p>", styles);
    w->act(QStringLiteral("style.pane"))->trigger();
    StylesPane *pane = w->stylesPane();
    QVERIFY(pane);
    pane->setFilter(write::StyleFilter::All);
    QVERIFY(pane->list()->count() > 0 && pane->list()->count() <= write::kMostStylesListed);
    pane->setFilter(write::StyleFilter::InUse);
    QCOMPARE(pane->list()->count(), 2);   // (Normal and the style of the paragraph)
    QCOMPARE(pane->currentKey(), QStringLiteral("p:S7"));
    // Typing, and moving the caret to a paragraph of another style, make the list again no more than they cost a pass over the text: none (the mark moves).
    const QString first = pane->keyAt(0);
    const int rebuilds = pane->rebuildCountForTest();
    const qint64 scans = w->document()->inUseScansForTest();
    QTextCursor c(w->document()->story());
    c.setPosition(4);
    for (int i = 0; i < 12; ++i) {
        insertText(*w->document(), c, QStringLiteral("m"));
        w->refreshNow();
    }
    QCOMPARE(pane->keyAt(0), first);
    QCOMPARE(pane->list()->count(), 2);
    QCOMPARE(pane->rebuildCountForTest(), rebuilds);
    QCOMPARE(w->document()->inUseScansForTest(), scans);   // (a pass over the text for what is in use waits for a moment after the last edit)
    // The caret into the paragraph of Normal: the mark moves, and the list stays.
    c.setPosition(w->document()->story()->characterCount() - 1);
    insertParagraph(*w->document(), c);
    applyParagraphStyle(*w->document(), c, QStringLiteral("Normal"));
    w->view()->setCursor(c);
    w->refreshNow();
    QCOMPARE(pane->currentKey(), QStringLiteral("p:Normal"));
    const int afterStyle = pane->rebuildCountForTest();
    selectRange(*w, 1, 1);
    w->refreshNow();
    QCOMPARE(pane->currentKey(), QStringLiteral("p:S7"));
    selectRange(*w, w->document()->story()->characterCount() - 1, w->document()->story()->characterCount() - 1);
    w->refreshNow();
    QCOMPARE(pane->currentKey(), QStringLiteral("p:Normal"));
    QCOMPARE(pane->rebuildCountForTest(), afterStyle);
    QVERIFY(pane->list()->item(pane->rowOf(QStringLiteral("p:Normal")))->text().startsWith(QStringLiteral("\u25B8")));
    QVERIFY(!pane->list()->item(pane->rowOf(QStringLiteral("p:S7")))->text().startsWith(QStringLiteral("\u25B8")));
    // After a moment the styles in use are looked at once more.
    QTRY_VERIFY(w->document()->inUseScansForTest() > scans);
}


void WriteTests::aCancelledStyleDialogOnABuiltInStyleTheDocumentLacksChangesNothing()
{
    const QByteArray zip = makeDocx("<w:p><w:r><w:t>Words</w:t></w:r></w:p>", kStyles);
    const QByteArray original = styleXml(zip);
    auto w = std::make_unique<WriteWindow>();
    w->resize(1000, 700);
    w->setDocument(open(zip), QString());
    w->show();
    w->refreshNow();
    w->act(QStringLiteral("style.pane"))->trigger();
    StylesPane *pane = w->stylesPane();
    pane->setFilter(write::StyleFilter::All);
    const int row = pane->rowOf(QStringLiteral("p:Title"));
    QVERIFY(row >= 0);
    pane->list()->setCurrentRow(row);
    // Cancel, and OK with nothing changed: the style is not added, the file is not marked, and styles.xml is the same bytes.
    for (bool accept : {false, true}) {
        withDialog([&] { w->act(QStringLiteral("style.modify"))->trigger(); }, QStringLiteral("styleDialog"), [](QDialog *d) { QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("styleName"))->text(), QStringLiteral("Title")); }, accept);
        QVERIFY(!w->document()->styleSheet().find(QStringLiteral("Title")));
        QVERIFY(!w->isModified());
        QVERIFY(!w->document()->canUndo());
        QCOMPARE(styleXml(w->document()->toDocx()), original);
    }
    // A change adds it, and one Undo takes it away to the same bytes.
    pane->list()->setCurrentRow(pane->rowOf(QStringLiteral("p:Title")));
    withDialog([&] { w->act(QStringLiteral("style.modify"))->trigger(); }, QStringLiteral("styleDialog"), [](QDialog *d) { d->findChild<QCheckBox *>(QStringLiteral("styleItalic"))->setChecked(true); });
    QVERIFY(w->document()->styleSheet().find(QStringLiteral("Title")));
    w->refreshNow();
    w->act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(!w->document()->styleSheet().find(QStringLiteral("Title")));
    QCOMPARE(styleXml(w->document()->toDocx()), original);
    QVERIFY(!w->isModified());
}


// Not a test of its own: the work of a process that the next function starts. A document with a header and a footer, read, typed in, and saved.
void WriteTests::savesTheDocumentWithHeaderAndFooter()
{
    QString out;
    if (!stable::childOutput(&out)) return;
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
    const QByteArray header = headerPart("hdr", {"<w:r><w:t>Top of the page</w:t></w:r>"});
    const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"});
    auto doc = open(makeDocxWithParts(paragraphs(30), sect, {{QStringLiteral("/word/header1.xml"), header}, {QStringLiteral("/word/footer1.xml"), footer}}));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    insertText(*doc, c, QStringLiteral("typed words"));
    const QByteArray saved = doc->toDocx();
    QVERIFY(!saved.isEmpty());
    QFile f(out);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QCOMPARE(f.write(saved), saved.size());
}


// The file is the same bytes each time it is saved, in whatever process: nothing written follows the order of a hash that Qt seeds anew for each process.
void WriteTests::aSavedDocumentIsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesTheDocumentWithHeaderAndFooter"); }
