#include "tst_write.h"


// What is pasted into a cell is in that cell, whatever cell it was copied from, and what is pasted outside a table is in none.
void WriteTests::pastedParagraphsAreInTheCellTheyWerePastedInto()
{
    auto doc = open(tableDocx(1, 2));
    QTextDocument *s = doc->story();
    auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
    QTextCursor c(s);
    c.setPosition(block(QStringLiteral("r0c0")).position() + 4);
    insertParagraph(*doc, c);
    typeText(*doc, c, QStringLiteral("second"));
    c.setPosition(block(QStringLiteral("r0c0")).position());
    c.setPosition(block(QStringLiteral("second")).position() + 6, QTextCursor::KeepAnchor);
    const QTextDocumentFragment two = selectionFragment(c);   // (two paragraphs of the first cell)
    c.setPosition(block(QStringLiteral("r0c1")).position() + 4);
    pasteFragment(*doc, c, two);
    const int other = cellKeyOf(block(QStringLiteral("r0c1r0c0")));
    QVERIFY(other != 0 && other != cellKeyOf(block(QStringLiteral("r0c0"))));
    int inCell = 0;
    for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) inCell += cellKeyOf(b) == other;
    QCOMPARE(inCell, 2);
    // Into the text before the table: no cell.
    c.setPosition(block(QStringLiteral("before")).position() + 6);
    pasteFragment(*doc, c, two);
    QCOMPARE(block(QStringLiteral("beforer0c0")).isValid(), true);
    QCOMPARE(cellKeyOf(block(QStringLiteral("beforer0c0"))), 0);
    int outside = 0;
    for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) outside += b.text() == QLatin1String("second") && cellKeyOf(b) == 0;
    QCOMPARE(outside, 1);
    checkWhole(*doc);
}


void WriteTests::aSavedDocumentRoundTripsItsOwnParagraphFormatting()
{
    // A document with direct formatting of every kind comes out with the same properties on every paragraph and run.
    const QByteArray body =
        "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:jc w:val=\"center\"/><w:ind w:left=\"360\" w:firstLine=\"180\"/><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"480\" w:lineRule=\"exact\"/>"
        "<w:tabs><w:tab w:val=\"right\" w:leader=\"dot\" w:pos=\"7200\"/></w:tabs><w:keepLines/><w:pageBreakBefore/></w:pPr>"
        "<w:r><w:rPr><w:rFonts w:ascii=\"Georgia\" w:hAnsi=\"Georgia\"/><w:i/><w:caps/><w:strike/><w:color w:val=\"00AA00\"/><w:sz w:val=\"30\"/><w:vertAlign w:val=\"superscript\"/><w:spacing w:val=\"20\"/></w:rPr><w:t>Heading with tab</w:t><w:tab/><w:t>3</w:t></w:r></w:p>"
        "<w:p><w:r><w:rPr><w:smallCaps/><w:u w:val=\"double\"/><w:highlight w:val=\"cyan\"/><w:lang w:val=\"fr-FR\"/></w:rPr><w:t>Second</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    const QByteArray once = documentXml(doc->toDocx());
    wml::Document a, b;
    QVERIFY(wml::readDocx(makeDocx(body, kStyles), &a));
    QVERIFY(wml::readDocx(doc->toDocx(), &b));
    QCOMPARE(a.body.size(), b.body.size());
    for (int i = 0; i < a.body.size(); ++i) {
        const wml::Paragraph &pa = a.body[i].para, &pb = b.body[i].para;
        QCOMPARE(wml::paraPropsXml(pb.props), wml::paraPropsXml(pa.props));
        QCOMPARE(pb.content.size(), pa.content.size());
        for (int k = 0; k < pa.content.size(); ++k) {
            QCOMPARE(wml::runPropsXml(pb.content[k].props), wml::runPropsXml(pa.content[k].props));
            QCOMPARE(pb.content[k].text, pa.content[k].text);
        }
    }
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), once);
}


// Kerning is the file's to ask for: a run kerns from the size (in half-points) that its own properties or its style's give as w:kern, and a run that says
// none or says zero does not. The setting is written back as it was read.
void WriteTests::kerningComesFromTheFilesKernSetting()
{
    const QByteArray styles = kStyles + "<w:style w:type=\"character\" w:styleId=\"Kerned\"><w:name w:val=\"Kerned\"/><w:rPr><w:kern w:val=\"18\"/></w:rPr></w:style>";
    const QByteArray body = "<w:p><w:r><w:rPr><w:kern w:val=\"20\"/></w:rPr><w:t>direct</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"Kerned\"/></w:rPr><w:t>styled</w:t></w:r>"
                            "<w:r><w:rPr><w:kern w:val=\"0\"/></w:rPr><w:t>zero</w:t></w:r><w:r><w:t>plain</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, styles));
    QVERIFY(doc);
    auto kernAt = [&](int pos) {
        QTextCursor c(doc->story());
        c.setPosition(pos + 1);
        const QTextCharFormat f = c.charFormat();
        return f.hasProperty(tp::KernAbove) ? f.property(tp::KernAbove).toDouble() : -1.0;
    };
    QCOMPARE(kernAt(0), 10.0);    // "direct": from 10 points
    QCOMPARE(kernAt(6), 9.0);     // "styled": from the style's 9
    QCOMPARE(kernAt(12), -1.0);   // "zero"
    QCOMPARE(kernAt(16), -1.0);   // "plain"
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:kern w:val=\"20\"/>"), x.constData());
    QCOMPARE(int(x.count("<w:kern ")), 1);   // (the styled run's is its style's, and the zero one is the default)
}


// A w:kern that is negative, enormous, or not a number is read without harm: below zero and a word that is no number kern nothing, one that is far past
// any size kerns from there (so never), and the file is written back whatever they said. (This reads as it did: the test is for what is kept.)
void WriteTests::hostileKernValuesAreReadWithoutHarm()
{
    const QByteArray body = "<w:p><w:r><w:rPr><w:kern w:val=\"-5\"/></w:rPr><w:t>negative</w:t></w:r><w:r><w:rPr><w:kern w:val=\"2147483647\"/></w:rPr><w:t>enormous</w:t></w:r>"
                            "<w:r><w:rPr><w:kern w:val=\"abc\"/></w:rPr><w:t>word</w:t></w:r><w:r><w:rPr><w:kern w:val=\"99999999999999\"/></w:rPr><w:t>overflow</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    auto kernAt = [&](int pos) {
        QTextCursor c(doc->story());
        c.setPosition(pos + 1);
        const QTextCharFormat f = c.charFormat();
        return f.hasProperty(tp::KernAbove) ? f.property(tp::KernAbove).toDouble() : -1.0;
    };
    QCOMPARE(kernAt(0), -1.0);                   // "negative"
    QVERIFY2(kernAt(8) < 0 || kernAt(8) > 1000, qPrintable(QString::number(kernAt(8))));   // "enormous": from a size no text has (the file's numbers are clamped as all are)
    QCOMPARE(kernAt(16), -1.0);                  // "word"
    QVERIFY2(kernAt(20) < 0 || kernAt(20) > 1000, qPrintable(QString::number(kernAt(20))));   // "overflow": clamped, and from a size no text has
    QVERIFY(!doc->toDocx().isEmpty());
}


void WriteTests::whatAParagraphSaysThatHasNoFieldIsKept()
{
    // Shading, a suppressed line number, and attributes of spacing and indent that this program has no field for come back as they were.
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:suppressLineNumbers/><w:shd w:val=\"clear\" w:fill=\"C0C0C0\"/><w:spacing w:before=\"100\" w:beforeAutospacing=\"1\"/>"
                            "<w:ind w:left=\"360\" w:leftChars=\"150\"/><w:rPr><w:b/><w:shd w:val=\"clear\" w:fill=\"FFFF00\"/></w:rPr></w:pPr>"
                            "<w:r><w:rPr><w:shd w:val=\"clear\" w:fill=\"00FFFF\"/><w:bdr w:val=\"single\"/></w:rPr><w:t>shaded</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:suppressLineNumbers/><w:shd w:val=\"clear\" w:fill=\"C0C0C0\"/>"), x.constData());
    QVERIFY2(x.contains("w:beforeAutospacing=\"1\""), x.constData());
    QVERIFY2(x.contains("w:leftChars=\"150\""), x.constData());
    QVERIFY2(x.contains("<w:shd w:val=\"clear\" w:fill=\"FFFF00\"/>"), x.constData());
    QVERIFY2(x.contains("<w:bdr w:val=\"single\"/><w:shd w:val=\"clear\" w:fill=\"00FFFF\"/>"), x.constData());   // (in the schema's order)
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
    // Applying another style leaves them.
    QTextCursor c(doc->story());
    applyParagraphStyle(*doc, c, QStringLiteral("Normal"));
    const QByteArray y = documentXml(doc->toDocx());
    QVERIFY(y.contains("w:fill=\"C0C0C0\"") && y.contains("w:leftChars=\"150\""));
}


void WriteTests::tabStopsTheEngineHasNoKindForAreKeptAndNotMade()
{
    // A list's own tab (num) is the definition's, and a bar tab is the paragraph's: neither is turned into a left tab, and the bar stays.
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"5\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                 "<w:pPr><w:tabs><w:tab w:val=\"num\" w:pos=\"1080\"/></w:tabs><w:ind w:left=\"1080\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"5\"><w:abstractNumId w:val=\"5\"/></w:num>";
    auto doc = open(makeDocx("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"5\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:tabs><w:tab w:val=\"bar\" w:pos=\"2000\"/><w:tab w:val=\"right\" w:pos=\"9000\"/></w:tabs></w:pPr><w:r><w:t>bar</w:t></w:r></w:p>", kStyles, numbering));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(!x.contains("w:val=\"left\""), x.constData());
    QVERIFY2(x.contains("<w:tabs><w:tab w:val=\"bar\" w:pos=\"2000\"/><w:tab w:val=\"right\" w:pos=\"9000\"/></w:tabs>"), x.constData());
    QCOMPARE(int(x.count("<w:tabs>")), 1);
}


void WriteTests::aKindOfJustifiedIsKeptUntilTheAlignmentIsChosen()
{
    auto doc = open(makeDocx("<w:p><w:pPr><w:jc w:val=\"distribute\"/></w:pPr><w:r><w:t>spread</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:jc w:val=\"both\"/></w:pPr><w:r><w:t>plain</w:t></w:r></w:p>", kStyles));
    QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignJustify);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:jc w:val=\"distribute\"/>") && x.contains("<w:jc w:val=\"both\"/>"), x.constData());
    // Choosing an alignment, even the justified one, makes it the plain kind.
    QTextCursor c(doc->story());
    setAlignment(*doc, c, Qt::AlignJustify);
    const QByteArray y = documentXml(doc->toDocx());
    QVERIFY2(!y.contains("distribute") && int(y.count("<w:jc w:val=\"both\"/>")) == 2, y.constData());
}


void WriteTests::aParagraphMarkThatSaysItsSizeKeepsSayingItUnderACharacterStyle()
{
    // An empty paragraph's mark has a character style that is small, and says its size is the paragraph's own: the size stays, or the
    // empty line gets as short as the style.
    const QByteArray styles = kStyles + "<w:style w:type=\"character\" w:styleId=\"Small\"><w:name w:val=\"Small\"/><w:rPr><w:sz w:val=\"16\"/></w:rPr></w:style>";
    auto doc = open(makeDocx("<w:p><w:pPr><w:rPr><w:rStyle w:val=\"Small\"/><w:sz w:val=\"23\"/></w:rPr></w:pPr></w:p><w:p><w:r><w:t>text</w:t></w:r></w:p>", styles));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:rPr><w:rStyle w:val=\"Small\"/><w:sz w:val=\"23\"/></w:rPr>"), x.constData());
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
    // A mark that says only the style is left as that, and is as small as the style on the page.
    auto plain = open(makeDocx("<w:p><w:pPr><w:rPr><w:rStyle w:val=\"Small\"/></w:rPr></w:pPr></w:p><w:p><w:r><w:t>text</w:t></w:r></w:p>", styles));
    QVERIFY2(documentXml(plain->toDocx()).contains("<w:rPr><w:rStyle w:val=\"Small\"/></w:rPr></w:pPr>"), documentXml(plain->toDocx()).constData());
    QCOMPARE(plain->story()->findBlockByNumber(0).charFormat().fontPointSize(), 8.0);
}


void WriteTests::aStyleThatNumbersKeepsItsOwnIndentsOverTheList()
{
    // The style says it is numbered and says its indents; the list's level says other indents. The style's are the paragraph's. A
    // paragraph that says "hanging by nothing" takes the style's first-line indent away, and the file says so after the save.
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"5\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"none\"/><w:lvlText w:val=\"\"/>"
                                 "<w:pPr><w:ind w:left=\"0\" w:hanging=\"0\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"5\"><w:abstractNumId w:val=\"5\"/></w:num>"
                                 "<w:abstractNum w:abstractNumId=\"6\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                 "<w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"6\"><w:abstractNumId w:val=\"6\"/></w:num>";
    const QByteArray styles = kStyles
        + "<w:style w:type=\"paragraph\" w:styleId=\"Sub\"><w:name w:val=\"Sub\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"5\"/></w:numPr>"
          "<w:ind w:firstLine=\"708\" w:left=\"4956\"/></w:pPr></w:style>"
          "<w:style w:type=\"paragraph\" w:styleId=\"Bare\"><w:name w:val=\"Bare\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"6\"/></w:numPr></w:pPr></w:style>";
    auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Sub\"/></w:pPr><w:r><w:t>plain</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:pStyle w:val=\"Sub\"/><w:ind w:hanging=\"0\" w:left=\"5245\"/></w:pPr><w:r><w:t>flush</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:pStyle w:val=\"Bare\"/></w:pPr><w:r><w:t>bare</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:pStyle w:val=\"Bare\"/><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"6\"/></w:numPr></w:pPr><w:r><w:t>said</w:t></w:r></w:p>", styles, numbering));
    QTextBlockFormat f0 = doc->story()->findBlockByNumber(0).blockFormat(), f1 = doc->story()->findBlockByNumber(1).blockFormat();
    QCOMPARE(f0.leftMargin(), 4956 / 20.0);
    QCOMPARE(f0.textIndent(), 708 / 20.0);
    QCOMPARE(f1.leftMargin(), 5245 / 20.0);
    QCOMPARE(f1.textIndent(), 0.0);
    QCOMPARE(doc->story()->findBlockByNumber(2).blockFormat().leftMargin(), 36.0);   // (the list's, for the style says none)
    QCOMPARE(doc->story()->findBlockByNumber(2).blockFormat().textIndent(), -18.0);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:ind w:left=\"5245\" w:firstLine=\"0\"/>") || x.contains("<w:ind w:firstLine=\"0\" w:left=\"5245\"/>"), x.constData());
    QCOMPARE(int(x.count("<w:ind")), 1);                    // (nothing else is said about indents)
    QCOMPARE(int(x.count("<w:numPr>")), 1);                 // (only the one the file said again)
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
    auto back = open(doc->toDocx());
    QCOMPARE(back->story()->findBlockByNumber(1).blockFormat().textIndent(), 0.0);
    QCOMPARE(back->story()->findBlockByNumber(0).blockFormat().textIndent(), 708 / 20.0);
}


void WriteTests::theFontsOfOtherScriptsAreKeptAsTheRunSaidThem()
{
    // The engine's formats hold the Latin font; what a run says of its East Asian and complex-script fonts, and of its hint, goes with it.
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Cjk\"><w:name w:val=\"Cjk\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:rFonts w:eastAsia=\"Noto Sans CJK\"/></w:rPr></w:style>";
    auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\" w:eastAsia=\"Arial\" w:cs=\"Arial\" w:hint=\"eastAsia\"/></w:rPr><w:t>all four</w:t></w:r></w:p>"
                             "<w:p><w:r><w:rPr><w:rFonts w:eastAsia=\"Arial\"/></w:rPr><w:t>only one</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:pStyle w:val=\"Cjk\"/></w:pPr><w:r><w:rPr><w:rFonts w:eastAsia=\"Noto Sans CJK\"/></w:rPr><w:t>as the style</w:t></w:r>"
                             "<w:r><w:rPr><w:rFonts w:eastAsia=\"Other\"/></w:rPr><w:t>not as the style</w:t></w:r></w:p>", styles));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:rFonts w:hint=\"eastAsia\" w:ascii=\"Arial\" w:hAnsi=\"Arial\" w:eastAsia=\"Arial\" w:cs=\"Arial\"/>"), x.constData());
    QVERIFY2(x.contains("<w:rFonts w:eastAsia=\"Arial\"/>"), x.constData());
    QVERIFY2(x.contains("<w:rFonts w:eastAsia=\"Other\"/>"), x.constData());
    QCOMPARE(int(x.count("Noto Sans CJK")), 0);   // (said as the style says: the style's, not the run's)
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
}


