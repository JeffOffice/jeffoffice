// The file a workbook of the older binary form is saved as: see xlsxbase.h.

#include "sheet/xlsxbase.h"
#include "odt/odtcommon.h"
#include "sheet/pagesetupxml.h"
#include "sheet/proofingext.h"
#include "sheet/drawingadd.h"
#include "sheet/foreignbook.h"

#include "io/zip.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QLocale>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <map>

namespace jp::office::sheet {

QString baseMainPart() { return QStringLiteral("/xl/workbook.xml"); }
QString baseSharedStringsPart() { return QStringLiteral("/xl/sharedStrings.xml"); }
QString baseStylesPart() { return QStringLiteral("/xl/styles.xml"); }
QString baseSheetPart(int index) { return QStringLiteral("/xl/worksheets/sheet%1.xml").arg(index + 1); }

namespace {

using xmlscan::escapeAttribute;
using xmlscan::escapeText;

const char kXmlHead[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
const char kMainNs[] = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
const char kRelNs[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

QByteArray num(qint64 v) { return QByteArray::number(v); }
QByteArray real(double v) { return QByteArray::number(v, 'g', QLocale::FloatingPointShortest); }
QByteArray attr(const char *name, const QByteArray &value) { return QByteArray(" ") + name + "=\"" + value + "\""; }
// An attribute's text: a tab or a line break in it is a character reference (a reader takes the raw ones for spaces).
QByteArray attributeValue(const QString &value)
{
    QByteArray v = escapeAttribute(value);
    v.replace('\t', "&#9;");
    v.replace('\n', "&#10;");
    return v;
}
QByteArray attrText(const char *name, const QString &value) { return attr(name, attributeValue(value)); }

// ---- colors, fonts, fills, borders

QByteArray colorElement(const char *name, const ColorSpec &c)
{
    QByteArray e = QByteArray("<") + name;
    switch (c.kind) {
    case ColorSpec::Kind::Rgb: e += attr("rgb", "FF" + QByteArray::number(c.rgb & 0xFFFFFF, 16).rightJustified(6, '0').toUpper()); break;
    case ColorSpec::Kind::Theme:
        e += attr("theme", num(c.index));
        if (c.tint != 0) e += attr("tint", real(c.tint));
        break;
    case ColorSpec::Kind::Indexed: e += attr("indexed", num(c.index)); break;
    case ColorSpec::Kind::Auto: e += attr("auto", "1"); break;
    default: return QByteArray();
    }
    return e + "/>";
}

QByteArray fontElement(const FontSpec &f)
{
    QByteArray e = "<font>";
    if (f.bold) e += "<b/>";
    if (f.italic) e += "<i/>";
    if (f.strike) e += "<strike/>";
    switch (f.underline) {
    case Underline::Single: e += "<u/>"; break;
    case Underline::Double: e += "<u val=\"double\"/>"; break;
    case Underline::SingleAccounting: e += "<u val=\"singleAccounting\"/>"; break;
    case Underline::DoubleAccounting: e += "<u val=\"doubleAccounting\"/>"; break;
    default: break;
    }
    if (f.vertAlign == 1) e += "<vertAlign val=\"superscript\"/>";
    else if (f.vertAlign == 2) e += "<vertAlign val=\"subscript\"/>";
    e += "<sz val=\"" + real(f.size) + "\"/>";
    e += colorElement("color", f.color);
    e += "<name val=\"" + attributeValue(f.name.isEmpty() ? QStringLiteral("Arial") : f.name) + "\"/>";
    if (f.family >= 0) e += "<family val=\"" + num(f.family) + "\"/>";
    if (f.charset >= 0) e += "<charset val=\"" + num(f.charset) + "\"/>";
    if (!f.scheme.isEmpty()) e += "<scheme val=\"" + attributeValue(f.scheme) + "\"/>";
    return e + "</font>";
}

const char *patternName(PatternKind k)
{
    static const char *const names[] = {"none", "solid", "mediumGray", "darkGray", "lightGray", "darkHorizontal", "darkVertical", "darkDown", "darkUp", "darkGrid", "darkTrellis",
                                        "lightHorizontal", "lightVertical", "lightDown", "lightUp", "lightGrid", "lightTrellis", "gray125", "gray0625"};
    const int i = int(k);
    return i >= 0 && i < int(sizeof(names) / sizeof(names[0])) ? names[i] : "none";
}

QByteArray fillElement(const FillSpec &f)
{
    if (f.gradient && !f.stops.isEmpty()) {
        QByteArray g = "<fill><gradientFill";
        if (f.path) g += " type=\"path\"";
        if (f.degree != 0) g += " degree=\"" + real(f.degree) + "\"";
        if (f.left != 0) g += " left=\"" + real(f.left) + "\"";
        if (f.right != 0) g += " right=\"" + real(f.right) + "\"";
        if (f.top != 0) g += " top=\"" + real(f.top) + "\"";
        if (f.bottom != 0) g += " bottom=\"" + real(f.bottom) + "\"";
        g += ">";
        for (const GradientStop &s : f.stops) {
            QByteArray color = colorElement("color", s.color);
            if (color.isEmpty()) color = "<color rgb=\"FF000000\"/>";
            g += "<stop position=\"" + real(s.position) + "\">" + color + "</stop>";
        }
        return g + "</gradientFill></fill>";
    }
    QByteArray e = QByteArray("<fill><patternFill patternType=\"") + patternName(f.pattern) + "\"";
    const QByteArray fg = colorElement("fgColor", f.fg), bg = colorElement("bgColor", f.bg);
    if (fg.isEmpty() && bg.isEmpty()) return e + "/></fill>";
    return e + ">" + fg + bg + "</patternFill></fill>";
}

const char *borderName(BorderStyle s)
{
    static const char *const names[] = {"", "thin", "medium", "dashed", "dotted", "thick", "double", "hair", "mediumDashed", "dashDot", "mediumDashDot", "dashDotDot", "mediumDashDotDot",
                                        "slantDashDot"};
    const int i = int(s);
    return i >= 0 && i < int(sizeof(names) / sizeof(names[0])) ? names[i] : "";
}

QByteArray sideElement(const char *name, const BorderSide &s)
{
    QByteArray e = QByteArray("<") + name;
    if (s.style == BorderStyle::None) return e + "/>";
    e += QByteArray(" style=\"") + borderName(s.style) + "\">";
    QByteArray color = colorElement("color", s.color);
    if (color.isEmpty()) color = "<color auto=\"1\"/>";
    return e + color + "</" + name + ">";
}

QByteArray borderElement(const BorderSpec &b)
{
    QByteArray e = "<border";
    if (b.diagonalUp) e += attr("diagonalUp", "1");
    if (b.diagonalDown) e += attr("diagonalDown", "1");
    e += ">";
    e += sideElement("left", b.left) + sideElement("right", b.right) + sideElement("top", b.top) + sideElement("bottom", b.bottom) + sideElement("diagonal", b.diagonal);
    return e + "</border>";
}

// ---- cell formats

const char *horizontalName(HAlign a)
{
    switch (a) {
    case HAlign::Left: return "left";
    case HAlign::Center: return "center";
    case HAlign::Right: return "right";
    case HAlign::Fill: return "fill";
    case HAlign::Justify: return "justify";
    case HAlign::CenterContinuous: return "centerContinuous";
    case HAlign::Distributed: return "distributed";
    default: return nullptr;
    }
}

const char *verticalName(VAlign a)
{
    switch (a) {
    case VAlign::Center: return "center";
    case VAlign::Top: return "top";
    case VAlign::Justify: return "justify";
    case VAlign::Distributed: return "distributed";
    default: return nullptr;
    }
}

QByteArray xfElement(const CellFormat &f, int xfId, bool cellFormat)
{
    QByteArray e = "<xf" + attr("numFmtId", num(f.numFmtId)) + attr("fontId", num(f.font)) + attr("fillId", num(f.fill)) + attr("borderId", num(f.border)) + attr("xfId", num(xfId));
    if (cellFormat) e += attr("applyNumberFormat", "1") + attr("applyFont", "1") + attr("applyFill", "1") + attr("applyBorder", "1") + attr("applyAlignment", "1");
    if (!f.locked || f.formulaHidden) e += attr("applyProtection", "1");
    if (f.quotePrefix) e += attr("quotePrefix", "1");
    QByteArray inner;
    const char *h = horizontalName(f.horizontal);
    const char *v = verticalName(f.vertical);
    if (h || v || f.wrap || f.shrink || f.indent || f.rotation || f.readingOrder) {
        inner += "<alignment";
        if (h) inner += attr("horizontal", h);
        if (v) inner += attr("vertical", v);
        if (f.rotation) inner += attr("textRotation", num(f.rotation));
        if (f.wrap) inner += attr("wrapText", "1");
        if (f.indent) inner += attr("indent", num(f.indent));
        if (f.shrink) inner += attr("shrinkToFit", "1");
        if (f.readingOrder) inner += attr("readingOrder", num(f.readingOrder));
        inner += "/>";
    }
    if (!f.locked || f.formulaHidden) inner += QByteArray("<protection") + (f.locked ? "" : " locked=\"0\"") + (f.formulaHidden ? " hidden=\"1\"" : "") + "/>";
    inner += proofingExtensionXml(f);
    return inner.isEmpty() ? e + "/>" : e + ">" + inner + "</xf>";
}

// The styles part. `formatCount` of the cell formats are written (the ones the file had: the workbook may have been given more by an edit since, which the changes carry), and the number
// formats other than `added`.
QByteArray stylesPart(const Styles &st, qsizetype formatCount, const QSet<int> &added)
{
    QByteArray o = kXmlHead;
    o += QByteArray("<styleSheet xmlns=\"") + kMainNs + "\">";
    // the number formats the file defines
    QList<int> ids;
    for (auto it = st.numFmts.constBegin(); it != st.numFmts.constEnd(); ++it)
        if (!added.contains(it.key())) ids.append(it.key());
    std::sort(ids.begin(), ids.end());
    if (!ids.isEmpty()) {
        o += "<numFmts" + attr("count", num(ids.size())) + ">";
        for (const int id : ids) o += "<numFmt" + attr("numFmtId", num(id)) + attrText("formatCode", st.numFmts.value(id)) + "/>";
        o += "</numFmts>";
    }
    // fonts, fills, and borders (a file has at least one font, and the first two fills are the two every file starts with)
    QVector<FontSpec> fonts = st.fonts;
    if (fonts.isEmpty()) fonts.append(FontSpec());
    o += "<fonts" + attr("count", num(fonts.size())) + ">";
    for (const FontSpec &f : std::as_const(fonts)) o += fontElement(f);
    o += "</fonts>";
    QVector<FillSpec> fills = st.fills;
    while (fills.size() < 2) {
        FillSpec f;
        if (fills.size() == 1) f.pattern = PatternKind::Gray125;
        fills.append(f);
    }
    o += "<fills" + attr("count", num(fills.size())) + ">";
    for (const FillSpec &f : std::as_const(fills)) o += fillElement(f);
    o += "</fills>";
    QVector<BorderSpec> borders = st.borders;
    if (borders.isEmpty()) borders.append(BorderSpec());
    o += "<borders" + attr("count", num(borders.size())) + ">";
    for (const BorderSpec &b : std::as_const(borders)) o += borderElement(b);
    o += "</borders>";
    // The style formats, one for each named style (in the order of the names; a style format with no name is not kept, and a cell format that follows one follows the Normal style), then
    // the cell formats (one for each the workbook has, in its order, so that a cell's place is the same here). Every style format is named by one name.
    QVector<CellFormat> formats = st.formats.mid(0, formatCount);
    if (formats.isEmpty()) formats.append(CellFormat());
    QVector<quint32> styleFormats;   // the place of each style's format in `formats`
    QHash<quint32, int> styleAt;     // the first style that has the format
    QByteArray named;
    QSet<QString> seen;
    int normal = 0;
    for (const NamedStyle &ns : st.namedStyles) {
        if (ns.format >= quint32(formats.size()) || ns.name.isEmpty() || seen.contains(ns.name.toLower())) continue;
        seen.insert(ns.name.toLower());
        const int at = int(styleFormats.size());
        styleFormats.append(ns.format);
        if (!styleAt.contains(ns.format)) styleAt.insert(ns.format, at);
        if (ns.builtin == 0) normal = at;
        named += "<cellStyle" + attrText("name", ns.name) + attr("xfId", num(at));
        if (ns.builtin >= 0) named += attr("builtinId", num(ns.builtin));
        named += "/>";
    }
    if (styleFormats.isEmpty()) {   // (a file has a Normal style)
        named = "<cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/>";
        o += "<cellStyleXfs count=\"1\">" + xfElement(CellFormat(), 0, false) + "</cellStyleXfs>";
    } else {
        o += "<cellStyleXfs" + attr("count", num(styleFormats.size())) + ">";
        for (const quint32 f : std::as_const(styleFormats)) o += xfElement(formats[int(f)], 0, false);
        o += "</cellStyleXfs>";
    }
    o += "<cellXfs" + attr("count", num(formats.size())) + ">";
    for (int i = 0; i < formats.size(); ++i) {
        int parent = normal;
        if (formats[i].isStyle && styleAt.contains(quint32(i))) parent = styleAt.value(quint32(i));
        else if (formats[i].parent != CellFormat::kNoParent && styleAt.contains(formats[i].parent)) parent = styleAt.value(formats[i].parent);
        o += xfElement(formats[i], parent, true);
    }
    o += "</cellXfs>";
    o += "<cellStyles" + attr("count", num(std::max<qsizetype>(1, styleFormats.size()))) + ">" + named + "</cellStyles>";
    o += "<dxfs count=\"0\"/>";
    if (!st.indexed.isEmpty()) {
        o += "<colors><indexedColors>";
        for (const quint32 c : st.indexed) o += "<rgbColor rgb=\"FF" + QByteArray::number(c & 0xFFFFFF, 16).rightJustified(6, '0').toUpper() + "\"/>";
        o += "</indexedColors></colors>";
    }
    return o + "</styleSheet>";
}

// ---- the workbook

QByteArray workbookPart(const WorkbookInfo &fileInfo)
{
    const WorkbookInfo &info = fileInfo;
    QByteArray o = kXmlHead;
    o += QByteArray("<workbook xmlns=\"") + kMainNs + "\" xmlns:r=\"" + kRelNs + "\">";
    if (info.dateSystem == DateSystem::Date1904) o += "<workbookPr date1904=\"1\"/>";
    o += "<bookViews><workbookView" + attr("firstSheet", num(std::max(0, info.firstSheet))) + attr("activeTab", num(std::max(0, info.activeTab))) + "/></bookViews>";
    o += "<sheets>";
    for (int i = 0; i < info.sheets.size(); ++i) {
        const SheetEntry &e = info.sheets[i];
        o += "<sheet" + attrText("name", e.name) + attr("sheetId", num(e.sheetId > 0 ? e.sheetId : i + 1));
        if (e.state == SheetState::Hidden) o += attr("state", "hidden");
        else if (e.state == SheetState::VeryHidden) o += attr("state", "veryHidden");
        o += attr("r:id", "rId" + num(i + 1)) + "/>";
    }
    o += "</sheets>";
    if (!info.definedNames.isEmpty()) {
        o += "<definedNames>";
        // a name the format refuses (an old file's "SMW 14") makes the whole file refused: its marks become underscores, and a number keeps it apart from the names already there
        auto allowed = [](const QString &name) {
            if (name.isEmpty() || name.size() > 255 || !(name[0].isLetter() || name[0] == QLatin1Char('_') || name[0] == QLatin1Char('\\'))) return false;
            for (const QChar c : name)
                if (!(c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\') || c == QLatin1Char('?'))) return false;
            return true;
        };
        QSet<QString> taken;
        for (const DefinedName &n : info.definedNames)
            if (allowed(n.name)) taken.insert(QString::number(n.localSheet) + QLatin1Char('|') + n.name.toLower());
        for (const DefinedName &n : info.definedNames) {
            QString name = n.name;
            if (!allowed(name)) {
                QString base;
                for (const QChar c : name.left(250)) base += c.isLetterOrNumber() || c == QLatin1Char('.') || c == QLatin1Char('\\') || c == QLatin1Char('?') ? c : QLatin1Char('_');
                if (base.isEmpty() || !(base[0].isLetter() || base[0] == QLatin1Char('_') || base[0] == QLatin1Char('\\'))) base.prepend(QLatin1Char('_'));
                name = base;
                for (int k = 2; taken.contains(QString::number(n.localSheet) + QLatin1Char('|') + name.toLower()); ++k) name = base + QLatin1Char('_') + QString::number(k);
                taken.insert(QString::number(n.localSheet) + QLatin1Char('|') + name.toLower());
            }
            o += "<definedName" + attrText("name", name);
            if (n.localSheet >= 0) o += attr("localSheetId", num(n.localSheet));
            if (n.hidden) o += attr("hidden", "1");
            if (!n.comment.isEmpty()) o += attrText("comment", n.comment);
            o += ">" + escapeText(n.ref) + "</definedName>";
        }
        o += "</definedNames>";
    }
    o += "<calcPr";
    if (info.calcMode == CalcMode::Manual) o += attr("calcMode", "manual");
    else if (info.calcMode == CalcMode::AutoNoTable) o += attr("calcMode", "autoNoTable");
    if (!info.calcOnSave) o += attr("calcOnSave", "0");
    if (info.iterate) o += attr("iterate", "1") + attr("iterateCount", num(info.iterateCount)) + attr("iterateDelta", QByteArray::number(info.iterateDelta, 'g', 12));
    if (info.fullCalcOnLoad) o += attr("fullCalcOnLoad", "1");
    return o + "/></workbook>";
}

// ---- a sheet

// Where the bytes of a part go as they are made: collected, and given to the ZIP in pieces.
class Out {
public:
    explicit Out(ZipWriter *zip) : m_zip(zip) { m_buf.reserve(kPiece * 2); }
    Out &operator+=(const QByteArray &bytes)
    {
        m_buf += bytes;
        if (m_buf.size() >= kPiece) flush();
        return *this;
    }
    Out &operator+=(const char *text)
    {
        m_buf += text;
        if (m_buf.size() >= kPiece) flush();
        return *this;
    }
    void flush()
    {
        if (!m_buf.isEmpty()) m_zip->write(m_buf.constData(), m_buf.size());
        m_buf.clear();
    }

private:
    static constexpr qsizetype kPiece = 1 << 16;
    ZipWriter *m_zip;
    QByteArray m_buf;
};

// A text node: with the space kept when it starts or ends with one, or has two together, or a line break.
QByteArray textElement(const char *name, const QString &text)
{
    bool keep = false;
    if (!text.isEmpty() && (text.front().isSpace() || text.back().isSpace())) keep = true;
    if (!keep && (text.contains(QLatin1Char('\n')) || text.contains(QLatin1Char('\t')) || text.contains(QLatin1String("  ")))) keep = true;
    return QByteArray("<") + name + (keep ? " xml:space=\"preserve\"" : "") + ">" + escapeText(text) + "</" + name + ">";
}

// The properties of a run of text in a font, in the order a program writes them.
QByteArray runProperties(const FontSpec &f)
{
    QByteArray e = "<rPr>";
    if (f.bold) e += "<b/>";
    if (f.italic) e += "<i/>";
    if (f.strike) e += "<strike/>";
    switch (f.underline) {
    case Underline::Single: e += "<u/>"; break;
    case Underline::Double: e += "<u val=\"double\"/>"; break;
    case Underline::SingleAccounting: e += "<u val=\"singleAccounting\"/>"; break;
    case Underline::DoubleAccounting: e += "<u val=\"doubleAccounting\"/>"; break;
    default: break;
    }
    if (f.vertAlign == 1) e += "<vertAlign val=\"superscript\"/>";
    else if (f.vertAlign == 2) e += "<vertAlign val=\"subscript\"/>";
    e += "<sz val=\"" + real(f.size) + "\"/>";
    e += colorElement("color", f.color);
    e += "<rFont val=\"" + attributeValue(f.name.isEmpty() ? QStringLiteral("Arial") : f.name) + "\"/>";
    return e + "</rPr>";
}

// The inside of a string with runs of formatting: a run for each, with the font it is in (one in the font of its cell has no properties), and the characters before the first run, if
// there are any, in a run of the cell's font.
QByteArray richRuns(const QString &text, const RichRuns &runs, const QVector<FontSpec> &fonts)
{
    QByteArray out;
    for (int i = 0; i < runs.size(); ++i) {
        const qsizetype from = std::min<qsizetype>(runs[i].start, text.size());
        const qsizetype to = i + 1 < runs.size() ? std::min<qsizetype>(runs[i + 1].start, text.size()) : text.size();
        if (i == 0 && from > 0) out += "<r>" + textElement("t", text.left(from)) + "</r>";
        if (to <= from) continue;
        out += "<r>";
        if (runs[i].font < quint32(fonts.size())) out += runProperties(fonts[int(runs[i].font)]);
        out += textElement("t", text.mid(from, to - from)) + "</r>";
    }
    return out.isEmpty() ? textElement("t", text) : out;
}

// The two inputs of a data table, from its text "TABLE(A1,B2)": the row input cell and the column input cell, either of which is empty for a table of one input.
void tableInputs(const QString &text, QString *r1, QString *r2)
{
    r1->clear();
    r2->clear();
    if (!text.startsWith(QLatin1String("TABLE(")) || !text.endsWith(QLatin1Char(')'))) return;
    const QStringList parts = text.mid(6, text.size() - 7).split(QLatin1Char(','));
    if (!parts.isEmpty()) *r1 = parts[0].trimmed();
    if (parts.size() > 1) *r2 = parts[1].trimmed();
}

class SheetWriter {
public:
    SheetWriter(const Workbook &book, const Sheet &sheet, Out *out, const std::atomic<bool> *cancel, qint64 *stringRefs)
        : m_book(book), m_sheet(sheet), m_out(*out), m_cancel(cancel), m_stringRefs(stringRefs)
    {
    }

    bool write()
    {
        const SheetView &v = m_sheet.view;
        m_out += kXmlHead;
        m_out += QByteArray("<worksheet xmlns=\"") + kMainNs + "\" xmlns:r=\"" + kRelNs + "\">";
        const QByteArray tab = colorElement("tabColor", m_sheet.tabColor), fit = fitToPageXml(m_sheet.page, QByteArray());
        QByteArray transition;
        if (m_sheet.transitionEvaluation) transition += attr("transitionEvaluation", "1");
        if (m_sheet.transitionEntry) transition += attr("transitionEntry", "1");
        if (!tab.isEmpty() || !fit.isEmpty() || !transition.isEmpty()) m_out += "<sheetPr" + transition + ">" + tab + fit + "</sheetPr>";
        if (m_sheet.dimension.valid) m_out += "<dimension" + attr("ref", rangeName(m_sheet.dimension).toLatin1()) + "/>";
        // how the sheet was left on the screen
        QByteArray view = "<sheetView";
        if (!v.showGridLines) view += attr("showGridLines", "0");
        if (!v.showHeadings) view += attr("showRowColHeaders", "0");
        if (!v.showZeros) view += attr("showZeros", "0");
        if (v.rightToLeft) view += attr("rightToLeft", "1");
        if (v.tabSelected) view += attr("tabSelected", "1");
        if (v.zoom != 100 && v.zoom > 0) view += attr("zoomScale", num(v.zoom)) + attr("zoomScaleNormal", num(v.zoom));
        if (v.topRow || v.leftCol) view += attr("topLeftCell", cellName(v.topRow, v.leftCol).toLatin1());
        view += attr("workbookViewId", "0") + ">";
        QByteArray activePane;
        if (v.split && (v.splitX || v.splitY)) {
            activePane = v.splitX && v.splitY ? "bottomRight" : v.splitY ? "bottomLeft" : "topRight";
            view += "<pane";
            if (v.splitX) view += attr("xSplit", num(v.splitX));
            if (v.splitY) view += attr("ySplit", num(v.splitY));
            view += attr("topLeftCell", cellName(v.paneRow, v.paneCol).toLatin1()) + attr("activePane", activePane) + attr("state", "split") + "/>";
        } else if (v.frozen && (v.frozenRows || v.frozenCols)) {
            activePane = v.frozenRows && v.frozenCols ? "bottomRight" : v.frozenRows ? "bottomLeft" : "topRight";
            view += "<pane";
            if (v.frozenCols) view += attr("xSplit", num(v.frozenCols));
            if (v.frozenRows) view += attr("ySplit", num(v.frozenRows));
            view += attr("topLeftCell", cellName(v.paneRow, v.paneCol).toLatin1()) + attr("activePane", activePane) + attr("state", "frozen") + "/>";
        }
        view += "<selection";
        if (!activePane.isEmpty()) view += attr("pane", activePane);
        const QByteArray active = cellName(v.activeRow, v.activeCol).toLatin1();
        view += attr("activeCell", active) + attr("sqref", active) + "/></sheetView>";
        m_out += "<sheetViews>" + view + "</sheetViews>";
        // the sizes of rows and columns that have none of their own
        const SheetFormat &f = m_sheet.format;
        QByteArray fmt = "<sheetFormatPr" + attr("baseColWidth", num(f.baseColWidth));
        if (f.defaultColWidth > 0) fmt += attr("defaultColWidth", real(f.defaultColWidth));
        fmt += attr("defaultRowHeight", real(f.defaultRowHeight));
        if (f.customHeight) fmt += attr("customHeight", "1");
        if (f.zeroHeight) fmt += attr("zeroHeight", "1");
        m_out += fmt + "/>";
        if (!m_sheet.columns.isEmpty()) {
            QByteArray cols = "<cols>";
            for (const ColumnSpan &c : m_sheet.columns) {
                cols += "<col" + attr("min", num(qint64(c.first) + 1)) + attr("max", num(qint64(c.last) + 1));
                if (c.width >= 0) cols += attr("width", real(c.width));
                if (c.style) cols += attr("style", num(c.style));
                if (c.hidden) cols += attr("hidden", "1");
                if (c.customWidth) cols += attr("customWidth", "1");
                cols += "/>";
            }
            m_out += cols + "</cols>";
        }
        if (!cells()) return false;
        if (m_sheet.isProtected) m_out += "<sheetProtection sheet=\"1\" objects=\"1\" scenarios=\"1\"/>";
        m_out += scenariosXml(m_sheet.scenarios);
        if (!m_sheet.merges.isEmpty()) {
            QByteArray merges = "<mergeCells" + attr("count", num(m_sheet.merges.size())) + ">";
            for (const CellRange &r : m_sheet.merges) merges += "<mergeCell" + attr("ref", rangeName(r).toLatin1()) + "/>";
            m_out += merges + "</mergeCells>";
        }
        m_out += pageSetupElementsXml(m_sheet.page, QByteArray());
        if (!m_sheet.importedCharts.isEmpty() || !m_sheet.importedDrawing.isEmpty()) m_out += "<drawing r:id=\"rId1\"/>";   // (the drawing the charts of an older workbook are in: see writeBaseXlsx)
        m_out += "</worksheet>";
        m_out.flush();
        return !m_stopped;
    }

private:
    const QByteArray &column(quint32 col)
    {
        if (quint32(m_columns.size()) <= col) m_columns.resize(int(col) + 1);
        QByteArray &name = m_columns[int(col)];
        if (name.isEmpty()) name = columnName(col).toLatin1();
        return name;
    }

    void openRow(quint32 row, const RowSpec *spec)
    {
        QByteArray r = "<row" + attr("r", num(qint64(row) + 1));
        // (a sheet that hides the rows it does not list: a row with cells and no row of its own is one of those, and says so)
        if (!spec && m_sheet.format.zeroHeight) r += attr("hidden", "1");
        if (spec) {
            if (spec->style) r += attr("s", num(spec->style));
            if (spec->customFormat) r += attr("customFormat", "1");
            if (spec->height >= 0) r += attr("ht", real(spec->height));
            if (spec->hidden) r += attr("hidden", "1");
            if (spec->customHeight) r += attr("customHeight", "1");
        }
        m_out += r + ">";
        m_rowOpen = true;
    }
    void closeRow()
    {
        if (m_rowOpen) m_out += "</row>";
        m_rowOpen = false;
    }
    // The rows that have a spec and no cells, up to (not including) `before`.
    void rowsWithoutCells(quint32 before)
    {
        while (m_spec < m_specs.size() && m_specs[m_spec].row < before) {
            const RowSpec &s = m_specs[m_spec++];
            if (m_spec < m_specs.size() && m_specs[m_spec].row == s.row) continue;   // (a row the file names twice: the last stands)
            openRow(s.row, &s);
            closeRow();
        }
    }

    void cell(quint32 row, quint32 col, const Cell &c)
    {
        QByteArray e = "<c" + attr("r", column(col) + num(qint64(row) + 1));
        if (c.style) e += attr("s", num(c.style));
        const bool formula = c.hasFormula() && !(c.flags & ArrayMember);
        QByteArray f;
        if (formula) {
            const QString text = m_sheet.formulaText(c);
            if (c.flags & DataTable) {
                QString r1, r2;
                tableInputs(text, &r1, &r2);
                const CellRange range = m_sheet.formulaRange(c);
                if ((!r1.isEmpty() || !r2.isEmpty()) && range.valid) {
                    f = "<f t=\"dataTable\"" + attr("ref", rangeName(range).toLatin1());
                    if (!r1.isEmpty() && !r2.isEmpty()) f += attr("dt2D", "1") + attr("dtr", "1") + attr("r1", r1.toLatin1()) + attr("r2", r2.toLatin1());
                    else if (!r1.isEmpty()) f += attr("dtr", "1") + attr("r1", r1.toLatin1());   // (one input, a row input cell)
                    else f += attr("r1", r2.toLatin1());                                          // (one input, a column input cell)
                    f += "/>";
                }
            } else if (!text.isEmpty()) {
                if (c.flags & ArrayFormula) {
                    const CellRange range = m_sheet.formulaRange(c);
                    f = range.valid ? "<f t=\"array\"" + attr("ref", rangeName(range).toLatin1()) + ">" : QByteArray("<f>");
                } else {
                    f = "<f>";
                }
                f += escapeText(text) + "</f>";
            }
        }
        QByteArray v;
        switch (c.type) {
        case CellType::Number:
        case CellType::Date:
            if (std::isfinite(c.number)) {
                v = "<v>" + real(c.number) + "</v>";
            } else {   // (the XML form has no such number)
                e += attr("t", "e");
                v = "<v>#NUM!</v>";
            }
            break;
        case CellType::Bool: e += attr("t", "b"); v = c.number != 0 ? "<v>1</v>" : "<v>0</v>"; break;
        case CellType::Error: e += attr("t", "e"); v = "<v>" + escapeText(errorText(CellError(quint8(c.number)))) + "</v>"; break;
        case CellType::SharedString:
            e += attr("t", "s");
            v = "<v>" + num(c.textIndex()) + "</v>";
            ++*m_stringRefs;
            break;
        case CellType::InlineString: {
            e += attr("t", "inlineStr");
            const QString text = m_sheet.text(c, m_book.sharedStrings());
            const auto runs = m_sheet.richTexts.constFind(c.textIndex());
            v = "<is>" + (runs != m_sheet.richTexts.cend() ? richRuns(text, runs.value(), m_sheet.richFonts) : textElement("t", text)) + "</is>";
            break;
        }
        case CellType::FormulaString: e += attr("t", "str"); v = textElement("v", m_sheet.text(c, m_book.sharedStrings())); break;
        default: break;
        }
        if (f.isEmpty() && v.isEmpty()) m_out += e + "/>";
        else m_out += e + ">" + f + v + "</c>";
    }

    bool cells()
    {
        m_specs = m_sheet.placedRowSpecs();
        std::stable_sort(m_specs.begin(), m_specs.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
        m_out += "<sheetData>";
        quint32 current = 0;
        bool any = false;
        qint64 done = 0;
        m_sheet.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (m_stopped) return;
            if (++done % 8192 == 0 && m_cancel && m_cancel->load(std::memory_order_relaxed)) {
                m_stopped = true;
                return;
            }
            if (!any || row != current) {
                closeRow();
                rowsWithoutCells(row);
                // (the row's own spec: the last the file gave it)
                const RowSpec *spec = nullptr;
                while (m_spec < m_specs.size() && m_specs[m_spec].row <= row) {
                    if (m_specs[m_spec].row == row) spec = &m_specs[m_spec];
                    ++m_spec;
                }
                openRow(row, spec);
                current = row;
                any = true;
            }
            cell(row, col, c);
        });
        if (m_stopped) return false;
        closeRow();
        rowsWithoutCells(0xFFFFFFFFu);
        m_out += "</sheetData>";
        return true;
    }

    const Workbook &m_book;
    const Sheet &m_sheet;
    Out &m_out;
    const std::atomic<bool> *m_cancel;
    qint64 *m_stringRefs;
    QVector<QByteArray> m_columns;
    QVector<RowSpec> m_specs;
    qsizetype m_spec = 0;
    bool m_rowOpen = false;
    bool m_stopped = false;
};

// The shared strings part with the first `count` strings (the file's: an edit may have added more since, which the changes carry).
QByteArray sharedStringsPart(const SharedStrings &shared, qsizetype count, qint64 references)
{
    QByteArray o = kXmlHead;
    o.reserve(qsizetype(shared.chars * 2 + count * 24 + 256));
    o += QByteArray("<sst xmlns=\"") + kMainNs + "\"" + attr("count", num(std::max<qint64>(references, count))) + attr("uniqueCount", num(count)) + ">";
    for (qsizetype i = 0; i < count && i < shared.strings.size(); ++i) {
        const auto runs = shared.rich.constFind(qint32(i));
        o += "<si>" + (runs != shared.rich.cend() ? richRuns(shared.strings[int(i)], runs.value(), shared.richFonts) : textElement("t", shared.strings[int(i)])) + "</si>";
    }
    return o + "</sst>";
}

} // namespace

namespace {

// Where the charts of a sheet of the older form lie: the record gives how far into a cell a corner is as a part of the cell, so the sizes of the sheet's own columns and rows turn that
// into EMU. Every place is kept to the grid of the XML form, and an end is never before its start.
class ChartGrid {
public:
    explicit ChartGrid(const Sheet &sheet) : m_sheet(sheet)
    {
        for (const RowSpec &r : sheet.rowSpecs) m_rows.insert(r.row, r);
    }
    DrawingAnchor anchorOf(const ImportedChart &c) const
    {
        DrawingAnchor a;
        if (c.absolute) {
            a.kind = AnchorKind::Absolute;
            a.x = a.y = 0;
            a.cx = 9144000;   // (a page: 10 by 7 inches)
            a.cy = 6400800;
            return a;
        }
        a.from.col = std::min<quint32>(c.col1, 16383);
        a.from.row = std::min<quint32>(c.row1, 1048575);
        a.to.col = std::min<quint32>(c.col2, 16383);
        a.to.row = std::min<quint32>(c.row2, 1048575);
        a.from.colOff = c.dx1 * columnEmu(a.from.col) / 1024;
        a.from.rowOff = c.dy1 * rowEmu(a.from.row) / 256;
        a.to.colOff = c.dx2 * columnEmu(a.to.col) / 1024;
        a.to.rowOff = c.dy2 * rowEmu(a.to.row) / 256;
        if (a.to.col < a.from.col) {
            a.to.col = a.from.col;
            a.to.colOff = a.from.colOff;
        }
        if (a.to.row < a.from.row) {
            a.to.row = a.from.row;
            a.to.rowOff = a.from.rowOff;
        }
        if (a.to.col == a.from.col && a.to.colOff < a.from.colOff) a.to.colOff = a.from.colOff;
        if (a.to.row == a.from.row && a.to.rowOff < a.from.rowOff) a.to.rowOff = a.from.rowOff;
        return a;
    }

private:
    qint64 columnEmu(quint32 col) const
    {
        double width = m_sheet.format.defaultColWidth > 0 ? m_sheet.format.defaultColWidth : 9.140625;
        for (const ColumnSpan &c : m_sheet.columns)
            if (col >= c.first && col <= c.last) {
                if (c.hidden) return 0;
                if (c.width >= 0) width = c.width;
                break;
            }
        return qint64(width * 7.0) * 9525;
    }
    qint64 rowEmu(quint32 row) const
    {
        double height = m_sheet.format.defaultRowHeight > 0 ? m_sheet.format.defaultRowHeight : 15;
        const auto it = m_rows.constFind(row);
        if (it != m_rows.constEnd()) {
            if (it->hidden) return 0;
            if (it->height >= 0) height = it->height;
        }
        return qint64(height * 12700.0);
    }
    const Sheet &m_sheet;
    QHash<quint32, RowSpec> m_rows;
};

} // namespace

std::shared_ptr<const Package> makeObjectPackage(const Workbook &book)
{
    const ForeignBook *foreign = book.foreignBook();
    if (!foreign) return nullptr;
    const QString drawingType = QString(kRelNs) + QStringLiteral("/drawing"), imageType = QString(kRelNs) + QStringLiteral("/image");
    PackageWriter writer;
    int drawings = 0, mediaFiles = 0;
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        if (!foreign->mayHaveDrawings(i)) continue;
        Sheet sheet;
        if (!book.readSheet(i, &sheet) || sheet.importedDrawing.isEmpty()) continue;
        const ImportedDrawing &d = sheet.importedDrawing;
        const QString sheetPart = baseSheetPart(i), drawingPart = QStringLiteral("/xl/drawings/drawing%1.xml").arg(++drawings);
        writer.addPart(sheetPart, QByteArray("<worksheet/>"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
        QByteArray anchors;
        for (const QByteArray &a : d.anchors) anchors += a;
        writer.addPart(drawingPart, newDrawingXml(anchors), QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"));
        writer.addRelationshipWithId(sheetPart, QStringLiteral("rId1"), drawingType, drawingPart);
        QVector<QString> partOfMedia;
        for (const ImportedMedia &m : d.media) {
            const QString extension = m.name.mid(m.name.lastIndexOf(QLatin1Char('.')) + 1).toLower();
            const char *type = odt::contentTypeOf(extension);
            const QString part = QStringLiteral("/xl/media/image%1.%2").arg(++mediaFiles).arg(extension);
            writer.addPart(part, m.bytes, type ? QString::fromLatin1(type) : QStringLiteral("application/octet-stream"));
            partOfMedia.append(part);
        }
        for (const ImportedRel &r : d.rels)
            if (r.media >= 0 && r.media < partOfMedia.size()) writer.addRelationshipWithId(drawingPart, r.id, imageType, partOfMedia[r.media]);
    }
    if (drawings == 0) return nullptr;
    auto package = std::make_shared<Package>();
    QString why;
    if (!package->readLazy(writer.finish(), &why)) return nullptr;
    return package;
}

bool writeBaseXlsx(const Workbook &book, QByteArray *out, QString *error, const std::atomic<bool> *cancel)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    if (!book.isConverted()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook is not one that was read from a file of another form."));
    // The file is made of the sheets that were read, as they were read (by the names they had, whether or not a person has since renamed, deleted, or moved them: the changes are put in
    // this file after, as in any other).
    WorkbookInfo info = book.info();
    {
        QVector<SheetEntry> read;
        for (SheetEntry e : info.sheets) {
            if (e.isNew) continue;
            if (!e.fileName.isEmpty()) e.name = e.fileName;
            e.deleted = false;
            read.append(e);
        }
        info.sheets = read;
    }
    ZipWriter zip;
    // The charts of the older form are counted first, so that the content types (the first entry of the file) can name them: the drawing of a sheet, and the parts of its charts.
    std::vector<int> drawingOf(size_t(info.sheets.size()), 0), firstChart(size_t(info.sheets.size()), 0);
    int drawings = 0, charts = 0;
    for (int i = 0; i < info.sheets.size(); ++i) {
        const int n = book.importedChartCount(i);
        if (n <= 0) continue;
        drawingOf[size_t(i)] = ++drawings;
        firstChart[size_t(i)] = charts + 1;
        charts += n;
    }
    // The drawing objects of a workbook read from an OpenDocument spreadsheet: a sheet that may have some is read to find them (and what they come to is kept, so that what is written is what was
    // counted), and the pictures are named by the extension they will have.
    std::vector<ImportedDrawing> importedOf(size_t(info.sheets.size()));
    std::vector<std::unique_ptr<Sheet>> readFirst(size_t(info.sheets.size()));   // (a sheet read to find its objects is kept to be written: it is read once)
    QSet<QString> mediaExtensions;
    if (const ForeignBook *foreign = book.foreignBook()) {
        for (int i = 0; i < info.sheets.size(); ++i) {
            if (cancel && cancel->load(std::memory_order_relaxed)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            if (drawingOf[size_t(i)] != 0 || !foreign->mayHaveDrawings(i)) continue;
            Sheet look;
            if (!book.readSheet(i, &look) || look.importedDrawing.isEmpty()) continue;
            importedOf[size_t(i)] = std::move(look.importedDrawing);
            readFirst[size_t(i)] = std::make_unique<Sheet>(std::move(look));
            drawingOf[size_t(i)] = ++drawings;
            for (const ImportedMedia &m : std::as_const(importedOf[size_t(i)].media)) mediaExtensions.insert(m.name.mid(m.name.lastIndexOf(QLatin1Char('.')) + 1).toLower());
        }
    }
    // [Content_Types].xml and the relationships
    {
        QByteArray types = kXmlHead;
        types += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                 "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                 "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>";
        for (int i = 0; i < info.sheets.size(); ++i)
            types += "<Override PartName=\"" + baseSheetPart(i).toLatin1() + "\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
        types += "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
                 "<Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/>";
        for (int d = 1; d <= drawings; ++d) types += "<Override PartName=\"/xl/drawings/drawing" + num(d) + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>";
        for (const QString &e : std::as_const(mediaExtensions))
            if (const char *type = odt::contentTypeOf(e)) types += "<Default Extension=\"" + e.toLatin1() + "\" ContentType=\"" + type + "\"/>";
        for (int c = 1; c <= charts; ++c) types += "<Override PartName=\"/xl/charts/chart" + num(c) + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/>";
        types += "</Types>";
        zip.add(QStringLiteral("[Content_Types].xml"), types, true);
        zip.add(QStringLiteral("_rels/.rels"),
                QByteArray(kXmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"" + kRelNs
                    + "/officeDocument\" Target=\"xl/workbook.xml\"/></Relationships>",
                true);
        QByteArray rels = kXmlHead;
        rels += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
        for (int i = 0; i < info.sheets.size(); ++i)
            rels += "<Relationship Id=\"rId" + num(i + 1) + "\" Type=\"" + kRelNs + "/worksheet\" Target=\"worksheets/sheet" + num(i + 1) + ".xml\"/>";
        rels += "<Relationship Id=\"rId" + num(info.sheets.size() + 1) + "\" Type=\"" + kRelNs + "/styles\" Target=\"styles.xml\"/>";
        rels += "<Relationship Id=\"rId" + num(info.sheets.size() + 2) + "\" Type=\"" + kRelNs + "/sharedStrings\" Target=\"sharedStrings.xml\"/></Relationships>";
        zip.add(QStringLiteral("xl/_rels/workbook.xml.rels"), rels, true);
    }
    zip.add(QStringLiteral("xl/workbook.xml"), workbookPart(info), true);
    QSet<int> addedFormats;
    for (const Workbook::AddedNumberFormat &f : book.addedNumberFormats()) addedFormats.insert(f.id);
    zip.add(QStringLiteral("xl/styles.xml"), stylesPart(book.styles(), qsizetype(book.cellFormatsInFile()), addedFormats), true);
    // the sheets, one at a time
    qint64 references = 0;
    int mediaFiles = 0;
    for (int i = 0; i < info.sheets.size(); ++i) {
        if (cancel && cancel->load(std::memory_order_relaxed)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
        Sheet sheet;
        if (readFirst[size_t(i)]) {
            sheet = std::move(*readFirst[size_t(i)]);
            readFirst[size_t(i)].reset();
        } else if (!book.readSheet(i, &sheet)) return fail(sheet.error.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "A sheet of the workbook could not be read to be saved.") : sheet.error);
        if (!sheet.error.isEmpty()) return fail(sheet.error);
        sheet.importedDrawing = importedOf[size_t(i)];
        zip.beginStream(baseSheetPart(i).mid(1), true);
        Out sink(&zip);
        SheetWriter writer(book, sheet, &sink, cancel, &references);
        const bool ok = writer.write();
        zip.endStream();
        if (!ok) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
        if (!sheet.importedCharts.isEmpty()) {
            // the charts of the older form: one drawing for the sheet, a part for each chart, and the relationships between them
            if (sheet.importedCharts.size() != book.importedChartCount(i)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A sheet's charts changed while the workbook was being saved."));
            QByteArray anchors;
            QByteArray drawingRels = kXmlHead;
            drawingRels += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
            int k = 0;
            const ChartGrid grid(sheet);
            for (const ImportedChart &chart : std::as_const(sheet.importedCharts)) {
                ++k;
                const int number = firstChart[size_t(i)] + k - 1;
                anchors += chartAnchorXml(grid.anchorOf(chart), k + 1, QStringLiteral("Chart %1").arg(k), QStringLiteral("rId%1").arg(k));
                zip.add(QStringLiteral("xl/charts/chart%1.xml").arg(number), chart.part, true);
                drawingRels += "<Relationship Id=\"rId" + num(k) + "\" Type=\"" + kRelNs + "/chart\" Target=\"../charts/chart" + num(number) + ".xml\"/>";
            }
            drawingRels += "</Relationships>";
            const int drawing = drawingOf[size_t(i)];
            zip.add(QStringLiteral("xl/drawings/drawing%1.xml").arg(drawing), newDrawingXml(anchors), true);
            zip.add(QStringLiteral("xl/drawings/_rels/drawing%1.xml.rels").arg(drawing), drawingRels, true);
            zip.add(QStringLiteral("xl/worksheets/_rels/sheet%1.xml.rels").arg(i + 1),
                    QByteArray(kXmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"" + kRelNs + "/drawing\" Target=\"../drawings/drawing" +
                        num(drawing) + ".xml\"/></Relationships>",
                    true);
        }
        if (!sheet.importedDrawing.isEmpty()) {
            // the drawing objects of an OpenDocument sheet: their anchors, and the relationships to the pictures (each picture a file of its own in the package's folder of media)
            QByteArray anchors;
            for (const QByteArray &a : std::as_const(sheet.importedDrawing.anchors)) anchors += a;
            QByteArray drawingRels = kXmlHead;
            drawingRels += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
            QVector<int> fileOfMedia(sheet.importedDrawing.media.size(), 0);
            for (int m = 0; m < sheet.importedDrawing.media.size(); ++m) {
                const ImportedMedia &media = sheet.importedDrawing.media[m];
                const QString extension = media.name.mid(media.name.lastIndexOf(QLatin1Char('.')) + 1).toLower();
                fileOfMedia[m] = ++mediaFiles;
                zip.add(QStringLiteral("xl/media/image%1.%2").arg(mediaFiles).arg(extension), media.bytes, extension != QLatin1String("png") && extension != QLatin1String("jpg") && extension != QLatin1String("jpeg"));
            }
            for (const ImportedRel &rel : std::as_const(sheet.importedDrawing.rels)) {
                if (rel.media < 0 || rel.media >= sheet.importedDrawing.media.size()) continue;
                const QString name = sheet.importedDrawing.media[rel.media].name;
                const QString extension = name.mid(name.lastIndexOf(QLatin1Char('.')) + 1).toLower();
                drawingRels += "<Relationship Id=\"" + rel.id.toUtf8() + "\" Type=\"" + kRelNs + "/image\" Target=\"../media/image" + num(fileOfMedia[rel.media]) + "." + extension.toUtf8() + "\"/>";
            }
            drawingRels += "</Relationships>";
            const int drawing = drawingOf[size_t(i)];
            zip.add(QStringLiteral("xl/drawings/drawing%1.xml").arg(drawing), newDrawingXml(anchors), true);
            zip.add(QStringLiteral("xl/drawings/_rels/drawing%1.xml.rels").arg(drawing), drawingRels, true);
            zip.add(QStringLiteral("xl/worksheets/_rels/sheet%1.xml.rels").arg(i + 1),
                    QByteArray(kXmlHead) + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"" + kRelNs + "/drawing\" Target=\"../drawings/drawing" +
                        num(drawing) + ".xml\"/></Relationships>",
                    true);
        }
    }
    zip.add(QStringLiteral("xl/sharedStrings.xml"), sharedStringsPart(book.sharedStrings(), qsizetype(book.sharedStringsInFile()), references), true);
    *out = zip.finish();
    return true;
}


// The `scenarios` element for a sheet's scenarios (empty for none).
QByteArray scenariosXml(const ScenarioList &list)
{
    QByteArray generated;
    if (!list.list.isEmpty()) {
        auto text = [](const QString &s) { return xmlscan::escapeAttribute(s); };
        generated = "<scenarios";
        if (list.current > 0) generated += " current=\"" + QByteArray::number(list.current) + "\"";
        if (list.show > 0) generated += " show=\"" + QByteArray::number(list.show) + "\"";
        if (!list.sqref.isEmpty()) generated += " sqref=\"" + text(list.sqref) + "\"";
        generated += ">";
        int written = 0;
        for (const Scenario &s : list.list) {
            if (++written > ScenarioList::kMaxScenarios) break;
            generated += "<scenario name=\"" + text(s.name) + "\"";
            if (s.locked) generated += " locked=\"1\"";
            if (s.hidden) generated += " hidden=\"1\"";
            generated += " count=\"" + QByteArray::number(int(s.cells.size())) + "\"";
            if (!s.user.isEmpty()) generated += " user=\"" + text(s.user) + "\"";
            if (!s.comment.isEmpty()) generated += " comment=\"" + text(s.comment) + "\"";
            generated += ">";
            for (const ScenarioCell &c : s.cells) {
                generated += "<inputCells r=\"" + text(c.ref) + "\"";
                if (c.deleted) generated += " deleted=\"1\"";
                if (c.undone) generated += " undone=\"1\"";
                generated += " val=\"" + text(c.value) + "\"";
                if (c.numFmtId >= 0) generated += " numFmtId=\"" + QByteArray::number(c.numFmtId) + "\"";
                generated += "/>";
            }
            generated += "</scenario>";
        }
        generated += "</scenarios>";
    }
    return generated;
}

QByteArray richRunsXml(const QString &text, const RichRuns &runs, const QVector<FontSpec> &fonts) { return richRuns(text, runs, fonts); }
QByteArray colorXml(const char *name, const ColorSpec &c) { return colorElement(name, c); }
QByteArray fontXml(const FontSpec &f) { return fontElement(f); }
QByteArray fillXml(const FillSpec &f) { return fillElement(f); }
QByteArray borderXml(const BorderSpec &b) { return borderElement(b); }
QByteArray xfXml(const CellFormat &f, int xfId, bool cellFormat) { return xfElement(f, xfId, cellFormat); }
QByteArray alignmentXml(const CellFormat &f)
{
    const QByteArray xf = xfElement(f, 0, false);
    const qsizetype at = xf.indexOf("<alignment");
    if (at < 0) return QByteArray();
    const qsizetype end = xf.indexOf("/>", at);
    return end < 0 ? QByteArray() : xf.mid(at, end + 2 - at);
}

} // namespace jp::office::sheet
