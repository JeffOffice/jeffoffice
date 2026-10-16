// JeffDeck's tests of text effects, shrunk text, and the lines of slide text.
#include "deckhelpers.h"

class DeckTextEffectsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // The presentation program makes a single line 1.2 em tall and shares it between above and below the baseline in the proportion of the
    // font's Windows ascent to its descent (read from the fonts in its PDFs): the baseline is that share of 1.2 em below the top of the line,
    // whichever font stands in for the original. Set closer, the line loses its top and keeps the descent, but its baseline is no higher than
    // three quarters of the way down it; set wider, its baseline is three quarters of the way down.
    void slideLinesPutTheBaselineWhereTheFontsWindowMetricsSayIt()
    {
        struct Case { QString family; double ascent, descent; };
        const Case cases[] = {{QStringLiteral("Arial"), 1854, 434},          {QStringLiteral("Calibri"), 1950, 550},   {QStringLiteral("Times New Roman"), 1825, 443},
                              {QStringLiteral("Book Antiqua"), 1967, 578},   {QStringLiteral("Gill Sans MT"), 1903, 472}, {QStringLiteral("Century Gothic"), 1989, 451}};
        for (const Case &k : cases)
            for (double spacing : {100.0, 90.0, 150.0}) {
                QTextDocument doc;
                QTextCursor c(&doc);
                QTextBlockFormat bf;
                if (spacing != 100) bf.setLineHeight(spacing, QTextBlockFormat::ProportionalHeight);
                c.setBlockFormat(bf);
                QTextCharFormat cf;
                cf.setFontFamilies({k.family});
                cf.setFontPointSize(40);
                c.insertText(QStringLiteral("Line"), cf);
                FrameSpec fs;
                fs.size = QSizeF(300, 300);
                fs.insets = QMarginsF(0, 0, 0, 0);
                LayoutEnv env;
                env.slideLines = true;
                StoryLayout lay;
                lay.build(&doc, {fs}, env);
                const auto lines = lay.lineInfo(0);
                QCOMPARE(int(lines.size()), 1);
                const double h = 1.2 * 40 * spacing / 100, descent = 1.2 * 40 * k.descent / (k.ascent + k.descent);
                const double expected = spacing > 100 ? 0.75 * h : spacing < 100 ? std::max(0.75 * h, h - descent) : h - descent;
                const QString what = k.family + QLatin1Char(' ') + QString::number(spacing) + QLatin1Char(' ') + QString::number(lines[0].baseline);
                QVERIFY2(std::abs(lines[0].baseline - expected) < 0.01, qPrintable(what + QStringLiteral(" expected ") + QString::number(expected)));
            }
    }

    // A slide's margins and indents stay where the file puts them when its text is shrunk to fit: only the text shrinks. A list's text starts one
    // hang in from its bullet at the same place whatever the fit, and a bullet with no font of its own is followed by its text at once.
    void slideIndentsDoNotShrinkWithTheirText()
    {
        for (double fit : {1.0, 0.5}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLeftMargin(27);
            bf.setTextIndent(-27);
            c.setBlockFormat(bf);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setProperty(tp::BulletChar, QStringLiteral("•"));
            lf.setProperty(tp::BulletFont, QStringLiteral("Arial"));
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Arial")});
            cf.setFontPointSize(20);
            c.createList(lf);
            c.insertText(QStringLiteral("A list item long enough to run over to a second line of this narrow text box"), cf);
            FrameSpec fs;
            fs.size = QSizeF(200, 300);
            fs.insets = QMarginsF(0, 0, 0, 0);
            LayoutEnv env;
            env.slideLines = true;
            env.fontScale = fit;
            StoryLayout lay;
            lay.build(&doc, {fs}, env);
            const auto lines = lay.lineInfo(0);
            QVERIFY(lines.size() >= 2);
            QVERIFY2(std::abs(lines[0].rect.left() - 27) < 0.01, qPrintable(QString::number(fit) + QLatin1Char(' ') + QString::number(lines[0].rect.left())));
            QVERIFY2(std::abs(lines[1].rect.left() - 27) < 0.01, qPrintable(QString::number(fit) + QLatin1Char(' ') + QString::number(lines[1].rect.left())));
            // The bullet is a hang before it, at the left edge.
            QVERIFY2(std::abs(lines[0].markerX) < 0.01, qPrintable(QString::number(fit) + QLatin1Char(' ') + QString::number(lines[0].markerX)));
            QCOMPARE(lines[1].markerX, -1.0);
        }
        // No hang and no font for the bullet: the text follows the bullet's own width (JeffPub's own lists leave a little room, a slide's don't).
        auto firstLeft = [](bool slide) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setProperty(tp::BulletChar, QStringLiteral("•"));
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Arial")});
            cf.setFontPointSize(20);
            c.createList(lf);
            c.insertText(QStringLiteral("Text"), cf);
            FrameSpec fs;
            fs.size = QSizeF(300, 300);
            fs.insets = QMarginsF(0, 0, 0, 0);
            LayoutEnv env;
            env.slideLines = slide;
            StoryLayout lay;
            lay.build(&doc, {fs}, env);
            return lay.lineInfo(0).value(0).rect.left();
        };
        QVERIFY2(std::abs(firstLeft(false) - firstLeft(true) - 4) < 0.01, qPrintable(QString::number(firstLeft(false)) + QLatin1Char(' ') + QString::number(firstLeft(true))));
        QVERIFY(firstLeft(true) > 5);
    }

private:
    // One bold bar (the letter I, 60 points) with a run's effect list, painted at 4 pixels to the point: the picture and where the bar
    // is on the middle row. `fx` is the run's effects (none when null).
    struct EffectBar {
        QImage img;
        int left = 0, right = 0, row = 0;   // the bar's edges on its middle row, pixels
        double top = 0, bottom = 0;         // the line's, in points
    };
    static EffectBar paintEffectBar(const TextEffects *fx, double fontScale = 1.0, QSizeF size = QSizeF(160, 150))
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("DejaVu Sans")});
        cf.setFontWeight(QFont::Bold);
        cf.setFontPointSize(60);
        cf.setForeground(Qt::black);
        if (fx) cf.setProperty(tp::TextFx, fx->toJson());
        c.insertText(QStringLiteral("I"), cf);
        FrameSpec fs;
        fs.size = size;
        fs.insets = QMarginsF(60, 20, 0, 0);
        LayoutEnv env;
        env.fontScale = fontScale;
        StoryLayout lay;
        lay.build(&doc, {fs}, env);
        EffectBar b;
        const auto lines = lay.lineInfo(0);
        b.top = lines[0].rect.top();
        b.bottom = lines[0].rect.bottom();
        b.img = QImage((size * 4).toSize(), QImage::Format_ARGB32_Premultiplied);
        b.img.fill(Qt::white);
        QPainter p(&b.img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.scale(4, 4);
        lay.paint(&p, 0, PaintOptions());
        p.end();
        // The bar's middle row: a little above the baseline, where the letter is a plain stem.
        b.row = int((lines[0].baseline - 12 * fontScale) * 4);
        b.left = b.right = -1;
        for (int x = 0; x < b.img.width(); ++x)
            if (qGray(b.img.pixel(x, b.row)) < 40) {
                if (b.left < 0) b.left = x;
                b.right = x;
            }
        return b;
    }

    // A frame of black words ("word word ...", 10 points unless said) with one effect list on every run, laid out; painted at `scale` pixels to the
    // point, and again when asked (the layout stays).
    struct EffectSample {
        std::unique_ptr<QTextDocument> doc = std::make_unique<QTextDocument>();
        std::unique_ptr<StoryLayout> layout = std::make_unique<StoryLayout>();
        QSizeF size;
        void build(const TextEffects &fx, int words, QSizeF frame, double pointSize = 10)
        {
            size = frame;
            QTextCursor c(doc.get());
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("DejaVu Sans")});
            cf.setFontPointSize(pointSize);
            cf.setForeground(Qt::black);
            cf.setProperty(tp::TextFx, fx.toJson());
            for (int i = 0; i < words; ++i) c.insertText(QStringLiteral("word "), cf);
            FrameSpec fs;
            fs.size = frame;
            fs.insets = QMarginsF(20, 20, 20, 20);
            layout->build(doc.get(), {fs}, LayoutEnv());
        }
        QImage paint(double scale) const
        {
            QImage img((size * scale).toSize(), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::white);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::TextAntialiasing);
            p.scale(scale, scale);
            layout->paint(&p, 0, PaintOptions());
            p.end();
            return img;
        }
    };
    static int countColored(const QImage &img, const std::function<bool(const QColor &)> &test)
    {
        int n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) n += test(img.pixelColor(x, y));
        return n;
    }
    static QString runListOf(const QTextDocument *story, const QString &text)
    {
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().text().contains(text)) return it.fragment().charFormat().stringProperty(tp::TextFx);
        return QStringLiteral("(no such run)");
    }

