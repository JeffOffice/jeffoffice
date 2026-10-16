// Tests of libs/office: DrawingML text bodies.
#include "officehelpers.h"

class OfficeTextTests : public QObject {
    Q_OBJECT

    // The namespace declarations of a fragment; the strict forms may be given.
    static QString declarations(const QString &a = QString(ns::A), const QString &p = QString(ns::P), const QString &r = QString(ns::R))
    {
        return QStringLiteral("xmlns:a=\"%1\" xmlns:p=\"%2\" xmlns:r=\"%3\"").arg(a, p, r);
    }

    // A text body read into a story. `paragraphs` is the XML of its a:p
    // elements; `layers` are the children of one a:lstStyle each, the lowest
    // priority first (the stack the shape would hand over). `lstStyle` is the
    // body's own.
    struct Body {
        std::vector<std::unique_ptr<QDomDocument>> parsed;
        QTextDocument doc;
        DrawingContext ctx;
        TextStyleStack styles;

        QDomElement element(const QString &xml)
        {
            parsed.push_back(std::make_unique<QDomDocument>());
            QString error;
            if (!parseXml(xml.toUtf8(), parsed.back().get(), &error)) qWarning("bad test XML: %s", qPrintable(error));
            return parsed.back()->documentElement();
        }

        explicit Body(const QString &paragraphs, const QStringList &layers = {}, const QString &lstStyle = QString(), const Package *package = nullptr,
                      const Theme &theme = colorTestTheme(), const QString &decl = declarations())
        {
            ctx.theme = theme;
            ctx.package = package;
            if (package) ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
            for (const QString &layer : layers) styles.append(element(QStringLiteral("<a:lstStyle %1>%2</a:lstStyle>").arg(decl, layer)));
            readTextBody(element(QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle>%2</a:lstStyle>%3</p:txBody>").arg(decl, lstStyle, paragraphs)),
                         &doc, ctx, styles);
        }

        // The blocks' text, joined by line feeds: toPlainText turns a line separator into one too.
        QString text() const
        {
            QStringList lines;
            for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) lines << b.text();
            return lines.join(QLatin1Char('\n'));
        }
        QTextBlock block(int n = 0) const { return doc.findBlockByNumber(n); }
        QTextBlockFormat blockFormat(int n = 0) const { return block(n).blockFormat(); }
        // The character format of the letter at `offset` in the block.
        QTextCharFormat charAt(int n = 0, int offset = 0) const
        {
            QTextCursor c(block(n));
            c.setPosition(block(n).position() + offset + 1);
            return c.charFormat();
        }
    };
    static QString para(const QString &runs, const QString &pPr = QString())
    {
        return QStringLiteral("<a:p>%1%2</a:p>").arg(pPr, runs);
    }

private Q_SLOTS:
    void textBodyReadsParagraphsAndRunsExactly()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>Hello </a:t></a:r><a:r><a:t>world</a:t></a:r></a:p>"
                              "<a:p><a:r><a:t>  Second\there  </a:t></a:r></a:p>"));
        QCOMPARE(b.doc.blockCount(), 2);
        QCOMPARE(b.text(), QStringLiteral("Hello world\n  Second\there  "));
    }

    void textBodyWithoutParagraphsIsOneEmptyParagraph()
    {
        Body none{QString()};
        QCOMPARE(none.doc.blockCount(), 1);
        QVERIFY(none.doc.isEmpty());
        // Reading replaces what the story held, and a missing element or
        // story is no crash.
        QTextDocument doc;
        doc.setPlainText(QStringLiteral("old\ntext"));
        readTextBody(QDomElement(), &doc, DrawingContext());
        QCOMPARE(doc.blockCount(), 1);
        QVERIFY(doc.isEmpty());
        readTextBody(QDomElement(), nullptr, DrawingContext());
    }

    void textBodyReadsTheStrictNamespaces()
    {
        Body b(QStringLiteral("<a:p><a:r><a:rPr sz=\"2000\" b=\"1\"/><a:t>strict</a:t></a:r></a:p>"), {}, QString(), nullptr, colorTestTheme(),
               declarations(QString(ns::AStrict), QString(ns::PStrict), QString(ns::RStrict)));
        QCOMPARE(b.text(), QStringLiteral("strict"));
        QCOMPARE(b.charAt().fontPointSize(), 20.0);
        QCOMPARE(b.charAt().fontWeight(), int(QFont::Bold));
    }

    void textHasThePresentationProgramsDefaultSize()
    {
        Body b(para(QStringLiteral("<a:r><a:t>x</a:t></a:r>")));
        QCOMPARE(b.charAt().fontPointSize(), 18.0);
    }

    void textInheritsFromThreeStyleLayers()
    {
        // Lowest first: a size, italic, and a theme color; then a bigger
        // size; then bold. The first run adds a color of its own, the second
        // its own size and no bold.
        const QStringList layers = {
            QStringLiteral("<a:lvl1pPr><a:defRPr sz=\"1800\" i=\"1\" b=\"0\"><a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></a:defRPr></a:lvl1pPr>"),
            QStringLiteral("<a:lvl1pPr><a:defRPr sz=\"2400\"/></a:lvl1pPr>"),
            QStringLiteral("<a:lvl1pPr><a:defRPr b=\"1\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("one"), QString(), solid(QStringLiteral("FF0000"))) + run(QStringLiteral(" two"), QStringLiteral("sz=\"3000\" b=\"0\""))), layers);
        const QTextCharFormat one = b.charAt(0, 0), two = b.charAt(0, 5);
        QCOMPARE(one.fontPointSize(), 24.0);
        QCOMPARE(one.fontWeight(), int(QFont::Bold));
        QVERIFY(one.fontItalic());
        QCOMPARE(one.foreground().color(), QColor(0xFF, 0, 0));
        QCOMPARE(one.stringProperty(jp::tp::ColorRefP), QStringLiteral("#FF0000"));
        QCOMPARE(two.fontPointSize(), 30.0);
        QVERIFY(two.fontWeight() != QFont::Bold);
        QVERIFY(two.fontItalic());
        QCOMPARE(two.foreground().color(), QColor(0x44, 0x72, 0xC4));
        QCOMPARE(two.stringProperty(jp::tp::ColorRefP), QStringLiteral("#4472C4"));
    }

    void textTakesTheDefaultsOfAListOnlyWhereTheLevelsLeaveGaps()
    {
        // a:defPPr is below the level of its own layer: it fills in a size
        // the level doesn't give, and the level's wins where both say.
        const QStringList layers = {
            QStringLiteral("<a:defPPr><a:defRPr sz=\"1000\" i=\"1\"/></a:defPPr><a:lvl2pPr><a:defRPr sz=\"2200\"/></a:lvl2pPr>")};
        Body b(para(run(QStringLiteral("a"))) + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr lvl=\"1\"/>")), layers);
        QCOMPARE(b.charAt(0).fontPointSize(), 10.0);
        QVERIFY(b.charAt(0).fontItalic());
        QCOMPARE(b.charAt(1).fontPointSize(), 22.0);
        QVERIFY(b.charAt(1).fontItalic());
    }

    void textBodysOwnListStyleComesFirst()
    {
        const QStringList layers = {QStringLiteral("<a:lvl1pPr algn=\"r\"><a:defRPr sz=\"2400\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("x"))), layers, QStringLiteral("<a:lvl1pPr algn=\"ctr\"><a:defRPr sz=\"4000\"/></a:lvl1pPr>"));
        QCOMPARE(b.charAt().fontPointSize(), 40.0);
        QCOMPARE(int(b.blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignHCenter));
        // Handed over as the top of the stack already, it reads the same.
        Body again{QString()};
        const QDomElement body = again.element(QStringLiteral("<p:txBody %1><a:bodyPr/><a:lstStyle><a:lvl1pPr><a:defRPr sz=\"3600\"/></a:lvl1pPr></a:lstStyle>%2</p:txBody>")
                                                   .arg(declarations(), para(run(QStringLiteral("x")))));
        readTextBody(body, &again.doc, again.ctx, TextStyleStack{body.firstChildElement().nextSiblingElement()});
        QCOMPARE(again.charAt().fontPointSize(), 36.0);
    }

    void textReadsAStackShapedLikeAPresentations()
    {
        // The presentation's default text style, a master's body style, and a
        // layout placeholder's list style, lowest first.
        const QStringList layers = {
            QStringLiteral("<a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr><a:lvl1pPr marL=\"0\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\">"
                           "<a:defRPr sz=\"1800\" kern=\"1200\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/>"
                           "<a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl1pPr>"),
            QStringLiteral("<a:lvl1pPr marL=\"228600\" indent=\"-228600\"><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPts val=\"1000\"/></a:spcBef>"
                           "<a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl1pPr>"
                           "<a:lvl2pPr marL=\"685800\" indent=\"-228600\"><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPts val=\"500\"/></a:spcBef>"
                           "<a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2400\"/></a:lvl2pPr>"),
            QStringLiteral("<a:lvl1pPr><a:defRPr b=\"1\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("Heading"), QString(), QStringLiteral("<a:latin typeface=\"+mj-lt\"/>"))) + para(run(QStringLiteral("Detail")), QStringLiteral("<a:pPr lvl=\"1\"/>")), layers);
        QCOMPARE(b.charAt(0).fontPointSize(), 28.0);
        QCOMPARE(b.charAt(0).fontWeight(), int(QFont::Bold));
        QCOMPARE(b.charAt(0).stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QCOMPARE(b.charAt(0).property(jp::tp::KernAbove).toDouble(), 12.0);
        QCOMPARE(b.charAt(0).stringProperty(jp::tp::Language), QStringLiteral("en-US"));
        QCOMPARE(b.charAt(0).stringProperty(jp::tp::ColorRefP), QStringLiteral("#101010"));
        QCOMPARE(b.blockFormat(0).leftMargin(), 18.0);
        QCOMPARE(b.blockFormat(0).textIndent(), -18.0);
        QCOMPARE(b.blockFormat(0).topMargin(), 10.0);
        QCOMPARE(b.blockFormat(0).lineHeight(), 90.0);
        QVERIFY(b.block(0).textList());
        // The level below has its own size and margins, and nothing of the first layer's bold.
        QCOMPARE(b.charAt(1).fontPointSize(), 24.0);
        QVERIFY(b.charAt(1).fontWeight() != QFont::Bold);
        QCOMPARE(b.charAt(1).stringProperty(jp::tp::ThemeFont), QString());
        QCOMPARE(b.blockFormat(1).leftMargin(), 54.0);
        QCOMPARE(b.blockFormat(1).topMargin(), 5.0);
        QCOMPARE(b.block(1).textList()->format().intProperty(jp::tp::ListLevel), 1);
    }

    void textLevelsHaveTheirOwnIndentsAndBullets()
    {
        const QStringList layers = {QStringLiteral(
            "<a:lvl1pPr marL=\"0\" indent=\"0\"><a:buNone/><a:defRPr sz=\"3200\"/></a:lvl1pPr>"
            "<a:lvl2pPr marL=\"457200\" indent=\"-285750\"><a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl2pPr>"
            "<a:lvl3pPr marL=\"914400\" indent=\"-228600\"><a:buFont typeface=\"Wingdings\"/><a:buChar char=\"&#167;\"/></a:lvl3pPr>")};
        Body b(para(run(QStringLiteral("one"))) + para(run(QStringLiteral("two")), QStringLiteral("<a:pPr lvl=\"1\"/>"))
                   + para(run(QStringLiteral("three")), QStringLiteral("<a:pPr lvl=\"2\"/>")),
               layers);
        QCOMPARE(b.doc.blockCount(), 3);
        // Level 1: no bullet; a block at level 0.
        QVERIFY(!b.block(0).textList());
        QCOMPARE(b.blockFormat(0).leftMargin(), 0.0);
        QCOMPARE(b.blockFormat(0).textIndent(), 0.0);
        QCOMPARE(b.blockFormat(0).intProperty(jp::tp::ListLevel), 0);
        QCOMPARE(b.charAt(0).fontPointSize(), 32.0);
        // Level 2: a bullet in Arial, the text 36 pt in, the bullet 22.5 pt before it.
        QTextList *two = b.block(1).textList();
        QVERIFY(two);
        QCOMPARE(two->format().style(), QTextListFormat::ListDisc);
        QCOMPARE(two->format().stringProperty(jp::tp::BulletChar), QString(QChar(0x2022)));
        QCOMPARE(two->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Arial"));
        QCOMPARE(two->format().intProperty(jp::tp::ListLevel), 1);
        QCOMPARE(b.blockFormat(1).intProperty(jp::tp::ListLevel), 1);
        QCOMPARE(b.blockFormat(1).leftMargin(), 36.0);
        QCOMPARE(b.blockFormat(1).textIndent(), -22.5);
        QCOMPARE(b.charAt(1).fontPointSize(), 28.0);
        // Level 3: another mark in another font, indented further; its size is the list's (none) so the default.
        QTextList *three = b.block(2).textList();
        QVERIFY(three);
        QCOMPARE(three->format().stringProperty(jp::tp::BulletChar), QString(QChar(0xA7)));
        QCOMPARE(three->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Wingdings"));
        QCOMPARE(three->format().intProperty(jp::tp::ListLevel), 2);
        QCOMPARE(b.blockFormat(2).leftMargin(), 72.0);
        QCOMPARE(b.blockFormat(2).textIndent(), -18.0);
        QCOMPARE(b.charAt(2).fontPointSize(), 18.0);
    }

    void textBulletsInheritTheirPartsSeparately()
    {
        // The layer gives the color and the size; the paragraph the mark and
        // the font; the next paragraph turns the bullet off, then follows the
        // text's color and size again.
        const QStringList layers = {QStringLiteral("<a:lvl1pPr><a:buClr><a:srgbClr val=\"00FF00\"/></a:buClr><a:buSzPct val=\"50000\"/></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr><a:buFont typeface=\"Courier New\"/><a:buChar char=\"-\"/></a:pPr>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr><a:buNone/></a:pPr>"))
                   + para(run(QStringLiteral("c"), QStringLiteral("sz=\"2000\"")),
                          QStringLiteral("<a:pPr><a:buClrTx/><a:buSzPts val=\"1500\"/><a:buFontTx/><a:buChar char=\"*\"/></a:pPr>"))
                   + para(run(QStringLiteral("d"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr><a:buClrTx/><a:buSzTx/><a:buBlip/></a:pPr>")),
               layers);
        const QTextListFormat a = b.block(0).textList()->format();
        QCOMPARE(a.stringProperty(jp::tp::BulletChar), QStringLiteral("-"));
        QCOMPARE(a.stringProperty(jp::tp::BulletFont), QStringLiteral("Courier New"));
        QCOMPARE(jp::ColorRef::fromString(a.stringProperty(jp::tp::BulletColor)).rgbValue(), QColor(0, 255, 0));
        QCOMPARE(a.property(jp::tp::BulletSize).toDouble(), 10.0);   // half of the first run's 20 pt
        QVERIFY(!b.block(1).textList());
        const QTextListFormat c = b.block(2).textList()->format();
        QCOMPARE(c.stringProperty(jp::tp::BulletChar), QStringLiteral("*"));
        QVERIFY(!c.hasProperty(jp::tp::BulletColor));
        QVERIFY(!c.hasProperty(jp::tp::BulletFont));
        QCOMPARE(c.property(jp::tp::BulletSize).toDouble(), 15.0);
        // A picture bullet is a plain one.
        const QTextListFormat d = b.block(3).textList()->format();
        QCOMPARE(d.style(), QTextListFormat::ListDisc);
        QVERIFY(!d.hasProperty(jp::tp::BulletChar));
        QVERIFY(!d.hasProperty(jp::tp::BulletColor));
        QVERIFY(!d.hasProperty(jp::tp::BulletSize));
    }

    void textBulletFontsMayNameTheThemeFonts()
    {
        Theme theme;
        QVERIFY(readTheme(handWrittenTheme(), &theme));
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr><a:buFont typeface=\"+mj-lt\"/><a:buChar char=\"o\"/></a:pPr>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr><a:buFont typeface=\"+mn-lt\"/><a:buChar char=\"o\"/></a:pPr>")),
               {}, QString(), nullptr, theme);
        QCOMPARE(b.block(0).textList()->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Georgia"));
        QCOMPARE(b.block(1).textList()->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Verdana"));
    }

    void textBulletsDrawnAtTheirIndents()
    {
        // The hanging bullet of a one-level list, laid out: the text starts
        // at marL, the bullet an indent before it.
        Body b(para(run(QStringLiteral("alpha beta"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr marL=\"457200\" indent=\"-228600\"><a:buChar char=\"&#8226;\"/></a:pPr>")));
        jp::FrameSpec fs;
        fs.size = QSizeF(400, 200);
        fs.insets = QMarginsF(0, 0, 0, 0);
        jp::StoryLayout lay;
        lay.build(&b.doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        QVERIFY2(qAbs(lines[0].rect.x() - 36) < 0.01, qPrintable(QString::number(lines[0].rect.x())));
    }

    void textAutonumbersContinueUntilInterrupted()
    {
        auto num = [](const QString &text, int lvl, const QString &bu) {
            return para(run(text), QStringLiteral("<a:pPr lvl=\"%1\">%2</a:pPr>").arg(lvl).arg(bu));
        };
        const QString arabic = QStringLiteral("<a:buAutoNum type=\"arabicPeriod\"/>");
        const QString arabicFrom5 = QStringLiteral("<a:buAutoNum type=\"arabicPeriod\" startAt=\"5\"/>");
        const QString alpha = QStringLiteral("<a:buAutoNum type=\"alphaLcParenR\"/>");
        Body b(num(QStringLiteral("a"), 0, arabic) + num(QStringLiteral("b"), 0, arabic)
                   + num(QStringLiteral("c"), 1, alpha) + num(QStringLiteral("d"), 1, alpha)   // nested, with their own count
                   + num(QStringLiteral("e"), 0, arabic)                                        // goes on at 3
                   + num(QStringLiteral("f"), 0, QStringLiteral("<a:buNone/>"))                 // interrupts
                   + num(QStringLiteral("g"), 0, arabic)                                        // starts again at 1
                   + num(QStringLiteral("h"), 0, arabicFrom5)                                   // another start, another count
                   + num(QStringLiteral("i"), 1, alpha)                                         // the outer item before it restarts the inner
                   + num(QStringLiteral("j"), 0, arabicFrom5),
               {});
        const QStringList want = {QStringLiteral("1."), QStringLiteral("2."), QStringLiteral("a)"), QStringLiteral("b)"), QStringLiteral("3."), QString(),
                                  QStringLiteral("1."), QStringLiteral("5."), QStringLiteral("a)"), QStringLiteral("6.")};
        QCOMPARE(b.doc.blockCount(), int(want.size()));
        for (int i = 0; i < want.size(); ++i) {
            QTextList *l = b.block(i).textList();
            QCOMPARE(l ? l->itemText(b.block(i)) : QString(), want[i]);
        }
    }

    void textEmptyParagraphsHaveNoBulletAndKeepTheNumbering()
    {
        const QString pPr = QStringLiteral("<a:pPr><a:buAutoNum type=\"arabicPeriod\"/></a:pPr>");
        Body b(para(run(QStringLiteral("a")), pPr) + QStringLiteral("<a:p>") + pPr + QStringLiteral("<a:endParaRPr sz=\"1800\"/></a:p>") + para(run(QStringLiteral("b")), pPr));
        QVERIFY(b.block(0).textList());
        QVERIFY(!b.block(1).textList());
        QCOMPARE(b.block(2).textList()->itemText(b.block(2)), QStringLiteral("2."));
    }

    // An empty paragraph the file marks as having no bullet says so (so that saving can keep it); one that has text, one that says nothing,
    // and one with a bullet in the file do not.
    void textEmptyParagraphsMarkedNoBulletSaySo()
    {
        const QString none = QStringLiteral("<a:pPr><a:buNone/></a:pPr>");
        const QString bullet = QStringLiteral("<a:pPr><a:buChar char=\"-\"/></a:pPr>");
        const QString end = QStringLiteral("<a:endParaRPr sz=\"1800\"/></a:p>");
        Body b(para(run(QStringLiteral("a")), none) + QStringLiteral("<a:p>") + none + end + QStringLiteral("<a:p>") + end + QStringLiteral("<a:p>") + bullet + end);
        QCOMPARE(b.doc.blockCount(), 4);
        QVERIFY(!b.block(0).blockFormat().boolProperty(jp::office::kNoBulletParagraph));
        QVERIFY(b.block(1).blockFormat().boolProperty(jp::office::kNoBulletParagraph));
        QVERIFY(!b.block(2).blockFormat().boolProperty(jp::office::kNoBulletParagraph));
        QVERIFY(!b.block(3).blockFormat().boolProperty(jp::office::kNoBulletParagraph));
        for (int i = 1; i < 4; ++i) QVERIFY(!b.block(i).textList());
    }

    void textAutonumberSchemesBecomeListStyles()
    {
        using S = QTextListFormat;
        struct Row { const char *type; S::Style style; const char *prefix; const char *suffix; int numberFormat; };
        const Row rows[] = {
            {"arabicPeriod", S::ListDecimal, "", ".", 1},      {"arabicParenR", S::ListDecimal, "", ")", 6},
            {"arabicParenBoth", S::ListDecimal, "(", ")", 7},  {"arabicPlain", S::ListDecimal, "", "", 0},
            {"alphaLcPeriod", S::ListLowerAlpha, "", ".", 2},  {"alphaLcParenR", S::ListLowerAlpha, "", ")", 0},
            {"alphaLcParenBoth", S::ListLowerAlpha, "(", ")", 0}, {"alphaUcPeriod", S::ListUpperAlpha, "", ".", 3},
            {"alphaUcParenR", S::ListUpperAlpha, "", ")", 0},  {"alphaUcParenBoth", S::ListUpperAlpha, "(", ")", 0},
            {"romanLcPeriod", S::ListLowerRoman, "", ".", 4},  {"romanLcParenR", S::ListLowerRoman, "", ")", 0},
            {"romanLcParenBoth", S::ListLowerRoman, "(", ")", 0}, {"romanUcPeriod", S::ListUpperRoman, "", ".", 5},
            {"romanUcParenR", S::ListUpperRoman, "", ")", 0},  {"romanUcParenBoth", S::ListUpperRoman, "(", ")", 0},
            // The ones of other scripts count in digits, with the same marks.
            {"circleNumDbPlain", S::ListDecimal, "", "", 0},   {"circleNumWdBlackPlain", S::ListDecimal, "", "", 0},
            {"circleNumWdWhitePlain", S::ListDecimal, "", "", 0}, {"arabicDbPeriod", S::ListDecimal, "", ".", 1},
            {"arabicDbPlain", S::ListDecimal, "", "", 0},      {"ea1ChsPeriod", S::ListDecimal, "", ".", 1},
            {"ea1ChsPlain", S::ListDecimal, "", "", 0},        {"ea1ChtPeriod", S::ListDecimal, "", ".", 1},
            {"ea1ChtPlain", S::ListDecimal, "", "", 0},        {"ea1JpnChsDbPeriod", S::ListDecimal, "", ".", 1},
            {"ea1JpnKorPlain", S::ListDecimal, "", "", 0},     {"ea1JpnKorPeriod", S::ListDecimal, "", ".", 1},
            {"arabic1Minus", S::ListDecimal, "", "", 0},       {"arabic2Minus", S::ListDecimal, "", "", 0},
            {"hebrew2Minus", S::ListDecimal, "", "", 0},       {"thaiAlphaPeriod", S::ListLowerAlpha, "", ".", 2},
            {"thaiAlphaParenR", S::ListLowerAlpha, "", ")", 0}, {"thaiAlphaParenBoth", S::ListLowerAlpha, "(", ")", 0},
            {"thaiNumPeriod", S::ListDecimal, "", ".", 1},     {"thaiNumParenR", S::ListDecimal, "", ")", 6},
            {"thaiNumParenBoth", S::ListDecimal, "(", ")", 7}, {"hindiAlphaPeriod", S::ListLowerAlpha, "", ".", 2},
            {"hindiAlpha1Period", S::ListLowerAlpha, "", ".", 2}, {"hindiNumPeriod", S::ListDecimal, "", ".", 1},
            {"hindiNumParenR", S::ListDecimal, "", ")", 6},
            // A scheme this edition doesn't know counts as the usual one.
            {"fromTheFuture", S::ListDecimal, "", ".", 1},
        };
        for (const Row &r : rows) {
            Body b(para(run(QStringLiteral("x")), QStringLiteral("<a:pPr><a:buAutoNum type=\"%1\"/></a:pPr>").arg(QLatin1String(r.type))));
            QTextList *l = b.block(0).textList();
            QVERIFY2(l, r.type);
            const QTextListFormat f = l->format();
            QVERIFY2(f.style() == r.style, r.type);
            QVERIFY2(f.numberPrefix() == QLatin1String(r.prefix), r.type);
            QVERIFY2(f.numberSuffix() == QLatin1String(r.suffix), r.type);
            QVERIFY2(f.intProperty(jp::tp::NumberFormat) == r.numberFormat, r.type);
            QVERIFY2(!jp::isBulletList(f.style()), r.type);
        }
    }

    void textAutonumberStartsAtItsNumber()
    {
        Body b(para(run(QStringLiteral("x")), QStringLiteral("<a:pPr><a:buAutoNum type=\"romanUcPeriod\" startAt=\"4\"/></a:pPr>"))
               + para(run(QStringLiteral("y")), QStringLiteral("<a:pPr><a:buAutoNum type=\"romanUcPeriod\" startAt=\"4\"/></a:pPr>")));
        QCOMPARE(b.block(0).textList()->itemText(b.block(0)), QStringLiteral("IV."));
        QCOMPARE(b.block(1).textList()->itemText(b.block(1)), QStringLiteral("V."));
    }

    void textParagraphsTakeAlignmentIndentsAndSpacing()
    {
        const QStringList layers = {QStringLiteral("<a:lvl1pPr marL=\"114300\" indent=\"-57150\" algn=\"just\"><a:spcBef><a:spcPts val=\"600\"/></a:spcBef></a:lvl1pPr>")};
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"2000\"")), QStringLiteral("<a:pPr marR=\"127000\"/>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr algn=\"ctr\" marL=\"0\" indent=\"254000\"><a:spcBef><a:spcPct val=\"50000\"/></a:spcBef>"
                                                                   "<a:spcAft><a:spcPts val=\"1250\"/></a:spcAft></a:pPr>"))
                   + para(run(QStringLiteral("c")), QStringLiteral("<a:pPr algn=\"r\"/>")) + para(run(QStringLiteral("d")), QStringLiteral("<a:pPr algn=\"l\"/>")),
               layers);
        QCOMPARE(b.blockFormat(0).leftMargin(), 9.0);
        QCOMPARE(b.blockFormat(0).textIndent(), -4.5);
        QCOMPARE(b.blockFormat(0).rightMargin(), 10.0);
        QCOMPARE(b.blockFormat(0).topMargin(), 6.0);
        QCOMPARE(b.blockFormat(0).bottomMargin(), 0.0);
        QCOMPARE(int(b.blockFormat(0).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(!b.blockFormat(0).boolProperty(jp::tp::Distribute));
        QCOMPARE(b.blockFormat(1).leftMargin(), 0.0);
        QCOMPARE(b.blockFormat(1).textIndent(), 20.0);
        // A percentage is of the line: 50% of one 18 pt line, 1.2 times the size.
        QCOMPARE(b.blockFormat(1).topMargin(), 0.5 * 1.2 * 18);
        QCOMPARE(b.blockFormat(1).bottomMargin(), 12.5);
        QCOMPARE(int(b.blockFormat(1).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignHCenter));
        QCOMPARE(int(b.blockFormat(2).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
        QCOMPARE(int(b.blockFormat(3).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignLeft));
    }

    void textPercentSpacingIsOfTheParagraphsLargestRun()
    {
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"1000\"")) + run(QStringLiteral("b"), QStringLiteral("sz=\"3000\"")),
                    QStringLiteral("<a:pPr><a:spcBef><a:spcPct val=\"100000\"/></a:spcBef><a:spcAft><a:spcPct val=\"50000\"/></a:spcAft></a:pPr>")));
        QCOMPARE(b.blockFormat().topMargin(), 1.2 * 30);
        QCOMPARE(b.blockFormat().bottomMargin(), 0.5 * 1.2 * 30);
    }

    void textDistributedAlignmentSpreadsTheLastLineToo()
    {
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr algn=\"dist\"/>")) + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr algn=\"justLow\"/>")));
        QCOMPARE(int(b.blockFormat(0).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(b.blockFormat(0).boolProperty(jp::tp::Distribute));
        QCOMPARE(int(b.blockFormat(1).alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(!b.blockFormat(1).boolProperty(jp::tp::Distribute));
    }

    void textLineSpacingIsProportionalOrExact()
    {
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc></a:pPr>"))
                   + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr><a:lnSpc><a:spcPts val=\"2400\"/></a:lnSpc></a:pPr>"))
                   + para(run(QStringLiteral("c"))),
               {QStringLiteral("<a:lvl1pPr><a:lnSpc><a:spcPct val=\"120000\"/></a:lnSpc></a:lvl1pPr>")});
        QCOMPARE(b.blockFormat(0).lineHeightType(), int(QTextBlockFormat::ProportionalHeight));
        QCOMPARE(b.blockFormat(0).lineHeight(), 90.0);
        QCOMPARE(b.blockFormat(1).lineHeightType(), int(QTextBlockFormat::FixedHeight));
        QCOMPARE(b.blockFormat(1).lineHeight(), 24.0);
        QCOMPARE(b.blockFormat(2).lineHeightType(), int(QTextBlockFormat::ProportionalHeight));
        QCOMPARE(b.blockFormat(2).lineHeight(), 120.0);
    }

    void textTabStopsAreReadWithTheirAlignment()
    {
        Body b(para(run(QStringLiteral("a\tb")),
                    QStringLiteral("<a:pPr><a:tabLst><a:tab pos=\"914400\" algn=\"l\"/><a:tab pos=\"1828800\" algn=\"r\"/><a:tab pos=\"2743200\" algn=\"ctr\"/>"
                                   "<a:tab pos=\"3657600\" algn=\"dec\"/><a:tab algn=\"l\"/></a:tabLst></a:pPr>")));
        const QList<QTextOption::Tab> tabs = b.blockFormat().tabPositions();
        QCOMPARE(tabs.size(), 4);
        QCOMPARE(tabs[0].position, 72.0);
        QCOMPARE(int(tabs[0].type), int(QTextOption::LeftTab));
        QCOMPARE(tabs[1].position, 144.0);
        QCOMPARE(int(tabs[1].type), int(QTextOption::RightTab));
        QCOMPARE(tabs[2].position, 216.0);
        QCOMPARE(int(tabs[2].type), int(QTextOption::CenterTab));
        QCOMPARE(tabs[3].position, 288.0);
        QCOMPARE(int(tabs[3].type), int(QTextOption::DelimiterTab));
        QCOMPARE(tabs[3].delimiter, QChar('.'));
    }

    void textDefaultTabSizeMakesStopsOnlyWhereATabIsUsed()
    {
        // 36 pt apart, from the last stop of its own on.
        Body b(para(run(QStringLiteral("a\tb")), QStringLiteral("<a:pPr defTabSz=\"457200\"/>"))
                   + para(run(QStringLiteral("no tab here")), QStringLiteral("<a:pPr defTabSz=\"457200\"/>"))
                   + para(run(QStringLiteral("a\tb")), QStringLiteral("<a:pPr defTabSz=\"457200\"><a:tabLst><a:tab pos=\"1000000\" algn=\"l\"/></a:tabLst></a:pPr>")));
        const QList<QTextOption::Tab> first = b.blockFormat(0).tabPositions();
        QVERIFY(first.size() >= 3);
        QCOMPARE(first[0].position, 36.0);
        QCOMPARE(first[1].position, 72.0);
        QCOMPARE(first[2].position, 108.0);
        QVERIFY(b.blockFormat(1).tabPositions().isEmpty());
        const QList<QTextOption::Tab> third = b.blockFormat(2).tabPositions();
        QVERIFY(third.size() >= 2);
        QCOMPARE(third[0].position, 1000000 / 12700.0);
        QVERIFY(third[1].position > third[0].position);
        QCOMPARE(std::fmod(third[1].position, 36.0), 0.0);
    }

    void textRightToLeftParagraphsKeepTheirSides()
    {
        Body b(para(run(QStringLiteral("a")), QStringLiteral("<a:pPr rtl=\"1\" algn=\"r\"/>")) + para(run(QStringLiteral("b")), QStringLiteral("<a:pPr rtl=\"1\" algn=\"l\"/>"))
               + para(run(QStringLiteral("c")), QStringLiteral("<a:pPr rtl=\"0\" algn=\"r\"/>")));
        QCOMPARE(b.blockFormat(0).layoutDirection(), Qt::RightToLeft);
        // The standard's left and right are the page's: the engine swaps a
        // right-to-left paragraph's, unless told they are absolute.
        QVERIFY(bool(b.blockFormat(0).alignment() & Qt::AlignRight));
        QVERIFY(bool(b.blockFormat(0).alignment() & Qt::AlignAbsolute));
        QVERIFY(bool(b.blockFormat(1).alignment() & Qt::AlignLeft));
        QVERIFY(bool(b.blockFormat(1).alignment() & Qt::AlignAbsolute));
        QVERIFY(b.blockFormat(2).layoutDirection() != Qt::RightToLeft);
        QVERIFY(bool(b.blockFormat(2).alignment() & Qt::AlignRight));
    }

    void textUnderlinesBecomeStylesAndKinds()
    {
        using U = QTextCharFormat;
        struct Row { const char *u; U::UnderlineStyle style; int kind; };
        const Row rows[] = {
            {"sng", U::SingleUnderline, 0},       {"words", U::SingleUnderline, 2},      {"dbl", U::SingleUnderline, 1},
            {"heavy", U::SingleUnderline, 4},     {"dotted", U::DotLine, 0},             {"dottedHeavy", U::DotLine, 4},
            {"dash", U::DashUnderline, 0},        {"dashHeavy", U::DashUnderline, 4},    {"dashLong", U::DashUnderline, 0},
            {"dashLongHeavy", U::DashUnderline, 4}, {"dotDash", U::DashDotLine, 0},      {"dotDashHeavy", U::DashDotLine, 4},
            {"dotDotDash", U::DashDotDotLine, 0}, {"dotDotDashHeavy", U::DashDotDotLine, 4}, {"wavy", U::WaveUnderline, 0},
            {"wavyHeavy", U::WaveUnderline, 4},   {"wavyDbl", U::WaveUnderline, 1},      {"none", U::NoUnderline, 0},
        };
        for (const Row &r : rows) {
            Body b(para(run(QStringLiteral("x"), QStringLiteral("u=\"%1\"").arg(QLatin1String(r.u)))));
            const QTextCharFormat f = b.charAt();
            QVERIFY2(f.underlineStyle() == r.style, r.u);
            QVERIFY2(f.intProperty(jp::tp::UnderlineKind) == r.kind, r.u);
        }
        // A run turns an underline from its layer off, and the layer's reaches a run that says nothing.
        Body b(para(run(QStringLiteral("a")) + run(QStringLiteral("b"), QStringLiteral("u=\"none\""))), {QStringLiteral("<a:lvl1pPr><a:defRPr u=\"sng\"/></a:lvl1pPr>")});
        QCOMPARE(b.charAt(0, 0).underlineStyle(), QTextCharFormat::SingleUnderline);
        QCOMPARE(b.charAt(0, 1).underlineStyle(), QTextCharFormat::NoUnderline);
    }

    void textStrikeBaselineSpacingCapsAndKerning()
    {
        Body b(para(run(QStringLiteral("a"), QStringLiteral("strike=\"sngStrike\"")) + run(QStringLiteral("b"), QStringLiteral("strike=\"dblStrike\""))
                    + run(QStringLiteral("c"), QStringLiteral("strike=\"noStrike\"")) + run(QStringLiteral("d"), QStringLiteral("baseline=\"30000\""))
                    + run(QStringLiteral("e"), QStringLiteral("baseline=\"-25000\"")) + run(QStringLiteral("f"), QStringLiteral("baseline=\"0\""))
                    + run(QStringLiteral("g"), QStringLiteral("spc=\"300\"")) + run(QStringLiteral("h"), QStringLiteral("spc=\"-50\""))
                    + run(QStringLiteral("i"), QStringLiteral("cap=\"all\"")) + run(QStringLiteral("j"), QStringLiteral("cap=\"small\""))
                    + run(QStringLiteral("k"), QStringLiteral("cap=\"none\"")) + run(QStringLiteral("l"), QStringLiteral("kern=\"1200\""))
                    + run(QStringLiteral("m"), QStringLiteral("kern=\"0\"")) + run(QStringLiteral("n"), QStringLiteral("lang=\"fr-CA\" noProof=\"1\""))),
               {QStringLiteral("<a:lvl1pPr><a:defRPr cap=\"all\"/></a:lvl1pPr>")});
        QCOMPARE(int(b.charAt(0, 0).fontCapitalization()), int(QFont::AllUppercase));   // from the layer
        QVERIFY(b.charAt(0, 0).fontStrikeOut());
        QVERIFY(b.charAt(0, 1).fontStrikeOut());
        QVERIFY(!b.charAt(0, 2).fontStrikeOut());
        QCOMPARE(b.charAt(0, 3).verticalAlignment(), QTextCharFormat::AlignSuperScript);
        QCOMPARE(b.charAt(0, 4).verticalAlignment(), QTextCharFormat::AlignSubScript);
        QCOMPARE(b.charAt(0, 5).verticalAlignment(), QTextCharFormat::AlignNormal);
        // Spacing is in points after each letter, as the engine's kerning is.
        QCOMPARE(b.charAt(0, 6).fontLetterSpacingType(), QFont::AbsoluteSpacing);
        QCOMPARE(b.charAt(0, 6).fontLetterSpacing(), 3.0);
        QCOMPARE(b.charAt(0, 7).fontLetterSpacing(), -0.5);
        QVERIFY(!b.charAt(0, 6).hasProperty(jp::tp::Tracking));
        QCOMPARE(int(b.charAt(0, 8).fontCapitalization()), int(QFont::AllUppercase));
        QCOMPARE(int(b.charAt(0, 9).fontCapitalization()), int(QFont::SmallCaps));
        QCOMPARE(int(b.charAt(0, 10).fontCapitalization()), int(QFont::MixedCase));
        QCOMPARE(b.charAt(0, 11).property(jp::tp::KernAbove).toDouble(), 12.0);
        QVERIFY(b.charAt(0, 12).hasProperty(jp::tp::KernAbove));
        QCOMPARE(b.charAt(0, 12).property(jp::tp::KernAbove).toDouble(), 0.0);
        QVERIFY(!b.charAt(0, 0).hasProperty(jp::tp::KernAbove));
        QCOMPARE(b.charAt(0, 13).stringProperty(jp::tp::Language), QStringLiteral("fr-CA"));
        QVERIFY(b.charAt(0, 13).boolProperty(jp::tp::NoProof));
        QVERIFY(b.charAt(0, 0).stringProperty(jp::tp::Language).isEmpty());
    }

    void textHighlightAndFillKinds()
    {
        Body b(para(run(QStringLiteral("a"), QString(), QStringLiteral("<a:highlight><a:srgbClr val=\"FFFF00\"/></a:highlight>"))
                    + run(QStringLiteral("b"), QString(), QStringLiteral("<a:solidFill><a:schemeClr val=\"tx1\"><a:alpha val=\"50000\"/></a:schemeClr></a:solidFill>"))
                    + run(QStringLiteral("c"), QString(), QStringLiteral("<a:gradFill><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"123456\"/></a:gs>"
                                                                         "<a:gs pos=\"100000\"><a:srgbClr val=\"FFFFFF\"/></a:gs></a:gsLst></a:gradFill>"))));
        QCOMPARE(b.charAt(0, 0).stringProperty(jp::tp::HighlightRefP), QStringLiteral("#FFFF00"));
        QVERIFY(b.charAt(0, 1).stringProperty(jp::tp::HighlightRefP).isEmpty());
        QVERIFY(qAbs(b.charAt(0, 1).foreground().color().alpha() - 128) <= 1);
        QVERIFY(qAbs(jp::ColorRef::fromString(b.charAt(0, 1).stringProperty(jp::tp::ColorRefP)).rgbValue().alpha() - 128) <= 1);
        QCOMPARE(b.charAt(0, 2).foreground().color(), QColor(0x12, 0x34, 0x56));
    }

    void textFontsAreThemeReferencesOrFamilies()
    {
        Theme theme;
        QVERIFY(readTheme(handWrittenTheme(), &theme));
        auto latin = [](const QString &typeface, const QString &more = QString()) {
            return QStringLiteral("<a:latin typeface=\"%1\"/>%2").arg(typeface, more);
        };
        Body b(para(run(QStringLiteral("a"), QString(), latin(QStringLiteral("+mj-lt")))
                    + run(QStringLiteral("b"), QString(), latin(QStringLiteral("+mn-lt")))
                    + run(QStringLiteral("c"), QString(), latin(QStringLiteral("Arial")))
                    + run(QStringLiteral("d"), QString(), latin(QStringLiteral("Arial"), QStringLiteral("<a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"Tahoma\"/>")))
                    + run(QStringLiteral("e"), QString(), QStringLiteral("<a:ea typeface=\"+mj-ea\"/>"))
                    + run(QStringLiteral("f"), QString(), QStringLiteral("<a:cs typeface=\"+mn-cs\"/>"))
                    + run(QStringLiteral("g"), QString(), QStringLiteral("<a:latin typeface=\"\"/>"))
                    + run(QStringLiteral("h"))),
               {QStringLiteral("<a:lvl1pPr><a:defRPr><a:latin typeface=\"Calibri\"/></a:defRPr></a:lvl1pPr>")}, QString(), nullptr, theme);
        auto families = [&](int i) { return b.charAt(0, i).fontFamilies().toStringList(); };
        // A theme font is no family: the engine takes the story's font scheme.
        QCOMPARE(b.charAt(0, 0).stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QVERIFY(!b.charAt(0, 0).hasProperty(QTextFormat::FontFamilies));
        QCOMPARE(b.charAt(0, 1).stringProperty(jp::tp::ThemeFont), QStringLiteral("minor"));
        QVERIFY(!b.charAt(0, 1).hasProperty(QTextFormat::FontFamilies));
        QCOMPARE(families(2), QStringList({QStringLiteral("Arial")}));
        QVERIFY(b.charAt(0, 2).stringProperty(jp::tp::ThemeFont).isEmpty());
        // The East Asian and complex-script fonts follow as fallbacks when they are families.
        QCOMPARE(families(3), QStringList({QStringLiteral("Arial"), QStringLiteral("Tahoma")}));
        // With no Latin font of its own anywhere above a run takes its layer's Calibri.
        QCOMPARE(families(4), QStringList({QStringLiteral("Calibri")}));
        QCOMPARE(families(5), QStringList({QStringLiteral("Calibri")}));
        QCOMPARE(families(6), QStringList({QStringLiteral("Calibri")}));
        QCOMPARE(families(7), QStringList({QStringLiteral("Calibri")}));
        // And the other way round: a theme font in a run replaces the layer's family.
        QVERIFY(!b.charAt(0, 0).fontFamilies().toStringList().contains(QStringLiteral("Calibri")));
    }

    void textThemeFontsOfEastAsianAndComplexScriptsSetTheThemeFont()
    {
        Body b(para(run(QStringLiteral("a"), QString(), QStringLiteral("<a:ea typeface=\"+mj-ea\"/>")) + run(QStringLiteral("b"), QString(), QStringLiteral("<a:cs typeface=\"+mn-cs\"/>"))
                    + run(QStringLiteral("c"), QString(), QStringLiteral("<a:ea typeface=\"SimSun\"/>"))));
        QCOMPARE(b.charAt(0, 0).stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QCOMPARE(b.charAt(0, 1).stringProperty(jp::tp::ThemeFont), QStringLiteral("minor"));
        // A family for the East Asian script alone is not the Latin text's font.
        QVERIFY(!b.charAt(0, 2).hasProperty(QTextFormat::FontFamilies));
    }

    void textSlideNumberIsThePageField()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>Slide </a:t></a:r><a:fld id=\"{B6F15528-21DE-4FAA-801E-634DDDAF4B2B}\" type=\"slidenum\"><a:rPr sz=\"1200\" b=\"1\"/>"
                              "<a:t>&#8249;#&#8250;</a:t></a:fld></a:p>"));
        QCOMPARE(b.text(), QStringLiteral("Slide ") + QString(QChar::ObjectReplacementCharacter));
        QCOMPARE(b.charAt(0, 6).stringProperty(jp::tp::Field), QStringLiteral("page"));
        QCOMPARE(b.charAt(0, 6).fontPointSize(), 12.0);
        QCOMPARE(b.charAt(0, 6).fontWeight(), int(QFont::Bold));
        QVERIFY(b.charAt(0, 0).stringProperty(jp::tp::Field).isEmpty());
        // Laid out on slide 7, it reads 7.
        jp::FrameSpec fs;
        fs.size = QSizeF(400, 100);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.ctx.pageNumber = 7;
        jp::StoryLayout lay;
        lay.build(&b.doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 1);
        QCOMPARE(lines[0].text, QStringLiteral("Slide 7"));
    }

    void textDateFieldsHaveTheirFormats()
    {
        // The names of days and months are the locale's.
        struct Restore {
            QLocale was;
            ~Restore() { QLocale::setDefault(was); }
        } restore;
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        struct Row { const char *type; const char *shown; };
        const Row rows[] = {
            {"datetime", "10/11/2026"},                     {"datetime1", "10/11/2026"},
            {"datetime2", "Sunday, October 11, 2026"},      {"datetime3", "11 October 2026"},
            {"datetime4", "October 11, 2026"},              {"datetime5", "11-Oct-26"},
            {"datetime6", "October 26"},                    {"datetime7", "Oct-26"},
            {"datetime8", "10/11/2026 4:07 PM"},            {"datetime9", "10/11/2026 4:07:09 PM"},
            {"datetime10", "16:07"},                        {"datetime11", "16:07:09"},
            {"datetime12", "4:07 PM"},                      {"datetime13", "4:07:09 PM"},
        };
        jp::FieldContext now;
        now.now = QDateTime(QDate(2026, 10, 11), QTime(16, 7, 9));
        for (const Row &r : rows) {
            Body b(QStringLiteral("<a:p><a:fld id=\"{00000000-0000-0000-0000-000000000000}\" type=\"%1\"><a:rPr/><a:t>whatever it said</a:t></a:fld></a:p>").arg(QLatin1String(r.type)));
            const QString code = b.charAt().stringProperty(jp::tp::Field);
            QVERIFY2(!code.isEmpty(), r.type);
            QVERIFY2(now.resolve(code) == QLatin1String(r.shown), qPrintable(QStringLiteral("%1: %2 (%3)").arg(QLatin1String(r.type), now.resolve(code), code)));
            QCOMPARE(b.text(), QString(QChar::ObjectReplacementCharacter));
        }
    }

    void textOtherFieldsShowWhatTheyHeld()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>[</a:t></a:r><a:fld id=\"{1}\" type=\"datetimeFigureOut\"><a:rPr/><a:t>Tuesday</a:t></a:fld>"
                              "<a:fld id=\"{2}\" type=\"somethingNew\"><a:t>cached</a:t></a:fld><a:fld id=\"{3}\"/><a:r><a:t>]</a:t></a:r></a:p>"));
        QCOMPARE(b.text(), QStringLiteral("[Tuesdaycached]"));
        for (int i = 0; i < b.doc.characterCount() - 1; ++i) QVERIFY(b.charAt(0, i).stringProperty(jp::tp::Field).isEmpty());
    }

    void textHyperlinksAreAnchorsToTheirExternalTargets()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addPart(QStringLiteral("/ppt/slides/slide2.xml"), "<p:sld/>", kSlideType);
        const QString slide = QStringLiteral("/ppt/slides/slide1.xml");
        QCOMPARE(w.addRelationship(slide, kRelBase + "hyperlink", QStringLiteral("https://example.com/a?b=1&c=2"), true), QStringLiteral("rId1"));
        QCOMPARE(w.addRelationship(slide, kRelBase + "slide", QStringLiteral("/ppt/slides/slide2.xml")), QStringLiteral("rId2"));
        Package p;
        QVERIFY(p.read(w.finish()));
        auto link = [](const QString &id, const QString &extra = QString()) {
            return QStringLiteral("<a:hlinkClick r:id=\"%1\" %2/>").arg(id, extra);
        };
        Body b(para(run(QStringLiteral("a"), QString(), solid(QStringLiteral("FF0000")) + link(QStringLiteral("rId1")))
                    + run(QStringLiteral("b"), QString(), solid(QStringLiteral("FF0000")) + link(QStringLiteral("rId2")))
                    + run(QStringLiteral("c"), QString(), link(QStringLiteral("rId9")))
                    + run(QStringLiteral("d"), QString(), link(QString()))
                    + run(QStringLiteral("e"))),
               {}, QString(), &p);
        const QTextCharFormat a = b.charAt(0, 0);
        QVERIFY(a.isAnchor());
        QCOMPARE(a.anchorHref(), QStringLiteral("https://example.com/a?b=1&c=2"));
        // The presentation program shows a link in the theme's link color, underlined.
        QCOMPARE(a.foreground().color(), QColor(0x05, 0x63, 0xC1));
        QCOMPARE(a.stringProperty(jp::tp::ColorRefP), QStringLiteral("#0563C1"));
        QCOMPARE(a.underlineStyle(), QTextCharFormat::SingleUnderline);
        // A jump to a slide has no address; a link that names nothing is no link.
        for (int i = 1; i <= 3; ++i) QVERIFY(!b.charAt(0, i).isAnchor());
        QVERIFY(!b.charAt(0, 4).isAnchor());
        QCOMPARE(b.charAt(0, 1).foreground().color(), QColor(0xFF, 0, 0));
        // Without a package there is nothing to look the id up in.
        Body none(para(run(QStringLiteral("a"), QString(), link(QStringLiteral("rId1")))));
        QVERIFY(!none.charAt().isAnchor());
    }

    void textHyperlinkMayKeepTheTextColor()
    {
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "hyperlink", QStringLiteral("mailto:a@example.com"), true);
        Package p;
        QVERIFY(p.read(w.finish()));
        const QString keep = QStringLiteral("<a:hlinkClick r:id=\"rId1\"><a:extLst><a:ext uri=\"{A12FA001-AC4F-418D-AE19-62706E023703}\">"
                                            "<ahyp:hlinkClr xmlns:ahyp=\"urn:example:hyperlink-color\" val=\"tx\"/></a:ext></a:extLst></a:hlinkClick>");
        Body b(para(run(QStringLiteral("a"), QString(), solid(QStringLiteral("FF0000")) + keep)), {}, QString(), &p);
        QVERIFY(b.charAt().isAnchor());
        QCOMPARE(b.charAt().foreground().color(), QColor(0xFF, 0, 0));
    }

    void textEmptyParagraphKeepsItsEndSize()
    {
        Body b(QStringLiteral("<a:p><a:endParaRPr sz=\"4400\"/></a:p><a:p><a:endParaRPr sz=\"1200\"/></a:p><a:p><a:r><a:t>x</a:t></a:r></a:p>"));
        QCOMPARE(b.block(0).charFormat().fontPointSize(), 44.0);
        QCOMPARE(b.block(1).charFormat().fontPointSize(), 12.0);
        QVERIFY(b.block(0).text().isEmpty());
        // Laid out, the empty line is as tall as its size says.
        jp::FrameSpec fs;
        fs.size = QSizeF(300, 400);
        fs.insets = QMarginsF(0, 0, 0, 0);
        jp::StoryLayout lay;
        lay.build(&b.doc, {fs}, jp::LayoutEnv());
        const auto lines = lay.lineInfo(0);
        QCOMPARE(lines.size(), 3);
        QVERIFY2(lines[0].rect.height() > 3 * lines[1].rect.height(), qPrintable(QStringLiteral("%1 and %2").arg(lines[0].rect.height()).arg(lines[1].rect.height())));
        QVERIFY(lines[0].rect.height() >= 44);
    }

    void textEndParagraphSizeInheritsToo()
    {
        Body b(QStringLiteral("<a:p/><a:p><a:endParaRPr b=\"1\"/></a:p>"), {QStringLiteral("<a:lvl1pPr><a:defRPr sz=\"3600\"/></a:lvl1pPr>")});
        QCOMPARE(b.block(0).charFormat().fontPointSize(), 36.0);
        QCOMPARE(b.block(1).charFormat().fontPointSize(), 36.0);
        QCOMPARE(b.block(1).charFormat().fontWeight(), int(QFont::Bold));
    }

    void textLineBreaksAndTabsStayInTheirParagraph()
    {
        Body b(QStringLiteral("<a:p><a:r><a:t>a</a:t></a:r><a:br><a:rPr sz=\"2000\"/></a:br><a:r><a:t>b c</a:t></a:r><a:br/><a:r><a:t>x_x000B_y&#9;z_x0009_w</a:t></a:r></a:p>"));
        QCOMPARE(b.doc.blockCount(), 1);
        const QChar sep(QChar::LineSeparator);
        QCOMPARE(b.text(), QStringLiteral("a") + sep + QStringLiteral("b c") + sep + QStringLiteral("x") + sep + QStringLiteral("y\tz\tw"));
        QCOMPARE(b.charAt(0, 1).fontPointSize(), 20.0);   // the break has its own size
        // A line feed inside a run is a line break too, never a new paragraph.
        Body lf(QStringLiteral("<a:p><a:r><a:t>one&#10;two&#13;three</a:t></a:r></a:p>"));
        QCOMPARE(lf.doc.blockCount(), 1);
        QVERIFY(lf.text().contains(sep));
        QVERIFY(!lf.text().contains(QLatin1Char('\n')));
    }

    void textBreaksLinesWhereTheSizeSaysItMust()
    {
        // The same two paragraphs at two sizes: bigger letters, more lines.
        auto lines = [](int sz) {
            const QString words = QStringLiteral("The quick brown fox jumps over the lazy dog and then runs back to the barn");
            Body b(para(run(words, QStringLiteral("sz=\"%1\"").arg(sz))) + para(run(QStringLiteral("Second paragraph with a few more words in it"), QStringLiteral("sz=\"%1\"").arg(sz))));
            jp::FrameSpec fs;
            fs.size = QSizeF(240, 2000);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&b.doc, {fs}, jp::LayoutEnv());
            return int(lay.lineInfo(0).size());
        };
        const int fewer = lines(1200), more = lines(3600);
        QVERIFY2(more > fewer, qPrintable(QStringLiteral("%1 lines at 12 pt, %2 at 36 pt").arg(fewer).arg(more)));
        QVERIFY(fewer >= 2);
    }

    void textBodyHostileInputIsClamped()
    {
        // Sizes within 1 to 4000 pt; nonsense is ignored.
        Body b(para(run(QStringLiteral("a"), QStringLiteral("sz=\"1\"")) + run(QStringLiteral("b"), QStringLiteral("sz=\"99999999\""))
                    + run(QStringLiteral("c"), QStringLiteral("sz=\"0\"")) + run(QStringLiteral("d"), QStringLiteral("sz=\"-300\""))
                    + run(QStringLiteral("e"), QStringLiteral("sz=\"abc\" spc=\"x\" baseline=\"y\" kern=\"z\"")) + run(QStringLiteral("f"), QStringLiteral("spc=\"99999999\""))));
        QCOMPARE(b.charAt(0, 0).fontPointSize(), 1.0);
        QCOMPARE(b.charAt(0, 1).fontPointSize(), 4000.0);
        QCOMPARE(b.charAt(0, 2).fontPointSize(), 1.0);
        QCOMPARE(b.charAt(0, 3).fontPointSize(), 1.0);
        QCOMPARE(b.charAt(0, 4).fontPointSize(), 18.0);
        QVERIFY(qAbs(b.charAt(0, 5).fontLetterSpacing()) <= 1584.0);
    }

    void textBodyDamagedElementsAreSurvived()
    {
        const QString junk = QStringLiteral(
            "<a:p><a:pPr lvl=\"99\" marL=\"abc\" indent=\"\" algn=\"sideways\" defTabSz=\"x\" rtl=\"maybe\"><a:lnSpc/><a:spcBef><a:spcPct/></a:spcBef><a:spcAft><a:spcPts val=\"-5\"/></a:spcAft>"
            "<a:buClr/><a:buSzPct/><a:buSzPts val=\"q\"/><a:buFont/><a:buChar/><a:buAutoNum/><a:buAutoNum type=\"arabicPeriod\" startAt=\"-4\"/>"
            "<a:tabLst><a:tab/><a:tab pos=\"x\"/></a:tabLst><a:defRPr sz=\"abc\"/></a:pPr>"
            "<a:r/><a:r><a:rPr><a:hlinkClick/><a:solidFill/><a:highlight/><a:latin/><a:ea/><a:cs/><a:gradFill/></a:rPr></a:r><a:fld/><a:br/><a:endParaRPr/><a:r><a:t/></a:r></a:p>"
            "<a:p><a:pPr lvl=\"-3\"/></a:p><a:p/><a:p><a:r><a:rPr u=\"bogus\" strike=\"bogus\" cap=\"bogus\" b=\"bogus\"/><a:t>t</a:t></a:r></a:p>");
        Body b(junk, {QStringLiteral("<a:lvl1pPr><a:defRPr/></a:lvl1pPr><a:lvl9pPr/><a:defPPr/>"), QString()});
        QCOMPARE(b.doc.blockCount(), 4);
        QCOMPARE(b.block(0).blockFormat().intProperty(jp::tp::ListLevel), 8);
        QCOMPARE(b.block(1).blockFormat().intProperty(jp::tp::ListLevel), 0);
        // And a body of the wrong kind of element.
        QDomDocument dom;
        QVERIFY(parseXml("<a:txBody xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><a:p><a:p><a:r><a:r/></a:r></a:p></a:p><a:t>stray</a:t></a:txBody>", &dom));
        QTextDocument doc;
        readTextBody(dom.documentElement(), &doc, DrawingContext());
        QVERIFY(doc.blockCount() >= 1);
        // And a stack with nothing in some of its layers.
        readTextBody(dom.documentElement(), &doc, DrawingContext(), TextStyleStack{QDomElement(), dom.documentElement(), QDomElement()});
        QVERIFY(doc.blockCount() >= 1);
    }

    void textBodySurvivesRandomDamage()
    {
        const QString layer = QStringLiteral(
            "<a:lstStyle %1><a:defPPr><a:defRPr sz=\"1800\"/></a:defPPr><a:lvl1pPr marL=\"342900\" indent=\"-342900\" algn=\"ctr\" defTabSz=\"914400\" rtl=\"0\">"
            "<a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPts val=\"1000\"/></a:spcBef><a:buClr><a:srgbClr val=\"FF0000\"/></a:buClr>"
            "<a:buSzPct val=\"80000\"/><a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/><a:tabLst><a:tab pos=\"914400\" algn=\"r\"/></a:tabLst>"
            "<a:defRPr sz=\"2800\" kern=\"1200\" b=\"1\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr>"
            "</a:lstStyle>").arg(declarations());
        const QString body = QStringLiteral(
            "<p:txBody %1><a:bodyPr/><a:lstStyle/>"
            "<a:p><a:pPr lvl=\"0\" marL=\"457200\"><a:buAutoNum type=\"arabicPeriod\" startAt=\"2\"/></a:pPr>"
            "<a:r><a:rPr lang=\"en-US\" sz=\"2000\" b=\"1\" i=\"1\" u=\"wavyHeavy\" strike=\"sngStrike\" baseline=\"30000\" spc=\"-50\" cap=\"small\">"
            "<a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill><a:highlight><a:srgbClr val=\"FFFF00\"/></a:highlight><a:latin typeface=\"Arial\"/>"
            "<a:hlinkClick r:id=\"rId1\"/></a:rPr><a:t>Some text</a:t></a:r><a:br><a:rPr sz=\"1200\"/></a:br>"
            "<a:fld id=\"{1}\" type=\"slidenum\"><a:rPr/><a:t>1</a:t></a:fld><a:fld id=\"{2}\" type=\"datetime9\"><a:t>now</a:t></a:fld>"
            "<a:endParaRPr sz=\"900\"/></a:p>"
            "<a:p><a:pPr lvl=\"1\" algn=\"dist\" rtl=\"1\"><a:lnSpc><a:spcPts val=\"2400\"/></a:lnSpc><a:spcAft><a:spcPct val=\"50000\"/></a:spcAft><a:buNone/></a:pPr>"
            "<a:r><a:t>tab\there</a:t></a:r></a:p></p:txBody>").arg(declarations());
        PackageWriter w;
        w.addPart(QStringLiteral("/ppt/slides/slide1.xml"), "<p:sld/>", kSlideType);
        w.addRelationship(QStringLiteral("/ppt/slides/slide1.xml"), kRelBase + "hyperlink", QStringLiteral("https://example.com/"), true);
        Package package;
        QVERIFY(package.read(w.finish()));
        DrawingContext ctx;
        ctx.package = &package;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        ctx.theme = colorTestTheme();
        QRandomGenerator gen(4711);
        int read = 0;
        for (int i = 0; i < 600; ++i) {
            QByteArray badBody = body.toUtf8(), badLayer = layer.toUtf8();
            for (QByteArray *bad : {&badBody, &badLayer})
                if (gen.bounded(3) != 0)
                    for (int k = 0, n = 1 + int(gen.bounded(4)); k < n; ++k) (*bad)[int(gen.bounded(quint32(bad->size())))] = char(gen.bounded(32u, 127u));
            QDomDocument bodyDom, layerDom;
            if (!parseXml(badBody, &bodyDom) || !parseXml(badLayer, &layerDom)) continue;
            ++read;
            QTextDocument doc;
            readTextBody(bodyDom.documentElement(), &doc, ctx, TextStyleStack{layerDom.documentElement()});
            QVERIFY(doc.blockCount() >= 1);
            if (i % 12 == 0) {
                // What was read goes through the text engine too.
                jp::FrameSpec fs;
                fs.size = QSizeF(300, 300);
                jp::StoryLayout lay;
                lay.build(&doc, {fs}, jp::LayoutEnv());
            }
        }
        QVERIFY(read > 100);
    }

    void textBodyStopsAtTenThousandParagraphsAndTwentyThousandRuns()
    {
        {
            QString many;
            many.reserve(10001 * 6);
            for (int i = 0; i < 10001; ++i) many += QStringLiteral("<a:p/>");
            Body b(many);
            QCOMPARE(b.doc.blockCount(), 10000);
        }
        {
            QString runs = QStringLiteral("<a:p>");
            runs.reserve(20001 * 20);
            for (int i = 0; i < 20001; ++i) runs += QStringLiteral("<a:r><a:t>x</a:t></a:r>");
            runs += QStringLiteral("</a:p>");
            Body b(runs);
            QCOMPARE(b.doc.blockCount(), 1);
            QCOMPARE(b.text().size(), 20000);
        }
    }
};

JP_TEST_CLASS(OfficeTextTests)
#include "tst_office_text.moc"
