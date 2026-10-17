// JeffPub's tests of stand-in fonts, letter widths, and bold drawn by hand.
#include "pubhelpers.h"
#include "../timing.h"

class PubFontTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // A run with no letter spacing of its own takes its paragraph style's
    // (a sign's headline style tightens its letters 87.5% and 3 points; they
    // came in at normal spacing, 8% wider than Publisher drew them).
    void styleLetterSpacingFromPub()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        jp::TextStyle st;
        st.name = QStringLiteral("Tight Headline");
        st.chr.setProperty(jp::tp::Tracking, 87.5);
        st.chr.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        st.chr.setFontLetterSpacing(-3);
        doc->styles << st;
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 400, 100);
        box->storyId = doc->createStory();
        QTextDocument *sd = doc->storyDoc(box->storyId);
        QTextCursor c(sd);
        QTextBlockFormat bf;
        bf.setProperty(jp::tp::StyleName, st.name);
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontPointSize(40);
        c.insertText(QStringLiteral("Help Wanted"), cf);   // no spacing of its own
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("tight.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QTextDocument *story = nullptr;
        for (auto it = back->stories.cbegin(); it != back->stories.cend(); ++it)
            if ((*it)->doc->toPlainText() == QLatin1String("Help Wanted")) story = (*it)->doc.get();
        QVERIFY(story);
        QTextCursor r(story);
        r.movePosition(QTextCursor::NextCharacter);
        QCOMPARE(jp::tp::trackingOf(r.charFormat()), 87.5);
        QCOMPARE(jp::tp::kerningOf(r.charFormat()), -3.0);
    }

    // Tracking is measured in the original font's average letter width
    // (Publisher's PDFs: 125% on Gill Sans MT Ext Condensed Bold adds 0.056
    // em a letter, 87.5% on Century Schoolbook Bold takes 0.066); a squeezed
    // stand-in's own average spread the letters twice as far.
    void trackingUsesTheOriginalFontsAverage()
    {
        QVERIFY(std::abs(jp::originalAverageCharEm(QStringLiteral("Arial"), false, false) - 904.0 / 2048) < 1e-4);   // as the old table had it
        const double ext = jp::originalAverageCharEm(QStringLiteral("Gill Sans MT Ext Condensed Bold"), true, false);   // one face only
        QVERIFY2(std::abs(0.25 * ext - 0.056) < 0.003, qPrintable(QString::number(ext)));
        const double csb = jp::originalAverageCharEm(QStringLiteral("Century Schoolbook"), true, false);
        QVERIFY2(std::abs(0.125 * csb - 0.066) < 0.002, qPrintable(QString::number(csb)));
        QCOMPARE(jp::originalAverageCharEm(QStringLiteral("No Such Font"), false, false), -1.0);
    }

    // A missing stock font's letters take the original's widths (measured
    // in the reference VM), not its stand-in's, so lines break where they
    // break in Publisher (Gill Sans MT's "W" is 1.04 em; its stand-in's
    // is its own).
    void missingFontsKeepTheirLetterWidths()
    {
        const QString fam = QStringLiteral("Gill Sans MT");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Gill Sans MT is installed here");
        const QHash<char16_t, double> *widths = jp::originalLetterWidths(fam, false, false);
        QVERIFY(widths);
        QVERIFY(std::abs(widths->value(u'W') - 1.042) < 0.001);
        QVERIFY(jp::originalLetterWidths(fam, true, false) != widths);
        QVERIFY(!jp::originalLetterWidths(QStringLiteral("No Such Font"), false, false));
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{fam});
        cf.setFontPointSize(12);   // below automatic kerning
        const QString text = QStringLiteral("Wilhelmina 2047 ");
        c.insertText(text, cf);
        jp::FrameSpec fs;
        fs.size = QSizeF(1000, 100);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        jp::StoryLayout lay;
        lay.build(&doc, {fs}, jp::LayoutEnv());
        auto x = [&](int pos) {
            int frame = -1;
            QRectF r;
            lay.caretRect(pos, &frame, &r);
            return r.x();
        };
        // Each letter is centered in the original's width, so words (and
        // the spaces after them) start and end where the original's do.
        double want = 0;
        for (int i = 0; i < text.size(); ++i) {
            want += widths->value(text[i].unicode()) * 12;
            if (i + 1 < text.size() && text[i + 1] != QLatin1Char(' ')) continue;
            const double got = x(i + 1) - x(0);
            QVERIFY2(std::abs(got - want) < 0.015, qPrintable(QStringLiteral("after '%1': %2, Gill Sans MT %3").arg(text[i]).arg(got).arg(want)));
        }
    }

    // From 14 pt up, a missing stock font's letters kern by the original's
    // pairs: Gill Sans MT Bold's "Yo" closes up by 0.14 em, as in a banner
    // of Publisher's. Smaller, they don't kern.
    void missingFontsKernAsTheOriginal()
    {
        const QString fam = QStringLiteral("Gill Sans MT");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Gill Sans MT is installed here");
        const QHash<char16_t, double> *widths = jp::originalLetterWidths(fam, true, false);
        const QHash<quint32, double> *kerning = jp::originalKerning(fam, true, false);
        QVERIFY(widths && kerning);
        const double yo = kerning->value(quint32(u'Y') << 16 | u'o');
        QVERIFY2(std::abs(yo + 0.140) < 0.001, qPrintable(QString::number(yo)));
        for (double size : {100.0, 12.0}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontWeight(QFont::Bold);
            cf.setFontPointSize(size);
            c.insertText(QStringLiteral("Yo you"), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(2000, 400);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            auto x = [&](int pos) {
                int frame = -1;
                QRectF r;
                lay.caretRect(pos, &frame, &r);
                return r.x();
            };
            const double want = (widths->value(u'Y') + widths->value(u'o') + (size >= 14 ? yo : 0)) * size;
            QVERIFY2(std::abs(x(2) - x(0) - want) < 0.01 * size / 12, qPrintable(QStringLiteral("%1 pt: %2, not %3").arg(size).arg(x(2) - x(0)).arg(want)));
        }
    }

    // A missing stock font's run is cut into pieces of their own letter
    // spacing, and a cut never parts characters that belong together: an
    // emoji's two UTF-16 halves (drawn as two replacement diamonds), a letter
    // and its combining accent (drawn beside it), or the letters of a joined
    // script (the last drawn in its isolated form). Only two characters the
    // original's table measures are ever cut apart.
    void letterWidthPiecesKeepCharactersTogether()
    {
        const QString fam = QStringLiteral("Gill Sans MT");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Gill Sans MT is installed here");
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{fam});
        cf.setFontPointSize(24);
        const QTextCharFormat rf = jp::resolveCharFormat(cf, jp::LayoutEnv());
        // Where the second and later pieces of a run of `text` start.
        auto cuts = [&](const QString &text) {
            QStringList at;
            for (const auto &r : jp::applyLetterWidths({QTextLayout::FormatRange{0, int(text.size()), rf}}, text))
                if (r.start > 0) at << QString::number(r.start);
            return at;
        };
        auto cutAt = [&](const QString &text, int pos) { return cuts(text).contains(QString::number(pos)); };
        // Plain letters do get a piece each, or nearly, so the rest proves something.
        QVERIFY2(cuts(QStringLiteral("abcdefghij")).size() >= 3, qPrintable(cuts(QStringLiteral("abcdefghij")).join(',')));
        // "ab", an emoji (two units), "cd".
        const QString emoji = QStringLiteral("ab\U0001F600cd");
        for (int pos : {2, 3, 4}) QVERIFY2(!cutAt(emoji, pos), qPrintable(QStringLiteral("cut at %1: %2").arg(pos).arg(cuts(emoji).join(','))));
        // "re" and an accent, "sume" and an accent (decomposed).
        const QString accents = QStringLiteral("re\u0301sume\u0301");
        for (int pos : {2, 3, 7}) QVERIFY2(!cutAt(accents, pos), qPrintable(QStringLiteral("cut at %1: %2").arg(pos).arg(cuts(accents).join(','))));
        // Four Arabic letters and a digit: nothing but the digit is measured.
        const QString arabic = QStringLiteral("\u0645\u0643\u062A\u0628" "1");
        QVERIFY2(cuts(arabic).isEmpty(), qPrintable(cuts(arabic).join(',')));
        // A joiner, a non-joiner, and a variation selector hold letters together.
        for (const QString &s : {QStringLiteral("a\u200Db"), QStringLiteral("a\u200Cb"), QStringLiteral("a\uFE0Fb")})
            QVERIFY2(cuts(s).isEmpty(), qPrintable(cuts(s).join(',')));
    }

    // One run of true small capitals in a font mustn't change how that font's
    // other runs are measured: the stand-in's advances were remembered by font
    // alone, so every normal run after it was set by the small capitals'
    // narrower letters (Impact: "banana band nab" came to 601 px for 475).
    void smallCapitalsLeaveLaterRunsLetterWidths()
    {
        const QString fam = QStringLiteral("Rockwell Extra Bold");   // used by no other test, so this one measures it first
        if (QFontDatabase::hasFamily(fam)) QSKIP("Rockwell Extra Bold is installed here");
        const QHash<char16_t, double> *widths = jp::originalLetterWidths(fam, false, false);
        QVERIFY(widths);
        const QString text = QStringLiteral("banana band nab ");
        for (bool smallCaps : {true, false}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontPointSize(12);
            if (smallCaps) {
                cf.setFontCapitalization(QFont::SmallCaps);
                cf.setProperty(jp::tp::TrueSmallCaps, true);
            }
            c.insertText(text, cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(1000, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            if (smallCaps) continue;   // laid out first only so that it is measured first
            auto x = [&](int pos) {
                int frame = -1;
                QRectF r;
                lay.caretRect(pos, &frame, &r);
                return r.x();
            };
            double want = 0;
            for (int i = 0; i < text.size(); ++i) {
                want += widths->value(text[i].unicode()) * 12;
                if (i + 1 < text.size() && text[i + 1] != QLatin1Char(' ')) continue;
                const double got = x(i + 1) - x(0);
                QVERIFY2(std::abs(got - want) < 0.1, qPrintable(QStringLiteral("after '%1': %2, Rockwell Extra Bold %3").arg(text[i]).arg(got).arg(want)));
            }
        }
    }

    // Book Antiqua's lines are as Publisher spaces them, by the font's
    // Windows metrics (1.2427 em; bold 1.2056, its extra space above the
    // letters): a garage sale sign's two best-fit lines filled the box
    // exactly there, and came out 11% closer here (typo metrics, 1.07 em).
    void bookAntiquaLineSpacing()
    {
        for (const auto &[bold, line, base] : {std::tuple{false, 1.2427, 0.9604}, std::tuple{true, 1.2056, 0.9404}}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Book Antiqua")});
            cf.setFontPointSize(100);
            if (bold) cf.setFontWeight(QFont::Bold);
            c.insertText(QStringLiteral("Garage"), cf);
            c.insertBlock();
            c.insertText(QStringLiteral("Sale"), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(600, 400);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            const auto lines = lay.lineInfo(0);
            QCOMPARE(lines.size(), 2);
            QVERIFY2(std::abs(lines[1].baseline - lines[0].baseline - line * 100) < 0.05, qPrintable(QString::number(lines[1].baseline - lines[0].baseline)));
            QVERIFY2(std::abs(lines[0].baseline - base * 100) < 0.05, qPrintable(QString::number(lines[0].baseline)));
        }
    }

    // Rockwell Condensed and Gloucester MT Extra Condensed are heavier than
    // their stand-ins' regular faces narrowed: line by line against
    // Publisher's PDFs (Oct 10), Roboto Slab Regular at 71% carried 0.85 of
    // Rockwell Condensed's ink and EB Garamond Regular at 72% only 0.6 of
    // Gloucester's. Their bold faces, narrowed to the original's letter
    // widths (60% and 62%), carry 1.0 and 0.85; Rockwell Condensed Bold
    // wants the ExtraBold (0.9 in Roboto Slab Bold at 86%), and Gloucester
    // too (0.85 in EB Garamond Bold).
    void condensedStandInsAreDrawnBold()
    {
        struct Case { const char *family, *standIn; int regular, bold, regularWeight, boldWeight; };
        for (const Case &c : {Case{"Rockwell Condensed", "Roboto Slab", 60, 86, 700, 800}, Case{"Gloucester MT Extra Condensed", "EB Garamond", 62, 62, 800, 800}}) {
            const QString fam = QString::fromLatin1(c.family);
            if (QFontDatabase::hasFamily(fam)) continue;   // installed: its own face
            QCOMPARE(jp::substituteWeight(fam), c.regularWeight);
            QCOMPARE(jp::substituteWeight(fam, true), c.boldWeight);
            for (bool bold : {false, true}) {
                QTextCharFormat cf;
                cf.setFontFamilies(QStringList{fam});
                cf.setFontPointSize(24);
                if (bold) cf.setFontWeight(QFont::Bold);
                const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
                const QFontInfo info(r.font());
                const QString what = QStringLiteral("%1 %2").arg(fam, bold ? QStringLiteral("bold") : QStringLiteral("regular"));
                QCOMPARE(info.family(), QString::fromLatin1(c.standIn));
                QVERIFY2(info.weight() >= (bold ? c.boldWeight : c.regularWeight) - 50, qPrintable(what + QStringLiteral(": weight %1").arg(info.weight())));
                QCOMPARE(r.fontStretch(), jp::stretchToAsk(bold ? c.bold : c.regular));
            }
        }
    }

    // Three missing fonts' stand-ins carried less ink than Publisher's
    // letters in the bundled Regular and Bold faces (Oct 10, line by line
    // against its PDFs): Tw Cen MT's regular runs are drawn in Jost Medium,
    // Rockwell Condensed's bold runs in Roboto Slab ExtraBold, Gloucester MT
    // Extra Condensed's in EB Garamond ExtraBold. Each face is a style of
    // its family, asked for by weight and by name (the Mac's font matching
    // gave Libre Franklin Bold for the ExtraBold it was asked), and every
    // platform draws the weight asked: Medium 500, ExtraBold 800.
    void heavierStandInFaces()
    {
        // How much ink a face puts down: the letters drawn, the alpha summed.
        const auto inkOf = [](const QFont &font) {
            QImage img(900, 160, QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            {
                QPainter p(&img);
                p.setFont(font);
                p.setPen(Qt::black);
                p.drawText(QPointF(10, 100), QStringLiteral("Hamburgefonstiv Wxyz"));
            }
            double sum = 0;
            for (int y = 0; y < img.height(); ++y) {
                const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
                for (int x = 0; x < img.width(); ++x) sum += qAlpha(row[x]);
            }
            return sum;
        };
        // weight 0: not checked (an italic run's face is the family's italic one);
        // heavier: carries more ink than the family's own regular (500) or bold (800);
        // stretch: percent (Medium is wider than Jost Regular, whose 94 became 85).
        struct Case { const char *family, *standIn, *style; bool bold, italic; int weight; bool heavier; int stretch; };
        for (const Case &c : {Case{"Tw Cen MT", "Jost", "Medium", false, false, 500, true, 85}, Case{"Tw Cen MT", "Jost", "Bold", true, false, 700, false, 89},
                              Case{"Tw Cen MT", "Jost", "Italic", false, true, 0, false, 94},
                              Case{"Rockwell Condensed", "Roboto Slab", "Bold", false, false, 700, false, 60},
                              Case{"Rockwell Condensed", "Roboto Slab", "ExtraBold", true, false, 800, true, 86},
                              Case{"Gloucester MT Extra Condensed", "EB Garamond", "ExtraBold", false, false, 800, true, 62},
                              Case{"Gloucester MT Extra Condensed", "EB Garamond", "ExtraBold", true, false, 800, true, 62}}) {
            const QString fam = QString::fromLatin1(c.family), standIn = QString::fromLatin1(c.standIn), style = QString::fromLatin1(c.style);
            if (QFontDatabase::hasFamily(fam)) continue;   // installed: its own face
            const QString what = QStringLiteral("%1 %2%3").arg(fam, c.bold ? QStringLiteral("bold") : QStringLiteral("regular"), c.italic ? QStringLiteral(" italic") : QString());
            // The bundled file is a style of the family, not a family of its own.
            QVERIFY2(QFontDatabase::styles(standIn).contains(style, Qt::CaseInsensitive), qPrintable(standIn + QStringLiteral(" lists: ") + QFontDatabase::styles(standIn).join(QLatin1Char(','))));
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontPointSize(24);
            if (c.bold) cf.setFontWeight(QFont::Bold);
            if (c.italic) cf.setFontItalic(true);
            const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
            const QFontInfo info(r.font());
            const QString drawn = QStringLiteral("%1: %2 %3 %4").arg(what, info.family(), info.styleName()).arg(info.weight());
            QCOMPARE(info.family(), standIn);
            QCOMPARE(info.italic(), c.italic);
            QCOMPARE(r.fontStretch(), jp::stretchToAsk(c.stretch));
            if (c.weight >= 750) QVERIFY2(info.weight() >= 750, qPrintable(drawn));
            else if (c.weight == 500) QVERIFY2(info.weight() == 500, qPrintable(drawn));
            else if (c.weight) QVERIFY2(info.weight() >= c.weight - 50 && info.weight() <= c.weight + 50, qPrintable(drawn));
            // Named when a weight alone could land on its neighbor (never an italic run: that name is the upright face).
            if (c.weight == 500 || c.weight == 800) QCOMPARE(r.fontStyleName().toString(), style);
            else QVERIFY(r.fontStyleName().toString().isEmpty());
            if (c.heavier) {
                QFont heavy = r.font(), plain = r.font();
                heavy.setStretch(QFont::Unstretched);
                plain.setStretch(QFont::Unstretched);
                plain.setStyleName(QString());
                plain.setWeight(c.weight >= 750 ? QFont::Bold : QFont::Normal);
                plain.setPixelSize(48);
                heavy.setPixelSize(48);
                QVERIFY2(inkOf(heavy) > inkOf(plain) * 1.03, qPrintable(drawn + QStringLiteral(": ink %1, the family's plain face %2").arg(inkOf(heavy)).arg(inkOf(plain))));
            }
        }
    }

    // The heavier face doesn't change how tall the letters are drawn:
    // Rockwell Condensed's regular letters stand as high as Roboto Slab's
    // (its capitals 3% taller, its small letters 3% shorter), its bold ones
    // 5.5% lower than the stand-in's.
    void rockwellCondensedKeepsItsHeights()
    {
        const QString fam = QStringLiteral("Rockwell Condensed");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Rockwell Condensed is installed here");
        for (bool bold : {false, true}) {
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontPointSize(24);
            if (bold) cf.setFontWeight(QFont::Bold);
            const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
            if (bold) QCOMPARE(r.property(jp::tp::GlyphScaleY).toDouble(), 0.945);
            else QVERIFY(!r.hasProperty(jp::tp::GlyphScaleY));
        }
    }

    // Publisher spaces the four stock fonts above by their Windows metrics
    // (hhea and win agree in each): measured against its PDFs, a paragraph's
    // lines of Gill Sans MT and Tw Cen MT sit within 0.05% of these, at
    // 90% to 133% spacing, bold the same.
    void stockFontsLineHeights()
    {
        struct Case { const char *family; double em; };
        for (const Case &c : {Case{"Gill Sans MT", (1903.0 + 472) / 2048}, Case{"Tw Cen MT", (1753.0 + 477) / 2048},
                              Case{"Rockwell Condensed", (1927.0 + 481) / 2048}, Case{"Gloucester MT Extra Condensed", (1936.0 + 446) / 2048}})
            for (bool bold : {false, true}) {
                QFont f(QString::fromLatin1(c.family));
                f.setFamilies({QString::fromLatin1(c.family)});
                f.setBold(bold);
                f.setPointSizeF(12);
                const double em = jp::naturalLineEm(f, QString::fromLatin1(c.family));
                QVERIFY2(std::abs(em - c.em) < 1e-6, qPrintable(QStringLiteral("%1: %2, not %3").arg(QLatin1String(c.family)).arg(em).arg(c.em)));
            }
    }

    // An octagon with no proportion of its own is Publisher's regular one,
    // its corners cut at 29.3% of each side (6326 of 21600, as its PDFs
    // draw a stop sign); libmspub had 5000, a squatter octagon.
    void octagonAsPublisherDrawsIt()
    {
        const libmspub::CustomShape *octagon = libmspub::getCustomShape(libmspub::OCTAGON);
        QVERIFY(octagon && octagon->m_numDefaultAdjustValues > 0);
        QCOMPARE(octagon->mp_defaultAdjustValues[0], 6326);
    }

    // Text pasted from a web page can leave a font named "inherit" (the CSS
    // word) in the font table. It is no font: Publisher draws that text in
    // the document's usual one, so it reads back in that, not in whatever a
    // missing family falls back to (a wider face that overflowed the box).
    void fontNamedInheritIsTheUsualFont()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 300, 100);
        box->storyId = doc->createStory();
        QTextCursor c(doc->storyDoc(box->storyId));
        QTextCharFormat usual, pasted;
        usual.setFontFamilies(QStringList{QStringLiteral("Georgia")});
        pasted.setFontFamilies(QStringList{QStringLiteral("inherit")});
        c.insertText(QStringLiteral("Usual text"), usual);
        c.insertBlock();
        c.insertText(QStringLiteral("Pasted text"), pasted);
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("inherit.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        int checked = 0;
        for (auto it = back->stories.cbegin(); it != back->stories.cend(); ++it) {
            const QTextBlock b = (*it)->doc->begin();
            if (!b.text().startsWith(QLatin1String("Usual"))) continue;
            for (QTextBlock x = b; x.isValid(); x = x.next())
                for (auto f = x.begin(); !f.atEnd(); ++f) {
                    QCOMPARE(f.fragment().charFormat().fontFamilies().toStringList().value(0), QStringLiteral("Georgia"));
                    ++checked;
                }
        }
        QVERIFY(checked >= 2);
        QVERIFY(!back->pubFonts.contains(QStringLiteral("inherit")));
    }

    // A paragraph's layout font carries no capitals of its own: Qt would draw
    // every run of the paragraph in the first run's, so "KEY POINTS (example)"
    // came out "KEY POINTS (EXAMPLE)". The layout makes capitals itself, run by run.
    void firstRunCapitalsStayInTheirRun()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat caps, plain;
        caps.setFontCapitalization(QFont::AllUppercase);
        c.insertText(QStringLiteral("KEY POINTS ("), caps);
        c.insertText(QStringLiteral("example"), plain);
        jp::LayoutEnv env;
        QCOMPARE(jp::baseFontFor(doc.begin(), env).capitalization(), QFont::MixedCase);
        QTextDocument small;
        QTextCursor sc(&small);
        QTextCharFormat sm;
        sm.setFontCapitalization(QFont::SmallCaps);
        sc.insertText(QStringLiteral("Key points "), sm);
        sc.insertText(QStringLiteral("example"), plain);
        QCOMPARE(jp::baseFontFor(small.begin(), env).capitalization(), QFont::MixedCase);
    }

    // The width in points of the first stem of a capital I, set in `family`
    // at `size`, as it is drawn for `out`.
    static double handBoldStem(const QString &family, bool bold, double size, jp::TextOutput out, double scale = 4)
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(320, 320));
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("t");
        t->rect = QRectF(20, 10, 280, 300);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->storyId = doc->createStory(QStringLiteral("I"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies(QStringList{family});
            f.setFontPointSize(size);
            if (bold) f.setFontWeight(QFont::Bold);
            c.mergeCharFormat(f);
        }
        doc->pages[0]->items.push_back(t);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        ctx.opt.textOutput = out;
        const QImage img = Renderer::renderToImage(ctx, 0, scale);
        int top = -1, bottom = -1;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) < 128) {
                    if (top < 0) top = y;
                    bottom = y;
                    break;
                }
        if (top < 0) return 0;
        const int y = (top + bottom) / 2;
        // The first stem's ink, in pixels, counting edge pixels by their share of the text's own darkness.
        double ink = 0, darkest = 0;
        bool in = false;
        for (int x = 0; x < img.width(); ++x) {
            const double d = 1 - qGray(img.pixel(x, y)) / 255.0;
            if (d > 0.02) { ink += d; darkest = std::max(darkest, d); in = true; }
            else if (in) break;
        }
        return ink / darkest / scale;
    }

    static bool handBoldFontsInstalled()
    {
        for (const char *f : {"Arial Black", "Rockwell Extra Bold", "Franklin Gothic Demi"})
            if (QFontDatabase::hasFamily(QString::fromLatin1(f))) return true;
        return false;
    }

    // Bold drawn by hand (a font with no bold face), on the screen and in
    // pictures: Publisher's picture of Franklin Gothic Demi set bold has
    // stems 0.017 em wider than the plain (0.84 pt at 48 pt, whatever the
    // picture's resolution), and Arial Black and Rockwell Extra Bold, bold or
    // not, are as wide as the font draws them (stems 10.65 and 12.63 pt at
    // 48 pt, the same set bold). What is laid out doesn't change with where
    // the text is drawn: its layout is shared.
    void handBoldOnTheScreenAndInPictures()
    {
        if (handBoldFontsInstalled()) QSKIP("these fonts are installed here");
        using namespace jp;
        const TextOutput pic = TextOutput::Picture;
        for (const char *name : {"Arial Black", "Rockwell Extra Bold"}) {
            const QString fam = QString::fromLatin1(name);
            const double plain = handBoldStem(fam, false, 48, pic), bold = handBoldStem(fam, true, 48, pic);
            QVERIFY2(plain > 5 && std::abs(bold - plain) < 0.12, qPrintable(QStringLiteral("%1: plain %2, bold %3").arg(fam).arg(plain).arg(bold)));
        }
        const QString demi = QStringLiteral("Franklin Gothic Demi");
        for (double size : {24.0, 48.0, 96.0}) {
            const double plain = handBoldStem(demi, false, size, pic), bold = handBoldStem(demi, true, size, pic);
            QVERIFY2(std::abs(bold - plain - 0.017 * size) < 0.12 + 0.002 * size, qPrintable(QStringLiteral("%1 pt: plain %2, bold %3").arg(size).arg(plain).arg(bold)));
        }
        // One layout for the screen, a PDF, and the printer.
        auto doc = Document::blank(QSizeF(320, 320));
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("t");
        t->rect = QRectF(20, 10, 280, 300);
        t->storyId = doc->createStory(QStringLiteral("Arial Black text"));
        QTextCursor c(doc->storyDoc(t->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("Arial Black")});
        f.setFontPointSize(30);
        c.mergeCharFormat(f);
        doc->pages[0]->items.push_back(t);
        LayoutCache cache;
        RenderOptions screen, pdf, paper;
        pdf.textOutput = TextOutput::Pdf;
        paper.textOutput = TextOutput::Print;
        const auto *a = cache.textFrame(*doc, *t, 1, screen).layout;
        QVERIFY(a);
        QCOMPARE(cache.textFrame(*doc, *t, 1, pdf).layout, a);
        QCOMPARE(cache.textFrame(*doc, *t, 1, paper).layout, a);
    }

    // Into pixels at screen resolution a letter drawn bold by hand grows a
    // whole pixel while it is under 30 pixels to the em, not 0.017 em:
    // Publisher's pictures of 10 point Franklin Gothic Book set bold have 1.78
    // times the ink of the plain at 96 dpi, 1.56 times at 150, and 1.24 times
    // at 300 (24 point: 1.23 at all three, and 1.24 for 8 to 24 point at 300).
    void handBoldGrowsAPixelInSmallLettersOnScreen()
    {
        if (handBoldFontsInstalled()) QSKIP("these fonts are installed here");
        using namespace jp;
        const QString demi = QStringLiteral("Franklin Gothic Demi"), black = QStringLiteral("Arial Black");
        const TextOutput pic = TextOutput::Picture;
        auto growth = [&](const QString &fam, double size, double dpi) {
            const double scale = dpi / 72;
            return handBoldStem(fam, true, size, pic, scale) - handBoldStem(fam, false, size, pic, scale);
        };
        QVERIFY2(std::abs(growth(demi, 10, 96) - 0.75) < 0.2, qPrintable(QString::number(growth(demi, 10, 96))));   // one pixel
        QVERIFY2(std::abs(growth(demi, 10, 150) - 0.48) < 0.2, qPrintable(QString::number(growth(demi, 10, 150))));
        QVERIFY2(std::abs(growth(demi, 10, 300) - 0.17) < 0.12, qPrintable(QString::number(growth(demi, 10, 300))));   // 42 pixels to the em: 0.017 em
        QVERIFY2(std::abs(growth(demi, 24, 96) - 0.41) < 0.2, qPrintable(QString::number(growth(demi, 24, 96))));
        QVERIFY(std::abs(growth(black, 10, 96)) < 0.1);
    }

    // The same in a PDF Publisher exports: every such letter, Arial Black and
    // Rockwell Extra Bold plain or bold and Franklin Gothic Demi when bold
    // (not when plain), is stroked 1/35 em (stems 1.37 pt wider at 48 pt;
    // all three export intents alike). From 216 pt Publisher sets the text as
    // outlines, drawn as the screen has it.
    void handBoldInAPdf()
    {
        if (handBoldFontsInstalled()) QSKIP("these fonts are installed here");
        using namespace jp;
        const TextOutput pic = TextOutput::Picture, pdf = TextOutput::Pdf;
        const QString black = QStringLiteral("Arial Black"), rock = QStringLiteral("Rockwell Extra Bold"), demi = QStringLiteral("Franklin Gothic Demi");
        for (const QString &fam : {black, rock})
            for (double size : {48.0, 96.0}) {
                const double screen = handBoldStem(fam, false, size, pic);
                for (bool bold : {false, true}) {
                    const double grown = handBoldStem(fam, bold, size, pdf) - screen;
                    QVERIFY2(std::abs(grown - size / 35) < 0.1 + 0.002 * size, qPrintable(QStringLiteral("%1 %2 pt%3: %4 wider").arg(fam).arg(size).arg(bold ? QStringLiteral(" bold") : QString()).arg(grown)));
                }
            }
        const double plain = handBoldStem(demi, false, 48, pic);
        QVERIFY(std::abs(handBoldStem(demi, false, 48, pdf) - plain) < 0.1);
        QVERIFY2(std::abs(handBoldStem(demi, true, 48, pdf) - plain - 48.0 / 35) < 0.12, "Franklin Gothic Demi bold");
        // Outlines: as the screen draws them.
        QVERIFY(std::abs(handBoldStem(black, false, 230, pdf) - handBoldStem(black, false, 230, pic)) < 0.2);
        QVERIFY(std::abs(handBoldStem(demi, true, 230, pdf) - handBoldStem(demi, true, 230, pic)) < 0.2);
        // Just under the size they are set as outlines at, they are stroked still.
        QVERIFY(handBoldStem(black, false, 214, pdf) - handBoldStem(black, false, 214, pic) > 5);
    }

    // And on paper: Publisher sends the same letters to the printer as bold
    // by simulation, and Windows' PDF printer strokes them 0.01 em (stems
    // 0.48 pt wider at 48 pt, 0.96 at 96): Arial Black and Rockwell Extra
    // Bold plain or bold, Franklin Gothic Demi only when bold; from 216 pt
    // they are outlines, as the screen has them.
    void handBoldOnPaper()
    {
        if (handBoldFontsInstalled()) QSKIP("these fonts are installed here");
        using namespace jp;
        const TextOutput pic = TextOutput::Picture, paper = TextOutput::Print;
        const QString black = QStringLiteral("Arial Black"), rock = QStringLiteral("Rockwell Extra Bold"), demi = QStringLiteral("Franklin Gothic Demi");
        for (const QString &fam : {black, rock})
            for (double size : {48.0, 96.0}) {
                const double screen = handBoldStem(fam, false, size, pic);
                for (bool bold : {false, true}) {
                    const double grown = handBoldStem(fam, bold, size, paper) - screen;
                    QVERIFY2(std::abs(grown - size * 0.01) < 0.1 + 0.001 * size, qPrintable(QStringLiteral("%1 %2 pt%3: %4 wider").arg(fam).arg(size).arg(bold ? QStringLiteral(" bold") : QString()).arg(grown)));
                }
            }
        const double plain = handBoldStem(demi, false, 48, pic);
        QVERIFY(std::abs(handBoldStem(demi, false, 48, paper) - plain) < 0.1);
        QVERIFY2(std::abs(handBoldStem(demi, true, 48, paper) - plain - 0.48) < 0.12, "Franklin Gothic Demi bold");
        QVERIFY(std::abs(handBoldStem(black, false, 230, paper) - handBoldStem(black, false, 230, pic)) < 0.2);
        QVERIFY(std::abs(handBoldStem(demi, true, 230, paper) - handBoldStem(demi, true, 230, pic)) < 0.2);
    }

    // True small capitals are an OpenType feature, which Qt takes from the
    // paragraph's layout font for every run: with the first run flagged the
    // whole paragraph came out in them, and with only a later run flagged,
    // none of it. A paragraph has them when all its letters are in flagged
    // runs; in a mixed one the flagged runs are small capitals made of
    // capitals, the unflagged runs as they are.
    void trueSmallCapsStayInTheirRun()
    {
        const QString family = QStringLiteral("EB Garamond");
        if (!QFontDatabase::hasFamily(family)) QSKIP("EB Garamond is not here");
        // Where the second run starts and ends, by which runs are flagged.
        auto widths = [&](bool firstSmall, bool secondSmall) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat a;
            a.setFontFamilies(QStringList{family});
            a.setFontPointSize(20);
            QTextCharFormat b = a;
            if (firstSmall) a.setProperty(jp::tp::TrueSmallCaps, true);
            if (secondSmall) b.setProperty(jp::tp::TrueSmallCaps, true);
            c.insertText(QStringLiteral("First run "), a);
            c.insertText(QStringLiteral("second run text"), b);
            jp::FrameSpec fs;
            fs.size = QSizeF(500, 40);
            fs.insets = QMarginsF(0, 0, 0, 0);
            jp::StoryLayout l;
            l.build(&doc, {fs}, jp::LayoutEnv());
            int frame = -1;
            QRectF r0, r1, r2;
            l.caretRect(0, &frame, &r0);
            l.caretRect(10, &frame, &r1);
            l.caretRect(int(doc.characterCount()) - 1, &frame, &r2);
            return QPointF(r1.x() - r0.x(), r2.x() - r1.x());
        };
        const QPointF none = widths(false, false), both = widths(true, true);
        QVERIFY2(both.x() > none.x() + 5 && both.y() > none.y() + 5, qPrintable(QStringLiteral("%1,%2 against %3,%4").arg(both.x()).arg(both.y()).arg(none.x()).arg(none.y())));
        // The first run flagged: the second is as it is.
        const QPointF first = widths(true, false);
        QVERIFY2(std::abs(first.y() - none.y()) < 0.01 && std::abs(first.x() - none.x()) > 1, qPrintable(QStringLiteral("%1,%2 against %3,%4").arg(first.x()).arg(first.y()).arg(none.x()).arg(none.y())));
        // The second run flagged: the first is as it is, the second in small capitals.
        const QPointF second = widths(false, true);
        QVERIFY2(std::abs(second.x() - none.x()) < 0.01 && std::abs(second.y() - none.y()) > 1, qPrintable(QStringLiteral("%1,%2 against %3,%4").arg(second.x()).arg(second.y()).arg(none.x()).arg(none.y())));
    }

    // A bundled font whose table directory claims a table that runs past the
    // end of the file is corrupt, and is left alone: the checksums were summed
    // over the length it claimed, beyond the font's bytes.
    void italicFlagsLeaveCorruptFontAlone()
    {
        QFile f(QStringLiteral(JP_TEST_DATA "/../../resources/fonts/LibreFranklin-ExtraBoldItalic.ttf"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray font = f.readAll();
        QVERIFY(!jp::fontWithItalicFlags(font).isEmpty());   // an intact one is fixed
        const auto u16 = [&](qsizetype at) { return quint16(uchar(font[at]) << 8 | uchar(font[at + 1])); };
        for (const char *tag : {"OS/2", "head", "post"}) {
            qsizetype record = -1;
            for (int i = 0, n = u16(4); i < n; ++i)
                if (font.mid(12 + 16 * i, 4) == tag) record = 12 + 16 * i;
            QVERIFY(record > 0);
            // The table's length past the end of the file, by a few bytes.
            const quint32 offset = quint32(u16(record + 8)) << 16 | u16(record + 10);
            const quint32 length = quint32(font.size() - offset + 8);
            QByteArray corrupt = font;
            for (int k = 0; k < 4; ++k) corrupt[record + 12 + k] = char(length >> (24 - 8 * k));
            QVERIFY2(jp::fontWithItalicFlags(corrupt).isEmpty(), tag);
        }
    }

    // Extra line spacing goes below each line, as .pub layouts place it: the
    // first line sits one ascent below the top, a paragraph after a more
    // widely spaced one starts lower by that paragraph's extra, and a box
    // holds as many lines as before.
    // A line of small capitals only (lowercase letters drawn at 0.8 x) is
    // spaced from the next as a line at the full size, as in Publisher's PDF
    // of the Age of Reason cover (100 pt small caps at 0.75 lines: baselines
    // 71.6 and 86.7 pt apart); a soft hyphen in it doesn't make it deeper.
    void smallCapsLineSpacing()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 400, 600);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->storyId = doc->createStory(QString());
        doc->pages[0]->items.push_back(t);
        QTextCursor c(doc->storyDoc(t->storyId));
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{QStringLiteral("Times New Roman")});
        cf.setFontPointSize(100);
        cf.setFontCapitalization(QFont::SmallCaps);
        QTextBlockFormat bf;
        bf.setLineHeight(75, QTextBlockFormat::ProportionalHeight);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("age"), cf);
        c.insertBlock(bf, cf);
        c.insertText(QString::fromUtf8("rea\u00ADson"), cf);
        c.insertBlock(bf, cf);
        c.insertText(QStringLiteral("AGE"), cf);
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        const auto lines = cache.textFrame(*doc, *t, 1, opt).layout->lineInfo(0);
        QCOMPARE(lines.size(), 3);
        const double gap1 = lines[1].baseline - lines[0].baseline, gap2 = lines[2].baseline - lines[1].baseline;
        const double line = 0.75 * 100 * (1420.0 + 442 + 307) / 2048;   // a full-size line (Times New Roman)
        QVERIFY2(std::abs(gap1 - line) < 0.5, qPrintable(QString::number(gap1)));
        // The full-size capitals' line sits the same way: set closer than
        // single, a line's descent counts at the full size too (Age of Reason
        // and North Carolina covers: baselines within 0.1 pt of Publisher's).
        QVERIFY2(std::abs(gap2 - line) < 0.5, qPrintable(QString::number(gap2)));
        // The first baseline: 0.75 x (single - descent) below the top.
        const double single = 100 * (1420.0 + 442 + 307) / 2048, descent = 100 * 442.0 / 2048;
        QVERIFY2(std::abs(lines[0].baseline - 0.75 * (single - descent)) < 0.5, qPrintable(QString::number(lines[0].baseline)));
    }

    // Text at a fractional size lays out at that size: Qt sizes fonts in
    // whole pixels (13.25 pt measured as 13, 13.5 as 14), which pushed a
    // flyer's "Civilian homeland security" onto two lines.
    void fractionalFontSizes()
    {
        auto endX = [](double size) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{QStringLiteral("Times New Roman")});
            cf.setFontPointSize(size);
            c.insertText(QStringLiteral("Civilian homeland security"), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(1000, 200);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            int frame = -1;
            QRectF r;
            lay.caretRect(doc.characterCount() - 1, &frame, &r);
            return r.x();
        };
        const double w13 = endX(13), w1325 = endX(13.25), w135 = endX(13.5);
        QVERIFY2(std::abs(w1325 / w13 - 13.25 / 13) < 0.003, qPrintable(QStringLiteral("%1 %2").arg(w13).arg(w1325)));
        QVERIFY2(std::abs(w135 / w13 - 13.5 / 13) < 0.003, qPrintable(QStringLiteral("%1 %2").arg(w13).arg(w135)));
    }

    // Bullets in symbol fonts: a .pub file keeps the bullet's font with its
    // character, and a missing symbol font's bullet shows as Unicode.
    void symbolFontBullets()
    {
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Wingdings"), 0xF0E8), QStringLiteral("\u2794"));   // the arrow
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Symbol"), 0xB7), QStringLiteral("\u2022"));
        QVERIFY(jp::symbolToUnicode(QStringLiteral("Arial"), 0x41).isEmpty());
        // The second and third sets, whose bullets presentation themes use.
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Wingdings 2"), 0xF097), QStringLiteral("\u25CF"));   // a round bullet
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Wingdings 2"), 0xF0A8), QStringLiteral("\u25A3"));   // a square in a square
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Wingdings 3"), 0xF075), QStringLiteral("\u25B6"));   // a triangle
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Wingdings 3"), 0x7D), QStringLiteral("\u25BA"));     // a narrow one
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("wingdings 3"), 0xF0B4), QStringLiteral("\u2B53"));   // a tag
        QString font;
        QCOMPARE(jp::unicodeToSymbol(QChar(0x2714), &font), 0xF0FCu);
        QCOMPARE(font, QStringLiteral("Wingdings"));

        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 200);
        t->storyId = doc->createStory(QString());
        doc->pages[0]->items.push_back(t);
        QTextCursor c(doc->storyDoc(t->storyId));
        auto addList = [&](const QString &bullet, const QString &bulletFont, const QString &text, bool first) {
            if (!first) c.insertBlock();
            c.insertText(text);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setProperty(jp::tp::BulletChar, bullet);
            if (!bulletFont.isEmpty()) lf.setProperty(jp::tp::BulletFont, bulletFont);
            c.createList(lf);
        };
        addList(QString(QChar(0xF0E8)), QStringLiteral("Wingdings"), QStringLiteral("An arrow"), true);
        addList(QString(QChar(0x2714)), QString(), QStringLiteral("A check mark"), false);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("bullets.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TextItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
        });
        QVERIFY(bt);
        QTextBlock b = back->storyDoc(bt->storyId)->begin();
        QVERIFY(b.textList() && b.next().textList());
        QCOMPARE(b.textList()->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Wingdings"));
        QCOMPARE(b.textList()->format().stringProperty(jp::tp::BulletChar), QString(QChar(0xF0E8)));
        // A Unicode check mark is saved as Wingdings' own.
        QCOMPARE(b.next().textList()->format().stringProperty(jp::tp::BulletFont), QStringLiteral("Wingdings"));
        QCOMPARE(b.next().textList()->format().stringProperty(jp::tp::BulletChar), QString(QChar(0xF0FC)));
    }

    // Webdings' triangles point in four directions, and a code the table doesn't name (0x41) is left as it is. A triangle has no emoji
    // picture, so it carries no U+FE0E in running text.
    void webdingsTriangles()
    {
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Webdings"), 0xF034), QStringLiteral("▸"));   // a right-pointing triangle
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("webdings"), 0x33), QStringLiteral("◂"));     // a left-pointing one
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Webdings"), 0x35), QStringLiteral("▴"));     // pointing up
        QCOMPARE(jp::symbolToUnicode(QStringLiteral("Webdings"), 0x36), QStringLiteral("▾"));     // pointing down
        QVERIFY(jp::symbolToUnicode(QStringLiteral("Webdings"), 0x41).isEmpty());
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Webdings"), 0xF034), QStringLiteral("▸"));
        QVERIFY(jp::symbolTextToUnicode(QStringLiteral("Webdings"), 0x41).isEmpty());
    }

    // The pictures that only the word processor knows (Wingdings E7, E9, and EA, the arrows left, up, and down; Webdings 67, the solid block of a banner) are not in the
    // shared mapping, which a publication's missing-font bullet, a presentation's text, and a metafile's drawing read: it is as it was, and the word processor's own
    // table (symbolPictureOf) has them, the shared ones too.
    void symbolPicturesOfTheWordProcessorAreNotTheSharedOnes()
    {
        for (const uint code : {0xE7u, 0xE9u, 0xEAu}) {
            QVERIFY2(jp::symbolToUnicode(QStringLiteral("Wingdings"), code).isEmpty(), qPrintable(QString::number(code, 16)));
            QVERIFY(jp::symbolToUnicode(QStringLiteral("Wingdings"), 0xF000 + code).isEmpty());
            QVERIFY(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), code).isEmpty());
        }
        QVERIFY(jp::symbolToUnicode(QStringLiteral("Webdings"), 0x67).isEmpty());
        QVERIFY(jp::symbolTextToUnicode(QStringLiteral("Webdings"), 0xF067).isEmpty());
        QCOMPARE(jp::symbolPictureOf(QStringLiteral("Wingdings"), 0xE7), QChar(0x2B05));
        QCOMPARE(jp::symbolPictureOf(QStringLiteral("Wingdings"), 0xF0E9), QChar(0x2B06));
        QCOMPARE(jp::symbolPictureOf(QStringLiteral("wingdings"), 0xEA), QChar(0x2B07));
        QCOMPARE(jp::symbolPictureOf(QStringLiteral("Webdings"), 0xF067), QChar(0x2588));
        QCOMPARE(jp::symbolPictureOf(QStringLiteral("Wingdings"), 0xE8), QChar(0x2794));   // (one the tables share)
        QCOMPARE(jp::symbolPictureOf(QStringLiteral("Symbol"), 0xB0), QChar(0x00B0));
        QVERIFY(jp::symbolPictureOf(QStringLiteral("Arial"), 0xEA).isNull());
    }

    // A symbol font's pictures are one table of 224 characters (codes 0x20 to 0xFF, or the same in the range 0xF020 to 0xF0FF) made once for the family, so a run
    // of text is turned into its pictures by looking up each character: the table says what the one-character function says, and there is none for a family
    // that is no symbol font.
    void aSymbolFontsPicturesAreOneTableOfCodes()
    {
        const QStringList families{QStringLiteral("Symbol"), QStringLiteral("Wingdings"), QStringLiteral("Wingdings 2"), QStringLiteral("Wingdings 3"), QStringLiteral("webdings")};
        for (const QString &family : families) {
            const jp::SymbolPictures *table = jp::symbolPicturesOf(family);
            QVERIFY2(table, qPrintable(family));
            QCOMPARE(jp::symbolPicturesOf(family.toUpper()), table);   // (the same table, whatever the case)
            for (uint code = 0x20; code <= 0xFF; ++code) {
                QCOMPARE(table->of(code), jp::symbolPictureOf(family, code));
                QCOMPARE(table->of(0xF000 + code), table->of(code));
            }
            QVERIFY(table->of(0x1F).isNull() && table->of(0x100).isNull() && table->of(0xF01F).isNull() && table->of(0xF100).isNull());
        }
        QCOMPARE(jp::symbolPicturesOf(QStringLiteral("Wingdings"))->of(0xF0EA), QChar(0x2B07));
        QVERIFY(!jp::symbolPicturesOf(QStringLiteral("Arial")));
        QVERIFY(!jp::symbolPicturesOf(QString()));
    }

    // The letters and signs of a symbol font in running text come to the Unicode character that looks like them; the ones that have a picture
    // of their own as emoji (the hands, the card suits) carry U+FE0E after them, which asks for the plain glyph, so they aren't drawn in
    // color; the others carry nothing. A circled number set in Wingdings, from the code itself (0x8C), is one in a dark circle.
    void symbolTextKeepsEmojiPlain()
    {
        const QChar plain(0xFE0E);
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Symbol"), 0xF061), QString(QChar(0x3B1)));                       // alpha
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Symbol"), 0xAE), QString(QChar(0x2192)));                        // an arrow
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Symbol"), 0xAA), QString(QChar(0x2660)) + plain);                // a spade
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Symbol"), 0xAB), QString(QChar(0x2194)) + plain);                // a double arrow
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x43), QString::fromUcs4(U"\U0001F44D") + plain);     // a thumb up
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x44), QString::fromUcs4(U"\U0001F44E") + plain);     // a thumb down
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x42), QString::fromUcs4(U"\U0001F44C") + plain);     // a hand
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x41), QString(QChar(0x270C)) + plain);             // a victory sign
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x47), QString(QChar(0x261D)) + plain);             // a hand pointing up
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x46), QString(QChar(0x261E)));                     // a hand pointing right: no emoji
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0xFC), QString(QChar(0x2714)) + plain);             // a check mark
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x8C), QString(QChar(0x2776)));                     // 1 in a dark circle
        QCOMPARE(jp::symbolTextToUnicode(QStringLiteral("Wingdings"), 0x9F), QString(QChar(0x2022)));                     // a bullet
        QVERIFY(jp::symbolTextToUnicode(QStringLiteral("Arial"), 0x41).isEmpty());
    }

    // All capitals show as capitals (Qt's layout ignores the setting), and
    // the text itself keeps its case.
    void allCapsShown()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 400, 100);
        t->storyId = doc->createStory(QStringLiteral("Report on the CIA"));
        doc->pages[0]->items.push_back(t);
        QTextDocument *d = doc->storyDoc(t->storyId);
        QTextCursor c(d);
        c.select(QTextCursor::Document);
        QTextCharFormat caps;
        caps.setFontCapitalization(QFont::AllUppercase);
        c.mergeCharFormat(caps);
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        const auto fl = cache.textFrame(*doc, *t, 1, opt);
        QVERIFY(fl.layout);
        QCOMPARE(fl.layout->lineInfo(0).value(0).text.remove(QChar(0x00AD)).trimmed(), QStringLiteral("REPORT ON THE CIA"));
        QCOMPARE(d->toPlainText(), QStringLiteral("Report on the CIA"));
    }

    // Tracking adds the font's average character width times the percentage
    // beyond 100 after each letter (as .pub layouts do); kerning adds points.
    void trackingAndKerning()
    {
        jp::LayoutEnv env;
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("Arial")});
        f.setFontPointSize(10);
        auto spacing = [&](const QTextCharFormat &g) {
            const QTextCharFormat r = jp::resolveCharFormat(g, env);
            return r.fontLetterSpacingType() == QFont::AbsoluteSpacing ? r.fontLetterSpacing() : 0.0;
        };
        QVERIFY(std::abs(spacing(f)) < 0.001);
        QTextCharFormat loose = f;
        loose.setProperty(jp::tp::Tracking, 125);
        QVERIFY2(std::abs(spacing(loose) - 0.25 * 904 / 2048 * 10) < 0.02, qPrintable(QString::number(spacing(loose))));
        QTextCharFormat both = f;
        both.setProperty(jp::tp::Tracking, 87.5);
        both.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        both.setFontLetterSpacing(1);
        QVERIFY(std::abs(spacing(both) - (1 - 0.125 * 904 / 2048 * 10)) < 0.02);
        // Files saved before tracking had its own property kept it as Qt
        // percentage spacing; it is read as tracking, not Qt's own stretching.
        QTextCharFormat old = f;
        old.setFontLetterSpacingType(QFont::PercentageSpacing);
        old.setFontLetterSpacing(125);
        QVERIFY(std::abs(spacing(old) - spacing(loose)) < 0.001);
    }

    // Comic Sans MS, Segoe UI and Arial Narrow, when missing, are drawn by
    // fonts made to their character widths (Comic Relief, Selawik, Liberation
    // Sans Narrow): a phrase is as wide as in the originals (their widths read
    // from the fonts on Windows). Comic Neue had run 8.6% narrower, Open Sans
    // 5.2% wider.
    void exactWidthStandIns()
    {
        const QString text = QStringLiteral("Defense Force volunteers serve their state");
        struct Case { const char *family; bool bold; double ems; };
        for (const Case &c : {Case{"Comic Sans MS", false, 20.312}, Case{"Comic Sans MS", true, 21.142}, Case{"Segoe UI", false, 18.393},
                              Case{"Arial Narrow", false, 15.454}, Case{"Arial Narrow", true, 16.544}}) {
            if (QFontDatabase::hasFamily(QString::fromLatin1(c.family))) continue;   // the original itself
            jp::LayoutEnv env;
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QString::fromLatin1(c.family)});
            // At 100 pt, as text is laid out (at 8 times its size), so Qt's
            // rounding of each advance to the pixel doesn't add up.
            f.setFontPointSize(100);
            if (c.bold) f.setFontWeight(QFont::Bold);
            const QTextCharFormat r = jp::resolveCharFormat(f, env);
            QTextLayout tl(text, r.font());
            tl.beginLayout();
            QTextLine line = tl.createLine();
            line.setLineWidth(100000);
            tl.endLayout();
            const double width = line.naturalTextWidth();
            QVERIFY2(std::abs(width - c.ems * 100) < c.ems * 100 * 0.002,
                     qPrintable(QStringLiteral("%1%2: %3 pt, not %4").arg(QLatin1String(c.family), c.bold ? QStringLiteral(" bold") : QString()).arg(width).arg(c.ems * 100)));
        }
    }

    // A missing Gill Sans MT is drawn by Cabin to Gill Sans MT's widths, its
    // bold too: Cabin's letters run 5% wider than the regular's and 10%
    // narrower than the bold's, and its spaces 24% narrower (widths of the
    // fonts in Publisher's PDFs). Lines broke at other words than Publisher's.
    void standInWidthsMatchOriginals()
    {
        if (QFontDatabase::hasFamily(QStringLiteral("Gill Sans MT"))) QSKIP("Gill Sans MT is installed: its own widths are used");
        const QString text = QStringLiteral("Defense Force volunteers serve their state");
        // The phrase's width in Gill Sans MT (ems), regular and bold.
        for (const auto &[bold, ems] : {std::pair{false, 17.388}, std::pair{true, 19.855}}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QStringLiteral("Gill Sans MT")});
            f.setFontPointSize(10);
            if (bold) f.setFontWeight(QFont::Bold);
            c.insertText(text, f);
            jp::FrameSpec fs;
            fs.size = QSizeF(10000, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            auto x = [&](int pos) {
                int frame = -1;
                QRectF r;
                lay.caretRect(pos, &frame, &r);
                return r.x();
            };
            const double width = x(text.size()) - x(0);
            QVERIFY2(std::abs(width - ems * 10) < ems * 10 * 0.002, qPrintable(QStringLiteral("%1: %2 pt, not %3").arg(bold ? "bold" : "regular").arg(width).arg(ems * 10)));
        }
    }

    // A font's stand-in is looked up once, not on every layout of every run
    // (asking the font database costs 0.2 ms here), and is looked up again
    // when fonts are added or removed.
    void substituteForIsRemembered()
    {
        const QString fam = QStringLiteral("Gill Sans MT");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Gill Sans MT is installed here");
        QVERIFY(!jp::substituteFor(fam).isEmpty());
        int found = 0;
        const qint64 ms = bestNanoseconds([&] {
            found = 0;
            for (int i = 0; i < 1000; ++i) found += !jp::substituteFor(fam).isEmpty();
        }) / 1000000;
        QCOMPARE(found, 1000);
        QVERIFY2(ms < 50, qPrintable(QStringLiteral("1,000 lookups took %1 ms").arg(ms)));
        // Fonts coming and going (a bundled font renamed "Arial" stands in for an installed Arial).
        if (QFontDatabase::hasFamily(QStringLiteral("Arial"))) QSKIP("Arial is installed here");
        QVERIFY(!jp::substituteFor(QStringLiteral("Arial")).isEmpty());
        QFile f(QStringLiteral(JP_TEST_DATA "/../../resources/fonts/Arimo-Regular.ttf"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray font = f.readAll();
        font.replace(QByteArray("\x00" "A\x00r\x00i\x00m\x00o", 10), QByteArray("\x00" "A\x00r\x00i\x00" "a\x00l", 10));
        font.replace(QByteArray("Arimo"), QByteArray("Arial"));
        const int id = QFontDatabase::addApplicationFontFromData(font);
        jp::fontsChanged();
        QVERIFY(id >= 0);
        const bool installed = jp::substituteFor(QStringLiteral("Arial")).isEmpty();
        QFontDatabase::removeApplicationFont(id);
        jp::fontsChanged();
        QVERIFY(installed);
        QVERIFY(!jp::substituteFor(QStringLiteral("Arial")).isEmpty());
    }

    // Whether a family is here is remembered for the text engine (it is asked for every run of a word processor's text, and a font match costs), and forgotten when fonts
    // are added or taken away: a family that was missing is used once its font is added, and not after it is taken away; the usual way of adding fonts says so.
    void aFontAddedOrRemovedChangesWhichFamiliesTheTextEngineKnows()
    {
        // The program's own fonts, added the usual way, are a change.
        const int generation = jp::fontGeneration();
        jp::loadBundledFonts();
        QVERIFY(jp::fontGeneration() > generation);
        const QString fam = QStringLiteral("Zrimo");
        QVERIFY(!QFontDatabase::hasFamily(fam));
        jp::LayoutEnv env;
        env.wordLines = true;
        auto familyOf = [&]() {
            QTextCharFormat cf;
            cf.setFontFamilies({fam});
            return jp::resolveCharFormat(cf, env).fontFamilies().toStringList().value(0);
        };
        // (Not here: set in the word processor's stand-in, and asked about twice, which the second time is remembered.)
        QCOMPARE(familyOf(), QStringLiteral("Calibri"));
        QCOMPARE(familyOf(), QStringLiteral("Calibri"));
        QFile f(QStringLiteral(JP_TEST_DATA "/../../resources/fonts/Arimo-Regular.ttf"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray font = f.readAll();
        font.replace(QByteArray("\x00" "A\x00r\x00i\x00m\x00o", 10), QByteArray("\x00" "Z\x00r\x00i\x00m\x00o", 10));
        font.replace(QByteArray("Arimo"), QByteArray("Zrimo"));
        const int id = QFontDatabase::addApplicationFontFromData(font);
        QVERIFY(id >= 0);
        QVERIFY(QFontDatabase::applicationFontFamilies(id).contains(fam));
        jp::fontsChanged();
        const QString added = familyOf();
        QFontDatabase::removeApplicationFont(id);
        jp::fontsChanged();
        const QString removed = familyOf();
        QCOMPARE(added, fam);
        QCOMPARE(removed, QStringLiteral("Calibri"));
    }

    // Franklin Gothic Heavy, the missing font Publisher's own designs use
    // most, is drawn in Libre Franklin ExtraBold, narrowed to its width
    // (98.30 ems for this sample in the Windows font).
    void franklinGothicHeavyStandIn()
    {
        if (QFontDatabase::hasFamily(QStringLiteral("Franklin Gothic Heavy"))) QSKIP("Franklin Gothic Heavy is installed");
        const QString sample = QStringLiteral("The quick brown fox jumps over the lazy dog. Pack my box with five dozen liquor jugs! Sphinx of black quartz, "
                                              "judge my vow. How vexingly quick daft zebras jump; 2026 Annual Report: Newsletter, Spring Edition.");
        jp::LayoutEnv env;
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("Franklin Gothic Heavy")});
        f.setFontPointSize(10);
        const QTextCharFormat r = jp::resolveCharFormat(f, env);
        const QFontInfo info(r.font());
        QCOMPARE(info.family(), QStringLiteral("Libre Franklin"));
        QVERIFY2(!info.styleName().contains(QLatin1String("Italic")), qPrintable(info.styleName()));
        const auto widthOf = [&](const QFont &font) {
            QTextLayout tl(sample, font);
            tl.beginLayout();
            QTextLine line = tl.createLine();
            line.setLineWidth(100000);
            tl.endLayout();
            return line.naturalTextWidth();
        };
        // The narrowing: 97% of the face's own width, on the Mac too.
        QTextCharFormat plain = r;
        plain.clearProperty(QTextFormat::FontStretch);
        const double narrowed = widthOf(r.font()) / widthOf(plain.font());
        QVERIFY2(std::abs(narrowed - 0.97) < 0.97 * 0.02, qPrintable(QString::number(narrowed)));
#ifndef Q_OS_MACOS
        // The face drawn (the Mac's font matching gives Heavy Libre Franklin Bold, even with
        // no stretch, so its weight and width differ from the Windows ExtraBold's).
        QVERIFY2(info.weight() >= 750, qPrintable(info.styleName()));
        const double width = widthOf(r.font()), want = 98.3008 * 10;
        QVERIFY2(std::abs(width - want) < want * 0.01, qPrintable(QStringLiteral("%1 pt, not %2").arg(width).arg(want)));
#endif
    }

    // How the platform's text engine draws the stretched stand-ins (each
    // engine stretches a font its own way: the Mac applied a QFont's stretch
    // twice). For each family and style the table stretches, one STRETCHPROBE
    // line gives the face drawn without and with the stretch (family, style,
    // weight, slant), the width a sample sentence takes in each, and how wide
    // its letters are drawn; stand-ins the platform can't find are listed too.
    // Where the face drawn is the stand-in, the stretch must make its text as
    // wide as asked (within 1.5%, and the Mac's whole-number rounding), in the
    // slant asked and the style named; every problem is listed at the end.
    void standInStretchOnThisPlatform()
    {
        const QString sample = QStringLiteral("The quick brown fox jumps over the lazy dog. Pack my box with five dozen liquor jugs! Sphinx of black quartz, "
                                              "judge my vow. How vexingly quick daft zebras jump; 2026 Annual Report: Newsletter, Spring Edition.");
        static const char *const styleNames[4] = {"regular", "bold", "italic", "bolditalic"};
        jp::LayoutEnv env;
        QStringList problems;
        qInfo("STRETCHPROBE platform=\"%s\" qt=%s", qPrintable(QGuiApplication::platformName()), qVersion());
        // Stand-ins this platform's font list can't find, by the table's name or as a style of a family.
        for (const QString &fam : jp::standInFamiliesForTest()) {
            const QString here = jp::standInOnThisPlatform(fam);
            if (here != fam) qInfo("STRETCHPROBE standIn=\"%s\" is a style of \"%s\", which lists: %s", qPrintable(fam), qPrintable(here), qPrintable(QFontDatabase::styles(here).join(QLatin1Char(','))));
            if (!QFontDatabase::hasFamily(here)) {
                qInfo("STRETCHPROBE missing=\"%s\"", qPrintable(fam));
                problems << QStringLiteral("stand-in %1 is missing").arg(fam);
            }
        }
        // How wide the sample's letters are drawn, ink edge to ink edge.
        const auto inkWidth = [&](const QFont &font, double advance) {
            QImage img(int(advance * 1.5) + 40, int(QFontMetricsF(font).height() * 2) + 20, QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            {
                QPainter p(&img);
                p.setFont(font);
                p.setPen(Qt::black);
                p.drawText(QPointF(20, QFontMetricsF(font).ascent() + 10), sample);
            }
            int lo = img.width(), hi = -1;
            for (int y = 0; y < img.height(); ++y) {
                const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
                for (int x = 0; x < img.width(); ++x)
                    if (qAlpha(row[x]) > 64) { lo = std::min(lo, x); hi = std::max(hi, x); }
            }
            return hi >= lo ? double(hi - lo + 1) : 0.0;
        };
        const auto face = [](const QFont &font) {
            const QFontInfo info(font);
            return QStringLiteral("%1|%2|%3").arg(info.family(), info.styleName()).arg(info.weight());
        };
        int cases = 0, checked = 0;
        for (const jp::StandInStretch &s : jp::standInStretchesForTest()) {
            for (int i = 0; i < 4; ++i) {
                if (!s.stretch[i]) continue;
                QTextCharFormat f;
                f.setFontFamilies(QStringList{s.family});
                f.setFontPointSize(20);
                f.setFontWeight(i & 1 ? QFont::Bold : QFont::Normal);
                f.setFontItalic(i & 2);
                // As the layout builds the font (the table's stretch in it where the
                // stand-in is what is drawn), then with the stretch left out.
                const QTextCharFormat with = jp::resolveCharFormat(f, env);
                QTextCharFormat without = with;
                without.clearProperty(QTextFormat::FontStretch);
                const double w0 = QFontMetricsF(without.font()).horizontalAdvance(sample);
                const double w1 = QFontMetricsF(with.font()).horizontalAdvance(sample);
                QVERIFY(w0 > 0);
                const double i0 = inkWidth(without.font(), w0), i1 = inkWidth(with.font(), w1);
                const QFontInfo info(with.font());
                qInfo("STRETCHPROBE family=\"%s\" style=%s standIn=\"%s\" asked=%d without=\"%s\" with=\"%s\" italic=%d widthWithout=%.2f widthWith=%.2f ratio=%.4f inkRatio=%.4f",
                      qPrintable(s.family), styleNames[i], qPrintable(s.standIn), s.stretch[i],
                      qPrintable(face(without.font())), qPrintable(face(with.font())), info.italic(), w0, w1, w1 / w0, i0 > 0 ? i1 / i0 : 0.0);
                ++cases;
                // Only the stand-in itself is checked (the Mac draws Rockwell and Tahoma in its own fonts).
                const QString drawn = info.family();
                if (drawn.compare(s.standIn, Qt::CaseInsensitive) != 0 && !s.standIn.startsWith(drawn + QLatin1Char(' '), Qt::CaseInsensitive)) continue;
                ++checked;
                const QString what = QStringLiteral("%1 %2 drawn as %3").arg(s.family, styleNames[i], face(with.font()));
                // A stretch can make Qt pick another face of the family (DejaVu Sans' Condensed),
                // which is as wide as it is; the whole numbers the Mac is asked for can't all hit the width.
                if (face(without.font()) == face(with.font())) {
                    const int asked = s.stretch[i], sent = jp::stretchToAsk(asked);
                    const double tolerance = 0.015 + (sent != asked ? 1.0 / sent : 0.0);
                    if (std::abs(w1 / w0 / (asked / 100.0) - 1) > tolerance)
                        problems << QStringLiteral("%1: %2 times as wide, not %3").arg(what).arg(w1 / w0, 0, 'f', 4).arg(asked / 100.0);
                }
                if (info.italic() != bool(i & 2))
                    problems << QStringLiteral("%1: italic is %2").arg(what).arg(info.italic());
                if (!(i & 2) && (info.styleName().contains(QLatin1String("Italic"), Qt::CaseInsensitive) || info.styleName().contains(QLatin1String("Oblique"), Qt::CaseInsensitive)))
                    problems << QStringLiteral("%1: an italic face, none asked").arg(what);
                // A stand-in that is a style of a family (Selawik Light) is drawn in that style, where the family lists it.
                const QString style = s.standIn.mid(drawn.size() + 1);
                if (!(i & 1) && drawn.compare(s.standIn, Qt::CaseInsensitive) != 0 && QFontDatabase::styles(drawn).contains(style, Qt::CaseInsensitive)
                    && info.styleName().compare(style, Qt::CaseInsensitive) != 0)
                    problems << QStringLiteral("%1: not the style %2").arg(what, style);
            }
        }
        for (const QString &p : problems) qInfo("STRETCHPROBE problem=\"%s\"", qPrintable(p));
        QVERIFY(cases > 0);
        QVERIFY(checked > 0);
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QLatin1Char('\n'))));
    }

    // The stretch handed to Qt: as asked, but on the Mac, which applies it
    // twice (a run at 80% drew at 64%), the whole number whose square is
    // nearest. A run's own scaling draws that wide on every platform.
    void stretchToAskDrawsTheWidthWanted()
    {
        QCOMPARE(jp::stretchToAsk(95, true), 97);
        QCOMPARE(jp::stretchToAsk(85, true), 92);
        QCOMPARE(jp::stretchToAsk(64, true), 80);
        QCOMPARE(jp::stretchToAsk(119, true), 109);
        QCOMPARE(jp::stretchToAsk(100, true), 100);
        QCOMPARE(jp::stretchToAsk(1, true), 10);
        QCOMPARE(jp::stretchToAsk(4000, true), 632);
        for (int p = 1; p <= 4000; ++p) {
            QCOMPARE(jp::stretchToAsk(p, false), p);
            const int s = jp::stretchToAsk(p, true);
            QVERIFY2(std::abs(s * s - 100 * p) <= s, qPrintable(QStringLiteral("%1 -> %2").arg(p).arg(s)));
        }
        const QString sample = QStringLiteral("The quick brown fox jumps over the lazy dog. Pack my box with five dozen liquor jugs!");
        jp::LayoutEnv env;
        const auto width = [&](int scale) {
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QStringLiteral("Arimo")});
            f.setFontPointSize(20);
            if (scale != 100) f.setFontStretch(scale);
            return QFontMetricsF(jp::resolveCharFormat(f, env).font()).horizontalAdvance(sample);
        };
        for (int scale : {50, 80, 95, 119, 150}) {
            const int sent = jp::stretchToAsk(scale);
            const double ratio = width(scale) / width(100);
            QVERIFY2(std::abs(ratio / (scale / 100.0) - 1) < 0.015 + (sent != scale ? 1.0 / sent : 0.0),
                     qPrintable(QStringLiteral("%1%% is drawn %2").arg(scale).arg(ratio)));
        }
        // A run in the theme's font names no family: its own scaling is asked the same way.
        QTextCharFormat theme;
        theme.setFontPointSize(20);
        theme.setFontStretch(80);
        QCOMPARE(jp::resolveCharFormat(theme, env).fontStretch(), jp::stretchToAsk(80));
        theme.setFontStretch(100);
        QVERIFY(!jp::resolveCharFormat(theme, env).hasProperty(QTextFormat::FontStretch));
    }

    // Selawik's Light, Semilight, and Semibold are families of their own on
    // Linux and styles of the family Selawik on Windows and the Mac: a font
    // whose stand-in is one of them is drawn in Selawik's style, not Arimo.
    void standInNamesOnThisPlatform()
    {
        QCOMPARE(jp::standInOnThisPlatform(QStringLiteral("Arimo")), QStringLiteral("Arimo"));
        QCOMPARE(jp::standInOnThisPlatform(QStringLiteral("Arimo Madeup Style")), QStringLiteral("Arimo"));
        QCOMPARE(jp::standInOnThisPlatform(QStringLiteral("Nonesuch Madeup")), QStringLiteral("Nonesuch Madeup"));
        QCOMPARE(jp::standInOnThisPlatform(QString()), QString());
        for (const char *weight : {"Light", "Semilight", "Semibold"}) {
            const QString name = QStringLiteral("Selawik ") + QLatin1String(weight), here = jp::standInOnThisPlatform(name);
            QVERIFY2(QFontDatabase::hasFamily(here), qPrintable(name + QStringLiteral(" -> ") + here));
        }
        QVERIFY(jp::substituteStyleName(QStringLiteral("Arimo")).isEmpty());
        QVERIFY(jp::substituteStyleName(QStringLiteral("Franklin Gothic Heavy")).isEmpty());
        if (QFontDatabase::hasFamily(QStringLiteral("Segoe UI Light")) || QFontDatabase::hasFamily(QStringLiteral("Segoe UI Semibold")))
            QSKIP("Segoe UI is installed");
        const bool family = QFontDatabase::hasFamily(QStringLiteral("Selawik Light"));
        QCOMPARE(jp::substituteStyleName(QStringLiteral("Segoe UI Light")), family ? QString() : QStringLiteral("Light"));
        QCOMPARE(jp::substituteStyleName(QStringLiteral("Segoe UI Semibold")), family ? QString() : QStringLiteral("Semibold"));
        // The font database is asked for the family's name: Selawik, never Arimo.
        for (const QString &name : {QStringLiteral("Segoe UI Light"), QStringLiteral("Segoe UI Semilight"), QStringLiteral("Segoe UI Semibold")}) {
            QFont f(name);
            f.setFamilies({name});
            QVERIFY2(QFontInfo(f).family().startsWith(QLatin1String("Selawik")), qPrintable(name + QStringLiteral(": ") + QFontInfo(f).family()));
        }
        // And a run in it is drawn in the style its name says where the family lists it (Light
        // apart from Semilight, which both weigh 300).
        jp::LayoutEnv env;
        for (const char *weight : {"Light", "Semilight", "Semibold"}) {
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QStringLiteral("Segoe UI ") + QLatin1String(weight)});
            f.setFontPointSize(20);
            const QFontInfo info(jp::resolveCharFormat(f, env).font());
            QVERIFY2(info.family().startsWith(QLatin1String("Selawik")), qPrintable(QStringLiteral("%1|%2").arg(info.family(), info.styleName())));
            if (QFontDatabase::styles(info.family()).contains(QLatin1String(weight), Qt::CaseInsensitive))
                QVERIFY2(info.styleName().compare(QLatin1String(weight), Qt::CaseInsensitive) == 0, qPrintable(QStringLiteral("%1|%2").arg(info.family(), info.styleName())));
        }
    }

    // A bundled font whose letters slant but whose flags say upright is not
    // told from the upright face by DirectWrite, and Qt on Windows drew
    // Franklin Gothic Heavy in the italic: its flags are set on loading,
    // with the checksums a font program checks; nothing else changes.
    void italicFlagsOfBundledFont()
    {
        const auto read = [](const char *name) {
            QFile f(QStringLiteral(JP_TEST_DATA "/../../resources/fonts/") + QLatin1String(name));
            return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
        };
        const auto u16 = [](const QByteArray &b, qsizetype at) { return quint16(uchar(b[at]) << 8 | uchar(b[at + 1])); };
        const auto u32 = [&](const QByteArray &b, qsizetype at) { return quint32(u16(b, at)) << 16 | u16(b, at + 2); };
        const auto sum = [&](QByteArray b) {
            b.append(QByteArray((4 - b.size() % 4) % 4, '\0'));
            quint32 total = 0;
            for (qsizetype at = 0; at < b.size(); at += 4) total += u32(b, at);
            return total;
        };
        const QByteArray broken = read("LibreFranklin-ExtraBoldItalic.ttf");
        QVERIFY(!broken.isEmpty());
        const QByteArray fixed = jp::fontWithItalicFlags(broken);
        QVERIFY(!fixed.isEmpty());
        QCOMPARE(fixed.size(), broken.size());
        QMap<QByteArray, qsizetype> tables;
        for (int i = 0, n = u16(fixed, 4); i < n; ++i) tables.insert(fixed.mid(12 + 16 * i, 4), 12 + 16 * i);
        const qsizetype os2 = u32(fixed, tables.value("OS/2") + 8), head = u32(fixed, tables.value("head") + 8);
        QVERIFY(u16(fixed, os2 + 62) & 1);          // fsSelection: italic
        QVERIFY(!(u16(fixed, os2 + 62) & 64));      // and not regular
        QVERIFY(u16(fixed, head + 44) & 2);         // macStyle: italic
        QCOMPARE(u16(fixed, os2 + 4), quint16(800));   // the weight is the font's own
        // Every table's checksum, the head table's with its adjustment left out, and the whole file's.
        for (auto it = tables.cbegin(); it != tables.cend(); ++it) {
            const qsizetype at = u32(fixed, it.value() + 8), length = u32(fixed, it.value() + 12);
            QByteArray table = fixed.mid(at, length);
            if (it.key() == "head") table.replace(8, 4, QByteArray(4, '\0'));
            QCOMPARE(sum(table), u32(fixed, it.value() + 4));
        }
        QCOMPARE(sum(fixed), quint32(0xB1B0AFBA));
        // Only the flags and checksums differ.
        int differing = 0;
        for (qsizetype i = 0; i < fixed.size(); ++i) differing += fixed[i] != broken[i];
        QVERIFY2(differing <= 14, qPrintable(QString::number(differing)));   // two flag bytes, three checksums, the adjustment
        // Fonts that need nothing are left alone.
        QVERIFY(jp::fontWithItalicFlags(fixed).isEmpty());
        QVERIFY(jp::fontWithItalicFlags(read("LibreFranklin-ExtraBold.ttf")).isEmpty());
        QVERIFY(jp::fontWithItalicFlags(read("LibreFranklin-BoldItalic.ttf")).isEmpty());
        QVERIFY(jp::fontWithItalicFlags(QByteArray("not a font")).isEmpty());
        QVERIFY(jp::fontWithItalicFlags(QByteArray()).isEmpty());
    }

    // Every bundled face named Italic (family and style together: Linux lists
    // "Libre Franklin ExtraBold Italic" as a family with the style Regular) is
    // italic to the platform's font database, and no other is. The ExtraBold
    // Italic was upright to DirectWrite, so a request for Franklin Gothic
    // Heavy could get it.
    void bundledFacesKeepTheirSlant()
    {
        QStringList wrong;
        for (const QString &family : jp::bundledFamilies())
            for (const QString &style : QFontDatabase::styles(family)) {
                const QString name = family + QLatin1Char(' ') + style;
                if (name.contains(QLatin1String("Oblique"), Qt::CaseInsensitive)) continue;   // (italic() is false for an oblique)
                if (QFontDatabase::italic(family, style) != name.contains(QLatin1String("Italic"), Qt::CaseInsensitive)) wrong << name;
            }
        QVERIFY2(wrong.isEmpty(), qPrintable(wrong.join(QLatin1String(", "))));
    }

    // A missing font's stand-in is drawn as high as the real font's letters
    // (Libre Franklin's capitals stand 11% taller than Franklin Gothic
    // Demi's), with its widths and line breaks unchanged.
    void standInDrawnAtRealHeight()
    {
        if (QFontDatabase::hasFamily(QStringLiteral("Franklin Gothic Demi"))) QSKIP("Franklin Gothic Demi is installed");
        jp::LayoutEnv env;
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("Franklin Gothic Demi")});
        f.setFontPointSize(100);
        const QTextCharFormat r = jp::resolveCharFormat(f, env);
        if (QFontInfo(r.font()).family() != QLatin1String("Libre Franklin")) QSKIP("the stand-in isn't drawn here");
        QVERIFY(r.hasProperty(jp::tp::GlyphScaleY));
        const double scale = r.property(jp::tp::GlyphScaleY).toDouble();
        QVERIFY2(scale > 0.85 && scale < 0.95, qPrintable(QString::number(scale)));

        // Drawn: the H's ink is the stand-in's capital height times the scale.
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("t");
        t->rect = QRectF(36, 36, 500, 200);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->storyId = doc->createStory(QStringLiteral("H"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            c.mergeCharFormat(f);
        }
        doc->pages[0]->items.push_back(t);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        QImage img(612 * 2, 792 * 2, QImage::Format_ARGB32);
        img.fill(Qt::white);
        {
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.scale(2, 2);
            jp::Renderer::paintPage(&p, ctx, 0);
        }
        int top = img.height(), bottom = -1;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) < 128) { top = std::min(top, y); bottom = std::max(bottom, y); }
        QVERIFY(bottom > top);
        const double ink = (bottom - top + 1) / 2.0;
        QFont sf(QStringLiteral("Libre Franklin"));
        sf.setWeight(QFont::Weight(r.fontWeight()));
        sf.setPixelSize(1000);
        const double cap = QFontMetricsF(sf).capHeight() / 1000 * 100;   // the stand-in's H at 100 pt
        QVERIFY2(std::abs(ink - cap * scale) < cap * 0.03, qPrintable(QStringLiteral("ink %1, stand-in cap %2, scale %3").arg(ink).arg(cap).arg(scale)));
    }

    // Text drawn run by run (a stand-in drawn taller or shorter) keeps its
    // underline and strikethrough.
    void scaledRunsKeepLines()
    {
        if (QFontDatabase::hasFamily(QStringLiteral("Franklin Gothic Demi"))) QSKIP("Franklin Gothic Demi is installed");
        jp::LayoutEnv env;
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("Franklin Gothic Demi")});
        f.setFontPointSize(60);
        if (!jp::resolveCharFormat(f, env).hasProperty(jp::tp::GlyphScaleY)) QSKIP("the stand-in isn't drawn here");
        auto ink = [&](bool underline, bool strike) {
            auto doc = Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<TextItem>();
            t->id = QStringLiteral("t");
            t->rect = QRectF(36, 36, 500, 200);
            t->insets = QMarginsF(0, 0, 0, 0);
            t->storyId = doc->createStory(QStringLiteral("mmmm"));
            QTextCharFormat g = f;
            g.setFontUnderline(underline);
            g.setFontStrikeOut(strike);
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            c.mergeCharFormat(g);
            doc->pages[0]->items.push_back(t);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            QImage img(612, 300, QImage::Format_ARGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            jp::Renderer::paintPage(&p, ctx, 0);
            p.end();
            QVector<int> rows;
            for (int y = 0; y < img.height(); ++y) {
                int n = 0;
                for (int x = 0; x < img.width(); ++x) n += qGray(img.pixel(x, y)) < 128;
                rows << n;
            }
            return rows;
        };
        const QVector<int> plain = ink(false, false), under = ink(true, false), strike = ink(false, true);
        int belowPlain = 0, belowUnder = 0, extraStrike = 0;
        int last = 0;
        for (int y = 0; y < plain.size(); ++y) if (plain[y]) last = y;
        for (int y = last + 1; y < plain.size(); ++y) { belowPlain += plain[y]; belowUnder += under[y]; }
        for (int y = 0; y < plain.size(); ++y) extraStrike += std::max(0, strike[y] - plain[y]);
        QVERIFY2(belowUnder > 100, qPrintable(QString::number(belowUnder)));   // a line under the letters
        QCOMPARE(belowPlain, 0);
        QVERIFY2(extraStrike > 100, qPrintable(QString::number(extraStrike)));   // a line through them
    }

    // A run that states its character scaling, even 100%, still gets its
    // missing font's stand-in weight and narrowing (times its own scaling):
    // sign designs' phone numbers in Franklin Gothic Heavy came out thin.
    void standInWithOwnScaling()
    {
        if (QFontDatabase::hasFamily(QStringLiteral("Franklin Gothic Heavy"))) QSKIP("Franklin Gothic Heavy is installed");
        jp::LayoutEnv env;
        QTextCharFormat plain;
        plain.setFontFamilies(QStringList{QStringLiteral("Franklin Gothic Heavy")});
        plain.setFontPointSize(10);
        const QTextCharFormat base = jp::resolveCharFormat(plain, env);
        // The stand-in's narrowing as the table has it (97), before the scaling.
        const int narrowing = jp::substituteStretch(QStringLiteral("Franklin Gothic Heavy"));
        QVERIFY(narrowing < 100);
        const QString sample = QStringLiteral("The quick brown fox jumps over the lazy dog. Pack my box with five dozen liquor jugs!");
        double width100 = 0;
        for (int scale : {100, 80}) {
            QTextCharFormat f = plain;
            f.setFontStretch(scale);
            const QTextCharFormat r = jp::resolveCharFormat(f, env);
            QCOMPARE(r.fontWeight(), base.fontWeight());
            // The stretch asked of Qt is the narrowing times the scaling (the Mac is asked for another number).
            const int want = int(std::lround(narrowing * scale / 100.0));
            QCOMPARE(r.hasProperty(QTextFormat::FontStretch) ? r.fontStretch() : 100, want == 100 ? 100 : jp::stretchToAsk(want));
            // And it is drawn that wide: 80% of the run's width at 100%.
            const double width = QFontMetricsF(r.font()).horizontalAdvance(sample);
            if (scale == 100) width100 = width;
            else {
                const int sent = jp::stretchToAsk(want);
                const double ratio = width / width100;
                QVERIFY2(std::abs(ratio / (scale / 100.0) - 1) < 0.02 + (sent != want ? 2.0 / sent : 0.0), qPrintable(QString::number(ratio)));
            }
#ifndef Q_OS_MACOS
            // The face drawn (the Mac's font matching picks Bold for ExtraBold, with or without a stretch).
            QVERIFY2(QFontInfo(r.font()).weight() >= 750, qPrintable(QFontInfo(r.font()).styleName()));
#endif
        }
    }

    // A missing AG_Futura keeps its own half-em spaces with its stand-in
    // Jost, whose spaces are 0.3 em (word gaps measured in Publisher's PDFs).
    void substituteSpaceWidth()
    {
        if (QFontDatabase::hasFamily(QStringLiteral("AG_Futura"))) QSKIP("AG_Futura is installed: its own spaces are used");
        jp::LayoutEnv env;
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("AG_Futura")});
        f.setFontPointSize(20);
        const QTextCharFormat r = jp::resolveCharFormat(f, env);
        QTextLayout tl(QStringLiteral("A B"), r.font());
        tl.beginLayout();
        QTextLine line = tl.createLine();
        line.setLineWidth(1000);
        tl.endLayout();
        const double space = line.cursorToX(2) - line.cursorToX(1);
        QVERIFY2(std::abs(space - 10) < 0.3, qPrintable(QString::number(space)));
        // Fonts whose stand-ins match keep their spaces as drawn.
        QTextCharFormat arial = f;
        arial.setFontFamilies(QStringList{QStringLiteral("Arial")});
        QVERIFY(!jp::resolveCharFormat(arial, env).hasProperty(QTextFormat::FontWordSpacing));
    }

    // Publisher 2000 and 98 files give a run's font as a number in each
    // slot of its font container (not a container per slot, as later
    // versions do). The sample's second box says it's in Arial, and
    // Publisher shows it in Arial.
    void pub2000RunFonts()
    {
        for (const char *name : {"poi-Sample2000.pub", "poi-Sample98.pub"}) {
            jp::PubImportReport rep;
            QString err;
            auto doc = jp::importPublisherFile(QStringLiteral(JP_TEST_DATA "/pub/") + QLatin1String(name), &err, &rep);
            QVERIFY2(doc, qPrintable(err));
            QVERIFY2(rep.fontsUsed.contains(QStringLiteral("Arial")), qPrintable(QLatin1String(name) + rep.fontsUsed.join(QLatin1Char(','))));
            QString arialText;
            for (const auto &it : doc->pages[0]->items)
                if (auto *t = dynamic_cast<jp::TextItem *>(it.get())) {
                    QTextCursor c(doc->storyDoc(t->storyId));
                    c.movePosition(QTextCursor::NextCharacter);
                    if (c.charFormat().fontFamilies().toStringList().value(0) == QLatin1String("Arial")) arialText = doc->storyDoc(t->storyId)->toPlainText();
                }
            QVERIFY2(arialText.contains(QStringLiteral("Arial, 20 point")), qPrintable(arialText));
        }
    }

    // Fonts in .pub: JeffPub's look-alikes (Tinos) go under the standard
    // font's name (Times New Roman), unless the publication came from a .pub
    // that named the look-alike itself; then the name it had is kept.
    void pubKeepsFontsItCameWith()
    {
        using namespace jp;
        auto make = [](const QStringList &pubFonts) {
            auto doc = Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, 72, 400, 100);
            t->storyId = doc->createStory(QStringLiteral("Set in Tinos"));
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies(QStringList{QStringLiteral("Tinos")});
            c.mergeCharFormat(f);
            doc->pages[0]->items.push_back(t);
            doc->pubFonts = pubFonts;
            return doc;
        };
        QTemporaryDir dir;
        auto fontAfter = [&](const Document &d) {
            const QString path = dir.filePath(QStringLiteral("fonts.pub"));
            QString err;
            if (!exportPublisher(d, path, &err)) return QStringLiteral("save failed: ") + err;
            PubImportReport rep;
            auto back = importPublisherFile(path, &err, &rep);
            if (!back) return QStringLiteral("open failed: ") + err;
            return rep.fontsUsed.join(QLatin1Char(','));
        };
        QCOMPARE(fontAfter(*make({})), QStringLiteral("Times New Roman"));
        QCOMPARE(fontAfter(*make({QStringLiteral("Tinos")})), QStringLiteral("Tinos"));
        // Opening a .pub remembers its fonts, and .jpub keeps the list.
        auto fromPub = make({QStringLiteral("Tinos")});
        const QString pubPath = dir.filePath(QStringLiteral("orig.pub"));
        QString err;
        QVERIFY(exportPublisher(*fromPub, pubPath, &err));
        auto opened = importPublisherFile(pubPath, &err);
        QVERIFY(opened && opened->pubFonts.contains(QStringLiteral("Tinos")));
        const QString jpub = dir.filePath(QStringLiteral("kept.jpub"));
        QVERIFY(savePublication(*opened, jpub, QImage(), &err));
        auto reloaded = loadPublication(jpub, &err);
        QVERIFY(reloaded && reloaded->pubFonts == opened->pubFonts);
    }

    // Publisher's single line height for fonts JeffPub draws with stand-ins,
    // in ems: each checked line by line against Publisher's own PDFs of 284
    // publications (Oct 7). The stand-ins' own tables were up to 0.4 em off
    // (Agency FB), and Arial Narrow and Bookman Old Style were read with
    // the wrong set of metrics.
    void publisherLineHeights()
    {
        using namespace jp;
        struct Case { const char *family; bool bold; double em; };
        const Case cases[] = {
            {"Arial Rounded MT Bold", true, (1938.0 + 432) / 2048},   // its 26 pt lines 30.06 pt apart
            {"Verdana", false, (1566.0 + 423 + 202) / 2048},
            {"Comic Sans MS", false, (1638.0 + 564) / 2048},
            {"Bookman Old Style", false, (1929.0 + 475) / 2048},
            {"Arial Narrow", false, (1916.0 + 434) / 2048},
            {"Times New Roman", true, (1387.0 + 442 + 307) / 2048},
            {"Times New Roman", false, (1420.0 + 442 + 307) / 2048},
            {"Agency FB", true, (2042.0 + 410) / 2048},
        };
        for (const Case &c : cases) {
            QFont f(QString::fromLatin1(c.family));
            f.setFamilies({QString::fromLatin1(c.family)});
            f.setBold(c.bold);
            f.setPointSizeF(12);
            const double em = naturalLineEm(f, QString::fromLatin1(c.family));
            QVERIFY2(std::abs(em - c.em) < 1e-6, qPrintable(QStringLiteral("%1: %2, not %3").arg(QLatin1String(c.family)).arg(em).arg(c.em)));
        }
    }

    // Line spacing of the fonts JeffPub knows Publisher's metrics for doesn't
    // depend on whether the font is installed: with the real Gill Sans MT on
    // Windows, lines came out 8% tighter than Publisher's. Here a bundled
    // font renamed "Arial" stands in for an installed Arial.
    void knownLineMetricsWhenInstalled()
    {
        using namespace jp;
        const double notInstalled = naturalLineEm(QFont(QStringLiteral("Arial")), QStringLiteral("Arial"));
        QFile f(QStringLiteral(JP_TEST_DATA "/../../resources/fonts/Arimo-Regular.ttf"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray font = f.readAll();
        // Same length, so the font's tables stay where they are.
        font.replace(QByteArray("\x00" "A\x00r\x00i\x00m\x00o", 10), QByteArray("\x00" "A\x00r\x00i\x00" "a\x00l", 10));
        font.replace(QByteArray("Arimo"), QByteArray("Arial"));
        const int id = QFontDatabase::addApplicationFontFromData(font);
        jp::fontsChanged();
        QVERIFY(id >= 0);
        QVERIFY(QFontDatabase::applicationFontFamilies(id).contains(QStringLiteral("Arial")));
        QFont arial(QStringLiteral("Arial"));
        arial.setPointSizeF(10);
        const double installed = naturalLineEm(arial, QStringLiteral("Arial"));
        QFontDatabase::removeApplicationFont(id);
        jp::fontsChanged();
        QVERIFY2(std::abs(installed - notInstalled) < 1e-6, qPrintable(QStringLiteral("%1 vs %2").arg(installed).arg(notInstalled)));
        QVERIFY(std::abs(notInstalled - (1491.0 + 431 + 307) / 2048) < 1e-6);
    }

    // A run's fonts are kept per script (slots in container 0x24); slot 0 is
    // the one for Latin text, as Publisher writes it. A run with fonts for
    // other scripts only (older newsletters: Courier New in slots 3-7,
    // Sendnya in slot 0x0c) takes its Latin font from its style, as
    // Publisher shows it, not the first slot's font.
    void pubLatinFontSlot()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 400, 100);
        // The file's first font (Arial) is the one a run without a Latin
        // font falls back to.
        t->storyId = doc->createStory(QStringLiteral("Set in Arial\nTyped in Courier New"));
        QTextCursor c(doc->storyDoc(t->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontFamilies(QStringList{QStringLiteral("Arial")});
        c.mergeCharFormat(f);
        c.movePosition(QTextCursor::End);
        c.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        f.setFontFamilies(QStringList{QStringLiteral("Courier New")});
        c.mergeCharFormat(f);
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("slots.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        PubImportReport rep;
        QVERIFY(importPublisherFile(path, &err, &rep));
        QVERIFY(rep.fontsUsed.contains(QStringLiteral("Courier New")));
        // Move the Latin and two other slots to slots Publisher uses for other
        // scripts: 00 -> 06, 01 -> 07, 02 -> 0c.
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(in.readAll(), &file));
        in.close();
        QByteArray q = file.stream(QStringLiteral("Quill/QuillSub/CONTENTS"));
        const QByteArray tail("\x88\x08\x00\x00\x00\x00\x18", 7);
        int moved = 0;
        for (qsizetype at = q.indexOf(tail); at > 0; at = q.indexOf(tail, at + 1)) {
            const char slot = q[at - 1];
            if (slot == 0 || slot == 1 || slot == 2) {
                q[at - 1] = slot == 0 ? 6 : slot == 1 ? 7 : 0x0c;
                ++moved;
            }
        }
        QVERIFY(moved >= 3);
        QVERIFY(file.setStream(QStringLiteral("Quill/QuillSub/CONTENTS"), q));
        const QString patched = dir.filePath(QStringLiteral("patched.pub"));
        QFile out(patched);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(cfb::write(file));
        out.close();
        PubImportReport rep2;
        auto back = importPublisherFile(patched, &err, &rep2);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(!rep2.fontsUsed.contains(QStringLiteral("Courier New")), qPrintable(rep2.fontsUsed.join(',')));
    }

    // Selecting text shows its size in the ribbon's size box, and the number
    // has room to be seen (the field inside the box mustn't be padded away).
    void fontSizeBoxShowsSelection()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("Hello world"));
        doc->pages[0]->items.push_back(t);
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(24);
            c.mergeCharFormat(f);
        }
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        const QString id = ed->doc()->pages[0]->items[0]->id;
        ed->select(id);
        ed->beginTextEdit(id, 0);
        QTextCursor c = ed->cursor();
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        // The ribbon catches up after a moment (longer on a slow machine).
        jp::SizeCombo *box = nullptr;
        auto shown = [&] {
            for (jp::SizeCombo *s : w.findChildren<jp::SizeCombo *>())
                if (s->isVisible()) { box = s; break; }
            return box && box->currentText() == QStringLiteral("24");
        };
        QTRY_VERIFY_WITH_TIMEOUT(shown(), 3000);
        QLineEdit *le = box->lineEdit();
        // Room for the widest size even with Windows' larger UI fonts (125%).
        QFont big = le->font();
        big.setPointSizeF(big.pointSizeF() * 1.25);
        QVERIFY2(le->contentsRect().width() >= QFontMetrics(big).horizontalAdvance(QStringLiteral("288")),
                 qPrintable(QStringLiteral("size field is %1 px wide").arg(le->contentsRect().width())));
    }

    // Every font list row is the same height, and every name stays inside it.
    void fontListRows()
    {
        jp::FontCombo combo;
        auto *view = combo.view();
        auto *model = combo.model();
        const int n = qMin(model->rowCount(), 400);
        QStyleOptionViewItem opt;
        opt.initFrom(view);
        opt.font = view->font();
        const int h = view->itemDelegate()->sizeHint(opt, model->index(4, 0)).height();
        QVERIFY(h >= 20 && h <= 40);
        QImage img(260, h * n, QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        for (int i = 0; i < n; ++i) {
            const QModelIndex idx = model->index(i, 0);
            QCOMPARE(view->itemDelegate()->sizeHint(opt, idx).height(), h);
            opt.rect = QRect(0, i * h, 260, h);
            view->itemDelegate()->paint(&p, opt, idx);
        }
        p.end();
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) img.save(qEnvironmentVariable("JP_SHOT_DIR") + "/fontlist.png");
        // Every row draws something.
        for (int i = 0; i < n; ++i) {
            bool ink = false;
            for (int y = i * h; y < (i + 1) * h && !ink; ++y)
                for (int x = 0; x < 260 && !ink; ++x) ink = qGray(img.pixel(x, y)) < 200;
            QVERIFY2(ink, qPrintable(model->index(i, 0).data().toString()));
        }
    }

    // A font with no italic face (Impact; Garamond's bold) is slanted by
    // Publisher itself, a third of each letter's height (the shear its PDFs
    // carry), from the upright face. The stand-in was drawn in its own italic
    // (EB Garamond's bold italic has curled m and n where Garamond's slanted
    // bold has serifs: a banner's "Happy Retirement" scored 12) or slanted
    // by Qt's fifth.
    void missingFontWithoutItalicIsSlantedAThird()
    {
        struct Case { const char *family; bool bold, italic, slanted; };
        const Case cases[] = {{"Garamond", true, true, true}, {"Garamond", false, true, false}, {"Garamond", true, false, false},
                              {"Impact", false, true, true}, {"Impact", false, false, false}};
        for (const Case &c : cases) {
            const QString fam = QString::fromLatin1(c.family);
            if (QFontDatabase::hasFamily(fam)) continue;   // installed: its own faces
            const QByteArray name = QByteArray(c.family) + (c.bold ? " bold" : "") + (c.italic ? " italic" : "");
            QTextCharFormat f;
            f.setFontFamilies(QStringList{fam});
            f.setFontPointSize(200);
            f.setFontWeight(c.bold ? QFont::Bold : QFont::Normal);
            f.setFontItalic(c.italic);
            const QTextCharFormat r = jp::resolveCharFormat(f, jp::LayoutEnv());
            QVERIFY2(r.hasProperty(jp::tp::SyntheticItalic) == c.slanted, name.constData());
            QCOMPARE(r.fontItalic(), c.italic && !c.slanted);

            auto doc = Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<TextItem>();
            t->id = QStringLiteral("t");
            t->rect = QRectF(36, 36, 500, 400);
            t->insets = QMarginsF(0, 0, 0, 0);
            t->storyId = doc->createStory(QStringLiteral("l"));
            {
                QTextCursor cur(doc->storyDoc(t->storyId));
                cur.select(QTextCursor::Document);
                cur.mergeCharFormat(f);
            }
            doc->pages[0]->items.push_back(t);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            QImage img(612, 792, QImage::Format_ARGB32);
            img.fill(Qt::white);
            {
                QPainter p(&img);
                p.setRenderHint(QPainter::Antialiasing);
                jp::Renderer::paintPage(&p, ctx, 0);
            }
            // The stem of the l: the left edge of the ink through the middle
            // of its height, serifs left out.
            int top = img.height(), bottom = -1;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qGray(img.pixel(x, y)) < 128) { top = std::min(top, y); bottom = std::max(bottom, y); }
            QVERIFY2(bottom > top, name.constData());
            double sy = 0, sx = 0, syy = 0, sxy = 0;
            int n = 0;
            for (int y = top + (bottom - top) * 35 / 100; y <= top + (bottom - top) * 65 / 100; ++y) {
                int x = 0;
                while (x < img.width() && qGray(img.pixel(x, y)) >= 128) ++x;
                if (x == img.width()) continue;
                sy += y; sx += x; syy += double(y) * y; sxy += double(x) * y;
                ++n;
            }
            QVERIFY2(n > 20, name.constData());
            const double slant = -(n * sxy - sx * sy) / (n * syy - sy * sy);   // how far the top leans right, per row
            const QString got = QString::fromLatin1(name) + QStringLiteral(": ") + QString::number(slant);
            if (c.slanted) QVERIFY2(std::abs(slant - 1.0 / 3) < 0.03, qPrintable(got));
            else if (!c.italic) QVERIFY2(std::abs(slant) < 0.03, qPrintable(got));   // upright stays upright
        }
    }

    // Script MT Bold's letters stand lower than its stand-in Pacifico's: a
    // banner's W was 220 points tall at Publisher's 165, and the score
    // against Publisher's picture was 9.5. The stand-in is drawn at 0.8 of
    // its height, its widths unchanged.
    void scriptMtBoldStandInIsDrawnLower()
    {
        const QString fam = QStringLiteral("Script MT Bold");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Script MT Bold is installed here");
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{fam});
        cf.setFontPointSize(100);
        const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
        if (QFontInfo(r.font()).family() != QLatin1String("Pacifico")) QSKIP("the stand-in isn't drawn here");
        QCOMPARE(r.property(jp::tp::GlyphScaleY).toDouble(), 0.8);
    }

    // Gill Sans MT Condensed's letters are narrower than the stand-in's
    // squeezed to the same advances (a banner's U is 75 points wide at 233 pt
    // in Publisher's picture, 0.32 em; 0.39 em at the table's old 83%): the
    // letters keep the font's own widths (resources/fontwidths.txt) as their
    // spacing, so the stand-in is squeezed to the letters' ink.
    void gillSansCondensedLettersAreAsNarrowAsTheOriginals()
    {
        const QString fam = QStringLiteral("Gill Sans MT Condensed");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Gill Sans MT Condensed is installed here");
        QTextCharFormat probe;
        probe.setFontFamilies(QStringList{fam});
        probe.setFontPointSize(200);
        if (QFontInfo(jp::resolveCharFormat(probe, jp::LayoutEnv()).font()).family() != QLatin1String("Archivo Narrow")) QSKIP("the stand-in isn't drawn here");
        // The ink of a 200 pt U drawn on a page in a family, ink edge to ink edge.
        auto inkOfU = [](const QString &family) {
            QTextCharFormat f;
            f.setFontFamilies(QStringList{family});
            f.setFontPointSize(200);
            auto doc = Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<TextItem>();
            t->id = QStringLiteral("t");
            t->rect = QRectF(36, 36, 500, 400);
            t->insets = QMarginsF(0, 0, 0, 0);
            t->storyId = doc->createStory(QStringLiteral("U"));
            {
                QTextCursor c(doc->storyDoc(t->storyId));
                c.select(QTextCursor::Document);
                c.mergeCharFormat(f);
            }
            doc->pages[0]->items.push_back(t);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            QImage img(612, 792, QImage::Format_ARGB32);
            img.fill(Qt::white);
            {
                QPainter p(&img);
                p.setRenderHint(QPainter::Antialiasing);
                jp::Renderer::paintPage(&p, ctx, 0);
            }
            int left = img.width(), right = -1;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qGray(img.pixel(x, y)) < 128) { left = std::min(left, x); right = std::max(right, x); }
            return right >= left ? (right - left + 1) / 200.0 : 0.0;
        };
        const double missing = inkOfU(fam), plain = inkOfU(QStringLiteral("Archivo Narrow"));
        QVERIFY(missing > 0 && plain > 0);
        // Against the stand-in drawn as it comes on the same system (each text
        // engine shades a letter's edges its own way: the Mac's came out 0.295
        // em where Linux's is 0.32), and near the original's 0.32 em.
        QVERIFY2(std::abs(missing / plain - 0.66) < 0.03, qPrintable(QStringLiteral("%1 of %2").arg(missing).arg(plain)));
        QVERIFY2(std::abs(missing - 0.32) < 0.04, qPrintable(QString::number(missing)));
    }

    // A wave with no amplitude of its own is Publisher's: its crests and
    // troughs 2809 of 21600 of the frame's height, as its PDFs draw a "Wet
    // Paint" sign's edge; libmspub had 1400, half the swing.
    void waveAsPublisherDrawsIt()
    {
        const libmspub::CustomShape *wave = libmspub::getCustomShape(libmspub::WAVE);
        QVERIFY(wave && wave->m_numDefaultAdjustValues > 0);
        QCOMPARE(wave->mp_defaultAdjustValues[0], 2809);
    }

    // A missing stock font that Publisher kerns by no pairs isn't kerned by
    // its stand-in's either: Century Schoolbook Bold's "Wa" is as wide as its
    // two letters (TeX Gyre Schola closes them up by 0.07 em), and so is
    // Book Antiqua's "AV" (TeX Gyre Pagella, 0.14 em).
    void missingFontsWithoutPairsDontKern()
    {
        struct Case { const char *family; bool bold; char16_t a, b; };
        int checked = 0;
        for (const Case &k : {Case{"Century Schoolbook", true, u'W', u'a'}, Case{"Book Antiqua", false, u'A', u'V'}}) {
            const QString fam = QString::fromLatin1(k.family);
            if (QFontDatabase::hasFamily(fam)) continue;
            const QHash<char16_t, double> *widths = jp::originalLetterWidths(fam, k.bold, false);
            QVERIFY(widths && !jp::originalKerning(fam, k.bold, false));
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontPointSize(100);
            if (k.bold) cf.setFontWeight(QFont::Bold);
            c.insertText(QString(QChar(k.a)) + QChar(k.b), cf);
            jp::FrameSpec fs;
            fs.size = QSizeF(2000, 400);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            jp::StoryLayout lay;
            lay.build(&doc, {fs}, jp::LayoutEnv());
            auto x = [&](int pos) {
                int frame = -1;
                QRectF r;
                lay.caretRect(pos, &frame, &r);
                return r.x();
            };
            const double want = (widths->value(k.a) + widths->value(k.b)) * 100;
            QVERIFY2(std::abs(x(2) - x(0) - want) < 0.5, qPrintable(QStringLiteral("%1: %2, not %3").arg(fam).arg(x(2) - x(0)).arg(want)));
            ++checked;
        }
        if (!checked) QSKIP("the fonts are installed here");
    }

    // The stand-ins of two fonts Publisher's sign designs set their biggest
    // words in are drawn as high as the real letters, by the heights read
    // from its PDFs: Lobster's capitals stand 11% too tall for Forte's, and
    // Archivo Narrow's 12% for Tw Cen MT Condensed's.
    void signFontStandInsAreDrawnAtRealHeight()
    {
        struct Case { const char *family, *standIn; double scale; };
        int checked = 0;
        for (const Case &k : {Case{"Forte", "Lobster", 0.9}, Case{"Tw Cen MT Condensed", "Archivo Narrow", 0.857}}) {
            const QString fam = QString::fromLatin1(k.family);
            if (QFontDatabase::hasFamily(fam)) continue;
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontPointSize(24);
            const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
            if (QFontInfo(r.font()).family() != QLatin1String(k.standIn)) continue;
            QVERIFY2(r.hasProperty(jp::tp::GlyphScaleY), k.family);
            QCOMPARE(r.property(jp::tp::GlyphScaleY).toDouble(), k.scale);
            ++checked;
        }
        if (!checked) QSKIP("the stand-ins aren't drawn here");
    }

    // Script MT Bold's stand-in, Pacifico, is drawn narrower than the
    // original's advances ask (98%): its letters are wider and heavier, and
    // ran into each other in a sign's "Garage Sale".
    void scriptMtBoldStandInIsNarrowed()
    {
        const QString fam = QStringLiteral("Script MT Bold");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Script MT Bold is installed here");
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{fam});
        cf.setFontPointSize(24);
        const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
        if (QFontInfo(r.font()).family() != QLatin1String("Pacifico")) QSKIP("the stand-in isn't drawn here");
        QCOMPARE(r.fontStretch(), jp::stretchToAsk(86));
    }

    // Best Fit counts a missing font's text by the original's height, not its
    // stand-in's: Franklin Gothic Demi's text is 1.13 em tall, Libre
    // Franklin's 1.21, and a sign's one-line headline set at 209 pt with 125%
    // spacing shrank to 98% for the rectangle beneath its box (its wrap
    // distance 2.9 pt into the box) that Publisher draws it above, unshrunk.
    void bestFitWrapsByTheOriginalsTextHeight()
    {
        const QString fam = QStringLiteral("Franklin Gothic Demi");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Franklin Gothic Demi is installed here");
        auto doc = Document::blank(QSizeF(792, 612));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(54.72, 49.5, 682.56, 255);
        t->autofit = TextItem::BestFit;
        t->fitAsStored = true;
        t->storyId = doc->createStory(QStringLiteral("Private"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat big;
            big.setFontFamilies(QStringList{fam});
            big.setFontPointSize(209.09);
            big.setFontWeight(QFont::Bold);
            c.mergeCharFormat(big);
            QTextBlockFormat bf;
            bf.setLineHeight(125, QTextBlockFormat::ProportionalHeight);
            c.mergeBlockFormat(bf);
        }
        doc->pages[0]->items.push_back(t);
        auto below = std::make_shared<ShapeItem>();
        below->rect = QRectF(47.52, 304.5, 696.96, 271.5);
        doc->pages[0]->items.push_back(below);
        LayoutCache cache;
        RenderOptions opt;
        QCOMPARE(cache.textFrame(*doc, *t, 1, opt).fitScale, 1.0);
    }

    // The first baseline of single spaced text sits higher in Franklin
    // Gothic and Gill Sans MT up to 31 pt (0.088 and 0.096 em; Publisher's
    // PDFs of 400 such lines), while larger text and text at any other
    // spacing stays at the font's ascent.
    void firstBaselineOfSmallSingleSpacedText()
    {
        struct Case { const char *family; double ascent, lift; };
        for (const Case &k : {Case{"Franklin Gothic Demi", 1877.0 / 2048, 0.088}, Case{"Gill Sans MT", 1903.0 / 2048, 0.096}}) {
            const QString fam = QString::fromLatin1(k.family);
            struct Probe { double size, spacing, em; };
            for (const Probe &p : {Probe{24, 100, k.ascent - k.lift}, Probe{36, 100, k.ascent}, Probe{24, 125, k.ascent}}) {
                QTextDocument doc;
                QTextCursor c(&doc);
                QTextBlockFormat bf;
                bf.setLineHeight(p.spacing, QTextBlockFormat::ProportionalHeight);
                c.setBlockFormat(bf);
                QTextCharFormat cf;
                cf.setFontFamilies(QStringList{fam});
                cf.setFontPointSize(p.size);
                c.insertText(QStringLiteral("Date"), cf);
                jp::FrameSpec fs;
                fs.size = QSizeF(600, 400);
                fs.insets = QMarginsF(0, 0, 0, 0);
                jp::StoryLayout lay;
                lay.build(&doc, {fs}, jp::LayoutEnv());
                const auto lines = lay.lineInfo(0);
                QCOMPARE(lines.size(), 1);
                QVERIFY2(std::abs(lines[0].baseline - p.em * p.size) < 0.05,
                         qPrintable(QStringLiteral("%1 %2 pt at %3%: %4, not %5").arg(fam).arg(p.size).arg(p.spacing).arg(lines[0].baseline).arg(p.em * p.size)));
            }
        }
    }

    // Publisher draws bold by hand on a font with no bold face: Franklin
    // Gothic Demi and Forte set bold, and Arial Black and Rockwell Extra Bold
    // always (how wide the extra stroke is depends on where the text goes:
    // handBoldOnTheScreenAndInPictures, handBoldInAPdf, handBoldOnPaper).
    // A font with a bold face (Arial) and one whose face is bold already
    // (Script MT Bold) are drawn as they were. The run's layout format only
    // marks the letters (1 when bold is set, 2 whether or not) and is the
    // face the run without bold gets, whatever the size.
    void boldIsDrawnByHandOnFontsWithoutBoldFaces()
    {
        struct Case { const char *family; bool bold; double size; int mark; };
        int checked = 0;
        for (const Case &k : {Case{"Franklin Gothic Demi", true, 24, 1}, Case{"Franklin Gothic Demi", false, 24, 0}, Case{"Forte", true, 100, 1},
                              Case{"Arial Black", false, 60, 2}, Case{"Rockwell Extra Bold", false, 60, 2}, Case{"Rockwell Extra Bold", false, 239, 2},
                              Case{"Script MT Bold", true, 24, 0}, Case{"Arial", true, 24, 0}}) {
            const QString fam = QString::fromLatin1(k.family);
            if (QFontDatabase::hasFamily(fam)) continue;
            QTextCharFormat cf;
            cf.setFontFamilies(QStringList{fam});
            cf.setFontPointSize(k.size);
            cf.setForeground(QColor(QStringLiteral("#336699")));
            if (k.bold) cf.setFontWeight(QFont::Bold);
            const QTextCharFormat r = jp::resolveCharFormat(cf, jp::LayoutEnv());
            const QString what = QStringLiteral("%1 %2 pt%3").arg(fam).arg(k.size).arg(k.bold ? QStringLiteral(" bold") : QString());
            QVERIFY2(r.intProperty(jp::tp::HandBold) == k.mark, qPrintable(what));
            QVERIFY2(!r.hasProperty(QTextFormat::TextOutline), qPrintable(what));
            if (k.mark) {
                // The stand-in is the face the run without bold gets (Qt would embolden a second time).
                QTextCharFormat plain = cf;
                plain.setFontWeight(QFont::Normal);
                QCOMPARE(r.fontWeight(), jp::resolveCharFormat(plain, jp::LayoutEnv()).fontWeight());
            }
            ++checked;
        }
        if (!checked) QSKIP("the fonts are installed here");
    }

    // The letters of a stand-in drawn taller or shorter (run by run) take
    // their stroke too: Franklin Gothic Demi set bold stands the stroke's
    // width higher than the same text unbolded (0.017 em on the screen, 1/35
    // in a PDF, 0.01 on paper).
    void handBoldStrokeShowsOnScaledStandIns()
    {
        const QString fam = QStringLiteral("Franklin Gothic Demi");
        if (QFontDatabase::hasFamily(fam)) QSKIP("Franklin Gothic Demi is installed here");
        auto inkHeight = [&](bool bold, TextOutput out) {
            auto doc = Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<TextItem>();
            t->id = QStringLiteral("t");
            t->rect = QRectF(36, 36, 500, 200);
            t->insets = QMarginsF(0, 0, 0, 0);
            t->storyId = doc->createStory(QStringLiteral("H"));
            {
                QTextCursor c(doc->storyDoc(t->storyId));
                c.select(QTextCursor::Document);
                QTextCharFormat f;
                f.setFontFamilies(QStringList{fam});
                f.setFontPointSize(100);
                if (bold) f.setFontWeight(QFont::Bold);
                c.mergeCharFormat(f);
            }
            doc->pages[0]->items.push_back(t);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            ctx.opt.textOutput = out;
            QImage img(612 * 4, 792 * 4, QImage::Format_ARGB32);
            img.fill(Qt::white);
            {
                QPainter p(&img);
                p.setRenderHint(QPainter::Antialiasing);
                p.scale(4, 4);
                jp::Renderer::paintPage(&p, ctx, 0);
            }
            int top = img.height(), bottom = -1;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qGray(img.pixel(x, y)) < 128) { top = std::min(top, y); bottom = std::max(bottom, y); }
            return bottom > top ? (bottom - top + 1) / 4.0 : 0.0;
        };
        jp::LayoutEnv env;
        QTextCharFormat f;
        f.setFontFamilies(QStringList{fam});
        f.setFontPointSize(100);
        const QTextCharFormat r = jp::resolveCharFormat(f, env);
        if (QFontInfo(r.font()).family() != QLatin1String("Libre Franklin") || !r.hasProperty(jp::tp::GlyphScaleY)) QSKIP("the scaled stand-in isn't drawn here");
        for (const auto &k : {std::make_pair(TextOutput::Picture, 0.017), std::make_pair(TextOutput::Pdf, 1.0 / 35), std::make_pair(TextOutput::Print, 0.01)}) {
            const double plain = inkHeight(false, k.first), bold = inkHeight(true, k.first);
            QVERIFY(plain > 0);
            const double stroke = 100.0 * k.second * r.property(jp::tp::GlyphScaleY).toDouble();
            QVERIFY2(std::abs(bold - plain - stroke) < 0.5, qPrintable(QStringLiteral("plain %1, bold %2, stroke %3").arg(plain).arg(bold).arg(stroke)));
        }
    }

    // Best fit grows text only as far as the box holds it: a banner's short
    // headline grew to fill the width, its one line three times taller than
    // the box (Publisher's banner designs fit theirs inside).
    // Best Fit judges a line by its text, not the spacing under it: sign
    // designs' one-line headlines with 125-130% line spacing shrank to
    // 0.81-0.84 although Publisher draws them at the size the file stores.
    void bestFitIgnoresSpacingUnderLastLine()
    {
        auto doc = Document::blank(QSizeF(792, 612));
        auto t = std::make_shared<TextItem>();
        t->insets = QMarginsF(0, 0, 0, 0);
        t->autofit = TextItem::BestFit;
        t->fitAsStored = true;
        t->storyId = doc->createStory(QStringLiteral("Private"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat big;
            big.setFontPointSize(100);
            c.mergeCharFormat(big);
        }
        doc->pages[0]->items.push_back(t);
        LayoutCache cache;
        RenderOptions opt;
        // The line's own height, at single spacing in a tall box.
        t->rect = QRectF(36, 36, 700, 400);
        const auto single = cache.textFrame(*doc, *t, 1, opt).layout->lineInfo(0);
        QCOMPARE(single.size(), 1);
        const double h = single.first().rect.height();
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextBlockFormat bf;
            bf.setLineHeight(130, QTextBlockFormat::ProportionalHeight);
            c.mergeBlockFormat(bf);
        }
        t->rect = QRectF(36, 36, 700, h * 1.1);   // holds the text, not 130% of it
        const auto fl = cache.textFrame(*doc, *t, 1, opt);
        QCOMPARE(fl.fitScale, 1.0);
    }

    void bestFitStaysInsideBox()
    {
        auto doc = Document::blank(QSizeF(4320, 612));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(528, 155, 3263, 301);
        t->autofit = TextItem::BestFit;
        t->storyId = doc->createStory(QStringLiteral("Register Here"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat big;
            big.setFontPointSize(72);
            c.mergeCharFormat(big);
        }
        doc->pages[0]->items.push_back(t);
        LayoutCache cache;
        RenderOptions opt;
        const auto fl = cache.textFrame(*doc, *t, 1, opt);
        QVERIFY(fl.layout && !fl.layout->overflow());
        const auto lines = fl.layout->lineInfo(0);
        QVERIFY(!lines.isEmpty());
        const double room = t->rect.height();
        for (const auto &li : lines) QVERIFY2(li.rect.bottom() <= room + 0.5, qPrintable(QString::number(li.rect.bottom())));
        // And it still grows: the line fills most of the box's height.
        QVERIFY2(lines.last().rect.bottom() - lines.first().rect.top() > 0.6 * room, qPrintable(QString::number(fl.fitScale)));

        // From a .pub, the text keeps the size the file stores (Publisher
        // fitted it): no growing, and shrinking only when it doesn't fit.
        t->fitAsStored = true;
        {
            TextItem copy;
            copy.fromJson(t->toJson());
            QVERIFY(copy.fitAsStored);
        }
        QCOMPARE(cache.textFrame(*doc, *t, 1, opt).fitScale, 1.0);
        t->rect = QRectF(528, 155, 250, 100);   // two lines of 72 points don't fit
        const double shrunk = cache.textFrame(*doc, *t, 1, opt).fitScale;
        QVERIFY2(shrunk < 1.0, qPrintable(QString::number(shrunk)));
    }
};

JP_TEST_CLASS(PubFontTests)
#include "tst_pub_fonts.moc"