void WriteTests::aSectionThatStartsPageNumbersAtZeroKeepsSayingIt()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>"
                            "<w:pgNumType w:start=\"0\" w:fmt=\"decimal\"/><w:titlePg/></w:sectPr>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>cover</w:t></w:r></w:p>", kStyles, QByteArray(), sect));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:pgNumType w:fmt=\"decimal\" w:start=\"0\"/>"), x.constData());
    // And a file that says nothing of it says nothing.
    auto plain = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", kStyles));
    QVERIFY(!documentXml(plain->toDocx()).contains("pgNumType"));
}


void WriteTests::aParagraphThatSaysNoFirstLineIndentKeepsSayingIt()
{
    // The style indents the first line; the paragraph says "hanging" by nothing, which takes that indent away.
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Indented\"><w:name w:val=\"Indented\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:ind w:firstLine=\"720\" w:left=\"0\" w:right=\"0\"/></w:pPr></w:style>";
    auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Indented\"/><w:ind w:hanging=\"0\" w:right=\"0\"/></w:pPr><w:r><w:t>flush</w:t></w:r></w:p>"
                             "<w:p><w:pPr><w:pStyle w:val=\"Indented\"/></w:pPr><w:r><w:t>indented</w:t></w:r></w:p>", styles));
    QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().textIndent(), 0.0);
    QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().textIndent(), 36.0);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:ind w:firstLine=\"0\"/>"), x.constData());
    QCOMPARE(int(x.count("<w:ind")), 1);
    // And it reads back the same: no first-line indent on the first, one on the second.
    auto back = open(doc->toDocx());
    QCOMPARE(back->story()->findBlockByNumber(0).blockFormat().textIndent(), 0.0);
    QCOMPARE(back->story()->findBlockByNumber(1).blockFormat().textIndent(), 36.0);
}


void WriteTests::hostileDocumentsAreHeldToLimits()
{
    QString err;
    // Not a package, not a document, not XML.
    QVERIFY(!WriteDoc::fromDocx(QByteArray("not a zip at all"), &err));
    QVERIFY(!err.isEmpty());
    QByteArray deep;
    for (int i = 0; i < 300; ++i) deep += "<w:p>";
    for (int i = 0; i < 300; ++i) deep += "</w:p>";
    QStringList deepNotes;
    QVERIFY2(WriteDoc::fromDocx(makeDocx(deep), &err, &deepNotes), qPrintable(err));   // (nested past the limit: opened, what is below it made plain, and said)
    QVERIFY(!deepNotes.isEmpty());
    // Sizes and counts that no page could have are clamped, and the document opens.
    const QByteArray odd = "<w:p><w:pPr><w:ind w:left=\"99999999999\" w:hanging=\"88888888\"/><w:spacing w:before=\"2000000000\" w:line=\"-5\"/><w:numPr><w:ilvl w:val=\"500\"/><w:numId w:val=\"77\"/></w:numPr></w:pPr>"
                           "<w:r><w:rPr><w:sz w:val=\"9999999\"/></w:rPr><w:t>x</w:t></w:r></w:p>";
    auto doc = open(makeDocx(odd, kStyles, QByteArray(), "<w:sectPr><w:pgSz w:w=\"1\" w:h=\"2000000000\"/><w:pgMar w:top=\"-900000000\" w:left=\"900000000\"/><w:cols w:num=\"100000\"/></w:sectPr>"));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 1);
    QVERIFY(doc->pager().page(0).size.width() >= 36 && doc->pager().page(0).size.height() <= 3168);
    QVERIFY(doc->flow().section(0).columnCount() <= 45);
    // A big document: 30,000 paragraphs open in time in proportion to their number (ten times the paragraphs, about ten times as long), lay out
    // the first page at once, and save.
    auto openWork = [](int count) {
        const QByteArray zip = makeDocx(paragraphs(count, 25), kStyles);
        return openAndPageWork(zip);
    };
    const OpenWork few = openWork(3000), many = openWork(30000);
    QVERIFY2(many.atMost(few, 12), qPrintable(QStringLiteral("3,000 paragraphs: %1; 30,000 paragraphs: %2").arg(few.text(), many.text())));
    auto big = open(makeDocx(paragraphs(30000, 25), kStyles));
    QVERIFY(big->pager().materialize(0));
    QVERIFY(big->pager().pagesLaidOut() <= 2);
    QVERIFY(!big->toDocx().isEmpty());
    QCOMPARE(big->story()->blockCount(), 30000);
}


// What the file keeps beside a paragraph's text (its w14:paraId, a bookmark) is unique to each paragraph in a modern file, and the formats must
// not be compared with every other one each time one is made: opening takes about as long with them as without. (Compared with each other,
// not against a clock: the same machine, the same load.)
void WriteTests::openingDoesNotSlowDownWithUniqueIdsAndBookmarks()
{
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
    auto build = [&](int kind) {
        QByteArray body;
        for (int i = 0; i < 8000; ++i) {
            const QByteArray n = QByteArray::number(i);
            if (kind == 1) body += "<w:p w14:paraId=\"" + QByteArray::number(0x10000000 + i, 16).toUpper() + "\" w14:textId=\"77777777\">";
            else body += "<w:p>";
            if (kind == 2) body += "<w:bookmarkStart w:id=\"" + n + "\" w:name=\"b" + n + "\"/>";
            body += "<w:r><w:t>paragraph " + n + "</w:t></w:r>";
            if (kind == 2) body += "<w:bookmarkEnd w:id=\"" + n + "\"/>";
            body += "</w:p>";
        }
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
        return w.finish();
    };
    auto openWork = [&](const QByteArray &zip) { return openAndPageWork(zip); };
    const QByteArray plain = build(0), withIds = build(1), withBookmarks = build(2);
    const OpenWork base = openWork(plain), ids = openWork(withIds), marks = openWork(withBookmarks);
    QVERIFY2(ids.atMost(base, 4), qPrintable(QStringLiteral("with ids: %1; without: %2").arg(ids.text(), base.text())));
    QVERIFY2(marks.atMost(base, 4), qPrintable(QStringLiteral("with bookmarks: %1; without: %2").arg(marks.text(), base.text())));
}


// The root of the main part declares its prefixes on lines of their own; a piece kept as XML uses one the writer would add if it were missing.
void WriteTests::aRootWithDeclarationsOnSeparateLinesGetsNoneTwice()
{
    const QByteArray doc = "<w:document\r\n xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"\r\n\txmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"\n"
                           "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"><w:body><w:p><w:sdt><w:sdtPr><w14:checkbox><w14:checked w14:val=\"0\"/></w14:checkbox></w:sdtPr>"
                           "<w:sdtContent><w:r><w:t>box</w:t></w:r></w:sdtContent></w:sdt></w:p></w:body></w:document>";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml(doc), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"), QStringLiteral("/word/document.xml"));
    const QByteArray original = w.finish();
    auto d = open(original);
    QVERIFY(d);
    const QByteArray saved = d->toDocx();
    const QStringList strict = rt::strictProblems(saved, original);
    QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
    QVERIFY(documentXml(saved).contains("w14:checkbox"));
}


// A prefix declared on the paragraph itself (not on the root) goes back with the attributes that use it.
void WriteTests::aNamespaceDeclaredOnAParagraphStaysDeclared()
{
    auto d = open(makeDocx("<w:p xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" w14:paraId=\"1234ABCD\"><w:r><w:t>local</w:t></w:r></w:p>"));
    QVERIFY(d);
    const QByteArray saved = d->toDocx();
    const QStringList strict = rt::strictProblems(saved);
    QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
    QVERIFY(documentXml(saved).contains("1234ABCD"));
}


// The styles and the lists have links of their own (a picture bullet's image): they are kept, and the image is not left without one.
void WriteTests::theLinksOfTheStylesAndTheListsAreKept()
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    office::PackageWriter w;
    const QByteArray ns = kNs;
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body><w:p><w:r><w:t>text</w:t></w:r></w:p></w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), xml("<w:styles " + ns + ">" + kStyles + "</w:styles>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addPart(QStringLiteral("/word/numbering.xml"), xml("<w:numbering " + ns + "><w:numPicBullet w:numPicBulletId=\"0\"><w:pict><v:imagedata r:id=\"rIdPic\"/></w:pict></w:numPicBullet></w:numbering>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
    w.addPart(QStringLiteral("/word/media/bullet.png"), QByteArray("\x89PNG-bytes"), QStringLiteral("image/png"));
    w.addPart(QStringLiteral("/word/media/swatch.png"), QByteArray("\x89PNG-other"), QStringLiteral("image/png"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdSt"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdNum"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/numbering.xml"), QStringLiteral("rIdPic"), rel + QStringLiteral("image"), QStringLiteral("/word/media/bullet.png"));
    w.addRelationshipWithId(QStringLiteral("/word/styles.xml"), QStringLiteral("rIdSw"), rel + QStringLiteral("image"), QStringLiteral("/word/media/swatch.png"));
    const QByteArray original = w.finish();
    auto d = open(original);
    QVERIFY(d);
    const QByteArray saved = d->toDocx();
    office::Package p;
    QVERIFY(p.read(saved));
    QCOMPARE(p.relationship(QStringLiteral("/word/numbering.xml"), QStringLiteral("rIdPic")).target, QStringLiteral("/word/media/bullet.png"));
    QCOMPARE(p.relationship(QStringLiteral("/word/styles.xml"), QStringLiteral("rIdSw")).target, QStringLiteral("/word/media/swatch.png"));
    QVERIFY(p.hasPart(QStringLiteral("/word/media/bullet.png")) && p.hasPart(QStringLiteral("/word/media/swatch.png")));
}


// A bookmark or a comment range may begin in a paragraph and end inside a table that is kept as XML (or the other way about), and two
// bookmarks may carry the same Id: all of them stay, each start with its end, and the second of an Id is given an Id of its own.
void WriteTests::bookmarksAndRangesAcrossKeptTablesAndOfRepeatedIdsStay()
{
    const QByteArray table1 = "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r><w:bookmarkEnd w:id=\"1\"/><w:commentRangeEnd w:id=\"7\"/></w:p></w:tc></w:tr></w:tbl>";
    const QByteArray table2 = "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:bookmarkStart w:id=\"2\" w:name=\"fromTable\"/><w:commentRangeStart w:id=\"8\"/><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    const QByteArray body = "<w:p><w:bookmarkStart w:id=\"1\" w:name=\"intoTable\"/><w:commentRangeStart w:id=\"7\"/><w:r><w:t>before</w:t></w:r></w:p>" + table1 +
                            "<w:p><w:r><w:t>between</w:t></w:r></w:p>" + table2 +
                            "<w:p><w:r><w:t>after</w:t></w:r><w:bookmarkEnd w:id=\"2\"/><w:commentRangeEnd w:id=\"8\"/></w:p>"
                            "<w:p><w:bookmarkStart w:id=\"3\" w:name=\"one\"/><w:r><w:t>x</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p>"
                            "<w:p><w:bookmarkStart w:id=\"3\" w:name=\"two\"/><w:r><w:t>y</w:t></w:r><w:bookmarkEnd w:id=\"3\"/></w:p>";
    auto d = open(makeDocx(body));
    QVERIFY(d);
    const QByteArray saved = d->toDocx();
    const QStringList strict = rt::strictProblems(saved);
    QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
    const QByteArray x = documentXml(saved);
    QCOMPARE(int(x.count("<w:bookmarkStart")), 4);
    QCOMPARE(int(x.count("<w:bookmarkEnd")), 4);
    QCOMPARE(int(x.count("<w:commentRangeStart")), 2);
    QCOMPARE(int(x.count("<w:commentRangeEnd")), 2);
    // Every bookmark start has an end of its Id, once; the names are all there.
    QRegularExpression starts(QStringLiteral("<w:bookmarkStart w:id=\"(\\d+)\" w:name=\"(\\w+)\"")), ends(QStringLiteral("<w:bookmarkEnd w:id=\"(\\d+)\""));
    QMap<QString, QString> name;
    for (auto it = starts.globalMatch(QString::fromUtf8(x)); it.hasNext();) {
        const auto m = it.next();
        QVERIFY2(!name.contains(m.captured(1)), "an Id is used by two bookmarks");
        name[m.captured(1)] = m.captured(2);
    }
    QMap<QString, int> endCount;
    for (auto it = ends.globalMatch(QString::fromUtf8(x)); it.hasNext();) ++endCount[it.next().captured(1)];
    QStringList names = name.values();
    names.sort();
    QCOMPARE(names, (QStringList{"fromTable", "intoTable", "one", "two"}));
    for (auto it = name.cbegin(); it != name.cend(); ++it) QVERIFY2(endCount.value(it.key()) == 1, qPrintable(QStringLiteral("bookmark %1 (Id %2) has %3 ends").arg(it.value(), it.key()).arg(endCount.value(it.key()))));
    const QStringList loss = rt::lossProblems(makeDocx(body), saved);
    for (const QString &l : loss) QVERIFY2(!l.startsWith(QLatin1String("element counts")) || (!l.contains(QLatin1String("bookmark")) && !l.contains(QLatin1String("commentRange"))), qPrintable(l));
}


// A table nobody touched is written as the file had it, whatever the file had in it that the program would write otherwise: a bookmark that starts and has no end, and a
// second of one name, a style that is the default said again, an indent of nothing, a toggle spelled "false", and a row that doesn't reach the end of the grid. Once
// something in it changes it is written from the story, and then the bookmark with no end is left out.
void WriteTests::aTableNobodyTouchedComesBackAsTheFileHadIt()
{
    const QByteArray table =
        "<w:tbl><w:tblPr><w:tblW w:w=\"2000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"14\"/></w:tblGrid>"
        "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:pPr><w:pStyle w:val=\"Normal\"/><w:ind w:left=\"0\" w:hanging=\"0\"/></w:pPr>"
        "<w:bookmarkStart w:id=\"0\" w:name=\"Check1\"/><w:bookmarkStart w:id=\"1\" w:name=\"Check1\"/><w:bookmarkEnd w:id=\"1\"/>"
        "<w:r><w:rPr><w:b w:val=\"false\"/></w:rPr><w:t>one</w:t></w:r></w:p></w:tc>"
        "<w:tc><w:tcPr><w:tcW w:w=\"1000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>two</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    const QByteArray original = makeDocx("<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>after</w:t></w:r></w:p>", kStyles);
    auto doc = open(original);
    QVERIFY(doc);
    const QByteArray saved = doc->toDocx();
    QVERIFY2(documentXml(saved).contains(table), documentXml(saved).constData());
    const QStringList strict = rt::strictProblems(saved, original), loss = rt::lossProblems(original, saved);
    QVERIFY2(strict.isEmpty() && loss.isEmpty(), qPrintable((strict + loss).join(QLatin1String("; "))));
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->toPlainText().indexOf(QLatin1String("one")));
    insertText(*doc, c, QStringLiteral("x"));
    const QByteArray changed = documentXml(doc->toDocx());
    QVERIFY2(!changed.contains(table) && changed.contains("xone"), changed.constData());
    QCOMPARE(int(changed.count("<w:bookmarkStart")), 1);
    QCOMPARE(int(changed.count("<w:bookmarkEnd")), 1);
    // A table written from the story is one a reader needn't repair: its rows reach the end of the grid (the file's row stops short of the last column, which the
    // row then says it leaves empty), whatever the rows said before.
    const QStringList shape = rt::strictProblems(doc->toDocx(), original);
    QVERIFY2(shape.isEmpty(), qPrintable(shape.join(QLatin1String("; "))));
    QVERIFY2(changed.contains("<w:trPr><w:gridAfter w:val=\"1\"/></w:trPr>"), changed.constData());
}


// Enter inside a hyperlink, a content control, a tracked change, or a tag keeps the wrapper on both halves, with its attributes (and so its
// link): one of each kind becomes two, and the second revision mark gets an Id of its own.
void WriteTests::enterInsideAWrapperKeepsItOnBothHalves()
{
    struct Case { const char *tag, *inText; int offset; };
    const Case cases[] = {{"w:hyperlink", "link text in the middle", 9}, {"w:sdt", "content control text", 7}, {"w:ins", "inserted words", 4}, {"w:smartTag", "Paris", 2}};
    for (const Case &k : cases) {
        auto doc = open(richDocx());
        QVERIFY(doc);
        const QMap<QString, int> before = rt::keptCountsOf(doc->toDocx());
        QTextCursor c(doc->story());
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
            const int at = b.text().indexOf(QString::fromLatin1(k.inText));
            if (at >= 0) {
                c.setPosition(b.position() + at + k.offset);
                break;
            }
        }
        insertParagraph(*doc, c);
        const QByteArray saved = doc->toDocx();
        const QStringList strict = rt::strictProblems(saved);
        QVERIFY2(strict.isEmpty(), qPrintable(QStringLiteral("%1: %2").arg(QString::fromLatin1(k.tag), strict.join(QLatin1Char('\n')))));
        const QMap<QString, int> now = rt::keptCountsOf(saved);
        QVERIFY2(now.value(QString::fromLatin1(k.tag)) == before.value(QString::fromLatin1(k.tag)) + 1, qPrintable(QStringLiteral("%1: %2 before, %3 now").arg(QString::fromLatin1(k.tag)).arg(before.value(QString::fromLatin1(k.tag))).arg(now.value(QString::fromLatin1(k.tag)))));
        const QByteArray x = documentXml(saved);
        if (QByteArray(k.tag) == "w:hyperlink") QCOMPARE(int(x.count("r:id=\"rIdLink\"")), 2);
        if (QByteArray(k.tag) == "w:ins") {
            QSet<QByteArray> ids;
            for (qsizetype at = x.indexOf("<w:ins "); at >= 0; at = x.indexOf("<w:ins ", at + 1)) ids.insert(x.mid(at, x.indexOf('>', at) - at).split('"')[1]);
            for (qsizetype at = x.indexOf("<w:del "); at >= 0; at = x.indexOf("<w:del ", at + 1)) ids.insert(x.mid(at, x.indexOf('>', at) - at).split('"')[1]);
            QCOMPARE(int(ids.size()), 3);   // (the file's two and the new one)
        }
    }
}


