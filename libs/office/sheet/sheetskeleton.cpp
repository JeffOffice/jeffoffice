// The XML of a sheet that a person added or copied, before its cells are put in (see xlsxbase.h): its view, default sizes, tab color, and the rules of conditional formatting and
// validation it has, round an empty sheetData.

#include "sheet/xlsxbase.h"

#include "sheet/cellref.h"
#include "sheet/xmlscan.h"

#include <cmath>

namespace jp::office::sheet {

namespace {

using xmlscan::escapeAttribute;
using xmlscan::escapeText;

QByteArray attr(const char *name, const QByteArray &value) { return QByteArray(" ") + name + "=\"" + value + "\""; }
QByteArray num(qint64 v) { return QByteArray::number(v); }
QByteArray real(double v) { return QByteArray::number(v, 'g', QLocale::FloatingPointShortest); }
QByteArray textAttr(const char *name, const QString &value) { return attr(name, escapeAttribute(value)); }

QByteArray sqref(const QVector<CellRange> &ranges)
{
    QByteArray out;
    for (const CellRange &r : ranges) {
        if (!r.valid) continue;
        if (!out.isEmpty()) out += ' ';
        out += rangeName(r).toLatin1();
    }
    return out;
}

const char *cfvoType(CondValue::Type t)
{
    switch (t) {
    case CondValue::Type::Number: return "num";
    case CondValue::Type::Percent: return "percent";
    case CondValue::Type::Max:
    case CondValue::Type::AutoMax: return "max";
    case CondValue::Type::Min:
    case CondValue::Type::AutoMin: return "min";
    case CondValue::Type::Formula: return "formula";
    case CondValue::Type::Percentile: return "percentile";
    }
    return "num";
}

QByteArray cfvo(const CondValue &v, bool iconSet = false, bool first = false)
{
    QByteArray e = QByteArray("<cfvo") + attr("type", cfvoType(v.type));
    if (v.type == CondValue::Type::Number || v.type == CondValue::Type::Percent || v.type == CondValue::Type::Formula || v.type == CondValue::Type::Percentile) e += textAttr("val", v.value);
    if (iconSet && !v.gte && !first) e += attr("gte", "0");
    return e + "/>";
}

QByteArray quotedText(const QString &text)
{
    QString q = text;
    q.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QByteArray("\"") + escapeText(q) + "\"";
}

// What a rule that looks for text, blanks, errors, or a date says as a formula, for its first cell (what the file has in `formula1` when it was read).
QByteArray ruleFormula(const CondRule &r, const QString &first)
{
    using T = CondRule::Type;
    const QString c = first;
    QString f;
    switch (r.type) {
    case T::ContainsText: f = QStringLiteral("NOT(ISERROR(SEARCH(%1,%2)))").arg(QString::fromUtf8(quotedText(r.text)), c); break;
    case T::NotContainsText: f = QStringLiteral("ISERROR(SEARCH(%1,%2))").arg(QString::fromUtf8(quotedText(r.text)), c); break;
    case T::BeginsWith: f = QStringLiteral("LEFT(%1,LEN(%2))=%2").arg(c, QString::fromUtf8(quotedText(r.text))); break;
    case T::EndsWith: f = QStringLiteral("RIGHT(%1,LEN(%2))=%2").arg(c, QString::fromUtf8(quotedText(r.text))); break;
    case T::ContainsBlanks: f = QStringLiteral("LEN(TRIM(%1))=0").arg(c); break;
    case T::NotContainsBlanks: f = QStringLiteral("LEN(TRIM(%1))>0").arg(c); break;
    case T::ContainsErrors: f = QStringLiteral("ISERROR(%1)").arg(c); break;
    case T::NotContainsErrors: f = QStringLiteral("NOT(ISERROR(%1))").arg(c); break;
    default: break;
    }
    return f.toUtf8();
}

const char *operatorName(CondRule::Operator o)
{
    switch (o) {
    case CondRule::Operator::LessThan: return "lessThan";
    case CondRule::Operator::LessThanOrEqual: return "lessThanOrEqual";
    case CondRule::Operator::Equal: return "equal";
    case CondRule::Operator::NotEqual: return "notEqual";
    case CondRule::Operator::GreaterThanOrEqual: return "greaterThanOrEqual";
    case CondRule::Operator::GreaterThan: return "greaterThan";
    case CondRule::Operator::Between: return "between";
    case CondRule::Operator::NotBetween: return "notBetween";
    }
    return "equal";
}

QByteArray colorsXml(const QVector<ColorSpec> &colors)
{
    QByteArray out;
    for (const ColorSpec &c : colors) out += colorXml("color", c);
    return out;
}

const char *kX14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";
const char *kXm = "http://schemas.microsoft.com/office/excel/2006/main";

// Whether a data bar looks other than the plain one a file without an extension has (a gradient fill, no border, the axis where the values say, bars that run to the right, and the usual
// lengths): only then does the file need the extension that says how.
bool needsExtension(const CondRule &r)
{
    return !r.gradient || r.border || r.negativeSameColor || r.rightToLeft || r.axis != CondRule::Axis::Automatic || r.negativeColor.isSet() || r.borderColor.isSet() || r.negativeBorderColor.isSet()
        || r.axisColor.isSet() || r.minLength != 10 || r.maxLength != 90;
}

QByteArray extensionId(int n) { return QByteArray("{00000000-0000-4000-8000-") + QByteArray::number(n, 16).rightJustified(12, '0').toUpper() + "}"; }

// The extension's value of a data bar's end (`x14:cfvo`): the number or formula is a child, not an attribute.
QByteArray x14Value(const CondValue &v)
{
    const char *type = "num";
    switch (v.type) {
    case CondValue::Type::Number: type = "num"; break;
    case CondValue::Type::Percent: type = "percent"; break;
    case CondValue::Type::Max: type = "max"; break;
    case CondValue::Type::AutoMax: type = "autoMax"; break;
    case CondValue::Type::Min: type = "min"; break;
    case CondValue::Type::AutoMin: type = "autoMin"; break;
    case CondValue::Type::Formula: type = "formula"; break;
    case CondValue::Type::Percentile: type = "percentile"; break;
    }
    QByteArray e = QByteArray("<x14:cfvo") + attr("type", type);
    const bool hasValue = v.type == CondValue::Type::Number || v.type == CondValue::Type::Percent || v.type == CondValue::Type::Formula || v.type == CondValue::Type::Percentile;
    return hasValue ? e + "><xm:f>" + escapeText(v.value) + "</xm:f></x14:cfvo>" : e + "/>";
}

} // namespace

QByteArray conditionalFormatsXml(const QVector<CondRule> &rules, QByteArray *extensions)
{
    using T = CondRule::Type;
    QByteArray out;
    QByteArray extended;   // the data bars' extensions, one `x14:conditionalFormatting` for each
    int extensionCount = 0;
    for (const CondRule &r : rules) {
        if (r.ranges.isEmpty()) continue;
        const QByteArray where = sqref(r.ranges);
        if (where.isEmpty()) continue;
        QString first = QStringLiteral("A1");
        for (const CellRange &range : r.ranges)
            if (range.valid) {
                first = cellName(range.row1, range.col1);
                break;
            }
        QByteArray rule = "<cfRule";
        const char *type = "expression";
        switch (r.type) {
        case T::CellIs: type = "cellIs"; break;
        case T::Expression: type = "expression"; break;
        case T::ColorScale: type = "colorScale"; break;
        case T::DataBar: type = "dataBar"; break;
        case T::IconSet: type = "iconSet"; break;
        case T::Top10: type = "top10"; break;
        case T::UniqueValues: type = "uniqueValues"; break;
        case T::DuplicateValues: type = "duplicateValues"; break;
        case T::ContainsText: type = "containsText"; break;
        case T::NotContainsText: type = "notContainsText"; break;
        case T::BeginsWith: type = "beginsWith"; break;
        case T::EndsWith: type = "endsWith"; break;
        case T::ContainsBlanks: type = "containsBlanks"; break;
        case T::NotContainsBlanks: type = "notContainsBlanks"; break;
        case T::ContainsErrors: type = "containsErrors"; break;
        case T::NotContainsErrors: type = "notContainsErrors"; break;
        case T::TimePeriod: type = "timePeriod"; break;
        case T::AboveAverage: type = "aboveAverage"; break;
        }
        rule += attr("type", type);
        if (r.dxf >= 0) rule += attr("dxfId", num(r.dxf));
        rule += attr("priority", num(r.priority > 0 ? r.priority : 1));
        if (r.stopIfTrue) rule += attr("stopIfTrue", "1");
        QByteArray children;
        switch (r.type) {
        case T::CellIs:
            rule += attr("operator", operatorName(r.op));
            children += "<formula>" + escapeText(r.formula1) + "</formula>";
            if (r.op == CondRule::Operator::Between || r.op == CondRule::Operator::NotBetween) children += "<formula>" + escapeText(r.formula2) + "</formula>";
            break;
        case T::Expression: children += "<formula>" + escapeText(r.formula1) + "</formula>"; break;
        case T::ContainsText:
        case T::NotContainsText:
        case T::BeginsWith:
        case T::EndsWith: {
            const char *op = r.type == T::ContainsText ? "containsText" : r.type == T::NotContainsText ? "notContains" : r.type == T::BeginsWith ? "beginsWith" : "endsWith";
            rule += attr("operator", op) + textAttr("text", r.text);
            const QByteArray f = r.formula1.isEmpty() ? ruleFormula(r, first) : r.formula1.toUtf8();
            children += "<formula>" + escapeText(QString::fromUtf8(f)) + "</formula>";
            break;
        }
        case T::ContainsBlanks:
        case T::NotContainsBlanks:
        case T::ContainsErrors:
        case T::NotContainsErrors: {
            const QByteArray f = r.formula1.isEmpty() ? ruleFormula(r, first) : r.formula1.toUtf8();
            children += "<formula>" + escapeText(QString::fromUtf8(f)) + "</formula>";
            break;
        }
        case T::TimePeriod:
            rule += textAttr("timePeriod", r.timePeriod);
            if (!r.formula1.isEmpty()) children += "<formula>" + escapeText(r.formula1) + "</formula>";
            break;
        case T::Top10:
            rule += attr("rank", num(r.rank));
            if (r.percent) rule += attr("percent", "1");
            if (r.bottom) rule += attr("bottom", "1");
            break;
        case T::AboveAverage:
            if (!r.above) rule += attr("aboveAverage", "0");
            if (r.equalAverage) rule += attr("equalAverage", "1");
            if (r.stdDev > 0) rule += attr("stdDev", num(r.stdDev));
            break;
        case T::ColorScale:
            children += "<colorScale>";
            for (const CondValue &v : r.values) children += cfvo(v);
            children += colorsXml(r.colors) + "</colorScale>";
            break;
        case T::DataBar:
            children += "<dataBar" + attr("minLength", num(r.minLength)) + attr("maxLength", num(r.maxLength)) + (r.showValue ? QByteArray() : attr("showValue", "0")) + ">";
            for (const CondValue &v : r.values) children += cfvo(v);
            children += colorsXml(r.colors.isEmpty() ? QVector<ColorSpec>{ColorSpec::fromRgb(0x638EC6)} : QVector<ColorSpec>{r.colors.first()}) + "</dataBar>";
            if (extensions && needsExtension(r)) {
                // (the look a data bar has besides its color: the extension that ties to the rule by an id says how it is filled, bordered, and where its axis is)
                const QByteArray id = extensionId(++extensionCount);
                children += QByteArray("<extLst><ext xmlns:x14=\"") + kX14 + "\" uri=\"{B025F937-C7B1-47D3-B67F-A62EFF666E3E}\"><x14:id>" + id + "</x14:id></ext></extLst>";
                QByteArray bar = QByteArray("<x14:cfRule type=\"dataBar\" id=\"") + id + "\"><x14:dataBar" + attr("minLength", num(r.minLength)) + attr("maxLength", num(r.maxLength));
                bar += attr("gradient", r.gradient ? "1" : "0");
                if (r.border) bar += attr("border", "1");
                if (r.negativeSameColor) bar += attr("negativeBarColorSameAsPositive", "1");
                if (r.axis == CondRule::Axis::Middle) bar += attr("axisPosition", "middle");
                else if (r.axis == CondRule::Axis::None) bar += attr("axisPosition", "none");
                if (r.rightToLeft) bar += attr("direction", "rightToLeft");
                if (!r.showValue) bar += attr("showValue", "0");
                bar += ">";
                for (const CondValue &v : r.values) bar += x14Value(v);
                if (r.borderColor.isSet()) bar += colorXml("x14:borderColor", r.borderColor);
                if (r.negativeColor.isSet()) bar += colorXml("x14:negativeFillColor", r.negativeColor);
                if (r.negativeBorderColor.isSet()) bar += colorXml("x14:negativeBorderColor", r.negativeBorderColor);
                if (r.axisColor.isSet()) bar += colorXml("x14:axisColor", r.axisColor);
                bar += "</x14:dataBar></x14:cfRule>";
                extended += QByteArray("<x14:conditionalFormatting xmlns:xm=\"") + kXm + "\">" + bar + "<xm:sqref>" + where + "</xm:sqref></x14:conditionalFormatting>";
            }
            break;
        case T::IconSet: {
            children += "<iconSet" + textAttr("iconSet", r.iconSet) + (r.reverse ? attr("reverse", "1") : QByteArray()) + (r.showValue ? QByteArray() : attr("showValue", "0")) + ">";
            bool isFirst = true;
            for (const CondValue &v : r.values) {
                children += cfvo(v, true, isFirst);
                isFirst = false;
            }
            children += "</iconSet>";
            break;
        }
        default: break;
        }
        out += "<conditionalFormatting" + attr("sqref", where) + ">" + rule + (children.isEmpty() ? QByteArray("/>") : ">" + children + "</cfRule>") + "</conditionalFormatting>";
    }
    if (extensions)
        *extensions = extended.isEmpty() ? QByteArray()
                                         : QByteArray("<extLst><ext xmlns:x14=\"") + kX14 + "\" uri=\"{78C0D931-6437-407d-A8EE-F0AAD7539E65}\"><x14:conditionalFormattings>" + extended
                                               + "</x14:conditionalFormattings></ext></extLst>";
    return out;
}

QByteArray validationsXml(const QVector<Validation> &rules)
{
    using V = Validation;
    QByteArray items;
    int count = 0;
    for (const Validation &v : rules) {
        if (v.type == V::Type::Any && !v.showInput && !v.showError) continue;
        const QByteArray where = sqref(v.ranges);
        if (where.isEmpty()) continue;
        const char *type = "none";
        switch (v.type) {
        case V::Type::Any: type = "none"; break;
        case V::Type::Whole: type = "whole"; break;
        case V::Type::Decimal: type = "decimal"; break;
        case V::Type::List: type = "list"; break;
        case V::Type::Date: type = "date"; break;
        case V::Type::Time: type = "time"; break;
        case V::Type::TextLength: type = "textLength"; break;
        case V::Type::Custom: type = "custom"; break;
        }
        QByteArray e = "<dataValidation" + attr("type", type);
        if (v.style == V::Style::Warning) e += attr("errorStyle", "warning");
        else if (v.style == V::Style::Information) e += attr("errorStyle", "information");
        const char *op = nullptr;
        switch (v.compare) {
        case V::Compare::Between: break;
        case V::Compare::NotBetween: op = "notBetween"; break;
        case V::Compare::Equal: op = "equal"; break;
        case V::Compare::NotEqual: op = "notEqual"; break;
        case V::Compare::Greater: op = "greaterThan"; break;
        case V::Compare::Less: op = "lessThan"; break;
        case V::Compare::GreaterEqual: op = "greaterThanOrEqual"; break;
        case V::Compare::LessEqual: op = "lessThanOrEqual"; break;
        }
        if (op) e += attr("operator", op);
        if (v.allowBlank) e += attr("allowBlank", "1");
        if (!v.dropDown) e += attr("showDropDown", "1");
        if (v.showInput) e += attr("showInputMessage", "1");
        if (v.showError) e += attr("showErrorMessage", "1");
        if (!v.errorTitle.isEmpty()) e += textAttr("errorTitle", v.errorTitle);
        if (!v.error.isEmpty()) e += textAttr("error", v.error);
        if (!v.promptTitle.isEmpty()) e += textAttr("promptTitle", v.promptTitle);
        if (!v.prompt.isEmpty()) e += textAttr("prompt", v.prompt);
        e += attr("sqref", where) + ">";
        if (!v.formula1.isEmpty()) e += "<formula1>" + escapeText(v.formula1) + "</formula1>";
        if (!v.formula2.isEmpty()) e += "<formula2>" + escapeText(v.formula2) + "</formula2>";
        items += e + "</dataValidation>";
        ++count;
    }
    if (count == 0) return QByteArray();
    return "<dataValidations" + attr("count", num(count)) + ">" + items + "</dataValidations>";
}

QByteArray newSheetSkeleton(const Sheet &sheet)
{
    const SheetView &v = sheet.view;
    QByteArray o = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    o += "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    const QByteArray tab = colorXml("tabColor", sheet.tabColor);
    QByteArray transition;
    if (sheet.transitionEvaluation) transition += attr("transitionEvaluation", "1");
    if (sheet.transitionEntry) transition += attr("transitionEntry", "1");
    if (!tab.isEmpty() || !transition.isEmpty()) o += "<sheetPr" + transition + ">" + tab + "</sheetPr>";
    QByteArray view = "<sheetView";
    if (!v.showGridLines) view += attr("showGridLines", "0");
    if (!v.showHeadings) view += attr("showRowColHeaders", "0");
    if (!v.showZeros) view += attr("showZeros", "0");
    if (v.rightToLeft) view += attr("rightToLeft", "1");
    if (v.zoom != 100 && v.zoom > 0) view += attr("zoomScale", num(v.zoom)) + attr("zoomScaleNormal", num(v.zoom));
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
    view += attr("activeCell", "A1") + attr("sqref", "A1") + "/></sheetView>";
    o += "<sheetViews>" + view + "</sheetViews>";
    const SheetFormat &f = sheet.format;
    QByteArray fmt = "<sheetFormatPr" + attr("baseColWidth", num(f.baseColWidth));
    if (f.defaultColWidth > 0) fmt += attr("defaultColWidth", real(f.defaultColWidth));
    fmt += attr("defaultRowHeight", real(f.defaultRowHeight));
    if (f.customHeight) fmt += attr("customHeight", "1");
    if (f.zeroHeight) fmt += attr("zeroHeight", "1");
    if (f.outlineLevelRow > 0) fmt += attr("outlineLevelRow", num(f.outlineLevelRow));
    if (f.outlineLevelCol > 0) fmt += attr("outlineLevelCol", num(f.outlineLevelCol));
    o += fmt + "/><sheetData/>";
    QByteArray extensions;
    o += conditionalFormatsXml(sheet.conditionalFormats, &extensions);
    o += validationsXml(sheet.validations);
    o += "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>";
    o += extensions;
    o += "</worksheet>";
    return o;
}

} // namespace jp::office::sheet
