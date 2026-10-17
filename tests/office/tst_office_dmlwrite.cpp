// Tests of libs/office: writing DrawingML.
#include "officehelpers.h"

class OfficeDmlWriteTests : public QObject {
    Q_OBJECT

    using Effects = jp::Effects;

    struct Run {
        QString text;
        QTextCharFormat format;
    };
    static QVector<Run> runsOf(const QTextBlock &b)
    {
        QVector<Run> out;
        for (auto it = b.begin(); !it.atEnd(); ++it) out.append({it.fragment().text(), it.fragment().charFormat()});
        return out;
    }

    static bool sameRect(const QRectF &a, const QRectF &b, double tolerance = 1e-3)
    {
        return near(a.x(), b.x(), tolerance) && near(a.y(), b.y(), tolerance) && near(a.width(), b.width(), tolerance) && near(a.height(), b.height(), tolerance);
    }

    static QStringList childNamesOf(const QDomElement &e)
    {
        QStringList out;
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) out << c.localName();
        return out;
    }
    // Whether names come in the order of the schema's sequence (each at or
    // after the one before); an entry of the sequence may name alternatives, "a|b".
    static bool inSequence(const QStringList &names, const QStringList &sequence)
    {
        int at = 0;
        for (const QString &n : names) {
            int i = -1;
            for (int k = 0; k < sequence.size() && i < 0; ++k)
                if (sequence[k].split(QLatin1Char('|')).contains(n)) i = k;
            if (i < 0 || i < at) return false;
            at = i;
        }
        return true;
    }

    static std::shared_ptr<TextItem> writtenTextBox(WriteRound &r, const QRectF &rect, const QString &text)
    {
        auto t = std::make_shared<TextItem>();
        t->rect = rect;
        t->storyId = r.addStory(text);
        return t;
    }

    // The shape, or the one in a group, that is the named preset.
    static const ShapeItem *namedShape(const ItemPtr &it, const QString &name)
    {
        if (it->type() == ItemType::Shape) {
            const auto *s = static_cast<const ShapeItem *>(it.get());
            return s->shape == name ? s : nullptr;
        }
        if (it->type() == ItemType::Group)
            for (const ItemPtr &c : static_cast<const GroupItem *>(it.get())->children)
                if (const ShapeItem *s = namedShape(c, name)) return s;
        return nullptr;
    }

