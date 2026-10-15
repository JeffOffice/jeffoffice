// The text of an open document presentation's objects (odpexport_p.h):
// paragraphs with their styles, runs with theirs, bullets and numbers as lists
// with a list style for each run of them, and the fields (the slide number,
// the date).

#include "io/odpexport_p.h"

#include "core/fonts.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QFont>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextList>
#include <QTextOption>
#include <algorithm>
#include <cmath>

namespace jp::deck::odpw {

namespace {

constexpr int kLevels = 9;

QString attrOf(const char *name, const QString &value) { return QLatin1Char(' ') + QLatin1String(name) + QStringLiteral("=\"") + esc(value) + QLatin1Char('"'); }

QString pt(double v) { return number(v, 2) + QLatin1String("pt"); }

int levelOf(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    QTextList *list = b.textList();
    int level = bf.hasProperty(tp::ListLevel) ? bf.intProperty(tp::ListLevel) : list ? list->format().intProperty(tp::ListLevel) : 0;
    return std::clamp(level, 0, kLevels - 1);
}

// The size of the first letters of a block, for a bullet that is a share of it.
double firstSizeOf(const QTextBlock &b)
{
    for (auto it = b.begin(); !it.atEnd(); ++it)
        if (it.fragment().isValid() && it.fragment().charFormat().hasProperty(QTextFormat::FontPointSize)) return it.fragment().charFormat().fontPointSize();
    return b.charFormat().hasProperty(QTextFormat::FontPointSize) ? b.charFormat().fontPointSize() : 11.0;
}

// The numbering the library counts with, as the format letter the standard has, and its punctuation.
void numbering(const QTextListFormat &lf, QString *format, QString *prefix, QString *suffix)
{
    switch (lf.style()) {
    case QTextListFormat::ListLowerAlpha: *format = QStringLiteral("a"); break;
    case QTextListFormat::ListUpperAlpha: *format = QStringLiteral("A"); break;
    case QTextListFormat::ListLowerRoman: *format = QStringLiteral("i"); break;
    case QTextListFormat::ListUpperRoman: *format = QStringLiteral("I"); break;
    default: *format = QStringLiteral("1"); break;
    }
    *prefix = lf.numberPrefix();
    *suffix = lf.numberSuffix();
    if (prefix->isEmpty() && suffix->isEmpty()) {
        switch (lf.intProperty(tp::NumberFormat)) {
        case 1: case 2: case 3: case 4: case 5: *suffix = QStringLiteral("."); break;
        case 6: *suffix = QStringLiteral(")"); break;
        case 7: *prefix = QStringLiteral("("); *suffix = QStringLiteral(")"); break;
        default: break;
        }
    }
}

} // namespace

QString Exporter::fontName(const QTextCharFormat &cf) const
{
    QStringList families = cf.fontFamilies().toStringList();
    families.removeAll(QString());
    if (!families.isEmpty()) return families.first();
    if (const QString one = cf.stringProperty(QTextFormat::FontFamily); !one.isEmpty()) return one;
    if (cf.stringProperty(tp::ThemeFont) == QLatin1String("major")) return m_fonts.heading;
    return m_fonts.body;
}

QColor Exporter::textColor(const QTextCharFormat &cf, const QColor &) const
{
    const QString ref = cf.stringProperty(tp::ColorRefP);
    if (!ref.isEmpty()) return resolve(ColorRef::fromString(ref));
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() != Qt::NoBrush) {
        const QColor c = cf.foreground().color();
        return c.alpha() == 0 ? QColor() : QColor(c.red(), c.green(), c.blue());
    }
    if (cf.isAnchor() && !cf.anchorHref().isEmpty()) return m_colors.slot(Hyperlink);
    return m_colors.slot(Main);
}