// Enter at the end of a paragraph leaves the paragraph mark's own with the second half: a section that ends in the paragraph, and the XML
// kept after it. The first keeps what comes before.
void WriteTests::enterAtTheEndOfAParagraphMovesTheMarksPropertiesToTheNewOne()
{
    auto doc = open(richDocx());
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 2);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(0).position() + doc->story()->findBlockByNumber(0).length() - 1);   // the end of the heading that ends the first section
    insertParagraph(*doc, c);
    QCOMPARE(doc->sectionCount(), 2);
    QVERIFY(!doc->story()->findBlockByNumber(0).blockFormat().boolProperty(flow::fp::SectionBreakAfter));
    QVERIFY(doc->story()->findBlockByNumber(1).blockFormat().boolProperty(flow::fp::SectionBreakAfter));
    QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
    QCOMPARE(doc->story()->findBlockByNumber(1).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Normal"));   // (what follows a heading)
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:sectPr")), 2);
    auto back = open(doc->toDocx());
    QCOMPARE(back->sectionCount(), 2);
    // The XML kept after the last paragraph: a block content control's end.
    auto d = open(makeDocx("<w:sdt><w:sdtPr><w:alias w:val=\"B\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>in the control</w:t></w:r></w:p></w:sdtContent></w:sdt>"));
    QTextCursor e(d->story());
    e.movePosition(QTextCursor::End);
    insertParagraph(*d, e);
    const QByteArray x = documentXml(d->toDocx());
    QVERIFY2(rt::strictProblems(d->toDocx()).isEmpty(), x.constData());
    QCOMPARE(int(x.count("<w:sdt>")), 1);
    QVERIFY2(x.indexOf("<w:sdt>") < x.indexOf("in the control") && x.indexOf("in the control") < x.indexOf("</w:p><w:p") && x.indexOf("</w:p><w:p") < x.indexOf("</w:sdtContent>"), x.constData());
}


// Backspace at the start of a paragraph that has XML kept before it (a block content control's end) joins it to the one before, and the
// control stays.
void WriteTests::backspaceIntoAParagraphWithXmlBeforeItKeepsTheXml()
{
    for (int how = 0; how < 2; ++how) {
        auto doc = open(richDocx());
        QVERIFY(doc);
        const QMap<QString, int> before = rt::keptCountsOf(doc->toDocx());
        QTextCursor c(doc->story());
        QTextBlock after = doc->story()->begin();
        for (; after.isValid(); after = after.next())
            if (after.text().startsWith(QLatin1String("After the block control"))) break;
        QVERIFY(after.isValid());
        const int blocks = doc->story()->blockCount();
        if (how == 0) {
            c.setPosition(after.position());
            deleteBack(*doc, c, false);
        } else {
            c.setPosition(after.previous().position() + after.previous().length() - 1);
            deleteForward(*doc, c, false);
        }
        QCOMPARE(doc->story()->blockCount(), blocks - 1);
        const QByteArray saved = doc->toDocx();
        const QStringList strict = rt::strictProblems(saved);
        QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
        const QMap<QString, int> now = rt::keptCountsOf(saved);
        QCOMPARE(now.value(QStringLiteral("w:sdt")), before.value(QStringLiteral("w:sdt")));
        QVERIFY(documentXml(saved).contains("Inside a block controlAfter the block control"));
    }
}


// Pasting before a heading that ends a section leaves the heading its style and its section break, whether what is pasted is words or
// paragraphs; the section break is not moved to what was pasted.
void WriteTests::pastingBeforeAHeadingThatEndsASectionLeavesItAlone()
{
    for (int paragraphs = 1; paragraphs <= 2; ++paragraphs) {
        auto doc = open(richDocx());
        QVERIFY(doc);
        QTextDocument *story = doc->story();
        QTextCursor sel(story);
        const QTextBlock src = story->findBlockByNumber(1);
        sel.setPosition(src.position() + 2);
        sel.setPosition(paragraphs == 1 ? src.position() + 12 : story->findBlockByNumber(2).position() + 6, QTextCursor::KeepAnchor);
        const QTextDocumentFragment held = selectionFragment(sel);
        QTextCursor at(story);
        at.setPosition(0);
        pasteFragment(*doc, at, held);
        QCOMPARE(doc->sectionCount(), 2);
        // The block that holds the heading's own words is the last of what was pasted into, and it is the heading.
        QTextBlock heading = story->begin();
        while (heading.isValid() && !heading.text().endsWith(QLatin1String("Heading that ends a section"))) heading = heading.next();
        QVERIFY(heading.isValid());
        QCOMPARE(heading.blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
        QVERIFY(heading.blockFormat().boolProperty(flow::fp::SectionBreakAfter));
        int breaks = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) breaks += b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
        QCOMPARE(breaks, 1);
        QCOMPARE(int(documentXml(doc->toDocx()).count("<w:sectPr")), 2);
    }
}


// A copy made in one document and pasted into another carries its pictures (the bytes, in a part of the new file, with a relationship of its
// own) and its hyperlinks (a relationship); a piece that refers to a part the new file can't be given (a chart) is not carried, and its
// text is pasted as text. Nothing saved points at a part that is not there.
void WriteTests::pastingFromAnotherDocumentCarriesPicturesAndLinksAndNothingDangles()
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                          "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"";
    const QByteArray png("\x89PNG\r\n\x1a\n-picture-bytes-");
    const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">A picture: </w:t></w:r><w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/><wp:docPr id=\"1\" name=\"Pic\"/><a:graphic>"
                            "<a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"0\" name=\"p.png\"/><pic:cNvPicPr/></pic:nvPicPr>"
                            "<pic:blipFill><a:blip r:embed=\"rIdImg\"/></pic:blipFill><pic:spPr/></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r>"
                            "<w:r><w:t xml:space=\"preserve\"> a </w:t></w:r><w:hyperlink r:id=\"rIdLink\"><w:r><w:t>link</w:t></w:r></w:hyperlink><w:r><w:t xml:space=\"preserve\"> a chart: </w:t></w:r>"
                            "<w:r><w:drawing><wp:inline><wp:extent cx=\"100\" cy=\"100\"/><wp:docPr id=\"2\" name=\"Chart\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                            "<c:chart r:id=\"rIdChart\"/></a:graphicData></a:graphic></wp:inline></w:drawing></w:r><w:r><w:t> end.</w:t></w:r></w:p>";
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + ns + "><w:body>" + body + "</w:body></w:document>"), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addPart(QStringLiteral("/word/media/image1.png"), png, QStringLiteral("image/png"));
    w.addPart(QStringLiteral("/word/charts/chart1.xml"), xml("<c:chartSpace " + ns + "/>"), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdImg"), rel + QStringLiteral("image"), QStringLiteral("/word/media/image1.png"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdLink"), rel + QStringLiteral("hyperlink"), QStringLiteral("https://example.com/target"), true);
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdChart"), rel + QStringLiteral("chart"), QStringLiteral("/word/charts/chart1.xml"));
    Clip clip;
    {
        auto source = open(w.finish());
        QVERIFY(source);
        QTextCursor sel(source->story());
        sel.select(QTextCursor::Document);
        clip = copySelection(*source, sel);
        QCOMPARE(int(clip.links.size()), 3);
        // (The same document: nothing is made over.)
        auto same = open(w.finish());
        QTextCursor at(same->story());
        at.movePosition(QTextCursor::End);
        auto target = open(makeDocx("<w:p><w:r><w:t>Target.</w:t></w:r></w:p>", kStyles));
        QVERIFY(target);
        QTextCursor t(target->story());
        t.movePosition(QTextCursor::End);
        pasteClip(*target, t, clip);
        const QByteArray saved = target->toDocx();
        const QStringList strict = rt::strictProblems(saved);
        QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
        QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
        office::Package p;
        QVERIFY(p.read(saved));
        const QByteArray x = p.part(p.mainPart());
        QVERIFY(x.contains("A picture: ") && x.contains("link") && x.contains(" end."));
        // The picture: a part of the new file with the bytes, a link to it of the new file's own.
        QStringList images;
        for (const QString &n : p.partNames())
            if (n.startsWith(QLatin1String("/word/media/"))) images << n;
        QCOMPARE(images.size(), 1);
        QCOMPARE(p.part(images[0]), png);
        QCOMPARE(int(x.count("r:embed=")), 1);
        // The hyperlink: its address, in a relationship of the new file.
        const QVector<office::Relationship> links = p.relationshipsOfType(p.mainPart(), QStringLiteral("/hyperlink"));
        QCOMPARE(links.size(), 1);
        QCOMPARE(links[0].target, QStringLiteral("https://example.com/target"));
        QVERIFY(links[0].external);
        // The chart: not carried; the file has no chart part and no reference to one.
        QVERIFY(!x.contains("c:chart"));
        for (const QString &n : p.partNames()) QVERIFY(!n.contains(QLatin1String("chart")));
    }
    // The last document is closed: the kept bytes a copy's formats stood for are gone, and a paste of it is the text.
    auto later = open(makeDocx("<w:p><w:r><w:t>Later.</w:t></w:r></w:p>", kStyles));
    QTextCursor lc(later->story());
    lc.movePosition(QTextCursor::End);
    pasteClip(*later, lc, clip);
    QVERIFY(later->story()->toPlainText().contains(QStringLiteral("link")));
    const QByteArray lsaved = later->toDocx();
    QVERIFY(rt::strictProblems(lsaved).isEmpty() && rt::danglingReferences(lsaved).isEmpty());
}


// A copy that took the start of a tag (a content control) and not its end, pasted inside another tag (a hyperlink), leaves the other tag as
// it was: the stray start is the one that is left out.
void WriteTests::aStrayStartOfATagPastedInsideAnotherLeavesTheOthersWhole()
{
    auto doc = open(richDocx());
    QVERIFY(doc);
    const QMap<QString, int> before = rt::keptCountsOf(doc->toDocx());
    QTextDocument *story = doc->story();
    const QTextBlock control = story->findBlockByNumber(3), link = story->findBlockByNumber(1);
    QTextCursor sel(story);
    sel.setPosition(control.position() + 2);                       // inside "A control: "
    sel.setPosition(control.position() + control.text().indexOf(QChar::ObjectReplacementCharacter) + 4, QTextCursor::KeepAnchor);   // past the start of the control and a little text
    const QTextDocumentFragment held = selectionFragment(sel);
    QTextCursor at(story);
    at.setPosition(link.position() + link.text().indexOf(QStringLiteral("link text")) + 4);   // inside the hyperlink's text
    pasteFragment(*doc, at, held);
    const QByteArray saved = doc->toDocx();
    QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
    const QMap<QString, int> now = rt::keptCountsOf(saved);
    QCOMPARE(now.value(QStringLiteral("w:hyperlink")), before.value(QStringLiteral("w:hyperlink")));
    QCOMPARE(now.value(QStringLiteral("w:sdt")), before.value(QStringLiteral("w:sdt")));
}


