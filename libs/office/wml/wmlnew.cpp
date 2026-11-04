// A new word processing document, and the styles a person may apply that a document might lack. The looks of the styles are
// JeffOffice's own.

#include "wml/wml.h"

#include <QRegularExpression>

#include "wml/xmltree.h"

#include <QCoreApplication>
#include <algorithm>
#include <QDateTime>

namespace jp::office::wml {

namespace {

Fonts themeFonts(bool major)
{
    Fonts f;
    const QString p = major ? QStringLiteral("major") : QStringLiteral("minor");
    f.asciiTheme = f.hAnsiTheme = p + QStringLiteral("HAnsi");
    f.eastAsiaTheme = p + QStringLiteral("EastAsia");
    f.csTheme = p + QStringLiteral("Bidi");
    return f;
}

Color accent(const QString &shade = QString())
{
    const Theme t = defaultTheme();
    QColor c = t.colors[Theme::Accent1];
    if (!shade.isEmpty()) c = c.darker(135);
    return Color{c.name().mid(1).toUpper(), QStringLiteral("accent1"), QString(), shade};
}

Style paragraphStyle(const QString &id, const QString &name, int priority)
{
    Style s;
    s.id = id;
    s.name = name;
    s.type = QStringLiteral("paragraph");
    s.basedOn = QStringLiteral("Normal");
    s.next = QStringLiteral("Normal");
    s.uiPriority = priority;
    s.qFormat = true;
    return s;
}

Style heading(int level)
{
    Style s = paragraphStyle(QStringLiteral("Heading%1").arg(level), QStringLiteral("heading %1").arg(level), 9);
    s.pPr.keepNext = true;
    s.pPr.keepLines = true;
    s.pPr.outlineLvl = level - 1;
    s.rPr.fonts = themeFonts(true);
    static const int sizes[] = {32, 26, 24, 22, 22, 22, 22, 22, 22};
    s.rPr.size = sizes[level - 1];
    s.rPr.color = accent(level <= 3 ? QStringLiteral("BF") : QString());
    if (level == 1) {
        s.pPr.before = 360;
        s.pPr.after = 80;
    } else if (level <= 3) {
        s.pPr.before = 160;
        s.pPr.after = 80;
    } else {
        s.pPr.before = 80;
        s.pPr.after = 40;
    }
    if (level >= 4) s.rPr.italic = level == 4;
    if (level >= 6) s.rPr.color = Color{QStringLiteral("595959"), QString(), QString(), QString()};
    if (level > 3) s.uiPriority = 9;
    return s;
}

} // namespace

QStringList builtinStyleIds()
{
    QStringList ids{QStringLiteral("Normal"), QStringLiteral("DefaultParagraphFont"), QStringLiteral("Title"), QStringLiteral("Subtitle")};
    for (int i = 1; i <= 9; ++i) ids << QStringLiteral("Heading%1").arg(i);
    ids << QStringLiteral("NoSpacing") << QStringLiteral("ListParagraph") << QStringLiteral("Quote") << QStringLiteral("IntenseQuote") << QStringLiteral("Emphasis")
        << QStringLiteral("Strong") << QStringLiteral("Hyperlink");
    return ids;
}


// ---- table styles

namespace {

QByteArray sideXml(const char *side, int size, const char *color, const char *val = "single")
{
    return QByteArray("<w:") + side + " w:val=\"" + val + "\" w:sz=\"" + QByteArray::number(size) + "\" w:space=\"0\" w:color=\"" + color + "\"/>";
}

QByteArray bordersXml(const char *group, const char *color, int size, bool inside, bool outside = true)
{
    QByteArray out = QByteArray("<w:") + group + ">";
    if (outside) out += sideXml("top", size, color) + sideXml("left", size, color) + sideXml("bottom", size, color) + sideXml("right", size, color);
    if (inside) out += sideXml("insideH", size, color) + sideXml("insideV", size, color);
    return out + "</w:" + group + ">";
}

QByteArray shadeXml(const char *fill) { return QByteArray("<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"") + fill + "\"/>"; }

// A part of a table style: bold, a text color, borders, and a fill.
QByteArray stylePart(const char *type, bool bold, const char *textColor, const QByteArray &borders, const char *fill)
{
    QByteArray out = QByteArray("<w:tblStylePr w:type=\"") + type + "\">";
    if (bold || textColor) {
        out += "<w:rPr>";
        if (bold) out += "<w:b/><w:bCs/>";
        if (textColor) out += QByteArray("<w:color w:val=\"") + textColor + "\"/>";
        out += "</w:rPr>";
    }
    if (!borders.isEmpty() || fill) {
        out += "<w:tcPr>";
        if (!borders.isEmpty()) out += "<w:tcBorders>" + borders + "</w:tcBorders>";
        if (fill) out += shadeXml(fill);
        out += "</w:tcPr>";
    }
    return out + "</w:tblStylePr>";
}

Style tableStyle(const QString &id, const QString &name, int priority, const QByteArray &tail, bool grid = false)
{
    Style s;
    s.id = id;
    s.name = name;
    s.type = QStringLiteral("table");
    s.basedOn = QStringLiteral("TableNormal");
    s.uiPriority = priority;
    s.unhideWhenUsed = true;
    if (grid) {
        s.pPr.after = 0;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
    }
    s.tail = tail;
    return s;
}

struct TableAccent {
    const char *id, *name, *color, *tint, *line;
};
const TableAccent kTableAccents[] = {{"Accent1", "Accent 1", "4472C4", "D9E2F3", "8EAADB"}, {"Accent2", "Accent 2", "ED7D31", "FBE5D6", "F4B183"},
                                     {"Accent3", "Accent 3", "A5A5A5", "EDEDED", "C9C9C9"}, {"Accent4", "Accent 4", "FFC000", "FFF2CC", "FFD966"},
                                     {"Accent5", "Accent 5", "5B9BD5", "DEEAF6", "9CC2E5"}, {"Accent6", "Accent 6", "70AD47", "E2EFD9", "A9D18E"}};

} // namespace

QStringList builtinTableStyleIds()
{
    QStringList ids{QStringLiteral("TableGrid"), QStringLiteral("PlainTable1"), QStringLiteral("PlainTable2"), QStringLiteral("GridTable1Light")};
    for (const TableAccent &a : kTableAccents) ids << QStringLiteral("GridTable4-") + QLatin1String(a.id);
    for (int i = 0; i < 3; ++i) ids << QStringLiteral("ListTable3-") + QLatin1String(kTableAccents[i].id);
    return ids;
}

Style builtinTableStyle(const QString &id)
{
    // (The ids a build wrote without the hyphen are still known, each as the style it was.)
    static const QRegularExpression unhyphened(QStringLiteral("^(GridTable4|ListTable3)(Accent[1-6])$"));
    if (const QRegularExpressionMatch m = unhyphened.match(id); m.hasMatch()) {
        Style s = builtinTableStyle(m.captured(1) + QLatin1Char('-') + m.captured(2));
        if (!s.id.isEmpty()) s.id = id;
        return s;
    }
    if (id == QLatin1String("TableNormal")) {
        Style s;
        s.id = id;
        s.name = QStringLiteral("Normal Table");
        s.type = QStringLiteral("table");
        s.isDefault = true;
        s.uiPriority = 99;
        s.semiHidden = s.unhideWhenUsed = true;
        s.tail = "<w:tblPr><w:tblInd w:w=\"0\" w:type=\"dxa\"/><w:tblCellMar><w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"108\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/>"
                 "<w:right w:w=\"108\" w:type=\"dxa\"/></w:tblCellMar></w:tblPr>";
        return s;
    }
    if (id == QLatin1String("TableGrid")) return tableStyle(id, QStringLiteral("Table Grid"), 39, "<w:tblPr>" + bordersXml("tblBorders", "auto", 4, true) + "</w:tblPr>", true);
    if (id == QLatin1String("PlainTable1")) {
        const QByteArray bottom = sideXml("bottom", 4, "BFBFBF"), top = sideXml("top", 4, "BFBFBF");
        return tableStyle(id, QStringLiteral("Plain Table 1"), 41,
                          "<w:tblPr><w:tblStyleRowBandSize w:val=\"1\"/><w:tblStyleColBandSize w:val=\"1\"/><w:tblBorders>" + top + bottom + "</w:tblBorders></w:tblPr>" +
                              stylePart("firstRow", true, nullptr, bottom, nullptr) + stylePart("lastRow", true, nullptr, top, nullptr) + stylePart("firstCol", true, nullptr, QByteArray(), nullptr) +
                              stylePart("lastCol", true, nullptr, QByteArray(), nullptr) + stylePart("band1Vert", false, nullptr, QByteArray(), "F2F2F2") +
                              stylePart("band1Horz", false, nullptr, QByteArray(), "F2F2F2"));
    }
    if (id == QLatin1String("PlainTable2")) {
        const QByteArray bottom = sideXml("bottom", 4, "7F7F7F"), top = sideXml("top", 4, "7F7F7F");
        return tableStyle(id, QStringLiteral("Plain Table 2"), 42,
                          "<w:tblPr><w:tblStyleRowBandSize w:val=\"1\"/><w:tblStyleColBandSize w:val=\"1\"/><w:tblBorders>" + top + bottom + "</w:tblBorders></w:tblPr>" +
                              stylePart("firstRow", true, nullptr, bottom, nullptr) + stylePart("lastRow", true, nullptr, top, nullptr) + stylePart("firstCol", true, nullptr, QByteArray(), nullptr) +
                              stylePart("lastCol", true, nullptr, QByteArray(), nullptr) + stylePart("band1Horz", false, nullptr, top + bottom, nullptr));
    }
    if (id == QLatin1String("GridTable1Light")) {
        return tableStyle(id, QStringLiteral("Grid Table 1 Light"), 46,
                          "<w:tblPr><w:tblStyleRowBandSize w:val=\"1\"/><w:tblStyleColBandSize w:val=\"1\"/>" + bordersXml("tblBorders", "BFBFBF", 4, true) + "</w:tblPr>" +
                              stylePart("firstRow", true, nullptr, sideXml("bottom", 12, "666666"), nullptr) + stylePart("lastRow", true, nullptr, sideXml("top", 12, "666666", "double"), nullptr) +
                              stylePart("firstCol", true, nullptr, QByteArray(), nullptr) + stylePart("lastCol", true, nullptr, QByteArray(), nullptr));
    }
    for (const TableAccent &a : kTableAccents) {
        if (id == QLatin1String("GridTable4-") + QLatin1String(a.id)) {
            const QByteArray line = sideXml("top", 4, a.color) + sideXml("left", 4, a.color) + sideXml("bottom", 4, a.color) + sideXml("right", 4, a.color);
            return tableStyle(id, QStringLiteral("Grid Table 4 - ") + QLatin1String(a.name), 49,
                              "<w:tblPr><w:tblStyleRowBandSize w:val=\"1\"/><w:tblStyleColBandSize w:val=\"1\"/>" + bordersXml("tblBorders", a.line, 4, true) + "</w:tblPr>" +
                                  stylePart("firstRow", true, "FFFFFF", line, a.color) + stylePart("lastRow", true, nullptr, sideXml("top", 4, a.color, "double"), nullptr) +
                                  stylePart("firstCol", true, nullptr, QByteArray(), nullptr) + stylePart("lastCol", true, nullptr, QByteArray(), nullptr) +
                                  stylePart("band1Vert", false, nullptr, QByteArray(), a.tint) + stylePart("band1Horz", false, nullptr, QByteArray(), a.tint));
        }
        if (id == QLatin1String("ListTable3-") + QLatin1String(a.id)) {
            return tableStyle(id, QStringLiteral("List Table 3 - ") + QLatin1String(a.name), 48,
                              "<w:tblPr><w:tblStyleRowBandSize w:val=\"1\"/><w:tblStyleColBandSize w:val=\"1\"/>" + bordersXml("tblBorders", a.color, 4, false) + "</w:tblPr>" +
                                  stylePart("firstRow", true, "FFFFFF", QByteArray(), a.color) + stylePart("lastRow", true, nullptr, sideXml("top", 4, a.color, "double"), nullptr) +
                                  stylePart("firstCol", true, nullptr, QByteArray(), nullptr) + stylePart("lastCol", true, nullptr, QByteArray(), nullptr) +
                                  stylePart("band1Horz", false, nullptr, sideXml("top", 4, a.color) + sideXml("bottom", 4, a.color), nullptr));
        }
    }
    return Style();
}

QString tableStyleScreenName(const Style &style)
{
    static const struct {
        const char *id, *name;
    } fixed[] = {{"TableNormal", QT_TRANSLATE_NOOP("jp::office::TableStyles", "No Style")},
                 {"TableGrid", QT_TRANSLATE_NOOP("jp::office::TableStyles", "Simple Grid")},
                 {"PlainTable1", QT_TRANSLATE_NOOP("jp::office::TableStyles", "Simple Lines")},
                 {"PlainTable2", QT_TRANSLATE_NOOP("jp::office::TableStyles", "Simple Lines, Dark")},
                 {"GridTable1Light", QT_TRANSLATE_NOOP("jp::office::TableStyles", "Light Grid")}};
    static const char *const colors[] = {QT_TRANSLATE_NOOP("jp::office::TableStyles", "Blue"), QT_TRANSLATE_NOOP("jp::office::TableStyles", "Orange"), QT_TRANSLATE_NOOP("jp::office::TableStyles", "Gray"),
                                         QT_TRANSLATE_NOOP("jp::office::TableStyles", "Gold"), QT_TRANSLATE_NOOP("jp::office::TableStyles", "Sky"), QT_TRANSLATE_NOOP("jp::office::TableStyles", "Green")};
    auto tr = [](const char *text) { return QCoreApplication::translate("jp::office::TableStyles", text); };
    const Style built = builtinTableStyle(style.id);
    // (a file's own style of the same id with a name of its own is shown by that name)
    if (built.id.isEmpty() || (!style.name.isEmpty() && style.name != built.name)) return style.name.isEmpty() ? style.id : style.name;
    for (const auto &f : fixed)
        if (style.id == QLatin1String(f.id)) return tr(f.name);
    static const QRegularExpression accent(QStringLiteral("^(GridTable4|ListTable3)-?Accent([1-6])$"));
    const QRegularExpressionMatch m = accent.match(style.id);
    if (m.hasMatch())
        return (m.captured(1) == QLatin1String("GridTable4") ? tr("Banded Grid") : tr("Banded List")) + QStringLiteral(", ") + tr(colors[m.captured(2).toInt() - 1]);
    return style.name.isEmpty() ? style.id : style.name;
}

Style tableStyleFromSpec(const TableStyleSpec &spec)
{
    const QByteArray color = spec.lineColor.isEmpty() ? QByteArray("auto") : spec.lineColor.toLatin1();
    const int size = std::clamp(spec.lineWidth, 2, 96);
    QByteArray tail = "<w:tblPr><w:tblStyleRowBandSize w:val=\"1\"/><w:tblStyleColBandSize w:val=\"1\"/>";
    if (spec.lines != TableStyleSpec::NoLines) tail += bordersXml("tblBorders", color, size, spec.lines == TableStyleSpec::Grid);
    tail += "</w:tblPr>";
    const QByteArray headerFill = spec.headerFill.toLatin1(), headerText = spec.headerText.toLatin1(), rowFill = spec.bandedRowFill.toLatin1(), columnFill = spec.bandedColumnFill.toLatin1();
    const QByteArray none;
    tail += stylePart("firstRow", spec.headerBold, headerText.isEmpty() ? nullptr : headerText.constData(), none, headerFill.isEmpty() ? nullptr : headerFill.constData());
    tail += stylePart("lastRow", spec.totalBold, nullptr, spec.lines == TableStyleSpec::NoLines ? none : sideXml("top", size, color, "double"), nullptr);
    tail += stylePart("firstCol", spec.firstColumnBold, nullptr, none, nullptr);
    tail += stylePart("lastCol", spec.lastColumnBold, nullptr, none, nullptr);
    if (!columnFill.isEmpty()) tail += stylePart("band1Vert", false, nullptr, none, columnFill.constData());
    if (!rowFill.isEmpty()) tail += stylePart("band1Horz", false, nullptr, none, rowFill.constData());
    Style s = tableStyle(QString(), spec.name, 99, tail);
    s.basedOn = spec.basedOn.isEmpty() ? QStringLiteral("TableNormal") : spec.basedOn;
    s.customStyle = true;
    s.unhideWhenUsed = false;
    s.qFormat = true;
    return s;
}

TableStyleSpec tableStyleSpecOf(const Style &style)
{
    TableStyleSpec spec;
    spec.name = style.name;
    spec.basedOn = style.basedOn.isEmpty() ? QStringLiteral("TableNormal") : style.basedOn;
    const TableStyleDef def = readTableStyle(style);
    const TableMark &t = def.base.tbl;
    spec.lines = t.insideH.stated() && t.insideH.val != QLatin1String("nil") && t.insideH.val != QLatin1String("none") ? TableStyleSpec::Grid
                 : t.top.stated() && t.top.val != QLatin1String("nil") && t.top.val != QLatin1String("none") ? TableStyleSpec::Box : TableStyleSpec::NoLines;
    if (spec.lines != TableStyleSpec::NoLines) {
        spec.lineColor = t.top.color.isEmpty() ? QStringLiteral("auto") : t.top.color;
        spec.lineWidth = t.top.size > 0 ? t.top.size : 4;
    }
    auto bold = [&](const char *part) {
        const auto it = def.parts.constFind(QString::fromLatin1(part));
        return it != def.parts.constEnd() && it->rPr && it->rPr->bold.value_or(false);
    };
    auto fill = [&](const char *part) {
        const auto it = def.parts.constFind(QString::fromLatin1(part));
        if (it == def.parts.constEnd() || it->cell.shdFill.isEmpty() || it->cell.shdFill == QLatin1String("auto")) return QString();
        return it->cell.shdFill;
    };
    spec.headerBold = bold("firstRow");
    spec.totalBold = bold("lastRow");
    spec.firstColumnBold = bold("firstCol");
    spec.lastColumnBold = bold("lastCol");
    spec.headerFill = fill("firstRow");
    const auto head = def.parts.constFind(QStringLiteral("firstRow"));
    if (head != def.parts.constEnd() && head->rPr && head->rPr->color && head->rPr->color->val != QLatin1String("auto")) spec.headerText = head->rPr->color->val;
    spec.bandedRowFill = fill("band1Horz");
    spec.bandedColumnFill = fill("band1Vert");
    return spec;
}

Style builtinStyle(const QString &id)
{
    if (id == QLatin1String("Normal")) {
        Style s;
        s.id = QStringLiteral("Normal");
        s.name = QStringLiteral("Normal");
        s.type = QStringLiteral("paragraph");
        s.isDefault = true;
        s.qFormat = true;
        return s;
    }
    if (id == QLatin1String("DefaultParagraphFont")) {
        Style s;
        s.id = id;
        s.name = QStringLiteral("Default Paragraph Font");
        s.type = QStringLiteral("character");
        s.isDefault = true;
        s.uiPriority = 1;
        s.semiHidden = s.unhideWhenUsed = true;
        return s;
    }
    if (id.startsWith(QLatin1String("Heading")) && id.size() == 8 && id[7] >= QLatin1Char('1') && id[7] <= QLatin1Char('9')) return heading(id[7].digitValue());
    // The lines of a table of contents (toc 1 to toc 9, each set in a little more than the one before) and the heading above one.
    if (id.startsWith(QLatin1String("TOC")) && id.size() == 4 && id[3] >= QLatin1Char('1') && id[3] <= QLatin1Char('9')) {
        const int level = id[3].digitValue();
        Style s = paragraphStyle(id, QStringLiteral("toc %1").arg(level), 39);
        s.qFormat = false;
        s.unhideWhenUsed = true;
        s.pPr.after = 100;
        if (level > 1) s.pPr.indLeft = 220 * (level - 1);
        return s;
    }
    if (id == QLatin1String("TOCHeading")) {
        Style s = paragraphStyle(id, QStringLiteral("TOC Heading"), 39);
        s.basedOn = QStringLiteral("Heading1");
        s.unhideWhenUsed = true;
        s.pPr.outlineLvl = 9;
        return s;
    }
    if (id == QLatin1String("Title")) {
        Style s = paragraphStyle(id, QStringLiteral("Title"), 10);
        s.pPr.contextualSpacing = true;
        s.pPr.after = 80;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        s.rPr.fonts = themeFonts(true);
        s.rPr.size = 56;
        s.rPr.spacing = -10;
        return s;
    }
    if (id == QLatin1String("Subtitle")) {
        Style s = paragraphStyle(id, QStringLiteral("Subtitle"), 11);
        s.pPr.after = 160;
        s.rPr.size = 28;
        s.rPr.spacing = 15;
        s.rPr.color = Color{QStringLiteral("595959"), QString(), QString(), QString()};
        return s;
    }
    if (id == QLatin1String("NoSpacing")) {
        Style s = paragraphStyle(id, QStringLiteral("No Spacing"), 1);
        s.next.clear();
        s.pPr.after = 0;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        return s;
    }
    if (id == QLatin1String("ListParagraph")) {
        Style s = paragraphStyle(id, QStringLiteral("List Paragraph"), 34);
        s.next.clear();
        s.pPr.indLeft = 720;
        s.pPr.contextualSpacing = true;
        return s;
    }
    if (id == QLatin1String("Quote")) {
        Style s = paragraphStyle(id, QStringLiteral("Quote"), 29);
        s.pPr.before = 200;
        s.pPr.after = 160;
        s.pPr.indLeft = 864;
        s.pPr.indRight = 864;
        s.pPr.jc = QStringLiteral("center");
        s.rPr.italic = true;
        s.rPr.color = Color{QStringLiteral("404040"), QString(), QString(), QString()};
        return s;
    }
    if (id == QLatin1String("IntenseQuote")) {
        Style s = paragraphStyle(id, QStringLiteral("Intense Quote"), 30);
        s.pPr.before = 360;
        s.pPr.after = 360;
        s.pPr.indLeft = 864;
        s.pPr.indRight = 864;
        s.pPr.jc = QStringLiteral("center");
        s.rPr.italic = true;
        s.rPr.color = accent();
        return s;
    }
    if (id == QLatin1String("Emphasis") || id == QLatin1String("Strong")) {
        Style s;
        s.id = id;
        s.name = id;
        s.type = QStringLiteral("character");
        s.uiPriority = id == QLatin1String("Strong") ? 22 : 20;
        s.qFormat = true;
        if (id == QLatin1String("Strong")) s.rPr.bold = true;
        else s.rPr.italic = true;
        return s;
    }
    if (id == QLatin1String("Header") || id == QLatin1String("Footer")) {
        // The styles of the text in a page's header and footer: a center tab and a right tab at the middle and the end of a line of the usual page's text.
        Style s = paragraphStyle(id, id, 99);
        s.next.clear();
        s.qFormat = false;
        s.unhideWhenUsed = true;
        s.pPr.after = 0;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        s.pPr.tabs = QVector<TabStop>{TabStop{4680, QStringLiteral("center"), QString()}, TabStop{9360, QStringLiteral("right"), QString()}};
        return s;
    }
    if (id == QLatin1String("FootnoteText") || id == QLatin1String("EndnoteText")) {
        // The text of a note: smaller than the text it is cited from, close set.
        Style s = paragraphStyle(id, id == QLatin1String("FootnoteText") ? QStringLiteral("footnote text") : QStringLiteral("endnote text"), 99);
        s.next.clear();
        s.qFormat = false;
        s.unhideWhenUsed = true;
        s.pPr.after = 0;
        s.pPr.line = 240;
        s.pPr.lineRule = QStringLiteral("auto");
        s.rPr.size = 20;
        return s;
    }
    if (id == QLatin1String("FootnoteReference") || id == QLatin1String("EndnoteReference")) {
        // The raised number of a note's mark and of the reference to it.
        Style s;
        s.id = id;
        s.name = id == QLatin1String("FootnoteReference") ? QStringLiteral("footnote reference") : QStringLiteral("endnote reference");
        s.type = QStringLiteral("character");
        s.uiPriority = 99;
        s.unhideWhenUsed = true;
        s.rPr.vertAlign = QStringLiteral("superscript");
        return s;
    }
    if (id == QLatin1String("Hyperlink")) {
        Style s;
        s.id = id;
        s.name = QStringLiteral("Hyperlink");
        s.type = QStringLiteral("character");
        s.uiPriority = 99;
        s.unhideWhenUsed = true;
        s.rPr.color = accent();
        s.rPr.underline = Underline{QStringLiteral("single"), QString()};
        return s;
    }
    return builtinTableStyle(id);
}

namespace {

QByteArray xmlDoc(const QByteArray &root) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + root; }

AbstractNum listDefinition(int id, bool bullets)
{
    AbstractNum a;
    a.id = id;
    a.multiLevelType = QStringLiteral("hybridMultilevel");
    static const char *const bulletText[] = {"•", "◦", "▪"};
    static const char *const numFmt[] = {"decimal", "lowerLetter", "lowerRoman"};
    for (int i = 0; i < 9; ++i) {
        Level &l = a.levels[i];
        l.present = true;
        l.start = 1;
        l.jc = QStringLiteral("left");
        if (bullets) {
            l.numFmt = QStringLiteral("bullet");
            l.text = QString::fromUtf8(bulletText[i % 3]);
        } else {
            l.numFmt = QString::fromLatin1(numFmt[i % 3]);
            l.text = QStringLiteral("%%1.").arg(i + 1);
        }
        l.pPr.indLeft = 720 * (i + 1);
        l.pPr.indFirst = -360;
    }
    return a;
}

} // namespace

Document newDocument()
{
    Document d;
    d.mainContentType = QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
    d.stylesName = QStringLiteral("/word/styles.xml");
    d.numberingName = QStringLiteral("/word/numbering.xml");
    d.settingsName = QStringLiteral("/word/settings.xml");
    d.themeName = QStringLiteral("/word/theme/theme1.xml");
    d.theme = defaultTheme();
    d.hasTheme = true;

    StyleSheet &s = d.styles;
    s.defaultRun.fonts = themeFonts(false);
    s.defaultRun.size = 22;
    s.defaultRun.extra.push_back({runChildRank("szCs"), "<w:szCs w:val=\"22\"/>"});
    s.defaultRun.extra.push_back({runChildRank("lang"), "<w:lang w:val=\"en-US\" w:eastAsia=\"en-US\" w:bidi=\"ar-SA\"/>"});
    s.defaultPara.after = 160;
    s.defaultPara.line = 259;
    s.defaultPara.lineRule = QStringLiteral("auto");
    for (const QString &id : builtinStyleIds()) s.styles.push_back(builtinStyle(id));
    s.changed();
    s.modified = true;

    d.numbering.abstracts << listDefinition(0, true) << listDefinition(1, false);
    d.numbering.nums << Num{1, 0, {}, QByteArray()} << Num{2, 1, {}, QByteArray()};
    d.numbering.modified = true;

    SectionProps &sec = d.lastSection;
    sec.hasPgSz = sec.hasPgMar = true;
    d.lastSectionStated = true;
    Block first;
    d.body.push_back(first);

    // The parts a new document keeps.
    const QByteArray w = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"";
    d.parts.push_back({d.settingsName, QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"),
                       xmlDoc("<w:settings " + w + "><w:zoom w:percent=\"100\"/><w:defaultTabStop w:val=\"720\"/><w:characterSpacingControl w:val=\"doNotCompress\"/>"
                              "<w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"15\"/></w:compat></w:settings>")});
    d.parts.push_back({d.themeName, QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"), writeTheme(d.theme)});
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    d.parts.push_back({QStringLiteral("/docProps/core.xml"), QStringLiteral("application/vnd.openxmlformats-package.core-properties+xml"),
                       xmlDoc("<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                              "xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
                              "<dcterms:created xsi:type=\"dcterms:W3CDTF\">" + now.toUtf8() + "</dcterms:created><dcterms:modified xsi:type=\"dcterms:W3CDTF\">" + now.toUtf8() +
                              "</dcterms:modified></cp:coreProperties>")});
    d.parts.push_back({QStringLiteral("/docProps/app.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.extended-properties+xml"),
                       xmlDoc("<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\"><Application>JeffWrite</Application></Properties>")});

    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    d.packageRelationships << Relationship{QStringLiteral("rId1"), rel + QStringLiteral("officeDocument"), d.mainName, false}
                           << Relationship{QStringLiteral("rId2"), QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"),
                                           QStringLiteral("/docProps/core.xml"), false}
                           << Relationship{QStringLiteral("rId3"), rel + QStringLiteral("extended-properties"), QStringLiteral("/docProps/app.xml"), false};
    d.mainRelationships << Relationship{QStringLiteral("rId1"), rel + QStringLiteral("styles"), d.stylesName, false}
                        << Relationship{QStringLiteral("rId2"), rel + QStringLiteral("numbering"), d.numberingName, false}
                        << Relationship{QStringLiteral("rId3"), rel + QStringLiteral("settings"), d.settingsName, false}
                        << Relationship{QStringLiteral("rId4"), rel + QStringLiteral("theme"), d.themeName, false};
    return d;
}

} // namespace jp::office::wml
