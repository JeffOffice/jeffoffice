// JeffDeck's tests of themes and variants.
#include "deckhelpers.h"
#include "../timing.h"

struct BackdropDeck {
    std::unique_ptr<Document> doc;
    QString plain, gray, callout, card, overRect, overPicture, table;
};

class DeckThemeTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // ---- themes ----

    // The fill of a shape or background as the colors it runs through: both ends of a gradient.
    static QVector<QColor> fillColors(const Fill &f, const ColorScheme &scheme)
    {
        QVector<QColor> out;
        if (f.type == Fill::Gradient) {
            if (f.stops.isEmpty()) out << f.color.resolve(scheme) << f.color2.resolve(scheme);
            for (const GradientStop &s : f.stops) out << s.color.resolve(scheme);
        } else if (f.type == Fill::Solid) {
            out << f.color.resolve(scheme);
        }
        return out;
    }

    // The colors a point of a layout can show: its background (every stop of a gradient), then the filled shapes of its slide
    // master (unless the layout hides them) and of its own drawn over each.
    static QVector<QColor> colorsUnder(const Document &d, const MasterPage &layout, const QPointF &p)
    {
        const QVector<MasterPage *> chain = d.masterChain(&layout);
        Fill background;
        for (int i = int(chain.size()) - 1; i >= 0 && background.type == Fill::NoFill; --i) background = chain[i]->background;
        QVector<QColor> shown = fillColors(background, d.colors);
        if (shown.isEmpty()) shown << QColor(Qt::white);
        for (const MasterPage *m : chain) {
            if (m != &layout && !layout.showMasterItems) continue;
            for (const auto &it : m->items) {
                if (!it->role.isEmpty() || it->type() != ItemType::Shape || it->fill.type != Fill::Solid) continue;
                const auto &s = static_cast<const ShapeItem &>(*it);
                if (!shapePath(s.shape, s.rect.size(), s.adj).contains(it->transform().inverted().map(p))) continue;
                const QColor over = it->fill.color.resolve(d.colors);
                for (QColor &c : shown) c = mix(c, over, 1 - it->fill.transparency);
            }
        }
        return shown;
    }

    // Every run and every bullet of every placeholder of the slide master and the layouts, and the hyperlink colors in each of them, readable
    // over what can be behind them (WCAG AA): text at 4.5 to 1, large text (18 points, or 14 and bold) and bullets at 3 to 1, links at 4.5.
    // What is behind is looked at in nine places in each placeholder, on the background and the shapes the master and layout draw.
    static QStringList unreadableText(const Document &d, const QString &what)
    {
        QStringList problems;
        const deck::DeckTheme theme = deck::themeOf(d);
        for (const auto &m : d.masters) {
            if (!deck::isLayout(*m) && !deck::isSlideMaster(*m)) continue;
            for (const auto &it : m->items) {
                const QString type = deck::placeholderType(*it);
                if (type.isEmpty() || type == QLatin1String("pic") || it->type() != ItemType::Text) continue;
                QVector<QColor> behind;
                const QRectF r = it->rect;
                for (double fx : {0.1, 0.5, 0.9})
                    for (double fy : {0.1, 0.5, 0.9})
                        for (const QColor &c : colorsUnder(d, *m, QPointF(r.left() + r.width() * fx, r.top() + r.height() * fy)))
                            if (!behind.contains(c)) behind << c;
                auto check = [&](const QString &kind, const QColor &fg, double need) {
                    for (const QColor &bg : std::as_const(behind)) {
                        const double c = deck::contrastRatio(fg, bg);
                        if (c < need) problems << QStringLiteral("%1, %2, %3: %4 %5 on %6 is %7 (needs %8)").arg(what, m->name, it->role, kind, fg.name(), bg.name()).arg(c, 0, 'f', 2).arg(need);
                    }
                };
                const QTextDocument *sd = d.storyDoc(static_cast<const TextItem &>(*it).storyId);
                QSet<const QTextList *> seen;
                for (QTextBlock b = sd ? sd->begin() : QTextBlock(); b.isValid(); b = b.next()) {
                    QVector<QTextCharFormat> formats;
                    formats << b.charFormat();   // the mark at the end of the paragraph: what is typed next
                    for (auto f = b.begin(); !f.atEnd(); ++f) formats << f.fragment().charFormat();
                    for (const QTextCharFormat &cf : std::as_const(formats)) {
                        const QColor fg = colorOfRun(d, cf);
                        if (!fg.isValid()) continue;
                        const double size = cf.fontPointSize();
                        const bool large = size >= 18 || (size >= 14 && cf.fontWeight() >= QFont::Bold);
                        check(QStringLiteral("text"), fg, large ? 3.0 : 4.5);
                    }
                    if (const QTextList *list = b.textList(); list && !seen.contains(list)) {
                        seen.insert(list);
                        const ColorRef bullet = ColorRef::fromString(list->format().stringProperty(tp::BulletColor));
                        if (bullet.kind() != ColorRef::None) check(QStringLiteral("bullet"), bullet.resolve(d.colors), 3.0);
                    }
                }
            }
        }
        // Hyperlinks: in a text box on the page they read at 4.5 to 1 on the background; typed into a placeholder they read over what is behind it
        // (4.5 in small text, 3 in large; not in the footers), whenever one color can read on all of that and the page; where none can (a band
        // that is dark on a light page, a gradient's middle), the page is what they read on.
        struct Need {
            QColor behind;
            double need;
        };
        QVector<Need> page, placed;
        for (const auto &m : d.masters) {
            if (!deck::isLayout(*m) && !deck::isSlideMaster(*m)) continue;
            Fill background;
            for (const MasterPage *c : d.masterChain(m.get())) background = c->background.type == Fill::NoFill ? background : c->background;
            for (const QColor &c : fillColors(background, d.colors).isEmpty() ? QVector<QColor>{QColor(Qt::white)} : fillColors(background, d.colors)) page.push_back({c, 4.5});
            for (const auto &it : m->items) {
                const QString type = deck::placeholderType(*it);
                if (type.isEmpty() || it->type() != ItemType::Text || type == QLatin1String("pic") || type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum")) continue;
                const QTextDocument *sd = d.storyDoc(static_cast<const TextItem &>(*it).storyId);
                QTextCharFormat cf = sd ? sd->begin().charFormat() : QTextCharFormat();
                for (QTextBlock b = sd ? sd->begin() : QTextBlock(); b.isValid(); b = b.next())
                    if (!b.begin().atEnd()) {
                        cf = b.begin().fragment().charFormat();
                        break;
                    }
                const bool large = cf.fontPointSize() >= 18 || (cf.fontPointSize() >= 14 && cf.fontWeight() >= QFont::Bold);
                for (double fx : {0.1, 0.5, 0.9})
                    for (double fy : {0.1, 0.5, 0.9})
                        for (const QColor &c : colorsUnder(d, *m, QPointF(it->rect.left() + it->rect.width() * fx, it->rect.top() + it->rect.height() * fy))) placed.push_back({c, large ? 3.0 : 4.5});
            }
        }
        auto luminance = [](const QColor &c) {
            auto channel = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
            return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
        };
        auto anyColorReads = [&](const QVector<Need> &needs) {
            for (int step = 0; step <= 800; ++step) {
                const double l = step / 800.0;
                bool all = true;
                for (const Need &n : needs) {
                    const double b = luminance(n.behind);
                    all = all && ((l + 0.05) / (b + 0.05) >= n.need || (b + 0.05) / (l + 0.05) >= n.need);
                }
                if (all) return true;
            }
            return false;
        };
        QVector<Need> every = page;
        every += placed;
        const bool pageReads = anyColorReads(page), everythingReads = anyColorReads(every);
        for (int slot : {10, 11}) {
            const QColor link = theme.colors[slot];
            for (const Need &n : pageReads ? (everythingReads ? every : page) : QVector<Need>())
                if (deck::contrastRatio(link, n.behind) < n.need)
                    problems << QStringLiteral("%1: hyperlink color %2 on %3 is %4 (needs %5)").arg(what, link.name(), n.behind.name()).arg(deck::contrastRatio(link, n.behind), 0, 'f', 2).arg(n.need);
        }
        problems.removeDuplicates();
        return problems;
    }

    // The words of a story, set in its first run's look.
    static void sayIn(Document &d, const Item &it, const QString &text)
    {
        QTextCursor c(d.storyDoc(static_cast<const TextItem &>(it).storyId));
        c.insertText(text);
    }

    static void bulletIn(Document &d, const Item &it, const QStringList &lines)
    {
        QTextDocument *sd = d.storyDoc(static_cast<const TextItem &>(it).storyId);
        QTextCursor c(sd);
        c.insertText(lines.join(QLatin1Char('\n')));
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        c.select(QTextCursor::Document);
        QTextList *list = nullptr;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
            QTextCursor bc(b);
            QTextBlockFormat bf = b.blockFormat();
            bf.setLeftMargin(18);
            bf.setTextIndent(-18);
            bc.setBlockFormat(bf);
            if (!list) list = bc.createList(lf);
            else list->add(b);
        }
    }

    // The format of the first run of a story (a block with no run: the block's).
    static QTextCharFormat firstRunOf(const Document &d, const Item &it)
    {
        const QTextDocument *sd = d.storyDoc(static_cast<const TextItem &>(it).storyId);
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
            if (!b.begin().atEnd()) return b.begin().fragment().charFormat();
        return sd->begin().charFormat();
    }

    static QString familyOf(const QTextCharFormat &cf) { return cf.fontFamilies().toStringList().value(0); }

    static QColor colorOfRun(const Document &d, const QTextCharFormat &cf)
    {
        const QString ref = cf.stringProperty(tp::ColorRefP);
        return ref.isEmpty() ? cf.foreground().color() : ColorRef::fromString(ref).resolve(d.colors);
    }

    // A presentation with a slide of each kind a theme must take over: its text bulleted, a hand-set red word and a bold one, a
    // text box and a shape in the theme's accent color, a text box in a font of its own.
    static std::unique_ptr<Document> deckToRestyle()
    {
        auto d = deck::newPresentation();
        const deck::DeckTheme theme = deck::themeOf(*d);
        sayIn(*d, *placeholderOn(*d->pages[0], QStringLiteral("ctrTitle")), QStringLiteral("Annual review"));
        sayIn(*d, *placeholderOn(*d->pages[0], QStringLiteral("subTitle")), QStringLiteral("Finance team"));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        sayIn(*d, *placeholderOn(*d->pages[1], QStringLiteral("title")), QStringLiteral("Results"));
        bulletIn(*d, *placeholderOn(*d->pages[1], QStringLiteral("obj")), {QStringLiteral("Sales rose"), QStringLiteral("Costs fell"), QStringLiteral("Hand-colored")});
        {
            QTextDocument *sd = d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("obj"))->storyId);
            QTextCursor c(sd->findBlockByNumber(2));
            c.select(QTextCursor::BlockUnderCursor);
            c.movePosition(QTextCursor::StartOfBlock);
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat red;
            red.setForeground(QColor(0xC0, 0x00, 0x00));
            red.setProperty(tp::ColorRefP, ColorRef::rgb(QColor(0xC0, 0x00, 0x00)).toString());
            red.setFontWeight(QFont::Bold);
            c.mergeCharFormat(red);
        }
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("twoObj")));
        sayIn(*d, *placeholderOn(*d->pages[2], QStringLiteral("title")), QStringLiteral("Compare"));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        sayIn(*d, *placeholderOn(*d->pages[3], QStringLiteral("title")), QStringLiteral("Notes"));
        auto box = std::make_shared<TextItem>();
        box->name = QStringLiteral("accent text");
        box->rect = QRectF(100, 200, 300, 60);
        box->storyId = d->createStory(QStringLiteral("Accent words"));
        {
            QTextCursor c(d->storyDoc(box->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies({theme.bodyFont});
            f.setFontPointSize(20);
            f.setForeground(theme.colors[4]);
            f.setProperty(tp::ColorRefP, ColorRef::rgb(theme.colors[4]).toString());
            c.mergeCharFormat(f);
        }
        d->pages[3]->items.push_back(box);
        auto own = std::make_shared<TextItem>();
        own->name = QStringLiteral("own font");
        own->rect = QRectF(100, 300, 300, 60);
        own->storyId = d->createStory(QStringLiteral("Typewriter words"));
        {
            QTextCursor c(d->storyDoc(own->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies({QStringLiteral("Courier Prime")});
            f.setFontPointSize(20);
            c.mergeCharFormat(f);
        }
        d->pages[3]->items.push_back(own);
        auto shape = std::make_shared<ShapeItem>();
        shape->name = QStringLiteral("accent shape");
        shape->rect = QRectF(500, 200, 200, 100);
        shape->fill = Fill::solid(ColorRef::rgb(theme.colors[5]));
        shape->stroke = Stroke::line(ColorRef::rgb(theme.colors[4]), 2);
        d->pages[3]->items.push_back(shape);
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        return d;
    }

    static const Item *itemNamed(const Page &p, const QString &name)
    {
        for (const auto &it : p.items)
            if (it->name == name) return it.get();
        return nullptr;
    }

    // JeffDeck ships nine themes of its own: the first is the one a new presentation has, the others are named and drawn by
    // JeffOffice (no other program's names), with four variants each, in fonts it bundles.
    void builtinThemesAreNineOriginalOnes()
    {
        QCOMPARE(deck::builtinThemeCount(), 9);
        QCOMPARE(deck::builtinThemeName(0), deck::defaultTheme().name);
        QCOMPARE(deck::builtinThemeIndex(QStringLiteral("JeffOffice")), 0);
        QCOMPARE(deck::builtinThemeIndex(QStringLiteral("No Such Theme")), -1);
        const QStringList others = {QStringLiteral("facet"), QStringLiteral("ion"), QStringLiteral("retrospect"), QStringLiteral("wisp"), QStringLiteral("integral"),
                                    QStringLiteral("organic"), QStringLiteral("slice"), QStringLiteral("gallery"), QStringLiteral("office theme"), QStringLiteral("berlin"),
                                    QStringLiteral("circuit"), QStringLiteral("badge"), QStringLiteral("banded"), QStringLiteral("basis"), QStringLiteral("celestial"),
                                    QStringLiteral("crop"), QStringLiteral("damask"), QStringLiteral("depth"), QStringLiteral("dividend"), QStringLiteral("droplet"),
                                    QStringLiteral("frame"), QStringLiteral("main event"), QStringLiteral("mesh"), QStringLiteral("metropolitan"), QStringLiteral("parallax"),
                                    QStringLiteral("parcel"), QStringLiteral("quotable"), QStringLiteral("savon"), QStringLiteral("slate"), QStringLiteral("view"),
                                    QStringLiteral("vapor trail"), QStringLiteral("wood type")};
        QStringList families = bundledFamilies();
        for (const QString &f : bundledFamilies()) families << interchangeFontName(f);   // (a font drawn to a standard font's widths goes by that name)
        QSet<QString> names, fontPairs;
        for (int i = 0; i < deck::builtinThemeCount(); ++i) {
            const QString name = deck::builtinThemeName(i);
            QVERIFY2(!name.isEmpty() && !names.contains(name), qPrintable(name));
            names.insert(name);
            QVERIFY2(!others.contains(name.toLower()), qPrintable(name));
            const deck::FontSet fonts = deck::builtinThemeFonts(i);
            QVERIFY2(families.contains(fonts.heading), qPrintable(name + ": " + fonts.heading + " is not a font that ships"));
            QVERIFY2(families.contains(fonts.body), qPrintable(name + ": " + fonts.body + " is not a font that ships"));
            const QString pair = fonts.heading + QLatin1Char('|') + fonts.body;
            QVERIFY2(!fontPairs.contains(pair), qPrintable(name + " repeats another theme's fonts"));
            fontPairs.insert(pair);
            const QVector<deck::Variant> variants = deck::builtinThemeVariants(i);
            QCOMPARE(variants.size(), 4);
            QSet<QString> looks;
            for (const deck::Variant &v : variants) {
                QVERIFY(v.background >= 0 && v.background < deck::kBackgroundStyles);
                QString look = QString::number(v.background);
                for (const QColor &c : v.colors.colors) {
                    QVERIFY(c.isValid());
                    look += c.name();
                }
                QVERIFY2(!looks.contains(look), qPrintable(name + " has two variants that are alike"));
                looks.insert(look);
            }
        }
        // The first is the presentation a new one starts from.
        QCOMPARE(deck::builtinThemeVariants(0).value(0).colors.name, deck::defaultTheme().name);
        for (int c = 0; c < 12; ++c) QCOMPARE(deck::builtinThemeVariants(0).value(0).colors.colors[c], deck::defaultTheme().colors[c]);
        QCOMPARE(deck::builtinThemeFonts(0).heading, deck::defaultTheme().headingFont);
        QCOMPARE(deck::builtinThemeFonts(0).body, deck::defaultTheme().bodyFont);
    }

    // A theme's presentation has one slide master and the nine standard layouts a new presentation has, in its colors and fonts;
    // the themes beyond the first draw something on the master, and every placeholder is on the slide.
    void themePresentationHasTheStandardLayouts()
    {
        auto standard = deck::newPresentation();
        QStringList types, names;
        for (const MasterPage *l : deck::layouts(*standard)) types << deck::layoutType(*l), names << l->name;
        for (int i = 0; i < deck::builtinThemeCount(); ++i) {
            for (int v = 0; v < 4; ++v) {
                auto d = deck::themePresentation(i, v);
                QVERIFY(d);
                const QString what = QStringLiteral("%1, variant %2").arg(deck::builtinThemeName(i)).arg(v + 1);
                QCOMPARE(d->pageSize(), QSizeF(deck::kSlideWidth, deck::kSlideHeight));
                const deck::DeckTheme theme = deck::themeOf(*d);
                QCOMPARE(theme.name, deck::builtinThemeName(i));
                QCOMPARE(theme.headingFont, deck::builtinThemeFonts(i).heading);
                QCOMPARE(theme.bodyFont, deck::builtinThemeFonts(i).body);
                const deck::Variant variant = deck::builtinThemeVariants(i).value(v);
                for (int c = 0; c < 10; ++c) QCOMPARE(theme.colors[c], variant.colors.colors[c]);   // (a hyperlink color may be made readable)
                QCOMPARE(d->fonts.heading, theme.headingFont);
                QCOMPARE(d->fonts.body, theme.bodyFont);
                int masters = 0;
                for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
                QCOMPARE(masters, 1);
                QStringList got, gotNames;
                for (const MasterPage *l : deck::layouts(*d)) got << deck::layoutType(*l), gotNames << l->name;
                QCOMPARE(got, types);
                QCOMPARE(gotNames, names);
                QCOMPARE(int(d->pages.size()), 1);
                QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[0])), QStringLiteral("title"));
                QCOMPARE(deck::slideTitle(*d, 0), deck::builtinThemeName(i));
                const QRectF page(QPointF(0, 0), d->pageSize());
                bool drawn = false;
                for (const auto &m : d->masters) {
                    for (const auto &it : m->items) {
                        if (it->role.isEmpty()) drawn = drawn || deck::isSlideMaster(*m);
                        else QVERIFY2(page.contains(it->rect), qPrintable(what + ": " + m->name + " " + it->role));
                    }
                    // A background in the variant's style.
                    if (deck::isSlideMaster(*m)) QCOMPARE(deck::backgroundStyleOf(m->background, theme), variant.background);
                }
                QVERIFY2(drawn == (i > 0), qPrintable(what + (i > 0 ? " draws nothing on its master" : " draws on its master")));
            }
        }
        // The first theme is a new presentation's own master and layouts, as they are.
        auto first = deck::themePresentation(0);
        QCOMPARE(first->masters.size(), standard->masters.size());
        for (int m = 0; m < standard->masters.size(); ++m) {
            QCOMPARE(first->masters[m]->items.size(), standard->masters[m]->items.size());
            for (size_t k = 0; k < standard->masters[m]->items.size(); ++k) QCOMPARE(first->masters[m]->items[k]->rect, standard->masters[m]->items[k]->rect);
        }
    }

    // The text of every placeholder reads against what is under it (WCAG AA: every run and bullet, links against every placeholder), in every
    // theme (the first one's footers and section text included) and variant, and on each of the twelve background styles in every variant's
    // colors: the text turns light on a dark background and dark on a light one, and so do hyperlinks.
    void themesAreReadable()
    {
        QStringList bad;
        for (int i = 0; i < deck::builtinThemeCount(); ++i)
            for (int v = 0; v < 4; ++v) {
                bad << unreadableText(*deck::themePresentation(i, v), QStringLiteral("%1, variant %2").arg(deck::builtinThemeName(i)).arg(v + 1));
                for (int style = 0; style < deck::kBackgroundStyles; ++style) {
                    auto d = deck::themePresentation(i, v);
                    deck::applyBackgroundStyle(*d, style);
                    bad << unreadableText(*d, QStringLiteral("%1, variant %2 on background style %3").arg(deck::builtinThemeName(i)).arg(v + 1).arg(style + 1));
                }
            }
        bad.removeDuplicates();
        QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 problems:\n").arg(bad.size()) + bad.mid(0, 12).join('\n')));
    }

    // Nine themes that look like nine: the title slides differ from each other, and a theme's variants differ from it.
    void themesLookDistinct()
    {
        auto picture = [](int theme, int variant) {
            auto d = deck::themePresentation(theme, variant);
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = d.get();
            ctx.cache = &cache;
            return Renderer::renderToImage(ctx, 0, 160.0 / d->pageSize().width()).convertToFormat(QImage::Format_RGB32);
        };
        auto difference = [](const QImage &a, const QImage &b) {
            if (a.size() != b.size()) return 255.0;
            double sum = 0;
            for (int y = 0; y < a.height(); ++y)
                for (int x = 0; x < a.width(); ++x) {
                    const QColor p = a.pixelColor(x, y), q = b.pixelColor(x, y);
                    sum += std::abs(p.red() - q.red()) + std::abs(p.green() - q.green()) + std::abs(p.blue() - q.blue());
                }
            return sum / (3.0 * a.width() * a.height());
        };
        QVector<QImage> first;
        for (int i = 0; i < deck::builtinThemeCount(); ++i) first << picture(i, 0);
        for (int i = 0; i < first.size(); ++i)
            for (int j = i + 1; j < first.size(); ++j)
                QVERIFY2(difference(first[i], first[j]) > 8, qPrintable(QStringLiteral("%1 and %2 look alike").arg(deck::builtinThemeName(i), deck::builtinThemeName(j))));
        // A variant on another background looks it; one in other colors looks different at least (its text colors differ, and the Variants gallery marks the accents).
        for (int i = 0; i < deck::builtinThemeCount(); ++i)
            for (int v = 1; v < 4; ++v) {
                const double d = difference(first[i], picture(i, v));
                if (deck::builtinThemeVariants(i)[v].background != deck::builtinThemeVariants(i)[0].background)
                    QVERIFY2(d > 4, qPrintable(QStringLiteral("%1 variant %2 looks like variant 1").arg(deck::builtinThemeName(i)).arg(v + 1)));
                else
                    QVERIFY2(d > 0, qPrintable(QStringLiteral("%1 variant %2 is variant 1").arg(deck::builtinThemeName(i)).arg(v + 1)));
            }
    }

    // The twelve background styles: the four colors flat, softly graded, and strongly graded; a background says which it is.
    void backgroundStylesAreTwelve()
    {
        QCOMPARE(deck::kBackgroundStyles, 12);
        for (int i = 0; i < deck::builtinThemeCount(); ++i) {
            const deck::DeckTheme theme = deck::themeOf(*deck::themePresentation(i));
            QSet<QString> seen;
            for (int s = 0; s < deck::kBackgroundStyles; ++s) {
                const Fill f = deck::backgroundStyle(theme, s);
                QVERIFY(f.type == Fill::Solid || f.type == Fill::Gradient);
                QCOMPARE(f.type == Fill::Gradient, s % 3 != 0);
                QCOMPARE(deck::backgroundStyleOf(f, theme), s);
                QCOMPARE(deck::isDarkBackground(f), deck::backgroundStyleIsDark(s));
                QCOMPARE(deck::backgroundStyleIsDark(s), s >= 6);
                QVERIFY(!deck::backgroundStyleName(s).isEmpty());
                const QString key = QString::fromUtf8(QJsonDocument(f.toJson()).toJson());
                QVERIFY2(!seen.contains(key), qPrintable(QStringLiteral("%1 style %2 repeats").arg(i).arg(s)));
                seen.insert(key);
            }
        }
        QCOMPARE(deck::backgroundStyleOf(Fill::solid(ColorRef::rgb(QColor(0x12, 0x34, 0x56))), deck::defaultTheme()), -1);
        QCOMPARE(deck::backgroundStyleOf(Fill(), deck::defaultTheme()), -1);
    }

    // Applying a theme to every slide: each slide goes to the layout of its own kind on the new slide master, its text and
    // hand-set looks stay, and what used the old theme's fonts and colors takes the new theme's.
    void applyingAThemeKeepsTheSlides()
    {
        auto d = deckToRestyle();
        const QStringList kinds = layoutTypes(*d);
        const QString was = d->pages[1]->items[0]->id;
        auto src = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor")));
        const deck::DeckTheme harbor = deck::themeOf(*src);
        QVERIFY(deck::applyThemeToAll(*d, *src));
        QCOMPARE(layoutTypes(*d), kinds);
        QCOMPARE(d->pages[1]->items[0]->id, was);   // the same objects, moved
        // One slide master and its nine layouts: the old ones are gone.
        int masters = 0, layoutCount = 0;
        for (const auto &m : d->masters) {
            masters += deck::isSlideMaster(*m);
            layoutCount += deck::isLayout(*m);
            if (deck::isLayout(*m)) QVERIFY(d->master(m->baseId) && deck::isSlideMaster(*d->master(m->baseId)));
        }
        QCOMPARE(masters, 1);
        QCOMPARE(layoutCount, 9);
        for (const auto &p : d->pages) QVERIFY(d->masterFor(*p));
        // The presentation's theme is the new one's, in every place it is kept.
        const deck::DeckTheme now = deck::themeOf(*d);
        QCOMPARE(now.name, QStringLiteral("Harbor"));
        for (int c = 0; c < 12; ++c) QCOMPARE(now.colors[c], harbor.colors[c]);
        QCOMPARE(d->fonts.heading, harbor.headingFont);
        QCOMPARE(d->fonts.body, harbor.bodyFont);
        office::Theme part;
        QVERIFY(office::readTheme(QByteArray::fromBase64(d->appData.value(QStringLiteral("themeXml")).toString().toLatin1()), &part));
        QCOMPARE(part.name, QStringLiteral("Harbor"));
        QCOMPARE(part.colors[4], harbor.colors[4]);
        // The words stay.
        QCOMPARE(deck::slideTitle(*d, 0), QStringLiteral("Annual review"));
        QCOMPARE(d->storyDoc(placeholderOn(*d->pages[0], QStringLiteral("subTitle"))->storyId)->toPlainText(), QStringLiteral("Finance team"));
        QCOMPARE(d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("obj"))->storyId)->toPlainText(), QStringLiteral("Sales rose\nCosts fell\nHand-colored"));
        QVERIFY(d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("obj"))->storyId)->begin().textList());
        // Placeholders stand where the new layouts put them.
        for (int i = 0; i < d->pages.size(); ++i) {
            const MasterPage &layout = *d->masterFor(*d->pages[i]);
            for (const auto &it : d->pages[i]->items) {
                if (it->role.isEmpty()) continue;
                bool found = false;
                for (const auto &l : layout.items)
                    if (l->role == it->role) {
                        found = true;
                        QCOMPARE(it->rect, l->rect);
                    }
                QVERIFY2(found, qPrintable(QStringLiteral("slide %1 %2").arg(i + 1).arg(it->role)));
            }
        }
        // Text that followed the old theme follows the new: the fonts and the colors of the layouts.
        const TextItem *title = placeholderOn(*d->pages[1], QStringLiteral("title"));
        const QTextCharFormat titleRun = firstRunOf(*d, *title);
        QCOMPARE(familyOf(titleRun), harbor.headingFont);
        const MasterPage &objLayout = *d->masterFor(*d->pages[1]);
        const Item *layoutTitle = nullptr;
        for (const auto &l : objLayout.items)
            if (l->role == title->role) layoutTitle = l.get();
        QVERIFY(layoutTitle);
        QCOMPARE(titleRun.fontPointSize(), firstRunOf(*d, *layoutTitle).fontPointSize());
        QCOMPARE(colorOfRun(*d, titleRun), colorOfRun(*d, firstRunOf(*d, *layoutTitle)));
        const QTextDocument *bodyDoc = d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("obj"))->storyId);
        QCOMPARE(familyOf(bodyDoc->begin().begin().fragment().charFormat()), harbor.bodyFont);
        // What was set by hand stays: the red bold line keeps its color and weight (and takes the body font).
        const QTextCharFormat hand = bodyDoc->findBlockByNumber(2).begin().fragment().charFormat();
        QCOMPARE(colorOfRun(*d, hand), QColor(0xC0, 0x00, 0x00));
        QCOMPARE(hand.fontWeight(), int(QFont::Bold));
        QCOMPARE(familyOf(hand), harbor.bodyFont);
        // Objects that used the old theme's colors take the new ones; a font of its own stays.
        const auto *accent = static_cast<const TextItem *>(itemNamed(*d->pages[3], QStringLiteral("accent text")));
        QCOMPARE(colorOfRun(*d, firstRunOf(*d, *accent)), harbor.colors[4]);
        QCOMPARE(familyOf(firstRunOf(*d, *accent)), harbor.bodyFont);
        const auto *own = static_cast<const TextItem *>(itemNamed(*d->pages[3], QStringLiteral("own font")));
        QCOMPARE(familyOf(firstRunOf(*d, *own)), QStringLiteral("Courier Prime"));
        const Item *shape = itemNamed(*d->pages[3], QStringLiteral("accent shape"));
        QCOMPARE(shape->fill.color.rgbValue(), harbor.colors[5]);
        QCOMPARE(shape->stroke.color.rgbValue(), harbor.colors[4]);
    }

    // A slide whose background was set by hand keeps it; a slide on a layout the theme has no kind for goes to the layout its
    // placeholders ask for.
    void applyingAThemeKeepsHandSetBackgrounds()
    {
        auto d = deckToRestyle();
        d->pages[2]->background = Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99)));
        auto custom = std::make_shared<MasterPage>(*deck::layoutOfType(*d, QStringLiteral("obj")));
        custom->id = QStringLiteral("LX");
        custom->name = QStringLiteral("Only mine");
        custom->props[QStringLiteral("type")] = QStringLiteral("cust");
        d->masters << custom;
        d->pages[3]->masterId = custom->id;   // a title and nothing else asked of it
        QVERIFY(deck::applyThemeToAll(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Ember")))));
        QCOMPARE(d->pages[2]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[3])), QStringLiteral("obj"));
        QVERIFY(!d->master(QStringLiteral("LX")));
        // Nothing to take: a presentation with no slide master is left as it is.
        Document empty;
        empty.masters.clear();
        auto before = deckToRestyle();
        QVERIFY(!deck::applyThemeToAll(*before, empty));
        QCOMPARE(deck::themeOf(*before).name, deck::defaultTheme().name);
    }

    // Applying a theme to some slides gives those slides a slide master of their own (a copy of the theme's), and leaves the others
    // and the presentation's theme alone; all slides is the same as every slide.
    void applyingAThemeToSelectedSlidesGivesThemANewMaster()
    {
        auto d = deckToRestyle();
        const QStringList kinds = layoutTypes(*d);
        const QString firstMaster = deck::slideMasterOf(*d, 0);
        auto src = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Tideline")));
        QVERIFY(deck::applyThemeToSlides(*d, *src, {1, 2}));
        QCOMPARE(layoutTypes(*d), kinds);
        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 2);
        QCOMPARE(deck::slideMasterOf(*d, 0), firstMaster);
        QCOMPARE(deck::slideMasterOf(*d, 3), firstMaster);
        QVERIFY(deck::slideMasterOf(*d, 1) != firstMaster);
        QCOMPARE(deck::slideMasterOf(*d, 1), deck::slideMasterOf(*d, 2));
        QCOMPARE(deck::themeOf(*d).name, deck::defaultTheme().name);
        QCOMPARE(d->masters[0]->id, firstMaster);   // the presentation's theme is still the first master's
        const MasterPage *second = d->master(deck::slideMasterOf(*d, 1));
        office::Theme part;
        QVERIFY(office::readTheme(QByteArray::fromBase64(second->props.value(QStringLiteral("themeXml")).toString().toLatin1()), &part));
        QCOMPARE(part.name, QStringLiteral("Tideline"));
        QCOMPARE(part.majorLatin, deck::builtinThemeFonts(deck::builtinThemeIndex(QStringLiteral("Tideline"))).heading);
        // The slides on it are set in its fonts, the others in the old ones.
        QCOMPARE(familyOf(firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("title")))), part.majorLatin);
        QCOMPARE(familyOf(firstRunOf(*d, *placeholderOn(*d->pages[0], QStringLiteral("ctrTitle")))), deck::defaultTheme().headingFont);
        // Trying one theme after another on the same slides leaves no master behind for each, and the presentation's own is never taken.
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Ember"))), {1, 2}));
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Marquee"))), {1, 2}));
        masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 2);
        for (const auto &m : d->masters) QVERIFY(m->baseId.isEmpty() || d->master(m->baseId));
        QCOMPARE(d->master(deck::slideMasterOf(*d, 1))->name, QStringLiteral("Marquee Slide Master"));
        QCOMPARE(deck::slideMasterOf(*d, 0), firstMaster);
        // Some slides on one of them: a master with slides left on it stays.
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Orchard"))), {1}));
        masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 3);
        // Every slide chosen is every slide.
        auto all = deckToRestyle();
        QVERIFY(deck::applyThemeToSlides(*all, *src, {0, 1, 2, 3, 4}));
        masters = 0;
        for (const auto &m : all->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        QCOMPARE(deck::themeOf(*all).name, QStringLiteral("Tideline"));
    }

    // A theme can be taken from a file: a presentation saved with the theme gives it to another.
    void applyingAThemeFromAFile()
    {
        QString err;
        auto src = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Almanac")), 1);
        const QByteArray bytes = deck::exportPresentation(*src, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto fromFile = deck::importPresentation(bytes, &err);
        QVERIFY2(fromFile, qPrintable(err));
        auto d = deckToRestyle();
        QVERIFY(deck::applyThemeToAll(*d, *fromFile));
        const deck::DeckTheme theme = deck::themeOf(*d), wanted = deck::themeOf(*src);
        QCOMPARE(theme.name, QStringLiteral("Almanac"));
        for (int c = 0; c < 12; ++c) QCOMPARE(theme.colors[c], wanted.colors[c]);
        QCOMPARE(theme.headingFont, wanted.headingFont);
        QCOMPARE(layoutTypes(*d), layoutTypes(*deckToRestyle()));
        const MasterPage *master = d->master(deck::slideMasterOf(*d, 0));
        const MasterPage *was = src->master(deck::slideMasterOf(*src, 0));
        int drawn = 0, drawnWas = 0;
        for (const auto &it : master->items) drawn += it->role.isEmpty();
        for (const auto &it : was->items) drawnWas += it->role.isEmpty();
        QVERIFY(drawn > 0);
        QCOMPARE(drawn, drawnWas);
        QCOMPARE(deck::backgroundStyleOf(master->background, theme), deck::builtinThemeVariants(deck::builtinThemeIndex(QStringLiteral("Almanac"))).value(1).background);
    }

    // The colors, fonts, and background of a presentation change as the Variants menu says.
    void variantsChangeColorsFontsAndBackgrounds()
    {
        auto d = deckToRestyle();
        const deck::DeckTheme was = deck::themeOf(*d);
        // Colors: the old scheme's colors become the new scheme's wherever they are used; what was set by hand stays.
        deck::ColorSet set = deck::colorSets().value(5);
        QVERIFY(!set.name.isEmpty());
        deck::applyColors(*d, set);
        const deck::DeckTheme now = deck::themeOf(*d);
        for (int c = 0; c < 10; ++c) QCOMPARE(now.colors[c], set.colors[c]);   // (hyperlink colors are made readable on the background)
        QCOMPARE(now.name, was.name);
        QCOMPARE(now.headingFont, was.headingFont);
        const auto *accent = static_cast<const TextItem *>(itemNamed(*d->pages[3], QStringLiteral("accent text")));
        followsAccent(colorOfRun(*d, firstRunOf(*d, *accent)), set.colors[4], colorsUnder(*d, *d->masterFor(*d->pages[3]), accent->rect.center()), 4.5, "accent text");
        QCOMPARE(itemNamed(*d->pages[3], QStringLiteral("accent shape"))->fill.color.rgbValue(), set.colors[5]);
        QCOMPARE(colorOfRun(*d, firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("title")))), set.colors[0]);
        const QTextDocument *bodyDoc = d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("obj"))->storyId);
        QCOMPARE(colorOfRun(*d, bodyDoc->findBlockByNumber(2).begin().fragment().charFormat()), QColor(0xC0, 0x00, 0x00));
        QCOMPARE(deck::colorSetOf(*d).colors[4], set.colors[4]);
        // The layouts and the slide master follow, so a new slide does.
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        QCOMPARE(colorOfRun(*d, firstRunOf(*d, *placeholderOn(*d->pages.last(), QStringLiteral("title")))), set.colors[0]);

        // Fonts.
        deck::FontSet fonts{QStringLiteral("Test"), QStringLiteral("Poppins"), QStringLiteral("Lora")};
        deck::applyFonts(*d, fonts);
        QCOMPARE(deck::themeOf(*d).headingFont, QStringLiteral("Poppins"));
        QCOMPARE(deck::themeOf(*d).bodyFont, QStringLiteral("Lora"));
        QCOMPARE(d->fonts.heading, QStringLiteral("Poppins"));
        QCOMPARE(familyOf(firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("title")))), QStringLiteral("Poppins"));
        QCOMPARE(familyOf(firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("obj")))), QStringLiteral("Lora"));
        QCOMPARE(familyOf(firstRunOf(*d, *accent)), QStringLiteral("Lora"));
        QCOMPARE(familyOf(firstRunOf(*d, *static_cast<const TextItem *>(itemNamed(*d->pages[3], QStringLiteral("own font"))))), QStringLiteral("Courier Prime"));
        QCOMPARE(deck::fontSetOf(*d).heading, QStringLiteral("Poppins"));

        // A dark background: the master's, with the slides' and layouts' own cleared; text turns light, the hand-set red stays red.
        d->pages[2]->background = Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99)));
        deck::applyBackgroundStyle(*d, 11);
        const deck::DeckTheme dark = deck::themeOf(*d);
        QCOMPARE(deck::backgroundStyleOf(d->master(deck::slideMasterOf(*d, 0))->background, dark), 11);
        QVERIFY(d->pages[2]->background.type == Fill::NoFill);
        QCOMPARE(colorOfRun(*d, firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("title")))), dark.colors[1]);
        QCOMPARE(colorOfRun(*d, firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("obj")))), dark.colors[1]);
        {
            // (A dark red that read on white does not on near black: it keeps its hue and goes lighter until it reads.)
            const QColor red = colorOfRun(*d, d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("obj"))->storyId)->findBlockByNumber(2).begin().fragment().charFormat());
            QVERIFY(red.hslHue() <= 5 || red.hslHue() >= 355);
            for (const QColor &bg : colorsUnder(*d, *d->masterFor(*d->pages[1]), placeholderOn(*d->pages[1], QStringLiteral("obj"))->rect.center())) QVERIFY(deck::contrastRatio(red, bg) >= 4.5);
        }
        const QStringList bad = unreadableText(*d, QStringLiteral("dark"));
        QVERIFY2(bad.isEmpty(), qPrintable(bad.mid(0, 8).join('\n')));
        // And back to a light one.
        deck::applyBackgroundStyle(*d, 0);
        QCOMPARE(colorOfRun(*d, firstRunOf(*d, *placeholderOn(*d->pages[1], QStringLiteral("obj")))), deck::themeOf(*d).colors[0]);
        // A variant is colors and a background at once; the presentation says which it shows.
        auto harbor = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor")));
        const QVector<deck::Variant> variants = deck::variantsOf(*harbor);
        QCOMPARE(variants.size(), 4);
        QCOMPARE(deck::currentVariant(*harbor), 0);
        for (int v = 0; v < 4; ++v) {
            auto copy = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor")));
            deck::applyVariant(*copy, variants[v]);
            QCOMPARE(deck::currentVariant(*copy), v);
            auto direct = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor")), v);
            QCOMPARE(deck::themeOf(*copy).colors[4], deck::themeOf(*direct).colors[4]);
            QCOMPARE(copy->masters[0]->background.toJson(), direct->masters[0]->background.toJson());
        }
        // A theme that isn't a built-in one has its own colors on four backgrounds.
        auto plain = deck::newPresentation();
        deck::DeckTheme mine = deck::themeOf(*plain);
        mine.name = QStringLiteral("Mine");
        deck::setTheme(*plain, mine);
        const QVector<deck::Variant> others = deck::variantsOf(*plain);
        QCOMPARE(others.size(), 4);
        for (const deck::Variant &v : others) QCOMPARE(v.colors.colors[4], mine.colors[4]);
        QCOMPARE(deck::currentVariant(*plain), 0);
    }

    // The colors of hyperlinks are kept readable when a background turns dark or light.
    void hyperlinkColorsStayReadable()
    {
        auto d = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Ember")));
        deck::applyBackgroundStyle(*d, 0);   // Ember is dark; its hyperlink color is a light one
        const deck::DeckTheme theme = deck::themeOf(*d);
        const Fill bg = d->masters[0]->background;
        for (const QColor &c : fillColors(bg, d->colors)) {
            QVERIFY(deck::contrastRatio(theme.colors[10], c) >= 4.5);
            QVERIFY(deck::contrastRatio(theme.colors[11], c) >= 4.5);
        }
        QCOMPARE(deck::contrastRatio(QColor(Qt::black), QColor(Qt::white)), 21.0);
    }

    // A theme is saved as slide master, layout, and theme parts, and read back unchanged: its colors, fonts, background, what
    // the master draws, and the look of every placeholder; a second save of what was read is the same.
    void appliedThemesAreSavedAndReadBack()
    {
        for (int i = 1; i < deck::builtinThemeCount(); ++i) {
            const QString name = deck::builtinThemeName(i);
            auto d = deckToRestyle();
            QVERIFY(deck::applyThemeToAll(*d, *deck::themePresentation(i, i % 4)));
            QString err;
            const QByteArray bytes = deck::exportPresentation(*d, &err);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            Exported e;
            QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(name + ": " + packageProblem(e)));
            auto back = deck::importPresentation(bytes, &err);
            QVERIFY2(back, qPrintable(err));
            // The theme part.
            office::Theme part;
            QVERIFY2(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &part, &err), qPrintable(err));
            const deck::DeckTheme theme = deck::themeOf(*d), got = deck::themeOf(*back);
            QCOMPARE(part.name, name);
            QCOMPARE(got.name, name);
            for (int c = 0; c < 12; ++c) QCOMPARE(got.colors[c], theme.colors[c]);
            QCOMPARE(got.headingFont, theme.headingFont);
            QCOMPARE(got.bodyFont, theme.bodyFont);
            // The masters and layouts.
            QCOMPARE(back->masters.size(), d->masters.size());
            QCOMPARE(layoutTypes(*back), layoutTypes(*d));
            for (int m = 0; m < d->masters.size(); ++m) {
                const MasterPage &a = *d->masters[m], &b = *back->masters[m];
                const QString where = QStringLiteral("%1: %2").arg(name, a.name);
                QCOMPARE(deck::isSlideMaster(b), deck::isSlideMaster(a));
                QCOMPARE(deck::layoutType(b), deck::layoutType(a));
                QCOMPARE(b.showMasterItems, a.showMasterItems);
                if (deck::isSlideMaster(a)) {
                    QCOMPARE(deck::backgroundStyleOf(b.background, got), deck::backgroundStyleOf(a.background, theme));
                    QVERIFY2(deck::backgroundStyleOf(b.background, got) >= 0, qPrintable(where));
                }
                // What it draws: the same shapes in the same places and colors.
                QVector<const Item *> drawnA, drawnB;
                for (const auto &it : a.items) if (it->role.isEmpty()) drawnA << it.get();
                for (const auto &it : b.items) if (it->role.isEmpty()) drawnB << it.get();
                QCOMPARE(drawnB.size(), drawnA.size());
                for (int k = 0; k < drawnA.size(); ++k) {
                    QVERIFY2(qAbs(drawnA[k]->rect.x() - drawnB[k]->rect.x()) < 0.6 && qAbs(drawnA[k]->rect.y() - drawnB[k]->rect.y()) < 0.6
                                 && qAbs(drawnA[k]->rect.width() - drawnB[k]->rect.width()) < 0.6 && qAbs(drawnA[k]->rect.height() - drawnB[k]->rect.height()) < 0.6,
                             qPrintable(where + QStringLiteral(" shape %1").arg(k)));
                    QCOMPARE(drawnB[k]->fill.type, drawnA[k]->fill.type);
                    if (drawnA[k]->fill.type == Fill::Solid) {
                        QCOMPARE(drawnB[k]->fill.color.resolve(back->colors), drawnA[k]->fill.color.resolve(d->colors));
                        QVERIFY(qAbs(drawnB[k]->fill.transparency - drawnA[k]->fill.transparency) < 0.01);
                    }
                    QCOMPARE(drawnB[k]->stroke.isNone(), drawnA[k]->stroke.isNone());
                    if (!drawnA[k]->stroke.isNone()) QCOMPARE(drawnB[k]->stroke.color.resolve(back->colors), drawnA[k]->stroke.color.resolve(d->colors));
                    QCOMPARE(drawnB[k]->flipH, drawnA[k]->flipH);
                    QCOMPARE(drawnB[k]->flipV, drawnA[k]->flipV);
                }
                // Each placeholder: where it is and how its text looks.
                for (const auto &it : a.items) {
                    if (it->role.isEmpty() || it->type() != ItemType::Text || deck::placeholderType(*it) == QLatin1String("pic")) continue;
                    const Item *other = nullptr;
                    for (const auto &o : b.items)
                        if (o->role == it->role) other = o.get();
                    QVERIFY2(other, qPrintable(where + " " + it->role));
                    QVERIFY2(qAbs(it->rect.x() - other->rect.x()) < 0.6 && qAbs(it->rect.y() - other->rect.y()) < 0.6 && qAbs(it->rect.width() - other->rect.width()) < 0.6,
                             qPrintable(where + " " + it->role));
                    const QTextCharFormat fa = firstRunOf(*d, *it), fb = firstRunOf(*back, *other);
                    QCOMPARE(fb.fontPointSize(), fa.fontPointSize());
                    QCOMPARE(colorOfRun(*back, fb), colorOfRun(*d, fa));
                    QVERIFY2(familyOf(fb).isEmpty() || familyOf(fb) == familyOf(fa), qPrintable(where + " " + it->role + ": " + familyOf(fb) + " for " + familyOf(fa)));
                    QCOMPARE(fb.fontWeight() >= QFont::DemiBold, fa.fontWeight() >= QFont::DemiBold);
                    QCOMPARE(fb.fontCapitalization(), fa.fontCapitalization());
                }
            }
            // The slides stay on the layouts of their kinds, with their words.
            QCOMPARE(int(back->pages.size()), int(d->pages.size()));
            QCOMPARE(deck::slideTitle(*back, 1), QStringLiteral("Results"));
            // Saving what was read, and reading that, and saving again gives the same masters, layouts, and theme (a read adds the language and
            // kerning a file says of its text, so the first save of a file read differs from the first save).
            Exported again, third;
            QVERIFY2(unpack(deck::exportPresentation(*back, &err), &again, &err), qPrintable(err));
            auto backAgain = deck::importPresentation(deck::exportPresentation(*back, &err), &err);
            QVERIFY2(backAgain, qPrintable(err));
            QVERIFY2(unpack(deck::exportPresentation(*backAgain, &err), &third, &err), qPrintable(err));
            for (const QString &partName : again.zip.keys())
                if (partName.contains(QLatin1String("slideMasters/slideMaster")) || partName.contains(QLatin1String("slideLayouts/slideLayout")) || partName.contains(QLatin1String("theme/theme1")))
                {
                    const QByteArray was = again.zip.value(partName), now = third.zip.value(partName);
                    int at = 0;
                    while (at < was.size() && at < now.size() && was[at] == now[at]) ++at;
                    QVERIFY2(was == now, qPrintable(name + ": " + partName + " changed on a third save, at " + QString::number(at) + ": " + QString::fromUtf8(was.mid(std::max(0, at - 80), 200)) + " <<>> "
                                                    + QString::fromUtf8(now.mid(std::max(0, at - 80), 200))));
                }
        }
    }

    // Slides with a master of their own are saved with its theme part and come back on it.
    void aThemeOnSomeSlidesIsSavedAsAMasterOfItsOwn()
    {
        auto d = deckToRestyle();
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Orchard"))), {1}));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QCOMPARE(e.pkg.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/slideMaster")).size(), 2);
        office::Theme second;
        QVERIFY(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme2.xml")), &second, &err));
        QCOMPARE(second.name, QStringLiteral("Orchard"));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(deck::themeOf(*back).name, deck::defaultTheme().name);
        QCOMPARE(layoutTypes(*back), layoutTypes(*d));
        QVERIFY(deck::slideMasterOf(*back, 1) != deck::slideMasterOf(*back, 0));
        QCOMPARE(deck::slideMasterOf(*back, 2), deck::slideMasterOf(*back, 0));
        QVERIFY(deck::applyThemeToAll(*back, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Marquee")))));
        QCOMPARE(deck::themeOf(*back).name, QStringLiteral("Marquee"));
    }

    // The theme part of a file is kept as the file had it when a theme is taken from it, and is the part written when the presentation is saved.
    void applyingAThemeKeepsThePartOfAFile()
    {
        QString err;
        auto source = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(source, qPrintable(err));
        auto d = deckToRestyle();
        QVERIFY(deck::applyThemeToAll(*d, *source));
        QCOMPARE(QByteArray::fromBase64(d->appData.value(QStringLiteral("themeXml")).toString().toLatin1()), themeXml());
        QCOMPARE(deck::themeOf(*d).name, deck::themeOf(*source).name);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QCOMPARE(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), themeXml());
        // Changing a color changes the part: the colors it says, and nothing else.
        deck::ColorSet set = deck::colorSetOf(*d);
        set.colors[4] = QColor(0x12, 0x34, 0x56);
        deck::applyColors(*d, set);
        office::Theme part;
        QVERIFY(office::readTheme(QByteArray::fromBase64(d->appData.value(QStringLiteral("themeXml")).toString().toLatin1()), &part));
        QCOMPARE(part.colors[4], QColor(0x12, 0x34, 0x56));
        QCOMPARE(part.majorLatin, deck::themeOf(*source).headingFont);
        QCOMPARE(part.minorLatin, deck::themeOf(*source).bodyFont);
    }

    // A file can't make a change of theme slow: slides and a theme whose layout have hundreds of placeholders each are not matched one
    // against the other (the slides go to the layout as they are).
    void applyingAThemeToASlideFullOfPlaceholdersIsQuick()
    {
        auto d = deck::newPresentation();
        auto many = [](Document &doc, int count) {
            ItemList items;
            for (int i = 0; i < count; ++i) {
                auto box = std::make_shared<TextItem>();
                box->role = deck::placeholderRole(QStringLiteral("obj"), i + 1);
                box->rect = QRectF(10 + i % 40 * 20, 10 + i / 40 * 5, 18, 4);
                box->storyId = doc.createStory(QStringLiteral("x"));
                items.push_back(box);
            }
            return items;
        };
        // Four slides of 3,000 shapes each, made new for every run (the theme goes onto them once).
        const auto fresh = [&] {
            d = deck::newPresentation();
            for (int s = 0; s < 4; ++s) {
                deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
                Page &slide = *d->pages.last();
                const ItemList more = many(*d, 3000);
                slide.items.insert(slide.items.end(), more.begin(), more.end());
            }
        };
        auto source = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Granite")));
        MasterPage *obj = deck::layoutOfType(*source, QStringLiteral("obj"));
        obj->items = many(*source, 3000);
        const qint64 themeMs = bestNanoseconds([&] { QVERIFY(deck::applyThemeToAll(*d, *source)); }, 3, fresh) / 1000000;
        QVERIFY2(themeMs < 5000, qPrintable(QStringLiteral("took %1 ms").arg(themeMs)));
        QCOMPARE(deck::themeOf(*d).name, QStringLiteral("Granite"));
        QCOMPARE(layoutTypes(*d), (QStringList{QStringLiteral("title"), QStringLiteral("obj"), QStringLiteral("obj"), QStringLiteral("obj"), QStringLiteral("obj")}));
    }

    // A grid of pictures: the arrow keys move between them, Enter chooses, and it scrolls by rows.
    void pictureGridMovesAndChooses()
    {
        PictureGrid g(60, 3, true);
        QVector<PictureGrid::Tile> tiles;
        for (int i = 0; i < 7; ++i) tiles << PictureGrid::Tile{QStringLiteral("Tile %1").arg(i + 1), {}};
        g.setTiles(tiles);
        g.setPictures([](int tile, int width, qreal) {
            QImage img(width, width * 9 / 16, QImage::Format_RGB32);
            img.fill(QColor::fromHsv(tile * 40, 200, 200));
            return img;
        });
        QCOMPARE(g.count(), 7);
        QCOMPARE(g.rows(), 3);
        QCOMPARE(g.nameAt(4), QStringLiteral("Tile 5"));
        QVERIFY(g.tileRect(0).isValid() && g.tileRect(6).isValid());
        QCOMPARE(g.tileAt(g.tileRect(4).center()), 4);
        QVERIFY(g.tileRect(1).left() > g.tileRect(0).left());
        QVERIFY(g.tileRect(3).top() > g.tileRect(0).top());
        g.setCurrent(2);
        QCOMPARE(g.current(), 2);
        QSignalSpy chosen(&g, &PictureGrid::chosen);
        QTest::keyClick(&g, Qt::Key_Home);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Down);
        QTest::keyClick(&g, Qt::Key_Return);
        QCOMPARE(chosen.count(), 1);
        QCOMPARE(chosen.first().first().toInt(), 4);
        QTest::keyClick(&g, Qt::Key_End);
        QTest::keyClick(&g, Qt::Key_Space);
        QCOMPARE(chosen.last().first().toInt(), 6);
        // One row at a time in view: the others are out of it until scrolled to, and keys bring them in.
        g.setVisibleRows(1);
        QCOMPARE(g.visibleRows(), 1);
        QVERIFY(g.tileRect(0).isEmpty() == (g.firstRow() != 0));
        g.setFirstRow(0);
        QVERIFY(!g.tileRect(0).isEmpty());
        QVERIFY(g.tileRect(3).isEmpty());
        QTest::keyClick(&g, Qt::Key_Home);
        QTest::keyClick(&g, Qt::Key_Down);
        QCOMPARE(g.firstRow(), 1);
        QVERIFY(!g.tileRect(3).isEmpty() && g.tileRect(0).isEmpty());
        g.setFirstRow(99);
        QCOMPARE(g.firstRow(), 2);
        // The picture of a tile is drawn when first needed.
        int asked = 0;
        g.setPictures([&asked](int, int width, qreal) {
            ++asked;
            return QImage(width, width * 9 / 16, QImage::Format_RGB32);
        });
        g.reload();
        QCOMPARE(asked, 0);
        QImage shot(g.sizeHint(), QImage::Format_RGB32);
        shot.fill(Qt::white);
        g.render(&shot);
        QCOMPARE(asked, 1);   // the one row in view
    }

    // The Themes gallery shows the nine themes as pictures of their title slides, marks the presentation's, and offers each its menu.
    void themeGalleryShowsEachTheme()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        ThemeGallery g(ed);
        QCOMPARE(g.grid()->count(), 9);
        QCOMPARE(g.grid()->nameAt(0), QStringLiteral("JeffOffice"));
        QCOMPARE(g.grid()->nameAt(8), deck::builtinThemeName(8));
        QCOMPARE(g.currentTheme(), 0);
        QCOMPARE(g.grid()->visibleRows(), 1);
        QCOMPARE(g.grid()->columns(), 4);
        // The pictures are the themes' title slides, each its own.
        QSet<QByteArray> seen;
        for (int i = 0; i < 9; ++i) {
            const QImage pic = themePicture(i, 0, 78, 1);
            QCOMPARE(pic.width(), 78);
            QVERIFY(pic.height() == 43 || pic.height() == 44);
            QByteArray bytes(reinterpret_cast<const char *>(pic.constBits()), pic.sizeInBytes());
            QVERIFY(!seen.contains(bytes));
            seen.insert(bytes);
        }
        QVERIFY(themePicture(1, 0, 156, 1).width() == 156);
        // A click, or Enter, chooses one for every slide.
        QSignalSpy chosen(&g, &ThemeGallery::themeChosen);
        QTest::keyClick(g.grid(), Qt::Key_Home);
        QTest::keyClick(g.grid(), Qt::Key_Right);
        QTest::keyClick(g.grid(), Qt::Key_Return);
        QCOMPARE(chosen.count(), 1);
        QCOMPARE(chosen.first().at(0).toInt(), 1);
        QCOMPARE(chosen.first().at(1).toBool(), true);
        g.grid()->resize(g.grid()->sizeHint());
        QTest::mouseClick(g.grid(), Qt::LeftButton, {}, g.grid()->tileRect(2).center());
        QCOMPARE(chosen.last().at(0).toInt(), 2);
        // The right-click menu.
        QSignalSpy browse(&g, &ThemeGallery::browseRequested);
        std::unique_ptr<QMenu> menu(g.contextMenu(3));
        QStringList texts;
        for (QAction *a : menu->actions())
            if (!a->isSeparator()) texts << a->text();
        QCOMPARE(texts, (QStringList{QStringLiteral("Apply to All Slides"), QStringLiteral("Apply to Selected Slides"), QStringLiteral("Browse for Themes…")}));
        menu->actions().at(0)->trigger();
        QCOMPARE(chosen.last().at(0).toInt(), 3);
        QCOMPARE(chosen.last().at(1).toBool(), true);
        menu->actions().at(1)->trigger();
        QCOMPARE(chosen.last().at(0).toInt(), 3);
        QCOMPARE(chosen.last().at(1).toBool(), false);
        menu->actions().at(3)->trigger();
        QCOMPARE(browse.count(), 1);
        // All of them in a popup, with the same menu, and Browse for Themes at the foot.
        QPointer<QFrame> all = g.openAll();
        QVERIFY(all);
        auto *grid = all->findChild<PictureGrid *>();
        QVERIFY(grid);
        QCOMPARE(grid->count(), 9);
        QCOMPARE(grid->rows(), 3);
        QCOMPARE(grid->current(), 0);
        auto *footer = all->findChild<QPushButton *>();
        QVERIFY(footer && footer->text().contains(QStringLiteral("Browse")));
        footer->click();
        QCOMPARE(browse.count(), 2);
        QTRY_VERIFY(!all || !all->isVisible());
    }

    // Choosing a theme in the Design tab's gallery applies it to every slide as one step, and the gallery marks it.
    void designTabAppliesAThemeInOneStep()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.show();
        auto *gallery = w.findChild<ThemeGallery *>();
        QVERIFY(gallery);
        w.newSlide(QStringLiteral("twoObj"));
        w.newSlide(QStringLiteral("titleOnly"));
        sayIn(*ed->doc(), *placeholderOn(*ed->doc()->pages[1], QStringLiteral("title")), QStringLiteral("Two"));
        const QStringList kinds = layoutTypes(*ed->doc());
        const int harbor = deck::builtinThemeIndex(QStringLiteral("Harbor"));
        const QString firstMaster = deck::slideMasterOf(*ed->doc(), 0);
        QTest::keyClick(gallery->grid(), Qt::Key_Home);
        for (int i = 0; i < harbor; ++i) QTest::keyClick(gallery->grid(), Qt::Key_Right);
        QTest::keyClick(gallery->grid(), Qt::Key_Return);
        QCOMPARE(deck::themeOf(*ed->doc()).name, QStringLiteral("Harbor"));
        QCOMPARE(layoutTypes(*ed->doc()), kinds);
        QCOMPARE(deck::slideTitle(*ed->doc(), 1), QStringLiteral("Two"));
        QVERIFY(deck::slideMasterOf(*ed->doc(), 0) != firstMaster);
        QTRY_COMPARE(gallery->currentTheme(), harbor);
        // One step undoes all of it, and redoes.
        ed->undo();
        QCOMPARE(deck::themeOf(*ed->doc()).name, deck::defaultTheme().name);
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 0), firstMaster);
        QCOMPARE(layoutTypes(*ed->doc()), kinds);
        QCOMPARE(deck::slideTitle(*ed->doc(), 1), QStringLiteral("Two"));
        QTRY_COMPARE(gallery->currentTheme(), 0);
        QCOMPARE(ed->doc()->fonts.heading, deck::defaultTheme().headingFont);
        ed->redo();
        QCOMPARE(deck::themeOf(*ed->doc()).name, QStringLiteral("Harbor"));
        QCOMPARE(ed->doc()->fonts.heading, deck::builtinThemeFonts(harbor).heading);
        ed->undo();
        // For the selected slides only: the slide being edited, or the Slide Sorter's selection.
        ed->setCurrentPage(1);
        w.applyTheme(deck::builtinThemeIndex(QStringLiteral("Granite")), false);
        QVERIFY(deck::slideMasterOf(*ed->doc(), 1) != firstMaster);
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 0), firstMaster);
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 2), firstMaster);
        QCOMPARE(deck::themeOf(*ed->doc()).name, deck::defaultTheme().name);
        ed->undo();
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 1), firstMaster);
        w.setView(DeckWindow::View::Sorter);
        w.slideSorter()->select({1, 2}, 2);
        w.applyTheme(deck::builtinThemeIndex(QStringLiteral("Orchard")), false);
        QVERIFY(deck::slideMasterOf(*ed->doc(), 1) != firstMaster);
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 1), deck::slideMasterOf(*ed->doc(), 2));
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 0), firstMaster);
        ed->undo();
        QCOMPARE(deck::slideMasterOf(*ed->doc(), 2), firstMaster);
    }

    // Browse for Themes takes the theme of another presentation file (a .pptx or .potx), and says so when a file has none to take.
    void designTabTakesTheThemeOfAFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        const QByteArray bytes = deck::exportPresentation(*deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Marquee")), 3), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QFile file(dir.filePath(QStringLiteral("marquee.potx")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(bytes);
        file.close();
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        QVERIFY(w.applyThemeFile(file.fileName(), true));
        QCOMPARE(deck::themeOf(*ed->doc()).name, QStringLiteral("Marquee"));
        QCOMPARE(layoutTypes(*ed->doc()), (QStringList{QStringLiteral("title"), QStringLiteral("obj")}));
        const deck::Variant v = deck::builtinThemeVariants(deck::builtinThemeIndex(QStringLiteral("Marquee"))).value(3);
        QCOMPARE(deck::themeOf(*ed->doc()).colors[4], v.colors.colors[4]);
        QCOMPARE(deck::backgroundStyleOf(ed->doc()->masters[0]->background, deck::themeOf(*ed->doc())), v.background);
        ed->undo();
        QCOMPARE(deck::themeOf(*ed->doc()).name, deck::defaultTheme().name);
    }

    // A theme taken from a presentation whose slide master has no layouts is refused before anything changes: the presentation keeps its
    // layouts and slides, whether the theme is for all slides or some, from a document or from a file (which says why, in a message).
    void aThemeWithoutLayoutsIsRefused()
    {
        auto source = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor")));
        auto bare = std::make_unique<Document>();
        bare->fromJson(source->toJson());
        QVERIFY(deck::layouts(*bare).size() > 0);
        for (auto it = bare->masters.begin(); it != bare->masters.end();)
            it = deck::isLayout(**it) ? bare->masters.erase(it) : it + 1;
        QCOMPARE(deck::layouts(*bare).size(), 0);
        QVERIFY(deck::isSlideMaster(firstMasterOf(*bare)));
        // Nothing of the presentation changes.
        auto d = deckToRestyle();
        const QByteArray before = QJsonDocument(d->toJson()).toJson(QJsonDocument::Compact);
        QVERIFY(!deck::applyThemeToAll(*d, *bare));
        QCOMPARE(QJsonDocument(d->toJson()).toJson(QJsonDocument::Compact), before);
        QVERIFY(!deck::applyThemeToSlides(*d, *bare, {1, 2}));
        QCOMPARE(QJsonDocument(d->toJson()).toJson(QJsonDocument::Compact), before);
        QVERIFY(deck::layouts(*d).size() >= 9);
        // From a file, the window says why and keeps the undo stack as it was.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*source, &err), &e, &err), qPrintable(err));
        QString master = part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"));
        master.remove(QRegularExpression(QStringLiteral("<p:sldLayoutIdLst>.*?</p:sldLayoutIdLst>"), QRegularExpression::DotMatchesEverythingOption));
        QString presentation = part(e, QStringLiteral("/ppt/presentation.xml"));
        presentation.remove(QRegularExpression(QStringLiteral("<p:sldIdLst>.*?</p:sldIdLst>"), QRegularExpression::DotMatchesEverythingOption));
        QMap<QString, QByteArray> changes;
        changes[QStringLiteral("ppt/slideMasters/slideMaster1.xml")] = master.toUtf8();
        changes[QStringLiteral("ppt/presentation.xml")] = presentation.toUtf8();
        QFile file(dir.filePath(QStringLiteral("bare.potx")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(repack(e, changes));
        file.close();
        auto read = deck::importAnyPresentationFile(file.fileName(), &err);
        QVERIFY2(read, qPrintable(err));
        QCOMPARE(deck::layouts(*read).size(), 0);   // the file has a master and no layouts
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        const int layoutsBefore = int(deck::layouts(*ed->doc()).size());
        const int undoBefore = ed->undoStack()->count();
        QString said;
        QTimer::singleShot(0, [&said] {
            for (int tries = 0; tries < 200 && said.isEmpty(); ++tries) {
                if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                    said = box->text();
                    box->button(QMessageBox::Ok)->click();
                    return;
                }
                QTest::qWait(10);
            }
        });
        QVERIFY(!w.applyThemeFile(file.fileName(), true));
        QVERIFY2(said.contains(QStringLiteral("bare.potx")) && said.contains(QStringLiteral("layout")), qPrintable(said));
        QCOMPARE(int(deck::layouts(*ed->doc()).size()), layoutsBefore);
        QCOMPARE(ed->undoStack()->count(), undoBefore);
        QCOMPARE(deck::themeOf(*ed->doc()).name, deck::defaultTheme().name);
    }

    // Applying the same theme and variant to selected slides again goes onto the slide master a first time made, not onto a copy of it; another
    // theme or variant, and a theme whose colors were changed since, make their own.
    void repeatedApplyToSelectedReusesTheMaster()
    {
        auto slideMasters = [](const Document &doc) {
            int n = 0;
            for (const auto &m : doc.masters) n += deck::isSlideMaster(*m);
            return n;
        };
        auto d = deckToRestyle();
        const int tideline = deck::builtinThemeIndex(QStringLiteral("Tideline"));
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(tideline), {1}));
        QCOMPARE(slideMasters(*d), 2);
        const QString made = deck::slideMasterOf(*d, 1);
        const int layoutsBefore = int(deck::layouts(*d).size());
        // The same theme again, from another copy of it, for other slides: the same master, and no more layouts.
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(tideline), {2}));
        QCOMPARE(slideMasters(*d), 2);
        QCOMPARE(int(deck::layouts(*d).size()), layoutsBefore);
        QCOMPARE(deck::slideMasterOf(*d, 2), made);
        QCOMPARE(deck::slideMasterOf(*d, 1), made);
        QCOMPARE(layoutTypes(*d), layoutTypes(*deckToRestyle()));
        // And for the slide that has it, nothing moves.
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(tideline), {1, 2}));
        QCOMPARE(slideMasters(*d), 2);
        QCOMPARE(deck::slideMasterOf(*d, 1), made);
        // The presentation's own theme taken again for all slides, then for some: its master is the one they go to.
        auto all = deckToRestyle();
        QVERIFY(deck::applyThemeToAll(*all, *deck::themePresentation(tideline)));
        QVERIFY(deck::applyThemeToSlides(*all, *deck::themePresentation(tideline), {1}));
        QCOMPARE(slideMasters(*all), 1);
        // Another variant of it is another master; so is another theme.
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(tideline, 2), {3}));
        QCOMPARE(slideMasters(*d), 3);
        QVERIFY(deck::slideMasterOf(*d, 3) != made);
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Ember"))), {0}));
        QCOMPARE(slideMasters(*d), 4);
        // A master whose colors were changed since is no longer that theme: the next time makes a new one.
        deck::ColorSet set = deck::colorSetOf(*d);
        set.colors[4] = QColor(0x12, 0x34, 0x56);
        deck::applyColors(*d, set);
        const int before = slideMasters(*d);
        QVERIFY(deck::applyThemeToSlides(*d, *deck::themePresentation(tideline), {4}));
        QCOMPARE(slideMasters(*d), before + 1);
        QVERIFY(deck::slideMasterOf(*d, 4) != made);
    }

    // A theme made for slides of one shape put on slides of another (16:9 on a 4:3 presentation, or the reverse) is stretched to fit, as masters
    // are when the slide size changes, except that a thing that is a square on the master (a circle, a square, a corner triangle) stays a
    // square: it is scaled by the smaller of the two ratios about the middle of where stretching puts it, and keeps to the edge of the slide it
    // was on. Bands and rules still reach the edges.
    void aThemeOfAnotherSlideShapeKeepsItsCirclesRound()
    {
        const QSizeF wide(deck::kSlideWidth, deck::kSlideHeight), standard(deck::kStandardWidth, deck::kStandardHeight);
        int squares = 0, stretched = 0;
        // Checks the copy of `source`'s masters in `d`, whose slides are of size `to` (the source's are of size `from`).
        auto check = [&](const Document &source, const Document &d, const QSizeF &from, const QSizeF &to, const QString &name) {
            QCOMPARE(d.pageSize(), to);
            QCOMPARE(source.pageSize(), from);
            QVector<const MasterPage *> was, now;
            for (const auto &m : source.masters) was << m.get();
            for (const auto &m : d.masters) now << m.get();
            QCOMPARE(now.size(), was.size());
            const double k = std::min(to.width() / from.width(), to.height() / from.height());
            for (int i = 0; i < was.size(); ++i) {
                QCOMPARE(now[i]->items.size(), was[i]->items.size());
                for (size_t n = 0; n < was[i]->items.size(); ++n) {
                    const Item &a = *was[i]->items[n], &b = *now[i]->items[n];
                    if (!a.role.isEmpty() || a.type() != ItemType::Shape) continue;
                    const bool square = std::abs(a.rect.width() - a.rect.height()) <= 0.02 * std::max(a.rect.width(), a.rect.height());
                    if (square) {
                        ++squares;
                        QVERIFY2(std::abs(b.rect.width() - b.rect.height()) < 0.05, qPrintable(QStringLiteral("%1 %2: %3 by %4").arg(name, was[i]->name).arg(b.rect.width()).arg(b.rect.height())));
                        QVERIFY(std::abs(b.rect.width() - a.rect.width() * k) < 0.05);
                        // On an edge of the slide, still on it (and as far past it as it was, scaled).
                        if (a.rect.right() >= from.width() - 1 && a.rect.bottom() >= from.height() - 1) {
                            QVERIFY(std::abs(b.rect.right() - (to.width() + (a.rect.right() - from.width()) * k)) < 0.05);
                            QVERIFY(std::abs(b.rect.bottom() - (to.height() + (a.rect.bottom() - from.height()) * k)) < 0.05);
                        }
                    } else if (a.rect.left() <= 0 && a.rect.right() >= from.width()) {
                        ++stretched;   // a band across the slide still reaches both edges
                        QVERIFY(std::abs(b.rect.left()) < 0.05 && std::abs(b.rect.right() - to.width()) < 0.05);
                    }
                }
            }
        };
        for (const QString &name : {QStringLiteral("Orchard"), QStringLiteral("Confetti"), QStringLiteral("Ember"), QStringLiteral("Granite"), QStringLiteral("Harbor")}) {
            // A 16:9 theme on a 4:3 presentation, then that presentation's theme on a 16:9 one.
            auto source = deck::themePresentation(deck::builtinThemeIndex(name));
            auto standardDeck = deckToRestyle();
            deck::setSlideSize(*standardDeck, standard);
            QVERIFY(deck::applyThemeToAll(*standardDeck, *source));
            check(*source, *standardDeck, wide, standard, name);
            auto wideDeck = deckToRestyle();
            QVERIFY(deck::applyThemeToAll(*wideDeck, *standardDeck));
            check(*standardDeck, *wideDeck, standard, wide, name);
        }
        QVERIFY(squares >= 16);
        QVERIFY(stretched >= 4);
    }

    // Taking the theme of a file is slow enough (the file is read, copied, and its slides set again) to show the wait cursor, and the cursor
    // is back when it is done, whether the file gave a theme or not.
    void takingAThemeFromAFileShowsTheWaitCursor()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        const QByteArray bytes = deck::exportPresentation(*deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Granite"))), &err);
        QFile file(dir.filePath(QStringLiteral("granite.potx")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(bytes);
        file.close();
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        QVERIFY(!QGuiApplication::overrideCursor());
        bool waiting = false;
        const QMetaObject::Connection watch = connect(ed, &Editor::changed, this, [&] { waiting = waiting || (QGuiApplication::overrideCursor() && QGuiApplication::overrideCursor()->shape() == Qt::WaitCursor); });
        QVERIFY(w.applyThemeFile(file.fileName(), true));
        QObject::disconnect(watch);
        QVERIFY2(waiting, "no wait cursor while the theme was taken");
        QVERIFY(!QGuiApplication::overrideCursor());
        // A file that can't be read: the message is not shown under a wait cursor, and nothing is left on.
        QFile bad(dir.filePath(QStringLiteral("bad.potx")));
        QVERIFY(bad.open(QIODevice::WriteOnly));
        bad.write("not a presentation");
        bad.close();
        bool cursorUnderMessage = true;
        QTimer::singleShot(0, [&cursorUnderMessage] {
            for (int tries = 0; tries < 200; ++tries) {
                if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                    cursorUnderMessage = QGuiApplication::overrideCursor() != nullptr;
                    box->button(QMessageBox::Ok)->click();
                    return;
                }
                QTest::qWait(10);
            }
        });
        QVERIFY(!w.applyThemeFile(bad.fileName(), true));
        QVERIFY(!cursorUnderMessage);
        QVERIFY(!QGuiApplication::overrideCursor());
    }

    // The pictures a theme file brings stay in the presentation's store after the change is undone (so that redo has them), and the saved file
    // carries only the pictures something uses: none of them while the change is undone, and the picture once it is done again.
    void picturesOfAThemeFileAreNotSavedOnceItIsUndone()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto source = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Orchard")));
        MasterPage &master = *source->masters.first();
        auto logo = std::make_shared<PictureItem>();
        logo->name = QStringLiteral("logo");
        logo->rect = QRectF(860, 20, 60, 60);
        logo->imageId = source->addImage(odptest::png(8, 8, QColor(0x20, 0x80, 0x40)), QStringLiteral("png"));
        logo->imgRect = QRectF(0, 0, 60, 60);
        master.items.push_back(logo);
        QString err;
        QFile file(dir.filePath(QStringLiteral("logo.potx")));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(deck::exportPresentation(*source, &err));
        file.close();
        auto mediaIn = [&](const Document &doc) {
            Exported e;
            if (!unpack(deck::exportPresentation(doc, &err), &e, &err)) return -1;
            int n = 0;
            for (const QString &name : e.pkg.partNames()) n += name.startsWith(QLatin1String("/ppt/media/"));
            return n;
        };
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        const int imagesBefore = int(ed->doc()->images.size());
        QCOMPARE(mediaIn(*ed->doc()), 0);
        QVERIFY(w.applyThemeFile(file.fileName(), true));
        QCOMPARE(int(ed->doc()->images.size()), imagesBefore + 1);
        QCOMPARE(mediaIn(*ed->doc()), 1);
        ed->undo();
        QCOMPARE(int(ed->doc()->images.size()), imagesBefore + 1);   // kept for redo
        QCOMPARE(mediaIn(*ed->doc()), 0);                            // but not saved
        ed->redo();
        QCOMPARE(mediaIn(*ed->doc()), 1);
    }

    // The Variants group shows the four variants of the theme with the one it has marked; a click applies one; its menu offers colors,
    // fonts, and the twelve background styles, each a step that undoes.
    void designTabVariantsAndTheirMenu()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.show();
        auto *variants = w.findChild<VariantGallery *>();
        QVERIFY(variants);
        QCOMPARE(variants->grid()->count(), 4);
        QCOMPARE(variants->grid()->current(), 0);
        const int ember = deck::builtinThemeIndex(QStringLiteral("Ember"));
        w.applyTheme(ember, true);
        QTRY_VERIFY(variants->grid()->nameAt(0).startsWith(QStringLiteral("Ember")));
        QCOMPARE(variants->grid()->current(), 0);
        QCOMPARE(variants->grid()->nameAt(2).contains(deck::backgroundStyleName(deck::builtinThemeVariants(ember)[2].background)), true);
        // A variant, by a click: its colors and background.
        variants->grid()->resize(variants->grid()->sizeHint());
        QTest::mouseClick(variants->grid(), Qt::LeftButton, {}, variants->grid()->tileRect(3).center());
        const deck::Variant third = deck::builtinThemeVariants(ember)[3];
        QCOMPARE(deck::themeOf(*ed->doc()).colors[4], third.colors.colors[4]);
        QCOMPARE(deck::backgroundStyleOf(ed->doc()->masters[0]->background, deck::themeOf(*ed->doc())), third.background);
        QTRY_COMPARE(variants->grid()->current(), 3);
        ed->undo();
        QTRY_COMPARE(variants->grid()->current(), 0);

        // The menu.
        QMenu *menu = variants->moreButton()->menu();
        QVERIFY(menu);
        QMenu *colors = menu->findChild<QMenu *>(QStringLiteral("variantColors"));
        QMenu *fonts = menu->findChild<QMenu *>(QStringLiteral("variantFonts"));
        QMenu *styles = menu->findChild<QMenu *>(QStringLiteral("variantBackgrounds"));
        QVERIFY(colors && fonts && styles);
        QCOMPARE(menu->actions().size(), 3);
        Q_EMIT colors->aboutToShow();
        QVERIFY(colors->findChild<ThemeColors *>());   // the presentation's own twelve
        QList<QAction *> schemes;
        for (QAction *a : colors->actions())
            if (a->isCheckable()) schemes << a;
        QCOMPARE(schemes.size(), deck::colorSets().size());
        QCOMPARE(schemes.size(), 18);
        int checked = 0;
        for (QAction *a : schemes) checked += a->isChecked();
        QCOMPARE(checked, 1);
        QVERIFY(schemes.at(4)->isChecked());   // Ember's own
        schemes.at(5)->trigger();
        QCOMPARE(deck::themeOf(*ed->doc()).colors[4], deck::colorSets().value(5).colors[4]);
        ed->undo();
        QCOMPARE(deck::themeOf(*ed->doc()).colors[4], deck::builtinThemeVariants(ember)[0].colors.colors[4]);
        Q_EMIT fonts->aboutToShow();
        QList<QAction *> faces;
        for (QAction *a : fonts->actions())
            if (a->isCheckable()) faces << a;
        QCOMPARE(faces.size(), deck::fontSets().size());
        QVERIFY(faces.size() > 9);
        QVERIFY(faces.at(ember)->isChecked());
        faces.at(3)->trigger();
        QCOMPARE(deck::themeOf(*ed->doc()).headingFont, deck::fontSets().value(3).heading);
        QCOMPARE(ed->doc()->fonts.heading, deck::fontSets().value(3).heading);
        ed->undo();
        QCOMPARE(deck::themeOf(*ed->doc()).headingFont, deck::builtinThemeFonts(ember).heading);
        Q_EMIT styles->aboutToShow();
        auto *grid = styles->findChild<BackgroundStyles *>();
        QVERIFY(grid);
        QCOMPARE(grid->current(), deck::builtinThemeVariants(ember)[0].background);
        QCOMPARE(grid->styleAt(grid->swatchRect(7).center()), 7);
        QCOMPARE(grid->swatchRect(4).left(), grid->swatchRect(5).left());   // a color's three styles stand one above another
        QVERIFY(grid->swatchRect(3).left() > grid->swatchRect(0).left());
        const int other = deck::builtinThemeVariants(ember)[2].background;
        QVERIFY(other != grid->current());
        Q_EMIT grid->chosen(other);
        QCOMPARE(deck::backgroundStyleOf(ed->doc()->masters[0]->background, deck::themeOf(*ed->doc())), other);
        QCOMPARE(colorOfRun(*ed->doc(), firstRunOf(*ed->doc(), *placeholderOn(*ed->doc()->pages[0], QStringLiteral("ctrTitle")))), deck::themeOf(*ed->doc()).colors[0]);
        QTRY_COMPARE(variants->grid()->current(), 2);   // Ember's third variant is its colors on that background
        ed->undo();
        QCOMPARE(deck::backgroundStyleOf(ed->doc()->masters[0]->background, deck::themeOf(*ed->doc())), deck::builtinThemeVariants(ember)[0].background);
    }

    // A dark presentation opened from a file has light text where none was chosen, so a new text box can be read; a light one is as it was.
    void darkPresentationsGiveNewTextALightColor()
    {
        QString err;
        DeckWindow w;
        Editor *ed = w.editor();
        const QColor usual = ed->doc()->colors.slot(Main);
        auto dark = deck::importPresentation(deck::exportPresentation(*deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Ember"))), &err), &err);
        QVERIFY2(dark, qPrintable(err));
        QCOMPARE(dark->colors.slot(Main), usual);
        const QColor light = deck::themeOf(*dark).colors[1];
        ed->setDocument(std::move(dark));
        QCOMPARE(ed->doc()->colors.slot(Main), light);
        auto bright = deck::importPresentation(deck::exportPresentation(*deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor"))), &err), &err);
        QVERIFY2(bright, qPrintable(err));
        ed->setDocument(std::move(bright));
        QCOMPARE(ed->doc()->colors.slot(Main), usual);
        // Applying a dark theme does it too, and a light one gives the dark text back.
        w.applyTheme(deck::builtinThemeIndex(QStringLiteral("Marquee")), true);
        QCOMPARE(ed->doc()->colors.slot(Main), deck::themeOf(*ed->doc()).colors[1]);
        w.applyTheme(deck::builtinThemeIndex(QStringLiteral("Tideline")), true);
        QCOMPARE(ed->doc()->colors.slot(Main), deck::themeOf(*ed->doc()).colors[0]);
        QCOMPARE(ed->doc()->colors.slot(Accent1), deck::themeOf(*ed->doc()).colors[4]);
    }




    // ---- text on a backdrop of its own ----

    // The explicit color of each run of a story (a block with no run: the block's), as drawn.
    static QVector<QColor> runColorsOf(const Document &d, const QString &storyId)
    {
        QVector<QColor> out;
        const QTextDocument *sd = d.storyDoc(storyId);
        if (!sd) return out;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
            if (b.begin().atEnd()) {
                out << colorOfRun(d, b.charFormat());
                continue;
            }
            for (auto it = b.begin(); !it.atEnd(); ++it) out << colorOfRun(d, it.fragment().charFormat());
        }
        return out;
    }

    static void setStoryColor(Document &d, const QString &storyId, const QColor &color)
    {
        QTextCursor c(d.storyDoc(storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setForeground(color);
        f.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
        f.setFontPointSize(20);
        c.mergeCharFormat(f);
    }

    // A slide of hand-colored things on a light design: a text box in black, one in a dark gray that is no color of the theme, a table
    // with a near-black header row and gray body cells, a callout in dark blue with white text, a white card with black text, black text
    // over a gray rectangle and over a picture.
    static BackdropDeck deckWithBackdrops()
    {
        BackdropDeck out;
        out.doc = deck::newPresentation();
        Document &d = *out.doc;
        // The text colors are the theme's own, as a file made in a design with black and white text has them.
        deck::DeckTheme theme = deck::themeOf(d);
        theme.colors[0] = Qt::black;
        theme.colors[1] = Qt::white;
        deck::setTheme(d, theme);
        deck::addSlide(d, *deck::layoutOfType(d, QStringLiteral("blank")));
        Page &slide = *d.pages.last();
        auto text = [&](const QString &name, const QRectF &r, const QString &words, const QColor &color) {
            auto box = std::make_shared<TextItem>();
            box->name = name;
            box->rect = r;
            box->storyId = d.createStory(words);
            setStoryColor(d, box->storyId, color);
            slide.items.push_back(box);
            return box;
        };
        auto shape = [&](const QString &name, const QRectF &r, const QColor &fill, const QString &words, const QColor &color) {
            auto s = std::make_shared<ShapeItem>();
            s->name = name;
            s->rect = r;
            s->fill = Fill::solid(ColorRef::rgb(fill));
            if (!words.isEmpty()) {
                s->storyId = d.createStory(words);
                setStoryColor(d, s->storyId, color);
            }
            slide.items.push_back(s);
            return s;
        };
        out.plain = text(QStringLiteral("plain"), QRectF(120, 20, 300, 40), QStringLiteral("Plain words"), Qt::black)->id;
        out.gray = text(QStringLiteral("gray"), QRectF(120, 70, 300, 40), QStringLiteral("Gray words"), QColor(0x1A, 0x1A, 0x1A))->id;
        auto table = std::make_shared<TableItem>();
        table->name = QStringLiteral("grid");
        table->rect = QRectF(40, 130, 400, 100);
        table->rows = 2;
        table->cols = 2;
        table->colW = {200, 200};
        table->rowH = {50, 50};
        table->cells.resize(4);
        const QColor fills[4] = {QColor(0x05, 0x05, 0x05), QColor(0x05, 0x05, 0x05), QColor(0xCB, 0xCB, 0xCB), QColor(0xE7, 0xE7, 0xE7)};
        for (int i = 0; i < 4; ++i) {
            table->cells[i].storyId = d.createStory(QStringLiteral("cell"));
            table->cells[i].fill = Fill::solid(ColorRef::rgb(fills[i]));
            setStoryColor(d, table->cells[i].storyId, i < 2 ? QColor(Qt::white) : QColor(Qt::black));
        }
        slide.items.push_back(table);
        out.table = table->id;
        out.callout = shape(QStringLiteral("callout"), QRectF(480, 20, 300, 80), QColor(0x1F, 0x4E, 0x79), QStringLiteral("Callout"), Qt::white)->id;
        out.card = shape(QStringLiteral("card"), QRectF(480, 120, 300, 80), Qt::white, QStringLiteral("Card"), Qt::black)->id;
        shape(QStringLiteral("rectangle"), QRectF(480, 220, 300, 80), QColor(0xCB, 0xCB, 0xCB), QString(), Qt::black);
        out.overRect = text(QStringLiteral("over rectangle"), QRectF(490, 230, 280, 60), QStringLiteral("On gray"), Qt::black)->id;
        auto photo = std::make_shared<PictureItem>();
        photo->name = QStringLiteral("photo");
        photo->rect = QRectF(480, 320, 300, 100);
        photo->imageId = d.addImage(odptest::png(8, 8, QColor(0xE0, 0xD8, 0xC8)), QStringLiteral("png"));
        photo->imgRect = QRectF(0, 0, 300, 100);
        slide.items.push_back(photo);
        out.overPicture = text(QStringLiteral("over picture"), QRectF(490, 330, 280, 60), QStringLiteral("On a photo"), Qt::black)->id;
        return out;
    }

    // A color that follows an accent of the scheme: the accent itself where it reads on what is behind it (`need` to 1), else a color of its
    // hue that does (a change of colors keeps the text readable).
    static void followsAccent(const QColor &actual, const QColor &accent, const QVector<QColor> &behind, double need, const char *what)
    {
        bool reads = true;
        for (const QColor &b : behind) reads = reads && deck::contrastRatio(accent, b) >= need;
        if (reads) {
            QVERIFY2(actual == accent, what);
            return;
        }
        for (const QColor &b : behind) QVERIFY2(deck::contrastRatio(actual, b) >= need, what);
        QVERIFY2(std::abs(actual.hslHue() - accent.hslHue()) <= 15, what);
    }

    static const MasterPage &firstMasterOf(const Document &d)
    {
        for (const auto &m : d.masters)
            if (deck::isSlideMaster(*m)) return *m;
        return *d.masters.first();
    }

    static const Item *itemById(const Page &p, const QString &id)
    {
        for (const auto &it : p.items)
            if (it->id == id) return it.get();
        return nullptr;
    }

    static QString storyOfItem(const Item &it)
    {
        if (it.type() == ItemType::Text) return static_cast<const TextItem &>(it).storyId;
        if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).storyId;
        return QString();
    }

    // Every run of the hand-colored slide against what is behind it once a dark background or a dark theme is in.
    static void checkBackdrops(const BackdropDeck &b, const QString &what)
    {
        const Document &d = *b.doc;
        const Page &slide = *d.pages.last();
        const MasterPage &layout = *d.masterFor(slide);
        auto readable = [&](const QColor &fg, const QVector<QColor> &behind, const QString &which) {
            for (const QColor &bg : behind)
                QVERIFY2(deck::contrastRatio(fg, bg) >= 4.5, qPrintable(QStringLiteral("%1, %2: %3 on %4 is %5").arg(what, which, fg.name(), bg.name()).arg(deck::contrastRatio(fg, bg), 0, 'f', 2)));
        };
        // Text on the slide's own background follows it: light on a dark one.
        for (const QString &id : {b.plain, b.gray}) {
            const Item &it = *itemById(slide, id);
            for (const QColor &c : runColorsOf(d, storyOfItem(it))) readable(c, colorsUnder(d, layout, it.rect.center()), it.name);
        }
        // Where a color of its own is behind, the text reads on it, and was not turned to the dark background's light.
        auto on = [&](const QString &id, const QVector<QColor> &behind, const QString &which) {
            for (const QColor &c : runColorsOf(d, storyOfItem(*itemById(slide, id)))) readable(c, behind, which);
        };
        on(b.callout, {QColor(0x1F, 0x4E, 0x79)}, QStringLiteral("callout"));
        on(b.overRect, {QColor(0xCB, 0xCB, 0xCB)}, QStringLiteral("text over a gray rectangle"));
        on(b.overPicture, {QColor(0xE0, 0xD8, 0xC8)}, QStringLiteral("text over a picture"));
        // A card in a color of the theme turns with the theme, text and all.
        const auto &card = static_cast<const ShapeItem &>(*itemById(slide, b.card));
        readable(runColorsOf(d, card.storyId).value(0), {card.fill.color.resolve(d.colors)}, QStringLiteral("card"));
        // A table: the cells with colors of their own keep them, and every cell's text reads on its fill.
        const auto &table = static_cast<const TableItem &>(*itemById(slide, b.table));
        QCOMPARE(table.cells[0].fill.color.resolve(d.colors), QColor(0x05, 0x05, 0x05));
        QCOMPARE(table.cells[2].fill.color.resolve(d.colors), QColor(0xCB, 0xCB, 0xCB));
        QCOMPARE(table.cells[3].fill.color.resolve(d.colors), QColor(0xE7, 0xE7, 0xE7));
        for (const TableCell &cell : table.cells) readable(runColorsOf(d, cell.storyId).value(0), {cell.fill.color.resolve(d.colors)}, QStringLiteral("table cell"));
    }

    // A dark background style or a dark theme turns hand-set black and white text light or dark only where the slide's own background is
    // behind it: inside a table cell or shape with a color of its own, or over a picture, the text stays as it was.
    void darkBackgroundsTurnOnlyTextOnTheBackground()
    {
        for (int style : {6, 9, 10, 11}) {
            BackdropDeck b = deckWithBackdrops();
            deck::applyBackgroundStyle(*b.doc, style);
            QVERIFY(deck::isDarkBackground(firstMasterOf(*b.doc).background));
            checkBackdrops(b, QStringLiteral("background style %1").arg(style + 1));
        }
        for (const QString &name : {QStringLiteral("Ember"), QStringLiteral("Granite"), QStringLiteral("Marquee")}) {
            BackdropDeck b = deckWithBackdrops();
            const int theme = deck::builtinThemeIndex(name);
            QVERIFY(theme > 0);
            deck::applyThemeToAll(*b.doc, *deck::themePresentation(theme));
            checkBackdrops(b, name);
        }
        // And back to a light one: nothing is left unreadable there either.
        BackdropDeck again = deckWithBackdrops();
        deck::applyBackgroundStyle(*again.doc, 10);
        deck::applyBackgroundStyle(*again.doc, 0);
        checkBackdrops(again, QStringLiteral("back to style 1"));
    }

    // ---- pictures in placeholders ----

    // A picture of `pixels` that stands in the placeholder of `type` on a slide: it fills the placeholder (cropped to fill as a picture
    // placeholder does) and keeps the placeholder's role. Returns it.
    static PictureItem *pictureIn(Document &d, Page &slide, const QString &type, const QSize &pixels)
    {
        for (auto &it : slide.items) {
            if (deck::placeholderType(*it) != type) continue;
            auto pic = std::make_shared<PictureItem>();
            pic->name = QStringLiteral("held picture");
            pic->role = it->role;
            pic->rect = it->rect;
            pic->imageId = d.addImage(odptest::png(pixels.width(), pixels.height(), QColor(0x30, 0x90, 0x60)), QStringLiteral("png"));
            pic->fitImage(pixels, true);
            it = pic;
            return pic.get();
        }
        return nullptr;
    }

    // The part of the picture its frame shows, in the picture's own proportions (0 to 1 across and down).
    static QRectF shownPart(const PictureItem &p)
    {
        const QRectF frame(QPointF(0, 0), p.rect.size());
        const QRectF seen = frame.intersected(p.imgRect);
        return QRectF((seen.left() - p.imgRect.left()) / p.imgRect.width(), (seen.top() - p.imgRect.top()) / p.imgRect.height(), seen.width() / p.imgRect.width(), seen.height() / p.imgRect.height());
    }

    // A picture in a content placeholder moves to the new layout's place and keeps what it showed, whole and in its proportions: it is fitted
    // inside the place, not cropped and not offset. One in a picture placeholder fills the new place, cropped to fill and centered. A picture
    // that stands in no placeholder stays where it is.
    void picturesInPlaceholdersFollowThemesAndLayouts()
    {
        for (int step = 0; step < 2; ++step) {   // 0: a change of theme, 1: a change of layout
            auto d = deck::newPresentation();
            deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
            deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("picTx")));
            PictureItem *content = pictureIn(*d, *d->pages[1], QStringLiteral("obj"), QSize(400, 300));
            PictureItem *placed = pictureIn(*d, *d->pages[2], QStringLiteral("pic"), QSize(300, 400));
            QVERIFY(content && placed);
            auto loose = std::make_shared<PictureItem>();
            loose->name = QStringLiteral("loose picture");
            loose->rect = QRectF(700, 380, 120, 90);
            loose->imageId = content->imageId;
            loose->imgRect = QRectF(-10, -5, 150, 100);   // cropped by hand
            d->pages[1]->items.push_back(loose);
            const QRectF looseRect = loose->rect, looseImg = loose->imgRect;
            const QRectF seenBefore = shownPart(*content);
            const QRectF frameBefore = content->rect;
            const QRectF placedSeen = shownPart(*placed);

            if (step == 0) {
                deck::applyThemeToAll(*d, *deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor"))));
            } else {
                deck::applyLayout(*d, *d->pages[1], *deck::layoutOfType(*d, QStringLiteral("twoObj")));
                deck::applyLayout(*d, *d->pages[2], *deck::layoutOfType(*d, QStringLiteral("picTx")));
            }
            const auto *c = static_cast<const PictureItem *>(itemNamed(*d->pages[1], QStringLiteral("held picture")));
            const auto *p = static_cast<const PictureItem *>(itemNamed(*d->pages[2], QStringLiteral("held picture")));
            QVERIFY(c && p);
            const MasterPage &layoutC = *d->masterFor(*d->pages[1]);
            const MasterPage &layoutP = *d->masterFor(*d->pages[2]);
            auto placeOf = [](const MasterPage &layout, const QString &role) {
                for (const auto &it : layout.items)
                    if (it->role == role) return it->rect;
                return QRectF();
            };
            // Content placeholder: inside the place, centered in it, touching it on one side, the same proportions, the same part shown, whole.
            const QRectF cell = placeOf(layoutC, c->role);
            QVERIFY(!cell.isEmpty());
            QVERIFY2(cell.adjusted(-0.01, -0.01, 0.01, 0.01).contains(c->rect), qPrintable(QStringLiteral("step %1: %2 not in %3").arg(step).arg(c->rect.x()).arg(cell.x())));
            QVERIFY(std::abs(c->rect.center().x() - cell.center().x()) < 0.01 && std::abs(c->rect.center().y() - cell.center().y()) < 0.01);
            QVERIFY(std::abs(c->rect.width() - cell.width()) < 0.01 || std::abs(c->rect.height() - cell.height()) < 0.01);
            QVERIFY(std::abs(c->rect.width() / c->rect.height() - frameBefore.width() / frameBefore.height()) < 0.001);
            const QRectF seenNow = shownPart(*c);
            QVERIFY(std::abs(seenNow.x() - seenBefore.x()) < 0.001 && std::abs(seenNow.y() - seenBefore.y()) < 0.001);
            QVERIFY(std::abs(seenNow.width() - seenBefore.width()) < 0.001 && std::abs(seenNow.height() - seenBefore.height()) < 0.001);
            // The picture is not stretched: its pixels are as wide as they are tall on the page.
            QVERIFY(std::abs(c->imgRect.width() / c->imgRect.height() - 400.0 / 300.0) < 0.001);
            // Picture placeholder: the new place exactly, filled by the picture (no gap), centered on what it showed, not stretched.
            const QRectF spot = placeOf(layoutP, p->role);
            QVERIFY(!spot.isEmpty());
            QCOMPARE(p->rect, spot);
            QVERIFY(p->imgRect.adjusted(-0.01, -0.01, 0.01, 0.01).contains(QRectF(QPointF(0, 0), p->rect.size())));
            QVERIFY(std::abs(p->imgRect.width() / p->imgRect.height() - 300.0 / 400.0) < 0.001);
            const QRectF placedNow = shownPart(*p);
            QVERIFY(std::abs((placedNow.center().x()) - placedSeen.center().x()) < 0.001 && std::abs(placedNow.center().y() - placedSeen.center().y()) < 0.001);
            // A picture in no placeholder stays as it was.
            const auto *l = static_cast<const PictureItem *>(itemNamed(*d->pages[1], QStringLiteral("loose picture")));
            QVERIFY(l);
            QCOMPARE(l->rect, looseRect);
            QCOMPARE(l->imgRect, looseImg);
        }
    }

    // ---- the color map of a dark master ----

    // A presentation with a slide of each kind of layout, its words said.
    static void addEveryLayout(Document &d)
    {
        const QVector<MasterPage *> all = deck::layouts(d);
        for (const MasterPage *l : all) {
            if (deck::layoutType(*l) == QLatin1String("title")) continue;
            deck::addSlide(d, *l);
            for (const auto &it : d.pages.last()->items)
                if (it->type() == ItemType::Text && !deck::placeholderType(*it).isEmpty() && deck::placeholderType(*it) != QLatin1String("sldNum") && deck::placeholderType(*it) != QLatin1String("pic"))
                    sayIn(d, *it, QStringLiteral("Words on %1").arg(l->name));
        }
    }

    // A slide master with a dark background is saved with the color map of a dark design, as the presentation program's own dark themes
    // have it (the background is dark 1, the text light 1), so that a text box typed on its slides is light; a light master has the usual
    // map. Read back, the same colors show.
    void darkMastersAreSavedWithTheDarkColorMap()
    {
        const QString dark = QStringLiteral("<p:clrMap bg1=\"dk1\" tx1=\"lt1\" bg2=\"dk2\" tx2=\"lt2\" ");
        const QString light = QStringLiteral("<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" ");
        int darkSeen = 0, lightSeen = 0;
        for (int theme = 0; theme < deck::builtinThemeCount(); ++theme)
            for (int variant = 0; variant < 4; ++variant) {
                const QString what = QStringLiteral("%1 variant %2").arg(deck::builtinThemeName(theme)).arg(variant + 1);
                auto d = deck::themePresentation(theme, variant);
                addEveryLayout(*d);
                const bool isDark = deck::isDarkBackground(firstMasterOf(*d).background);
                QString err;
                const QByteArray bytes = deck::exportPresentation(*d, &err);
                QVERIFY2(!bytes.isEmpty(), qPrintable(err));
                // Kept beside the screenshots, for opening the dark ones in the presentation program.
                if (isDark && !qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                    QFile kept(QDir(qEnvironmentVariable("JP_SHOT_DIR")).filePath(QStringLiteral("theme-%1-%2.pptx").arg(theme).arg(variant)));
                    if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
                }
                Exported e;
                QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
                QVERIFY2(packageProblem(e).isEmpty(), qPrintable(what + ": " + packageProblem(e)));
                const QString master = part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"));
                QVERIFY2(master.contains(isDark ? dark : light), qPrintable(what + (isDark ? " is dark" : " is light") + ": " + master.mid(master.indexOf(QStringLiteral("<p:clrMap")), 200)));
                QVERIFY2(!master.contains(isDark ? light : dark), qPrintable(what));
                (isDark ? darkSeen : lightSeen)++;
                // The layouts and slides follow their master's map.
                QVERIFY2(part(e, QStringLiteral("/ppt/slideLayouts/slideLayout1.xml")).contains(QStringLiteral("<a:masterClrMapping/>")), qPrintable(what));
                // Read back: the same color in every run, and a dark master is still dark.
                auto back = deck::importPresentation(bytes, &err);
                QVERIFY2(back, qPrintable(err));
                QCOMPARE(deck::isDarkBackground(firstMasterOf(*back).background), isDark);
                QCOMPARE(back->pages.size(), d->pages.size());
                for (int i = 0; i < d->pages.size(); ++i) {
                    QVector<QColor> was, now;
                    // (Of the words said: a place that is empty is not kept as a shape of the slide.)
                    auto say = [](const Document &doc, const Page &page, QVector<QColor> *into) {
                        walkItems(page.items, [&](const ItemPtr &it) {
                            if (it->type() != ItemType::Text) return;
                            const QString story = static_cast<const TextItem &>(*it).storyId;
                            if (doc.storyDoc(story) && !doc.storyDoc(story)->isEmpty()) *into << runColorsOf(doc, story);
                        });
                    };
                    say(*d, *d->pages[i], &was);
                    say(*back, *back->pages[i], &now);
                    QVERIFY2(was == now, qPrintable(QStringLiteral("%1, slide %2").arg(what).arg(i + 1)));
                }
            }
        QVERIFY(darkSeen >= 8 && lightSeen >= 8);
        // Any dark background style makes the map dark, and a light one turns it back.
        auto d = deck::newPresentation();
        deck::applyBackgroundStyle(*d, 10);
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml")).contains(dark));
        deck::applyBackgroundStyle(*d, 1);
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml")).contains(light));
    }

    // A slide whose own background is a picture loses it to a background style; text that stood on the picture (taken to have read there) is
    // made to read on what replaces it, large text too (4.5 to 1).
    void textOnAPictureBackgroundReadsOnTheStyleThatReplacesIt()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("blank")));
        Page &slide = *d->pages.last();
        slide.background.type = Fill::Picture;
        slide.background.imageId = d->addImage(odptest::png(8, 8, QColor(0xE0, 0xE8, 0xD0)), QStringLiteral("png"));
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(60, 60, 600, 120);
        box->storyId = d->createStory(QStringLiteral("Big words"));
        setStoryColor(*d, box->storyId, QColor(0x20, 0x20, 0x20));
        {
            QTextCursor c(d->storyDoc(box->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(44);
            c.mergeCharFormat(f);
        }
        slide.items.push_back(box);
        deck::applyBackgroundStyle(*d, 10);
        QVERIFY(slide.background.type == Fill::NoFill);
        const MasterPage &layout = *d->masterFor(slide);
        for (const QColor &bg : colorsUnder(*d, layout, box->rect.center()))
            for (const QColor &c : runColorsOf(*d, box->storyId)) QVERIFY2(deck::contrastRatio(c, bg) >= 4.5, qPrintable(QStringLiteral("%1 on %2").arg(c.name(), bg.name())));
    }

    // A presentation that isn't in a built-in theme has variants too: its own colors on four backgrounds.
    void variantsOfAnotherThemeAreItsColorsOnFourBackgrounds()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        auto *variants = w.findChild<VariantGallery *>();
        QVERIFY(variants);
        deck::DeckTheme mine = deck::themeOf(*ed->doc());
        mine.name = QStringLiteral("Mine");
        deck::setTheme(*ed->doc(), mine);
        Q_EMIT ed->changed();
        QCOMPARE(variants->grid()->count(), 4);
        QCOMPARE(variants->grid()->current(), 0);
        Q_EMIT variants->variantChosen(2);
        QVERIFY(deck::backgroundStyleIsDark(deck::backgroundStyleOf(ed->doc()->masters[0]->background, deck::themeOf(*ed->doc()))));
        QCOMPARE(deck::themeOf(*ed->doc()).name, QStringLiteral("Mine"));
        QTRY_COMPARE(variants->grid()->current(), 2);
    }

    // ---- bullets of a layout's placeholder ----

    // The body of a layout with bullets as a theme of the presentation program has them: a paragraph for each of five levels, each in a list
    // of its own with a bullet in a font, a color, and a size, and its own indent, spacing, and text size.
    static void giveBullets(Document &d, MasterPage &layout)
    {
        for (const auto &it : layout.items) {
            // The body of a layout, or of the slide master (the placeholder that has the text styles of the body).
            const QString type = deck::placeholderType(*it);
            if ((type != QLatin1String("obj") && !(type == QLatin1String("body") && deck::isSlideMaster(layout))) || it->type() != ItemType::Text) continue;
            QTextDocument *sd = d.storyDoc(static_cast<TextItem *>(it.get())->storyId);
            sd->clear();
            QTextCursor c(sd);
            static const double margins[5] = {27, 58.5, 90, 126, 162}, hangs[5] = {-27, -22.5, -18, -18, -18}, sizes[5] = {18, 16, 14, 12, 12};
            static const char *words[5] = {"Click to edit Master text styles", "Second level", "Third level", "Fourth level", "Fifth level"};
            for (int i = 0; i < 5; ++i) {
                QTextBlockFormat bf;
                bf.setLeftMargin(margins[i]);
                bf.setTextIndent(hangs[i]);
                bf.setTopMargin(10);
                if (i) bf.setProperty(tp::ListLevel, i);
                QTextCharFormat cf;
                cf.setFontPointSize(sizes[i]);
                cf.setForeground(QColor(0x40, 0x40, 0x40));
                cf.setProperty(tp::ColorRefP, QStringLiteral("#404040"));
                cf.setFontFamilies({QStringLiteral("Carlito")});
                if (i) c.insertBlock(bf, cf);
                else {
                    c.setBlockFormat(bf);
                    c.setBlockCharFormat(cf);
                }
                QTextListFormat lf;
                lf.setStyle(QTextListFormat::ListDisc);
                lf.setProperty(tp::BulletFont, QStringLiteral("Wingdings 3"));
                lf.setProperty(tp::BulletChar, QString(QChar(0xF07D)));
                lf.setProperty(tp::BulletColor, QStringLiteral("#90C226"));
                lf.setProperty(tp::BulletSize, sizes[i] * 0.8);
                lf.setProperty(tp::ListLevel, i);
                c.createList(lf);
                c.insertText(QString::fromLatin1(words[i]), cf);
            }
        }
    }

    static std::unique_ptr<Document> bulletedDeck()
    {
        auto d = deck::newPresentation();
        for (const auto &m : d->masters)
            if (deck::isSlideMaster(*m) || deck::layoutType(*m) == QLatin1String("obj")) giveBullets(*d, *m);
        return d;
    }

    // What a paragraph's list says: its level and its marker's character, font, color, and size ("" and 0 for none).
    static QString listSaid(const QTextBlock &b)
    {
        const QTextList *l = b.textList();
        if (!l) return QStringLiteral("none");
        const QTextListFormat lf = l->format();
        return QStringLiteral("%1|%2|%3|%4|%5|%6").arg(lf.intProperty(tp::ListLevel)).arg(int(lf.style())).arg(lf.stringProperty(tp::BulletChar), lf.stringProperty(tp::BulletFont),
                                                                                                        lf.stringProperty(tp::BulletColor)).arg(lf.property(tp::BulletSize).toDouble());
    }

    static const TextItem *bodyOf(const PageBase &page) { return static_cast<const TextItem *>(placeholderOn(page, QStringLiteral("obj"))); }

    static const MasterPage &layoutOf(const Document &d, const QString &type) { return *deck::layoutOfType(d, type); }

    static QTextBlock layoutParagraph(const Document &d, int level)
    {
        const TextItem *place = bodyOf(layoutOf(d, QStringLiteral("obj")));
        return d.storyDoc(place->storyId)->findBlockByNumber(level);
    }

    // A new slide takes the list of its layout's placeholder, so what is typed has its bullets: the character, font, color, and size of the
    // bullet, and the indent, spacing, and text size of the first level.
    void newSlideKeepsTheLayoutsBullets()
    {
        auto d = bulletedDeck();
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        const TextItem *body = bodyOf(*d->pages[1]);
        QVERIFY(body);
        QTextDocument *sd = d->storyDoc(body->storyId);
        QVERIFY(sd->isEmpty());
        const QTextBlock first = sd->begin(), layoutFirst = layoutParagraph(*d, 0);
        QVERIFY(first.textList());
        QCOMPARE(listSaid(first), listSaid(layoutFirst));
        QCOMPARE(first.blockFormat().leftMargin(), 27.0);
        QCOMPARE(first.blockFormat().textIndent(), -27.0);
        QCOMPARE(first.blockFormat().topMargin(), 10.0);
        QCOMPARE(first.charFormat().fontPointSize(), 18.0);
        QCOMPARE(first.charFormat().stringProperty(tp::ColorRefP), QStringLiteral("#404040"));
        // Typing in it, and a second paragraph, are bulleted.
        QTextCursor c(sd);
        c.insertText(QStringLiteral("One"));
        c.insertBlock();
        c.insertText(QStringLiteral("Two"));
        QCOMPARE(sd->blockCount(), 2);
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
            QCOMPARE(listSaid(b), listSaid(layoutFirst));
            QCOMPARE(b.begin().fragment().charFormat().fontPointSize(), 18.0);
        }
        // The layout's own paragraphs are as they were, and the other placeholders have no list.
        QCOMPARE(layoutParagraph(*d, 1).textList()->format().intProperty(tp::ListLevel), 1);
        QVERIFY(!d->storyDoc(placeholderOn(*d->pages[1], QStringLiteral("title"))->storyId)->begin().textList());
        // A slide with a slide's footers too: a layout that has a list in no other placeholder gives none to them.
        deck::FooterOptions o;
        o.slideNumber = true;
        deck::applyFooters(*d, o);
        for (const auto &it : d->pages[1]->items)
            if (deck::placeholderType(*it) == QLatin1String("sldNum")) QVERIFY(!d->storyDoc(static_cast<TextItem *>(it.get())->storyId)->begin().textList());
    }

    // A slide given a layout takes its bullets in the placeholders it had empty and in the ones it gets; the text it has stays as it was.
    void changingALayoutKeepsTheLayoutsBullets()
    {
        auto d = bulletedDeck();
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("twoObj")));
        Page &slide = *d->pages[1];
        QVector<const TextItem *> bodies;
        for (const auto &it : slide.items)
            if (deck::placeholderType(*it) == QLatin1String("obj")) bodies << static_cast<const TextItem *>(it.get());
        QCOMPARE(bodies.size(), 2);
        sayIn(*d, *bodies[0], QStringLiteral("Typed before"));
        deck::applyLayout(*d, slide, layoutOf(*d, QStringLiteral("obj")));
        QCOMPARE(deck::layoutType(*d->masterFor(slide)), QStringLiteral("obj"));
        const QTextBlock typed = d->storyDoc(bodies[0]->storyId)->begin();
        QCOMPARE(typed.text(), QStringLiteral("Typed before"));
        QVERIFY(!typed.textList());   // its own paragraph stays as it was
        // An empty placeholder takes the layout's list; with Title Only first, the body the layout adds has it.
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("twoObj")));
        Page &second = *d->pages[2];
        deck::applyLayout(*d, second, layoutOf(*d, QStringLiteral("obj")));
        const TextItem *emptied = bodyOf(second);
        QVERIFY(d->storyDoc(emptied->storyId)->isEmpty());
        QCOMPARE(listSaid(d->storyDoc(emptied->storyId)->begin()), listSaid(layoutParagraph(*d, 0)));
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("titleOnly")));
        Page &third = *d->pages[3];
        QVERIFY(!bodyOf(third));
        deck::applyLayout(*d, third, layoutOf(*d, QStringLiteral("obj")));
        QCOMPARE(listSaid(d->storyDoc(bodyOf(third)->storyId)->begin()), listSaid(layoutParagraph(*d, 0)));
        // Back to a layout with no list: the empty paragraph has none, as that layout says.
        deck::applyLayout(*d, third, layoutOf(*d, QStringLiteral("twoObj")));
        QCOMPARE(listSaid(d->storyDoc(bodyOf(third)->storyId)->begin()), QStringLiteral("none"));
    }

    // A paragraph moves a level in or out and takes its new level's bullet, indent, spacing, and size from the layout; its text that looked as
    // the old level's does too; the paragraphs around it stay.
    void listLevelsTakeTheLayoutsLevels()
    {
        auto d = bulletedDeck();
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        const TextItem *body = bodyOf(*d->pages[1]);
        QTextDocument *sd = d->storyDoc(body->storyId);
        QTextCursor c(sd);
        c.insertText(QStringLiteral("One"));
        c.insertBlock();
        c.insertText(QStringLiteral("Two"));
        c.insertBlock();
        c.insertText(QStringLiteral("Three"));
        // Two goes in a level.
        QVERIFY(deck::layoutHasList(*d, *body));
        QVERIFY(deck::changeListLevel(*d, *body, 1, 1, 1));
        const QTextBlock two = sd->findBlockByNumber(1);
        QCOMPARE(listSaid(two), listSaid(layoutParagraph(*d, 1)));
        QCOMPARE(two.blockFormat().intProperty(tp::ListLevel), 1);
        QCOMPARE(two.blockFormat().leftMargin(), 58.5);
        QCOMPARE(two.blockFormat().textIndent(), -22.5);
        QCOMPARE(two.begin().fragment().charFormat().fontPointSize(), 16.0);
        QCOMPARE(two.charFormat().fontPointSize(), 16.0);
        QCOMPARE(two.text(), QStringLiteral("Two"));
        // The others are as they were.
        QCOMPARE(listSaid(sd->findBlockByNumber(0)), listSaid(layoutParagraph(*d, 0)));
        QCOMPARE(sd->findBlockByNumber(0).blockFormat().leftMargin(), 27.0);
        QCOMPARE(listSaid(sd->findBlockByNumber(2)), listSaid(layoutParagraph(*d, 0)));
        // Another level in, a hand-set color and bold stay, then back out two levels (never past the first).
        QTextCursor mark(sd->findBlockByNumber(1));
        mark.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QTextCharFormat red;
        red.setForeground(QColor(0xC0, 0, 0));
        red.setProperty(tp::ColorRefP, QStringLiteral("#C00000"));
        red.setFontWeight(QFont::Bold);
        mark.mergeCharFormat(red);
        QVERIFY(deck::changeListLevel(*d, *body, 1, 1, 1));
        QCOMPARE(listSaid(sd->findBlockByNumber(1)), listSaid(layoutParagraph(*d, 2)));
        QCOMPARE(sd->findBlockByNumber(1).begin().fragment().charFormat().fontPointSize(), 14.0);
        QCOMPARE(sd->findBlockByNumber(1).begin().fragment().charFormat().stringProperty(tp::ColorRefP), QStringLiteral("#C00000"));
        QCOMPARE(sd->findBlockByNumber(1).begin().fragment().charFormat().fontWeight(), int(QFont::Bold));
        QVERIFY(deck::changeListLevel(*d, *body, 1, 1, -1));
        QVERIFY(deck::changeListLevel(*d, *body, 1, 1, -1));
        QVERIFY(deck::changeListLevel(*d, *body, 1, 1, -1));
        QCOMPARE(listSaid(sd->findBlockByNumber(1)), listSaid(layoutParagraph(*d, 0)));
        QVERIFY(!sd->findBlockByNumber(1).blockFormat().hasProperty(tp::ListLevel));
        QCOMPARE(sd->findBlockByNumber(1).blockFormat().leftMargin(), 27.0);
        QCOMPARE(sd->findBlockByNumber(1).begin().fragment().charFormat().fontPointSize(), 18.0);
        // A level the layout has no paragraph for takes the deepest it has.
        for (int i = 0; i < 7; ++i) QVERIFY(deck::changeListLevel(*d, *body, 2, 2, 1));
        QCOMPARE(sd->findBlockByNumber(2).blockFormat().intProperty(tp::ListLevel), 7);
        QCOMPARE(sd->findBlockByNumber(2).textList()->format().intProperty(tp::ListLevel), 7);
        QCOMPARE(sd->findBlockByNumber(2).blockFormat().leftMargin(), 162.0 + 3 * 36.0);
        // A layout with no list gives the levels nothing to take: the call says so and changes nothing.
        auto plain = deck::newPresentation();
        deck::addSlide(*plain, layoutOf(*plain, QStringLiteral("obj")));
        const TextItem *plainBody = bodyOf(*plain->pages[1]);
        QVERIFY(!deck::layoutHasList(*plain, *plainBody));
        QVERIFY(!deck::changeListLevel(*plain, *plainBody, 0, 0, 1));
        QVERIFY(!deck::layoutHasList(*plain, *placeholderOn(*plain->pages[1], QStringLiteral("title"))));
    }

    // Increase and Decrease List Level in a slide's placeholder (one step that undoes) take the layout's levels; in a text box that is no
    // placeholder, and in a placeholder whose layout has no list, they are the editor's.
    void listLevelButtonsUseTheLayoutsLevels()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        ed->setDocument(bulletedDeck());
        w.newSlide(QStringLiteral("obj"));
        const QString storyId = bodyOf(*ed->doc()->pages[1])->storyId, itemId = bodyOf(*ed->doc()->pages[1])->id;   // (an undo makes the objects new)
        ed->beginTextEdit(itemId);
        QTextCursor c = ed->cursor();
        c.insertText(QStringLiteral("One"));
        c.insertBlock();
        c.insertText(QStringLiteral("Two"));
        ed->setCursor(c);
        ed->textEdited();
        QTRY_VERIFY(w.act("para.indentInc")->isEnabled());
        const QString before = QString::fromUtf8(QJsonDocument(storyToJson(ed->doc()->storyDoc(storyId))).toJson());
        w.act("para.indentInc")->trigger();
        QTextDocument *sd = ed->doc()->storyDoc(storyId);
        QCOMPARE(listSaid(sd->findBlockByNumber(1)), listSaid(layoutParagraph(*ed->doc(), 1)));
        QCOMPARE(sd->findBlockByNumber(1).blockFormat().leftMargin(), 58.5);
        QCOMPARE(listSaid(sd->findBlockByNumber(0)), listSaid(layoutParagraph(*ed->doc(), 0)));
        ed->undo();
        QCOMPARE(QString::fromUtf8(QJsonDocument(storyToJson(ed->doc()->storyDoc(storyId))).toJson()), before);
        ed->redo();
        QCOMPARE(listSaid(ed->doc()->storyDoc(storyId)->findBlockByNumber(1)), listSaid(layoutParagraph(*ed->doc(), 1)));
        ed->beginTextEdit(itemId);
        ed->setCursor(QTextCursor(ed->doc()->storyDoc(storyId)->findBlockByNumber(1)));
        QTRY_VERIFY(w.act("para.indentDec")->isEnabled());
        w.act("para.indentDec")->trigger();
        QCOMPARE(listSaid(ed->doc()->storyDoc(storyId)->findBlockByNumber(1)), listSaid(layoutParagraph(*ed->doc(), 0)));
        ed->endTextEdit();
        // A text box of the slide's own: the editor's own indent.
        auto box = ed->newTextBox(QRectF(100, 400, 300, 60), QStringLiteral("Box"));
        ed->addItem(box);
        ed->beginTextEdit(box->id);
        QTRY_VERIFY(w.act("para.indentInc")->isEnabled());
        const double was = ed->doc()->storyDoc(static_cast<TextItem *>(box.get())->storyId)->begin().blockFormat().leftMargin();
        w.act("para.indentInc")->trigger();
        QCOMPARE(ed->doc()->storyDoc(static_cast<TextItem *>(box.get())->storyId)->begin().blockFormat().leftMargin(), was + 18.0);
        ed->endTextEdit();
    }

    // The saved file doesn't write a bullet (or the lack of one) on an empty paragraph of a slide's placeholder: it takes the layout's, as a
    // new slide of the presentation program does, and the slide master's text styles say the bullets (font, color, size, character) for the
    // program to take. What is typed says its own bullet. Read again, the empty paragraph has the layout's list.
    void newSlidesWriteNoBulletOfTheirOwn()
    {
        auto d = bulletedDeck();
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        auto bulletsOfParagraphs = [](const QByteArray &xml) {
            QDomDocument dom;
            QVector<QStringList> out;
            std::function<void(const QDomElement &)> visit = [&](const QDomElement &el) {
                if (office::isElement(el, office::ns::A, QStringLiteral("p"))) {
                    QStringList names;
                    for (const QDomElement &k : office::children(office::child(el, office::ns::A, QStringLiteral("pPr")))) names << k.localName();
                    out << names.filter(QRegularExpression(QStringLiteral("^bu")));
                    return;
                }
                for (const QDomElement &k : office::children(el)) visit(k);
            };
            visit(rootOf(xml, &dom));
            return out;
        };
        const QByteArray slide = e.zip.value(QStringLiteral("ppt/slides/slide2.xml"));
        QCOMPARE(bulletsOfParagraphs(slide), (QVector<QStringList>{QStringList(), QStringList()}));   // the title's and the body's: nothing of their own
        // The master's text styles carry the bullets of its body: the first level's in the font, color, and size the list has.
        const QString master = part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"));
        const int body = master.indexOf(QStringLiteral("<p:bodyStyle>"));
        QVERIFY(body > 0);
        const QString first = master.mid(body, master.indexOf(QStringLiteral("</a:lvl1pPr>"), body) - body);
        QVERIFY2(first.contains(QStringLiteral("<a:buFont typeface=\"Wingdings 3\"")), qPrintable(first));
        QVERIFY2(first.contains(QStringLiteral("<a:buClr><a:srgbClr val=\"90C226\"/></a:buClr>")), qPrintable(first));
        QVERIFY2(first.contains(QStringLiteral("<a:buSzPct val=\"80000\"/>")), qPrintable(first));
        QVERIFY2(first.contains(QStringLiteral("<a:buChar char=\"\"/>")), qPrintable(first));
        // The second level's too, in its own.
        const int second = master.indexOf(QStringLiteral("<a:lvl2pPr"), body);
        QVERIFY(second > 0 && master.mid(second, master.indexOf(QStringLiteral("</a:lvl2pPr>"), second) - second).contains(QStringLiteral("<a:buSzPct val=\"80000\"/>")));
        // Typed text says its bullet, as every paragraph with text does.
        QTextCursor c(d->storyDoc(bodyOf(*d->pages[1])->storyId));
        c.insertText(QStringLiteral("Typed"));
        Exported typed;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &typed, &err), qPrintable(err));
        const QVector<QStringList> written = bulletsOfParagraphs(typed.zip.value(QStringLiteral("ppt/slides/slide2.xml")));
        QCOMPARE(written.size(), 2);
        QVERIFY(written[1].contains(QStringLiteral("buChar")));
        QVERIFY(written[1].contains(QStringLiteral("buFont")));
        // Read again, a new slide's empty paragraph is left out of the list by the file, and takes the layout's when the presentation is opened
        // in a window: the font, color, and size of the bullet, and the first level's indent.
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        const TextItem *read = bodyOf(*back->pages[2]);
        QVERIFY(read);
        const QTextBlock empty = back->storyDoc(read->storyId)->begin();
        QVERIFY(back->storyDoc(read->storyId)->isEmpty());
        QVERIFY(!empty.textList());
        QCOMPARE(empty.blockFormat().leftMargin(), 27.0);   // the layout's indent, as the file gives it
        QCOMPARE(empty.charFormat().fontPointSize(), 18.0);
        deck::inheritLayoutLists(*back);
        QVERIFY(empty.textList());
        QCOMPARE(listSaid(empty), listSaid(layoutParagraph(*back, 0)));
        QCOMPARE(empty.textList()->format().stringProperty(tp::BulletFont), QStringLiteral("Wingdings 3"));
        // The paragraphs with text in them, and the title's empty one, are as they were.
        QVERIFY(back->storyDoc(bodyOf(*back->pages[1])->storyId)->begin().textList());
        QVERIFY(!back->storyDoc(placeholderOn(*back->pages[2], QStringLiteral("title"))->storyId)->begin().textList());
        // In a window it is done as the presentation opens.
        DeckWindow w;
        w.editor()->setDocument(std::move(back));
        const QTextBlock typedInto = w.editor()->doc()->storyDoc(bodyOf(*w.editor()->doc()->pages[2])->storyId)->begin();
        QCOMPARE(listSaid(typedInto), listSaid(layoutParagraph(*w.editor()->doc(), 0)));
    }

    // A package of an exported file, with the parts named in `changes` (by entry name, no leading slash) replaced.
    static QByteArray repack(const Exported &e, const QMap<QString, QByteArray> &changes)
    {
        office::PackageWriter w;
        for (const QString &name : e.pkg.partNames()) w.addPart(name, changes.contains(name.mid(1)) ? changes.value(name.mid(1)) : e.zip.value(name.mid(1)), e.pkg.contentType(name));
        QStringList sources = e.pkg.partNames();
        sources << QStringLiteral("/");
        for (const QString &source : std::as_const(sources)) {
            QVector<office::Relationship> links = e.pkg.relationships(source);
            std::sort(links.begin(), links.end(), [](const office::Relationship &a, const office::Relationship &b) { return a.id.mid(3).toInt() < b.id.mid(3).toInt(); });
            for (const office::Relationship &r : std::as_const(links)) w.addRelationship(source, r.type, r.target, r.external);
        }
        return w.finish();
    }

    // The names of the bullet settings (a:bu...) in the properties of each paragraph of a slide's XML, in order.
    static QVector<QStringList> bulletNamesOf(const QByteArray &xml)
    {
        QDomDocument dom;
        QVector<QStringList> out;
        std::function<void(const QDomElement &)> visit = [&](const QDomElement &el) {
            if (office::isElement(el, office::ns::A, QStringLiteral("p"))) {
                QStringList names;
                for (const QDomElement &k : office::children(office::child(el, office::ns::A, QStringLiteral("pPr")))) names << k.localName();
                out << names.filter(QRegularExpression(QStringLiteral("^bu")));
                return;
            }
            for (const QDomElement &k : office::children(el)) visit(k);
        };
        visit(rootOf(xml, &dom));
        return out;
    }

    // A paragraph the file marks as having no bullet (a:buNone) keeps that when it is empty: it is not given the layout's list when the
    // presentation opens (typing in it gives plain text), and it is saved with a:buNone again; an empty paragraph that says nothing still takes
    // the layout's list, and is saved saying nothing.
    void emptyParagraphsMarkedNoBulletStayWithoutBullets()
    {
        auto d = bulletedDeck();
        for (int i = 0; i < 3; ++i) deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        auto body = [](QByteArray xml, const QString &paragraphs) {
            QString text = QString::fromUtf8(xml);
            const QRegularExpression place(QStringLiteral("(<p:ph [^>]*idx=\"1\"[^>]*/>.*?<a:lstStyle/>)(.*?)(</p:txBody>)"), QRegularExpression::DotMatchesEverythingOption);
            text.replace(place, QStringLiteral("\\1") + paragraphs + QStringLiteral("\\3"));
            return text.toUtf8();
        };
        const QString none = QStringLiteral("<a:p><a:pPr marL=\"0\" indent=\"0\"><a:buNone/></a:pPr><a:endParaRPr lang=\"en-US\"/></a:p>");
        const QString said = QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>First</a:t></a:r></a:p>");
        const QString quiet = QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>");
        // Slide 2: words, an empty paragraph with no bullet, an empty one that says nothing. Slide 3: one empty paragraph with no bullet.
        // Slide 4: one that says nothing.
        QMap<QString, QByteArray> changes;
        changes[QStringLiteral("ppt/slides/slide2.xml")] = body(e.zip.value(QStringLiteral("ppt/slides/slide2.xml")), said + none + quiet);
        changes[QStringLiteral("ppt/slides/slide3.xml")] = body(e.zip.value(QStringLiteral("ppt/slides/slide3.xml")), none);
        changes[QStringLiteral("ppt/slides/slide4.xml")] = body(e.zip.value(QStringLiteral("ppt/slides/slide4.xml")), quiet);
        auto back = deck::importPresentation(repack(e, changes), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 4);
        deck::inheritLayoutLists(*back);
        const QTextDocument *two = back->storyDoc(bodyOf(*back->pages[1])->storyId);
        QCOMPARE(two->blockCount(), 3);
        QVERIFY(two->findBlockByNumber(0).textList());
        QVERIFY(!two->findBlockByNumber(1).textList());
        QVERIFY(!two->findBlockByNumber(2).textList());
        QVERIFY(!back->storyDoc(bodyOf(*back->pages[2])->storyId)->begin().textList());   // marked: no list
        QVERIFY(back->storyDoc(bodyOf(*back->pages[3])->storyId)->begin().textList());    // said nothing: the layout's
        // Saved again: a:buNone stays on the paragraphs that had it, and the ones that said nothing say nothing.
        Exported saved;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &saved, &err), qPrintable(err));
        QVERIFY2(packageProblem(saved).isEmpty(), qPrintable(packageProblem(saved)));
        const QVector<QStringList> second = bulletNamesOf(saved.zip.value(QStringLiteral("ppt/slides/slide2.xml")));
        QCOMPARE(second.size(), 4);   // the title's, then the body's three
        QVERIFY(second[1].contains(QStringLiteral("buChar")));
        QCOMPARE(second[2], QStringList{QStringLiteral("buNone")});
        QCOMPARE(second[3], QStringList());
        QCOMPARE(bulletNamesOf(saved.zip.value(QStringLiteral("ppt/slides/slide3.xml"))), (QVector<QStringList>{QStringList(), QStringList{QStringLiteral("buNone")}}));
        QCOMPARE(bulletNamesOf(saved.zip.value(QStringLiteral("ppt/slides/slide4.xml"))), (QVector<QStringList>{QStringList(), QStringList()}));
        // In a window, as the presentation opens, and once more through a save: the same.
        DeckWindow w;
        auto again = deck::importPresentation(repack(e, changes), &err);
        QVERIFY2(again, qPrintable(err));
        w.editor()->setDocument(std::move(again));
        QVERIFY(!w.editor()->doc()->storyDoc(bodyOf(*w.editor()->doc()->pages[2])->storyId)->begin().textList());
        QVERIFY(w.editor()->doc()->storyDoc(bodyOf(*w.editor()->doc()->pages[3])->storyId)->begin().textList());
        // Typing into the marked one gives plain text, not a bullet.
        QTextCursor(w.editor()->doc()->storyDoc(bodyOf(*w.editor()->doc()->pages[2])->storyId)).insertText(QStringLiteral("typed"));
        QVERIFY(!w.editor()->doc()->storyDoc(bodyOf(*w.editor()->doc()->pages[2])->storyId)->begin().textList());
    }

    // The built-in themes bullet the text of their body places as the presentation program's do: a paragraph for each of five levels in a list of
    // its own, the bullet in the first accent color, each level smaller and further in than the one before. The first theme, a new presentation's
    // own, has no bullets, as it always had none; no theme bullets a subtitle, a section's text, a heading of a comparison, or a caption.
    void builtinThemesBulletTheirBodies()
    {
        auto bulleted = [](const Document &d, const MasterPage &page, const QString &role) {
            for (const auto &it : page.items) {
                if (it->role != role || it->type() != ItemType::Text) continue;
                const QTextDocument *sd = d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
                int listed = 0;
                for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) listed += b.textList() != nullptr;
                return listed == 0 ? 0 : listed == sd->blockCount() ? 2 : 1;   // none, some, all
            }
            return -1;   // no such placeholder
        };
        for (int i = 0; i < deck::builtinThemeCount(); ++i) {
            auto d = deck::themePresentation(i);
            const QString name = deck::builtinThemeName(i);
            const deck::DeckTheme theme = deck::themeOf(*d);
            const MasterPage *master = nullptr;
            for (const auto &m : d->masters)
                if (deck::isSlideMaster(*m)) master = m.get();
            // Where the bullets are, and where they aren't.
            struct Where {
                const char *layout, *role;
                int want;
            };
            const Where places[] = {{nullptr, "ph:body:1", 2},   {"obj", "ph:obj:1", 2},   {"twoObj", "ph:obj:1", 2},   {"twoObj", "ph:obj:2", 2},
                                    {"twoTxTwoObj", "ph:obj:2", 2}, {"twoTxTwoObj", "ph:obj:4", 2}, {"objTx", "ph:obj:1", 2},  {"title", "ph:subTitle:1", 0},
                                    {"secHead", "ph:body:1", 0}, {"twoTxTwoObj", "ph:body:1", 0}, {"twoTxTwoObj", "ph:body:3", 0}, {"objTx", "ph:body:2", 0},
                                    {"obj", "ph:title", 0},      {"obj", "ph:sldNum", 0}};
            for (const Where &w : places) {
                const MasterPage &page = w.layout ? *deck::layoutOfType(*d, QString::fromLatin1(w.layout)) : *master;
                QCOMPARE(bulleted(*d, page, QString::fromLatin1(w.role)), i == 0 ? 0 : w.want);
            }
            if (i == 0) continue;
            // The levels of the slide master's body: five lists, each a level, with the same bullet in the accent color, smaller and further in.
            const MasterPage &layout = *deck::layoutOfType(*d, QStringLiteral("obj"));
            for (const MasterPage *page : {master, &layout}) {
                const TextItem *body = static_cast<const TextItem *>(page == master ? placeholderOn(*page, QStringLiteral("body")) : placeholderOn(*page, QStringLiteral("obj")));
                const QTextDocument *sd = d->storyDoc(body->storyId);
                QCOMPARE(sd->blockCount(), 5);
                double lastSize = 1000, lastMargin = -1;
                for (int level = 0; level < 5; ++level) {
                    const QTextBlock b = sd->findBlockByNumber(level);
                    const QString where = QStringLiteral("%1 level %2").arg(name).arg(level + 1);
                    QVERIFY2(b.textList(), qPrintable(where));
                    const QTextListFormat lf = b.textList()->format();
                    QCOMPARE(lf.intProperty(tp::ListLevel), level);
                    QVERIFY2(!lf.stringProperty(tp::BulletChar).isEmpty() && !lf.stringProperty(tp::BulletFont).isEmpty(), qPrintable(where));
                    QCOMPARE(ColorRef::fromString(lf.stringProperty(tp::BulletColor)).rgbValue(), theme.colors[4]);
                    QVERIFY2(b.blockFormat().leftMargin() > lastMargin, qPrintable(where));
                    QVERIFY2(b.blockFormat().textIndent() < 0, qPrintable(where));
                    const double size = b.begin().fragment().charFormat().fontPointSize();
                    QVERIFY2(size <= lastSize && size >= 14, qPrintable(where));
                    lastSize = size;
                    lastMargin = b.blockFormat().leftMargin();
                    QCOMPARE(b.blockFormat().hasProperty(tp::ListLevel), level > 0);
                }
                QVERIFY(sd->findBlockByNumber(0).begin().fragment().charFormat().fontPointSize() > lastSize);
            }
        }
    }

    // A new slide in a built-in theme is bulleted as its layout says, its levels take the layout's, the file leaves the bullet of an empty
    // paragraph to the layout, and the slide master's text styles say the bullet of each level; read again, the layouts keep their lists.
    void newSlidesOfBuiltinThemesAreBulleted()
    {
        for (int i = 1; i < deck::builtinThemeCount(); ++i) {
            auto d = deck::themePresentation(i, i % 4);
            const QString name = deck::builtinThemeName(i);
            deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
            const TextItem *body = bodyOf(*d->pages[1]);
            QVERIFY2(body, qPrintable(name));
            QCOMPARE(listSaid(d->storyDoc(body->storyId)->begin()), listSaid(layoutParagraph(*d, 0)));
            followsAccent(ColorRef::fromString(d->storyDoc(body->storyId)->begin().textList()->format().stringProperty(tp::BulletColor)).rgbValue(), deck::themeOf(*d).colors[4],
                          colorsUnder(*d, layoutOf(*d, QStringLiteral("obj")), body->rect.center()), 3.0, qPrintable(name));
            QString err;
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(name + ": " + packageProblem(e)));
            const QString slide = part(e, QStringLiteral("/ppt/slides/slide2.xml"));
            QVERIFY2(!slide.contains(QRegularExpression(QStringLiteral("<a:bu(Char|None|AutoNum|Font|Clr|Sz)"))), qPrintable(name));
            const QString master = part(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"));
            const int style = master.indexOf(QStringLiteral("<p:bodyStyle>"));
            const QString first = master.mid(style, master.indexOf(QStringLiteral("</a:lvl1pPr>"), style) - style);
            // (The bullets are the theme's first accent where it reads on the page; where it doesn't, a color of its hue that does.)
            const bool accentReads = deck::contrastRatio(deck::themeOf(*d).colors[4], colorsUnder(*d, layoutOf(*d, QStringLiteral("obj")), body->rect.center()).value(0)) >= 3.0;
            QVERIFY2(first.contains(accentReads ? QStringLiteral("<a:buClr><a:schemeClr val=\"accent1\"/></a:buClr>") : QStringLiteral("<a:buClr><a:srgbClr")) && first.contains(QStringLiteral("<a:buChar char=")), qPrintable(name + ": " + first));
            // Two levels typed in.
            QTextCursor c(d->storyDoc(body->storyId));
            c.insertText(QStringLiteral("First"));
            c.insertBlock();
            c.insertText(QStringLiteral("Second"));
            QVERIFY(deck::changeListLevel(*d, *body, 1, 1, 1));
            QCOMPARE(listSaid(d->storyDoc(body->storyId)->findBlockByNumber(1)), listSaid(layoutParagraph(*d, 1)));
            QCOMPARE(d->storyDoc(body->storyId)->findBlockByNumber(1).begin().fragment().charFormat().fontPointSize(), layoutParagraph(*d, 1).begin().fragment().charFormat().fontPointSize());
            // Saved and read again: the layouts' paragraphs are in their lists as they were, and the slide's text.
            auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
            QVERIFY2(back, qPrintable(err));
            const QTextDocument *was = d->storyDoc(layoutOf(*d, QStringLiteral("obj")).items.front() ? static_cast<const TextItem *>(placeholderOn(layoutOf(*d, QStringLiteral("obj")), QStringLiteral("obj")))->storyId : QString());
            const QTextDocument *now = back->storyDoc(static_cast<const TextItem *>(placeholderOn(layoutOf(*back, QStringLiteral("obj")), QStringLiteral("obj")))->storyId);
            QCOMPARE(now->blockCount(), was->blockCount());
            for (int level = 0; level < 5; ++level) QCOMPARE(listSaid(now->findBlockByNumber(level)).toLower(), listSaid(was->findBlockByNumber(level)).toLower());
            const QTextDocument *typed = back->storyDoc(bodyOf(*back->pages[1])->storyId);
            QCOMPARE(typed->findBlockByNumber(1).textList()->format().intProperty(tp::ListLevel), 1);
            QCOMPARE(typed->findBlockByNumber(1).blockFormat().leftMargin(), d->storyDoc(body->storyId)->findBlockByNumber(1).blockFormat().leftMargin());
        }
    }

    // A theme with bullets, applied to a presentation whose text had none from its layout, bullets it, as the presentation program does: the
    // paragraphs that were as the old layout's go to the new layout's, with list, indent, and size; those set another way stay; and another
    // theme without bullets takes them away again.
    void applyingABulletedThemeBulletsTheText()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        const TextItem *body = bodyOf(*d->pages[1]);
        QTextDocument *sd = d->storyDoc(body->storyId);
        QTextCursor c(sd);
        c.insertText(QStringLiteral("Plain"));
        c.insertBlock();
        c.insertText(QStringLiteral("Centered by hand"));
        c.insertBlock();
        c.insertText(QStringLiteral("Bulleted by hand"));
        {
            QTextBlockFormat bf;
            bf.setAlignment(Qt::AlignHCenter);
            QTextCursor(sd->findBlockByNumber(1)).mergeBlockFormat(bf);
            QTextBlock hand = sd->findBlockByNumber(2);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListSquare);
            QTextCursor(hand).createList(lf);
        }
        const int harbor = deck::builtinThemeIndex(QStringLiteral("Harbor"));
        auto source = deck::themePresentation(harbor);
        const deck::DeckTheme theme = deck::themeOf(*source);
        QVERIFY(deck::applyThemeToAll(*d, *source));
        const QTextDocument *after = d->storyDoc(bodyOf(*d->pages[1])->storyId);
        const MasterPage &layout = *d->masterFor(*d->pages[1]);
        const QTextBlock first = layoutParagraph(*d, 0);
        Q_UNUSED(layout);
        QVERIFY(after->findBlockByNumber(0).textList());
        QCOMPARE(listSaid(after->findBlockByNumber(0)), listSaid(first));
        QCOMPARE(after->findBlockByNumber(0).blockFormat().leftMargin(), first.blockFormat().leftMargin());
        QCOMPARE(after->findBlockByNumber(0).begin().fragment().charFormat().fontPointSize(), first.begin().fragment().charFormat().fontPointSize());
        QCOMPARE(ColorRef::fromString(after->findBlockByNumber(0).textList()->format().stringProperty(tp::BulletColor)).rgbValue(), theme.colors[4]);
        // The paragraph centered by hand stays centered, and has the bullet; the one with a bullet of its own keeps it.
        QVERIFY(after->findBlockByNumber(1).textList());
        QCOMPARE(after->findBlockByNumber(1).blockFormat().alignment(), Qt::AlignHCenter);
        QCOMPARE(int(after->findBlockByNumber(2).textList()->format().style()), int(QTextListFormat::ListSquare));
        QVERIFY(after->findBlockByNumber(2).textList()->format().stringProperty(tp::BulletChar).isEmpty());
        // The first theme again, with none: the paragraphs that followed take its none.
        QVERIFY(deck::applyThemeToAll(*d, *deck::themePresentation(0)));
        const QTextDocument *plain = d->storyDoc(bodyOf(*d->pages[1])->storyId);
        QVERIFY(!plain->findBlockByNumber(0).textList());
        QCOMPARE(plain->findBlockByNumber(0).blockFormat().leftMargin(), 0.0);
        QVERIFY(plain->findBlockByNumber(2).textList());   // the hand-made one stays
    }

    // The colors of bullets follow a change of colors, in the slides and in the layouts and slide master.
    void bulletColorsFollowTheColors()
    {
        auto d = deck::themePresentation(deck::builtinThemeIndex(QStringLiteral("Harbor")));
        deck::addSlide(*d, layoutOf(*d, QStringLiteral("obj")));
        const TextItem *body = bodyOf(*d->pages[1]);
        QTextCursor(d->storyDoc(body->storyId)).insertText(QStringLiteral("Text"));
        const deck::ColorSet set = deck::colorSets().value(4);   // Ember's
        deck::applyColors(*d, set);
        auto color = [](const QTextBlock &b) { return ColorRef::fromString(b.textList()->format().stringProperty(tp::BulletColor)).rgbValue(); };
        const QVector<QColor> behind = colorsUnder(*d, layoutOf(*d, QStringLiteral("obj")), bodyOf(layoutOf(*d, QStringLiteral("obj")))->rect.center());
        followsAccent(color(d->storyDoc(bodyOf(*d->pages[1])->storyId)->begin()), set.colors[4], behind, 3.0, "slide");
        followsAccent(color(layoutParagraph(*d, 0)), set.colors[4], behind, 3.0, "layout level 1");
        followsAccent(color(layoutParagraph(*d, 3)), set.colors[4], behind, 3.0, "layout level 4");
        // And a dark background keeps them (accent colors don't turn), readable on it.
        deck::applyBackgroundStyle(*d, 9);
        for (const QColor &bg : colorsUnder(*d, layoutOf(*d, QStringLiteral("obj")), bodyOf(layoutOf(*d, QStringLiteral("obj")))->rect.center()))
            QVERIFY(deck::contrastRatio(color(layoutParagraph(*d, 0)), bg) >= 3.0);
        QVERIFY(std::abs(color(layoutParagraph(*d, 0)).hslHue() - set.colors[4].hslHue()) <= 15);
    }

    // A theme's picture fills (its textures) point at pictures the reader did not keep:
    // written as they were, they would point nowhere, so they become plain fills.
    void exportLeavesOutThePicturesOfATheme()
    {
        Parts p = samplePptx();
        QByteArray theme = p.data["/ppt/theme/theme1.xml"].xml;
        const QByteArray texture =
            "<a:blipFill dpi=\"0\" rotWithShape=\"1\"><a:blip xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:embed=\"rId1\">"
            "<a:duotone><a:schemeClr val=\"phClr\"/></a:duotone></a:blip><a:srcRect/><a:tile tx=\"0\" ty=\"0\" sx=\"100000\" sy=\"100000\" flip=\"none\" algn=\"tl\"/></a:blipFill>";
        theme.replace("<a:bgFillStyleLst>", "<a:bgFillStyleLst>" + texture);
        p.data["/ppt/theme/theme1.xml"].xml = theme;
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        // The rest of the part is as it was.
        QCOMPARE(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), QByteArray(theme).replace(texture, "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"));
        office::Theme read;
        QVERIFY2(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &read, &err), qPrintable(err));
        QCOMPARE(read.colors[4], QColor(0xB6, 0x42, 0x0F));
        // The same for the theme of a second slide master, which is kept in its master's properties.
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(theme.toBase64());
        d->masters << second;
        Exported two;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &two, &err), qPrintable(err));
        QVERIFY2(packageProblem(two).isEmpty(), qPrintable(packageProblem(two)));
        QVERIFY(!part(two, QStringLiteral("/ppt/theme/theme2.xml")).contains(QStringLiteral("embed")));
        // A theme that points at something else that isn't kept is made again from its colors and fonts.
        QByteArray odd = QByteArray(theme).replace("<a:objectDefaults/>", "");
        odd.replace("</a:themeElements>", "</a:themeElements><a:objectDefaults><a:spDef><a:spPr/><a:bodyPr/><a:lstStyle/><a:style><a:lnRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:lnRef>"
                                          "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef><a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef>"
                                          "<a:fontRef idx=\"minor\"><a:hlinkClick xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId5\"/></a:fontRef></a:style></a:spDef></a:objectDefaults>");
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(odd.toBase64());
        Exported three;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &three, &err), qPrintable(err));
        QVERIFY2(packageProblem(three).isEmpty(), qPrintable(packageProblem(three)));
        QVERIFY2(office::readTheme(three.zip.value(QStringLiteral("ppt/theme/theme2.xml")), &read, &err), qPrintable(err));
        QCOMPARE(read.colors[4], QColor(0xB6, 0x42, 0x0F));
    }

    // The same presentation as a template or a show differs in one content type.
    void exportKindsDifferInTheMainPartsType()
    {
        auto d = deck::newPresentation();
        QString err;
        const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.");
        const struct { deck::PresentationKind kind; const char *type; } kinds[] = {
            {deck::PresentationKind::Presentation, "presentation.main+xml"}, {deck::PresentationKind::Template, "template.main+xml"}, {deck::PresentationKind::Show, "slideshow.main+xml"}};
        for (const auto &k : kinds) {
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*d, &err, nullptr, k.kind), &e, &err), qPrintable(err));
            QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/presentation.xml")), base + QLatin1String(k.type));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        }
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.pptx")), deck::PresentationKind::Presentation);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.POTX")), deck::PresentationKind::Template);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.ppsx")), deck::PresentationKind::Show);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a")), deck::PresentationKind::Presentation);
    }

    // Sections are written as the extension that lists each section's slides
    // by the numbers the slide list gives them, and read back the same (an
    // empty section too); a presentation without sections writes no list. A
    // file's slides that no section lists join the section before them, and
    // those before the first section's are in a Default Section.
    void sectionsAreWrittenAndReadBack()
    {
        const QString ext = QStringLiteral("{521415D9-36F7-43E2-AB2F-B90AF26B5E84}");
        const QString p14 = QStringLiteral("http://schemas.microsoft.com/office/powerpoint/2010/main");
        auto d = lettered(5);
        deck::addSection(*d, 2, QStringLiteral("Plan & Goals"));
        deck::addSection(*d, 4, QStringLiteral("Budget"));
        deck::addSection(*d, 4, QStringLiteral("Last"));   // leaves Budget with no slides
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2, 0, 1}));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QDomDocument dom;
        const QDomElement root = rootOf(e.zip.value(QStringLiteral("ppt/presentation.xml")), &dom);
        QStringList slideIds;
        for (const QDomElement &s : office::children(office::child(root, office::ns::P, QStringLiteral("sldIdLst")), office::ns::P, QStringLiteral("sldId"))) slideIds << office::attr(s, QStringLiteral("id"));
        QCOMPARE(int(slideIds.size()), 5);
        // After the default text style, as the schema has it: the extension list is the last child.
        QCOMPARE(root.lastChildElement().localName(), QStringLiteral("extLst"));
        const QDomElement extension = office::child(office::child(root, office::ns::P, QStringLiteral("extLst")), office::ns::P, QStringLiteral("ext"));
        QCOMPARE(extension.attribute(QStringLiteral("uri")), ext);
        const QVector<QDomElement> written = office::children(office::child(extension, p14, QStringLiteral("sectionLst")), p14, QStringLiteral("section"));
        QCOMPARE(int(written.size()), 4);
        const QVector<deck::Section> sections = deck::sectionsOf(*d);
        int next = 0;
        for (int i = 0; i < written.size(); ++i) {
            QCOMPARE(office::attr(written[i], QStringLiteral("name")), sections[i].name);
            QCOMPARE(office::attr(written[i], QStringLiteral("id")), sections[i].id);
            QStringList listed;
            for (const QDomElement &s : office::children(office::child(written[i], p14, QStringLiteral("sldIdLst")), p14, QStringLiteral("sldId"))) listed << office::attr(s, QStringLiteral("id"));
            QCOMPARE(listed, slideIds.mid(next, sections[i].count));
            next += sections[i].count;
        }
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 5);
        QCOMPARE(sectionNames(*back), (QStringList{"Default Section", "Plan & Goals", "Budget", "Last"}));
        QCOMPARE(sectionCounts(*back), (QList<int>{2, 2, 0, 1}));
        for (int i = 0; i < 4; ++i) QCOMPARE(deck::sectionsOf(*back)[i].id, sections[i].id);
        // Saved again, they are the same.
        Exported again;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &again, &err), qPrintable(err));
        QCOMPARE(again.zip.value(QStringLiteral("ppt/presentation.xml")), e.zip.value(QStringLiteral("ppt/presentation.xml")));

        // Without sections nothing is written.
        auto plain = lettered(3);
        Exported none;
        QVERIFY2(unpack(deck::exportPresentation(*plain, &err), &none, &err), qPrintable(err));
        QVERIFY(!part(none, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("sectionLst")));
        QVERIFY(!part(none, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("extLst")));
        auto plainBack = deck::importPresentation(deck::exportPresentation(*plain, &err), &err);
        QVERIFY2(plainBack, qPrintable(err));
        QVERIFY(!deck::hasSections(*plainBack));

        // A file's own lists: ids the file doesn't have are passed over, a slide in none joins the section before it.
        auto edited = [&](const QString &sections) {
            QMap<QString, QByteArray> zip = e.zip;
            QString xml = QString::fromUtf8(zip.value(QStringLiteral("ppt/presentation.xml")));
            const qsizetype from = xml.indexOf(QStringLiteral("<p:extLst>")), to = xml.indexOf(QStringLiteral("</p:extLst>")) + 11;
            xml.replace(from, to - from, sections.isEmpty() ? QString() : QStringLiteral("<p:extLst><p:ext uri=\"%1\"><p14:sectionLst xmlns:p14=\"%2\">%3</p14:sectionLst></p:ext></p:extLst>").arg(ext, p14, sections));
            zip[QStringLiteral("ppt/presentation.xml")] = xml.toUtf8();
            ZipWriter z;
            for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
            return deck::importPresentation(z.finish(), &err);
        };
        auto section = [&](const QString &name, const QStringList &ids) {
            QString list;
            for (const QString &id : ids) list += QStringLiteral("<p14:sldId id=\"%1\"/>").arg(id);
            return QStringLiteral("<p14:section name=\"%1\" id=\"{00000000-0000-4000-8000-00000000000%2}\"><p14:sldIdLst>%3</p14:sldIdLst></p14:section>").arg(name).arg(name.size()).arg(list);
        };
        auto odd = edited(section(QStringLiteral("Opening"), {slideIds[0], QStringLiteral("999")}) + section(QStringLiteral("Middle"), {slideIds[2], slideIds[3]})
                          + section(QStringLiteral("Closing"), {slideIds[4]}) + section(QStringLiteral("Epilogue"), {}));
        QVERIFY2(odd, qPrintable(err));
        QCOMPARE(sectionNames(*odd), (QStringList{"Opening", "Middle", "Closing", "Epilogue"}));
        QCOMPARE(sectionCounts(*odd), (QList<int>{2, 2, 1, 0}));   // the second slide is in none: it joins Opening
        QCOMPARE(deck::sectionsOf(*odd)[1].id, QStringLiteral("{00000000-0000-4000-8000-000000000006}"));
        // Slides in none before the first section's get a Default Section of their own.
        auto late = edited(section(QStringLiteral("Only"), {slideIds[1], slideIds[2], slideIds[3], slideIds[4]}));
        QVERIFY2(late, qPrintable(err));
        QCOMPARE(sectionNames(*late), (QStringList{"Default Section", "Only"}));
        QCOMPARE(sectionCounts(*late), (QList<int>{1, 4}));
        // A list with no sections, or no list, is no sections.
        auto empty = edited(QString());
        QVERIFY2(empty && !deck::hasSections(*empty), qPrintable(err));
        auto emptyList = edited(QStringLiteral(" "));
        QVERIFY2(emptyList && !deck::hasSections(*emptyList), qPrintable(err));
    }
};

JP_TEST_CLASS(DeckThemeTests)
#include "tst_deck_themes.moc"
