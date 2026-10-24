// The file a workbook of the older binary form is saved as: see xlsxbase.h.

#include "sheet/xlsxbase.h"

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

QString say(const char *text) { return QCoreApplication::translate("jp::office::sheet::Save", text); }

const char kXmlHead[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
const char kMainNs[] = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
const char kRelNs[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

QByteArray num(qint64 v) { return QByteArray::number(v); }
QByteArray real(double v) { return QByteArray::number(v, 'g', QLocale::FloatingPointShortest); }
QByteArray attr(const char *name, const QByteArray &value) { return QByteArray(" ") + name + "=\"" + value + "\""; }
QByteArray attrText(const char *name, const QString &value) { return attr(name, escapeAttribute(value)); }

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
    e += "<name val=\"" + escapeAttribute(f.name.isEmpty() ? QStringLiteral("Arial") : f.name) + "\"/>";
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
    if (!f.locked) e += attr("applyProtection", "1");
    QByteArray inner;
    const char *h = horizontalName(f.horizontal);
    const char *v = verticalName(f.vertical);
    if (h || v || f.wrap || f.shrink || f.indent || f.rotation) {
        inner += "<alignment";
        if (h) inner += attr("horizontal", h);
        if (v) inner += attr("vertical", v);
        if (f.rotation) inner += attr("textRotation", num(f.rotation));
        if (f.wrap) inner += attr("wrapText", "1");
        if (f.indent) inner += attr("indent", num(f.indent));
        if (f.shrink) inner += attr("shrinkToFit", "1");
        inner += "/>";
    }
    if (!f.locked) inner += "<protection locked=\"0\"/>";
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
    // the formats of the named styles, then the cell formats (one for each the workbook has, in its order, so that a cell's place is the same here)
    QVector<CellFormat> formats = st.formats.mid(0, formatCount);
    if (formats.isEmpty()) formats.append(CellFormat());
    QHash<quint32, int> styleAt;
    QVector<quint32> styleFormats;
    for (int i = 0; i < formats.size(); ++i)
        if (formats[i].isStyle) {
            styleAt.insert(quint32(i), int(styleFormats.size()));
            styleFormats.append(quint32(i));
        }
    o += "<cellStyleXfs" + attr("count", num(std::max<qsizetype>(1, styleFormats.size()))) + ">";
    if (styleFormats.isEmpty()) o += xfElement(CellFormat(), 0, false);
    for (const quint32 i : std::as_const(styleFormats)) o += xfElement(formats[int(i)], 0, false);
    o += "</cellStyleXfs>";
    o += "<cellXfs" + attr("count", num(formats.size())) + ">";
    for (int i = 0; i < formats.size(); ++i) {
        int parent = 0;
        if (formats[i].isStyle) parent = styleAt.value(quint32(i), 0);
        else if (formats[i].parent != CellFormat::kNoParent) parent = styleAt.value(formats[i].parent, 0);
        o += xfElement(formats[i], parent, true);
    }
    o += "</cellXfs>";
    // the named styles (a file has a Normal one)
    QByteArray named;
    int namedCount = 0;
    QSet<QString> seen;
    for (const NamedStyle &ns : st.namedStyles) {
        const auto at = styleAt.constFind(ns.format);
        if (at == styleAt.cend() || ns.name.isEmpty() || seen.contains(ns.name.toLower())) continue;
        seen.insert(ns.name.toLower());
        named += "<cellStyle" + attrText("name", ns.name) + attr("xfId", num(at.value()));
        if (ns.builtin >= 0) named += attr("builtinId", num(ns.builtin));
        named += "/>";
        ++namedCount;
    }
    if (namedCount == 0) {
        named = "<cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/>";
        namedCount = 1;
    }
    o += "<cellStyles" + attr("count", num(namedCount)) + ">" + named + "</cellStyles>";
    o += "<dxfs count=\"0\"/>";
    if (!st.indexed.isEmpty()) {
        o += "<colors><indexedColors>";
        for (const quint32 c : st.indexed) o += "<rgbColor rgb=\"FF" + QByteArray::number(c & 0xFFFFFF, 16).rightJustified(6, '0').toUpper() + "\"/>";
        o += "</indexedColors></colors>";
    }
    return o + "</styleSheet>";
}

// ---- the workbook

QByteArray workbookPart(const WorkbookInfo &info)
{
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
        for (const DefinedName &n : info.definedNames) {
            o += "<definedName" + attrText("name", n.name);
            if (n.localSheet >= 0) o += attr("localSheetId", num(n.localSheet));
            if (n.hidden) o += attr("hidden", "1");
            o += ">" + escapeText(n.ref) + "</definedName>";
        }
        o += "</definedNames>";
    }
    o += "<calcPr";
    if (info.calcMode == CalcMode::Manual) o += attr("calcMode", "manual");
    else if (info.calcMode == CalcMode::AutoNoTable) o += attr("calcMode", "autoNoTable");
    if (info.iterate) o += attr("iterate", "1");
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

// The two inputs of a data table, from its text "TABLE(A1,B2)".
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
        const QByteArray tab = colorElement("tabColor", m_sheet.tabColor);
        if (!tab.isEmpty()) m_out += "<sheetPr>" + tab + "</sheetPr>";
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
        if (v.frozen && (v.frozenRows || v.frozenCols)) {
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
        if (!m_sheet.merges.isEmpty()) {
            QByteArray merges = "<mergeCells" + attr("count", num(m_sheet.merges.size())) + ">";
            for (const CellRange &r : m_sheet.merges) merges += "<mergeCell" + attr("ref", rangeName(r).toLatin1()) + "/>";
            m_out += merges + "</mergeCells>";
        }
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

    void cell(const Cell &c)
    {
        QByteArray e = "<c" + attr("r", column(c.col) + num(qint64(c.row) + 1));
        if (c.style) e += attr("s", num(c.style));
        const bool formula = c.hasFormula() && !(c.flags & ArrayMember);
        QByteArray f;
        if (formula) {
            const QString text = m_sheet.formulaText(c);
            if (c.flags & DataTable) {
                QString r1, r2;
                tableInputs(text, &r1, &r2);
                const CellRange range = m_sheet.formulaRange(c);
                if (!r1.isEmpty() && range.valid) {
                    f = "<f t=\"dataTable\"" + attr("ref", rangeName(range).toLatin1());
                    if (!r2.isEmpty()) f += attr("dt2D", "1") + attr("dtr", "1");
                    f += attr("r1", r1.toLatin1());
                    if (!r2.isEmpty()) f += attr("r2", r2.toLatin1());
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
        case CellType::Date: v = "<v>" + real(c.number) + "</v>"; break;
        case CellType::Bool: e += attr("t", "b"); v = c.number != 0 ? "<v>1</v>" : "<v>0</v>"; break;
        case CellType::Error: e += attr("t", "e"); v = "<v>" + escapeText(errorText(CellError(quint8(c.number)))) + "</v>"; break;
        case CellType::SharedString:
            e += attr("t", "s");
            v = "<v>" + num(c.textIndex()) + "</v>";
            ++*m_stringRefs;
            break;
        case CellType::InlineString: e += attr("t", "inlineStr"); v = "<is>" + textElement("t", m_sheet.text(c, m_book.sharedStrings())) + "</is>"; break;
        case CellType::FormulaString: e += attr("t", "str"); v = textElement("v", m_sheet.text(c, m_book.sharedStrings())); break;
        default: break;
        }
        if (f.isEmpty() && v.isEmpty()) m_out += e + "/>";
        else m_out += e + ">" + f + v + "</c>";
    }

    bool cells()
    {
        m_specs = m_sheet.rowSpecs;
        std::stable_sort(m_specs.begin(), m_specs.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
        m_out += "<sheetData>";
        quint32 current = 0;
        bool any = false;
        qint64 done = 0;
        m_sheet.cells.forEach([&](const Cell &c) {
            if (m_stopped) return;
            if (++done % 8192 == 0 && m_cancel && m_cancel->load(std::memory_order_relaxed)) {
                m_stopped = true;
                return;
            }
            if (!any || c.row != current) {
                closeRow();
                rowsWithoutCells(c.row);
                // (the row's own spec: the last the file gave it)
                const RowSpec *spec = nullptr;
                while (m_spec < m_specs.size() && m_specs[m_spec].row <= c.row) {
                    if (m_specs[m_spec].row == c.row) spec = &m_specs[m_spec];
                    ++m_spec;
                }
                openRow(c.row, spec);
                current = c.row;
                any = true;
            }
            cell(c);
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
    for (qsizetype i = 0; i < count && i < shared.strings.size(); ++i) o += "<si>" + textElement("t", shared.strings[int(i)]) + "</si>";
    return o + "</sst>";
}

} // namespace

bool writeBaseXlsx(const Workbook &book, QByteArray *out, QString *error, const std::atomic<bool> *cancel)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    if (!book.isBinary()) return fail(say("The workbook is not one that was read from the older file format."));
    const WorkbookInfo &info = book.info();
    ZipWriter zip;
    // [Content_Types].xml and the relationships
    {
        QByteArray types = kXmlHead;
        types += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                 "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                 "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>";
        for (int i = 0; i < info.sheets.size(); ++i)
            types += "<Override PartName=\"" + baseSheetPart(i).toLatin1() + "\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
        types += "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
                 "<Override PartName=\"/xl/sharedStrings.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml\"/></Types>";
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
    for (int i = 0; i < info.sheets.size(); ++i) {
        if (cancel && cancel->load(std::memory_order_relaxed)) return fail(say("The save was stopped."));
        Sheet sheet;
        if (!book.readSheet(i, &sheet)) return fail(sheet.error.isEmpty() ? say("A sheet of the workbook could not be read to be saved.") : sheet.error);
        if (!sheet.error.isEmpty()) return fail(sheet.error);
        zip.beginStream(baseSheetPart(i).mid(1), true);
        Out sink(&zip);
        SheetWriter writer(book, sheet, &sink, cancel, &references);
        const bool ok = writer.write();
        zip.endStream();
        if (!ok) return fail(say("The save was stopped."));
    }
    zip.add(QStringLiteral("xl/sharedStrings.xml"), sharedStringsPart(book.sharedStrings(), qsizetype(book.sharedStringsInFile()), references), true);
    *out = zip.finish();
    return true;
}

} // namespace jp::office::sheet