// A file with macros is saved without them, all of them (the project, its signature, the data it keeps, and the links), and the person is told
// once, in a sentence, at the save.
void WriteTests::savingADocumentWithMacrosLeavesThemOutAndSaysSo()
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + QByteArray(kNs) + "><w:body><w:p><w:r><w:t>with macros</w:t></w:r></w:p></w:body></w:document>"),
              QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"));
    w.addPart(QStringLiteral("/word/vbaProject.bin"), QByteArray("VBA-project"), QStringLiteral("application/vnd.ms-office.vbaProject"));
    w.addPart(QStringLiteral("/word/vbaData.xml"), xml("<wne:vbaSuppData xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\"/>"), QStringLiteral("application/vnd.ms-word.vbaData+xml"));
    w.addPart(QStringLiteral("/word/customizations.xml"), xml("<x/>"), QStringLiteral("application/vnd.ms-word.keyMapCustomizations+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdVba"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), QStringLiteral("/word/vbaProject.bin"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdKeys"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/keyMapCustomizations"), QStringLiteral("/word/customizations.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/vbaProject.bin"), QStringLiteral("rIdData"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/wordVbaData"), QStringLiteral("/word/vbaData.xml"));
    const QByteArray original = w.finish();
    auto doc = open(original);
    QVERIFY(doc);
    QVERIFY(doc->hadMacros());
    const QByteArray saved = doc->toDocx();
    office::Package p;
    QVERIFY(p.read(saved));
    QVERIFY(!p.hasPart(QStringLiteral("/word/vbaProject.bin")) && !p.hasPart(QStringLiteral("/word/vbaData.xml")));
    QVERIFY(p.hasPart(QStringLiteral("/word/customizations.xml")));   // (not a macro: kept)
    for (const office::Relationship &r : p.relationships(p.mainPart())) QVERIFY2(!r.type.contains(QLatin1String("vba")), qPrintable(r.type));
    QCOMPARE(p.contentType(p.mainPart()), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    for (const QString &n : p.partNames())
        for (const office::Relationship &r : p.relationships(n)) QVERIFY2(r.external || p.hasPart(r.target), qPrintable(n + QStringLiteral(" -> ") + r.target));
    // Saved from the window: one sentence, once.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile f(dir.filePath(QStringLiteral("macros.docm")));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(original);
    f.close();
    WriteWindow win;
    QStringList said;
    win.setNotifier([&](const QString &, const QString &text) { said << text; });
    QVERIFY(win.openFile(f.fileName()));
    QVERIFY(win.document()->hadMacros());
    QVERIFY(said.isEmpty());
    QVERIFY(win.saveTo(dir.filePath(QStringLiteral("plain.docx"))));
    QCOMPARE(said.size(), 1);
    QVERIFY2(said[0].contains(QLatin1String("macros")) && said[0].contains(QLatin1String(".docx")) && said[0].count(QLatin1Char('.')) <= 2, qPrintable(said[0]));
    QVERIFY(win.saveTo(dir.filePath(QStringLiteral("again.docx"))));
    QCOMPARE(said.size(), 1);
}


// A file that could not be read whole (here, a part named so that it can't be one) says so on opening, and is not written over: Save asks
// for a new name, with a sentence on why, and the original stays as it was.
void WriteTests::aFileThatCouldNotBeReadWholeIsNotSavedOverItsOriginal()
{
    QMap<QString, QByteArray> items;
    QVERIFY(jp::readZip(makeDocx("<w:p><w:r><w:t>whole enough</w:t></w:r></w:p>", kStyles), items));
    jp::ZipWriter z;
    for (auto it = items.cbegin(); it != items.cend(); ++it) z.add(it.key(), it.value(), true);
    z.add(QStringLiteral("word/../../evil.xml"), "x", true);
    const QByteArray original = z.finish();
    QStringList notes;
    QString err;
    auto doc = WriteDoc::fromDocx(original, &err, &notes);
    QVERIFY2(doc, qPrintable(err));
    QVERIFY(doc->leftSomethingOut());
    QVERIFY2(!notes.isEmpty() && notes.join(QLatin1Char(' ')).contains(QLatin1String("left out")), qPrintable(notes.join(QLatin1Char(' '))));
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("original.docx"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(original);
    f.close();
    WriteWindow win;
    QStringList said;
    win.setNotifier([&](const QString &, const QString &text) { said << text; });
    const QString copy = dir.filePath(QStringLiteral("copy.docx"));
    QStringList asked;
    win.setSavePathChooser([&](const QString &suggested, const QString &) { asked << suggested; return copy; });
    QVERIFY(win.openFile(path));
    QCOMPARE(said.size(), 1);   // (what was left out)
    win.view()->insertText(QStringLiteral("edited "));
    QVERIFY(win.save());
    QCOMPARE(asked.size(), 1);
    QCOMPARE(said.size(), 2);
    QVERIFY2(said[1].contains(QLatin1String("new name")), qPrintable(said[1]));
    QVERIFY(QFileInfo::exists(copy));
    QFile again(path);
    QVERIFY(again.open(QIODevice::ReadOnly));
    QCOMPARE(again.readAll(), original);   // (the original is as it was)
    // Save As onto the original is refused, too.
    QVERIFY(!win.saveTo(path));
    QCOMPARE(said.size(), 3);
    QFile once(path);
    QVERIFY(once.open(QIODevice::ReadOnly));
    QCOMPARE(once.readAll(), original);
    // And a document whole is saved where it was, as before.
    const QString whole = dir.filePath(QStringLiteral("whole.docx"));
    QFile wf(whole);
    QVERIFY(wf.open(QIODevice::WriteOnly));
    wf.write(makeDocx("<w:p><w:r><w:t>fine</w:t></w:r></w:p>", kStyles));
    wf.close();
    WriteWindow other;
    QStringList otherSaid;
    other.setNotifier([&](const QString &, const QString &text) { otherSaid << text; });
    QVERIFY(other.openFile(whole));
    other.view()->insertText(QStringLiteral("x"));
    QVERIFY(other.save());
    QVERIFY(otherSaid.isEmpty());
}


void WriteTests::aFileLeftPartsOutIsNotSavedOverItsOriginalByAnotherName()
{
    QMap<QString, QByteArray> items;
    QVERIFY(jp::readZip(makeDocx("<w:p><w:r><w:t>whole enough</w:t></w:r></w:p>", kStyles), items));
    jp::ZipWriter z;
    for (auto it = items.cbegin(); it != items.cend(); ++it) z.add(it.key(), it.value(), true);
    z.add(QStringLiteral("word/../../evil.xml"), "x", true);
    const QByteArray original = z.finish();
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("original.docx"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(original);
    f.close();
    WriteWindow win;
    QStringList said;
    win.setNotifier([&](const QString &, const QString &text) { said << text; });
    QVERIFY(win.openFile(path));
    QVERIFY(win.document()->leftSomethingOut());
    QCOMPARE(said.size(), 1);   // (what was left out)
    const auto unchanged = [&] {
        QFile again(path);
        return again.open(QIODevice::ReadOnly) && again.readAll() == original;
    };
    // A link to the file is the file.
    const QString alias = dir.filePath(QStringLiteral("alias.docx"));
    if (QFile::link(path, alias)) {   // (not every system lets a test make one)
        QVERIFY(!win.saveTo(alias));
        QCOMPARE(said.size(), 2);
        QVERIFY(unchanged());
    }
    // And a name that differs in capitals only is the file, where the system keeps files that way.
    const QString capitals = dir.filePath(QStringLiteral("ORIGINAL.docx"));
    if (QFileInfo::exists(capitals)) {
        const int before = said.size();
        QVERIFY(!win.saveTo(capitals));
        QCOMPARE(said.size(), before + 1);
        QVERIFY(unchanged());
    }
}

// A part with a % (or a space, or a letter outside ASCII) in its name is legal in a package, and stays: with its bytes, its content type, and
// the link that names it.
void WriteTests::partsWithPercentSignsAndSpacesInTheirNamesAreKept()
{
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    office::PackageWriter w;
    w.addPart(QStringLiteral("/word/document.xml"), xml("<w:document " + QByteArray(kNs) + "><w:body><w:p><w:r><w:t>pictures</w:t></w:r></w:p></w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    const QStringList names = {QStringLiteral("/word/media/a%b.png"), QStringLiteral("/word/media/my image.png"), QStringLiteral("/word/media/\u00fc.png"), QStringLiteral("/word/media/50%25.png")};
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    for (int i = 0; i < names.size(); ++i) {
        w.addPart(names[i], QByteArray("png-") + QByteArray::number(i), QStringLiteral("image/png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdP%1").arg(i), rel + QStringLiteral("image"), names[i]);
    }
    const QByteArray original = w.finish();
    office::Package check;
    QVERIFY(check.read(original));
    for (int i = 0; i < names.size(); ++i) QCOMPARE(check.part(names[i]), QByteArray("png-") + QByteArray::number(i));
    auto doc = open(original);
    QVERIFY(doc);
    const QByteArray saved = doc->toDocx();
    office::Package p;
    QVERIFY(p.read(saved));
    for (int i = 0; i < names.size(); ++i) {
        QVERIFY2(p.hasPart(names[i]), qPrintable(names[i]));
        QCOMPARE(p.part(names[i]), QByteArray("png-") + QByteArray::number(i));
        QCOMPARE(p.contentType(names[i]), QStringLiteral("image/png"));
        QCOMPARE(p.relationship(p.mainPart(), QStringLiteral("rIdP%1").arg(i)).target, names[i]);
    }
    const QStringList loss = rt::lossProblems(original, saved);
    for (const QString &l : loss) QVERIFY2(!l.startsWith(QLatin1String("part")) && !l.startsWith(QLatin1String("relationships")) && !l.startsWith(QLatin1String("content type")), qPrintable(l));
    // The names in the ZIP are the escaped ones.
    QMap<QString, QByteArray> items;
    QVERIFY(jp::readZip(saved, items));
    QVERIFY(items.contains(QStringLiteral("word/media/a%25b.png")) && items.contains(QStringLiteral("word/media/my%20image.png")));
}


// Tables nested deeper than the limit (as in a real file that tests such readers) don't refuse the file: the structure down to the limit is kept,
// and the text below it comes as plain paragraphs, with a note. What is saved opens again, plainly.
void WriteTests::tablesNestedPastTheLimitOpenWithTheTextBelowAsPlainParagraphs()
{
    const int levels = 700;
    QByteArray body;
    for (int i = 0; i < levels; ++i) body += "<w:tbl><w:tblGrid><w:gridCol w:w=\"2400\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>Nested level " + QByteArray::number(i) + "</w:t></w:r></w:p>";
    for (int i = 0; i < levels; ++i) body += "</w:tc></w:tr></w:tbl><w:p><w:r><w:t>after " + QByteArray::number(i) + "</w:t></w:r></w:p>";
    const QByteArray original = makeDocx(body);
    QString err;
    QStringList notes;
    auto doc = WriteDoc::fromDocx(original, &err, &notes);
    QVERIFY2(doc, qPrintable(err));
    QVERIFY2(!notes.isEmpty() && notes.join(QLatin1Char(' ')).contains(QLatin1String("deeper")), qPrintable(notes.join(QLatin1Char(' '))));
    QVERIFY(doc->leftSomethingOut());
    // (The table is a piece kept as XML, shown as a stand-in; what is in it is in the file when it is saved.)
    int locked = 0;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) locked += isLocked(b);
    QCOMPARE(locked, 1);
    const QByteArray saved = doc->toDocx();
    QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
    const QByteArray x = documentXml(saved);
    for (int i : {0, 1, 199, 200, levels - 1}) {
        QVERIFY2(x.contains("Nested level " + QByteArray::number(i) + "<"), qPrintable(QString::number(i)));
        QVERIFY2(x.contains("after " + QByteArray::number(i) + "<"), qPrintable(QString::number(i)));
    }
    QStringList again;
    auto back = WriteDoc::fromDocx(saved, &err, &again);
    QVERIFY2(back, qPrintable(err));
    // (The table below the limit is the stand-in still, and the file's reader says so; nothing is made plain this time.)
    QVERIFY2(again.size() == 1 && again[0].contains(QLatin1String("stand-in")), qPrintable(again.join(QLatin1Char(' '))));
    QCOMPARE(documentXml(back->toDocx()), x);
}


void WriteTests::aRichDocumentComesBackFaithfulAndWellFormed()
{
    const QByteArray original = richDocx();
    auto doc = open(original);
    QVERIFY(doc);
    QString err;
    const QByteArray saved = doc->toDocx(&err);
    QVERIFY2(!saved.isEmpty(), qPrintable(err));
    const QStringList strict = rt::strictProblems(saved, original), loss = rt::lossProblems(original, saved);
    QVERIFY2(strict.isEmpty(), qPrintable(strict.join(QLatin1Char('\n'))));
    QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1Char('\n'))));
    auto back = open(saved);
    QVERIFY(back);
    const QStringList story = rt::storyProblems(*doc, *back);
    QVERIFY2(story.isEmpty(), qPrintable(story.join(QLatin1Char('\n'))));
}


// Random editing of a document that holds everything a file keeps: after each few commands the file it makes is well-formed, opens again,
// writes what it wrote, has a section for each break, and (when the commands only add) has lost nothing that the file had.
// JW_FUZZ_SEEDS says how many seeds; JW_FUZZ_FROM where to begin; JW_FUZZ_COUNTS=1 prints how many seeds failed each check.
void WriteTests::randomEditingOfARichDocumentKeepsWhatItHolds()
{
    const QByteArray original = richDocx();
    const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 8;
    QMap<QString, int> failed;
    QMap<QString, QString> example;
    QSet<QString> seedsFailing;
    for (int mode = 0; mode < 3; ++mode) {   // 0: commands that only add; 1: those that take away too; 2: those, and then every one undone
        auto base = open(original);
        QVERIFY(base);
        const QMap<QString, int> start = rt::keptCountsOf(base->toDocx());
        for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
            auto doc = open(original);
            const QByteArray baseline = documentXml(doc->toDocx());
            std::mt19937 rng(seed * 2 + mode);
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
            bool seedFailed = false;   // (the file can't be made or read: no more steps)
            QSet<QString> thisSeed;
            auto fail = [&](const QString &check, const QString &detail, bool fatal = false) {
                const QString key = QStringLiteral("mode %1: %2").arg(mode).arg(check);
                if (!thisSeed.contains(key)) {
                    thisSeed.insert(key);
                    failed[key] += 1;
                    if (!example.contains(key)) example[key] = QStringLiteral("seed %1: %2\nlast steps: %3").arg(seed).arg(detail, log.mid(std::max<qsizetype>(0, log.size() - 4)).join(QStringLiteral(" | ")));
                    if (qEnvironmentVariableIsSet("JW_FUZZ_DUMP")) {
                        QString dump;
                        for (QTextBlock bk = doc->story()->begin(); bk.isValid(); bk = bk.next()) {
                            QString line;
                            for (QTextBlock::iterator it = bk.begin(); !it.atEnd(); ++it) {
                                const QTextFragment fr = it.fragment();
                                if (fr.charFormat().hasProperty(prop::Atom)) {
                                    const wml::Inline a = unpackAtom(storedBytes(fr.charFormat(), prop::Atom));
                                    const QString mark = (a.role == wml::Inline::Open ? QStringLiteral("<") : a.role == wml::Inline::Close ? QStringLiteral("</") : QStringLiteral("[")) + a.tag + (a.role == wml::Inline::Single ? QStringLiteral("]") : QStringLiteral(">"));
                                    for (QChar ch : fr.text()) line += ch == QChar::ObjectReplacementCharacter ? mark : QString(ch);
                                } else {
                                    line += fr.text();
                                }
                            }
                            dump += QStringLiteral("  [%1] %2%3%4\n").arg(bk.blockNumber()).arg(line.left(200), bk.blockFormat().boolProperty(flow::fp::SectionBreakAfter) ? QStringLiteral(" SECT") : QString(), isLocked(bk) ? QStringLiteral(" LOCKED") : QString());
                        }
                        qInfo("seed %d mode %d %s: %s\n%s", int(seed), mode, qPrintable(check), qPrintable(detail), qPrintable(dump));
                    }
                }
                seedFailed = seedFailed || fatal;
                seedsFailing.insert(QStringLiteral("%1/%2").arg(mode).arg(seed));
            };
            for (int step = 0; step < 120 && !seedFailed; ++step) {
                int op = pick(mode == 0 ? 15 : 23);
                if (mode == 0 && op >= 12) op += 5;   // (the commands that add to the tables: a row, a column, Tab)
                if (mode == 2 && (op == 10 || op == 11)) continue;   // (no undo or redo among the commands: they are all undone at the end)
                log << QStringLiteral("op%1@%2").arg(op).arg(c.position());
                switch (op) {
                case 0: place(); typeText(*doc, c, QStringLiteral("word ")); break;
                case 1: place(); insertParagraph(*doc, c); break;
                case 2: place(); insertPageBreak(*doc, c); break;
                case 3: place(); applyParagraphStyle(*doc, c, pick(2) ? QStringLiteral("Heading1") : QStringLiteral("Normal")); break;
                case 4: selectSome(); toggleBold(*doc, c); break;
                case 5: selectSome(); toggleList(*doc, c, pick(2)); break;
                case 6: selectSome(); held = selectionFragment(c); break;
                case 7: case 8: place(); pasteFragment(*doc, c, held); break;
                case 9: if (pick(3) == 0) { place(); insertSectionBreak(*doc, c); } break;
                case 10: doc->story()->undo(&c); break;
                case 11: doc->story()->redo(&c); break;
                case 12: place(); deleteBack(*doc, c, false); break;
                case 13: place(); deleteForward(*doc, c, false); break;
                case 14: selectSome(); removeSelection(*doc, c); break;
                case 15: place(); insertParagraph(*doc, c); deleteBack(*doc, c, false); break;
                case 17: case 18: case 19: case 20: case 21: case 22: tableCommand(*doc, c, op - 17, pick); break;
                default: selectSome(); c.removeSelectedText(); break;
                }
                if (qEnvironmentVariableIsSet("JW_FUZZ_TRACE")) {
                    QString line;
                    for (QTextBlock bk = doc->story()->begin(); bk.isValid() && bk.blockNumber() < 3; bk = bk.next()) {
                        QString t;
                        for (QTextBlock::iterator it = bk.begin(); !it.atEnd(); ++it) {
                            const QTextFragment fr = it.fragment();
                            if (fr.charFormat().hasProperty(prop::Atom)) {
                                const wml::Inline a = unpackAtom(storedBytes(fr.charFormat(), prop::Atom));
                                for (QChar ch : fr.text()) t += ch == QChar::ObjectReplacementCharacter ? QStringLiteral("{%1%2}").arg(a.role == wml::Inline::Close ? QStringLiteral("/") : QString(), a.tag.left(3)) : QString(ch);
                            } else {
                                t += fr.text();
                            }
                        }
                        line += QStringLiteral(" [%1]").arg(t.left(110));
                    }
                    qInfo("step %d op%d@%d: %s", step, op, c.position(), qPrintable(line));
                }
                if (mode == 2) continue;
                if (step % 6 != 5) continue;
                QString err;
                const QByteArray zip = doc->toDocx(&err);
                if (zip.isEmpty()) { fail(QStringLiteral("not written"), err, true); break; }
                const QStringList strict = rt::strictProblems(zip, original);
                if (!strict.isEmpty()) { fail(QStringLiteral("not well-formed"), strict.first().left(200), true); break; }
                auto back = WriteDoc::fromDocx(zip, &err);
                if (!back) { fail(QStringLiteral("does not open again"), err, true); break; }
                int breaks = 0;
                for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) breaks += b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                if (doc->sectionCount() != breaks + 1 || back->sectionCount() != doc->sectionCount()) fail(QStringLiteral("sections"), QStringLiteral("%1 sections, %2 breaks, %3 after reopening").arg(doc->sectionCount()).arg(breaks).arg(back->sectionCount()));
                const QByteArray again = documentXml(back->toDocx());
                if (again != documentXml(zip)) {
                    const QByteArray one = documentXml(zip);
                    int at = 0;
                    while (at < one.size() && at < again.size() && one[at] == again[at]) ++at;
                    fail(QStringLiteral("drifts"), QStringLiteral("the second save differs from the first at %1: %2 | %3").arg(at).arg(QString::fromUtf8(one.mid(std::max(0, at - 150), 330)), QString::fromUtf8(again.mid(std::max(0, at - 150), 330))));
                }
                if (mode == 0) {
                    const QMap<QString, int> now = rt::keptCountsOf(zip);
                    for (auto it = start.cbegin(); it != start.cend(); ++it)
                        if (now.value(it.key()) < it.value()) fail(QStringLiteral("lost ") + it.key(), QStringLiteral("%1 %2->%3").arg(it.key()).arg(it.value()).arg(now.value(it.key())));
                }
            }
            if (mode == 2) {
                // Every command undone: the file it makes is what the document made before them.
                int undone = 0;
                while (doc->story()->isUndoAvailable() && undone < 100000) {
                    doc->story()->undo();
                    ++undone;
                }
                const QByteArray now = documentXml(doc->toDocx());
                if (now != baseline) {
                    int at = 0;
                    while (at < now.size() && at < baseline.size() && now[at] == baseline[at]) ++at;
                    fail(QStringLiteral("undo leaves a change"), QStringLiteral("after %1 undos, the file differs from the one before the commands at %2: %3 | %4").arg(undone).arg(at).arg(QString::fromUtf8(baseline.mid(std::max(0, at - 100), 260)), QString::fromUtf8(now.mid(std::max(0, at - 100), 260))));
                }
            }
        }
    }
    QStringList report;
    for (auto it = failed.cbegin(); it != failed.cend(); ++it) report << QStringLiteral("%1: %2 seeds\n  %3").arg(it.key()).arg(it.value()).arg(example.value(it.key()));
    if (qEnvironmentVariableIsSet("JW_FUZZ_COUNTS")) qInfo("fuzz: %d of %d runs failed:\n%s", int(seedsFailing.size()), 3 * seeds, qPrintable(report.join(QLatin1Char('\n'))));
    QVERIFY2(failed.isEmpty(), qPrintable(QStringLiteral("%1 of %2 runs failed:\n%3").arg(seedsFailing.size()).arg(3 * seeds).arg(report.join(QLatin1Char('\n')))));
}