QString Exporter::charAttrs(const QTextCharFormat &cf, bool)
{
    QString out;
    const QString font = fontDecl(fontName(cf));
    if (!font.isEmpty()) {
        out += attrOf("style:font-name", font);
        QStringList families = cf.fontFamilies().toStringList();
        families.removeAll(QString());
        if (families.size() > 1) out += attrOf("style:font-name-asian", fontDecl(families[1]));
        if (families.size() > 2) out += attrOf("style:font-name-complex", fontDecl(families[2]));
    }
    // (Text the presentation program shrank is written at the whole-point size it draws it at, and only its size is shrunk.)
    const double fileSize = std::clamp(cf.hasProperty(QTextFormat::FontPointSize) ? cf.fontPointSize() : 11.0, 1.0, 1638.0);
    const double size = std::clamp(slideShrunkSize(fileSize, m_fitScale), 1.0, 1638.0);
    out += attrOf("fo:font-size", pt(size)) + attrOf("style:font-size-asian", pt(size)) + attrOf("style:font-size-complex", pt(size));
    // The weight: the two usual ones by name, the others (light, medium, semibold...) by their number.
    const int w = std::clamp(int(std::lround(cf.fontWeight() / 100.0)) * 100, 100, 900);
    const QString weight = w == 400 ? QStringLiteral("normal") : w == 700 ? QStringLiteral("bold") : QString::number(w);
    const QString slant = cf.fontItalic() ? QStringLiteral("italic") : QStringLiteral("normal");
    out += attrOf("fo:font-weight", weight) + attrOf("style:font-weight-asian", weight) + attrOf("style:font-weight-complex", weight);
    out += attrOf("fo:font-style", slant) + attrOf("style:font-style-asian", slant) + attrOf("style:font-style-complex", slant);
    const QColor color = textColor(cf, QColor());
    if (color.isValid()) out += attrOf("fo:color", hexOf(color)) + attrOf("style:use-window-font-color", QStringLiteral("false"));
    // The underline, and the kinds beside it.
    const int kind = cf.intProperty(tp::UnderlineKind);
    QString underline = QStringLiteral("none");
    switch (cf.underlineStyle()) {
    case QTextCharFormat::NoUnderline: break;
    case QTextCharFormat::DashUnderline: underline = QStringLiteral("dash"); break;
    case QTextCharFormat::DotLine: underline = QStringLiteral("dotted"); break;
    case QTextCharFormat::DashDotLine: underline = QStringLiteral("dot-dash"); break;
    case QTextCharFormat::DashDotDotLine: underline = QStringLiteral("dot-dot-dash"); break;
    case QTextCharFormat::WaveUnderline: underline = QStringLiteral("wave"); break;
    default: underline = QStringLiteral("solid"); break;
    }
    if (cf.isAnchor() && underline == QLatin1String("none") && cf.stringProperty(tp::ColorRefP).isEmpty()) underline = QStringLiteral("solid");
    out += attrOf("style:text-underline-style", underline);
    if (underline != QLatin1String("none")) {
        out += attrOf("style:text-underline-width", kind & 4 ? QStringLiteral("bold") : QStringLiteral("auto")) + attrOf("style:text-underline-color", QStringLiteral("font-color"));
        out += attrOf("style:text-underline-type", kind & 1 ? QStringLiteral("double") : QStringLiteral("single"));
        if (kind & 2) out += attrOf("style:text-underline-mode", QStringLiteral("skip-white-space"));
    }
    if (cf.fontStrikeOut()) out += attrOf("style:text-line-through-style", QStringLiteral("solid")) + attrOf("style:text-line-through-type", QStringLiteral("single"));
    else out += attrOf("style:text-line-through-style", QStringLiteral("none"));
    if (cf.fontCapitalization() == QFont::AllUppercase) out += attrOf("fo:text-transform", QStringLiteral("uppercase"));
    else if (cf.fontCapitalization() == QFont::AllLowercase) out += attrOf("fo:text-transform", QStringLiteral("lowercase"));
    else if (cf.fontCapitalization() == QFont::SmallCaps || cf.boolProperty(tp::TrueSmallCaps)) out += attrOf("fo:font-variant", QStringLiteral("small-caps"));
    else out += attrOf("fo:text-transform", QStringLiteral("none"));
    if (const double spacing = tp::kerningOf(cf); spacing != 0 && std::isfinite(spacing)) out += attrOf("fo:letter-spacing", cm(spacing));
    // Pairs of letters are kerned from a size up (14 points unless the text says otherwise), as the text is set here: by the size it is
    // drawn at, which is the size it is written at.
    {
        const double own = slideShrunkSize(std::clamp(cf.hasProperty(QTextFormat::FontPointSize) ? cf.fontPointSize() : 11.0, 0.5, 1638.0), m_fitScale);
        const double kernFrom = cf.hasProperty(tp::KernAbove) ? cf.property(tp::KernAbove).toDouble() : 14.0;
        const bool kerned = (!cf.hasProperty(QTextFormat::FontKerning) || cf.fontKerning()) && own >= kernFrom;
        out += attrOf("style:letter-kerning", kerned ? QStringLiteral("true") : QStringLiteral("false"));
    }
    if (cf.verticalAlignment() == QTextCharFormat::AlignSuperScript) out += attrOf("style:text-position", QStringLiteral("super 58%"));
    else if (cf.verticalAlignment() == QTextCharFormat::AlignSubScript) out += attrOf("style:text-position", QStringLiteral("sub 58%"));
    else out += attrOf("style:text-position", QStringLiteral("0% 100%"));
    if (const QString ref = cf.stringProperty(tp::HighlightRefP); !ref.isEmpty()) {
        const QColor mark = resolve(ColorRef::fromString(ref));
        if (mark.isValid()) out += attrOf("fo:background-color", hexOf(mark));
    }
    if (cf.boolProperty(tp::Shadow)) out += attrOf("fo:text-shadow", QStringLiteral("1pt 1pt"));
    if (cf.boolProperty(tp::Emboss)) out += attrOf("style:font-relief", QStringLiteral("embossed"));
    else if (cf.boolProperty(tp::Engrave)) out += attrOf("style:font-relief", QStringLiteral("engraved"));
    if (cf.boolProperty(tp::NoProof)) {
        out += attrOf("fo:language", QStringLiteral("zxx")) + attrOf("fo:country", QStringLiteral("none"));
    } else if (const QString lang = cf.stringProperty(tp::Language); !lang.isEmpty()) {
        out += attrOf("fo:language", lang.section(QLatin1Char('-'), 0, 0));
        if (lang.contains(QLatin1Char('-'))) out += attrOf("fo:country", lang.section(QLatin1Char('-'), 1, 1));
    }
    return out;
}