private Q_SLOTS:
    void dmlWriteTextBoxKeepsItsFrameAndBody()
    {
        WriteRound r;
        auto t = writtenTextBox(r, QRectF(36.5, 40.25, 200, 80.5), QStringLiteral("Hello"));
        t->rotation = 30;
        t->flipH = true;
        t->name = QStringLiteral("Title box");
        t->altText = QStringLiteral("A <title> & \"more\"");
        t->fill = Fill::solid(ColorRef::rgb(QColor(0xFF, 0x88, 0x00)), 0.25);
        t->stroke = Stroke::line(ColorRef::rgb(QColor(0x11, 0x22, 0x33)), 2);
        t->insets = QMarginsF(3, 4, 5, 6);
        t->valign = jp::VAlign::Middle;
        t->autofit = TextItem::ShrinkOnOverflow;
        t->fitScale = 0.8;
        t->columns = 2;
        t->columnGap = 12;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Text));
        const auto *b = static_cast<TextItem *>(r.items[0].get());
        QVERIFY(sameRect(b->rect, t->rect));
        QVERIFY(near(b->rotation, 30, 1e-4));
        QVERIFY(b->flipH && !b->flipV);
        QCOMPARE(b->name, t->name);
        QCOMPARE(b->altText, t->altText);
        QVERIFY(b->fill.type == Fill::Solid);
        QCOMPARE(dmlHex(b->fill.color), QStringLiteral("FF8800"));
        QVERIFY(near(b->fill.transparency, 0.25, 1e-3));
        QVERIFY(near(b->stroke.width, 2));
        QCOMPARE(dmlHex(b->stroke.color), QStringLiteral("112233"));
        QCOMPARE(b->insets, QMarginsF(3, 4, 5, 6));
        QVERIFY(b->valign == jp::VAlign::Middle);
        QVERIFY(b->autofit == TextItem::ShrinkOnOverflow);
        QVERIFY(near(b->fitScale, 0.8, 1e-5));
        QCOMPARE(b->columns, 2);
        QVERIFY(near(b->columnGap, 12));
        QVERIFY(!b->vertical);
        QCOMPARE(r.storyOf(r.items[0])->toPlainText(), QStringLiteral("Hello"));
        // A plain text box says so (it is no placeholder), and has a name even where it had none.
        QVERIFY(r.text().contains(QStringLiteral("txBox=\"1\"")));
        WriteRound unnamed;
        auto u = writtenTextBox(unnamed, QRectF(0, 0, 10, 10), QStringLiteral("x"));
        QVERIFY2(unnamed.run({u}), qPrintable(unnamed.error));
        QVERIFY(!static_cast<TextItem *>(unnamed.items[0].get())->name.isEmpty());
        QVERIFY(!unnamed.text().contains(QStringLiteral("descr")));
    }

    void dmlWriteTextBodyKeepsAutofitAndDirection()
    {
        const struct { TextItem::Autofit autofit; const char *element; } fits[] = {
            {TextItem::NoAutofit, "noAutofit"}, {TextItem::ShrinkOnOverflow, "normAutofit"}, {TextItem::GrowBox, "spAutoFit"}};
        for (const auto &f : fits) {
            WriteRound r;
            auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
            t->autofit = f.autofit;
            t->vertical = true;
            t->valign = jp::VAlign::Bottom;
            QVERIFY2(r.run({t}), qPrintable(r.error));
            const auto *b = static_cast<TextItem *>(r.items[0].get());
            QVERIFY2(b->autofit == f.autofit, f.element);
            QVERIFY(near(b->fitScale, 1.0));
            QVERIFY(b->vertical);
            QVERIFY(b->valign == jp::VAlign::Bottom);
            QCOMPARE(int(elementsNamed(r.spTree, f.element).size()), 1);
        }
    }

    void dmlWriteRunsKeepTheirLook()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        auto put = [&](const QString &text, const std::function<void(QTextCharFormat &)> &set) {
            QTextCharFormat f;
            f.setFontPointSize(14);
            set(f);
            c.insertText(text, f);
        };
        put(QStringLiteral("plain "), [](QTextCharFormat &) {});
        put(QStringLiteral("styled "), [](QTextCharFormat &f) {
            f.setFontPointSize(20.5);
            f.setFontWeight(QFont::Bold);
            f.setFontItalic(true);
            f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            f.setFontStrikeOut(true);
            f.setForeground(QColor(0x11, 0x22, 0x33));
            f.setProperty(jp::tp::ColorRefP, QStringLiteral("#112233"));
        });
        put(QStringLiteral("doubled "), [](QTextCharFormat &f) {
            f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            f.setProperty(jp::tp::UnderlineKind, 1);
        });
        put(QStringLiteral("wavy "), [](QTextCharFormat &f) { f.setUnderlineStyle(QTextCharFormat::WaveUnderline); });
        put(QStringLiteral("up "), [](QTextCharFormat &f) { f.setVerticalAlignment(QTextCharFormat::AlignSuperScript); });
        put(QStringLiteral("down "), [](QTextCharFormat &f) { f.setVerticalAlignment(QTextCharFormat::AlignSubScript); });
        put(QStringLiteral("spaced "), [](QTextCharFormat &f) {
            f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            f.setFontLetterSpacing(1.5);
            f.setProperty(jp::tp::KernAbove, 12.0);
        });
        put(QStringLiteral("caps "), [](QTextCharFormat &f) { f.setFontCapitalization(QFont::AllUppercase); });
        put(QStringLiteral("small "), [](QTextCharFormat &f) { f.setFontCapitalization(QFont::SmallCaps); });
        put(QStringLiteral("face "), [](QTextCharFormat &f) { f.setFontFamilies({QStringLiteral("Carlito"), QStringLiteral("Noto Sans CJK JP")}); });
        put(QStringLiteral("heading "), [](QTextCharFormat &f) { f.setProperty(jp::tp::ThemeFont, QStringLiteral("major")); });
        put(QStringLiteral("body "), [](QTextCharFormat &f) { f.setProperty(jp::tp::ThemeFont, QStringLiteral("minor")); });
        put(QStringLiteral("marked "), [](QTextCharFormat &f) { f.setProperty(jp::tp::HighlightRefP, QStringLiteral("#FFFF00")); });
        put(QStringLiteral("french "), [](QTextCharFormat &f) {
            f.setProperty(jp::tp::Language, QStringLiteral("fr-FR"));
            f.setProperty(jp::tp::NoProof, true);
        });
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 100);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QVector<Run> runs = runsOf(r.storyOf(r.items[0])->firstBlock());
        QCOMPARE(int(runs.size()), 14);
        QStringList text;
        for (const Run &run : runs) text << run.text;
        QCOMPARE(text, (QStringList{"plain ", "styled ", "doubled ", "wavy ", "up ", "down ", "spaced ", "caps ", "small ", "face ", "heading ", "body ", "marked ", "french "}));
        for (const Run &run : runs) QVERIFY2(run.format.fontWeight() == QFont::Normal || run.text == QLatin1String("styled "), qPrintable(run.text));
        QVERIFY(near(runs[0].format.fontPointSize(), 14));
        QVERIFY(!runs[0].format.fontItalic() && !runs[0].format.fontStrikeOut());
        QVERIFY(runs[0].format.underlineStyle() == QTextCharFormat::NoUnderline);
        QVERIFY(near(runs[1].format.fontPointSize(), 20.5));
        QCOMPARE(int(runs[1].format.fontWeight()), int(QFont::Bold));
        QVERIFY(runs[1].format.fontItalic() && runs[1].format.fontStrikeOut());
        QVERIFY(runs[1].format.underlineStyle() == QTextCharFormat::SingleUnderline);
        QCOMPARE(runs[1].format.stringProperty(jp::tp::ColorRefP), QStringLiteral("#112233"));
        QCOMPARE(runs[1].format.foreground().color(), QColor(0x11, 0x22, 0x33));
        QVERIFY(runs[2].format.underlineStyle() == QTextCharFormat::SingleUnderline);
        QCOMPARE(runs[2].format.intProperty(jp::tp::UnderlineKind), 1);
        QVERIFY(runs[3].format.underlineStyle() == QTextCharFormat::WaveUnderline);
        QVERIFY(runs[4].format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
        QVERIFY(runs[5].format.verticalAlignment() == QTextCharFormat::AlignSubScript);
        QVERIFY(runs[6].format.fontLetterSpacingType() == QFont::AbsoluteSpacing && near(runs[6].format.fontLetterSpacing(), 1.5));
        QVERIFY(near(runs[6].format.property(jp::tp::KernAbove).toDouble(), 12));
        QVERIFY(runs[7].format.fontCapitalization() == QFont::AllUppercase);
        QVERIFY(runs[8].format.fontCapitalization() == QFont::SmallCaps);
        QCOMPARE(runs[9].format.fontFamilies().toStringList(), (QStringList{"Carlito", "Noto Sans CJK JP"}));
        QCOMPARE(runs[10].format.stringProperty(jp::tp::ThemeFont), QStringLiteral("major"));
        QCOMPARE(runs[11].format.stringProperty(jp::tp::ThemeFont), QStringLiteral("minor"));
        QVERIFY(runs[10].format.fontFamilies().toStringList().isEmpty());
        QCOMPARE(runs[12].format.stringProperty(jp::tp::HighlightRefP), QStringLiteral("#FFFF00"));
        QCOMPARE(runs[13].format.stringProperty(jp::tp::Language), QStringLiteral("fr-FR"));
        QVERIFY(runs[13].format.boolProperty(jp::tp::NoProof));
        // The theme font names no typeface of its own, and the others name theirs.
        QVERIFY(r.text().contains(QStringLiteral("+mj-lt")) && r.text().contains(QStringLiteral("+mn-lt")));
        QVERIFY(r.text().contains(QStringLiteral("typeface=\"Carlito\"")));
    }

    void dmlWriteTextEscapesAndCleansWhatXmlCannotHold()
    {
        WriteRound r;
        const QString odd = QStringLiteral("a < b & c > \"d\" 'e' \u00E9\U0001F600") + QChar(0x01) + QChar(0x0B) + QChar(0xFFFE) + QStringLiteral("end\ttab");
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), odd);
        t->name = QStringLiteral("n\"ame") + QChar(0x02);
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QCOMPARE(r.storyOf(r.items[0])->toPlainText(), QStringLiteral("a < b & c > \"d\" 'e' \u00E9\U0001F600end\ttab"));
        QCOMPARE(static_cast<TextItem *>(r.items[0].get())->name, QStringLiteral("n\"ame"));
    }

    void dmlWriteParagraphsKeepTheirFormat()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(16);
        auto block = [&](const QString &text, const std::function<void(QTextBlockFormat &)> &set, bool first = false) {
            QTextBlockFormat bf;
            set(bf);
            if (!first) c.insertBlock(bf, cf);
            else c.setBlockFormat(bf);
            c.insertText(text, cf);
        };
        block(QStringLiteral("centered"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignHCenter);
            bf.setLeftMargin(36);
            bf.setTextIndent(-18);
            bf.setTopMargin(6);
            bf.setBottomMargin(12.5);
            bf.setLineHeight(120, QTextBlockFormat::ProportionalHeight);
        }, true);
        block(QStringLiteral("right"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignRight);
            bf.setRightMargin(10);
            bf.setLineHeight(24, QTextBlockFormat::FixedHeight);
        });
        block(QStringLiteral("justified"), [](QTextBlockFormat &bf) { bf.setAlignment(Qt::AlignJustify); });
        block(QStringLiteral("spread"), [](QTextBlockFormat &bf) {
            bf.setAlignment(Qt::AlignJustify);
            bf.setProperty(jp::tp::Distribute, true);
        });
        block(QStringLiteral("left, no spacing"), [](QTextBlockFormat &bf) { bf.setAlignment(Qt::AlignLeft); });
        block(QStringLiteral("deeper"), [](QTextBlockFormat &bf) {
            bf.setProperty(jp::tp::ListLevel, 2);
            bf.setLeftMargin(72);
        });
        block(QStringLiteral("tabbed\tstops"), [](QTextBlockFormat &bf) {
            QList<QTextOption::Tab> tabs;
            tabs << QTextOption::Tab(36, QTextOption::LeftTab) << QTextOption::Tab(100, QTextOption::RightTab) << QTextOption::Tab(200, QTextOption::CenterTab)
                 << QTextOption::Tab(300, QTextOption::DelimiterTab, QLatin1Char('.'));
            bf.setTabPositions(tabs);
        });
        block(QStringLiteral("backwards"), [](QTextBlockFormat &bf) {
            bf.setLayoutDirection(Qt::RightToLeft);
            bf.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        });
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 300);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QTextDocument *back = r.storyOf(r.items[0]);
        QCOMPARE(back->blockCount(), 8);
        QVector<QTextBlockFormat> f;
        for (QTextBlock b = back->begin(); b.isValid(); b = b.next()) f << b.blockFormat();
        QCOMPARE(int(f[0].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignHCenter));
        QVERIFY(near(f[0].leftMargin(), 36) && near(f[0].textIndent(), -18) && near(f[0].topMargin(), 6) && near(f[0].bottomMargin(), 12.5));
        QVERIFY(f[0].lineHeightType() == QTextBlockFormat::ProportionalHeight && near(f[0].lineHeight(), 120));
        QCOMPARE(int(f[1].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
        QVERIFY(near(f[1].rightMargin(), 10));
        QVERIFY(f[1].lineHeightType() == QTextBlockFormat::FixedHeight && near(f[1].lineHeight(), 24));
        QCOMPARE(int(f[2].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(!f[2].boolProperty(jp::tp::Distribute));
        QCOMPARE(int(f[3].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignJustify));
        QVERIFY(f[3].boolProperty(jp::tp::Distribute));
        QCOMPARE(int(f[4].alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignLeft));
        QVERIFY(near(f[4].leftMargin(), 0) && near(f[4].textIndent(), 0));
        QCOMPARE(f[5].intProperty(jp::tp::ListLevel), 2);
        QVERIFY(near(f[5].leftMargin(), 72));
        const QList<QTextOption::Tab> tabs = f[6].tabPositions();
        QCOMPARE(int(tabs.size()), 4);
        QVERIFY(near(tabs[0].position, 36) && tabs[0].type == QTextOption::LeftTab);
        QVERIFY(near(tabs[1].position, 100) && tabs[1].type == QTextOption::RightTab);
        QVERIFY(near(tabs[2].position, 200) && tabs[2].type == QTextOption::CenterTab);
        QVERIFY(near(tabs[3].position, 300) && tabs[3].type == QTextOption::DelimiterTab);
        QVERIFY(f[7].layoutDirection() == Qt::RightToLeft);
        QVERIFY(f[7].alignment() & Qt::AlignAbsolute);
        QVERIFY(f[7].alignment() & Qt::AlignLeft);
    }

    void dmlWriteTabsLeaveOutTheDefaultStopsAReaderMade()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextBlockFormat bf;
        QList<QTextOption::Tab> tabs;
        for (int i = 1; i <= 40; ++i) tabs << QTextOption::Tab(i * 36.0, QTextOption::LeftTab);
        bf.setTabPositions(tabs);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("a\tb"));
        auto t = std::make_shared<TextItem>();
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        // Forty stops a point-and-a-half apart in a row are the default ones: they come as a distance.
        QVERIFY2(elementsNamed(r.spTree, "tab").size() < 10, qPrintable(QString::number(elementsNamed(r.spTree, "tab").size())));
        QVERIFY(r.text().contains(QStringLiteral("defTabSz=\"457200\"")));
        const QTextBlockFormat back = r.storyOf(r.items[0])->firstBlock().blockFormat();
        QVERIFY(back.tabPositions().isEmpty() || near(back.tabPositions().first().position, 36));
    }

    void dmlWriteBulletsAndNumbersKeepTheirList()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(20);
        auto item = [&](const QString &text, const QTextListFormat &lf, bool first = false) {
            if (!first) c.insertBlock(QTextBlockFormat(), cf);
            c.insertText(text, cf);
            if (lf.style() != QTextListFormat::ListStyleUndefined) c.createList(lf);
        };
        QTextListFormat custom;
        custom.setStyle(QTextListFormat::ListDisc);
        custom.setProperty(jp::tp::BulletChar, QStringLiteral("\u00A7"));
        custom.setProperty(jp::tp::BulletFont, QStringLiteral("Wingdings"));
        custom.setProperty(jp::tp::BulletColor, QStringLiteral("#FF0000"));
        custom.setProperty(jp::tp::BulletSize, 10.0);
        item(QStringLiteral("marked"), custom, true);
        QTextListFormat plainDisc;
        plainDisc.setStyle(QTextListFormat::ListDisc);
        item(QStringLiteral("plain bullet"), plainDisc);
        item(QStringLiteral("no list"), QTextListFormat());
        QTextListFormat decimal;
        decimal.setStyle(QTextListFormat::ListDecimal);
        decimal.setNumberSuffix(QStringLiteral("."));
        decimal.setStart(3);
        decimal.setProperty(jp::tp::NumberFormat, 1);
        item(QStringLiteral("three"), decimal);
        item(QStringLiteral("four"), QTextListFormat());   // joins the list above (below)
        c.block().previous().textList()->add(c.block());
        QTextListFormat alpha;
        alpha.setStyle(QTextListFormat::ListLowerAlpha);
        alpha.setNumberPrefix(QStringLiteral("("));
        alpha.setNumberSuffix(QStringLiteral(")"));
        item(QStringLiteral("alpha"), alpha);
        QTextListFormat roman;
        roman.setStyle(QTextListFormat::ListUpperRoman);
        roman.setNumberSuffix(QStringLiteral(")"));
        item(QStringLiteral("roman"), roman);
        QTextListFormat inner;
        inner.setStyle(QTextListFormat::ListDisc);
        inner.setProperty(jp::tp::ListLevel, 1);
        item(QStringLiteral("inner"), inner);
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 300);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QTextDocument *back = r.storyOf(r.items[0]);
        QCOMPARE(back->blockCount(), 8);
        QVector<QTextBlock> blocks;
        for (QTextBlock b = back->begin(); b.isValid(); b = b.next()) blocks << b;
        QTextList *l0 = blocks[0].textList();
        QVERIFY(l0 && l0->format().style() == QTextListFormat::ListDisc);
        QCOMPARE(l0->format().stringProperty(jp::tp::BulletChar), QStringLiteral("\u00A7"));
        QCOMPARE(l0->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Wingdings"));
        QCOMPARE(l0->format().stringProperty(jp::tp::BulletColor), QStringLiteral("#FF0000"));
        QVERIFY(near(l0->format().property(jp::tp::BulletSize).toDouble(), 10));
        QVERIFY(blocks[1].textList() && blocks[1].textList() != l0);
        QVERIFY(blocks[1].textList()->format().style() == QTextListFormat::ListDisc);
        QVERIFY(!blocks[1].textList()->format().hasProperty(jp::tp::BulletFont));
        QVERIFY(!blocks[2].textList());
        QTextList *numbers = blocks[3].textList();
        QVERIFY(numbers && numbers->format().style() == QTextListFormat::ListDecimal);
        QCOMPARE(numbers->format().numberSuffix(), QStringLiteral("."));
        QCOMPARE(numbers->format().start(), 3);
        QVERIFY(blocks[4].textList() == numbers);   // the next item goes on in the list
        QCOMPARE(numbers->itemNumber(blocks[4]), 1);
        QTextList *letters = blocks[5].textList();
        QVERIFY(letters && letters->format().style() == QTextListFormat::ListLowerAlpha);
        QCOMPARE(letters->format().numberPrefix(), QStringLiteral("("));
        QCOMPARE(letters->format().numberSuffix(), QStringLiteral(")"));
        QTextList *romans = blocks[6].textList();
        QVERIFY(romans && romans->format().style() == QTextListFormat::ListUpperRoman);
        QCOMPARE(romans->format().numberSuffix(), QStringLiteral(")"));
        QTextList *deeper = blocks[7].textList();
        QVERIFY(deeper);
        QCOMPARE(deeper->format().intProperty(jp::tp::ListLevel), 1);
        QCOMPARE(blocks[7].blockFormat().intProperty(jp::tp::ListLevel), 1);
        // A bullet with no mark of its own says a bullet, in the text's font.
        QVERIFY(!r.text().contains(QStringLiteral("typeface=\"Arial\"")));
        QCOMPARE(int(elementsNamed(r.spTree, "buNone").size()), 0);
    }

    void dmlWritePlaceholderTextSaysNoBulletWhenItHasNone()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        c.insertText(QStringLiteral("listed"));
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        c.createList(lf);
        c.insertBlock(QTextBlockFormat(), QTextCharFormat());
        c.insertText(QStringLiteral("plain"));
        auto holder = std::make_shared<TextItem>();
        holder->role = QStringLiteral("ph:body:1");
        holder->storyId = id;
        auto box = std::make_shared<TextItem>();
        box->storyId = r.addStory(QStringLiteral("one\ntwo"));
        QVERIFY2(r.run({holder, box}), qPrintable(r.error));
        // Only the placeholder's paragraph with no list says so; a text box says nothing.
        const QVector<QDomElement> bodies = elementsNamed(r.spTree, "txBody");
        QCOMPARE(int(bodies.size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "buNone").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[0], "buChar").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[1], "buNone").size()), 0);
        // The layout and master may space and mark paragraphs in their own way: a paragraph of a placeholder says how it is.
        QCOMPARE(int(elementsNamed(bodies[0], "lnSpc").size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "spcBef").size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "spcAft").size()), 2);
        QCOMPARE(int(elementsNamed(bodies[0], "buClrTx").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[0], "buSzTx").size()), 1);
        QCOMPARE(int(elementsNamed(bodies[0], "buFontTx").size()), 1);
        for (const char *name : {"lnSpc", "spcBef", "spcAft", "buClrTx", "buSzTx", "buFontTx"}) QCOMPARE(int(elementsNamed(bodies[1], name).size()), 0);
        // And the flag is the writer's own: it is off again after a placeholder.
        QVERIFY(!r.ctx.placeholderText);
    }

    void dmlWriteFieldsBreaksAndEmptyParagraphs()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("Page "), cf);
        QTextCharFormat field = cf;
        field.setProperty(jp::tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QStringLiteral(" of "), cf);
        field.setProperty(jp::tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QString(QChar::LineSeparator) + QStringLiteral("second line"), cf);
        c.insertBlock(QTextBlockFormat(), cf);
        field.setProperty(jp::tp::Field, QStringLiteral("date:M/d/yyyy"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QStringLiteral(" "), cf);
        field.setProperty(jp::tp::Field, QStringLiteral("time:HH:mm"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        // An empty paragraph keeps its size, and one with only a break is not empty.
        QTextCharFormat big;
        big.setFontPointSize(31);
        c.insertBlock(QTextBlockFormat(), big);
        c.insertBlock(QTextBlockFormat(), cf);
        c.insertText(QString(QChar::LineSeparator), cf);
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 300);
        t->storyId = id;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QTextDocument *back = r.storyOf(r.items[0]);
        QCOMPARE(back->blockCount(), 4);
        const QVector<Run> first = runsOf(back->firstBlock());
        QStringList codes;
        for (const Run &run : first)
            if (!run.format.stringProperty(jp::tp::Field).isEmpty()) codes << run.format.stringProperty(jp::tp::Field);
        QCOMPARE(codes, (QStringList{"page", "page"}));
        QCOMPARE(back->firstBlock().text(), QString(QStringLiteral("Page ") + QChar::ObjectReplacementCharacter + QStringLiteral(" of ") + QChar::ObjectReplacementCharacter
                                                    + QChar(QChar::LineSeparator) + QStringLiteral("second line")));
        const QVector<Run> second = runsOf(back->firstBlock().next());
        QCOMPARE(second[0].format.stringProperty(jp::tp::Field), QStringLiteral("date:M/d/yyyy"));
        QCOMPARE(second.last().format.stringProperty(jp::tp::Field), QStringLiteral("time:HH:mm"));
        // Each field has an id of its own, and a type the standard names.
        const QVector<QDomElement> fields = elementsNamed(r.spTree, "fld");
        QCOMPARE(int(fields.size()), 4);
        QSet<QString> fieldIds;
        for (const QDomElement &f : fields) {
            QVERIFY(QRegularExpression(QStringLiteral("^\\{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}\\}$")).match(f.attribute(QStringLiteral("id"))).hasMatch());
            fieldIds << f.attribute(QStringLiteral("id"));
        }
        QCOMPARE(int(fieldIds.size()), 4);
        QCOMPARE(fields[0].attribute(QStringLiteral("type")), QStringLiteral("slidenum"));
        QVERIFY(fields[2].attribute(QStringLiteral("type")).startsWith(QLatin1String("datetime")));
        // The empty paragraph is as tall as it was.
        QVERIFY(near(back->firstBlock().next().next().charFormat().fontPointSize(), 31));
        QVERIFY(back->firstBlock().next().next().text().isEmpty());
        QVERIFY(!back->lastBlock().text().isEmpty());
        QCOMPARE(int(elementsNamed(r.spTree, "endParaRPr").size()), 1);
    }

    void dmlWriteHyperlinksAreRelationshipsMadeOnce()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(12);
        c.insertText(QStringLiteral("see "), cf);
        QTextCharFormat link = cf;
        link.setAnchor(true);
        link.setAnchorHref(QStringLiteral("https://example.com/a?b=1&c=2"));
        c.insertText(QStringLiteral("here"), link);
        c.insertText(QStringLiteral(" or "), cf);
        c.insertText(QStringLiteral("there"), link);
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 400, 100);
        t->storyId = id;
        t->hyperlink = QStringLiteral("mailto:someone@example.com");
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QVector<Run> runs = runsOf(r.storyOf(r.items[0])->firstBlock());
        QCOMPARE(int(runs.size()), 4);
        QVERIFY(!runs[0].format.isAnchor());
        QVERIFY(runs[1].format.isAnchor() && runs[3].format.isAnchor());
        QCOMPARE(runs[1].format.anchorHref(), QStringLiteral("https://example.com/a?b=1&c=2"));
        QCOMPARE(runs[3].format.anchorHref(), QStringLiteral("https://example.com/a?b=1&c=2"));
        QCOMPARE(r.items[0]->hyperlink, QStringLiteral("mailto:someone@example.com"));
        // Two addresses: the item's, and the one both runs use.
        const QVector<Relationship> rels = r.env.package.relationships(r.ctx.part);
        int external = 0;
        for (const Relationship &rel : rels) external += rel.external && rel.type.endsWith(QLatin1String("/hyperlink"));
        QCOMPARE(external, 2);
    }

    void dmlWriteShapesKeepPresetAndAdjustments_data()
    {
        QTest::addColumn<QString>("shape");
        QTest::addColumn<QString>("preset");
        QTest::addColumn<QSizeF>("size");
        QTest::addColumn<QVector<double>>("adj");
        QTest::newRow("rect") << "rect" << "rect" << QSizeF(100, 50) << QVector<double>();
        QTest::newRow("roundRect") << "roundRect" << "roundRect" << QSizeF(100, 50) << QVector<double>{0.3};
        QTest::newRow("right arrow") << "rightArrow" << "rightArrow" << QSizeF(160, 80) << QVector<double>{0.4, 0.6};
        QTest::newRow("up arrow") << "upArrow" << "upArrow" << QSizeF(80, 160) << QVector<double>{0.3, 0.5};
        QTest::newRow("parallelogram") << "parallelogram" << "parallelogram" << QSizeF(200, 100) << QVector<double>{0.15};
        QTest::newRow("trapezoid") << "trapezoid" << "trapezoid" << QSizeF(120, 200) << QVector<double>{0.2};
        QTest::newRow("star") << "star5" << "star5" << QSizeF(100, 100) << QVector<double>{0.3};
        QTest::newRow("quad arrow") << "quadArrow" << "quadArrow" << QSizeF(100, 100) << QVector<double>{0.1, 0.2, 0.25};
        QTest::newRow("u-turn arrow") << "uturnArrow" << "uturnArrow" << QSizeF(100, 100) << QVector<double>{0.2, 0.3};
        QTest::newRow("diagonal snip") << "snip2diag" << "snip2DiagRect" << QSizeF(100, 60) << QVector<double>{0.25};
        QTest::newRow("double wave") << "doubleWave" << "doubleWave" << QSizeF(100, 60) << QVector<double>{0.05};
        QTest::newRow("bent arrow") << "bentArrow" << "bentArrow" << QSizeF(100, 100) << QVector<double>{0.1, 0.2, 0.3};
        QTest::newRow("triangle") << "triangle" << "triangle" << QSizeF(100, 100) << QVector<double>{0.3};
        QTest::newRow("ellipse") << "ellipse" << "ellipse" << QSizeF(100, 100) << QVector<double>();
        QTest::newRow("flow chart") << "fcDecision" << "flowChartDecision" << QSizeF(100, 100) << QVector<double>();
        QTest::newRow("smiley") << "smiley" << "smileyFace" << QSizeF(100, 100) << QVector<double>{0.02};
    }

    void dmlWriteShapesKeepPresetAndAdjustments()
    {
        QFETCH(QString, shape);
        QFETCH(QString, preset);
        QFETCH(QSizeF, size);
        QFETCH(QVector<double>, adj);
        QCOMPARE(presetForShape(shape), preset);
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(20, 30, size.width(), size.height());
        s->shape = shape;
        s->adj = adj;
        s->fill = Fill::solid(ColorRef::rgb(QColor(0x20, 0x40, 0x80)));
        s->stroke = Stroke::line(ColorRef::rgb(QColor(0x80, 0x40, 0x20)), 1.5);
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Shape));
        const auto *b = static_cast<ShapeItem *>(r.items[0].get());
        QCOMPARE(b->shape, shape);
        QVERIFY(b->customPath.isEmpty());
        QVERIFY(sameRect(b->rect, s->rect));
        QCOMPARE(dmlHex(b->fill.color), QStringLiteral("204080"));
        QVERIFY(near(b->stroke.width, 1.5));
        QCOMPARE(elementsNamed(r.spTree, "prstGeom").value(0).attribute(QStringLiteral("prst")), preset);
        // The adjustments come back as they were, whether or not they were written (the standard's own are not).
        const QVector<double> want = adj.isEmpty() ? libraryAdjustments(preset, {}, size) : adj;
        QCOMPARE(int(b->adj.size()), int(want.size()));
        for (int i = 0; i < want.size(); ++i) QVERIFY2(near(b->adj[i], want[i], 1e-4), qPrintable(QStringLiteral("adjustment %1: %2 for %3").arg(i).arg(b->adj[i]).arg(want[i])));
    }

    void dmlWriteShapesLeaveOutAnAdjustmentThatIsTheStandards()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 100, 100);
        s->shape = QStringLiteral("roundRect");
        s->adj = {0.16667};
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QCOMPARE(int(elementsNamed(r.spTree, "gd").size()), 0);
        WriteRound other;
        s->adj = {0.4};
        QVERIFY2(other.run({s}), qPrintable(other.error));
        const QVector<QDomElement> gds = elementsNamed(other.spTree, "gd");
        QCOMPARE(int(gds.size()), 1);
        QCOMPARE(gds[0].attribute(QStringLiteral("name")), QStringLiteral("adj"));
        QCOMPARE(gds[0].attribute(QStringLiteral("fmla")), QStringLiteral("val 40000"));
    }

    // A preset with more than one adjustment has all of them written once one
    // is not the standard's: the presentation program refuses a file whose
    // u-turn arrow says adj1 and not adj2 to adj5 (or whose arrow says adj2
    // and not adj1), and opens the same file with all or none.
    void dmlWriteShapesWriteAllOfAPresetsAdjustmentsOrNone()
    {
        auto names = [](const QVector<QDomElement> &gds) {
            QStringList out;
            for (const QDomElement &g : gds) out << g.attribute(QStringLiteral("name")) + QLatin1Char('=') + g.attribute(QStringLiteral("fmla"));
            return out;
        };
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 160, 80);
        s->shape = QStringLiteral("rightArrow");
        s->adj = {0.5, 0.75};   // the shaft as the standard has it, a longer head
        WriteRound arrow;
        QVERIFY2(arrow.run({s}), qPrintable(arrow.error));
        QCOMPARE(names(elementsNamed(arrow.spTree, "gd")), (QStringList{"adj1=val 50000", "adj2=val 75000"}));

        s->shape = QStringLiteral("uturnArrow");
        s->rect = QRectF(0, 0, 100, 100);
        s->adj = {0.2};          // the library's first is the standard's adj1; the others keep the standard's
        WriteRound turn;
        QVERIFY2(turn.run({s}), qPrintable(turn.error));
        QCOMPARE(names(elementsNamed(turn.spTree, "gd")),
                 (QStringList{"adj1=val 20000", "adj2=val 25000", "adj3=val 25000", "adj4=val 43750", "adj5=val 75000"}));

        s->shape = QStringLiteral("rightArrow");
        s->rect = QRectF(0, 0, 160, 80);
        s->adj = {0.5, 0.5};     // the standard's own: none
        WriteRound plain;
        QVERIFY2(plain.run({s}), qPrintable(plain.error));
        QCOMPARE(int(elementsNamed(plain.spTree, "gd").size()), 0);
    }

    // The arc-based presets, with values the standard's own are not, as a file gives them: angles in 60000ths of a degree,
    // and a block arc's band as a fraction of 100,000.
    void dmlArcPresetsKeepTheirKindAndAdjustments_data()
    {
        QTest::addColumn<QString>("shape");
        QTest::addColumn<QVector<double>>("values");
        QTest::addColumn<QVector<double>>("adj");
        QTest::newRow("arc") << "arc" << QVector<double>{5400000, 10800000} << QVector<double>{0.25, 0.5};
        QTest::newRow("chord") << "chord" << QVector<double>{1350000, 5400000} << QVector<double>{0.0625, 0.25};
        QTest::newRow("pie") << "pie" << QVector<double>{5400000, 10800000} << QVector<double>{0.25, 0.5};
        QTest::newRow("block arc") << "blockArc" << QVector<double>{5400000, 16200000, 10000} << QVector<double>{0.25, 0.75, 0.1};
    }

    // A shape of the library, given its adjustments, is written as its preset with them, and read back as it was.
    void dmlArcPresetsKeepTheirKindAndAdjustments()
    {
        QFETCH(QString, shape);
        QFETCH(QVector<double>, values);
        QFETCH(QVector<double>, adj);
        const QRectF frame(20, 30, 136.6, 90);
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = frame;
        s->shape = shape;
        s->adj = adj;
        s->fill = Fill::solid(ColorRef::rgb(QColor(0x20, 0x40, 0x80)));
        s->stroke = Stroke::line(ColorRef::rgb(QColor(0x80, 0x40, 0x20)), 1.5);
        QVERIFY2(r.run({s}), qPrintable(r.error));
        const QVector<QDomElement> presets = elementsNamed(r.spTree, "prstGeom");
        QCOMPARE(int(presets.size()), 1);
        QCOMPARE(presets[0].attribute(QStringLiteral("prst")), shape);
        QCOMPARE(int(elementsNamed(r.spTree, "custGeom").size()), 0);
        const QVector<QDomElement> gds = elementsNamed(r.spTree, "gd");
        QCOMPARE(int(gds.size()), int(values.size()));
        for (int i = 0; i < values.size(); ++i) QCOMPARE(gds[i].attribute(QStringLiteral("fmla")), QStringLiteral("val %1").arg(qint64(values[i])));
        QCOMPARE(int(r.items.size()), 1);
        const ShapeItem *b = namedShape(r.items[0], shape);
        QVERIFY2(b, qPrintable(shape));
        QVERIFY2(sameRect(b->rect, frame, 0.01), qPrintable(dmlRectText(b->rect)));
        QCOMPARE(int(b->adj.size()), int(adj.size()));
        for (int i = 0; i < adj.size(); ++i) QVERIFY2(near(b->adj[i], adj[i]), qPrintable(QStringLiteral("adjustment %1: %2 for %3").arg(i).arg(b->adj[i]).arg(adj[i])));
    }

    void dmlArcPresetsReadFromAFileAreSavedAsThePreset_data()
    {
        QTest::addColumn<QString>("shape");
        QTest::addColumn<QVector<double>>("values");
        QTest::addColumn<QVector<double>>("adj");
        QTest::newRow("arc") << "arc" << QVector<double>{5400000, 10800000} << QVector<double>{0.25, 0.5};
        QTest::newRow("chord") << "chord" << QVector<double>{1350000, 5400000} << QVector<double>{0.0625, 0.25};
        QTest::newRow("pie") << "pie" << QVector<double>{5400000, 10800000} << QVector<double>{0.25, 0.5};
        QTest::newRow("block arc") << "blockArc" << QVector<double>{5400000, 16200000, 10000} << QVector<double>{0.25, 0.75, 0.1};
    }

    // A preset read from a .pptx is still that preset when it is moved and saved again: not a freeform, and not a rectangle.
    void dmlArcPresetsReadFromAFileAreSavedAsThePreset()
    {
        QFETCH(QString, shape);
        QFETCH(QVector<double>, values);
        QFETCH(QVector<double>, adj);
        QString gds;
        for (int i = 0; i < values.size(); ++i) gds += QStringLiteral("<a:gd name=\"adj%1\" fmla=\"val %2\"/>").arg(i + 1).arg(qint64(values[i]));
        DmlEnv env;
        ItemPtr it = dmlOne(env, dmlSp(dmlXfrm(20, 30, 136.6, 90) + QStringLiteral("<a:prstGeom prst=\"%1\"><a:avLst>%2</a:avLst></a:prstGeom>").arg(shape, gds)
                                       + dmlSolid(QStringLiteral("204080")) + QStringLiteral("<a:ln w=\"19050\">%1</a:ln>").arg(dmlSolid(QStringLiteral("802040")))));
        QVERIFY2(namedShape(it, shape), qPrintable(shape));
        it->moveBy(10, 5);
        WriteRound r;
        QVERIFY2(r.run({it}), qPrintable(r.error));
        QCOMPARE(int(elementsNamed(r.spTree, "custGeom").size()), 0);
        const QVector<QDomElement> presets = elementsNamed(r.spTree, "prstGeom");
        QCOMPARE(int(presets.size()), 1);
        QCOMPARE(presets[0].attribute(QStringLiteral("prst")), shape);
        const QVector<QDomElement> written = elementsNamed(r.spTree, "gd");
        QCOMPARE(int(written.size()), int(values.size()));
        for (int i = 0; i < values.size(); ++i) QCOMPARE(written[i].attribute(QStringLiteral("fmla")), QStringLiteral("val %1").arg(qint64(values[i])));
        QCOMPARE(int(r.items.size()), 1);
        const ShapeItem *b = namedShape(r.items[0], shape);
        QVERIFY2(b, qPrintable(shape));
        QVERIFY2(sameRect(b->rect, QRectF(30, 35, 136.6, 90), 0.01), qPrintable(dmlRectText(b->rect)));
        QCOMPARE(int(b->adj.size()), int(adj.size()));
        for (int i = 0; i < adj.size(); ++i) QVERIFY2(near(b->adj[i], adj[i]), qPrintable(QStringLiteral("adjustment %1").arg(i)));
    }

    void dmlWriteShapeHoldsItsText()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 160, 80);
        s->shape = QStringLiteral("roundRect");
        s->storyId = r.addStory(QStringLiteral("Inside"));
        s->insets = QMarginsF(5, 6, 7, 8);
        s->valign = jp::VAlign::Bottom;
        auto empty = std::make_shared<ShapeItem>();
        empty->rect = QRectF(200, 0, 50, 50);
        QVERIFY2(r.run({s, empty}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 2);
        const auto *a = static_cast<ShapeItem *>(r.items[0].get());
        QCOMPARE(r.storyOf(r.items[0])->toPlainText(), QStringLiteral("Inside"));
        QCOMPARE(a->insets, QMarginsF(5, 6, 7, 8));
        QVERIFY(a->valign == jp::VAlign::Bottom);
        // A shape with no text has a text body with none in it, which keeps where text would sit, and no story.
        QVERIFY(static_cast<ShapeItem *>(r.items[1].get())->storyId.isEmpty());
        QCOMPARE(int(elementsNamed(r.spTree, "txBody").size()), 2);
        QCOMPARE(static_cast<ShapeItem *>(r.items[1].get())->insets, empty->insets);
        QVERIFY(static_cast<ShapeItem *>(r.items[1].get())->valign == empty->valign);
    }

    void dmlWriteFreeformShapesKeepTheirPath()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(10, 20, 200, 100);
        s->shape = QStringLiteral("rect");
        QPainterPath p;
        p.moveTo(0, 100);
        p.lineTo(100, 0);
        p.cubicTo(150, 20, 190, 60, 200, 100);
        p.closeSubpath();
        p.moveTo(80, 90);
        p.lineTo(120, 90);
        p.lineTo(100, 50);   // an open one, left open
        p.moveTo(150, 90);
        p.cubicTo(170, 60, 190, 60, 150, 90);   // one that ends where it began, closed by nothing
        p.closeSubpath();
        s->customPath = p;
        s->fill = Fill::solid(ColorRef::rgb(Qt::green));
        s->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1);
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 1);
        const auto *b = static_cast<ShapeItem *>(r.items[0].get());
        QVERIFY(sameRect(b->rect, s->rect));
        QVERIFY(!b->customPath.isEmpty());
        QVERIFY(sameRect(b->customPath.boundingRect(), p.boundingRect(), 1e-3));
        QCOMPARE(int(elementsNamed(r.spTree, "custGeom").size()), 1);
        QCOMPARE(int(elementsNamed(r.spTree, "moveTo").size()), 3);
        QCOMPARE(int(elementsNamed(r.spTree, "cubicBezTo").size()), 2);
        QCOMPARE(int(elementsNamed(r.spTree, "close").size()), 2);
        // The path is the same path: the points of the curve and of both pieces.
        QCOMPARE(b->customPath.elementCount(), p.elementCount());
        for (int i = 0; i < p.elementCount(); ++i) {
            QVERIFY2(near(b->customPath.elementAt(i).x, p.elementAt(i).x, 1e-3) && near(b->customPath.elementAt(i).y, p.elementAt(i).y, 1e-3), qPrintable(QString::number(i)));
            QCOMPARE(int(b->customPath.elementAt(i).type), int(p.elementAt(i).type));
        }
    }

    void dmlWriteLibraryShapesWithoutAPresetBecomeFreeforms()
    {
        const jp::ShapeDef *def = jp::shapeDef(QStringLiteral("tab"));
        QVERIFY(def);
        QVERIFY(presetForShape(def->id).isEmpty());
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 120, 80);
        s->shape = def->id;
        QVERIFY2(r.run({s}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        const auto *b = static_cast<ShapeItem *>(r.items[0].get());
        QVERIFY(!b->customPath.isEmpty());
        const QRectF want = jp::shapePath(def->id, s->rect.size()).boundingRect();
        QVERIFY(sameRect(b->customPath.boundingRect(), want, 1e-3));
        QCOMPARE(int(elementsNamed(r.spTree, "prstGeom").size()), 0);
    }

    void dmlWriteFillsKeepTheirKind()
    {
        const auto through = [](const Fill &fill, WriteRound *keep = nullptr) {
            WriteRound local;
            WriteRound &r = keep ? *keep : local;
            const bool ok = r.runWith([&](QXmlStreamWriter &w) {
                w.writeStartElement(QStringLiteral("a:spPr"));
                writeFill(w, fill, r.ctx);
                w.writeEndElement();
            });
            if (!ok) return Fill::solid(ColorRef::rgb(Qt::magenta));
            return readFill(child(r.spTree, QString(ns::A), QStringLiteral("spPr")), QDomElement(), r.env.ctx);
        };
        // None, and a color that is none.
        QVERIFY(through(Fill::none()).isNone());
        QVERIFY(through(Fill::solid(ColorRef::none())).isNone());
        // Solid, with its transparency; and a scheme color is the color the scheme gives.
        Fill f = through(Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0xCC)), 0.4));
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("3366CC"));
        QVERIFY(near(f.transparency, 0.4, 1e-2));
        f = through(Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0xCC))));
        QCOMPARE(f.transparency, 0.0);
        WriteRound scheme;
        f = through(Fill::solid(ColorRef::scheme(jp::Accent2)), &scheme);
        QCOMPARE(dmlHex(f.color), hexOf(ColorRef::scheme(jp::Accent2).resolve(scheme.ctx.colors)));
        // A gradient: its stops with their places and clearness, and its direction.
        Fill g;
        g.type = Fill::Gradient;
        g.angle = 45;
        g.stops = {{0, ColorRef::rgb(QColor(0xFF, 0, 0)), 0}, {0.5, ColorRef::rgb(QColor(0, 0xFF, 0)), 0.5}, {1, ColorRef::rgb(QColor(0, 0, 0xFF)), 0}};
        f = through(g);
        QVERIFY(f.type == Fill::Gradient && f.gradType == Fill::Linear);
        QVERIFY(near(f.angle, 45, 1e-3));
        QCOMPARE(int(f.stops.size()), 3);
        QVERIFY(near(f.stops[1].pos, 0.5, 1e-4));
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FF0000"));
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("00FF00"));
        QCOMPARE(dmlHex(f.stops[2].color), QStringLiteral("0000FF"));
        QVERIFY(near(f.stops[1].transparency, 0.5, 1e-2));
        // A gradient of two colors and no stops has the two for stops.
        f = through(Fill::gradient(ColorRef::rgb(Qt::white), ColorRef::rgb(Qt::black), 90));
        QCOMPARE(int(f.stops.size()), 2);
        QCOMPARE(dmlHex(f.stops[0].color), QStringLiteral("FFFFFF"));
        QCOMPARE(dmlHex(f.stops[1].color), QStringLiteral("000000"));
        QVERIFY(near(f.angle, 90, 1e-3));
        const struct { Fill::GradType type; const char *path; } kinds[] = {{Fill::Radial, "circle"}, {Fill::Rectangular, "rect"}, {Fill::PathGrad, "shape"}};
        for (const auto &k : kinds) {
            WriteRound r;
            g.gradType = k.type;
            f = through(g, &r);
            QVERIFY2(f.type == Fill::Gradient && f.gradType == k.type, k.path);
            QCOMPARE(elementsNamed(r.spTree, "path").value(0).attribute(QStringLiteral("path")), QString::fromLatin1(k.path));
        }
        // A gradient with one stop is a plain color.
        Fill one;
        one.type = Fill::Gradient;
        one.stops = {{0.3, ColorRef::rgb(QColor(1, 2, 3)), 0}};
        f = through(one);
        QVERIFY(f.type == Fill::Solid);
        QCOMPARE(dmlHex(f.color), QStringLiteral("010203"));
        // Patterns: each of the library's, with its two colors.
        for (int i = 0; i < 13; ++i) {
            Fill p;
            p.type = Fill::Pattern;
            p.pattern = i;
            p.color = ColorRef::rgb(QColor(0x10, 0x20, 0x30));
            p.color2 = ColorRef::rgb(QColor(0xF0, 0xE0, 0xD0));
            f = through(p);
            QVERIFY2(f.type == Fill::Pattern && f.pattern == i, qPrintable(QString::number(i)));
            QCOMPARE(dmlHex(f.color), QStringLiteral("102030"));
            QCOMPARE(dmlHex(f.color2), QStringLiteral("F0E0D0"));
        }
        // Pictures: stretched, with some clearness, and tiled at a scale.
        WriteRound pictures;
        const QString id = pictures.addPicture(Qt::blue);
        Fill pic;
        pic.type = Fill::Picture;
        pic.imageId = id;
        pic.transparency = 0.3;
        f = through(pic, &pictures);
        QVERIFY(f.type == Fill::Picture && !f.tile);
        QVERIFY(near(f.transparency, 0.3, 1e-4));
        QCOMPARE(pictures.env.images.size(), 1);
        QCOMPARE(pictures.env.images[0].bytes, pictures.pictures[id]);
        WriteRound tiles;
        pic.imageId = tiles.addPicture(Qt::blue);
        pic.transparency = 0;
        pic.tile = true;
        pic.tileScale = 0.5;
        f = through(pic, &tiles);
        QVERIFY(f.type == Fill::Picture && f.tile);
        QVERIFY(near(f.tileScale, 0.5, 1e-4));
        // A texture is a picture that is tiled; a picture that isn't there is no fill.
        WriteRound textures;
        Fill tex;
        tex.type = Fill::Texture;
        tex.imageId = textures.addPicture(Qt::green);
        f = through(tex, &textures);
        QVERIFY(f.type == Fill::Picture && f.tile);
        QVERIFY(through(pic).isNone());
    }

    void dmlWriteLinesKeepTheirKind()
    {
        const auto through = [](const Stroke &stroke, WriteRound *keep = nullptr) {
            WriteRound local;
            WriteRound &r = keep ? *keep : local;
            const bool ok = r.runWith([&](QXmlStreamWriter &w) {
                w.writeStartElement(QStringLiteral("a:spPr"));
                writeLine(w, stroke, r.ctx);
                w.writeEndElement();
            });
            if (!ok) return Stroke::line(ColorRef::rgb(Qt::magenta), 99);
            return readLine(child(r.spTree, QString(ns::A), QStringLiteral("spPr")), QDomElement(), r.env.ctx);
        };
        Stroke s = through(Stroke::line(ColorRef::rgb(QColor(0x12, 0x34, 0x56)), 3.5));
        QVERIFY(!s.isNone());
        QVERIFY(near(s.width, 3.5));
        QCOMPARE(dmlHex(s.color), QStringLiteral("123456"));
        QCOMPARE(s.transparency, 0.0);
        QVERIFY(s.dash == Stroke::SolidLine && s.compound == Stroke::Single && s.cap == Qt::FlatCap && s.join == Qt::MiterJoin);
        QVERIFY(through(Stroke::none()).isNone());
        Stroke clear = Stroke::line(ColorRef::rgb(Qt::black), 0);
        QVERIFY(through(clear).isNone());
        s = Stroke::line(ColorRef::rgb(Qt::black), 1);
        s.transparency = 0.5;
        QVERIFY(near(through(s).transparency, 0.5, 1e-2));
        for (int d = int(Stroke::SolidLine); d <= int(Stroke::LongDashDotDot); ++d) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
            t.dash = Stroke::Dash(d);
            QVERIFY2(through(t).dash == t.dash, qPrintable(QString::number(d)));
        }
        for (Qt::PenCapStyle cap : {Qt::FlatCap, Qt::RoundCap, Qt::SquareCap}) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
            t.cap = cap;
            QVERIFY(through(t).cap == cap);
        }
        for (Qt::PenJoinStyle join : {Qt::MiterJoin, Qt::RoundJoin, Qt::BevelJoin}) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
            t.join = join;
            QVERIFY(through(t).join == join);
        }
        for (int c = int(Stroke::Single); c <= int(Stroke::Triple); ++c) {
            Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 4);
            t.compound = Stroke::Compound(c);
            QVERIFY2(through(t).compound == t.compound, qPrintable(QString::number(c)));
        }
        Stroke inset = Stroke::line(ColorRef::rgb(Qt::black), 4);
        inset.inset = true;
        QVERIFY(through(inset).inset);
        // Arrowheads: each kind and size at either end.
        for (int a = int(Arrow::None); a <= int(Arrow::Oval); ++a)
            for (int size = 0; size <= 2; ++size) {
                Stroke t = Stroke::line(ColorRef::rgb(Qt::black), 1);
                t.startArrow = Arrow(a);
                t.endArrow = Arrow((a + 1) % 6);
                t.startSize = size;
                t.endSize = 2 - size;
                const Stroke back = through(t);
                QVERIFY2(back.startArrow == t.startArrow && back.endArrow == t.endArrow, qPrintable(QString::number(a)));
                if (t.startArrow != Arrow::None) QCOMPARE(back.startSize, size);
                if (t.endArrow != Arrow::None) QCOMPARE(back.endSize, 2 - size);
            }
    }

    void dmlWriteEffectsKeepTheirShadowGlowAndSoftEdges()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(0, 0, 100, 60);
        s->fx.shadow.on = true;
        s->fx.shadow.color = ColorRef::rgb(QColor(0x40, 0x40, 0x40));
        s->fx.shadow.transparency = 0.5;
        s->fx.shadow.blur = 6;
        s->fx.shadow.distance = 4;
        s->fx.shadow.angle = 135;
        s->fx.glow.on = true;
        s->fx.glow.color = ColorRef::rgb(QColor(0xFF, 0xCC, 0));
        s->fx.glow.size = 8;
        s->fx.glow.transparency = 0.6;
        s->fx.softEdge = 3;
        auto inner = std::make_shared<ShapeItem>();
        inner->rect = QRectF(200, 0, 100, 60);
        inner->fx.shadow.on = true;
        inner->fx.shadow.inner = true;
        auto plain = std::make_shared<ShapeItem>();
        plain->rect = QRectF(400, 0, 100, 60);
        QVERIFY2(r.run({s, inner, plain}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 3);
        const jp::Effects fx = r.items[0]->fx;
        QVERIFY(fx.shadow.on && !fx.shadow.inner);
        QCOMPARE(dmlHex(fx.shadow.color), QStringLiteral("404040"));
        QVERIFY(near(fx.shadow.transparency, 0.5, 1e-2));
        QVERIFY(near(fx.shadow.blur, 6) && near(fx.shadow.distance, 4) && near(fx.shadow.angle, 135, 1e-3));
        QVERIFY(fx.glow.on);
        QCOMPARE(dmlHex(fx.glow.color), QStringLiteral("FFCC00"));
        QVERIFY(near(fx.glow.size, 8) && near(fx.glow.transparency, 0.6, 1e-2));
        QVERIFY(near(fx.softEdge, 3));
        QVERIFY(r.items[1]->fx.shadow.on && r.items[1]->fx.shadow.inner);
        QVERIFY(!r.items[2]->fx.any());
        QCOMPARE(int(elementsNamed(r.spTree, "effectLst").size()), 2);
    }

    void dmlWritePicturesAreStoredOnce()
    {
        WriteRound r;
        const QString red = r.addPicture(Qt::red), blue = r.addPicture(Qt::blue);
        auto make = [&](const QString &image, const QRectF &rect) {
            auto p = std::make_shared<PictureItem>();
            p->imageId = image;
            p->rect = rect;
            p->imgRect = QRectF(0, 0, rect.width(), rect.height());
            return p;
        };
        auto first = make(red, QRectF(10, 10, 200, 100));
        first->imgRect = QRectF(-20, -10, 240, 130);   // cut at the left, top, and bottom; and at the right
        first->name = QStringLiteral("Logo");
        first->altText = QStringLiteral("The logo");
        auto second = make(red, QRectF(300, 10, 100, 100));
        second->maskShape = QStringLiteral("ellipse");
        second->rotation = 90;
        second->transparency = 0.3;
        second->brightness = 20;
        second->contrast = -10;
        second->recolor = PictureItem::Grayscale;
        auto third = make(blue, QRectF(10, 200, 50, 50));
        third->imgRect = QRectF(5, 5, 40, 40);   // room is left around: a negative cut
        QVERIFY2(r.run({first, second, third}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        // Two pictures were stored, not three, and the package says what they are.
        QCOMPARE(r.media.size(), 2);
        QVERIFY(r.media.contains(red) && r.media.contains(blue));
        for (const QString &part : r.media) {
            QVERIFY2(r.env.package.hasPart(part), qPrintable(part));
            QCOMPARE(r.env.package.contentType(part), QStringLiteral("image/png"));
        }
        QCOMPARE(r.env.package.part(r.media[red]), r.pictures[red]);
        int pictureLinks = 0;
        for (const Relationship &rel : r.env.package.relationships(r.ctx.part)) pictureLinks += rel.type.endsWith(QLatin1String("/image"));
        QCOMPARE(pictureLinks, 2);
        QCOMPARE(int(r.items.size()), 3);
        QCOMPARE(r.env.images.size(), 2);
        const auto *a = static_cast<PictureItem *>(r.items[0].get());
        const auto *b = static_cast<PictureItem *>(r.items[1].get());
        const auto *c = static_cast<PictureItem *>(r.items[2].get());
        QCOMPARE(a->imageId, b->imageId);
        QVERIFY(a->imageId != c->imageId);
        QCOMPARE(r.env.images[0].bytes, r.pictures[red]);
        QCOMPARE(r.env.images[1].bytes, r.pictures[blue]);
        QVERIFY(sameRect(a->rect, first->rect));
        QVERIFY(sameRect(a->imgRect, first->imgRect, 5e-3));
        QCOMPARE(a->name, QStringLiteral("Logo"));
        QCOMPARE(a->altText, QStringLiteral("The logo"));
        QCOMPARE(a->maskShape, QStringLiteral("rect"));
        QCOMPARE(b->maskShape, QStringLiteral("ellipse"));
        QVERIFY(near(b->rotation, 90, 1e-4));
        QVERIFY(near(b->transparency, 0.3, 1e-4));
        QVERIFY(near(b->brightness, 20, 1e-3) && near(b->contrast, -10, 1e-3));
        QVERIFY(b->recolor == PictureItem::Grayscale);
        QVERIFY(sameRect(c->imgRect, third->imgRect, 5e-3));
        // A cut is a source rectangle and the picture is stretched over the frame.
        QVERIFY(r.text().contains(QStringLiteral("<a:srcRect")));
        QCOMPARE(int(elementsNamed(r.spTree, "srcRect").size()), 2);
        QCOMPARE(int(elementsNamed(r.spTree, "stretch").size()), 3);
    }

    void dmlWritePicturesKeepTheirFormat()
    {
        WriteRound r;
        QMap<QString, QPair<QString, QByteArray>> files;   // image id -> format named, bytes
        QImage img(6, 3, QImage::Format_RGB32);
        img.fill(Qt::darkCyan);
        for (const char *format : {"PNG", "JPG", "BMP"}) {
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            QVERIFY(img.save(&buffer, format));
            files.insert(QString::fromLatin1(format).toLower(), {QString::fromLatin1(format).toLower(), bytes});
        }
        files.insert(QStringLiteral("vector"), {QStringLiteral("svg"), "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"40\" height=\"20\"><rect width=\"40\" height=\"20\" fill=\"red\"/></svg>"});
        files.insert(QStringLiteral("metafile"), {QStringLiteral("emf"), QByteArray::fromHex("010000005800000000000000")});
        // A format the name of which is wrong or unknown is told by its bytes.
        files.insert(QStringLiteral("unnamed"), {QString(), files.value(QStringLiteral("png")).second});
        r.ctx.imageBytes = [&](const QString &id, QString *format) {
            if (format) *format = files.value(id).first;
            return files.value(id).second;
        };
        ItemList list;
        for (auto it = files.cbegin(); it != files.cend(); ++it) {
            auto p = std::make_shared<PictureItem>();
            p->imageId = it.key();
            p->rect = QRectF(0, 0, 40, 20);
            p->imgRect = QRectF(0, 0, 40, 20);
            p->name = it.key();
            list.push_back(p);
        }
        QVERIFY2(r.run(list), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        const QMap<QString, QPair<QString, QString>> want = {   // part's extension, type
            {"png", {"png", "image/png"}}, {"jpg", {"jpeg", "image/jpeg"}}, {"bmp", {"bmp", "image/bmp"}}, {"vector", {"png", "image/png"}},
            {"metafile", {"emf", "image/x-emf"}}, {"unnamed", {"png", "image/png"}}};
        QCOMPARE(r.media.size(), int(want.size()) + 1);   // and the vector picture's SVG, beside its pixels
        for (auto it = want.cbegin(); it != want.cend(); ++it) {
            const QString part = r.media.value(it.key());
            QVERIFY2(part.endsWith(QLatin1Char('.') + it.value().first), qPrintable(it.key() + ": " + part));
            QCOMPARE(r.env.package.contentType(part), it.value().second);
        }
        // A vector picture is drawn as pixels, and what was read back is a picture.
        QVERIFY(r.env.package.part(r.media.value(QStringLiteral("vector"))).startsWith("\x89PNG"));
        QImage drawn;
        QVERIFY(drawn.loadFromData(r.env.package.part(r.media.value(QStringLiteral("vector")))));
        QCOMPARE(QColor(drawn.pixel(drawn.width() / 2, drawn.height() / 2)), QColor(Qt::red));
        QCOMPARE(drawn.width() * 20, drawn.height() * 40);
        // And the SVG itself is linked from the blip, as the presentation program does, in a part of its own.
        const QString svgPart = r.media.value(QStringLiteral("svg:vector"));
        QVERIFY2(svgPart.endsWith(QLatin1String(".svg")), qPrintable(svgPart));
        QCOMPARE(r.env.package.contentType(svgPart), QStringLiteral("image/svg+xml"));
        QCOMPARE(r.env.package.part(svgPart), files.value(QStringLiteral("vector")).second);
        QCOMPARE(int(elementsNamed(r.spTree, "svgBlip").size()), 1);
        QSet<QString> parts;
        for (const QString &part : std::as_const(r.media)) parts << part;
        QCOMPARE(int(parts.size()), int(r.media.size()));   // every picture its own part
    }

    void dmlWritePicturesWithoutBytesAreLeftOutWithANote()
    {
        WriteRound r;
        auto p = std::make_shared<PictureItem>();
        p->imageId = QStringLiteral("gone");
        p->rect = QRectF(0, 0, 50, 50);
        QVERIFY2(r.run({p}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 0);
        QCOMPARE(int(r.skipped.size()), 1);
        QVERIFY(r.media.isEmpty());
    }

    void dmlWritePicturePlaceholderIsAShapeWithAPictureSlot()
    {
        WriteRound r;
        auto p = std::make_shared<PictureItem>();
        p->role = QStringLiteral("ph:pic:13");
        p->rect = QRectF(10, 20, 300, 200);
        QVERIFY2(r.run({p}), qPrintable(r.error));
        const QVector<QDomElement> phs = elementsNamed(r.spTree, "ph");
        QCOMPARE(int(phs.size()), 1);
        QCOMPARE(phs[0].attribute(QStringLiteral("type")), QStringLiteral("pic"));
        QCOMPARE(phs[0].attribute(QStringLiteral("idx")), QStringLiteral("13"));
        QCOMPARE(int(elementsNamed(r.spTree, "pic").size()), 0);
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Picture));
        QVERIFY(static_cast<PictureItem *>(r.items[0].get())->imageId.isEmpty());
        QVERIFY(sameRect(r.items[0]->rect, p->rect));
        QVERIFY(r.media.isEmpty());
    }

    void dmlWriteLinesKeepTheirEnds()
    {
        const QPointF a(10, 10), b(110, 60), c(10, 60), d(110, 10);
        const QPair<QPointF, QPointF> ends[] = {{a, b}, {b, a}, {c, d}, {d, c}, {a, d}, {a, c}};
        for (const auto &e : ends) {
            WriteRound r;
            auto l = std::make_shared<LineItem>();
            l->p1 = e.first;
            l->p2 = e.second;
            l->syncRect();
            l->stroke = Stroke::line(ColorRef::rgb(Qt::red), 2);
            l->stroke.startArrow = Arrow::Oval;
            l->stroke.endArrow = Arrow::Triangle;
            l->stroke.endSize = 2;
            l->name = QStringLiteral("Line");
            QVERIFY2(r.run({l}), qPrintable(r.error));
            QCOMPARE(int(r.items.size()), 1);
            QCOMPARE(int(r.items[0]->type()), int(ItemType::Line));
            const auto *back = static_cast<LineItem *>(r.items[0].get());
            QVERIFY2(near(back->p1.x(), e.first.x()) && near(back->p1.y(), e.first.y()), qPrintable(QStringLiteral("%1,%2").arg(e.first.x()).arg(e.first.y())));
            QVERIFY(near(back->p2.x(), e.second.x()) && near(back->p2.y(), e.second.y()));
            QVERIFY(back->route == LineItem::Straight);
            QVERIFY(near(back->stroke.width, 2));
            QVERIFY(back->stroke.startArrow == Arrow::Oval && back->stroke.endArrow == Arrow::Triangle && back->stroke.endSize == 2);
            QCOMPARE(int(elementsNamed(r.spTree, "cxnSp").size()), 1);
        }
    }

    void dmlWriteBentAndCurvedConnectorsKeepTheirRoute()
    {
        const QPair<QPointF, QPointF> ends[] = {{QPointF(10, 10), QPointF(110, 60)}, {QPointF(110, 60), QPointF(10, 10)},
                                                {QPointF(10, 60), QPointF(110, 10)}, {QPointF(110, 10), QPointF(10, 60)}};
        for (LineItem::Route route : {LineItem::Elbow, LineItem::Curved})
            for (const auto &e : ends)
                for (int sv = 0; sv < 2; ++sv)
                    for (int ev = 0; ev < 2; ++ev) {
                        WriteRound r;
                        auto l = std::make_shared<LineItem>();
                        l->p1 = e.first;
                        l->p2 = e.second;
                        l->route = route;
                        l->bend = 0.3;
                        l->startVertical = sv;
                        l->endVertical = ev;
                        l->syncRect();
                        const QString what = QStringLiteral("route %1 from %2,%3 to %4,%5 vertical %6 %7")
                                                 .arg(int(route)).arg(e.first.x()).arg(e.first.y()).arg(e.second.x()).arg(e.second.y()).arg(sv).arg(ev);
                        QVERIFY2(r.run({l}), qPrintable(r.error));
                        QVERIFY2(r.items.size() == 1 && r.items[0]->type() == ItemType::Line, qPrintable(what));
                        const auto *back = static_cast<LineItem *>(r.items[0].get());
                        QVERIFY2(back->route == route, qPrintable(what));
                        QVERIFY2(near(back->p1.x(), e.first.x()) && near(back->p1.y(), e.first.y()) && near(back->p2.x(), e.second.x()) && near(back->p2.y(), e.second.y()), qPrintable(what));
                        QVERIFY2(back->startVertical == bool(sv) && back->endVertical == bool(ev), qPrintable(what));
                        // The route's corners are where they were (a corner of one turn is the same corner).
                        const QVector<QPointF> was = l->routePoints(), now = back->routePoints();
                        QVERIFY2(was.size() == now.size(), qPrintable(what));
                        for (int i = 0; i < was.size(); ++i) QVERIFY2(near(was[i].x(), now[i].x(), 1e-4) && near(was[i].y(), now[i].y(), 1e-4), qPrintable(what));
                        if (sv == ev) QVERIFY2(near(back->bend, 0.3, 1e-4), qPrintable(what));
                    }
    }

    void dmlWriteConnectorsStayAttachedToTheirShapes()
    {
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(10, 10, 80, 40);
        auto circle = std::make_shared<ShapeItem>();
        circle->rect = QRectF(200, 100, 60, 60);
        circle->shape = QStringLiteral("ellipse");
        auto cloud = std::make_shared<ShapeItem>();
        cloud->rect = QRectF(300, 10, 60, 60);
        cloud->shape = QStringLiteral("cloud");
        auto late = std::make_shared<TextItem>();
        late->rect = QRectF(300, 200, 60, 60);
        auto l = std::make_shared<LineItem>();
        l->p1 = QPointF(90, 30);
        l->p2 = QPointF(200, 130);
        l->start = LineItem::Glue{box->id, 3};
        l->end = LineItem::Glue{circle->id, 0};
        l->syncRect();
        // The second is attached to a shape that comes after it, and to one that has no points to join to.
        auto early = std::make_shared<LineItem>();
        early->p1 = QPointF(0, 0);
        early->p2 = QPointF(50, 50);
        early->start = LineItem::Glue{late->id, 1};
        early->end = LineItem::Glue{cloud->id, 2};
        early->syncRect();
        const ItemList list{box, circle, l, early, cloud, late};
        // Written as they come, a connector is attached to what was written before it.
        {
            WriteRound r;
            r.run(list);
            QCOMPARE(int(r.items.size()), 6);
            const auto *back = static_cast<LineItem *>(r.items[2].get());
            QCOMPARE(back->start.id, r.items[0]->id);
            QCOMPARE(back->start.site, 3);
            QCOMPARE(back->end.id, r.items[1]->id);
            QCOMPARE(back->end.site, 0);
            const auto *loose = static_cast<LineItem *>(r.items[3].get());
            QVERIFY(loose->start.id.isEmpty() && loose->end.id.isEmpty());
            QVERIFY(r.text().contains(QStringLiteral("stCxn")));
        }
        // With the numbers given first, to what is written after it too.
        {
            WriteRound r;
            QVERIFY2(r.runWith([&](QXmlStreamWriter &w) {
                assignShapeIds(list, r.ctx);
                for (const ItemPtr &it : list) writeShape(w, *it, r.ctx);
            }), qPrintable(r.error));
            r.items = readShapeTree(r.spTree, r.env.ctx);
            QCOMPARE(int(r.items.size()), 6);
            const auto *early2 = static_cast<LineItem *>(r.items[3].get());
            QCOMPARE(early2->start.id, r.items[5]->id);
            QCOMPARE(early2->start.site, 1);
            QVERIFY(early2->end.id.isEmpty());   // a cloud has no points to join
            QCOMPARE(static_cast<LineItem *>(r.items[2].get())->start.id, r.items[0]->id);
            // The numbers are the ones the shapes are written with.
            QSet<int> numbers;
            for (const ItemPtr &it : r.items) numbers << it->sourceId;
            QCOMPARE(int(numbers.size()), 6);
        }
    }

    // What can't be written is given no number, so nothing is attached to it.
    void dmlWriteGivesNoNumberToWhatItCannotWrite()
    {
        WriteRound r;
        auto table = std::make_shared<jp::TableItem>();
        auto gone = std::make_shared<PictureItem>();
        gone->imageId = QStringLiteral("gone");
        auto empty = std::make_shared<GroupItem>();
        auto shape = std::make_shared<ShapeItem>();
        const ItemList list{table, gone, empty, shape};
        assignShapeIds(list, r.ctx);
        QCOMPARE(int(r.ctx.shapeIds.size()), 1);
        QVERIFY(r.ctx.shapeIds.contains(shape->id));
        QCOMPARE(r.ctx.shapeIds.value(shape->id), 2);
    }

    void dmlWriteGroupsKeepTheirChildren()
    {
        WriteRound r;
        auto text = writtenTextBox(r, QRectF(20, 30, 100, 40), QStringLiteral("In a group"));
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(150, 40, 60, 60);
        shape->shape = QStringLiteral("ellipse");
        auto a = std::make_shared<ShapeItem>(), b = std::make_shared<ShapeItem>();
        a->rect = QRectF(300, 300, 30, 30);
        b->rect = QRectF(340, 320, 30, 30);
        b->rotation = 45;
        auto inner = std::make_shared<GroupItem>();
        inner->children = {a, b};
        inner->syncRect();
        auto g = std::make_shared<GroupItem>();
        g->children = {text, shape, inner};
        g->syncRect();
        g->name = QStringLiteral("Group A");
        g->altText = QStringLiteral("Three things");
        QVERIFY2(r.run({g}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Group));
        const auto *back = static_cast<GroupItem *>(r.items[0].get());
        QCOMPARE(back->name, QStringLiteral("Group A"));
        QCOMPARE(back->altText, QStringLiteral("Three things"));
        QCOMPARE(int(back->children.size()), 3);
        QVERIFY(sameRect(back->children[0]->rect, text->rect));
        QVERIFY(sameRect(back->children[1]->rect, shape->rect));
        QCOMPARE(static_cast<ShapeItem *>(back->children[1].get())->shape, QStringLiteral("ellipse"));
        QCOMPARE(int(back->children[2]->type()), int(ItemType::Group));
        const auto *innerBack = static_cast<GroupItem *>(back->children[2].get());
        QCOMPARE(int(innerBack->children.size()), 2);
        QVERIFY(sameRect(innerBack->children[0]->rect, a->rect));
        QVERIFY(sameRect(innerBack->children[1]->rect, b->rect));
        QVERIFY(near(innerBack->children[1]->rotation, 45, 1e-4));
        QVERIFY(sameRect(back->bounds(), g->bounds(), 1e-3));
        // The group's frame is its children's: the child space is the page's.
        const QVector<QDomElement> xfrms = elementsNamed(r.spTree, "chOff");
        QVERIFY(!xfrms.isEmpty());
        const QDomElement x = xfrms[0].parentNode().toElement();
        QCOMPARE(child(x, QString(ns::A), QStringLiteral("off")).attribute(QStringLiteral("x")), child(x, QString(ns::A), QStringLiteral("chOff")).attribute(QStringLiteral("x")));
        QCOMPARE(child(x, QString(ns::A), QStringLiteral("ext")).attribute(QStringLiteral("cx")), child(x, QString(ns::A), QStringLiteral("chExt")).attribute(QStringLiteral("cx")));
        // A group with nothing in it is left out.
        WriteRound empty;
        QVERIFY2(empty.run({std::make_shared<GroupItem>()}), qPrintable(empty.error));
        QCOMPARE(int(empty.items.size()), 0);
        QCOMPARE(int(empty.skipped.size()), 1);
    }

    void dmlWritePlaceholdersKeepTheirTypeAndIndex_data()
    {
        QTest::addColumn<QString>("role");
        QTest::addColumn<QString>("type");
        QTest::addColumn<QString>("idx");
        QTest::newRow("title") << "ph:title" << "title" << "";
        QTest::newRow("centered title") << "ph:ctrTitle" << "ctrTitle" << "";
        QTest::newRow("subtitle") << "ph:subTitle:1" << "subTitle" << "1";
        QTest::newRow("body") << "ph:body:1" << "body" << "1";
        QTest::newRow("content has no type") << "ph:obj" << "" << "";
        QTest::newRow("second content") << "ph:obj:2" << "" << "2";
        QTest::newRow("date") << "ph:dt:10" << "dt" << "10";
        QTest::newRow("footer") << "ph:ftr:11" << "ftr" << "11";
        QTest::newRow("slide number") << "ph:sldNum:12" << "sldNum" << "12";
        QTest::newRow("big index") << "ph:body:4294967295" << "body" << "4294967295";
    }

    void dmlWritePlaceholdersKeepTheirTypeAndIndex()
    {
        QFETCH(QString, role);
        QFETCH(QString, type);
        QFETCH(QString, idx);
        WriteRound r;
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
        t->role = role;
        QVERIFY2(r.run({t}), qPrintable(r.error));
        const QVector<QDomElement> phs = elementsNamed(r.spTree, "ph");
        QCOMPARE(int(phs.size()), 1);
        QCOMPARE(phs[0].hasAttribute(QStringLiteral("type")), !type.isEmpty());
        QCOMPARE(phs[0].attribute(QStringLiteral("type")), type);
        QCOMPARE(phs[0].hasAttribute(QStringLiteral("idx")), !idx.isEmpty());
        QCOMPARE(phs[0].attribute(QStringLiteral("idx")), idx);
        // A placeholder is no text box, and its place in the layout is not to be moved by the group.
        QVERIFY(!r.text().contains(QStringLiteral("txBox")));
        QVERIFY(r.text().contains(QStringLiteral("noGrp=\"1\"")));
        QCOMPARE(int(r.items.size()), 1);
        QCOMPARE(int(r.items[0]->type()), int(ItemType::Text));
    }

    void dmlWriteOtherRolesAreNotPlaceholders()
    {
        WriteRound r;
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
        t->role = QStringLiteral("title");
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QCOMPARE(int(elementsNamed(r.spTree, "ph").size()), 0);
        QVERIFY(r.text().contains(QStringLiteral("txBox=\"1\"")));
    }

    void dmlWriteKeepsTheNumbersAFileGave()
    {
        WriteRound r;
        r.ctx.nextShapeId = 2;
        r.ctx.usedIds = {1, 5};
        r.ctx.reservedIds = {3, 7, 20, 21};
        auto shape = [&](int source) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = QRectF(0, 0, 10, 10);
            s->sourceId = source;
            return s;
        };
        auto a = shape(7), fresh = shape(0), c = shape(3), duplicate = shape(7), one = shape(1), used = shape(5);
        auto e = shape(21), f = shape(0);
        auto g = std::make_shared<GroupItem>();
        g->children = {e, f};
        g->syncRect();
        g->sourceId = 20;
        QVERIFY2(r.run({a, fresh, c, duplicate, one, used, g}), qPrintable(r.error));
        // The numbers a file gave are kept where free; a new shape takes the next that no one has asked for.
        QCOMPARE(int(r.items.size()), 7);
        QCOMPARE(r.items[0]->sourceId, 7);
        QCOMPARE(r.items[1]->sourceId, 2);
        QCOMPARE(r.items[2]->sourceId, 3);
        QCOMPARE(r.items[3]->sourceId, 4);   // 7 was taken
        QCOMPARE(r.items[4]->sourceId, 6);   // 1 is the shape tree's own
        QCOMPARE(r.items[5]->sourceId, 8);   // 5 was taken before
        QCOMPARE(r.items[6]->sourceId, 20);
        const auto *back = static_cast<GroupItem *>(r.items[6].get());
        QCOMPARE(back->children[0]->sourceId, 21);
        QCOMPARE(back->children[1]->sourceId, 9);
        QSet<int> seen;
        for (const QDomElement &p : elementsNamed(r.spTree, "cNvPr")) {
            QVERIFY2(!seen.contains(p.attribute(QStringLiteral("id")).toInt()), qPrintable(p.attribute(QStringLiteral("id"))));
            seen << p.attribute(QStringLiteral("id")).toInt();
        }
        QCOMPARE(int(seen.size()), 9);
        QVERIFY(r.ctx.usedIds.contains(7) && r.ctx.usedIds.contains(21) && r.ctx.usedIds.contains(9));
        // What the context says each item got.
        QCOMPARE(r.ctx.shapeIds.value(a->id), 7);
        QCOMPARE(r.ctx.shapeIds.value(duplicate->id), 4);
        QCOMPARE(r.ctx.shapeIds.value(e->id), 21);
    }

    void dmlWriteNumbersNewShapesFromTheContext()
    {
        WriteRound r;
        r.ctx.nextShapeId = 100;
        auto a = std::make_shared<ShapeItem>();
        auto b = std::make_shared<LineItem>();
        auto t = writtenTextBox(r, QRectF(0, 0, 10, 10), QStringLiteral("x"));
        auto p = std::make_shared<PictureItem>();
        p->imageId = r.addPicture();
        QVERIFY2(r.run({a, b, t, p}), qPrintable(r.error));
        QCOMPARE(int(r.items.size()), 4);
        QCOMPARE(r.items[0]->sourceId, 100);
        QCOMPARE(r.items[1]->sourceId, 101);
        QCOMPARE(r.items[2]->sourceId, 102);
        QCOMPARE(r.items[3]->sourceId, 103);
        QCOMPARE(r.ctx.nextShapeId, 104);
    }

    // A table with no cells is left out with a note; text art is written as
    // a box whose text is warped into it.
    void dmlWriteLeavesTablesWithNoCellsOutWithANote()
    {
        WriteRound r;
        auto table = std::make_shared<jp::TableItem>();
        table->name = QStringLiteral("Prices");
        auto art = std::make_shared<TextArtItem>();
        art->text = QStringLiteral("Sale");
        QVERIFY2(r.run({table, art}), qPrintable(r.error));
        QCOMPARE(int(r.skipped.size()), 1);
        QVERIFY(r.skipped[0].contains(QStringLiteral("Prices")));
        const QVector<QDomElement> warps = elementsNamed(r.spTree, "prstTxWarp");
        QCOMPARE(int(warps.size()), 1);
        QCOMPARE(warps[0].attribute(QStringLiteral("prst")), QStringLiteral("textPlain"));
    }

    // A table is a frame of the cells' own fills, lines, and text; the cells
    // that are hidden under a merged one say how.
    void dmlWriteTablesKeepTheirCellsAndMerges()
    {
        WriteRound r;
        auto t = std::make_shared<jp::TableItem>();
        t->name = QStringLiteral("Table <1>");
        t->rect = QRectF(20, 30, 1, 1);
        t->rows = 3;
        t->cols = 3;
        t->colW = {100, 80, 60};
        t->rowH = {30, 30, 40};
        t->cells.resize(9);
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col) {
                jp::TableCell &c = t->cell(row, col);
                c.storyId = r.addStory(QStringLiteral("r%1c%2").arg(row).arg(col));
                c.margins = QMarginsF(5, 3, 5, 3);
            }
        t->cell(0, 0).colSpan = 2;
        t->cell(0, 0).rowSpan = 2;
        for (const auto &at : {QPair<int, int>{0, 1}, {1, 0}, {1, 1}}) t->cell(at.first, at.second).covered = true;
        t->cell(0, 0).fill = Fill::solid(ColorRef::rgb(QColor(0xFF, 0, 0)));
        t->cell(0, 0).border.top = Stroke::line(ColorRef::rgb(QColor(0, 0, 0xFF)), 2);
        t->cell(1, 2).border.left = Stroke::line(ColorRef::rgb(QColor(0, 0x80, 0)), 1);   // and the cell on its left, which has none, shares it
        jp::TableCell &last = t->cell(2, 2);
        last.margins = QMarginsF(1, 2, 3, 4);
        last.valign = jp::VAlign::Bottom;
        last.vertical = true;
        last.diagonal = 1;
        last.diagonalLine = Stroke::line(ColorRef::rgb(Qt::black), 0.5);
        t->fill = Fill::solid(ColorRef::rgb(QColor(0xEE, 0xEE, 0xEE)));
        t->stroke = Stroke::line(ColorRef::rgb(QColor(0x10, 0x20, 0x30)), 3);
        QVERIFY2(r.run({t}), qPrintable(r.error));
        QVERIFY2(r.skipped.isEmpty(), qPrintable(r.skipped.join('\n')));
        // The frame's place and the table's size, from its columns and rows.
        const QDomElement frame = elementsNamed(r.spTree, "graphicFrame").value(0);
        QVERIFY(!frame.isNull());
        const QDomElement xfrm = child(frame, QString(ns::P), QStringLiteral("xfrm"));
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("off")).attribute(QStringLiteral("x")), QStringLiteral("254000"));
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("ext")).attribute(QStringLiteral("cx")), QStringLiteral("3048000"));
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("ext")).attribute(QStringLiteral("cy")), QStringLiteral("1270000"));
        QCOMPARE(child(child(frame, QString(ns::P), QStringLiteral("nvGraphicFramePr")), QString(ns::P), QStringLiteral("cNvPr")).attribute(QStringLiteral("name")), QStringLiteral("Table <1>"));
        QStringList widths, heights;
        for (const QDomElement &g : elementsNamed(frame, "gridCol")) widths << g.attribute(QStringLiteral("w"));
        for (const QDomElement &row : elementsNamed(frame, "tr")) heights << row.attribute(QStringLiteral("h"));
        QCOMPARE(widths, (QStringList{"1270000", "1016000", "762000"}));
        QCOMPARE(heights, (QStringList{"381000", "381000", "508000"}));
        const QVector<QDomElement> cells = elementsNamed(frame, "tc");
        QCOMPARE(int(cells.size()), 9);
        // A merged cell says how far it reaches; those under it say that they are, across or down or both.
        QCOMPARE(cells[0].attribute(QStringLiteral("gridSpan")), QStringLiteral("2"));
        QCOMPARE(cells[0].attribute(QStringLiteral("rowSpan")), QStringLiteral("2"));
        QVERIFY(cells[1].attribute(QStringLiteral("hMerge")) == QLatin1String("1") && !cells[1].hasAttribute(QStringLiteral("vMerge")));
        QVERIFY(cells[3].attribute(QStringLiteral("vMerge")) == QLatin1String("1") && !cells[3].hasAttribute(QStringLiteral("hMerge")));
        QVERIFY(cells[4].attribute(QStringLiteral("hMerge")) == QLatin1String("1") && cells[4].attribute(QStringLiteral("vMerge")) == QLatin1String("1"));
        QVERIFY(!cells[2].hasAttribute(QStringLiteral("gridSpan")) && !cells[2].hasAttribute(QStringLiteral("hMerge")));
        auto textOf = [&](int i) {
            QString out;
            for (const QDomElement &tt : elementsNamed(cells[i], "t")) out += tt.text();
            return out;
        };
        QCOMPARE(textOf(0), QStringLiteral("r0c0"));
        QCOMPARE(textOf(5), QStringLiteral("r1c2"));
        QCOMPARE(textOf(1), QString());   // hidden: no text
        auto props = [&](int i) { return child(cells[i], QString(ns::A), QStringLiteral("tcPr")); };
        auto line = [&](int i, const char *side) { return child(props(i), QString(ns::A), QString::fromLatin1(side)); };
        auto fillColor = [&](int i) { return child(child(props(i), QString(ns::A), QStringLiteral("solidFill")), QString(ns::A), QStringLiteral("srgbClr")).attribute(QStringLiteral("val")); };
        // Fills: its own, and the table's where it has none.
        QCOMPARE(fillColor(0), QStringLiteral("FF0000"));
        QCOMPARE(fillColor(2), QStringLiteral("EEEEEE"));
        // Lines: the cell's own, the table's round its edge, and one shared with the cell next to it.
        QCOMPARE(line(0, "lnT").attribute(QStringLiteral("w")), QStringLiteral("38100"));   // the table's own line is on its edge, over the cell's
        QCOMPARE(child(child(line(5, "lnL"), QString(ns::A), QStringLiteral("solidFill")), QString(ns::A), QStringLiteral("srgbClr")).attribute(QStringLiteral("val")), QStringLiteral("008000"));
        QVERIFY(!child(line(5, "lnT"), QString(ns::A), QStringLiteral("noFill")).isNull());
        QCOMPARE(line(2, "lnR").attribute(QStringLiteral("w")), QStringLiteral("38100"));
        // The text's place in the cell.
        QCOMPARE(props(8).attribute(QStringLiteral("marL")), QStringLiteral("12700"));
        QCOMPARE(props(8).attribute(QStringLiteral("marT")), QStringLiteral("25400"));
        QCOMPARE(props(8).attribute(QStringLiteral("marR")), QStringLiteral("38100"));
        QCOMPARE(props(8).attribute(QStringLiteral("marB")), QStringLiteral("50800"));
        QCOMPARE(props(8).attribute(QStringLiteral("anchor")), QStringLiteral("b"));
        QCOMPARE(props(8).attribute(QStringLiteral("vert")), QStringLiteral("vert"));
        QVERIFY(!line(8, "lnTlToBr").isNull() && line(8, "lnBlToTr").isNull());
        QCOMPARE(props(0).attribute(QStringLiteral("anchor")), QStringLiteral("t"));
        // The table is no shape that is read back as one.
        QCOMPARE(int(r.items.size()), 0);
    }

    void dmlWriteElementsComeInTheSchemasOrder()
    {
        WriteRound r;
        const QString id = r.addStory();
        QTextCursor c(r.story(id));
        QTextCharFormat cf;
        cf.setFontPointSize(18);
        cf.setFontWeight(QFont::Bold);
        cf.setFontFamilies({QStringLiteral("Carlito")});
        cf.setForeground(QColor(1, 2, 3));
        cf.setProperty(jp::tp::ColorRefP, QStringLiteral("#010203"));
        cf.setProperty(jp::tp::HighlightRefP, QStringLiteral("#FFFF00"));
        cf.setAnchor(true);
        cf.setAnchorHref(QStringLiteral("https://example.com/"));
        QTextBlockFormat bf;
        bf.setLineHeight(110, QTextBlockFormat::ProportionalHeight);
        bf.setTopMargin(3);
        bf.setBottomMargin(4);
        bf.setTabPositions({QTextOption::Tab(50, QTextOption::LeftTab)});
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("rich"), cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setProperty(jp::tp::BulletFont, QStringLiteral("Arial"));
        lf.setProperty(jp::tp::BulletColor, QStringLiteral("#FF0000"));
        lf.setProperty(jp::tp::BulletSize, 24.0);
        c.createList(lf);
        c.insertBlock();
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(0, 0, 200, 100);
        t->storyId = id;
        t->role = QStringLiteral("ph:body:1");
        t->hyperlink = QStringLiteral("https://example.com/item");
        t->fill = Fill::gradient(ColorRef::rgb(Qt::red), ColorRef::rgb(Qt::blue));
        t->stroke = Stroke::line(ColorRef::rgb(Qt::black), 2);
        t->stroke.dash = Stroke::DashLine;
        t->stroke.join = Qt::RoundJoin;
        t->stroke.startArrow = Arrow::Oval;
        t->stroke.endArrow = Arrow::Triangle;
        t->fx.shadow.on = true;
        t->fx.glow.on = true;
        t->fx.softEdge = 2;
        t->autofit = TextItem::ShrinkOnOverflow;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(250, 0, 100, 100);
        s->shape = QStringLiteral("roundRect");
        s->adj = {0.3};
        s->storyId = r.addStory(QStringLiteral("shape text"));
        auto p = std::make_shared<PictureItem>();
        p->rect = QRectF(400, 0, 100, 100);
        p->imageId = r.addPicture();
        p->imgRect = QRectF(-10, -10, 120, 120);
        p->hyperlink = QStringLiteral("https://example.com/pic");
        auto l = std::make_shared<LineItem>();
        l->p1 = QPointF(0, 150);
        l->p2 = QPointF(100, 250);
        l->route = LineItem::Elbow;
        auto g = std::make_shared<GroupItem>();
        g->children = {std::make_shared<ShapeItem>()};
        g->syncRect();
        auto free = std::make_shared<ShapeItem>();
        free->rect = QRectF(0, 300, 50, 50);
        QPainterPath path;
        path.addEllipse(0, 0, 50, 50);
        free->customPath = path;
        Fill picFill;
        picFill.type = Fill::Picture;
        picFill.imageId = p->imageId;
        free->fill = picFill;
        auto table = std::make_shared<jp::TableItem>();
        table->rows = 2;
        table->cols = 2;
        table->colW = {50, 50};
        table->rowH = {20, 20};
        table->cells.resize(4);
        for (jp::TableCell &cell : table->cells) {
            cell.storyId = r.addStory(QStringLiteral("cell"));
            cell.fill = Fill::gradient(ColorRef::rgb(Qt::red), ColorRef::rgb(Qt::blue));
            cell.border.left = cell.border.right = cell.border.top = cell.border.bottom = Stroke::line(ColorRef::rgb(Qt::black), 1);
        }
        table->cell(0, 0).diagonal = 2;
        table->cell(0, 0).diagonalLine = Stroke::line(ColorRef::rgb(Qt::black), 1);
        QVERIFY2(r.run({t, s, p, l, g, free, table}), qPrintable(r.error));
        const QMap<QString, QStringList> orders = {
            {"sp", {"nvSpPr", "spPr", "style", "txBody"}},
            {"pic", {"nvPicPr", "blipFill", "spPr", "style"}},
            {"cxnSp", {"nvCxnSpPr", "spPr", "style"}},
            {"grpSp", {"nvGrpSpPr", "grpSpPr", "sp|grpSp|graphicFrame|cxnSp|pic"}},
            {"nvSpPr", {"cNvPr", "cNvSpPr", "nvPr"}},
            {"nvPicPr", {"cNvPr", "cNvPicPr", "nvPr"}},
            {"nvCxnSpPr", {"cNvPr", "cNvCxnSpPr", "nvPr"}},
            {"nvGrpSpPr", {"cNvPr", "cNvGrpSpPr", "nvPr"}},
            {"cNvPr", {"hlinkClick", "hlinkHover"}},
            {"spPr", {"xfrm", "custGeom|prstGeom", "noFill|solidFill|gradFill|blipFill|pattFill|grpFill", "ln", "effectLst"}},
            {"grpSpPr", {"xfrm"}},
            {"xfrm", {"off", "ext", "chOff", "chExt"}},
            {"custGeom", {"avLst", "gdLst", "ahLst", "cxnLst", "rect", "pathLst"}},
            {"ln", {"noFill|solidFill|gradFill|pattFill", "prstDash|custDash", "round|bevel|miter", "headEnd", "tailEnd"}},
            {"effectLst", {"glow", "innerShdw", "outerShdw", "softEdge"}},
            {"gradFill", {"gsLst", "lin|path"}},
            {"blipFill", {"blip", "srcRect", "tile|stretch"}},
            {"txBody", {"bodyPr", "lstStyle", "p"}},
            {"bodyPr", {"noAutofit|normAutofit|spAutoFit"}},
            {"p", {"pPr", "r|br|fld", "endParaRPr"}},
            {"pPr", {"lnSpc", "spcBef", "spcAft", "buClrTx|buClr", "buSzTx|buSzPct|buSzPts", "buFontTx|buFont", "buNone|buAutoNum|buChar|buBlip", "tabLst", "defRPr"}},
            {"rPr", {"ln", "noFill|solidFill|gradFill|blipFill|pattFill|grpFill", "effectLst", "highlight", "uLnTx|uLn", "uFillTx|uFill", "latin", "ea", "cs", "sym", "hlinkClick|hlinkMouseOver", "rtl"}},
            {"r", {"rPr", "t"}},
            {"graphicFrame", {"nvGraphicFramePr", "xfrm", "graphic"}},
            {"nvGraphicFramePr", {"cNvPr", "cNvGraphicFramePr", "nvPr"}},
            {"tbl", {"tblPr", "tblGrid", "tr"}},
            {"tc", {"txBody", "tcPr"}},
            {"tcPr", {"lnL", "lnR", "lnT", "lnB", "lnTlToBr", "lnBlToTr", "noFill|solidFill|gradFill|blipFill|pattFill|grpFill"}},
            {"lnL", {"noFill|solidFill|gradFill|pattFill", "prstDash|custDash", "round|bevel|miter", "headEnd", "tailEnd"}},
        };
        QVector<QDomElement> stack{r.spTree};
        int checked = 0;
        while (!stack.isEmpty()) {
            const QDomElement e = stack.takeLast();
            const auto rule = orders.constFind(e.localName());
            if (rule != orders.constEnd()) {
                const QStringList names = childNamesOf(e);
                QVERIFY2(inSequence(names, *rule), qPrintable(e.localName() + ": " + names.join(' ')));
                ++checked;
            }
            for (QDomElement k = e.firstChildElement(); !k.isNull(); k = k.nextSiblingElement()) stack << k;
        }
        QVERIFY(checked > 40);
        // The parts every element must have.
        for (const QDomElement &p2 : elementsNamed(r.spTree, "cNvPr")) {
            QVERIFY(p2.hasAttribute(QStringLiteral("id")));
            QVERIFY(p2.hasAttribute(QStringLiteral("name")));
        }
        for (const QDomElement &e : elementsNamed(r.spTree, "fld")) QVERIFY(e.hasAttribute(QStringLiteral("id")));
        for (const QDomElement &e : elementsNamed(r.spTree, "xfrm")) QVERIFY(child(e, QString(ns::A), QStringLiteral("off")).hasAttribute(QStringLiteral("x")));
    }

    void dmlWriteTheFrameOfAnItemIsInEmuAndWhole()
    {
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(-12.5, 1e9, -5, 0.00001);   // a shape out of the page, and one with no size
        s->rotation = -90.25;
        s->flipV = true;
        QVERIFY2(r.run({s}), qPrintable(r.error));
        const QDomElement xfrm = child(child(elementsNamed(r.spTree, "sp").value(0), QString(ns::P), QStringLiteral("spPr")), QString(ns::A), QStringLiteral("xfrm"));
        QVERIFY(!xfrm.isNull());
        bool ok = false;
        const qint64 rot = xfrm.attribute(QStringLiteral("rot")).toLongLong(&ok);
        QVERIFY(ok && rot >= 0 && rot < 21600000);
        QVERIFY(near(rot / 60000.0, 269.75, 1e-6));
        QCOMPARE(xfrm.attribute(QStringLiteral("flipV")), QStringLiteral("1"));
        QVERIFY(!xfrm.hasAttribute(QStringLiteral("flipH")));
        const QDomElement ext = child(xfrm, QString(ns::A), QStringLiteral("ext"));
        QVERIFY(ext.attribute(QStringLiteral("cx")).toLongLong() >= 0);
        QVERIFY(ext.attribute(QStringLiteral("cy")).toLongLong() >= 0);
        QCOMPARE(child(xfrm, QString(ns::A), QStringLiteral("off")).attribute(QStringLiteral("x")), QStringLiteral("-158750"));
    }

    // A context with nothing in it (no package, no pictures, no stories) writes what it can, and leaves out the rest.
    void dmlWriteWithAnEmptyContextWritesWhatItCan()
    {
        DrawingWriteContext ctx;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        QStringList skipped;
        QByteArray out;
        QXmlStreamWriter w(&out);
        w.writeStartElement(QStringLiteral("p:spTree"));
        auto shape = std::make_shared<ShapeItem>();
        shape->hyperlink = QStringLiteral("https://example.com/");
        auto text = std::make_shared<TextItem>();
        text->storyId = QStringLiteral("none");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = QStringLiteral("none");
        auto line = std::make_shared<LineItem>();
        auto group = std::make_shared<GroupItem>();
        group->children = {shape, text, pic};
        for (const ItemPtr &it : ItemList{shape, text, pic, line, group}) writeShape(w, *it, ctx, &skipped);
        writeShape(w, *shape, ctx);   // and no list of what is left out
        w.writeEndElement();
        QDomDocument dom;
        QVERIFY(parseXml("<p:spTree " + dmlNamespaces().toUtf8() + ">" + out.mid(out.indexOf('>') + 1), &dom));
        QCOMPARE(int(skipped.size()), 2);   // the picture, and the one inside the group
        QVERIFY(!out.contains("hlinkClick"));
        QVERIFY(!out.contains("r:embed"));
        // The text of a story that isn't there is one empty paragraph.
        QVERIFY(out.contains("<a:p><a:endParaRPr"));
    }

    // Numbers that aren't numbers, or are far out of reach, do not make a file that can't be read.
    void dmlWriteHoldsOddNumbersToWhatAFileCanHold()
    {
        const double nan = std::numeric_limits<double>::quiet_NaN(), inf = std::numeric_limits<double>::infinity();
        WriteRound r;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(nan, inf, -inf, 1e300);
        s->rotation = nan;
        s->shape = QStringLiteral("roundRect");
        s->adj = {nan};
        s->stroke = Stroke::line(ColorRef::rgb(Qt::black), inf);
        s->stroke.startSize = 99;
        s->stroke.startArrow = Arrow::Oval;
        s->fx.shadow.on = true;
        s->fx.shadow.blur = nan;
        s->fx.shadow.distance = 1e300;
        s->fx.shadow.angle = inf;
        s->fx.softEdge = -5;
        s->fx.glow.on = true;
        s->fx.glow.size = nan;
        QPainterPath p;
        p.moveTo(nan, 0);
        p.lineTo(1e300, -1e300);
        p.lineTo(5, 5);
        s->customPath = p;
        auto t = writtenTextBox(r, QRectF(0, 0, 100, 50), QStringLiteral("x"));
        t->insets = QMarginsF(nan, inf, -1e300, 1e300);
        t->fitScale = nan;
        t->autofit = TextItem::ShrinkOnOverflow;
        t->columns = 1000000;
        t->columnGap = nan;
        QTextCursor c(r.story(t->storyId));
        QTextCharFormat cf;
        cf.setFontPointSize(1e300);
        cf.setProperty(jp::tp::KernAbove, nan);
        cf.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        cf.setFontLetterSpacing(inf);
        c.insertText(QStringLiteral("odd"), cf);
        QTextBlockFormat bf;
        bf.setLineHeight(1e300, QTextBlockFormat::ProportionalHeight);
        bf.setTopMargin(inf);
        bf.setLeftMargin(-inf);
        c.setBlockFormat(bf);
        auto l = std::make_shared<LineItem>();
        l->p1 = QPointF(nan, nan);
        l->p2 = QPointF(inf, -inf);
        l->route = LineItem::Elbow;
        l->bend = nan;
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = r.addPicture();
        pic->rect = QRectF(nan, nan, nan, nan);
        pic->imgRect = QRectF(0, 0, 1e-300, 1e300);
        pic->brightness = nan;
        pic->transparency = inf;
        QVERIFY2(r.run({s, t, l, pic}), qPrintable(r.error));
        QVERIFY(!r.text().contains(QLatin1String("nan")) && !r.text().contains(QLatin1String("inf")));
        for (const QDomElement &e : elementsNamed(r.spTree, "ext")) {
            QVERIFY(e.attribute(QStringLiteral("cx")).toLongLong() >= 0);
            QVERIFY(e.attribute(QStringLiteral("cy")).toLongLong() >= 0);
        }
    }
};

JP_TEST_CLASS(OfficeDmlWriteTests)
#include "tst_office_dmlwrite.moc"