// ---- hostile tables

OpenWork WriteTests::openAndPageWork(const QByteArray &zip, int *pages, QStringList *notes, std::unique_ptr<WriteDoc> *keep)
{
    QString err;
    const qint64 tree = office::wml::treeStepsForTest(), chars = StoryLayout::paragraphCharsReadForTest(), runs = StoryLayout::runsPassedForTest(), ranges = StoryLayout::formatRangesMadeForTest();
    auto doc = WriteDoc::fromDocx(zip, &err, notes);
    if (!doc) {
        qWarning("open: %s", qPrintable(err));
        return OpenWork{-1};
    }
    const int n = doc->pager().pageCount();
    if (pages) *pages = n;
    OpenWork w;
    w.tree = office::wml::treeStepsForTest() - tree;
    w.chars = StoryLayout::paragraphCharsReadForTest() - chars;
    w.runs = StoryLayout::runsPassedForTest() - runs;
    w.ranges = StoryLayout::formatRangesMadeForTest() - ranges;
    w.pages = doc->pager().pagesLaidOut();
    w.rows = doc->pager().tableRowsMeasured();
    w.cells = doc->pager().tableCellsLaidOut();
    if (keep) *keep = std::move(doc);
    return w;
}


// A table of `rows` by `columns` short cells.
QByteArray WriteTests::manyCells(int rows, int columns)
{
    const int width = std::max(300, 9000 / columns);
    QByteArray rowsXml;
    for (int r = 0; r < rows; ++r) {
        QByteArray cells;
        for (int c = 0; c < columns; ++c) cells += cellX(cellP("r" + QByteArray::number(r) + "c" + QByteArray::number(c)), QByteArray(), width);
        rowsXml += rowX(cells);
    }
    return tableX(rowsXml, columns, width);
}


// A table that has more cells than a document lays out (300 rows of 256 columns) is kept as it was, with a note that says so, and costs next to nothing to open; and a
// document that has as many as it may opens in seconds, and in time that grows with its cells.
void WriteTests::hostileTablesOpenInTimeThatGrowsWithTheirCells()
{
    {   // more cells than the document lays out: a stand-in, with the note, saved as it was
        const QByteArray table = manyCells(300, 256);
        const QByteArray zip = makeDocx(cellP("before") + table + cellP("after"), kStyles);
        QStringList notes;
        int pages = 0;
        const OpenWork work = openAndPageWork(zip, &pages, &notes);
        // Counted: the table is a stand-in, and none of its rows or cells is measured or laid out.
        QVERIFY2(work.rows == 0 && work.cells == 0, qPrintable(work.text()));
        QVERIFY2(work.tree <= 150000 && work.chars <= 100 && work.ranges <= 20, qPrintable(work.text()));   // (next to nothing: the XML is passed over once)
        QVERIFY2(notes.size() == 1 && notes[0].contains(QLatin1String("table")), qPrintable(notes.join(QLatin1Char('|'))));
        QCOMPARE(pages, 1);
        auto doc = open(zip);
        QVERIFY(doc);
        QVERIFY(documentXml(doc->toDocx()).contains(table));
    }
    {   // 20,000 rows: real, in pages, in time that is not much more than for a tenth of them
        int small = 0, big = 0;
        const OpenWork workSmall = openAndPageWork(makeDocx(cellP("before") + manyCells(2000, 2) + cellP("after"), kStyles), &small);
        const OpenWork workBig = openAndPageWork(makeDocx(cellP("before") + manyCells(20000, 2) + cellP("after"), kStyles), &big);
        QVERIFY2(small > 30 && small < 80, qPrintable(QString::number(small)));
        QVERIFY2(big > 10 * small - 10 && big < 10 * small + 20, qPrintable(QStringLiteral("%1 pages for 2,000 rows, %2 for 20,000").arg(small).arg(big)));
        QVERIFY2(workBig.atMost(workSmall, 12), qPrintable(QStringLiteral("2,000 rows: %1; 20,000 rows: %2").arg(workSmall.text(), workBig.text())));
    }
    {   // as many cells as a document lays out
        int pages = 0;
        QStringList notes;
        const OpenWork work = openAndPageWork(makeDocx(cellP("before") + manyCells(5000, 10) + cellP("after"), kStyles), &pages, &notes);
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char('|'))));
        QVERIFY2(pages > 150 && pages < 400, qPrintable(QString::number(pages)));
        // Counted: the cells are laid out once each (50,000), and the rest of the work is in step with them.
        QVERIFY2(work.cells <= 2 * 50000 && work.rows <= 2 * 5000 && work.tree <= 2200000 && work.chars <= 1500000 && work.runs <= 150000 && work.ranges <= 150000, qPrintable(work.text()));
    }
}


// A cell of three thousand paragraphs (a row of a thousand parts on small pages) opens, and every paragraph of it is on a page.
void WriteTests::aCellOfThreeThousandLinesOpensAndIsPagedToTheEnd()
{
    QByteArray lines;
    for (int i = 0; i < 3000; ++i) lines += cellP("line " + QByteArray::number(i));
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"5200\" w:h=\"2400\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray zip = makeDocx(tableX(rowX(cellX(lines, QByteArray(), 3000)), 1, 3000) + cellP("after"), kStyles, QByteArray(), sect);
    std::unique_ptr<WriteDoc> doc;
    int pages = 0;
    const OpenWork work = openAndPageWork(zip, &pages, nullptr, &doc);
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    // Counted: the 3,000 lines are read about once each (a run and a range for each), and the pages are those of the lines.
    QVERIFY2(work.tree <= 70000 && work.chars <= 110000 && work.runs <= 10000 && work.ranges <= 10000 && work.pages <= 4500, qPrintable(work.text()));
    QVERIFY2(pages > 1000 && pages < 4000, qPrintable(QString::number(pages)));
    // Every paragraph of the cell is on a page: the last part has the last, and the parts go on from one to the next.
    double offset = -1;
    for (int k = 0; k < pages; k += 37) {
        const auto page = pg.materialize(k);
        QVERIFY(page && !page->tables.isEmpty());
        const auto *part = page->tables[0]->partOf(0);
        if (part) {
            QVERIFY2(part->piece.offsets[0] > offset, qPrintable(QString::number(k)));
            offset = part->piece.offsets[0];
        }
    }
    int lastWithTable = pages - 1;
    while (lastWithTable > 0 && pg.materialize(lastWithTable)->tables.isEmpty()) --lastWithTable;
    const auto last = pg.materialize(lastWithTable);
    QVERIFY(last && !last->tables.isEmpty() && last->tables[0]->partOf(0) && last->tables[0]->partOf(0)->piece.next == 0);
    QVERIFY(lastWithTable >= pages - 2);
    QVERIFY(plain(*doc).contains(QStringLiteral("line 2999")));
    QCOMPARE(rt::strictProblems(doc->toDocx()).size(), 0);
}


// Tables nested past the limit of depth are kept whole where they are, with the note, and the ones above it are tables, laid out with the tables inside them.
void WriteTests::tablesNestedPastTheLimitAreLaidOutDownToIt()
{
    QByteArray inner = cellP("level 11");
    for (int level = 10; level >= 0; --level)
        inner = cellP("level " + QByteArray::number(level)) + tableX(rowX(cellX(inner, QByteArray(), 4000) + cellX(cellP("beside " + QByteArray::number(level)), QByteArray(), 2000)), 2, 3000) + cellP("end of " + QByteArray::number(level));
    const QByteArray zip = makeDocx(inner, kStyles);
    QStringList notes;
    int pages = 0;
    const OpenWork work = openAndPageWork(zip, &pages, &notes);
    QVERIFY2(work.rows <= 30 && work.cells <= 60 && work.tree <= 2000 && work.chars <= 1000, qPrintable(work.text()));   // (counted: next to nothing)
    QVERIFY2(notes.size() == 1 && notes[0].contains(QLatin1String("table")), qPrintable(notes.join(QLatin1Char('|'))));
    QVERIFY(pages >= 1 && pages < 20);
    auto doc = open(zip);
    QVERIFY(doc);
    for (int k = 0; k < doc->pager().pageCount(); ++k) QVERIFY(doc->pager().materialize(k));
    const QByteArray saved = doc->toDocx();
    QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
    QVERIFY(documentXml(saved).contains("level 11<"));
}


// ---- random editing of tables

// A document of tables for the random commands: a table with a style, a header row, cells joined down (one with a long text), a row with a gap before its cells and one
// with a gap after, a long cell, a table in a cell with another in its own, a plain table after it, and text around them; on small pages, so that rows go over page
// breaks.
// A box set at a place that the text goes round, as a run (VML, as a file converted from the older form has them): `kind` is the wrap: 0 square, 1 top and bottom, 2 square
// with the text on the left, 3 on the larger side, 4 tight with the text on the right, 5 a square one set from the page.
QByteArray WriteTests::wallRun(int kind, int left, int top, int w, int h)
{
    static const char *const wraps[] = {"<w10:wrap type=\"square\"/>", "<w10:wrap type=\"topAndBottom\"/>", "<w10:wrap type=\"square\" side=\"left\"/>", "<w10:wrap type=\"square\" side=\"largest\"/>",
                                        "<w10:wrap type=\"tight\" side=\"right\"/>", "<w10:wrap type=\"square\"/>"};
    const QByteArray from = kind == 5 ? ";mso-position-horizontal-relative:page;mso-position-vertical-relative:page" : QByteArray();
    return "<w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"><v:rect id=\"wall" + QByteArray::number(left * 31 + top) + "\" style=\"position:absolute;margin-left:" +
           QByteArray::number(left) + "pt;margin-top:" + QByteArray::number(top) + "pt;width:" + QByteArray::number(w) + "pt;height:" + QByteArray::number(h) + "pt" + from + "\" fillcolor=\"red\" stroked=\"f\">" + wraps[kind % 6] + "</v:rect></w:pict></w:r>";
}