QString Exporter::textStyleOf(Scope &sc, const QTextCharFormat &cf, bool field)
{
    return sc.styles->style(QStringLiteral("text"), sc.master ? QStringLiteral("MT") : QStringLiteral("T"), QString(), QStringLiteral("<style:text-properties%1/>").arg(charAttrs(cf, field)));
}

// The share a paragraph's format says (100 when it says none).
static double shareOf(const QTextBlockFormat &bf)
{
    return std::clamp(bf.hasProperty(QTextFormat::LineHeight) && bf.lineHeight() > 0 && bf.lineHeightType() == QTextBlockFormat::ProportionalHeight ? bf.lineHeight() : 100.0, 1.0, 1000.0);
}

QString Exporter::paraAttrs(const QTextBlockFormat &bf, bool inList)
{
    QString out;
    // Alignment: a right-to-left paragraph's left and right are its start and end, unless said to be the page's.
    const Qt::Alignment al = bf.alignment() & Qt::AlignHorizontal_Mask;
    const bool absolute = bf.alignment() & Qt::AlignAbsolute;
    QString align = QStringLiteral("start");
    if (al & Qt::AlignHCenter) align = QStringLiteral("center");
    else if (al & Qt::AlignRight) align = absolute || bf.layoutDirection() == Qt::LeftToRight ? QStringLiteral("right") : QStringLiteral("end");
    else if (al & Qt::AlignJustify) align = QStringLiteral("justify");
    else if (absolute) align = QStringLiteral("left");
    out += attrOf("fo:text-align", align);
    if (bf.layoutDirection() == Qt::RightToLeft) out += attrOf("style:writing-mode", QStringLiteral("rl-tb"));
    // Indents: a list's are the list style's, the paragraph says none.
    double marL = bf.leftMargin(), indent = bf.textIndent();
    if (inList) {
        marL = 0;
        indent = 0;
    }
    out += attrOf("fo:margin-left", cm(std::clamp(marL, 0.0, 4000.0))) + attrOf("fo:margin-right", cm(std::clamp(bf.rightMargin(), 0.0, 4000.0)));
    out += attrOf("fo:text-indent", cm(std::clamp(indent, -4000.0, 4000.0)));
    // (The first paragraph of a box has no space before it, on a slide, and so in the other suite.)
    out += attrOf("fo:margin-top", cm(std::max(0.0, bf.topMargin()))) + attrOf("fo:margin-bottom", cm(std::max(0.0, bf.bottomMargin())));
    // A line is as tall as the paragraph's share of what a slide sets: 1.2 times the text (the frame's text style says so).
    if (bf.hasProperty(QTextFormat::LineHeight) && bf.lineHeight() > 0 && bf.lineHeightType() == QTextBlockFormat::FixedHeight)
        out += attrOf("fo:line-height", cm(bf.lineHeight()));
    else
        out += attrOf("fo:line-height", number(m_fitCut > 0 ? std::max(1.0, shareOf(bf) - m_fitCut) : shareOf(bf), 1) + QLatin1Char('%'));
    if (bf.boolProperty(tp::KeepTogether)) out += attrOf("fo:keep-together", QStringLiteral("always"));
    // How far a tab goes where there is no stop of its own.
    if (m_doc.defaultTabStop >= 1) out += attrOf("loext:tab-stop-distance", cm(std::clamp(m_doc.defaultTabStop, 1.0, 4000.0)));
    return out;
}