private Q_SLOTS:
    // A shadow lands where the effect list says: this far from the letters (in points) in this direction (a:outerShdw's dist and dir).
    void runShadowLandsWhereTheListSays()
    {
        const EffectBar plain = paintEffectBar(nullptr);
        QVERIFY(plain.left > 0 && plain.right > plain.left);
        const QColor red(255, 0, 0);
        auto redEdges = [](const EffectBar &b, int row, int *left, int *right) {
            *left = *right = -1;
            for (int x = 0; x < b.img.width(); ++x) {
                const QColor c = b.img.pixelColor(x, row);
                if (c.red() > 200 && c.green() < 60 && c.blue() < 60) {
                    if (*left < 0) *left = x;
                    *right = x;
                }
            }
        };
        // To the right, 10 points: 40 pixels (the program's pictures put it a little nearer than the file says).
        const double nearer = kShadowNear;
        const TextEffects right = shadowOf(red, 0, 10, 0);
        EffectBar b = paintEffectBar(&right);
        QCOMPARE(b.left, plain.left);
        int l, r;
        redEdges(b, b.row, &l, &r);
        QVERIFY2(l > 0 && std::abs((r - b.right) - 40 * nearer) <= 5, qPrintable(QStringLiteral("%1 %2 %3").arg(l).arg(r).arg(b.right)));
        // Down and to the right, 14.142 points: 10 points each way.
        const TextEffects diagonal = shadowOf(red, 0, 14.1421, 45);
        b = paintEffectBar(&diagonal);
        redEdges(b, b.row, &l, &r);
        QVERIFY2(std::abs((r - b.right) - 40 * nearer) <= 5, qPrintable(QString::number(r - b.right)));
        int topPlain = -1, topShadow = -1;
        for (int y = 0; y < plain.img.height(); ++y) {
            if (topPlain < 0 && qGray(plain.img.pixel(plain.left + 4, y)) < 40) topPlain = y;
        }
        // The shadow's top edge, down the column past the bar's right edge, is 10 points (40 pixels) under the bar's.
        for (int y = 0; y < b.img.height(); ++y) {
            const QColor c = b.img.pixelColor(b.right + 20, y);
            if (topShadow < 0 && c.red() > 200 && c.green() < 60) topShadow = y;
        }
        QVERIFY2(topShadow > 0 && std::abs((topShadow - topPlain) - 40 * nearer) <= 6, qPrintable(QStringLiteral("%1 %2").arg(topShadow).arg(topPlain)));
        // Text the program shrank to fit has its shadow shrunk with it.
        const EffectBar half = paintEffectBar(&right, 0.5);
        redEdges(half, half.row, &l, &r);
        QVERIFY2(l > 0 && std::abs((r - half.right) - 20 * nearer) <= 5, qPrintable(QString::number(r - half.right)));
        // A shadow with no color of its own alpha is as light as the alpha says.
        const TextEffects faint = shadowOf(QColor(255, 0, 0, 102), 0, 10, 0);
        b = paintEffectBar(&faint);
        const QColor sample = b.img.pixelColor(b.right + 20, b.row);
        QVERIFY2(std::abs(sample.green() - 153) <= 16 && sample.red() > 230, qPrintable(sample.name()));
    }

    // A shadow's blur is the width of a box blurring its edge, which is a Gaussian a little under a third as wide (as the presentation
    // program draws it, a little softer still): the edge is half there where it would be, and nearly gone two spreads away.
    void runShadowBlurIsAGaussianOfAThirdTheRadius()
    {
        // 40 points to the right is clear of the bar; 4 points of blur is a spread of 1.155 points (a little more: 1.27, 5.1 pixels).
        const TextEffects hard = shadowOf(QColor(255, 0, 0), 0, 40, 0), soft = shadowOf(QColor(255, 0, 0), 4, 40, 0);
        const EffectBar a = paintEffectBar(&hard), b = paintEffectBar(&soft);
        int edge = -1;
        for (int x = b.right + 20; x < a.img.width() && edge < 0; ++x)
            if (a.img.pixelColor(x, a.row).green() < 128) edge = x;
        QVERIFY(edge > 0);
        auto coverage = [&](int x) { return 1 - b.img.pixelColor(x, b.row).green() / 255.0; };
        QVERIFY2(std::abs(coverage(edge) - 0.5) < 0.2, qPrintable(QString::number(coverage(edge))));
        QVERIFY2(coverage(edge - 9) < 0.2 && coverage(edge - 9) > 0.0, qPrintable(QString::number(coverage(edge - 9))));
        QVERIFY2(coverage(edge + 10) > 0.8, qPrintable(QString::number(coverage(edge + 10))));
        // Not at all where the hard one has nothing and the blur doesn't reach.
        QVERIFY(b.img.pixelColor(edge - 24, b.row).green() >= 250);
    }

    // The same spread comes out of blurAlpha: an edge blurred by a spread of 3 rises from a sixth to five sixths of its height over twice 3 pixels.
    void blurAlphaSpreadsLikeAGaussian()
    {
        QImage img(240, 20, QImage::Format_Alpha8);
        img.fill(0);
        for (int y = 0; y < 20; ++y)
            for (int x = 0; x < 120; ++x) img.scanLine(y)[x] = 255;
        blurAlpha(img, 3.0);
        const uchar *row = img.constScanLine(10);
        auto crossing = [&](double level) {
            for (int x = 100; x < 140; ++x)
                if (row[x] <= level * 255 && row[x - 1] > level * 255) return x - 1 + (row[x - 1] - level * 255) / double(row[x - 1] - row[x]);
            return -1.0;
        };
        const double high = crossing(0.8413), low = crossing(0.1587);
        QVERIFY(high > 0 && low > 0);
        QVERIFY2(std::abs((low - high) / 2 - 3.0) < 0.35, qPrintable(QString::number((low - high) / 2)));
        // Half way is where the edge was, and the far side is untouched.
        QVERIFY2(std::abs(crossing(0.5) - 119.5) < 0.7, qPrintable(QString::number(crossing(0.5))));
        QCOMPARE(int(row[40]), 255);
        QCOMPARE(int(row[239]), 0);
        // (the edge of the picture counts as clear, so a border of the shape blurs toward nothing)
        QVERIFY(row[0] < 255);
        // Nothing changes for a spread of nothing.
        QImage same = img;
        blurAlpha(same, 0.0);
        QCOMPARE(same, img);
    }

    // A glow is a halo around the letters, as far as its radius reaches.
    void runGlowSurroundsTheLetters()
    {
        TextEffects fx;
        fx.glow.on = true;
        fx.glow.color = QColor(0, 0, 255, 200);
        fx.glow.radius = 12;
        const EffectBar plain = paintEffectBar(nullptr), glowing = paintEffectBar(&fx);
        QCOMPARE(glowing.left, plain.left);
        // Five points (20 pixels) left of the bar is tinted; thirty are not.
        QVERIFY2(glowing.img.pixelColor(glowing.left - 20, glowing.row).red() < 200, qPrintable(glowing.img.pixelColor(glowing.left - 20, glowing.row).name()));
        QVERIFY(glowing.img.pixelColor(glowing.left - 120, glowing.row).red() >= 250);
        QVERIFY(plain.img.pixelColor(plain.left - 20, plain.row).red() >= 250);
    }

    // A reflection is the letters turned over under their line, fading out.
    void runReflectionFallsBelowTheLine()
    {
        TextEffects fx;
        fx.reflection.on = true;
        fx.reflection.startAlpha = 0.5;
        fx.reflection.endAlpha = 0;
        fx.reflection.endPos = 0.5;
        fx.reflection.sy = -1;
        fx.reflection.angle = 90;
        const EffectBar plain = paintEffectBar(nullptr, 1.0, QSizeF(160, 220)), mirrored = paintEffectBar(&fx, 1.0, QSizeF(160, 220));
        const int x = plain.left + (plain.right - plain.left) / 2;
        int darkest = 255, below = -1;
        for (int y = int(mirrored.bottom * 4) + 1; y < mirrored.img.height(); ++y) {
            QVERIFY(plain.img.pixelColor(x, y).red() >= 250);
            const int v = mirrored.img.pixelColor(x, y).red();
            if (v < 250 && below < 0) below = y;
            darkest = std::min(darkest, v);
        }
        QVERIFY2(below > 0, "no reflection");
        // Half as dark as the letters at the most, and no darker than the fade starts.
        QVERIFY2(darkest > 100 && darkest < 200, qPrintable(QString::number(darkest)));
    }

    // An inner shadow darkens the inside of the letters from the side it comes from, and nothing outside them.
    void runInnerShadowStaysInsideTheLetters()
    {
        TextEffects fx;
        fx.inner.on = true;
        fx.inner.color = QColor(255, 0, 0);
        fx.inner.blur = 0;
        fx.inner.distance = 3;
        fx.inner.angle = 0;
        const EffectBar plain = paintEffectBar(nullptr), shaded = paintEffectBar(&fx);
        // (the bar's edges, whatever color it is inside: not white)
        int left = -1, right = -1;
        for (int x = 0; x < shaded.img.width(); ++x)
            if (shaded.img.pixelColor(x, shaded.row).green() < 128) {
                if (left < 0) left = x;
                right = x;
            }
        QVERIFY2(std::abs(left - plain.left) <= 1 && std::abs(right - plain.right) <= 1, qPrintable(QStringLiteral("%1 %2 / %3 %4").arg(left).arg(right).arg(plain.left).arg(plain.right)));
        // At the left edge, inside, red; further in, the letter's own black; outside, white.
        const QColor edge = shaded.img.pixelColor(plain.left + 3, shaded.row), deep = shaded.img.pixelColor((plain.left + plain.right) / 2, shaded.row);
        QVERIFY2(edge.red() > 150 && edge.green() < 60, qPrintable(edge.name()));
        QVERIFY2(deep.red() < 40, qPrintable(deep.name()));
        QVERIFY(shaded.img.pixelColor(plain.left - 6, shaded.row).red() >= 250);
        QVERIFY(shaded.img.pixelColor(plain.right + 6, shaded.row).red() >= 250);
    }

    // A run's effect list is read from the file: a shadow, a glow, a reflection with an inner shadow, and a list in a text's list style
    // that a run's own empty list turns off. Written back, each list is the one the file had.
    void runEffectListsAreReadAndWrittenBack()
    {
        const QString shadow = QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"38100\" dist=\"38100\" dir=\"2700000\" algn=\"tl\"><a:srgbClr val=\"000000\"/></a:outerShdw></a:effectLst>");
        const QString glow = QStringLiteral("<a:effectLst><a:glow rad=\"101600\"><a:schemeClr val=\"accent1\"><a:satMod val=\"175000\"/><a:alpha val=\"40000\"/></a:schemeClr></a:glow></a:effectLst>");
        const QString both = QStringLiteral("<a:effectLst><a:innerShdw blurRad=\"63500\" dist=\"50800\" dir=\"13500000\"><a:prstClr val=\"black\"><a:alpha val=\"50000\"/></a:prstClr></a:innerShdw>"
                                            "<a:reflection blurRad=\"6350\" stA=\"50000\" endA=\"300\" endPos=\"35000\" dist=\"12700\" dir=\"5400000\" sy=\"-100000\" algn=\"bl\" rotWithShape=\"0\"/></a:effectLst>");
        auto run = [](const QString &text, const QString &list) {
            return QStringLiteral("<a:r><a:rPr lang=\"en-US\"><a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill>%1</a:rPr><a:t>%2</a:t></a:r>").arg(list, text);
        };
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString body = QStringLiteral("<a:p>") + run(QStringLiteral("Shadowed"), shadow) + run(QStringLiteral("Glowing"), glow) + run(QStringLiteral("Mirrored"), both)
                           + run(QStringLiteral("Plain"), QString()) + QStringLiteral("</a:p><a:p>") + run(QStringLiteral("Own"), QStringLiteral("<a:effectLst/>"))
                           + QStringLiteral("<a:r><a:rPr lang=\"en-US\"/><a:t>Inherited</a:t></a:r></a:p>");
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Effects\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 4572000, 914400) + QStringLiteral("</p:spPr><p:txBody><a:bodyPr/><a:lstStyle><a:lvl1pPr>")
                                                       + QStringLiteral("<a:defRPr>") + shadow + QStringLiteral("</a:defRPr></a:lvl1pPr></a:lstStyle>") + body + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *t = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Effects")) t = static_cast<const TextItem *>(it.get());
        QVERIFY(t);
        QMap<QString, QString> lists;   // run text -> its effects
        const QTextDocument *story = d->storyDoc(t->storyId);
        QVERIFY(story);
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) lists[it.fragment().text()] = it.fragment().charFormat().stringProperty(tp::TextFx);
        // The list style's shadow reaches a run that says nothing; a run's own empty list says none; the others say their own.
        QVERIFY(!lists["Inherited"].isEmpty());
        const TextEffects inherited = TextEffects::fromJson(lists["Inherited"]);
        QVERIFY(inherited.outer.on);
        QCOMPARE(lists["Own"], TextEffects::noneJson());   // (a list that says there are none, not no list)
        const TextEffects s = TextEffects::fromJson(lists["Shadowed"]);
        QVERIFY(s.outer.on && !s.inner.on && !s.glow.on && !s.reflection.on);
        QCOMPARE(s.outer.blur, 3.0);
        QCOMPARE(s.outer.distance, 3.0);
        QCOMPARE(s.outer.angle, 45.0);
        QCOMPARE(s.outer.color, QColor(0, 0, 0));
        QCOMPARE(s.outer.align, QStringLiteral("tl"));
        const TextEffects g = TextEffects::fromJson(lists["Glowing"]);
        QVERIFY(g.glow.on && !g.outer.on);
        QCOMPARE(g.glow.radius, 8.0);
        QCOMPARE(g.glow.color.alpha(), 102);   // 40% of the color
        const TextEffects m = TextEffects::fromJson(lists["Mirrored"]);
        QVERIFY(m.inner.on && m.reflection.on && !m.outer.on);
        QCOMPARE(m.inner.angle, 225.0);
        QCOMPARE(m.inner.distance, 4.0);
        QCOMPARE(m.inner.color.alpha(), 128);
        QCOMPARE(m.reflection.startAlpha, 0.5);
        QCOMPARE(m.reflection.endPos, 0.35);
        QCOMPARE(m.reflection.sy, -1.0);
        QCOMPARE(m.reflection.angle, 90.0);
        QCOMPARE(m.reflection.align, QStringLiteral("bl"));
        QVERIFY(TextEffects::fromJson(lists["Plain"]).outer.on);   // (the list style's, as the one that says nothing has it)
        // Written back as the file had them.
        Exported e;
        QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        // (each element with its attributes in order of name, as a document parsed again holds them)
        auto canonical = [](const QString &xml) {
            QDomDocument dom;
            if (!dom.setContent(QStringLiteral("<root xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">") + xml + QStringLiteral("</root>"))) return QStringLiteral("not XML: ") + xml;
            std::function<QString(const QDomElement &)> walk = [&](const QDomElement &el) {
                QStringList attrs;
                const QDomNamedNodeMap named = el.attributes();
                for (int i = 0; i < named.length(); ++i) attrs << named.item(i).nodeName() + QLatin1Char('=') + named.item(i).nodeValue();
                attrs.sort();
                QString out = QLatin1Char('<') + el.nodeName() + QLatin1Char(' ') + attrs.join(QLatin1Char(' ')) + QLatin1Char('>');
                for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) out += walk(c);
                return out + QStringLiteral("</>");
            };
            return walk(dom.documentElement().firstChildElement());
        };
        for (const QString &list : {shadow, glow, both}) {
            bool found = false;
            for (int at = slide.indexOf(QStringLiteral("<a:effectLst>")); at >= 0 && !found; at = slide.indexOf(QStringLiteral("<a:effectLst>"), at + 1)) {
                const QString written = slide.mid(at, slide.indexOf(QStringLiteral("</a:effectLst>"), at) + 14 - at);
                found = canonical(written) == canonical(list);
            }
            QVERIFY2(found, qPrintable(list));
        }
        // After the run's fill, before anything that follows it in the schema (a:latin).
        QVERIFY(slide.contains(QStringLiteral("<a:srgbClr val=\"336699\"/></a:solidFill><a:effectLst>")));
        // Read again, the same effects.
        auto again = deck::importPresentation(bytes, &err);
        QVERIFY2(again, qPrintable(err));
        const TextItem *t2 = nullptr;
        for (const auto &it : again->pages[0]->items)
            if (it->name == QLatin1String("Effects")) t2 = static_cast<const TextItem *>(it.get());
        QVERIFY(t2);
        const QTextDocument *story2 = again->storyDoc(t2->storyId);
        QMap<QString, QString> lists2;
        for (QTextBlock b = story2->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) lists2[it.fragment().text()] = it.fragment().charFormat().stringProperty(tp::TextFx);
        QCOMPARE(lists2["Shadowed"], lists["Shadowed"]);
        QCOMPARE(lists2["Glowing"], lists["Glowing"]);
        QCOMPARE(lists2["Mirrored"], lists["Mirrored"]);
    }

    // A file can give every run an effect list of its own: only so many different lists are drawn on a frame, a list of thousands of
    // elements is read in a few and kept as only its first of each kind.
    void effectListsFromAStrangersFileAreHeldToLimits()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        for (int i = 0; i < 400; ++i) {
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("DejaVu Sans")});
            cf.setFontPointSize(10);
            cf.setForeground(Qt::black);
            cf.setProperty(tp::TextFx, shadowOf(QColor(255, 0, 0), 1 + i * 0.01, 2 + (i % 7) * 0.1, i % 360).toJson());
            c.insertText(QStringLiteral("word "), cf);
        }
        FrameSpec fs;
        fs.size = QSizeF(500, 400);
        StoryLayout lay;
        lay.build(&doc, {fs}, LayoutEnv());
        QImage img(2000, 1600, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        p.scale(4, 4);
        QElapsedTimer clock;
        clock.start();
        lay.paint(&p, 0, PaintOptions());
        p.end();
        QVERIFY2(clock.elapsed() < 20000, qPrintable(QString::number(clock.elapsed())));
        // The first lists have their shadows (red under the words), the rest are drawn plain.
        int red = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) red += img.pixelColor(x, y).red() > 200 && img.pixelColor(x, y).green() < 120;
        QVERIFY(red > 0);
        // A list with thousands of elements.
        QString many = QStringLiteral("<a:effectLst>");
        for (int i = 0; i < 5000; ++i) many += QStringLiteral("<a:outerShdw blurRad=\"%1\" dist=\"38100\" dir=\"2700000\"><a:srgbClr val=\"000000\"/></a:outerShdw>").arg(i);
        many += QStringLiteral("</a:effectLst>");
        Parts parts = samplePptx();
        QString xml = QString::fromUtf8(parts.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Many\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 4572000, 914400) + QStringLiteral("</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\">") + many
                                                       + QStringLiteral("</a:rPr><a:t>Words</a:t></a:r></a:p></p:txBody></p:sp></p:spTree>"));
        parts.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        clock.restart();
        auto d = deck::importPresentation(pack(parts), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(clock.elapsed() < 20000, qPrintable(QString::number(clock.elapsed())));
        const TextItem *t = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Many")) t = static_cast<const TextItem *>(it.get());
        QVERIFY(t);
        const QTextDocument *story = d->storyDoc(t->storyId);
        const QString list = story->begin().begin().fragment().charFormat().stringProperty(tp::TextFx);
        QVERIFY(!list.isEmpty());
        const TextEffects fx = TextEffects::fromJson(list);
        QVERIFY(fx.outer.on);
        QCOMPARE(fx.outer.blur, 0.0);   // the first of them
        QVERIFY(fx.xml.size() < 1000);  // (only the first of each kind is kept)
        QVERIFY(list.size() < 3000);
    }

    // The Shadow button gives words the shadow the presentation program gives them (a soft one, 3 points down and to the right) and takes
    // it off again; it takes off the shadow a file gave them too. A slide's words with it on are drawn with that shadow.
    void shadowButtonPutsOnAndTakesOffTheProgramsShadow()
    {
        DeckWindow w;
        editTitle(w, QStringLiteral("Hello"), 0, 5);
        QAction *button = w.act(QStringLiteral("fmt.shadow"));
        QVERIFY(button);
        QTRY_VERIFY(button->isEnabled());
        QVERIFY(!button->isChecked());
        button->trigger();
        // (the shadow is in the run's effect list, where it is saved, and not in the older property)
        QVERIFY(Editor::hasTextShadow(w.editor()->currentCharFormat()));
        QVERIFY(!w.editor()->currentCharFormat().boolProperty(tp::Shadow));
        QTRY_VERIFY(button->isChecked());
        button->trigger();
        QVERIFY(!Editor::hasTextShadow(w.editor()->currentCharFormat()));
        // A shadow from a file's effect list: the button shows it, and a click takes it off (the glow it came with stays).
        TextEffects fx = shadowOf(QColor(0, 0, 0), 3, 3, 45);
        fx.glow.on = true;
        fx.glow.color = QColor(0, 0, 255);
        fx.glow.radius = 5;
        w.editor()->setCharProperty(tp::TextFx, fx.toJson(), QStringLiteral("Effects"));
        QTRY_VERIFY(button->isChecked());
        button->trigger();
        const TextEffects left = TextEffects::fromJson(w.editor()->currentCharFormat().stringProperty(tp::TextFx));
        QVERIFY(!left.outer.on && left.glow.on);
        QTRY_VERIFY(!button->isChecked());
        // On a slide, the button's shadow is drawn as the program draws it: the same as the effect list it writes.
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("DejaVu Sans")});
        cf.setFontWeight(QFont::Bold);
        cf.setFontPointSize(60);
        cf.setForeground(Qt::black);
        cf.setProperty(tp::Shadow, true);
        QTextDocument doc;
        QTextCursor(&doc).insertText(QStringLiteral("I"), cf);
        auto paint = [&](bool slideLines) {
            FrameSpec fs;
            fs.size = QSizeF(160, 150);
            fs.insets = QMarginsF(60, 20, 0, 0);
            LayoutEnv env;
            env.slideLines = slideLines;
            StoryLayout lay;
            lay.build(&doc, {fs}, env);
            QImage img(QSize(640, 600), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::white);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::TextAntialiasing);
            p.scale(4, 4);
            lay.paint(&p, 0, PaintOptions());
            p.end();
            return img;
        };
        const QImage slide = paint(true), publication = paint(false);
        EffectBar plain = paintEffectBar(nullptr);
        const TextEffects shadow = defaultTextShadow();
        const EffectBar listed = paintEffectBar(&shadow);
        // The slide's matches the effect list's; the publication's keeps its own (nearer, 1.2 points).
        int differs = 0, same = 0;
        for (int y = 0; y < 600; ++y)
            for (int x = 0; x < 640; ++x) {
                if (slide.pixel(x, y) != publication.pixel(x, y)) ++differs;
                if (y < listed.img.height() && x < listed.img.width() && qAbs(qGray(slide.pixel(x, y)) - qGray(listed.img.pixel(x, y))) <= 3) ++same;
            }
        QVERIFY(differs > 30);
        QVERIFY2(same > 640 * 560, qPrintable(QString::number(same)));
        QVERIFY(plain.left > 0);
    }

    // A shadow in a master's title style reaches the titles that follow it, and is saved in the master's title style again; a title the
    // file has no list for takes it.
    void titleStyleEffectsReachTitlesAndAreSavedInTheMaster()
    {
        const QString shadow = QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"114300\" dist=\"101600\" dir=\"2700000\" algn=\"tl\" rotWithShape=\"0\"><a:srgbClr val=\"000000\"><a:alpha val=\"40000\"/></a:srgbClr></a:outerShdw></a:effectLst>");
        Parts p = samplePptx();
        QString master = QString::fromUtf8(p.data["/ppt/slideMasters/slideMaster1.xml"].xml);
        const QString before = QStringLiteral("<a:defRPr sz=\"4400\"><a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>");
        QVERIFY(master.contains(before));
        master.replace(before, QStringLiteral("<a:defRPr sz=\"4400\">") + shadow + QStringLiteral("<a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>"));
        p.data["/ppt/slideMasters/slideMaster1.xml"].xml = master.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        // The first slide's title ("Quarterly results") has the master's shadow.
        const QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:title"));
        QVERIFY(story);
        const QString list = story->begin().begin().fragment().charFormat().stringProperty(tp::TextFx);
        QVERIFY(!list.isEmpty());
        const TextEffects fx = TextEffects::fromJson(list);
        QVERIFY(fx.outer.on);
        QCOMPARE(fx.outer.blur, 9.0);
        QCOMPARE(fx.outer.distance, 8.0);
        QCOMPARE(fx.outer.angle, 45.0);
        QCOMPARE(fx.outer.color.alpha(), 102);
        // The master's title style says it again when the file is saved.
        Exported e;
        QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString saved = part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"));
        const int style = saved.indexOf(QStringLiteral("<p:titleStyle>"));
        QVERIFY(style >= 0);
        const QString titleStyle = saved.mid(style, saved.indexOf(QStringLiteral("</p:titleStyle>")) - style);
        QVERIFY2(titleStyle.contains(QStringLiteral("<a:outerShdw")), qPrintable(titleStyle));
        QVERIFY(titleStyle.indexOf(QStringLiteral("<a:effectLst>")) < titleStyle.indexOf(QStringLiteral("<a:latin")));
    }

    // The budget for a page's effects covers the whole painting: layouts painted in one scope share it, so what a later one needs past it is
    // drawn without the effect (the words are drawn all the same); and a picture too big for it takes nothing else with it: a shadow that needs
    // no picture is still there when a reflection can't be afforded.
    void effectPicturesShareOneBudgetForAPainting()
    {
        const TextEffects blurred = shadowOf(QColor(255, 0, 0), 6, 4, 45);
        EffectSample first, second, third;
        first.build(blurred, 60, QSizeF(300, 100));
        second.build(blurred, 60, QSizeF(300, 100));
        third.build(blurred, 60, QSizeF(300, 100));
        const auto red = [](const QColor &c) { return c.red() > 200 && c.green() < 200 && c.blue() < 200; };
        qint64 used = 0;
        {
            EffectBudget scope(nullptr, 1000000000);
            QVERIFY(countColored(first.paint(2), red) > 20);
            used = 1000000000 - scope.left();
        }
        QVERIFY(used > 1000);
        // Room for a picture and a half: the first layout painted in the scope has its shadow, the next does not (and the words are there).
        {
            EffectBudget scope(nullptr, used * 3 / 2);
            const QImage a = second.paint(2), b = third.paint(2);
            QVERIFY(countColored(a, red) > 20);
            QCOMPARE(countColored(b, red), 0);
            QVERIFY(countColored(b, [](const QColor &c) { return c.red() < 60 && c.green() < 60; }) > 100);
        }
        // Nothing at all: a hard shadow is vector, so a reflection that can't be had leaves it as it is.
        TextEffects hard = shadowOf(QColor(255, 0, 0), 0, 4, 45), both = hard;
        both.reflection.on = true;
        both.reflection.sy = -1;
        both.reflection.startAlpha = 0.5;
        EffectSample plain, mirrored;
        plain.build(hard, 20, QSizeF(300, 100));
        mirrored.build(both, 20, QSizeF(300, 100));
        EffectBudget none(nullptr, 0);
        const QImage a = plain.paint(2), b = mirrored.paint(2);
        QVERIFY(countColored(a, red) > 20);
        QCOMPARE(a, b);
    }

    // A file's effect numbers are held to 200 points, and a frame of effects at that size, many of them, paints in a few seconds: a glow on
    // thousands of letters costs no more than one on a few (it is made of the picture of the letters), and what the painting's pixels can't
    // pay for is left out.
    void effectsAtTheCapsPaintWithinTheBudget()
    {
        QCOMPARE(TextEffects::fromJson(QStringLiteral("{\"o\":{\"b\":1584,\"d\":1584},\"g\":{\"r\":1584},\"s\":1584}")).outer.blur, 200.0);
        QCOMPARE(TextEffects::fromJson(QStringLiteral("{\"o\":{\"b\":1584,\"d\":1584},\"g\":{\"r\":1584}}")).glow.radius, 200.0);
        // Read from a file: 1,584 points of blur, distance, and glow.
        Parts parts = samplePptx();
        QString xml = QString::fromUtf8(parts.data["/ppt/slides/slide1.xml"].xml);
        const QString list = QStringLiteral("<a:effectLst><a:glow rad=\"20116800\"><a:srgbClr val=\"0000FF\"/></a:glow><a:outerShdw blurRad=\"20116800\" dist=\"20116800\" dir=\"2700000\"><a:srgbClr val=\"FF0000\"/></a:outerShdw>"
                                             "<a:reflection blurRad=\"20116800\" dist=\"20116800\"/></a:effectLst>");
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Big\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(0, 0, 4572000, 914400)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\">") + list + QStringLiteral("</a:rPr><a:t>Words</a:t></a:r></a:p></p:txBody></p:sp></p:spTree>"));
        parts.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto read = deck::importPresentation(pack(parts), &err);
        QVERIFY2(read, qPrintable(err));
        const TextItem *big = nullptr;
        for (const auto &it : read->pages[0]->items)
            if (it->name == QLatin1String("Big")) big = static_cast<const TextItem *>(it.get());
        QVERIFY(big);
        const TextEffects got = TextEffects::fromJson(runListOf(read->storyDoc(big->storyId), QStringLiteral("Words")));
        QCOMPARE(got.outer.blur, 200.0);
        QCOMPARE(got.outer.distance, 200.0);
        QCOMPARE(got.glow.radius, 200.0);
        QCOMPARE(got.reflection.blur, 200.0);
        // Twenty text boxes the size of a slide, each with its own list of every effect at the caps, on a slide.
        auto d = deck::newPresentation();
        for (int i = 0; i < 20; ++i) {
            auto box = std::make_shared<TextItem>();
            box->rect = QRectF(10 + i * 4, 10 + i * 4, 800, 440);
            box->storyId = d->createStory();
            QTextCursor c(d->storyDoc(box->storyId));
            TextEffects fx = shadowOf(QColor(255, 0, 0, 255 - i * 3), 1584, 1584, 45);
            fx.glow.on = true;
            fx.glow.color = QColor(0, 0, 255, 200 - i);
            fx.glow.radius = 1584;
            fx.inner = fx.outer;
            fx.reflection.on = true;
            fx.reflection.blur = 1584;
            fx.reflection.sy = -1;
            QTextCharFormat cf;
            cf.setFontPointSize(10);
            cf.setProperty(tp::TextFx, TextEffects::fromJson(fx.toJson()).toJson());
            for (int w = 0; w < 300; ++w) c.insertText(QStringLiteral("word "), cf);
            d->pages[0]->items.push_back(box);
        }
        LayoutCache cache;
        const int before = effectPicturesBuiltForTest();
        QElapsedTimer clock;
        clock.start();
        const QImage slide = paintedSlide(d.get(), &cache, 0);
        QVERIFY2(clock.elapsed() < 10000, qPrintable(QString::number(clock.elapsed())));
        // Some pictures were made, not the 100 (20 boxes, five effects) the lists ask for.
        QVERIFY(effectPicturesBuiltForTest() - before > 0);
        QVERIFY2(effectPicturesBuiltForTest() - before < 100, qPrintable(QString::number(effectPicturesBuiltForTest() - before)));
        QVERIFY(!slide.isNull());
        // Three thousand letters with a glow of 200 points.
        EffectSample many;
        TextEffects glow;
        glow.glow.on = true;
        glow.glow.color = QColor(0, 0, 255, 200);
        glow.glow.radius = 1584;
        many.build(glow, 660, QSizeF(500, 400), 6);
        clock.restart();
        const QImage glowing = many.paint(2);
        QVERIFY2(clock.elapsed() < 10000, qPrintable(QString::number(clock.elapsed())));
        QVERIFY(countColored(glowing, [](const QColor &c) { return c.blue() > 200 && c.red() < 200; }) > 100);
    }

    // A picture of an effect is kept for the layout it was made of, at the size it was painted: painting again (a caret that blinks, a window
    // that scrolls, the show's next animation step) makes none; another size, or a layout built again, makes new ones.
    void effectPicturesAreKeptForTheNextPaint()
    {
        TextEffects fx = shadowOf(QColor(255, 0, 0), 6, 4, 45);
        fx.glow.on = true;
        fx.glow.color = QColor(0, 0, 255, 160);
        fx.glow.radius = 8;
        fx.inner.on = true;
        fx.inner.blur = 2;
        fx.inner.distance = 2;
        fx.reflection.on = true;
        fx.reflection.sy = -1;
        EffectSample sample;
        sample.build(fx, 30, QSizeF(300, 150));
        const int start = effectPicturesBuiltForTest();
        const QImage first = sample.paint(2);
        const int made = effectPicturesBuiltForTest() - start;
        QVERIFY2(made >= 4, qPrintable(QString::number(made)));
        const QImage again = sample.paint(2);
        QCOMPARE(effectPicturesBuiltForTest() - start, made);
        QCOMPARE(again, first);
        sample.paint(3);
        QVERIFY(effectPicturesBuiltForTest() - start > made);
        const int now = effectPicturesBuiltForTest();
        EffectSample rebuilt;
        rebuilt.build(fx, 30, QSizeF(300, 150));
        rebuilt.paint(2);
        QVERIFY(effectPicturesBuiltForTest() > now);
    }

    // A file's effect list that says there are none (an empty a:effectLst) is a list of its own: a run that has it takes off what its style
    // gives, which holds when the file is saved and read again; a run that says nothing has the style's.
    void aRunsEmptyEffectListSurvivesSaving()
    {
        const QString shadow = QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"114300\" dist=\"101600\" dir=\"2700000\"><a:srgbClr val=\"000000\"/></a:outerShdw></a:effectLst>");
        Parts p = samplePptx();
        QString master = QString::fromUtf8(p.data["/ppt/slideMasters/slideMaster1.xml"].xml);
        const QString before = QStringLiteral("<a:defRPr sz=\"4400\"><a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>");
        QVERIFY(master.contains(before));
        master.replace(before, QStringLiteral("<a:defRPr sz=\"4400\">") + shadow + QStringLiteral("<a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>"));
        p.data["/ppt/slideMasters/slideMaster1.xml"].xml = master.toUtf8();
        QString slide = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString run = QStringLiteral("<a:rPr lang=\"en-US\"/><a:t>Quarterly results</a:t>");
        QVERIFY(slide.contains(run));
        slide.replace(run, QStringLiteral("<a:rPr lang=\"en-US\"><a:effectLst/></a:rPr><a:t>Quarterly results</a:t>"));
        p.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto titles = [&](const Document &doc, QString *first, QString *second) {
            *first = runListOf(placeholderStory(doc, *doc.pages[0], QStringLiteral("ph:title")), QStringLiteral("Quarterly"));
            *second = runListOf(placeholderStory(doc, *doc.pages[1], QStringLiteral("ph:ctrTitle")), QStringLiteral("Second"));
        };
        QString first, second;
        titles(*d, &first, &second);
        QCOMPARE(first, TextEffects::noneJson());
        QVERIFY(TextEffects::fromJson(second).outer.on);
        Exported e;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString saved = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        const int at = saved.indexOf(QStringLiteral("Quarterly results"));
        QVERIFY(at > 0);
        const QString props = saved.mid(saved.lastIndexOf(QStringLiteral("<a:rPr"), at), at - saved.lastIndexOf(QStringLiteral("<a:rPr"), at));
        QVERIFY2(props.contains(QStringLiteral("<a:effectLst/>")) || props.contains(QStringLiteral("<a:effectLst></a:effectLst>")), qPrintable(props));
        auto again = deck::importPresentation(bytes, &err);
        QVERIFY2(again, qPrintable(err));
        titles(*again, &first, &second);
        QCOMPARE(first, TextEffects::noneJson());
        QVERIFY(TextEffects::fromJson(second).outer.on);
    }

    // The Shadow button's shadow is saved (in the run's effect list, as the program writes it) and read again; and it works on each run by
    // itself: a mixed selection gets the shadow where it has none and keeps every run's other effects, and takes it off the same way.
    void shadowButtonSavesItsShadowAndWorksRunByRun()
    {
        DeckWindow w;
        QTextDocument *story = editTitle(w, QStringLiteral("AAAA BBBB CCCC"), 0, 14);
        QAction *button = w.act(QStringLiteral("fmt.shadow"));
        QTRY_VERIFY(button->isEnabled());
        TextEffects glowing, both;
        glowing.glow.on = true;
        glowing.glow.color = QColor(0, 0, 255);
        glowing.glow.radius = 5;
        both = glowing;
        both.outer = defaultTextShadow().outer;
        auto format = [&](int from, int to, const TextEffects *fx) {
            QTextCursor c(story);
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            QTextCharFormat cf;
            if (fx) cf.setProperty(tp::TextFx, fx->toJson());
            c.mergeCharFormat(cf);
        };
        format(0, 5, &glowing);   // "AAAA ": a glow, no shadow
        format(10, 14, &both);    // "CCCC": a shadow and a glow
        QVERIFY(!Editor::hasTextShadow(w.editor()->currentCharFormat()));   // (the first run's)
        button->trigger();
        auto of = [&](const QString &text) { return TextEffects::fromJson(runListOf(story, text)); };
        for (const QString &text : {QStringLiteral("AAAA"), QStringLiteral("BBBB"), QStringLiteral("CCCC")}) QVERIFY2(of(text).outer.on, qPrintable(text));
        QVERIFY(of(QStringLiteral("AAAA")).glow.on && of(QStringLiteral("CCCC")).glow.on);
        QVERIFY2(!of(QStringLiteral("BBBB")).glow.on, qPrintable(runListOf(story, QStringLiteral("BBBB")) + QStringLiteral(" | ") + story->toPlainText()));
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) QVERIFY(!it.fragment().charFormat().boolProperty(tp::Shadow));
        // Saved, the shadow is the program's own, and read again it is still there.
        QString err;
        const QByteArray bytes = deck::exportPresentation(*w.editor()->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<a:outerShdw blurRad=\"38100\" dist=\"38100\" dir=\"2700000\"")), qPrintable(slide.left(4000)));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(back->pages[0]->items[0]->type() == ItemType::Text);
        const QTextDocument *read = back->storyDoc(static_cast<const TextItem *>(back->pages[0]->items[0].get())->storyId);
        QVERIFY(read);
        QVERIFY2(TextEffects::fromJson(runListOf(read, QStringLiteral("BBBB"))).outer.on, qPrintable(read->toPlainText()));
        // Off again: from every run, the glows stay, and a run with nothing left says so (a style's shadow would not come back).
        QTRY_VERIFY(button->isChecked());
        button->trigger();
        for (const QString &text : {QStringLiteral("AAAA"), QStringLiteral("BBBB"), QStringLiteral("CCCC")}) QVERIFY2(!of(text).outer.on, qPrintable(text));
        QVERIFY(of(QStringLiteral("AAAA")).glow.on && of(QStringLiteral("CCCC")).glow.on);
        QCOMPARE(runListOf(story, QStringLiteral("BBBB")), TextEffects::noneJson());
    }

    // The Font window's Shadow box, on a slide, shows and edits the words' effect list as the Shadow button does; in a publication it is the
    // Shadow property as it was.
    void fontWindowShadowBoxFollowsTheEffectListOnASlide()
    {
        for (const bool slide : {true, false}) {
            DeckWindow w;
            w.editor()->doc()->slideText = slide;
            QTextDocument *story = editTitle(w, QStringLiteral("Hello"), 0, 5);
            QTimer::singleShot(0, [] {
                untilDone([] {
                    auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                    if (!dlg) return false;
                    for (QCheckBox *b : dlg->findChildren<QCheckBox *>())
                        if (b->text() == QStringLiteral("Shadow")) b->setChecked(!b->isChecked());
                    dlg->accept();
                    return true;
                });
            });
            fontDialog(&w, w.editor());
            const QTextCharFormat cf = w.editor()->currentCharFormat();
            if (slide) {
                QVERIFY(Editor::hasTextShadow(cf));
                QVERIFY(!cf.boolProperty(tp::Shadow));
                QVERIFY(TextEffects::fromJson(runListOf(story, QStringLiteral("Hello"))).outer.on);
            } else {
                QVERIFY(cf.boolProperty(tp::Shadow));
                QVERIFY(runListOf(story, QStringLiteral("Hello")).isEmpty());
            }
        }
    }

    // The effect lists written here follow the schema's order (glow, inner shadow, outer shadow, reflection, soft edges); one a file had is
    // written in that order with only the elements the schema has for a list, and a list of effects not drawn here (blur) is kept.
    void effectListsAreWrittenInTheSchemasOrder()
    {
        auto saved = [&](const QString &list, QString *slideXml) {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
            xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Listed\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(0, 0, 4572000, 914400)
                                                           + QStringLiteral("</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\">") + list
                                                           + QStringLiteral("</a:rPr><a:t>Words</a:t></a:r></a:p></p:txBody></p:sp></p:spTree>"));
            p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            if (!d) return false;
            Exported e;
            const QByteArray bytes = deck::exportPresentation(*d, &err);
            if (!unpack(bytes, &e, &err)) return false;
            *slideXml = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
            return true;
        };
        auto listIn = [](const QString &xml) {
            const int at = xml.indexOf(QStringLiteral("<a:effectLst"), xml.indexOf(QStringLiteral("name=\"Listed\"")));
            const int end = xml.indexOf(QStringLiteral("</a:effectLst>"), at);
            return at < 0 ? QString() : xml.mid(at, end < 0 ? 40 : end - at);
        };
        auto order = [](const QString &list, const QStringList &names) {
            int last = -1;
            for (const QString &n : names) {
                const int at = list.indexOf(QStringLiteral("<a:") + n);
                if (at < 0 || at < last) return false;
                last = at;
            }
            return true;
        };
        QString xml;
        // As a file had them, out of order and with a stranger: in order, and without it.
        QVERIFY(saved(QStringLiteral("<a:effectLst><a:softEdge rad=\"12700\"/><a:outerShdw blurRad=\"38100\" dist=\"38100\" dir=\"2700000\"><a:srgbClr val=\"000000\"/></a:outerShdw>"
                                     "<a:glow rad=\"63500\"><a:srgbClr val=\"FF0000\"/></a:glow><a:stranger/><a:blur rad=\"6350\"/></a:effectLst>"), &xml));
        QString list = listIn(xml);
        QVERIFY2(order(list, {QStringLiteral("blur"), QStringLiteral("glow"), QStringLiteral("outerShdw"), QStringLiteral("softEdge")}), qPrintable(list));
        QVERIFY2(!list.contains(QStringLiteral("stranger")), qPrintable(list));
        // A list of effects that aren't drawn here is kept.
        QVERIFY(saved(QStringLiteral("<a:effectLst><a:blur rad=\"63500\"/></a:effectLst>"), &xml));
        QVERIFY2(listIn(xml).contains(QStringLiteral("<a:blur rad=\"63500\"")), qPrintable(listIn(xml)));
        // Made here, with everything: in order.
        auto d = deck::newPresentation();
        auto box = std::make_shared<TextItem>();
        box->name = QStringLiteral("Listed");
        box->rect = QRectF(20, 20, 300, 100);
        box->storyId = d->createStory();
        TextEffects fx = shadowOf(QColor(0, 0, 0), 3, 3, 45);
        fx.glow.on = true;
        fx.glow.color = QColor(255, 0, 0);
        fx.glow.radius = 5;
        fx.inner = fx.outer;
        fx.reflection.on = true;
        fx.reflection.sy = -1;
        fx.softEdge = 2;
        QTextCharFormat cf;
        cf.setProperty(tp::TextFx, fx.toJson());
        QTextCursor(d->storyDoc(box->storyId)).insertText(QStringLiteral("Words"), cf);
        d->pages[0]->items.push_back(box);
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        list = listIn(part(e, QStringLiteral("/ppt/slides/slide1.xml")));
        QVERIFY2(order(list, {QStringLiteral("glow"), QStringLiteral("innerShdw"), QStringLiteral("outerShdw"), QStringLiteral("reflection"), QStringLiteral("softEdge")}), qPrintable(list));
    }

    // A shape's shadow is blurred as the presentation program blurs it on a slide: the radius is a box over the edge, a Gaussian of
    // about a third of it (a publication's shadows keep their own blur).
    void shapeShadowBlursAsTheProgramBlursIt()
    {
        for (const bool slide : {true, false}) {
            std::shared_ptr<Document> d = slide ? std::shared_ptr<Document>(deck::newPresentation()) : std::make_shared<Document>();
            if (!slide) d->pages.push_back(std::make_shared<Page>());
            auto box = std::make_shared<ShapeItem>();
            box->shape = QStringLiteral("rect");
            box->rect = QRectF(100, 100, 200, 100);
            box->fill = Fill::solid(ColorRef::rgb(Qt::white));
            box->stroke = Stroke::none();
            box->fx.shadow.on = true;
            box->fx.shadow.color = ColorRef::rgb(Qt::black);
            box->fx.shadow.transparency = 0;
            box->fx.shadow.blur = 6;
            box->fx.shadow.distance = 40;
            box->fx.shadow.angle = 0;
            d->pages[0]->items.push_back(box);
            LayoutCache cache;
            const QImage img = paintedSlide(d.get(), &cache, 0);
            // Along a row through the box, from the shadow's solid part out to the white: a Gaussian's edge goes from 84 to 16 percent of its
            // height over two spreads.
            auto crossing = [&](double level) {
                for (int x = 305; x < 400; ++x) {
                    const double now = qGray(img.pixel(x, 150)) / 255.0, before = qGray(img.pixel(x - 1, 150)) / 255.0;
                    if (now >= level && before < level) return x - 1 + (level - before) / (now - before);
                }
                return -1.0;
            };
            const double half = crossing(0.5), wide = crossing(0.8413) - crossing(0.1587);
            QVERIFY2(half > 0, qPrintable(QString::number(half)));
            QVERIFY2(std::abs(half - 340) < 2.0, qPrintable(QString::number(half)));
            if (slide) QVERIFY2(std::abs(wide - 2 * 0.2887 * 6) < 1.5, qPrintable(QString::number(wide)));
            else QVERIFY2(wide > 2 * 0.2887 * 6 + 1.0, qPrintable(QString::number(wide)));   // the older, wider blur
        }
    }

    // Text Art's shadow (the gallery's styles have one) is saved with its letters and read back, and drawn on the slide.
    void textArtShadowIsSavedReadAndDrawn()
    {
        auto d = deck::newPresentation();
        auto art = newTextArt(QStringLiteral("fill-a1-shadow"));
        art->name = QStringLiteral("Art");
        art->rect = QRectF(100, 100, 400, 150);
        art->text = QStringLiteral("HI");
        QVERIFY(art->fx.shadow.on);
        d->pages[0]->items.push_back(art);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:outerShdw")));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        TextArtItem *read = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->name == QLatin1String("Art") && it->type() == ItemType::TextArt) read = static_cast<TextArtItem *>(it.get());
        QVERIFY(read);
        QVERIFY(read->fx.shadow.on);
        QVERIFY2(std::abs(read->fx.shadow.blur - art->fx.shadow.blur) < 0.01 && std::abs(read->fx.shadow.distance - art->fx.shadow.distance) < 0.01, "blur and distance");
        QVERIFY(std::abs(read->fx.shadow.angle - art->fx.shadow.angle) < 0.01);
        QVERIFY(std::abs(read->fx.shadow.transparency - art->fx.shadow.transparency) < 0.01);
        // The shadow shows on the slide: letters with it have gray where the same letters without it have the page.
        LayoutCache cache;
        const QImage with = paintedSlide(back.get(), &cache, 0);
        read->fx.shadow.on = false;
        LayoutCache bare;
        const QImage without = paintedSlide(back.get(), &bare, 0);
        int darker = 0;
        for (int y = 0; y < with.height(); ++y)
            for (int x = 0; x < with.width(); ++x)
                if (qGray(with.pixel(x, y)) + 20 < qGray(without.pixel(x, y))) ++darker;
        QVERIFY2(darker > 100, qPrintable(QString::number(darker)));
    }

    // A slide's lines are the presentation program's: a single line is 1.2 times the text size whatever the font, a wider line's
    // extra space is above it, and the paragraph at the top of the box takes no space before it.
    void slideLinesAreTheProgramsLines()
    {
        for (const QString &family : {QStringLiteral("Liberation Serif"), QStringLiteral("DejaVu Sans")}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setLineHeight(150, QTextBlockFormat::ProportionalHeight);
            bf.setTopMargin(12);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontFamilies({family});
            cf.setFontPointSize(20);
            c.insertText(QStringLiteral("One"), cf);
            c.insertText(QString(QChar::LineSeparator), cf);
            c.insertText(QStringLiteral("Two"), cf);
            FrameSpec fs;
            fs.size = QSizeF(300, 300);
            fs.insets = QMarginsF(0, 0, 0, 0);
            LayoutEnv env;
            env.slideLines = true;
            StoryLayout lay;
            lay.build(&doc, {fs}, env);
            const auto lines = lay.lineInfo(0);
            QCOMPARE(int(lines.size()), 2);
            QVERIFY2(std::abs(lines[0].rect.top()) < 0.01, qPrintable(family + QLatin1Char(' ') + QString::number(lines[0].rect.top())));
            QVERIFY2(std::abs(lines[1].baseline - lines[0].baseline - 1.5 * 1.2 * 20) < 0.01, qPrintable(family + QLatin1Char(' ') + QString::number(lines[1].baseline - lines[0].baseline)));
            // The extra space is mostly above the text: the first baseline is three quarters of the way down the line.
            QVERIFY2(lines[0].baseline > 1.5 * 1.2 * 20 * 0.7, qPrintable(family + QLatin1Char(' ') + QString::number(lines[0].baseline)));
            QCOMPARE(lines[0].below, 0.0);
        }
        // A bullet drawn bigger than its text (150 percent) leaves its line the text's.
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setProperty(tp::BulletSize, 30.0);
        QTextCharFormat cf;
        cf.setFontPointSize(20);
        c.insertText(QStringLiteral("One"), cf);
        c.createList(lf);
        c.insertBlock();
        c.insertText(QStringLiteral("Two"), cf);
        FrameSpec fs;
        fs.size = QSizeF(300, 300);
        fs.insets = QMarginsF(0, 0, 0, 0);
        LayoutEnv env;
        env.slideLines = true;
        StoryLayout lay;
        lay.build(&doc, {fs}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(int(lines.size()), 2);
        QVERIFY2(std::abs(lines[0].rect.height() - 1.2 * 20) < 0.01, qPrintable(QString::number(lines[0].rect.height())));
    }

    // Text the presentation program shrank to fit keeps the size and line spacing it was given (a:normAutofit's fontScale and
    // lnSpcReduction), read and written back, until it is edited here: then JeffDeck fits it.
    void shrunkTextKeepsTheProgramsFit()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString lines;
        for (int i = 0; i < 12; ++i) lines += para(QStringLiteral("Line %1 of a long list").arg(i + 1));
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Shrunk\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 4572000, 914400)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit fontScale=\"62500\" lnSpcReduction=\"20000\"/></a:bodyPr><a:lstStyle/>")
                                                       + lines + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        TextItem *t = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Shrunk")) t = static_cast<TextItem *>(it.get());
        QVERIFY(t);
        QCOMPARE(int(t->autofit), int(TextItem::ShrinkOnOverflow));
        QVERIFY(t->fitAsStored);
        QCOMPARE(t->fitScale, 0.625);
        QCOMPARE(t->fitLineCut, 20.0);
        // Drawn at that size (18 points at 62.5 percent are 11), though it doesn't fit (the program's metrics are its own), the lines 20 points closer.
        LayoutCache cache;
        auto fl = cache.textFrame(*d, *t, 1, RenderOptions());
        QCOMPARE(fl.fitScale, 0.625);
        const auto stored = fl.layout->lineInfo(0);
        QCOMPARE(int(stored.size()), 12);
        QVERIFY2(std::abs(stored[1].baseline - stored[0].baseline - 0.8 * 1.2 * 11) < 0.05, qPrintable(QString::number(stored[1].baseline - stored[0].baseline)));
        // Written back so.
        Exported e;
        QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:normAutofit fontScale=\"62500\" lnSpcReduction=\"20000\"/>")));
        // Edited here, it is fitted here: smaller, to hold all twelve lines.
        t->fitAsStored = false;
        fl = cache.textFrame(*d, *t, 1, RenderOptions());
        QVERIFY2(fl.fitScale < 0.625, qPrintable(QString::number(fl.fitScale)));
        // The window does that as the text is typed.
        DeckWindow w;
        w.editor()->replaceDocument(deck::importPresentation(pack(p), &err), QString());
        Editor *ed = w.editor();
        TextItem *box = nullptr;
        for (const auto &it : ed->doc()->pages[0]->items)
            if (it->name == QLatin1String("Shrunk")) box = static_cast<TextItem *>(it.get());
        QVERIFY(box && box->fitAsStored);
        ed->select(box->id);
        ed->beginTextEdit(box->id, 0);
        QVERIFY(box->fitAsStored);   // clicking into it changes nothing
        ed->insertText(QStringLiteral("x"));
        QVERIFY(!box->fitAsStored);
    }

    // Text the presentation program shrank to fit is drawn in whole points: each size times the scale, rounded to the nearest point
    // (halves up, 1 at least), and a line is 1.2 times that size. Probed in the program: 28 pt at 85 percent is 24, not 23.8.
    // Other layouts (JeffPub's) keep the exact product.
    void shrunkTextIsDrawnInWholePoints()
    {
        struct Case { double size, scale, want; };
        const QVector<Case> cases = {{28, 0.85, 24}, {18, 0.925, 17}, {10.5, 0.925, 10}, {12, 0.625, 8}, {10, 0.85, 9}, {24, 0.7, 17}, {2, 0.25, 1}, {6, 0.25, 2}, {10.5, 1.0, 10.5}};
        for (const Case &c : cases) {
            QTextDocument doc;
            QTextCursor cur(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Liberation Serif")});
            cf.setFontPointSize(c.size);
            cur.insertText(QStringLiteral("Text"), cf);
            FrameSpec fs;
            fs.size = QSizeF(300, 300);
            fs.insets = QMarginsF(0, 0, 0, 0);
            LayoutEnv env;
            env.slideLines = true;
            env.fontScale = c.scale;
            StoryLayout lay;
            lay.build(&doc, {fs}, env);
            const auto lines = lay.lineInfo(0);
            QCOMPARE(int(lines.size()), 1);
            const QString what = QStringLiteral("%1 at %2: ").arg(c.size).arg(c.scale);
            QVERIFY2(std::abs(lines[0].pointSize - c.want) < 0.01, qPrintable(what + QString::number(lines[0].pointSize)));
            QVERIFY2(std::abs(lines[0].rect.height() - 1.2 * c.want) < 0.01, qPrintable(what + QString::number(lines[0].rect.height())));
            env.slideLines = false;
            StoryLayout other;
            other.build(&doc, {fs}, env);
            QVERIFY2(std::abs(other.lineInfo(0)[0].pointSize - std::max(1.0, c.size * c.scale)) < 0.01, qPrintable(what + QString::number(other.lineInfo(0)[0].pointSize)));
        }
    }

    // What isn't text stays as written when the text shrinks (probed in the program at 100, 85, 70, and 55 percent): the indents, the
    // space before and after in points, an exact line spacing, and the line spacing cut by lnSpcReduction counts off the percent.
    void shrunkTextKeepsItsDistances()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        bf.setLeftMargin(27);
        bf.setTextIndent(-9);
        bf.setTopMargin(10);
        bf.setBottomMargin(6);
        bf.setLineHeight(24, QTextBlockFormat::FixedHeight);
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Liberation Serif")});
        cf.setFontPointSize(20);
        c.insertText(QStringLiteral("One"), cf);
        c.insertBlock(bf);
        c.insertText(QStringLiteral("Two"), cf);
        FrameSpec fs;
        fs.size = QSizeF(300, 300);
        fs.insets = QMarginsF(0, 0, 0, 0);
        LayoutEnv env;
        env.slideLines = true;
        env.fontScale = 0.5;
        StoryLayout lay;
        lay.build(&doc, {fs}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(int(lines.size()), 2);
        QVERIFY2(std::abs(lines[0].rect.left() - 18) < 0.01, qPrintable(QString::number(lines[0].rect.left())));
        QVERIFY2(std::abs(lines[0].rect.height() - 24) < 0.01, qPrintable(QString::number(lines[0].rect.height())));
        QVERIFY2(std::abs(lines[1].rect.top() - lines[0].rect.bottom() - 16) < 0.01, qPrintable(QString::number(lines[1].rect.top() - lines[0].rect.bottom())));
        // An exact spacing isn't cut by lnSpcReduction either.
        env.lineCut = 20;
        StoryLayout cut;
        cut.build(&doc, {fs}, env);
        QVERIFY2(std::abs(cut.lineInfo(0)[0].rect.height() - 24) < 0.01, qPrintable(QString::number(cut.lineInfo(0)[0].rect.height())));
    }

    // Space before or after a paragraph that a file gives as a share of the line follows the size the text is drawn at (shrunk or the text's own
    // size changed), and the same space given in points stays (probed: 20 percent of a 14 point line is 3.36 points, 12 points stay 12 at 55
    // percent). A margin set since the file was read is in points, the share ended, however near the old margin it is.
    void shrunkTextSpaceByShareFollowsTheSize()
    {
        // The space before the second of two paragraphs, `fontScale` and `size` as given: its format, the first paragraph plain.
        auto before = [](const QTextBlockFormat &second, double size, double fontScale) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Liberation Serif")});
            cf.setFontPointSize(size);
            c.insertText(QStringLiteral("One"), cf);
            c.insertBlock(second, cf);
            c.insertText(QStringLiteral("Two"), cf);
            FrameSpec fs;
            fs.size = QSizeF(300, 300);
            fs.insets = QMarginsF(0, 0, 0, 0);
            LayoutEnv env;
            env.slideLines = true;
            env.fontScale = fontScale;
            StoryLayout lay;
            lay.build(&doc, {fs}, env);
            const auto lines = lay.lineInfo(0);
            return lines.size() == 2 ? lines[1].rect.top() - lines[0].rect.bottom() : -1.0;
        };
        // What a file's 20 percent before and 50 percent after came to at 28 points.
        QTextBlockFormat share;
        share.setTopMargin(0.2 * 1.2 * 28);
        share.setBottomMargin(0.5 * 1.2 * 28);
        share.setProperty(tp::SpaceBeforeShare, 0.2);
        share.setProperty(tp::SpaceAfterShare, 0.5);
        share.setProperty(tp::SpaceBeforeFrom, 0.2 * 1.2 * 28);
        share.setProperty(tp::SpaceAfterFrom, 0.5 * 1.2 * 28);
        QTextBlockFormat points;
        points.setTopMargin(6.72);
        QTextBlockFormat edited = share;
        edited.setTopMargin(9);        // set since the file was read: points now
        QTextBlockFormat nearby = share;
        nearby.setTopMargin(6.75);     // typed a few hundredths from the old margin: points too
        QTextBlockFormat shown = share;
        shown.setTopMargin(6.72);      // what the Paragraph dialog hands back for it (to the hundredth): nothing changed
        QTextBlockFormat odd = share;
        odd.setProperty(tp::SpaceBeforeFrom, 4.152);
        odd.setTopMargin(4.15);        // the same for a margin that isn't a whole hundredth
        auto near = [](double a, double b) { return std::abs(a - b) < 0.01; };
        double gap = before(share, 28, 0.55);    // 28 points are 15
        QVERIFY2(near(gap, 0.2 * 1.2 * 15), qPrintable(QString::number(gap)));
        gap = before(share, 20, 0.55);           // the text's size changed to 20, which is 11: the share goes on
        QVERIFY2(near(gap, 0.2 * 1.2 * 11), qPrintable(QString::number(gap)));
        gap = before(share, 20, 1.0);            // not shrunk: the share of the size it has
        QVERIFY2(near(gap, 0.2 * 1.2 * 20), qPrintable(QString::number(gap)));
        gap = before(points, 28, 0.55);
        QVERIFY2(near(gap, 6.72), qPrintable(QString::number(gap)));
        gap = before(edited, 28, 0.55);
        QVERIFY2(near(gap, 9), qPrintable(QString::number(gap)));
        gap = before(nearby, 28, 0.55);
        QVERIFY2(near(gap, 6.75), qPrintable(QString::number(gap)));
        gap = before(shown, 28, 0.55);
        QVERIFY2(near(gap, 0.2 * 1.2 * 15), qPrintable(QString::number(gap)));
        gap = before(odd, 28, 0.55);
        QVERIFY2(near(gap, 0.2 * 1.2 * 15), qPrintable(QString::number(gap)));
        // After: the first paragraph's, with the second plain.
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Liberation Serif")});
        cf.setFontPointSize(28);
        c.setBlockFormat(share);
        c.insertText(QStringLiteral("One"), cf);
        c.insertBlock(QTextBlockFormat(), cf);
        c.insertText(QStringLiteral("Two"), cf);
        FrameSpec fs;
        fs.size = QSizeF(300, 300);
        fs.insets = QMarginsF(0, 0, 0, 0);
        LayoutEnv env;
        env.slideLines = true;
        env.fontScale = 0.55;
        StoryLayout lay;
        lay.build(&doc, {fs}, env);
        const auto lines = lay.lineInfo(0);
        QCOMPARE(int(lines.size()), 2);
        QVERIFY2(near(lines[1].rect.top() - lines[0].rect.bottom(), 0.5 * 1.2 * 15), qPrintable(QString::number(lines[1].rect.top() - lines[0].rect.bottom())));
    }

    // Saving shrunk text writes its space as the file had it: a share of the line as a share, not as the points it came to at the size the text
    // was written at (the program doesn't shrink points), so on opening again nothing has moved. The shares go with the text when it is
    // copied (the lossless copy and a duplicated slide use the story's JSON), and with the file.
    void savingShrunkTextKeepsItsSpaceShares()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString paras;
        for (int i = 1; i <= 4; ++i)
            paras += QStringLiteral("<a:p><a:pPr><a:spcBef><a:spcPct val=\"20000\"/></a:spcBef><a:spcAft><a:spcPct val=\"30000\"/></a:spcAft></a:pPr>"
                                    "<a:r><a:rPr lang=\"en-US\" sz=\"2800\"><a:latin typeface=\"Liberation Serif\"/></a:rPr><a:t>Paragraph number %1</a:t></a:r></a:p>").arg(i);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Shrunk\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 5400000, 3800000)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit fontScale=\"85000\" lnSpcReduction=\"10000\"/></a:bodyPr><a:lstStyle/>")
                                                       + paras + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto find = [](Document &doc) -> TextItem * {
            for (const auto &it : doc.pages[0]->items)
                if (it->name == QLatin1String("Shrunk") && it->type() == ItemType::Text) return static_cast<TextItem *>(it.get());
            return nullptr;
        };
        auto lines = [](Document &doc, TextItem *t) {
            LayoutCache cache;
            return cache.textFrame(doc, *t, 1, RenderOptions()).layout->lineInfo(0);
        };
        TextItem *t = find(*d);
        QVERIFY(t && t->fitAsStored);
        const auto before = lines(*d, t);
        QCOMPARE(int(before.size()), 4);
        // 24 points: 20 percent before and 30 percent after of a line of 1.2 times that.
        QVERIFY2(std::abs(before[1].rect.top() - before[0].rect.bottom() - (0.3 + 0.2) * 1.2 * 24) < 0.01, qPrintable(QString::number(before[1].rect.top() - before[0].rect.bottom())));
        // Written as shares.
        QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<a:spcBef><a:spcPct val=\"20000\"/></a:spcBef>")), qPrintable(slide.left(3000)));
        QVERIFY(slide.contains(QStringLiteral("<a:spcAft><a:spcPct val=\"30000\"/></a:spcAft>")));
        // And where they were on opening it again, shares and all.
        auto again = deck::importPresentation(bytes, &err);
        QVERIFY2(again, qPrintable(err));
        TextItem *t2 = find(*again);
        QVERIFY(t2);
        const auto after = lines(*again, t2);
        QCOMPARE(int(after.size()), 4);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(std::abs(after[i].rect.top() - before[i].rect.top()) < 0.01, qPrintable(QStringLiteral("%1: %2 vs %3").arg(i).arg(after[i].rect.top()).arg(before[i].rect.top())));
            QVERIFY2(std::abs(after[i].baseline - before[i].baseline) < 0.01, qPrintable(QString::number(i)));
        }
        QTextDocument *story = again->storyDoc(t2->storyId);
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            QCOMPARE(tp::spaceShareOf(b.blockFormat(), true), 0.2);
            QCOMPARE(tp::spaceShareOf(b.blockFormat(), false), 0.3);
        }
        // Copied (the story's JSON), the text keeps its shares and so its place.
        QTextDocument copy;
        storyFromJson(&copy, storyToJson(story));
        for (QTextBlock b = copy.begin(); b.isValid(); b = b.next()) {
            QCOMPARE(tp::spaceShareOf(b.blockFormat(), true), 0.2);
            QCOMPARE(tp::spaceShareOf(b.blockFormat(), false), 0.3);
        }
        // Points set since are written as points: the share is over.
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            QTextBlockFormat bf = b.blockFormat();
            bf.setTopMargin(10);
            QTextCursor(b).setBlockFormat(bf);
        }
        bytes = deck::exportPresentation(*again, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString edited = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(edited.contains(QStringLiteral("<a:spcBef><a:spcPts val=\"1000\"/></a:spcBef>")));
        QVERIFY(edited.contains(QStringLiteral("<a:spcAft><a:spcPct val=\"30000\"/></a:spcAft>")));
    }

    // The program rounds a size to a whole point however little it shrinks it, so a size can come out bigger than the text's own (probed: 10.6
    // points at 99.99 percent are 11, at 100 percent 10.6), and never smaller as the scale goes up below 100. JeffDeck's own fit searches the
    // scales under 100 (the whole-point sizes step up with the scale there) after trying 100 itself: a box that needs a tiny shrink gets
    // the largest whole-point size that fits, one that fits as it is keeps its size.
    void shrunkSizesStepWithTheScaleAndFitTakesTheLargestThatFits()
    {
        QCOMPARE(slideShrunkSize(10.6, 1.0), 10.6);
        QCOMPARE(slideShrunkSize(10.6, 0.9999), 11.0);
        QCOMPARE(slideShrunkSize(10.6, 0.99), 10.0);
        for (double size : {1.0, 2.5, 9.7, 10.5, 10.6, 13.9, 24.6, 44.0}) {
            double last = 0;
            for (double scale = 0.05; scale < 1.0; scale += 0.0007) {
                const double now = slideShrunkSize(size, scale);
                QVERIFY2(now >= last, qPrintable(QStringLiteral("%1 at %2").arg(size).arg(scale)));
                last = now;
            }
        }
        // A box of eight 10.6 point lines (12.72 points each): 108 points tall it needs a hair less than that, 110 holds them as they are.
        auto fitOf = [](double height, QVector<double> *sizes) {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
            QString paras;
            for (int i = 1; i <= 8; ++i)
                paras += QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\" sz=\"1060\"><a:latin typeface=\"Liberation Serif\"/></a:rPr><a:t>Line %1</a:t></a:r></a:p>").arg(i);
            xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Fit\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                           + xfrm(0, 0, 4572000, qint64(height * 12700))
                                                           + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit/></a:bodyPr><a:lstStyle/>") + paras + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
            p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            if (!d) return -1.0;
            TextItem *t = nullptr;
            for (const auto &it : d->pages[0]->items)
                if (it->name == QLatin1String("Fit")) t = static_cast<TextItem *>(it.get());
            if (!t) return -1.0;
            t->fitAsStored = false;   // fitted here
            LayoutCache cache;
            const auto fl = cache.textFrame(*d, *t, 1, RenderOptions());
            for (const auto &l : fl.layout->lineInfo(0)) sizes->append(l.pointSize);
            return fl.fitScale;
        };
        QVector<double> tight, roomy;
        const double needs = fitOf(108, &tight), holds = fitOf(110, &roomy);
        QVERIFY2(needs > 0.97 && needs < 1.0, qPrintable(QString::number(needs)));
        QCOMPARE(int(tight.size()), 8);
        for (double size : std::as_const(tight)) QCOMPARE(size, 10.0);   // 10.6 shrunk a hair is 10 whole points, and they fit
        QCOMPARE(holds, 1.0);
        for (double size : std::as_const(roomy)) QCOMPARE(size, 10.6);
    }

    // Copying shrunk text in the window and pasting it (the lossless copy, the story's JSON) keeps the shares of the line its space is given in,
    // so the pasted paragraphs are spaced as they were.
    void copyingShrunkTextKeepsItsSpaceShares()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString paras;
        for (int i = 1; i <= 3; ++i)
            paras += QStringLiteral("<a:p><a:pPr><a:spcBef><a:spcPct val=\"20000\"/></a:spcBef><a:spcAft><a:spcPct val=\"30000\"/></a:spcAft></a:pPr>"
                                    "<a:r><a:rPr lang=\"en-US\" sz=\"2800\"/><a:t>Paragraph %1</a:t></a:r></a:p>").arg(i);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Shrunk\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 5400000, 3800000)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr><a:normAutofit fontScale=\"85000\"/></a:bodyPr><a:lstStyle/>")
                                                       + paras + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        DeckWindow w;
        w.editor()->replaceDocument(deck::importPresentation(pack(p), &err), QString());
        Editor *ed = w.editor();
        TextItem *box = nullptr;
        for (const auto &it : ed->doc()->pages[0]->items)
            if (it->name == QLatin1String("Shrunk")) box = static_cast<TextItem *>(it.get());
        QVERIFY(box);
        ed->select(box->id);
        ed->beginTextEdit(box->id, 0);
        QTextCursor all = ed->cursor();
        all.select(QTextCursor::Document);
        ed->setCursor(all);
        ed->copy();
        QTextCursor end = ed->cursor();
        end.movePosition(QTextCursor::End);
        ed->setCursor(end);
        ed->paste();
        QTextDocument *story = ed->doc()->storyDoc(box->storyId);
        // Three paragraphs and the three pasted after them, the first of which went on the end of the last one.
        QCOMPARE(story->blockCount(), 5);
        int shared = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            if (b.text().isEmpty()) continue;
            QCOMPARE(tp::spaceShareOf(b.blockFormat(), true), 0.2);
            QCOMPARE(tp::spaceShareOf(b.blockFormat(), false), 0.3);
            ++shared;
        }
        QCOMPARE(shared, 5);
    }

    // Pair kerning follows the size the text is drawn at: a run kerned from 12 points is kerned at 15 points and not at 9 when shrunk
    // from 28 (probed in the program). Other layouts count the size given.
    void shrunkTextKernsAtTheSizeItIsDrawnAt()
    {
        auto kerned = [](double size, double scale, bool slide, double from) {
            QTextCharFormat cf;
            cf.setFontPointSize(size);
            if (from > 0) cf.setProperty(tp::KernAbove, from);
            LayoutEnv env;
            env.slideLines = slide;
            env.fontScale = scale;
            return resolveCharFormat(cf, env).fontKerning();
        };
        QVERIFY(kerned(28, 1.0, true, 12));
        QVERIFY(kerned(28, 0.55, true, 12));
        QVERIFY(!kerned(28, 0.325, true, 12));
        QVERIFY(kerned(28, 0.325, false, 12));
        QVERIFY(!kerned(13, 1.0, false, 0));
    }

    // A slide's line that isn't single (a share of the line other than 100 percent, lnSpcReduction counted, or an exact height, shrunk or
    // not): taller than a single line, its baseline is 3/4 of the way down it whatever the font (probed in 20 fonts); shorter, it keeps the
    // font's descent below it but sits no higher than 3/4 down. The program's single line is the font's own.
    void slideBaselinesOfSpacedLinesFollowTheProgram()
    {
        struct Case { QTextBlockFormat::LineHeightTypes type; double height, scale, cut, line; };
        const QVector<Case> cases = {{QTextBlockFormat::ProportionalHeight, 150, 1.0, 0, 1.5 * 1.2 * 24}, {QTextBlockFormat::ProportionalHeight, 90, 1.0, 0, 0.9 * 1.2 * 24},
                                     {QTextBlockFormat::ProportionalHeight, 90, 1.0, 20, 0.7 * 1.2 * 24}, {QTextBlockFormat::SingleHeight, 0, 1.0, 10, 0.9 * 1.2 * 24},
                                     {QTextBlockFormat::ProportionalHeight, 110, 1.0, 10, 1.2 * 24}, {QTextBlockFormat::FixedHeight, 30, 1.0, 0, 30},
                                     {QTextBlockFormat::FixedHeight, 30, 0.7, 20, 30}, {QTextBlockFormat::FixedHeight, 15, 1.0, 0, 15}};
        for (const QString &family : {QStringLiteral("Liberation Serif"), QStringLiteral("DejaVu Sans")}) {
            auto layout = [&](const Case &c) {
                QTextDocument doc;
                QTextCursor cur(&doc);
                QTextBlockFormat bf;
                if (c.type != QTextBlockFormat::SingleHeight) bf.setLineHeight(c.height, c.type);
                cur.setBlockFormat(bf);
                QTextCharFormat cf;
                cf.setFontFamilies({family});
                cf.setFontPointSize(c.scale < 1 ? 24 / c.scale : 24);   // 34.3 points shrunk to the 24 the case counts on (rounded)
                cur.insertText(QStringLiteral("Text"), cf);
                FrameSpec fs;
                fs.size = QSizeF(300, 300);
                fs.insets = QMarginsF(0, 0, 0, 0);
                LayoutEnv env;
                env.slideLines = true;
                env.fontScale = c.scale;
                env.lineCut = c.cut;
                StoryLayout lay;
                lay.build(&doc, {fs}, env);
                const auto lines = lay.lineInfo(0);
                return lines.size() == 1 ? lines[0] : StoryLayout::LineInfo();
            };
            // The font's descent: what a single line has below its baseline.
            const auto single = layout({QTextBlockFormat::SingleHeight, 0, 1.0, 0, 1.2 * 24});
            const double descent = single.rect.bottom() - single.baseline;
            QVERIFY2(descent > 1 && descent < 10, qPrintable(family + QLatin1Char(' ') + QString::number(descent)));
            for (const Case &c : cases) {
                const auto line = layout(c);
                const QString what = family + QLatin1Char(' ') + QString::number(c.height) + QLatin1Char(' ') + QString::number(c.cut) + QLatin1String(": ");
                QVERIFY2(std::abs(line.rect.height() - c.line) < 0.01, qPrintable(what + QString::number(line.rect.height())));
                const double wanted = c.line > 1.2 * 24 + 0.01 ? 0.75 * c.line : std::max(0.75 * c.line, c.line - descent);
                QVERIFY2(std::abs(line.baseline - line.rect.top() - wanted) < 0.01, qPrintable(what + QString::number(line.baseline - line.rect.top()) + QStringLiteral(" vs ") + QString::number(wanted)));
            }
        }
    }

    // A box of text the program shrank, as a file gives it: the sizes, the cut line spacing, the space before as a share of the line
    // and the space after in points, all as probed in the program (28 points at 85 percent are 24; 90 percent less 20 is 70).
    void shrunkTextIsSetAsTheProgramSetsIt()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString paras;
        for (const char *word : {"Alpha", "Beta"})
            paras += QStringLiteral("<a:p><a:pPr marL=\"342900\" indent=\"-342900\"><a:lnSpc><a:spcPct val=\"90000\"/></a:lnSpc><a:spcBef><a:spcPct val=\"20000\"/></a:spcBef>"
                                    "<a:spcAft><a:spcPts val=\"600\"/></a:spcAft><a:buFont typeface=\"Arial\"/><a:buChar char=\"&#8226;\"/></a:pPr>"
                                    "<a:r><a:rPr lang=\"en-US\" sz=\"2800\"><a:latin typeface=\"Liberation Serif\"/></a:rPr><a:t>%1</a:t></a:r></a:p>").arg(QLatin1String(word));
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Shrunk\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>")
                                                       + xfrm(0, 0, 4572000, 2743200)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\"><a:normAutofit fontScale=\"85000\" lnSpcReduction=\"20000\"/></a:bodyPr><a:lstStyle/>")
                                                       + paras + QStringLiteral("</p:txBody></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        TextItem *t = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Shrunk")) t = static_cast<TextItem *>(it.get());
        QVERIFY(t && t->fitAsStored);
        LayoutCache cache;
        const auto lines = cache.textFrame(*d, *t, 1, RenderOptions()).layout->lineInfo(0);
        QCOMPARE(int(lines.size()), 2);
        QVERIFY2(std::abs(lines[0].pointSize - 24) < 0.01, qPrintable(QString::number(lines[0].pointSize)));
        const double line = 1.2 * 24 * 0.70;
        QVERIFY2(std::abs(lines[0].rect.height() - line) < 0.01, qPrintable(QString::number(lines[0].rect.height())));
        // The line is shorter than a single one: it keeps the font's descent below the baseline, but sits no higher than 3/4 down.
        QTextDocument one;
        QTextCursor oneCursor(&one);
        QTextCharFormat oneFormat;
        oneFormat.setFontFamilies({QStringLiteral("Liberation Serif")});
        oneFormat.setFontPointSize(24);
        oneCursor.insertText(QStringLiteral("Text"), oneFormat);
        FrameSpec oneSpec;
        oneSpec.size = QSizeF(300, 300);
        oneSpec.insets = QMarginsF(0, 0, 0, 0);
        LayoutEnv oneEnv;
        oneEnv.slideLines = true;
        StoryLayout oneLayout;
        oneLayout.build(&one, {oneSpec}, oneEnv);
        const double descent = oneLayout.lineInfo(0)[0].rect.bottom() - oneLayout.lineInfo(0)[0].baseline;
        const double wanted = std::max(0.75 * line, line - descent);
        QVERIFY2(std::abs(lines[0].baseline - lines[0].rect.top() - wanted) < 0.01, qPrintable(QString::number(lines[0].baseline - lines[0].rect.top()) + QStringLiteral(" vs ") + QString::number(wanted)));
        // 6 points after, then 20 percent of a 24 point line before: 6 + 5.76.
        QVERIFY2(std::abs(lines[1].rect.top() - lines[0].rect.bottom() - (6 + 0.2 * 1.2 * 24)) < 0.01, qPrintable(QString::number(lines[1].rect.top() - lines[0].rect.bottom())));
        // The text starts 27 points in, the hang being as written.
        QVERIFY2(std::abs(lines[0].rect.left() - 27) < 0.01, qPrintable(QString::number(lines[0].rect.left())));
    }
};

JP_TEST_CLASS(DeckTextEffectsTests)
#include "tst_deck_texteffects.moc"