// The tables of that document, and its styles and its section's properties; `noteInCell` is the XML of a run put in the cell b1 (a footnote's reference).
RichTables WriteTests::richTables(const QByteArray &noteInCell)
{
    QByteArray longCell, tallCell;
    for (int i = 0; i < 24; ++i) tallCell += cellP("tall " + QByteArray::number(i));
    for (int i = 0; i < 9; ++i) longCell += cellP("merged line " + QByteArray::number(i));
    const QByteArray deep = tableX(rowX(cellX(cellP("deep"), QByteArray(), 700)), 1, 700);
    const QByteArray nested = tableX(rowX(cellX(cellP("n00"), QByteArray(), 800) + cellX(cellP("n01") + deep + cellP("n01 tail"), QByteArray(), 800)) + rowX(cellX(cellP("n10"), QByteArray(), 800) + cellX(cellP("n11"), QByteArray(), 800)), 2, 800);
    const QByteArray styled = tableX(rowX(cellX(cellP("h0"), QByteArray(), 1100) + cellX(cellP("h1"), QByteArray(), 1100) + cellX(cellP("h2"), QByteArray(), 1100), "<w:tblHeader/>") +
                                         rowX(cellX(longCell, "<w:vMerge w:val=\"restart\"/>", 1100) + cellX(cellP("a1"), "<w:tcBorders><w:bottom w:val=\"double\" w:sz=\"12\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1100) + cellX(cellP("a2") + nested + cellP("a2 tail"), QByteArray(), 1100)) +
                                         rowX(cellX("<w:p/>", "<w:vMerge/>", 1100) + cellX("<w:p><w:r><w:t>b1</w:t></w:r>" + noteInCell + "</w:p>", QByteArray(), 1100) + cellX(cellP("b2"), QByteArray(), 1100)) +
                                         rowX(cellX(cellP("G"), "<w:gridSpan w:val=\"2\"/>", 2200), "<w:gridBefore w:val=\"1\"/><w:wBefore w:w=\"1100\" w:type=\"dxa\"/>") +
                                         rowX(cellX(cellP("d0"), QByteArray(), 1100) + cellX(tallCell, QByteArray(), 1100) + cellX(cellP("d2"), "<w:tcBorders><w:top w:val=\"single\" w:sz=\"24\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1100), "<w:trHeight w:val=\"700\" w:hRule=\"atLeast\"/>") +
                                         rowX(cellX(cellP("e0"), QByteArray(), 1100) + cellX(cellP("e1"), QByteArray(), 1100), "<w:gridAfter w:val=\"1\"/><w:wAfter w:w=\"1100\" w:type=\"dxa\"/>"),
                                     3, 1100, "<w:tblStyle w:val=\"TG\"/><w:tblW w:w=\"3300\" w:type=\"dxa\"/><w:tblLook w:val=\"0420\"/>");
    const QByteArray plain = tableX(rowX(cellX(cellP("p00")) + cellX(cellP("p01")), "<w:trHeight w:val=\"600\" w:hRule=\"atLeast\"/>") +
                                        rowX(cellX(cellP("p10")) + cellX(cellP("p11"), "<w:tcBorders><w:left w:val=\"nil\"/><w:bottom w:val=\"single\" w:sz=\"18\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders><w:vAlign w:val=\"bottom\"/>"),
                                             "<w:trHeight w:val=\"500\" w:hRule=\"atLeast\"/>"),
                                    2, 1600);
    // A table with space between its cells (its lines round the lot, the cells' own as they say), rows of a set height, and cells joined down.
    const QByteArray none = "<w:tcBorders><w:top w:val=\"nil\"/><w:left w:val=\"nil\"/><w:bottom w:val=\"nil\"/><w:right w:val=\"nil\"/></w:tcBorders>";
    const QByteArray spaced = tableX(rowX(cellX(cellP("s00"), none, 1200) + cellX(cellP("s01"), "<w:tcBorders><w:bottom w:val=\"single\" w:sz=\"24\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1200), "<w:trHeight w:val=\"500\" w:hRule=\"atLeast\"/>") +
                                     rowX(cellX(cellP("s10") + cellP("s10 more"), "<w:vMerge w:val=\"restart\"/>", 1200) + cellX(cellP("s11"), none, 1200)) +
                                     rowX(cellX("<w:p/>", "<w:vMerge/>", 1200) + cellX(cellP("s21"), "<w:tcBorders><w:top w:val=\"double\" w:sz=\"8\" w:space=\"0\" w:color=\"auto\"/></w:tcBorders>", 1200)),
                                     2, 1200, "<w:tblW w:w=\"0\" w:type=\"auto\"/><w:tblCellSpacing w:w=\"15\" w:type=\"dxa\"/><w:tblBorders><w:top w:val=\"single\" w:sz=\"12\" w:space=\"0\" w:color=\"auto\"/><w:left w:val=\"single\" w:sz=\"8\" w:space=\"0\" w:color=\"auto\"/>"
                                     "<w:bottom w:val=\"single\" w:sz=\"12\" w:space=\"0\" w:color=\"auto\"/><w:right w:val=\"single\" w:sz=\"8\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders>");
    const QByteArray styles = kStyles +
                              "<w:style w:type=\"paragraph\" w:styleId=\"Heading2\"><w:name w:val=\"heading 2\"/><w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/><w:qFormat/>"
                              "<w:pPr><w:spacing w:before=\"120\" w:after=\"60\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arimo\" w:hAnsi=\"Arimo\"/><w:b/><w:sz w:val=\"26\"/></w:rPr></w:style>"
                              "<w:style w:type=\"table\" w:styleId=\"TG\"><w:name w:val=\"TG\"/><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
                              "<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
                              "<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr><w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/></w:rPr></w:tblStylePr>"
                              "<w:tblStylePr w:type=\"band2Horz\"><w:tcPr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"EEEEEE\"/></w:tcPr></w:tblStylePr></w:style>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"5200\" w:h=\"4400\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    return {styled, plain, styles, sect, spaced};
}

// A table set at a place, with the text going round it: two rows of two cells, `wide` points a cell, at the right margin, from the paragraph after it.
QByteArray WriteTests::floatingTable(int wide)
{
    const QByteArray pr = "<w:tblpPr w:leftFromText=\"120\" w:rightFromText=\"120\" w:vertAnchor=\"text\" w:horzAnchor=\"margin\" w:tblpXSpec=\"right\" w:tblpY=\"60\"/><w:tblOverlap w:val=\"never\"/><w:tblW w:w=\"0\" w:type=\"auto\"/>";
    const int twips = wide * 20;
    return tableX(rowX(cellX(cellP("float a"), QByteArray(), twips) + cellX(cellP("float b words"), QByteArray(), twips)) + rowX(cellX(cellP("float c"), QByteArray(), twips) + cellX(cellP("d"), QByteArray(), twips)), 2, twips, pr);
}

QByteArray WriteTests::richTableDocx()
{
    const RichTables t = richTables();
    // (Objects that the text goes round: in the paragraphs around the tables, of each wrap: edits that move their anchors across the pages change which pages have them.)
    const QByteArray frame = "<w:framePr w:w=\"1800\" w:hSpace=\"120\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\" w:y=\"60\"/>";
    return makeDocx(plainP("before the tables and a few more words so that a line goes round the box", wallRun(0, 110, 4, 50, 40)) + t.styled + framedP("FRAME one of two paragraphs", frame) + framedP("and the second of them, which is longer than a line of the frame", frame) +
                        plainP("between them, with enough words that more than a line or two are beside the box that is set from the page", wallRun(5, 150, 60, 40, 50)) +
                        floatingTable(60) + plainP("after the table that is set at a place, with words enough for the text to go round it for a few lines, and a few more words") + t.plain +
                        plainP("then one with space between its cells") + t.spaced +
                        plainP("after the tables, and a line or two of words more", wallRun(1, 20, 6, 40, 14) + wallRun(2, 60, 14, 40, 30)),
                    t.styles, QByteArray(), t.sect);
}


// A file for the random commands with the tables, notes, headers, footers, and sections together: a first section of tables (the one of richTableDocx, a note cited in
// the paragraph before them, one in a cell, and a long one after them), with headers and footers for the first, odd, and even pages of other heights; a continuous section
// of two columns with a table in it and a note; and a section on a page of its own with a header of its own, a note, and a page break.
QByteArray WriteTests::richMixedDocx(int gammaWords, bool endnotes)
{
    const RichTables t = richTables(refTo(3));
    const QString rel = QStringLiteral("rId");
    auto lines = [](const char *name, int n) {
        QList<QByteArray> out;
        for (int i = 1; i <= n; ++i) out << "<w:r><w:t>" + QByteArray(name) + " " + QByteArray::number(i) + "</w:t></w:r>";
        return out;
    };
    QMap<QString, QByteArray> parts;
    parts[QStringLiteral("/word/header1.xml")] = headerPart("hdr", lines("ODD HEAD", 1));
    parts[QStringLiteral("/word/header2.xml")] = headerPart("hdr", lines("FIRST HEAD", 2));
    parts[QStringLiteral("/word/header3.xml")] = headerPart("hdr", lines("EVEN HEAD", 4));
    parts[QStringLiteral("/word/header4.xml")] = headerPart("hdr", lines("THIRD HEAD", 3));
    parts[QStringLiteral("/word/footer1.xml")] = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                                                                   "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple>"});
    parts[QStringLiteral("/word/footer2.xml")] = headerPart("ftr", lines("FIRST FOOT", 2));
    parts[QStringLiteral("/word/footer3.xml")] = headerPart("ftr", lines("EVEN FOOT", 3));
    parts[QStringLiteral("/word/footnotes.xml")] = notesPart({{2, "ALPHA note of the paragraph before the tables"}, {3, wordsOf("BETA", 25)}, {4, wordsOf("GAMMA", gammaWords)}, {5, wordsOf("DELTA", 8)}, {6, wordsOf("EPSILON", 15)}});
    if (endnotes) parts[QStringLiteral("/word/endnotes.xml")] = endnotesOf({{12, "ENDNOTE ONE of the paragraph before the tables"}, {13, wordsOf("TWO", 30)}, {14, wordsOf("THREE", 12)}});
    const QByteArray refsOne = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:headerReference w:type=\"first\" r:id=\"rIdheader2\"/><w:headerReference w:type=\"even\" r:id=\"rIdheader3\"/>"
                               "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:footerReference w:type=\"first\" r:id=\"rIdfooter2\"/><w:footerReference w:type=\"even\" r:id=\"rIdfooter3\"/>";
    const QByteArray one = sectionXml(7200, 6600, 720, refsOne, QByteArray(), "<w:titlePg/>");
    const QByteArray two = sectionXml(7200, 6600, 720, QByteArray(), "<w:type w:val=\"continuous\"/>", "<w:cols w:num=\"2\" w:space=\"360\"/>");
    const QByteArray three = sectionXml(7200, 6600, 900, "<w:headerReference w:type=\"default\" r:id=\"rIdheader4\"/>");
    // (Objects the text goes round: one of each kind among the notes, tables, and sections, so that the text goes round them across pages and columns.)
    QByteArray body = plainP(wordsOf("before the tables", 14), refTo(2) + (endnotes ? endRef(12) : QByteArray()) + wallRun(0, 150, 4, 60, 50)) + t.styled + plainP(wordsOf("between them", 12), refTo(4) + wallRun(1, 40, 8, 50, 16)) + t.plain +
                      plainP("then one with space between its cells") + t.spaced + plainP("after the tables", QByteArray(), one);
    body += floatingTable(50) + plainP(wordsOf("after the table set at a place", 20)) + framedP("FRAME of the second section", "<w:framePr w:w=\"1400\" w:h=\"500\" w:hRule=\"exact\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"text\" w:xAlign=\"left\"/>") +
            plainP(wordsOf("Second section", 18), wallRun(3, 110, 10, 50, 40)) + tableOfRows(9, 2, 1000, [](int r, int c) { return r == 4 && c == 1 ? plainP("cites", refTo(5)) : cellP("r" + QByteArray::number(r) + "c" + QByteArray::number(c)); }) +
            plainP(wordsOf("end of the second", 30), wallRun(2, 60, 2, 40, 30), two);
    body += plainP(wordsOf("Third section", 40), refTo(6) + (endnotes ? endRef(14) : QByteArray()) + wallRun(4, 160, 6, 70, 40) + wallRun(5, 200, 120, 60, 60)) + "<w:p><w:r><w:t>before a break</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after it</w:t></w:r></w:p>" + plainP(wordsOf("Last", 60), wallRun(0, 20, 30, 50, 24));
    return makeDocxWithParts(body, three, parts, endnotes ? QByteArray("<w:evenAndOddHeaders/><w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>") : QByteArray("<w:evenAndOddHeaders/>"), t.styles);
}


// What a paginator that was told of the edits has that a new one on the same story does not (empty when there is nothing): the page count, and for each page its start,
// its lines, the tables on it with the heights of their rows, its notes, its header and footer, and its boxes (tests/flow/pageaudit.h).
QString WriteTests::layoutDiffersFromFresh(WriteDoc &doc, bool made)
{
    flow::Paginator fresh(&doc.flow());
    return audit::differenceFromFresh(doc.pager(), fresh, made);
}


// The story's paragraphs with their places and cells, and the pages' starts of the paginator and of a new one (for JW_FUZZ_DUMP).
QString WriteTests::layoutDump(WriteDoc &doc)
{
    QString out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        const flow::CellRecord *rec = doc.flow().tables.get(cellKeyOf(b));
        QString shown = b.text().left(24);
        shown.replace(QChar(flow::kPageBreak), QStringLiteral("<PB>"));
        if (b.blockFormat().pageBreakPolicy() != QTextFormat::PageBreak_Auto) shown += QStringLiteral(" {policy %1}").arg(int(b.blockFormat().pageBreakPolicy()));
        if (b.blockFormat().boolProperty(flow::fp::SectionBreakAfter)) shown += QStringLiteral(" {SECT}");
        out += QStringLiteral("  @%1 [%2] key %3 row %4%5\n").arg(b.position()).arg(shown).arg(cellKeyOf(b)).arg(rec ? reinterpret_cast<quintptr>(rec->row.get()) % 100000 : 0)
                   .arg(rec ? QStringLiteral(" span %1 merge %2 gap %3/%4 parent %5").arg(rec->cell.span).arg(rec->cell.vmerge == flow::CellData::VMerge::Restart ? QLatin1String("restart") : rec->cell.vmerge == flow::CellData::VMerge::Continue ? QLatin1String("continue") : QLatin1String("-")).arg(rec->row->before).arg(rec->row->after).arg(rec->parent) : QString());
    }
    flow::Paginator fresh(&doc.flow());
    flow::Paginator &pg = doc.pager();
    out += QStringLiteral("pages (the paginator / a new one):");
    for (int k = 0; k < std::max(pg.pageCount(), fresh.pageCount()); ++k) {
        const auto a = k < pg.pageCount() ? pg.page(k).start : flow::Paginator::PageStart(), b = k < fresh.pageCount() ? fresh.page(k).start : flow::Paginator::PageStart();
        out += QStringLiteral(" %1:(%2,%3,%4)/(%5,%6,%7)").arg(k).arg(a.pos).arg(a.skip).arg(a.piece).arg(b.pos).arg(b.skip).arg(b.piece);
    }
    return out;
}


// The story as the commands leave it: each paragraph's text, its cell's record, and its style (undo gives it back as it was).
QString WriteTests::storyState(WriteDoc &doc)
{
    QString out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        out += QStringLiteral("[%1|%2|%3|%4]").arg(b.text()).arg(cellKeyOf(b)).arg(b.blockFormat().stringProperty(tp::StyleName)).arg(int(b.blockFormat().alignment()));
    // (And the words of the notes the text cites: a note nothing cites is not the document's.)
    for (const int id : doc.citedFootnotes()) out += QStringLiteral("{note %1: %2}").arg(id).arg(doc.editableFootnote(id)->toPlainText());
    return out;
}


// The commands of the Table tabs, at random on a table with merged cells and notes: each is one step that undoes and redoes to the same table, the file saved after it
// opens the same, and the pages lay out as a new paginator lays them out. (JW_FUZZ_SEEDS=n for a longer run.)
void WriteTests::randomTableTabCommandsKeepTablesWholeAndOpenTheSame()
{
    const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 10;
    const int steps = qEnvironmentVariableIntValue("JW_FUZZ_STEPS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_STEPS") : 40;
    for (unsigned seed = 1; seed <= unsigned(seeds); ++seed) {
        auto doc = open(richTableDocx());
        QVERIFY(doc);
        std::mt19937 rng(seed + 5000u);
        auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
        QTextCursor c(doc->story());
        QStringList log;
        const QByteArray started = documentXml(doc->toDocx());
        for (int step = 0; step < steps; ++step) {
            const int kind = 1 + pick(10);
            const QString before = tablesSignature(*doc);
            log << QString::number(kind);
            if (pick(8) == 0) {
                // (and the older commands among them)
                tableCommand(*doc, c, pick(5), pick);
            } else {
                tableTabCommand(*doc, c, kind, pick);
            }
            const QString why = QStringLiteral("seed %1 step %2 (%3)").arg(seed).arg(step).arg(log.join(QLatin1Char(',')));
            const QString lay = layoutDiffersFromFresh(*doc);
            QVERIFY2(lay.isEmpty(), qPrintable(why + QLatin1Char(' ') + lay));
            const QString after = tablesSignature(*doc);
            if (step % 3 == 0) {
                const QByteArray zip = doc->toDocx();
                QVERIFY2(rt::strictProblems(zip).isEmpty(), qPrintable(why + QLatin1Char(' ') + rt::strictProblems(zip).join(QLatin1Char('\n'))));
                auto back = open(zip);
                QVERIFY2(back, qPrintable(why));
                QVERIFY2(tablesSignature(*back) == after, qPrintable(why + QStringLiteral(": the file opens differently")));
            }
            // One undo takes a step back to the table as it was, and a redo makes it again (when the command changed something).
            if (after != before && step % 4 == 1) {
                doc->story()->undo();
                QVERIFY2(tablesSignature(*doc) == before, qPrintable(why + QStringLiteral(": undo does not give back the table")));
                doc->story()->redo();
                QVERIFY2(tablesSignature(*doc) == after, qPrintable(why + QStringLiteral(": redo does not make the table again")));
            }
        }
        // All of it undone, the file written is the one that was opened.
        while (doc->story()->isUndoAvailable()) doc->story()->undo();
        QVERIFY2(documentXml(doc->toDocx()) == started, qPrintable(QStringLiteral("seed %1: all undone, the document is not as it was opened").arg(seed)));
    }
}


// Hundreds of random commands on a document of tables (every one that works on a table, the ones that delete across cells, paste, and type in cells that are joined or in
// tables in cells): after each, the file it makes is well-formed and has tables of the shape tables have, the paginator that was told of the edits has the pages and the
// rows' heights a new one finds, and what was done undoes and does again. JW_FUZZ_SEEDS says how many seeds, JW_FUZZ_FROM where to begin, JW_FUZZ_STEPS how many commands;
// JW_FUZZ_COUNTS=1 says what the seeds found, JW_FUZZ_TRACE=1 what each did (JW_FUZZ_PAGES=1: and how many pages the file had after each). A second, shorter run is of a file
// with the notes, headers, footers, and sections as well (richMixedDocx), whose commands also type where notes are cited, put in section breaks, and change the page setup
// and the headers of a section; there the pages (their lines, table rows, notes, headers and footers, and boxes) are compared, and the file is not written (JP_FUZZ=n: n seeds of it).
void WriteTests::randomEditingOfTablesKeepsThemWholeAndTheLayoutRight()
{
    QMap<QString, int> failed;
    QMap<QString, QString> example;
    QSet<QString> failing;
    int seedsRun = 0;
    // (Two kinds of file: the tables alone, and then the tables with notes, headers, footers, and sections: a short run of those, whose edits also type where notes are
    // cited, put section breaks in, and change what the headers and footers are. JP_FUZZ=n: seeds 1 to n of the second, for a longer run.)
    for (const bool mixed : {false, true}) {
        // (The file of the odd seeds has endnotes, and their commands edit and make notes, and they are shorter; the even seeds have the file and the commands the fuzz has always had.)
        const QByteArray baseFile = mixed ? richMixedDocx(220, false) : richTableDocx();
        const QByteArray notesFile = mixed ? richMixedDocx() : QByteArray();
        const int seeds = mixed ? (qEnvironmentVariableIntValue("JP_FUZZ") > 0 ? qEnvironmentVariableIntValue("JP_FUZZ") : 6)
                                : (qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 12);
        const int steps = qEnvironmentVariableIntValue("JW_FUZZ_STEPS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_STEPS") : (mixed ? 40 : 70);
        for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
            const bool noteSeed = mixed && seed % 2 == 1;
            const QByteArray &original = noteSeed ? notesFile : baseFile;
            auto doc = open(original);
            QVERIFY(doc);
            ++seedsRun;
            std::mt19937 rng(seed + (mixed ? 1000u : 0u));
            auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
            QTextCursor c(doc->story());
            QTextDocumentFragment held;
            QStringList log;
            QString lastDump;
            auto fail = [&](const QString &what, const QString &detail) {
                failing.insert((mixed ? QStringLiteral("mixed ") : QString()) + QString::number(seed));
                if (failed[what]++ == 0) example[what] = QStringLiteral("%1seed %2: %3 | last: %4").arg(mixed ? "mixed " : "").arg(seed).arg(detail.left(260), log.mid(std::max<qsizetype>(0, log.size() - 5)).join(QStringLiteral(" , ")));
                if (qEnvironmentVariableIsSet("JW_FUZZ_DUMP")) qInfo("seed %d: %s: %s\nlast: %s\nBEFORE THE LAST COMMAND:\n%s\nAFTER:\n%s", int(seed), qPrintable(what), qPrintable(detail), qPrintable(log.mid(std::max<qsizetype>(0, log.size() - 5)).join(QStringLiteral(" , "))), qPrintable(lastDump), qPrintable(layoutDump(*doc)));
            };
            static const char *const names[] = {"type", "enter", "insertRow", "insertCol", "tab", "delRow", "delCol", "delTable", "removeSel", "bigSel", "backspace", "delete", "copy", "paste", "undo", "redo",
                                                "style", "bold", "align", "pageBreak", "list", "typeInCell", "removeAcross", "shiftTab", "typeNoted", "sectionBreak", "pageSetup", "headerSwap",
                                                "headerType", "headerEnter", "headerDelete", "headerOption", "pageNumber", "noteType", "noteInsert", "noteDeleteReference", "noteCutPaste", "noteFormat", "endnoteType", "endnoteInsert", "endnoteDeleteReference", "endnoteCutPaste", "endnoteEdit"};
            bool headersEdited = false;   // (a header or footer was edited: the file it makes is checked)
            bool notesEdited = false;     // (a note was edited, made, or lost its reference: the file it makes is checked)
            // The reference of a note the text cites, at random: the note's Id and the place of the reference character.
            auto someReference = [&](int *id, int *at) {
                const QVector<int> ids = doc->citedFootnotes();
                if (ids.isEmpty()) return false;
                *id = ids[pick(int(ids.size()))];
                const QVector<int> refs = doc->footnoteReferences(*id);
                if (refs.isEmpty()) return false;
                *at = refs[pick(int(refs.size()))];
                return true;
            };
            // A header or footer story of a random section and slot, in the form it is edited in (null where there is none).
            auto someHeader = [&]() -> QTextDocument * {
                return doc->editableHeaderFooter(pick(doc->sectionCount()), pick(2), pick(3));
            };
            if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                fail(QStringLiteral("layout differs from a new paginator's as the file is opened"), d);
                continue;
            }
            auto selectCells = [&] {   // (from a place in one cell to a place in another)
                QVector<QTextBlock> cells;
                for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
                    if (cellKeyOf(b)) cells << b;
                if (cells.isEmpty()) return false;
                const QTextBlock a = cells[pick(int(cells.size()))], z = cells[pick(int(cells.size()))];
                c.setPosition(a.position() + pick(a.length()));
                c.setPosition(z.position() + pick(z.length()), QTextCursor::KeepAnchor);
                return true;
            };
            for (int step = 0; step < (noteSeed && qEnvironmentVariableIntValue("JW_FUZZ_STEPS") <= 0 ? 30 : steps); ++step) {
                const int op = pick(mixed ? (noteSeed ? 43 : 33) : 24);   // (the commands that edit and make notes are those of the seeds that are odd: the seeds that are even have the commands the file has always had)
                const QString before = storyState(*doc);
                lastDump = qEnvironmentVariableIsSet("JW_FUZZ_DUMP") ? layoutDump(*doc) : QString();
                auto place = [&] { c.setPosition(pick(doc->story()->characterCount() - 1)); };
                auto selectSome = [&](int most) {
                    const int a = pick(doc->story()->characterCount() - 1), len = pick(most);
                    c.setPosition(a);
                    c.setPosition(std::min(a + len, doc->story()->characterCount() - 1), QTextCursor::KeepAnchor);
                };
                bool one = false;   // (a command that is one step to undo)
                switch (op) {
                case 0: place(); typeText(*doc, c, QStringLiteral("word ")); break;
                case 1: place(); insertParagraph(*doc, c); break;
                case 2: case 3: case 5: case 6:
                    if (pick(3) == 0 && selectCells()) {
                        if (op == 2) insertTableRow(*doc, c, pick(2));
                        else if (op == 3) insertTableColumn(*doc, c, pick(2));
                        else if (op == 5) deleteTableRows(*doc, c);
                        else deleteTableColumns(*doc, c);
                    } else {
                        tableCommand(*doc, c, op == 2 ? 0 : op == 3 ? 1 : op == 5 ? 3 : 4, pick);
                    }
                    one = true;
                    break;
                case 4: tableCommand(*doc, c, 2, pick); break;
                case 7: tableCommand(*doc, c, 5, pick); one = true; break;
                case 8: selectSome(100); removeSelection(*doc, c); one = true; break;
                case 9: selectSome(mixed ? 1500 : 100000); removeSelection(*doc, c); one = true; break;
                case 10: place(); deleteBack(*doc, c, false); break;
                case 11: place(); deleteForward(*doc, c, false); break;
                case 12: selectSome(200); held = selectionFragment(c); break;
                case 13: place(); pasteFragment(*doc, c, held); one = true; break;
                case 14: doc->undo(); break;
                case 15: doc->redo(); break;
                case 16: selectSome(300); applyParagraphStyle(*doc, c, pick(2) ? QStringLiteral("Heading2") : QStringLiteral("Normal")); one = true; break;
                case 17: selectSome(200); toggleBold(*doc, c); one = true; break;
                case 18: selectSome(200); setAlignment(*doc, c, pick(2) ? Qt::AlignRight : Qt::AlignHCenter); one = true; break;
                case 19: place(); insertPageBreak(*doc, c); break;
                case 20: selectSome(150); toggleList(*doc, c, pick(2)); one = true; break;
                case 21: if (placeInACell(*doc, c, pick)) typeText(*doc, c, QStringLiteral("typed in a cell with words")); break;
                case 22: if (selectCells()) removeSelection(*doc, c); one = true; break;
                case 23: if (placeInACell(*doc, c, pick)) insertTab(*doc, c, true); break;
                case 24: {   // typing in a paragraph, or a cell, that cites a note
                    QVector<QTextBlock> noted;
                    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
                        for (auto it = b.begin(); !it.atEnd(); ++it)
                            if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"))) {
                                noted << b;
                                break;
                            }
                    if (!noted.isEmpty()) {
                        const QTextBlock b = noted[pick(int(noted.size()))];
                        c.setPosition(b.position() + pick(b.length()));
                        typeText(*doc, c, QStringLiteral("noted words "));
                    }
                    break;
                }
                case 25: place(); insertSectionBreak(*doc, c); break;
                case 26: {   // a section's page setup: whether its first page has a header of its own, how far the header and the footer are from the edge, the margins
                    const int index = pick(doc->sectionCount());
                    wml::SectionProps props = doc->sectionProps(index);
                    switch (pick(4)) {
                    case 0: props.titlePg = !props.titlePg; break;
                    case 1: props.header = 360 + 360 * pick(5); break;
                    case 2: props.top = 720 + 360 * pick(4); break;
                    default: props.footer = 360 + 360 * pick(5); props.bottom = 720 + 360 * pick(3); break;
                    }
                    doc->setSectionProps(index, props);
                    break;
                }
                case 28: {   // typing in a header or a footer
                    if (QTextDocument *h = someHeader()) {
                        QTextCursor hc(h);
                        hc.setPosition(pick(h->characterCount() - 1));
                        typeText(*doc, hc, QStringLiteral("head words "));
                        headersEdited = true;
                    }
                    break;
                }
                case 29: {   // a paragraph more in one: the header is taller
                    if (QTextDocument *h = someHeader()) {
                        QTextCursor hc(h);
                        hc.setPosition(pick(h->characterCount() - 1));
                        for (int i = 0, n = 1 + pick(4); i < n; ++i) insertParagraph(*doc, hc);
                        typeText(*doc, hc, QStringLiteral("a taller one"));
                        headersEdited = true;
                    }
                    break;
                }
                case 30: {   // text taken out of one, across paragraphs and fields
                    if (QTextDocument *h = someHeader()) {
                        QTextCursor hc(h);
                        const int a = pick(h->characterCount() - 1);
                        hc.setPosition(a);
                        hc.setPosition(std::min(a + pick(60), h->characterCount() - 1), QTextCursor::KeepAnchor);
                        if (pick(2)) removeSelection(*doc, hc);
                        else deleteBack(*doc, hc, pick(2));
                        headersEdited = true;
                    }
                    break;
                }
                case 31: {   // the options: a header made, linked, or unlinked; a different first page; different odd and even pages
                    const int index = pick(doc->sectionCount());
                    switch (pick(5)) {
                    case 0: doc->createHeaderFooter(index, pick(2), pick(3)); break;
                    case 1: doc->setLinkedToPrevious(index, pick(2), pick(3), pick(2)); break;
                    case 2: doc->setDifferentFirstPage(index, pick(2)); break;
                    case 3: doc->setDifferentOddEven(pick(2)); break;
                    default: doc->createHeaderFooter(index, pick(2), pick(3)); doc->setLinkedToPrevious(index, pick(2), pick(3), false); break;
                    }
                    headersEdited = true;
                    break;
                }
                case 32: {   // a page number in one
                    if (QTextDocument *h = someHeader()) {
                        QTextCursor hc(h);
                        hc.setPosition(pick(h->characterCount() - 1));
                        insertPageNumber(*doc, hc);
                        headersEdited = true;
                    }
                    break;
                }
                case 33: {   // words typed in a note, in the middle of it or at its end
                    int id = 0, at = 0;
                    if (someReference(&id, &at))
                        if (QTextDocument *note = doc->editableFootnote(id)) {
                            QTextCursor nc(note);
                            nc.setPosition(noteTextStart(note) + pick(std::max(1, note->characterCount() - 1 - noteTextStart(note))));
                            typeText(*doc, nc, QStringLiteral("note words "));
                            notesEdited = one = true;
                        }
                    break;
                }
                case 34: {   // Insert Footnote somewhere in the text, and words typed in the note it makes
                    place();
                    QTextCursor ic = c;
                    if (QTextDocument *note = insertFootnote(*doc, ic)) {
                        typeText(*doc, ic, QStringLiteral("made note"));
                        Q_UNUSED(note);
                        notesEdited = one = true;
                    }
                    break;
                }
                case 35: {   // a reference taken out, with its note
                    int id = 0, at = 0;
                    if (someReference(&id, &at)) {
                        QTextCursor rc(doc->story());
                        rc.setPosition(at);
                        rc.setPosition(at + 1, QTextCursor::KeepAnchor);
                        removeSelection(*doc, rc);
                        notesEdited = one = true;
                    }
                    break;
                }
                case 36: {   // a reference cut (or copied) and pasted somewhere else: the note moves (or is copied)
                    int id = 0, at = 0;
                    if (someReference(&id, &at)) {
                        QTextCursor rc(doc->story());
                        rc.setPosition(at);
                        rc.setPosition(at + 1, QTextCursor::KeepAnchor);
                        const QTextDocumentFragment taken = selectionFragment(rc);
                        if (pick(2)) removeSelection(*doc, rc);
                        place();
                        pasteFragment(*doc, c, taken);
                        notesEdited = true;
                    }
                    break;
                }
                case 37: {   // a note's words taken out, formatted, or joined (and the number never taken)
                    int id = 0, at = 0;
                    if (someReference(&id, &at))
                        if (QTextDocument *note = doc->editableFootnote(id)) {
                            QTextCursor nc(note);
                            const int a = pick(std::max(1, note->characterCount() - 1));
                            nc.setPosition(a);
                            nc.setPosition(std::min(a + pick(40), note->characterCount() - 1), QTextCursor::KeepAnchor);
                            switch (pick(4)) {
                            case 0: removeSelection(*doc, nc); break;
                            case 1: toggleBold(*doc, nc); break;
                            case 2: deleteBack(*doc, nc, pick(2)); break;
                            default: insertParagraph(*doc, nc); break;
                            }
                            if (note->characterAt(0) != QChar::ObjectReplacementCharacter) {
                                fail(QStringLiteral("a note's number was taken"), QString());
                                break;
                            }
                            notesEdited = one = true;
                        }
                    break;
                }
                case 38: {   // words typed in an endnote
                    const QVector<int> ids = doc->citedEndnotes();
                    int from = 0, to = 0;
                    if (!ids.isEmpty() && doc->endnoteWords(ids[pick(int(ids.size()))], &from, &to)) {
                        QTextCursor nc(doc->story());
                        nc.setPosition(from + pick(to - from + 1));
                        typeText(*doc, nc, QStringLiteral("end words "));
                        notesEdited = one = true;
                    }
                    break;
                }
                case 39: {   // Insert Endnote somewhere in the text, and words typed in it
                    place();
                    QTextCursor ic = c;
                    if (insertEndnote(*doc, ic)) {
                        typeText(*doc, ic, QStringLiteral("made endnote"));
                        notesEdited = one = true;
                    }
                    break;
                }
                case 40: {   // an endnote's reference taken out, with its note
                    const QVector<int> ids = doc->citedEndnotes();
                    if (!ids.isEmpty()) {
                        const QVector<int> refs = doc->endnoteReferences(ids[pick(int(ids.size()))]);
                        if (!refs.isEmpty()) {
                            const int at = refs[pick(int(refs.size()))];
                            QTextCursor rc(doc->story());
                            rc.setPosition(at);
                            rc.setPosition(at + 1, QTextCursor::KeepAnchor);
                            removeSelection(*doc, rc);
                            notesEdited = one = true;
                        }
                    }
                    break;
                }
                case 41: {   // a reference to an endnote cut (or copied) and pasted somewhere else: the note moves (or is copied)
                    const QVector<int> ids = doc->citedEndnotes();
                    if (!ids.isEmpty()) {
                        const QVector<int> refs = doc->endnoteReferences(ids[pick(int(ids.size()))]);
                        if (!refs.isEmpty()) {
                            const int at = refs[pick(int(refs.size()))];
                            QTextCursor rc(doc->story());
                            rc.setPosition(at);
                            rc.setPosition(at + 1, QTextCursor::KeepAnchor);
                            const QTextDocumentFragment taken = selectionFragment(rc);
                            if (pick(2)) removeSelection(*doc, rc);
                            place();
                            pasteFragment(*doc, c, taken);
                            notesEdited = true;
                        }
                    }
                    break;
                }
                case 42: {   // an endnote's words taken out, formatted, joined, or cut by a selection that begins in them and goes anywhere (it stops at the note's end: nothing else is touched)
                    const QVector<int> ids = doc->citedEndnotes();
                    int from = 0, to = 0;
                    if (!ids.isEmpty() && doc->endnoteWords(ids[pick(int(ids.size()))], &from, &to)) {
                        QTextCursor nc(doc->story());
                        const int a = from + pick(to - from + 1);
                        nc.setPosition(a);
                        nc.setPosition(pick(3) == 0 ? pick(doc->story()->characterCount() - 1) : std::min(a + pick(40), to), QTextCursor::KeepAnchor);
                        switch (pick(5)) {
                        case 0: removeSelection(*doc, nc); break;
                        case 1: toggleBold(*doc, nc); break;
                        case 2: deleteBack(*doc, nc, pick(2)); break;
                        case 3: insertParagraph(*doc, nc); break;
                        default: deleteForward(*doc, nc, pick(2)); break;
                        }
                        notesEdited = one = true;
                    }
                    break;
                }
                default: {   // the headers a section names swapped for other parts (the odd pages' for the even pages', the third section's for the first page's)
                    const int index = pick(doc->sectionCount());
                    wml::SectionProps props = doc->sectionProps(index);
                    bool swapped = false;
                    for (wml::RawChild &child : props.extra) {
                        if (!child.xml.contains("headerReference")) continue;
                        for (const auto &pair : {std::make_pair("rIdheader1", "rIdheader3"), std::make_pair("rIdheader3", "rIdheader1"), std::make_pair("rIdheader4", "rIdheader2"), std::make_pair("rIdheader2", "rIdheader4")})
                            if (!swapped && child.xml.contains(pair.first)) {
                                child.xml.replace(pair.first, pair.second);
                                swapped = true;
                            }
                    }
                    if (swapped) doc->setSectionProps(index, props);
                    break;
                }
                }
                log << QStringLiteral("%1@%2").arg(QLatin1String(names[op])).arg(c.position());
            if (qEnvironmentVariableIsSet("JW_FUZZ_PAGES")) log.last() += QStringLiteral("(%1)").arg(doc->pager().pageCount());
                const QString what = QLatin1String(names[op]);
                // Every endnote the text cites is set once, in the order of the references, and each begins with its number that no edit has taken.
                if (mixed && notesEdited) {
                    QString lost;
                    for (const int id : doc->citedEndnotes()) {
                        int from = 0, to = 0;
                        if (!doc->endnoteWords(id, &from, &to)) lost = QStringLiteral("endnote %1 is cited and not set").arg(id);
                        else if (from < 1 || doc->story()->characterAt(from - 1) != QChar::ObjectReplacementCharacter) lost = QStringLiteral("endnote %1 has lost its number").arg(id);
                    }
                    if (!lost.isEmpty()) {
                        fail(QStringLiteral("the endnotes after ") + what, lost);
                        break;
                    }
                }
                // The paginator that was told of the edits has the pages and the rows' heights that a new one finds.
                if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                    fail(QStringLiteral("layout differs from a new paginator's after ") + what, d);
                    break;
                }
                // The text goes round the objects, on the pages that have them, as the page layer draws it (every third command: the pages are made for it).
                if (step % 3 == 2)
                    if (const QString d = audit::wallProblems(doc->pager(), doc->story()); !d.isEmpty()) {
                        fail(QStringLiteral("the text does not go round the objects after ") + what, d);
                        break;
                    }
                // The text is on the pages whole, and every frame and table set at a place is drawn once or in the text (every third command: the pages are made for it).
                if (const QString d = audit::coverageProblems(doc->pager(), doc->story()); !d.isEmpty()) {
                    fail(QStringLiteral("text is lost from the pages after ") + what, d);
                    break;
                }
                if (step % 3 == 1)
                    if (const QString d = audit::floatProblems(doc->pager(), doc->flow()); !d.isEmpty()) {
                        fail(QStringLiteral("a frame or a table set at a place is lost after ") + what, d);
                        break;
                    }
                if (!mixed) {
                    QString err;
                    const QByteArray zip = doc->toDocx(&err);
                    if (zip.isEmpty()) { fail(QStringLiteral("not written"), err); break; }
                    if (const QStringList strict = rt::strictProblems(zip, original); !strict.isEmpty()) { fail(QStringLiteral("saved file after %1: ").arg(what) + strict.first().left(120), strict.join(QLatin1Char(';'))); break; }
                }
                if (step % 4 != 3) continue;
                if (mixed && headersEdited) {
                    // The file keeps the headers and footers as edited: it opens, and each one reads again as the same story.
                    QString err;
                    const QByteArray zip = doc->toDocx(&err);
                    auto back = zip.isEmpty() ? nullptr : WriteDoc::fromDocx(zip, &err);
                    if (!back) {
                        fail(QStringLiteral("the file with the headers does not open again"), err);
                        break;
                    }
                    QString differs;
                    if (back->sectionCount() != doc->sectionCount()) differs = QStringLiteral("sections");
                    for (int s = 0; s < doc->sectionCount() && differs.isEmpty(); ++s)
                        for (const bool footer : {false, true})
                            for (int slot = 0; slot < 3 && differs.isEmpty(); ++slot) {
                                QTextDocument *was = doc->headerFooterStory(s, footer, slot), *now = back->editableHeaderFooter(s, footer, slot);
                                if (!was != !now) differs = QStringLiteral("section %1 %2 slot %3 is there in only one").arg(s).arg(footer ? "footer" : "header").arg(slot);
                                else if (was && was->toPlainText() != now->toPlainText()) differs = QStringLiteral("section %1 %2 slot %3 reads %4 not %5").arg(s).arg(footer ? "footer" : "header").arg(slot).arg(now->toPlainText().left(40), was->toPlainText().left(40));
                            }
                    if (differs.isEmpty() && (back->differentOddEven() != doc->differentOddEven())) differs = QStringLiteral("different odd and even pages");
                    if (!differs.isEmpty()) {
                        fail(QStringLiteral("the headers after saving are not the headers that were edited"), differs);
                        break;
                    }
                }
                if (mixed && notesEdited) {
                    // The file keeps the notes as edited: it is whole, it opens, and the notes the text cites say the same words.
                    QString err;
                    const QByteArray zip = doc->toDocx(&err);
                    auto back = zip.isEmpty() ? nullptr : WriteDoc::fromDocx(zip, &err);
                    if (!back) {
                        fail(QStringLiteral("the file with the notes does not open again"), err);
                        break;
                    }
                    if (const QStringList strict = rt::strictProblems(zip, original); !strict.isEmpty()) {
                        fail(QStringLiteral("the file with the notes is not whole"), strict.first().left(120));
                        break;
                    }
                    QString differs;
                    const QVector<int> ids = doc->citedFootnotes();
                    if (back->citedFootnotes() != ids) differs = QStringLiteral("the notes cited are %1 and %2 after saving").arg(ids.size()).arg(back->citedFootnotes().size());
                    auto endText = [](WriteDoc &d, int id) {
                        QStringList out;
                        for (const QTextBlock &b : endnoteParagraphs(d, id)) out << b.text();
                        return out.join(QLatin1Char('\n'));
                    };
                    if (back->citedEndnotes() != doc->citedEndnotes()) differs = QStringLiteral("the endnotes cited are %1 and %2 after saving").arg(doc->citedEndnotes().size()).arg(back->citedEndnotes().size());
                    for (const int id : doc->citedEndnotes())
                        if (differs.isEmpty() && endText(*doc, id) != endText(*back, id)) differs = QStringLiteral("endnote %1 reads %2 and not %3").arg(id).arg(endText(*back, id).left(40), endText(*doc, id).left(40));
                    for (int i = 0; i < ids.size() && differs.isEmpty(); ++i) {
                        QTextDocument *was = doc->editableFootnote(ids[i]), *now = back->editableFootnote(ids[i]);
                        if (!now || was->toPlainText() != now->toPlainText()) differs = QStringLiteral("note %1 reads %2 and not %3").arg(ids[i]).arg(now ? now->toPlainText().left(40) : QStringLiteral("<none>"), was->toPlainText().left(40));
                        else if (doc->footnoteReferences(ids[i]).size() != back->footnoteReferences(ids[i]).size()) differs = QStringLiteral("note %1 has other references after saving").arg(ids[i]);
                    }
                    if (!differs.isEmpty()) {
                        fail(QStringLiteral("the notes after saving are not the notes that were edited"), differs);
                        break;
                    }
                }
                if (!mixed) {
                    QString err;
                    const QByteArray zip = doc->toDocx(&err);
                    auto back = WriteDoc::fromDocx(zip, &err);
                    if (!back) { fail(QStringLiteral("does not open again"), err); break; }
                    if (documentXml(back->toDocx()) != documentXml(zip)) fail(QStringLiteral("the second save differs"), QString());
                    QString a = plain(*doc), b = plain(*back);
                    a.remove(QChar::ObjectReplacementCharacter);
                    b.remove(QChar::ObjectReplacementCharacter);
                    if (a != b) fail(QStringLiteral("what opens again has other text"), QString());
                }
                // What was done undoes, and does again, and the pages follow both.
                if (one && op != 14 && op != 15) {
                    const QString after = storyState(*doc);
                    if (after != before) {
                        int undone = 0;
                        while (undone < 8 && doc->canUndo() && storyState(*doc) != before) {
                            doc->undo();
                            ++undone;
                        }
                        if (storyState(*doc) != before) {
                            fail(QStringLiteral("undo of %1 does not give the story back").arg(what), QString());
                            break;
                        }
                        if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                            fail(QStringLiteral("layout differs from a new paginator's after undoing ") + what, d);
                            break;
                        }
                        for (int i = 0; i < undone; ++i) doc->redo();
                        if (storyState(*doc) != after) {
                            fail(QStringLiteral("redo of %1 does not do it again").arg(what), QString());
                            break;
                        }
                        if (const QString d = layoutDiffersFromFresh(*doc); !d.isEmpty()) {
                            fail(QStringLiteral("layout differs from a new paginator's after redoing ") + what, d);
                            break;
                        }
                    }
                }
            }
    
            if (qEnvironmentVariableIsSet("JW_FUZZ_TRACE")) qInfo("%sseed %u: %d pages; %s", mixed ? "mixed " : "", seed, doc->pager().pageCount(), qPrintable(log.join(QLatin1Char(' '))));
        }
    }
    QStringList report;
    for (auto it = failed.cbegin(); it != failed.cend(); ++it) report << QStringLiteral("%1: %2 times\n  %3").arg(it.key()).arg(it.value()).arg(example.value(it.key()));
    if (qEnvironmentVariableIsSet("JW_FUZZ_COUNTS")) qInfo("table fuzz: %d of %d seeds failed:\n%s", int(failing.size()), seedsRun, qPrintable(report.join(QLatin1Char('\n'))));
    QVERIFY2(failed.isEmpty(), qPrintable(QStringLiteral("%1 of %2 seeds failed:\n%3").arg(failing.size()).arg(seedsRun).arg(report.join(QLatin1Char('\n')))));
}