// The paragraph properties of a block's format: the tab stops are children.
QString Exporter::paraProps(const QTextBlockFormat &bf, bool inList)
{
    QString props = QStringLiteral("<style:paragraph-properties%1").arg(paraAttrs(bf, inList));
    const QList<QTextOption::Tab> tabs = bf.tabPositions();
    if (tabs.isEmpty()) return props + QStringLiteral("/>");
    props += QStringLiteral("><style:tab-stops>");
    for (int i = 0; i < tabs.size() && i < 32; ++i) {
        const QString type = tabs[i].type == QTextOption::RightTab ? QStringLiteral("right") : tabs[i].type == QTextOption::CenterTab ? QStringLiteral("center") : tabs[i].type == QTextOption::DelimiterTab ? QStringLiteral("char") : QStringLiteral("left");
        props += QStringLiteral("<style:tab-stop style:position=\"%1\" style:type=\"%2\"/>").arg(cm(std::clamp(tabs[i].position, 0.0, 4000.0)), type);
    }
    return props + QStringLiteral("</style:tab-stops></style:paragraph-properties>");
}


QString Exporter::slideLinesStyle(Scope &sc)
{
    return sc.styles->style(QStringLiteral("paragraph"), sc.master ? QStringLiteral("MPf") : QStringLiteral("Pf"), QString(), QStringLiteral("<style:paragraph-properties style:font-independent-line-spacing=\"true\"/>"));
}

