// Writing text as DrawingML (drawingml_write.h): paragraphs with their
// properties, bullets and numbering, runs with their formatting, line breaks,
// and fields; the counterpart of drawingml_text.cpp.

#include "office/drawingml_props.h"
#include "office/drawingml_write.h"

#include "office/drawingml_write_util.h"
#include "office/xml.h"
#include "text/textprops.h"

#include <QDate>
#include <QDateTime>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextList>
#include <QTextOption>
#include <QUuid>
#include <algorithm>
#include <cmath>

namespace jp::office {

using namespace dmlwrite;

namespace {

constexpr int kLevels = 9;

void begin(QXmlStreamWriter &w, const char *name) { w.writeStartElement(QLatin1String(name)); }
void attribute(QXmlStreamWriter &w, const char *name, const QString &value) { w.writeAttribute(QLatin1String(name), value); }
void attribute(QXmlStreamWriter &w, const char *name, qint64 value) { w.writeAttribute(QLatin1String(name), QString::number(value)); }
void leaf(QXmlStreamWriter &w, const char *name)
{
    begin(w, name);
    w.writeEndElement();
}

// ---- runs

QString underlineName(const QTextCharFormat &cf)
{
    const int kind = cf.intProperty(tp::UnderlineKind);
    const bool heavy = kind & 4;
    switch (cf.underlineStyle()) {
    case QTextCharFormat::NoUnderline: return QString();
    case QTextCharFormat::DotLine: return heavy ? QStringLiteral("dottedHeavy") : QStringLiteral("dotted");
    case QTextCharFormat::DashUnderline: return heavy ? QStringLiteral("dashHeavy") : QStringLiteral("dash");
    case QTextCharFormat::DashDotLine: return heavy ? QStringLiteral("dotDashHeavy") : QStringLiteral("dotDash");
    case QTextCharFormat::DashDotDotLine: return heavy ? QStringLiteral("dotDotDashHeavy") : QStringLiteral("dotDotDash");
    case QTextCharFormat::WaveUnderline: return kind & 1 ? QStringLiteral("wavyDbl") : heavy ? QStringLiteral("wavyHeavy") : QStringLiteral("wavy");
    default: break;
    }
    if (kind & 2) return QStringLiteral("words");
    if (kind & 1) return QStringLiteral("dbl");
    return heavy ? QStringLiteral("heavy") : QStringLiteral("sng");
}

// The text color a format shows: the color it names, else the brush it was given.
QColor colorOf(const QTextCharFormat &cf, const DrawingWriteContext &ctx)
{
    const QString ref = cf.stringProperty(tp::ColorRefP);
    if (!ref.isEmpty()) return resolve(ColorRef::fromString(ref), ctx);
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() != Qt::NoBrush) return cf.foreground().color();
    return QColor();
}

// A symbol font, whose characters are found by their own character set.
bool isSymbolFont(const QString &face)
{
    return face.compare(QLatin1String("Symbol"), Qt::CaseInsensitive) == 0 || face.startsWith(QLatin1String("Wingdings"), Qt::CaseInsensitive) ||
           face.compare(QLatin1String("Webdings"), Qt::CaseInsensitive) == 0;
}

// a:rPr (or a:endParaRPr): what a run looks like.
void writeRunProperties(QXmlStreamWriter &w, const char *tag, const QTextCharFormat &cf, DrawingWriteContext &ctx, bool link)
{
    begin(w, tag);
    if (const QString lang = cf.stringProperty(tp::Language); !lang.isEmpty()) attribute(w, "lang", clean(lang));
    const double size = cf.hasProperty(QTextFormat::FontPointSize) ? cf.fontPointSize() : 11.0;
    attribute(w, "sz", std::clamp<qint64>(rounded(size * 100), 100, 400000));
    attribute(w, "b", cf.fontWeight() >= QFont::DemiBold ? QStringLiteral("1") : QStringLiteral("0"));
    attribute(w, "i", cf.fontItalic() ? QStringLiteral("1") : QStringLiteral("0"));
    if (const QString u = underlineName(cf); !u.isEmpty()) attribute(w, "u", u);
    if (cf.fontStrikeOut()) attribute(w, "strike", QStringLiteral("sngStrike"));
    if (cf.hasProperty(tp::KernAbove)) attribute(w, "kern", std::clamp<qint64>(rounded(cf.property(tp::KernAbove).toDouble() * 100), 0, 400000));
    if (cf.fontCapitalization() == QFont::AllUppercase) attribute(w, "cap", QStringLiteral("all"));
    else if (cf.fontCapitalization() == QFont::SmallCaps) attribute(w, "cap", QStringLiteral("small"));
    if (const double spacing = tp::kerningOf(cf); spacing != 0 && std::isfinite(spacing)) attribute(w, "spc", std::clamp<qint64>(rounded(spacing * 100), -400000, 400000));
    if (cf.verticalAlignment() == QTextCharFormat::AlignSuperScript) attribute(w, "baseline", QStringLiteral("30000"));
    else if (cf.verticalAlignment() == QTextCharFormat::AlignSubScript) attribute(w, "baseline", QStringLiteral("-25000"));
    if (cf.boolProperty(tp::NoProof)) attribute(w, "noProof", QStringLiteral("1"));

    const QColor color = colorOf(cf, ctx);
    if (color.isValid()) {
        if (color.alpha() == 0) {
            leaf(w, "a:noFill");
        } else {
            begin(w, "a:solidFill");
            writeColor(w, color);
            w.writeEndElement();
        }
    }
    if (const QString fx = cf.stringProperty(tp::TextFx); !fx.isEmpty()) writeTextEffects(w, TextEffects::fromJson(fx));
    if (const QString ref = cf.stringProperty(tp::HighlightRefP); !ref.isEmpty()) {
        const QColor mark = resolve(ColorRef::fromString(ref), ctx);
        if (mark.isValid()) {
            begin(w, "a:highlight");
            writeColor(w, mark);
            w.writeEndElement();
        }
    }
    // The faces: a theme font names no face of its own; others come in the order latin, east asian, complex.
    QStringList families = cf.fontFamilies().toStringList();
    if (families.isEmpty()) {
        if (const QString one = cf.stringProperty(QTextFormat::FontFamily); !one.isEmpty()) families << one;
    }
    families.removeAll(QString());
    if (!families.isEmpty()) {
        begin(w, "a:latin");
        attribute(w, "typeface", clean(families[0]));
        w.writeEndElement();
        if (families.size() > 1) {
            begin(w, "a:ea");
            attribute(w, "typeface", clean(families[1]));
            w.writeEndElement();
        }
        if (families.size() > 2) {
            begin(w, "a:cs");
            attribute(w, "typeface", clean(families[2]));
            w.writeEndElement();
        }
    } else if (const QString theme = cf.stringProperty(tp::ThemeFont); !theme.isEmpty()) {
        const QString kind = theme == QLatin1String("major") ? QStringLiteral("mj") : QStringLiteral("mn");
        const struct { const char *element, *script; } faces[] = {{"a:latin", "lt"}, {"a:ea", "ea"}, {"a:cs", "cs"}};
        for (const auto &f : faces) {
            begin(w, f.element);
            attribute(w, "typeface", QStringLiteral("+%1-%2").arg(kind, QLatin1String(f.script)));
            w.writeEndElement();
        }
    }
    if (link && cf.isAnchor() && !cf.anchorHref().isEmpty()) {
        const QString rId = relateAddress(ctx, cf.anchorHref());
        if (!rId.isEmpty()) {
            begin(w, "a:hlinkClick");
            attribute(w, "r:id", rId);
            // A link in the text's own color, not the theme's link color, says so.
            const QColor linkColor = ctx.theme.colors[Theme::Hyperlink];
            if (color.isValid() && linkColor.isValid() && color.rgb() != linkColor.rgb()) {
                begin(w, "a:extLst");
                begin(w, "a:ext");
                attribute(w, "uri", QStringLiteral("{A12FA001-AC4F-418D-AE19-62706E023703}"));
                begin(w, "ahyp:hlinkClr");
                w.writeAttribute(QStringLiteral("xmlns:ahyp"), QStringLiteral("http://schemas.microsoft.com/office/drawing/2018/hyperlinkcolor"));
                attribute(w, "val", QStringLiteral("tx"));
                w.writeEndElement();
                w.writeEndElement();
                w.writeEndElement();
            }
            w.writeEndElement();
        }
    }
    w.writeEndElement();
}

// The types of the standard's date and time fields for the layouts the program
// makes, and the layout that goes with each (as the reading takes them).
struct DateField {
    const char *type, *code;
};
const DateField kDateFields[] = {
    {"datetime1", "date:M/d/yyyy"}, {"datetime2", "date:dddd, MMMM d, yyyy"}, {"datetime3", "date:d MMMM yyyy"}, {"datetime4", "date:MMMM d, yyyy"},
    {"datetime5", "date:d-MMM-yy"}, {"datetime6", "date:MMMM yy"}, {"datetime7", "date:MMM-yy"}, {"datetime8", "datetime:M/d/yyyy h:mm AP"},
    {"datetime9", "datetime:M/d/yyyy h:mm:ss AP"}, {"datetime10", "time:HH:mm"}, {"datetime11", "time:HH:mm:ss"}, {"datetime12", "time:h:mm AP"},
    {"datetime13", "time:h:mm:ss AP"}};

// A field as a:fld: its type and the text it shows now; false for a field the standard has no type for.
bool writeField(QXmlStreamWriter &w, const QString &code, const QTextCharFormat &cf, DrawingWriteContext &ctx)
{
    QString type, shown;
    if (code == QLatin1String("page")) {
        type = QStringLiteral("slidenum");
        shown = QStringLiteral("\u2039#\u203A");
    } else {
        const QString kind = code.section(QLatin1Char(':'), 0, 0), format = code.section(QLatin1Char(':'), 1);
        if (kind != QLatin1String("date") && kind != QLatin1String("time") && kind != QLatin1String("datetime")) return false;
        for (const DateField &d : kDateFields)
            if (code == QLatin1String(d.code)) type = QLatin1String(d.type);
        // A layout the standard has no type for is the nearest: the one for a date, a time, or both.
        if (type.isEmpty()) type = kind == QLatin1String("date") ? QStringLiteral("datetime1") : kind == QLatin1String("time") ? QStringLiteral("datetime12") : QStringLiteral("datetime8");
        const QDateTime now = QDateTime::currentDateTime();
        shown = kind == QLatin1String("date") ? now.date().toString(format.isEmpty() ? QStringLiteral("M/d/yyyy") : format)
              : kind == QLatin1String("time") ? now.time().toString(format.isEmpty() ? QStringLiteral("h:mm AP") : format)
                                              : now.toString(format.isEmpty() ? QStringLiteral("M/d/yyyy h:mm AP") : format);
    }
    begin(w, "a:fld");
    attribute(w, "id", QUuid::createUuid().toString(QUuid::WithBraces).toUpper());
    attribute(w, "type", type);
    writeRunProperties(w, "a:rPr", cf, ctx, false);
    begin(w, "a:t");
    w.writeCharacters(clean(shown));
    w.writeEndElement();
    w.writeEndElement();
    return true;
}

void writeTextRun(QXmlStreamWriter &w, const QString &text, const QTextCharFormat &cf, DrawingWriteContext &ctx)
{
    begin(w, "a:r");
    writeRunProperties(w, "a:rPr", cf, ctx, true);
    begin(w, "a:t");
    w.writeCharacters(text);
    w.writeEndElement();
    w.writeEndElement();
}

// A fragment of a block: its text with line breaks (a:br) and fields (a:fld)
// where they are. False if nothing of it was written.
bool writeFragment(QXmlStreamWriter &w, const QString &text, const QTextCharFormat &cf, DrawingWriteContext &ctx)
{
    const QString field = cf.stringProperty(tp::Field);
    bool wrote = false;
    QString pending;
    auto flush = [&]() {
        const QString t = clean(pending);
        pending.clear();
        if (t.isEmpty()) return;
        writeTextRun(w, t, cf, ctx);
        wrote = true;
    };
    for (const QChar c : text) {
        if (c == QChar(QChar::LineSeparator)) {
            flush();
            begin(w, "a:br");
            writeRunProperties(w, "a:rPr", cf, ctx, false);
            w.writeEndElement();
            wrote = true;
        } else if (c == QChar(QChar::ObjectReplacementCharacter)) {
            flush();
            if (!field.isEmpty() && writeField(w, field, cf, ctx)) wrote = true;
        } else {
            pending += c;
        }
    }
    flush();
    return wrote;
}

// ---- paragraphs

QString alignName(const QTextBlockFormat &bf)
{
    Qt::Alignment al = bf.alignment() & Qt::AlignHorizontal_Mask;
    // A right-to-left paragraph's left and right are its start and end, unless said to be the page's.
    if (bf.layoutDirection() == Qt::RightToLeft && !(bf.alignment() & Qt::AlignAbsolute)) {
        if (al & Qt::AlignLeft) al = (al & ~Qt::Alignment(Qt::AlignLeft)) | Qt::AlignRight;
        else if (al & Qt::AlignRight) al = (al & ~Qt::Alignment(Qt::AlignRight)) | Qt::AlignLeft;
    }
    if (al & Qt::AlignHCenter) return QStringLiteral("ctr");
    if (al & Qt::AlignRight) return QStringLiteral("r");
    if (al & Qt::AlignJustify) return bf.boolProperty(tp::Distribute) ? QStringLiteral("dist") : QStringLiteral("just");
    return QStringLiteral("l");
}

// The numbering scheme the standard names for a list.
QString numberingScheme(const QTextListFormat &lf)
{
    QString base;
    switch (lf.style()) {
    case QTextListFormat::ListLowerAlpha: base = QStringLiteral("alphaLc"); break;
    case QTextListFormat::ListUpperAlpha: base = QStringLiteral("alphaUc"); break;
    case QTextListFormat::ListLowerRoman: base = QStringLiteral("romanLc"); break;
    case QTextListFormat::ListUpperRoman: base = QStringLiteral("romanUc"); break;
    default: base = QStringLiteral("arabic"); break;
    }
    QString prefix = lf.numberPrefix(), suffix = lf.numberSuffix();
    // A list that only says how it is numbered ("1.", "(1)") has no punctuation of its own.
    if (prefix.isEmpty() && suffix.isEmpty()) {
        switch (lf.intProperty(tp::NumberFormat)) {
        case 1: case 2: case 3: case 4: case 5: suffix = QStringLiteral("."); break;
        case 6: suffix = QStringLiteral(")"); break;
        case 7: prefix = QStringLiteral("("); suffix = QStringLiteral(")"); break;
        default: break;
        }
    }
    if (prefix == QLatin1String("(") && suffix == QLatin1String(")")) return base + QStringLiteral("ParenBoth");
    if (suffix == QLatin1String(")")) return base + QStringLiteral("ParenR");
    if (suffix == QLatin1String(".")) return base + QStringLiteral("Period");
    // Plain numbers have a scheme of their own; the others are counted with a period.
    return base == QLatin1String("arabic") ? QStringLiteral("arabicPlain") : base + QStringLiteral("Period");
}

// Tab stops a reader made to carry the default distance on are left out, the
// distance said instead: a run of left stops a step apart, from a multiple of it.
double defaultTabStep(QList<QTextOption::Tab> *tabs)
{
    const int n = int(tabs->size());
    if (n < 4 || tabs->at(n - 1).type != QTextOption::LeftTab) return 0;
    const double step = tabs->at(n - 1).position - tabs->at(n - 2).position;
    if (step < 1) return 0;
    int from = n - 1;
    while (from > 0 && tabs->at(from - 1).type == QTextOption::LeftTab && std::fabs(tabs->at(from).position - tabs->at(from - 1).position - step) < 0.01) --from;
    if (n - from < 4) return 0;
    const double first = tabs->at(from).position;
    if (std::fabs(first - std::round(first / step) * step) > 0.01) return 0;
    tabs->erase(tabs->begin() + from, tabs->end());
    return step;
}

const char *tabAlignName(QTextOption::TabType t)
{
    return t == QTextOption::RightTab ? "r" : t == QTextOption::CenterTab ? "ctr" : t == QTextOption::DelimiterTab ? "dec" : "l";
}

// The size of the first letters of a block, for a bullet that is a share of it.
double firstSizeOf(const QTextBlock &b)
{
    for (auto it = b.begin(); !it.atEnd(); ++it)
        if (it.fragment().isValid() && it.fragment().charFormat().hasProperty(QTextFormat::FontPointSize)) return it.fragment().charFormat().fontPointSize();
    return b.charFormat().hasProperty(QTextFormat::FontPointSize) ? b.charFormat().fontPointSize() : 0.0;
}

void writeBullet(QXmlStreamWriter &w, const QTextBlock &b, QTextList *list, DrawingWriteContext &ctx)
{
    // An empty paragraph the file said has no bullet says so again, wherever it is.
    const bool marked = !list && b.text().isEmpty() && b.blockFormat().boolProperty(kNoBulletParagraph);
    if (ctx.emptyParagraphsInherit && ctx.placeholderText && b.text().isEmpty()) {
        // The paragraph takes the layout's bullet, unless the file said it has none.
        if (marked) leaf(w, "a:buNone");
        return;
    }
    if (!list) {
        // Where the layout and master give a bullet, a paragraph that has none says so.
        if (ctx.placeholderText || marked) leaf(w, "a:buNone");
        return;
    }
    const QTextBlockFormat bf = b.blockFormat();
    const QTextListFormat lf = list->format();
    // The marker follows the text unless it says otherwise; where the layout and master say another, so does the paragraph.
    const bool inherits = ctx.placeholderText;
    bool colored = false;
    if (const QString ref = lf.stringProperty(tp::BulletColor); !ref.isEmpty()) {
        const QColor c = resolve(ColorRef::fromString(ref), ctx);
        if (c.isValid()) {
            begin(w, "a:buClr");
            writeColor(w, c);
            w.writeEndElement();
            colored = true;
        }
    }
    if (!colored && inherits) leaf(w, "a:buClrTx");
    const double bulletSize = lf.property(tp::BulletSize).toDouble();
    if (bulletSize <= 0 && inherits) leaf(w, "a:buSzTx");
    if (const double size = bulletSize; size > 0) {
        const double first = firstSizeOf(b);
        if (first > 0) {
            begin(w, "a:buSzPct");
            attribute(w, "val", std::clamp<qint64>(percent(size / first), 25000, 400000));
        } else {
            begin(w, "a:buSzPts");
            attribute(w, "val", std::clamp<qint64>(rounded(size * 100), 100, 400000));
        }
        w.writeEndElement();
    }
    const QString face = lf.stringProperty(tp::BulletFont);
    if (face.isEmpty() && inherits) leaf(w, "a:buFontTx");
    if (!face.isEmpty()) {
        begin(w, "a:buFont");
        attribute(w, "typeface", clean(face));
        if (isSymbolFont(face)) {
            attribute(w, "pitchFamily", QStringLiteral("2"));
            attribute(w, "charset", QStringLiteral("2"));
        }
        w.writeEndElement();
    }
    QString custom = bf.stringProperty(tp::BulletChar);
    if (custom.isEmpty()) custom = lf.stringProperty(tp::BulletChar);
    const QTextListFormat::Style style = lf.style();
    if (!custom.isEmpty() || isBulletList(style) || style == QTextListFormat::ListStyleUndefined) {
        if (custom.isEmpty()) custom = style == QTextListFormat::ListCircle ? QStringLiteral("\u25E6") : style == QTextListFormat::ListSquare ? QStringLiteral("\u25AA") : QStringLiteral("\u2022");
        // One character (a letter outside the first plane takes two units).
        QString mark = clean(custom);
        mark = mark.isEmpty() ? QStringLiteral("\u2022") : mark.left(mark[0].isHighSurrogate() ? 2 : 1);
        begin(w, "a:buChar");
        attribute(w, "char", mark);
        w.writeEndElement();
    } else {
        begin(w, "a:buAutoNum");
        attribute(w, "type", numberingScheme(lf));
        if (lf.start() > 1) attribute(w, "startAt", std::clamp(lf.start(), 1, 32767));
        w.writeEndElement();
    }
}

void writeParagraphProperties(QXmlStreamWriter &w, const QTextBlock &b, DrawingWriteContext &ctx)
{
    const QTextBlockFormat bf = b.blockFormat();
    QTextList *list = b.textList();
    int level = bf.hasProperty(tp::ListLevel) ? bf.intProperty(tp::ListLevel) : list ? list->format().intProperty(tp::ListLevel) : 0;
    level = std::clamp(level, 0, kLevels - 1);
    double marL = bf.leftMargin(), indent = bf.textIndent();
    // A list whose text starts at a place of its own: the paragraph hangs from there.
    if (list && list->format().hasProperty(tp::BulletTab)) {
        const double stop = list->format().property(tp::BulletTab).toDouble();
        indent = marL + indent - stop;
        marL = stop;
    }
    QList<QTextOption::Tab> tabs = bf.tabPositions();
    const double defaultTab = defaultTabStep(&tabs);

    begin(w, "a:pPr");
    attribute(w, "marL", extent(std::clamp(marL, 0.0, 4000.0)));
    if (bf.rightMargin() > 0) attribute(w, "marR", extent(std::clamp(bf.rightMargin(), 0.0, 4000.0)));
    if (level > 0) attribute(w, "lvl", level);
    attribute(w, "indent", position(std::clamp(indent, -4000.0, 4000.0)));
    attribute(w, "algn", alignName(bf));
    if (defaultTab > 0) attribute(w, "defTabSz", extent(defaultTab));
    if (bf.layoutDirection() == Qt::RightToLeft) attribute(w, "rtl", QStringLiteral("1"));

    // Where the layout and master have spacing of their own, a paragraph says its own, even when it is none.
    const bool inherits = ctx.placeholderText;
    bool spaced = false;
    if (bf.hasProperty(QTextFormat::LineHeight) && bf.lineHeight() > 0) {
        if (bf.lineHeightType() == QTextBlockFormat::ProportionalHeight) {
            begin(w, "a:lnSpc");
            begin(w, "a:spcPct");
            attribute(w, "val", std::clamp<qint64>(rounded(bf.lineHeight() * 1000), 0, 13200000));
            w.writeEndElement();
            w.writeEndElement();
            spaced = true;
        } else if (bf.lineHeightType() == QTextBlockFormat::FixedHeight) {
            begin(w, "a:lnSpc");
            begin(w, "a:spcPts");
            attribute(w, "val", std::clamp<qint64>(rounded(bf.lineHeight() * 100), 0, 158400));
            w.writeEndElement();
            w.writeEndElement();
            spaced = true;
        }
    }
    if (!spaced && inherits) {
        begin(w, "a:lnSpc");
        begin(w, "a:spcPct");
        attribute(w, "val", QStringLiteral("100000"));
        w.writeEndElement();
        w.writeEndElement();
    }
    for (const auto &gap : {qMakePair("a:spcBef", QTextFormat::BlockTopMargin), qMakePair("a:spcAft", QTextFormat::BlockBottomMargin)}) {
        if (!bf.hasProperty(gap.second) && !inherits) continue;
        begin(w, gap.first);
        // A share of the line a file gave stays one (the margin holds what it came to at the size the text was written at, and the program
        // takes points as they are where it takes a share at the size the text is drawn at).
        if (const double share = tp::spaceShareOf(bf, gap.second == QTextFormat::BlockTopMargin); share > 0) {
            begin(w, "a:spcPct");
            attribute(w, "val", std::clamp<qint64>(rounded(share * 100000), 0, 13200000));
        } else {
            begin(w, "a:spcPts");
            attribute(w, "val", std::clamp<qint64>(rounded(bf.doubleProperty(gap.second) * 100), 0, 158400));
        }
        w.writeEndElement();
        w.writeEndElement();
    }
    writeBullet(w, b, list, ctx);
    if (!tabs.isEmpty()) {
        begin(w, "a:tabLst");
        for (int i = 0; i < tabs.size() && i < 32; ++i) {
            begin(w, "a:tab");
            attribute(w, "pos", extent(std::clamp(tabs[i].position, 0.0, 4000.0)));
            attribute(w, "algn", QLatin1String(tabAlignName(tabs[i].type)));
            w.writeEndElement();
        }
        w.writeEndElement();
    }
    w.writeEndElement();
}

void writeParagraph(QXmlStreamWriter &w, const QTextBlock &b, DrawingWriteContext &ctx)
{
    begin(w, "a:p");
    writeParagraphProperties(w, b, ctx);
    bool wrote = false;
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (f.isValid() && writeFragment(w, f.text(), f.charFormat(), ctx)) wrote = true;
    }
    // The mark after the last letter says how tall an empty paragraph is.
    if (!wrote) writeRunProperties(w, "a:endParaRPr", b.charFormat(), ctx, false);
    w.writeEndElement();
}

} // namespace

void writeParagraphs(QXmlStreamWriter &w, const QTextDocument *doc, DrawingWriteContext &ctx)
{
    if (!doc) return;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) writeParagraph(w, b, ctx);
}

} // namespace jp::office