// The documents of the random commands have objects that the text goes round, on their pages (else the commands would not try the walls).
void WriteTests::theDocumentsOfTheRandomCommandsHaveObjectsTheTextGoesRound()
{
    for (const bool mixed : {false, true}) {
        auto doc = open(mixed ? richMixedDocx() : richTableDocx());
        QVERIFY(doc);
        int walls = 0, pagesWithWalls = 0;
        for (int k = 0; k < doc->pager().pageCount(); ++k) {
            const auto page = doc->pager().materialize(k);
            QVERIFY(page);
            const int here = int(audit::wallsOf(*page).size());
            walls += here;
            pagesWithWalls += here > 0;
        }
        QVERIFY2(walls >= 4 && pagesWithWalls >= 2, qPrintable(QStringLiteral("%1 walls on %2 pages").arg(walls).arg(pagesWithWalls)));
        QVERIFY(doc->flow().frames.size() >= 1);
        int floating = 0;
        for (int k = 0; k < doc->pager().pageCount(); ++k) {
            const auto page = doc->pager().materialize(k);   // (kept: the tables are the page's, which a temporary would take with it before the loop)
            QVERIFY(page);
            for (const auto &t : page->tables) floating += t->floating;
        }
        QVERIFY2(floating >= 1, "a table set at a place is drawn on some page");
        QVERIFY2(audit::wallProblems(doc->pager(), doc->story()).isEmpty(), qPrintable(audit::wallProblems(doc->pager(), doc->story())));
    }
}