// A paragraph that hangs (its first line stands out to the left of the rest) and starts with a tab: the tab goes to where the other lines
// begin, so the first line begins there too. The other suite puts that tab on the next stop a whole step on, and has no stop at the indent
// to put it on that this program reads as it is meant; so the tab is left out and the first line is not set out to the left.
static bool startsAtTheIndent(const QTextBlock &b, bool inList)
{
    if (inList || b.blockFormat().textIndent() > -0.5) return false;
    const QString text = b.text();
    if (text.size() < 2 || text[0] != QLatin1Char('\t') || text[1] == QLatin1Char('\t')) return false;
    // No stop of its own before the indent that the tab would go to.
    const double indent = b.blockFormat().leftMargin();
    for (const QTextOption::Tab &t : b.blockFormat().tabPositions())
        if (t.position < indent - 0.01 && t.position > indent + b.blockFormat().textIndent() + 0.01) return false;
    return true;
}

QString Exporter::paraStyleOf(Scope &sc, const QTextBlock &b, bool inList, const TextOpts &opts)
{
    // The format of the paragraph's mark says how tall an empty one is.
    QTextBlockFormat bf = b.blockFormat();
    // The space before and after: points as written, a share of the line at the size the text was shrunk to (as it is laid out here).
    bf.setTopMargin(slideSpaceOf(b, tp::SpaceBeforeShare, m_fitScale));
    bf.setBottomMargin(slideSpaceOf(b, tp::SpaceAfterShare, m_fitScale));
    if (startsAtTheIndent(b, inList)) bf.setTextIndent(0);
    const QString props = paraProps(bf, inList) + QStringLiteral("<style:text-properties%1/>").arg(charAttrs(b.charFormat()));
    return sc.styles->style(QStringLiteral("paragraph"), sc.master ? QStringLiteral("MP") : QStringLiteral("P"), QString(), props);
}

// ---- lists