// A shape that says it has no fill or no line has none (a shape's own fill is a color it was not given); one that says nothing is filled white, with a line, as the file's form has it.
void WriteTests::aShapeWithNoFillOrLineHasNone()
{
    const QByteArray vml = "xmlns:v=\"urn:schemas-microsoft-com:vml\"";
    auto objectOf = [&](const QByteArray &attributes) {
        const QByteArray body = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict " + vml + "><v:rect id=\"r\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:50pt;height:20pt\" " + attributes + "/></w:pict></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        return doc && doc->flow().objects.size() == 1 ? doc->flow().objects[0].item : ItemPtr();
    };
    const ItemPtr none = objectOf("filled=\"f\" stroked=\"f\""), plain = objectOf(""), red = objectOf("fillcolor=\"red\" strokecolor=\"blue\" strokeweight=\"2pt\"");
    QVERIFY(none && plain && red);
    QVERIFY(none->fill.isNone() && none->stroke.isNone());
    QVERIFY(!plain->fill.isNone() && !plain->stroke.isNone());
    QVERIFY(!red->fill.isNone() && !red->stroke.isNone() && red->stroke.width == 2.0);
}


// ---- floating tables

// A table that says it is set at a place (w:tblpPr) has its place: from the text, the margin, or the page, by an offset or at the start, middle, or end, and the room the text
// keeps from it; a table that says nothing of the kind has none; and the file comes back as it was.
void WriteTests::aTableSetAtAPlaceHasItsPlace()
{
    const QByteArray pr = "<w:tblpPr w:leftFromText=\"187\" w:rightFromText=\"200\" w:topFromText=\"40\" w:bottomFromText=\"60\" w:vertAnchor=\"page\" w:horzAnchor=\"margin\" w:tblpXSpec=\"right\" w:tblpY=\"1440\"/>"
                          "<w:tblOverlap w:val=\"never\"/><w:tblW w:w=\"0\" w:type=\"auto\"/>";
    const QByteArray floating = tableX(rowX(cellX(cellP("a")) + cellX(cellP("b"))), 2, 1600, pr), inText = tableX(rowX(cellX(cellP("c"))), 1, 1600);
    const QByteArray zip = makeDocx(plainP("Before") + floating + plainP("After") + inText + plainP("End"), kStyles);
    auto doc = open(zip);
    QVERIFY(doc);
    QVector<std::optional<flow::TableData::Place>> places;
    int lastKey = 0;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
        const int key = b.blockFormat().intProperty(flow::fp::Cell);
        if (key && key != lastKey) {
            const flow::CellRecord *rec = doc->flow().tables.get(key);
            if (places.isEmpty() || (places.size() == 1 && rec->table.get() != doc->flow().tables.get(lastKey)->table.get())) places << rec->table->place;
        }
        lastKey = key ? key : lastKey;
    }
    QCOMPARE(int(places.size()), 2);
    QVERIFY(places[0].has_value() && !places[1].has_value());
    const flow::TableData::Place &p = *places[0];
    using Place = flow::TableData::Place;
    QVERIFY(p.across == Place::From::Margin && p.down == Place::From::Page);
    QVERIFY(p.acrossSpec == Place::Spec::End && p.downSpec == Place::Spec::None);
    QCOMPARE(p.y, 72.0);
    QCOMPARE(p.x, 0.0);
    QCOMPARE(p.room.left(), 9.35);
    QCOMPARE(p.room.right(), 10.0);
    QCOMPARE(p.room.top(), 2.0);
    QCOMPARE(p.room.bottom(), 3.0);
    const QByteArray saved = doc->toDocx();
    QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
    QVERIFY(documentXml(saved).contains("w:tblpPr"));
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(documentXml(back->toDocx()), documentXml(saved));
}


// A table whose every paragraph is in one frame that nothing sits beside is set at the frame's place, as a table that says so is; one whose frame has the text going round it, or
// whose paragraphs are not all framed, is a table in the text.
void WriteTests::aTableOfFramedParagraphsHasTheFramesPlace()
{
    const QByteArray frame = "<w:framePr w:hSpace=\"180\" w:wrap=\"notBeside\" w:vAnchor=\"text\" w:hAnchor=\"page\" w:x=\"1701\" w:y=\"471\"/>";
    auto framed = [&](const QByteArray &text, const QByteArray &fr) { return "<w:p><w:pPr>" + fr + "</w:pPr><w:r><w:t>" + text + "</w:t></w:r></w:p>"; };
    auto placeOf = [&](const QByteArray &table) {
        auto doc = open(makeDocx(plainP("Before") + table + plainP("After"), kStyles));
        std::optional<flow::TableData::Place> place;
        if (!doc) return std::make_pair(false, place);
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (const int key = b.blockFormat().intProperty(flow::fp::Cell)) {
                place = doc->flow().tables.get(key)->table->place;
                break;
            }
        return std::make_pair(true, place);
    };
    const auto all = placeOf(tableX(rowX(cellX(framed("a", frame)) + cellX(framed("b", frame))), 2, 1600));
    QVERIFY(all.first && all.second.has_value());
    using Place = flow::TableData::Place;
    QVERIFY(all.second->across == Place::From::Page && all.second->down == Place::From::Text);
    QCOMPARE(all.second->x, 85.05);
    QCOMPARE(all.second->y, 23.55);
    QCOMPARE(all.second->room.left(), 9.0);
    QVERIFY(!placeOf(tableX(rowX(cellX(framed("a", frame)) + cellX(cellP("b"))), 2, 1600)).second.has_value());
    QByteArray around = frame;
    around.replace("notBeside", "around");
    QVERIFY(!placeOf(tableX(rowX(cellX(framed("a", around)) + cellX(framed("b", around))), 2, 1600)).second.has_value());
}