QString Exporter::listLevelXml(int level, const QTextBlockFormat &bf, const QTextListFormat &lf, double firstSize, double *outMarL, double *outIndent)
{
    double marL = bf.leftMargin(), indent = bf.textIndent();
    // A list whose text starts at a place of its own: the paragraph hangs from there.
    if (lf.hasProperty(tp::BulletTab)) {
        const double stop = lf.property(tp::BulletTab).toDouble();
        indent = marL + indent - stop;
        marL = stop;
    }
    marL = std::clamp(marL, 0.0, 4000.0);
    indent = std::clamp(indent, -4000.0, 4000.0);
    if (outMarL) *outMarL = marL;
    if (outIndent) *outIndent = indent;

    QString custom = bf.stringProperty(tp::BulletChar);
    if (custom.isEmpty()) custom = lf.stringProperty(tp::BulletChar);
    const QTextListFormat::Style style = lf.style();
    const bool bullet = !custom.isEmpty() || isBulletList(style) || style == QTextListFormat::ListStyleUndefined;
    QString face = lf.stringProperty(tp::BulletFont);

    // Where the label stands and how wide it is, as the other suite's slides have it: the label starts `space-before` in from the left of the
    // frame, and the text starts a label's width after that (a paragraph's own margins are none).
    // (A first line set in from the rest has the label where it starts, with no width to hang in.)
    const double labelAt = marL + indent, labelWidth = indent < 0 ? -indent : 0;
    const QString label = QStringLiteral("<style:list-level-properties text:space-before=\"%1\" text:min-label-width=\"%2\"/>").arg(cm(std::max(0.0, labelAt)), cm(labelWidth));
    QString textProps;
    if (const QString ref = lf.stringProperty(tp::BulletColor); !ref.isEmpty() && resolve(ColorRef::fromString(ref)).isValid())
        textProps += attrOf("fo:color", hexOf(resolve(ColorRef::fromString(ref)))) + attrOf("style:use-window-font-color", QStringLiteral("false"));
    else
        textProps += attrOf("style:use-window-font-color", QStringLiteral("true"));
    QString open;
    // The size of the label is a share of the text's; the other suite makes a bullet small where none is said.
    double share = 1.0;
    if (const double size = lf.property(tp::BulletSize).toDouble(); size > 0 && firstSize > 0) share = std::clamp(size / firstSize, 0.25, 4.0);
    if (bullet) {
        if (custom.isEmpty()) custom = style == QTextListFormat::ListCircle ? QStringLiteral("\u25E6") : style == QTextListFormat::ListSquare ? QStringLiteral("\u25AA") : QStringLiteral("\u2022");
        QString mark = custom.left(custom[0].isHighSurrogate() ? 2 : 1);
        // A letter of a symbol font is the picture it stands for, a letter of ours.
        if (!face.isEmpty() && isSymbolFont(face) && !mark.isEmpty()) {
            const QString mapped = symbolToUnicode(face, mark[0].unicode());
            if (!mapped.isEmpty()) {
                mark = mapped;
                face.clear();
            }
        }
        if (!face.isEmpty()) {
            const QString family = interchangeFontName(face);
            textProps += attrOf("fo:font-family", family.contains(QLatin1Char(' ')) ? QLatin1Char('\'') + family + QLatin1Char('\'') : family);
        }
        textProps += attrOf("fo:font-size", percent(share));
        open = QStringLiteral("<text:list-level-style-bullet text:level=\"%1\" text:bullet-char=\"%2\"").arg(level).arg(esc(mark));
        return open + QLatin1Char('>') + label + QStringLiteral("<style:text-properties%1/></text:list-level-style-bullet>").arg(textProps);
    }
    QString format, prefix, suffix;
    numbering(lf, &format, &prefix, &suffix);
    if (!face.isEmpty()) {
        const QString family = interchangeFontName(face);
        textProps += attrOf("fo:font-family", family.contains(QLatin1Char(' ')) ? QLatin1Char('\'') + family + QLatin1Char('\'') : family);
    }
    open = QStringLiteral("<text:list-level-style-number text:level=\"%1\"").arg(level) + attrOf("style:num-format", format);
    if (!prefix.isEmpty()) open += attrOf("style:num-prefix", prefix);
    if (!suffix.isEmpty()) open += attrOf("style:num-suffix", suffix);
    if (lf.start() > 1) open += attrOf("text:start-value", QString::number(std::clamp(lf.start(), 1, 32767)));
    return open + QLatin1Char('>') + label + QStringLiteral("<style:text-properties%1/></text:list-level-style-number>").arg(textProps);
}

QString Exporter::listStyleFor(Scope &sc, const QVector<QTextBlock> &run)
{
    QString levels;
    QSet<int> seen;
    // The levels in order, each as its first paragraph has it.
    QVector<QPair<int, QTextBlock>> byLevel;
    for (const QTextBlock &b : run)
        if (!seen.contains(levelOf(b))) {
            seen.insert(levelOf(b));
            byLevel.append({levelOf(b), b});
        }
    std::sort(byLevel.begin(), byLevel.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    for (const auto &entry : std::as_const(byLevel)) levels += listLevelXml(entry.first + 1, entry.second.blockFormat(), entry.second.textList()->format(), firstSizeOf(entry.second), nullptr, nullptr);
    return sc.styles->add(sc.master ? QStringLiteral("ML") : QStringLiteral("L"), QStringLiteral("<text:list-style style:name=\""), QStringLiteral("\">") + levels + QStringLiteral("</text:list-style>"));
}

// ---- paragraphs

namespace {

// How a date or time field reads now, for the text the file shows until it is updated.
QString shownField(const QString &code)
{
    const QString kind = code.section(QLatin1Char(':'), 0, 0), format = code.section(QLatin1Char(':'), 1);
    const QDateTime now = QDateTime::currentDateTime();
    if (kind == QLatin1String("date")) return now.date().toString(format.isEmpty() ? QStringLiteral("M/d/yyyy") : format);
    if (kind == QLatin1String("time")) return now.time().toString(format.isEmpty() ? QStringLiteral("h:mm AP") : format);
    return now.toString(format.isEmpty() ? QStringLiteral("M/d/yyyy h:mm AP") : format);
}

// The state of the text of a paragraph while it is written: white space is collapsed in this kind of file, so a space after a space,
// or at the start, is written as a count.
struct Spacing {
    bool afterSpace = true;
};

void writeText(X &x, const QString &text, Spacing &sp)
{
    QString run;
    auto flush = [&] {
        if (!run.isEmpty()) x.text(run);
        run.clear();
    };
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        if (c == QLatin1Char(' ')) {
            if (sp.afterSpace) {
                // A run of spaces: the first is a space, the rest counted.
                int n = 1;
                while (i + n < text.size() && text[i + n] == QLatin1Char(' ')) ++n;
                flush();
                x.open("text:s");
                if (n > 1) x.attr("text:c", n);
                x.close();
                i += n - 1;
            } else {
                run += c;
                sp.afterSpace = true;
            }
        } else if (c == QLatin1Char('\t')) {
            flush();
            x.leaf("text:tab");
            sp.afterSpace = true;
        } else if (c == QChar(QChar::LineSeparator) || c == QLatin1Char('\n')) {
            flush();
            x.leaf("text:line-break");
            sp.afterSpace = true;
        } else {
            run += c;
            sp.afterSpace = false;
        }
    }
    flush();
}

} // namespace

void Exporter::writeParagraph(X &x, Scope &sc, const QTextBlock &b, bool inList, const TextOpts &opts)
{
    x.open("text:p");
    if (opts.shape > 0) {
        // A paragraph an animation aims at has an id of its own.
        const auto id = m_paragraphIds.constFind(qMakePair(opts.shape, b.blockNumber()));
        if (id != m_paragraphIds.cend()) x.attr("xml:id", id.value()).attr("text:id", id.value());
    }
    x.attr("text:style-name", paraStyleOf(sc, b, inList, opts));
    Spacing sp;
    bool skipTab = startsAtTheIndent(b, inList);
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (!f.isValid()) continue;
        const QTextCharFormat cf = f.charFormat();
        const QString text = f.text();
        const QString field = cf.stringProperty(tp::Field);
        QString pending;
        auto flush = [&] {
            if (pending.isEmpty()) return;
            const QString span = textStyleOf(sc, cf);
            const bool link = cf.isAnchor() && !cf.anchorHref().isEmpty();
            x.open("text:span").attr("text:style-name", span);
            if (link) {
                // The other suite reads a link's words as plain text: nothing is nested in the link, and the span is around it.
                x.open("text:a").attr("xlink:type", "simple").attr("xlink:href", cf.anchorHref()).text(pending).close();
                sp.afterSpace = pending.endsWith(QLatin1Char(' '));
            } else {
                writeText(x, pending, sp);
            }
            x.close();
            pending.clear();
        };
        for (const QChar c : text) {
            if (skipTab) {
                skipTab = false;
                if (c == QLatin1Char('\t')) continue;
            }
            if (c != QChar(QChar::ObjectReplacementCharacter)) {
                pending += c;
                continue;
            }
            flush();
            if (field.isEmpty()) {
                sayOnce(QStringLiteral("inline"), QCoreApplication::translate("OdpExport", "Objects set in the middle of text are left out."));
                continue;
            }
            const QString span = textStyleOf(sc, cf, true);
            x.open("text:span").attr("text:style-name", span);
            if (field == QLatin1String("page")) {
                x.open("text:page-number").text(QStringLiteral("<number>")).close();
            } else {
                const QString kind = field.section(QLatin1Char(':'), 0, 0);
                bool isTime = false;
                const QString style = dateStyle(*sc.styles, field, &isTime);
                if (kind == QLatin1String("time")) x.open("text:time");
                else x.open("text:date");
                x.attr("style:data-style-name", style).attr("text:fixed", "false").text(shownField(field)).close();
            }
            x.close();
            sp.afterSpace = false;
        }
        flush();
    }
    x.close();
}

void Exporter::writeStory(X &x, Scope &sc, const QTextDocument *doc, const TextOpts &opts)
{
    if (!doc) return;
    QVector<QTextBlock> blocks;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) blocks << b;
    int depth = 0, runEnd = 0;
    QString listStyle, previousTop;
    bool continues = false;
    auto closeTo = [&](int target) {
        while (depth > target) {
            x.close();   // the item
            x.close();   // the list
            --depth;
        }
    };
    // How a listed paragraph's level is laid out and labeled, apart from where the numbers start.
    auto definitionOf = [&](const QTextBlock &b) {
        static const QRegularExpression start(QStringLiteral(" text:start-value=\"[0-9]+\""));
        QString xml = listLevelXml(levelOf(b) + 1, b.blockFormat(), b.textList()->format(), firstSizeOf(b), nullptr, nullptr);
        return xml.remove(start);
    };
    for (int i = 0; i < blocks.size(); ++i) {
        const QTextBlock &b = blocks[i];
        const TextOpts &here = opts;
        if (!b.textList()) {
            closeTo(0);
            previousTop.clear();
            writeParagraph(x, sc, b, false, here);
            continue;
        }
        // A bullet in front of nothing isn't drawn here, and the other suite draws it: an empty paragraph is written outside the list, and the
        // numbers go on after it.
        if (b.length() <= 1) {
            closeTo(0);
            writeParagraph(x, sc, b, false, here);
            continue;
        }
        const int target = levelOf(b) + 1;
        if (depth == 0 || i >= runEnd) {
            // A run of listed paragraphs that agree on how each level looks shares one list style; numbers go on from a run before it that looks the same.
            closeTo(0);
            QVector<QTextBlock> run;
            QHash<int, QString> definitions;
            for (int k = i; k < blocks.size() && blocks[k].textList(); ++k) {
                const QString def = definitionOf(blocks[k]);
                const auto known = definitions.constFind(levelOf(blocks[k]));
                if (known != definitions.cend() && known.value() != def) break;
                definitions.insert(levelOf(blocks[k]), def);
                run << blocks[k];
            }
            runEnd = i + int(run.size());
            listStyle = listStyleFor(sc, run);
            const QString top = definitionOf(b);
            continues = !previousTop.isEmpty() && previousTop == top && b.textList()->format().style() != QTextListFormat::ListDisc && !isBulletList(b.textList()->format().style());
            previousTop = levelOf(b) == 0 ? top : QString();
        }
        closeTo(std::min(depth, target));
        if (depth == target) {
            x.close();   // the item before this one
            x.open("text:list-item");
        }
        while (depth < target) {
            x.open("text:list");
            if (depth == 0) {
                x.attr("text:style-name", listStyle);
                if (continues) x.attr("text:continue-numbering", "true");
            }
            x.open("text:list-item");
            ++depth;
        }
        writeParagraph(x, sc, b, true, here);
    }
    closeTo(0);
}

// ---- the text of a master's placeholder

QVector<Exporter::Level> Exporter::levelsOf(const QTextDocument *doc) const
{
    QVector<Level> levels(kLevels);
    if (!doc) return levels;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        const int at = levelOf(b);
        if (levels[at].seen) continue;
        Level &l = levels[at];
        l.seen = true;
        l.chr = b.charFormat();
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            if (it.fragment().isValid()) {
                l.chr = it.fragment().charFormat();
                break;
            }
        }
        l.blk = b.blockFormat();
        if (QTextList *list = b.textList()) {
            l.listed = true;
            l.list = list->format();
        }
    }
    return levels;
}

} // namespace jp::deck::odpw
