#include "sheet/sheetreader.h"
#include "sheet/xmlscan.h"
#include "sheet/builtinstyles.h"

#include "calc/numfmt.h"
#include "sheet/rowskip.h"

#include "office/theme.h"
#include "office/xml.h"
#include "io/zipindex.h"
#include "office/xmlstream.h"
#include "io/cfb.h"
#include "sheet/csv.h"
#include "sheet/ods.h"
#include "sheet/xls.h"
#include "sheet/xlsxbase.h"
#include "sheet/pagesetup.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QXmlStreamReader>
#include <algorithm>
#include <cstring>
#include <cmath>

namespace jp::office::sheet {

namespace {

constexpr qint64 kProgressEvery = 4096;
constexpr qint64 kReportEveryBytes = 65536;


// The text a file writes a character in the form _xHHHH_ as it stands for ("_x000D_" is a carriage return); an underscore before an "x" is written
// _x005F_, so that what comes after it is not read again.
void decodeEscapes(QString *s) { xmlscan::decodeEscapes(s); }

// A stream of XML tokens from a device, through the guard that keeps an element from crowding the parser with attributes; and what to say when the
// reader stops for a reason that isn't in the XML.
struct XmlSource {
    QIODevice *raw;
    XmlGuardDevice guard;
    QXmlStreamReader reader;
    // The most bytes of XML in one piece is four to a character of a string at the most (and at least what no tag or declaration comes to).
    XmlSource(QIODevice *in, const Limits &limits) : raw(in), guard(in, std::max<qint64>(4 * limits.stringChars, 65536)), reader(&guard) {}

    // Why the reading stopped short, in words; empty if it didn't.
    QString problem() const
    {
        if (guard.crowded()) return QCoreApplication::translate("Sheet", "Part of the file has more settings on one item than any real file has, so it can't be read.");
        if (guard.tooLong()) return QCoreApplication::translate("Sheet", "Part of the file is longer than any real file's, so it can't be read.");
        if (guard.notUtf8()) return QCoreApplication::translate("Sheet", "The file is not written in a form this program can read.");
        if (guard.doctype()) return QCoreApplication::translate("Sheet", "The file begins with instructions that this program does not allow, so it can't be read.");
        if (guard.malformed()) return QCoreApplication::translate("Sheet", "The file is damaged: it is not written as a spreadsheet file should be.");
        if (guard.sourceFailed()) {
            if (const auto *entry = dynamic_cast<const jp::ZipEntryDevice *>(raw)) {
                if (entry->failure() == jp::ZipFailure::TooLarge) return QCoreApplication::translate("Sheet", "The part is larger than any real file's part, so it is not read.");
                if (entry->failure() == jp::ZipFailure::OverBudget) return QCoreApplication::translate("Sheet", "The file's parts, read together, come to more than this program reads.");
            }
            return QCoreApplication::translate("Sheet", "The part is damaged: its data or its checksum is wrong.");
        }
        if (reader.hasError()) return QCoreApplication::translate("Sheet", "The file is damaged.");
        return {};
    }
};

bool isElement(const QXmlStreamReader &r, const char *name) { return r.name() == QLatin1String(name); }

QStringView attribute(const QXmlStreamReader &r, const char *name, bool *present = nullptr)
{
    const QXmlStreamAttributes attributes = r.attributes();
    for (const QXmlStreamAttribute &a : attributes)
        if (a.namespaceUri().isEmpty() && a.name() == QLatin1String(name)) {
            if (present) *present = true;
            return a.value();
        }
    if (present) *present = false;
    return {};
}

// The settings of a `sheetProtection` element. In the file an item set is one that is not allowed; all but the two selections are set unless the file says otherwise.
void readProtection(const QXmlStreamReader &r, SheetProtection *out)
{
    auto set = [&](const char *name, bool otherwise) {
        bool present = false;
        const QStringView v = attribute(r, name, &present);
        return present ? parseBool(v.toString()) : otherwise;
    };
    SheetProtection p;
    p.enabled = set("sheet", false);
    p.objects = set("objects", false);
    p.scenarios = set("scenarios", false);
    p.allowFormatCells = !set("formatCells", true);
    p.allowFormatColumns = !set("formatColumns", true);
    p.allowFormatRows = !set("formatRows", true);
    p.allowInsertColumns = !set("insertColumns", true);
    p.allowInsertRows = !set("insertRows", true);
    p.allowInsertLinks = !set("insertHyperlinks", true);
    p.allowDeleteColumns = !set("deleteColumns", true);
    p.allowDeleteRows = !set("deleteRows", true);
    p.allowSelectLocked = !set("selectLockedCells", false);
    p.allowSort = !set("sort", true);
    p.allowFilter = !set("autoFilter", true);
    p.allowPivot = !set("pivotTables", true);
    p.allowSelectUnlocked = !set("selectUnlockedCells", false);
    p.password.legacy = attribute(r, "password").toString().left(16);
    p.password.algorithm = attribute(r, "algorithmName").toString().left(32);
    p.password.hash = attribute(r, "hashValue").toString().left(1024);
    p.password.salt = attribute(r, "saltValue").toString().left(1024);
    bool ok = false;
    const qint64 spin = attribute(r, "spinCount").toString().toLongLong(&ok);
    p.password.spin = ok ? spin : 0;
    *out = p;
}

// A `hyperlink` element: the cells, the place in the workbook or the id of the relationship that names the address outside it, the shown text, and the screen tip. A link past the limits
// (more than kMaxLinks of them, too long a text, cells that are not a range) is left out.
void readLink(const QXmlStreamReader &r, QVector<LinkSpec> *links)
{
    if (links->size() >= kMaxLinks) return;
    LinkSpec link;
    if (!parseRangeRef(attribute(r, "ref"), &link.range)) return;
    link.location = attribute(r, "location").toString();
    link.display = attribute(r, "display").toString();
    link.tip = attribute(r, "tooltip").toString();
    for (const QXmlStreamAttribute &a : r.attributes())
        if (a.name() == QLatin1String("id") && !a.namespaceUri().isEmpty()) link.relId = a.value().toString();
    if (link.location.size() > kMaxLinkTarget || link.display.size() > kMaxLinkText || link.tip.size() > kMaxLinkText || link.relId.size() > 64) return;
    if (link.location.isEmpty() && link.relId.isEmpty()) return;
    links->push_back(std::move(link));
}

// The text of a number as a number, never through the C library's conversions and never by the locale.
bool toNumber(const QString &text, double *value)
{
    bool ok = false;
    const double v = QStringView(text).trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    *value = v;
    return true;
}

} // namespace

// ---- the shared strings

namespace {
bool flagOf(const QXmlStreamReader &r);
double boundedNumber(QStringView text, double low, double high, double fallback);
ColorSpec colorOf(const QXmlStreamReader &r);
} // namespace

bool readSharedStrings(QIODevice *in, const Limits &limits, SharedStrings *out, QString *error)
{
    *out = SharedStrings();
    XmlSource src(in, limits);
    QXmlStreamReader &r = src.reader;
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    int depth = 0, phonetic = 0;
    bool inItem = false, inText = false, rootSeen = false;
    QString text;
    // The runs of a rich string, when they are asked for: where each begins, and the font its properties name (the same font once among the strings).
    const bool wantRuns = limits.richText;
    RichRuns runs;
    bool inRun = false, inProps = false, haveProps = false, anyOwnFont = false;
    quint32 runStart = 0;
    qint64 runCount = 0;
    FontSpec runFont;
    QHash<QByteArray, quint32> fontIds;
    auto internFont = [&](const FontSpec &f) {
        QByteArray key = f.name.toUtf8() + '|' + QByteArray::number(f.size) + '|' + char('0' + f.bold) + char('0' + f.italic) + char('0' + f.strike) + char('0' + int(f.underline)) + char('0' + f.vertAlign)
            + '|' + QByteArray::number(int(f.color.kind)) + ',' + QByteArray::number(f.color.rgb) + ',' + QByteArray::number(f.color.index) + ',' + QByteArray::number(f.color.tint);
        auto it = fontIds.constFind(key);
        if (it == fontIds.cend()) {
            out->richFonts.append(f);
            it = fontIds.insert(key, quint32(out->richFonts.size() - 1));
        }
        return it.value();
    };
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            return fail(QCoreApplication::translate("Sheet", "The file begins with instructions that this program does not allow, so it can't be read."));
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) return fail(QCoreApplication::translate("Sheet", "The file has items inside items more deeply than any real file has."));
            if (depth == 1) {
                if (!isElement(r, "sst")) return fail(QCoreApplication::translate("Sheet", "The part is not a list of shared strings."));
                rootSeen = true;
            } else if (depth == 2 && isElement(r, "si")) {
                if (out->strings.size() >= limits.sharedStrings) return fail(QCoreApplication::translate("Sheet", "The file has more shared strings than any real file has."));
                inItem = true;
                text.clear();
                runs.clear();
                anyOwnFont = false;
            } else if (inItem && !phonetic) {
                // The text of a string is its <t> and the <t> of its runs; the phonetic runs (rPh) are not text.
                if (isElement(r, "rPh") || isElement(r, "phoneticPr")) phonetic = depth;
                else if (isElement(r, "t") && (depth == 3 || depth == 4)) inText = true;
                else if (wantRuns && depth == 3 && isElement(r, "r")) {
                    inRun = true;
                    haveProps = false;
                    runFont = FontSpec();
                    runStart = quint32(text.size());
                } else if (wantRuns && inRun && depth == 4 && isElement(r, "rPr")) {
                    inProps = true;
                    haveProps = true;
                } else if (wantRuns && inProps && depth == 5) {
                    if (isElement(r, "b")) runFont.bold = flagOf(r);
                    else if (isElement(r, "i")) runFont.italic = flagOf(r);
                    else if (isElement(r, "strike")) runFont.strike = flagOf(r);
                    else if (isElement(r, "sz")) runFont.size = boundedNumber(attribute(r, "val"), 1, 409.5, 11);
                    else if (isElement(r, "rFont")) runFont.name = attribute(r, "val").left(kMaxFontNameChars).toString();
                    else if (isElement(r, "color")) runFont.color = colorOf(r);
                    else if (isElement(r, "u")) {
                        const QStringView v = attribute(r, "val");
                        runFont.underline = v == QLatin1String("none") ? Underline::None : v == QLatin1String("double") ? Underline::Double
                                            : v == QLatin1String("singleAccounting") ? Underline::SingleAccounting : v == QLatin1String("doubleAccounting") ? Underline::DoubleAccounting : Underline::Single;
                    } else if (isElement(r, "vertAlign")) {
                        const QStringView v = attribute(r, "val");
                        runFont.vertAlign = v == QLatin1String("superscript") ? 1 : v == QLatin1String("subscript") ? 2 : 0;
                    }
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (phonetic == depth) phonetic = 0;
            if (inText && isElement(r, "t")) inText = false;
            if (wantRuns && inProps && depth == 4) inProps = false;
            if (wantRuns && inRun && depth == 3 && !phonetic) {
                inRun = false;
                if (haveProps) anyOwnFont = true;
                if (runCount < limits.richRuns) {
                    RichRun run;
                    run.start = runStart;
                    run.font = haveProps ? internFont(runFont) : RichRun::kCellFont;
                    runs.append(run);
                    ++runCount;
                }
            }
            if (depth == 2 && inItem) {
                inItem = false;
                // (the runs were counted in the text as written; where it has escapes, they are counted again in the text they stand for)
                if (wantRuns && !runs.isEmpty() && text.contains(QLatin1String("_x"))) {
                    for (RichRun &run : runs) {
                        QString before = text.left(int(run.start));
                        decodeEscapes(&before);
                        run.start = quint32(before.size());
                    }
                }
                decodeEscapes(&text);
                out->chars += text.size();
                if (out->chars > limits.sharedStringChars) return fail(QCoreApplication::translate("Sheet", "The shared strings have more characters than any real file has."));
                if (wantRuns && anyOwnFont && !runs.isEmpty()) out->rich.insert(qint32(out->strings.size()), runs);
                out->strings.append(text);
                text.clear();
            }
            --depth;
            break;
        case QXmlStreamReader::Characters:
            if (inText && !phonetic) {
                text += r.text();
                if (text.size() > limits.stringChars) return fail(QCoreApplication::translate("Sheet", "A string is longer than any real file has."));
            }
            break;
        default:
            break;
        }
    }
    const QString why = src.problem();
    if (!why.isEmpty()) return fail(why);
    if (!rootSeen) return fail(QCoreApplication::translate("Sheet", "The part is not a list of shared strings."));
    return true;
}

// ---- the styles part

namespace {

// An element that holds a yes or no in its `val` (`<b/>` is yes).
bool flagOf(const QXmlStreamReader &r)
{
    bool present = false;
    const QStringView v = attribute(r, "val", &present);
    return !present || parseBool(v.toString(), true);
}

// A size in the file's units: a number that is finite, brought into [low, high]; the fallback for text that is no number.
double boundedNumber(QStringView text, double low, double high, double fallback)
{
    bool ok = false;
    const double v = text.trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(v)) return fallback;
    return std::min(high, std::max(low, v));
}

// The zoom a sheet is left at, in percent: the most the program zooms to either way.
double boundedZoom(QStringView text) { return boundedNumber(text, 10, 400, 100); }

ColorSpec colorOf(const QXmlStreamReader &r)
{
    ColorSpec c;
    const QXmlStreamAttributes attributes = r.attributes();
    for (const QXmlStreamAttribute &a : attributes) {
        if (!a.namespaceUri().isEmpty()) continue;
        const QStringView name = a.name();
        bool ok = false;
        if (name == QLatin1String("rgb")) {
            const quint32 v = a.value().trimmed().toUInt(&ok, 16);
            if (ok) {
                c.kind = ColorSpec::Kind::Rgb;
                c.rgb = v & 0xFFFFFF;
            }
        } else if (name == QLatin1String("theme")) {
            const int v = a.value().toInt(&ok);
            if (ok && v >= 0) {
                c.kind = ColorSpec::Kind::Theme;
                c.index = v;
            }
        } else if (name == QLatin1String("indexed")) {
            const int v = a.value().toInt(&ok);
            if (ok && v >= 0) {
                c.kind = ColorSpec::Kind::Indexed;
                c.index = v;
            }
        } else if (name == QLatin1String("auto")) {
            if (parseBool(a.value().toString())) c.kind = ColorSpec::Kind::Auto;
        } else if (name == QLatin1String("tint")) {
            c.tint = boundedNumber(a.value(), -1, 1, 0);
        }
    }
    if (c.kind != ColorSpec::Kind::Theme) c.tint = 0;
    return c;
}

BorderStyle borderStyleOf(QStringView name)
{
    static const struct {
        const char *name;
        BorderStyle style;
    } names[] = {{"thin", BorderStyle::Thin}, {"medium", BorderStyle::Medium}, {"dashed", BorderStyle::Dashed}, {"dotted", BorderStyle::Dotted}, {"thick", BorderStyle::Thick},
                 {"double", BorderStyle::Double}, {"hair", BorderStyle::Hair}, {"mediumDashed", BorderStyle::MediumDashed}, {"dashDot", BorderStyle::DashDot},
                 {"mediumDashDot", BorderStyle::MediumDashDot}, {"dashDotDot", BorderStyle::DashDotDot}, {"mediumDashDotDot", BorderStyle::MediumDashDotDot},
                 {"slantDashDot", BorderStyle::SlantDashDot}};
    for (const auto &n : names)
        if (name == QLatin1String(n.name)) return n.style;
    return BorderStyle::None;
}

PatternKind patternOf(QStringView name)
{
    static const struct {
        const char *name;
        PatternKind pattern;
    } names[] = {{"solid", PatternKind::Solid}, {"mediumGray", PatternKind::MediumGray}, {"darkGray", PatternKind::DarkGray}, {"lightGray", PatternKind::LightGray},
                 {"darkHorizontal", PatternKind::DarkHorizontal}, {"darkVertical", PatternKind::DarkVertical}, {"darkDown", PatternKind::DarkDown}, {"darkUp", PatternKind::DarkUp},
                 {"darkGrid", PatternKind::DarkGrid}, {"darkTrellis", PatternKind::DarkTrellis}, {"lightHorizontal", PatternKind::LightHorizontal},
                 {"lightVertical", PatternKind::LightVertical}, {"lightDown", PatternKind::LightDown}, {"lightUp", PatternKind::LightUp}, {"lightGrid", PatternKind::LightGrid},
                 {"lightTrellis", PatternKind::LightTrellis}, {"gray125", PatternKind::Gray125}, {"gray0625", PatternKind::Gray0625}};
    for (const auto &n : names)
        if (name == QLatin1String(n.name)) return n.pattern;
    return PatternKind::None;
}

HAlign hAlignOf(QStringView name)
{
    static const struct {
        const char *name;
        HAlign align;
    } names[] = {{"left", HAlign::Left}, {"center", HAlign::Center}, {"right", HAlign::Right}, {"fill", HAlign::Fill}, {"justify", HAlign::Justify},
                 {"centerContinuous", HAlign::CenterContinuous}, {"distributed", HAlign::Distributed}};
    for (const auto &n : names)
        if (name == QLatin1String(n.name)) return n.align;
    return HAlign::General;
}

VAlign vAlignOf(QStringView name)
{
    if (name == QLatin1String("top")) return VAlign::Top;
    if (name == QLatin1String("center")) return VAlign::Center;
    if (name == QLatin1String("justify")) return VAlign::Justify;
    if (name == QLatin1String("distributed")) return VAlign::Distributed;
    return VAlign::Bottom;
}

} // namespace

bool readStyles(QIODevice *in, const Limits &limits, Styles *out, QString *error)
{
    *out = Styles();
    XmlSource src(in, limits);
    QXmlStreamReader &r = src.reader;
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    enum class Where { Other, NumFmts, CellXfs, StyleXfs, CellStyles, Fonts, Fills, Borders, Colors, Dxfs, TableStyles };
    enum class DxfPart { None, Font, Fill, Border } dxfPart = DxfPart::None;
    BorderSide *dxfSide = nullptr;
    Where where = Where::Other;
    int depth = 0;
    qint64 numFmtElements = 0;
    bool rootSeen = false;
    // the font, fill, or border being read (their elements have children), and which side of the border and which stop of a gradient
    bool inGradient = false, gradientColorTaken = false, inIndexed = false;
    BorderSide *side = nullptr;
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            return fail(QCoreApplication::translate("Sheet", "The file begins with instructions that this program does not allow, so it can't be read."));
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) return fail(QCoreApplication::translate("Sheet", "The file has items inside items more deeply than any real file has."));
            if (depth == 1) {
                if (!isElement(r, "styleSheet")) return fail(QCoreApplication::translate("Sheet", "The part is not a list of styles."));
                rootSeen = true;
            } else if (depth == 2) {
                where = isElement(r, "numFmts")   ? Where::NumFmts
                        : isElement(r, "cellXfs") ? Where::CellXfs
                        : isElement(r, "cellStyleXfs") ? Where::StyleXfs
                        : isElement(r, "cellStyles") ? Where::CellStyles
                        : isElement(r, "fonts")   ? Where::Fonts
                        : isElement(r, "fills")   ? Where::Fills
                        : isElement(r, "borders") ? Where::Borders
                        : isElement(r, "colors")  ? Where::Colors
                        : isElement(r, "dxfs")    ? Where::Dxfs
                        : isElement(r, "tableStyles") ? Where::TableStyles
                                                  : Where::Other;
            } else if (depth == 3 && where == Where::NumFmts && isElement(r, "numFmt")) {
                if (++numFmtElements > limits.numFmts) return fail(QCoreApplication::translate("Sheet", "The file has more number formats than any real file has."));
                bool ok = false;
                const int id = attribute(r, "numFmtId").toInt(&ok);
                const QString code = attribute(r, "formatCode").toString();
                out->chars += code.size();
                if (out->chars > limits.numFmtChars) return fail(QCoreApplication::translate("Sheet", "The number formats have more characters than any real file has."));
                if (ok) out->numFmts.insert(id, code);
            } else if (depth == 3 && (where == Where::CellXfs || where == Where::StyleXfs) && isElement(r, "xf")) {
                if (where == Where::CellXfs && out->cellXfs.size() >= limits.cellXfs) return fail(QCoreApplication::translate("Sheet", "The file has more cell formats than any real file has."));
                if (where == Where::StyleXfs && out->styleXfs.size() >= 100000) return fail(QCoreApplication::translate("Sheet", "The file has more cell styles than any real file has."));
                bool ok = false;
                CellFormat f;
                const int id = attribute(r, "numFmtId").toInt(&ok);
                f.numFmtId = ok ? id : 0;
                f.font = attribute(r, "fontId").toUInt(&ok);
                if (!ok) f.font = 0;
                f.fill = attribute(r, "fillId").toUInt(&ok);
                if (!ok) f.fill = 0;
                f.border = attribute(r, "borderId").toUInt(&ok);
                if (!ok) f.border = 0;
                f.xfId = attribute(r, "xfId").toUInt(&ok);
                if (!ok) f.xfId = 0;
                f.quotePrefix = parseBool(attribute(r, "quotePrefix").toString());
                if (where == Where::CellXfs) {
                    out->cellXfs.append(f.numFmtId);
                    out->formats.append(f);
                } else {
                    out->styleXfs.append(f);
                }
            } else if (depth == 4 && (where == Where::CellXfs || where == Where::StyleXfs) && isElement(r, "alignment") && !(where == Where::CellXfs ? out->formats.isEmpty() : out->styleXfs.isEmpty())) {
                CellFormat &f = where == Where::CellXfs ? out->formats.last() : out->styleXfs.last();
                f.horizontal = hAlignOf(attribute(r, "horizontal"));
                f.vertical = vAlignOf(attribute(r, "vertical"));
                f.wrap = parseBool(attribute(r, "wrapText").toString());
                f.shrink = parseBool(attribute(r, "shrinkToFit").toString());
                f.indent = quint8(std::lround(boundedNumber(attribute(r, "indent"), 0, 255, 0)));
                const double turn = boundedNumber(attribute(r, "textRotation"), 0, 255, 0);
                f.rotation = quint8(turn == 255 ? 255 : turn <= 180 ? std::lround(turn) : 0);
                f.readingOrder = quint8(std::lround(boundedNumber(attribute(r, "readingOrder"), 0, 2, 0)));
            } else if (depth == 4 && (where == Where::CellXfs || where == Where::StyleXfs) && isElement(r, "protection") && !(where == Where::CellXfs ? out->formats.isEmpty() : out->styleXfs.isEmpty())) {
                CellFormat &f = where == Where::CellXfs ? out->formats.last() : out->styleXfs.last();
                const QStringView locked = attribute(r, "locked");
                f.locked = locked.isEmpty() ? true : parseBool(locked.toString());
                f.formulaHidden = parseBool(attribute(r, "hidden").toString());
            } else if (depth == 3 && where == Where::CellStyles && isElement(r, "cellStyle")) {
                if (out->cellStyles.size() < 100000) {
                    CellStyleName n;
                    n.name = attribute(r, "name").left(255).toString();
                    bool ok = false;
                    const int builtin = attribute(r, "builtinId").toInt(&ok);
                    n.builtin = ok ? builtin : -1;
                    n.xfId = attribute(r, "xfId").toUInt(&ok);
                    if (!ok) n.xfId = 0;
                    n.hidden = parseBool(attribute(r, "hidden").toString());
                    out->cellStyles.append(n);
                }
            } else if (depth == 3 && where == Where::Fonts && isElement(r, "font")) {
                if (out->fonts.size() >= limits.fonts) return fail(QCoreApplication::translate("Sheet", "The file has more fonts than any real file has."));
                out->fonts.append(FontSpec());
            } else if (depth == 4 && where == Where::Fonts && !out->fonts.isEmpty()) {
                FontSpec &f = out->fonts.last();
                if (isElement(r, "b")) f.bold = flagOf(r);
                else if (isElement(r, "i")) f.italic = flagOf(r);
                else if (isElement(r, "strike")) f.strike = flagOf(r);
                else if (isElement(r, "sz")) f.size = boundedNumber(attribute(r, "val"), 1, 409.5, 11);
                else if (isElement(r, "name")) f.name = attribute(r, "val").left(kMaxFontNameChars).toString();
                else if (isElement(r, "family")) f.family = qint32(std::lround(boundedNumber(attribute(r, "val"), -1, 14, -1)));
                else if (isElement(r, "charset")) f.charset = qint32(std::lround(boundedNumber(attribute(r, "val"), -1, 255, -1)));
                else if (isElement(r, "scheme")) f.scheme = attribute(r, "val").left(16).toString();
                else if (isElement(r, "color")) f.color = colorOf(r);
                else if (isElement(r, "u")) {
                    const QStringView v = attribute(r, "val");
                    f.underline = v == QLatin1String("none") ? Underline::None : v == QLatin1String("double") ? Underline::Double
                                  : v == QLatin1String("singleAccounting") ? Underline::SingleAccounting : v == QLatin1String("doubleAccounting") ? Underline::DoubleAccounting : Underline::Single;
                } else if (isElement(r, "vertAlign")) {
                    const QStringView v = attribute(r, "val");
                    f.vertAlign = v == QLatin1String("superscript") ? 1 : v == QLatin1String("subscript") ? 2 : 0;
                }
            } else if (depth == 3 && where == Where::Fills && isElement(r, "fill")) {
                if (out->fills.size() >= limits.fills) return fail(QCoreApplication::translate("Sheet", "The file has more fills than any real file has."));
                out->fills.append(FillSpec());
                inGradient = false;
            } else if (depth == 4 && where == Where::Fills && !out->fills.isEmpty()) {
                FillSpec &f = out->fills.last();
                if (isElement(r, "patternFill")) f.pattern = patternOf(attribute(r, "patternType"));
                else if (isElement(r, "gradientFill")) {
                    f.pattern = PatternKind::Solid;
                    f.gradient = true;
                    inGradient = true;
                    gradientColorTaken = false;
                }
            } else if (depth == 5 && where == Where::Fills && !out->fills.isEmpty() && !inGradient) {
                FillSpec &f = out->fills.last();
                if (isElement(r, "fgColor")) f.fg = colorOf(r);
                else if (isElement(r, "bgColor")) f.bg = colorOf(r);
            } else if (depth == 6 && where == Where::Fills && !out->fills.isEmpty() && inGradient && !gradientColorTaken && isElement(r, "color")) {
                out->fills.last().fg = colorOf(r);
                gradientColorTaken = true;
            } else if (depth == 3 && where == Where::Borders && isElement(r, "border")) {
                if (out->borders.size() >= limits.borders) return fail(QCoreApplication::translate("Sheet", "The file has more borders than any real file has."));
                BorderSpec b;
                b.diagonalUp = parseBool(attribute(r, "diagonalUp").toString());
                b.diagonalDown = parseBool(attribute(r, "diagonalDown").toString());
                out->borders.append(b);
                side = nullptr;
            } else if (depth == 4 && where == Where::Borders && !out->borders.isEmpty()) {
                BorderSpec &b = out->borders.last();
                side = isElement(r, "left") || isElement(r, "start") ? &b.left : isElement(r, "right") || isElement(r, "end") ? &b.right : isElement(r, "top") ? &b.top
                       : isElement(r, "bottom") ? &b.bottom : isElement(r, "diagonal") ? &b.diagonal : nullptr;
                if (side) side->style = borderStyleOf(attribute(r, "style"));
            } else if (depth == 5 && where == Where::Borders && side && isElement(r, "color")) {
                side->color = colorOf(r);
            } else if (depth == 3 && where == Where::TableStyles && isElement(r, "tableStyle")) {
                if (out->tableStyles.size() < 1000) {
                    CustomTableStyle st;
                    st.name = attribute(r, "name").toString();
                    out->tableStyles.append(st);
                }
            } else if (depth == 4 && where == Where::TableStyles && !out->tableStyles.isEmpty() && isElement(r, "tableStyleElement")) {
                CustomTableStyle &st = out->tableStyles.last();
                static const struct { const char *name; TablePart part; } kinds[] = {
                    {"wholeTable", TablePart::WholeTable}, {"headerRow", TablePart::HeaderRow}, {"totalRow", TablePart::TotalRow}, {"firstColumn", TablePart::FirstColumn},
                    {"lastColumn", TablePart::LastColumn}, {"firstRowStripe", TablePart::FirstRowStripe}, {"secondRowStripe", TablePart::SecondRowStripe},
                    {"firstColumnStripe", TablePart::FirstColumnStripe}, {"secondColumnStripe", TablePart::SecondColumnStripe}, {"firstHeaderCell", TablePart::FirstHeaderCell},
                    {"lastHeaderCell", TablePart::LastHeaderCell}, {"firstTotalCell", TablePart::FirstTotalCell}, {"lastTotalCell", TablePart::LastTotalCell}};
                const QStringView type = attribute(r, "type");
                for (const auto &k : kinds)
                    if (type == QLatin1String(k.name) && st.parts.size() < 64) {
                        CustomTableStyle::Part part;
                        part.part = k.part;
                        bool good = false;
                        const int dxf = attribute(r, "dxfId").toInt(&good);
                        part.dxf = good ? dxf : -1;
                        part.size = std::clamp(attribute(r, "size").toInt(), 1, 9);
                        if (attribute(r, "size").isEmpty()) part.size = 1;
                        st.parts.append(part);
                    }
            } else if (depth == 3 && where == Where::Dxfs && isElement(r, "dxf")) {
                if (out->dxfs.size() < 100000) out->dxfs.append(DifferentialFormat());
                dxfPart = DxfPart::None;
                dxfSide = nullptr;
            } else if (where == Where::Dxfs && !out->dxfs.isEmpty() && depth >= 4) {
                DifferentialFormat &d = out->dxfs.last();
                if (depth == 4) {
                    dxfPart = isElement(r, "font") ? DxfPart::Font : isElement(r, "fill") ? DxfPart::Fill : isElement(r, "border") ? DxfPart::Border : DxfPart::None;
                    dxfSide = nullptr;
                    if (isElement(r, "numFmt")) {
                        const QString code = attribute(r, "formatCode").toString();
                        if (!code.isEmpty()) d.numberFormat = code;
                        else {
                            bool ok = false;
                            const int id = attribute(r, "numFmtId").toInt(&ok);
                            if (ok) d.numberFormat = out->numFmts.value(id, jp::calc::builtinFormatCode(id));
                        }
                    } else if (dxfPart == DxfPart::Border) {
                        d.hasBorder = true;
                        d.border.diagonalUp = parseBool(attribute(r, "diagonalUp").toString());
                        d.border.diagonalDown = parseBool(attribute(r, "diagonalDown").toString());
                    } else if (dxfPart == DxfPart::Fill) {
                        d.hasFill = true;
                    }
                } else if (dxfPart == DxfPart::Font && depth == 5) {
                    if (isElement(r, "b")) d.bold = flagOf(r);
                    else if (isElement(r, "i")) d.italic = flagOf(r);
                    else if (isElement(r, "strike")) d.strike = flagOf(r);
                    else if (isElement(r, "color")) d.fontColor = colorOf(r);
                    else if (isElement(r, "u")) {
                        const QStringView v = attribute(r, "val");
                        d.underline = v == QLatin1String("none") ? Underline::None : v == QLatin1String("double") ? Underline::Double
                                      : v == QLatin1String("singleAccounting") ? Underline::SingleAccounting : v == QLatin1String("doubleAccounting") ? Underline::DoubleAccounting : Underline::Single;
                    }
                } else if (dxfPart == DxfPart::Fill && depth == 5 && isElement(r, "patternFill")) {
                    d.fill.pattern = patternOf(attribute(r, "patternType"));
                    if (attribute(r, "patternType").isEmpty()) d.fill.pattern = PatternKind::Solid;   // (a differential fill with no type is a solid one)
                } else if (dxfPart == DxfPart::Fill && depth == 6) {
                    if (isElement(r, "fgColor")) d.fill.fg = colorOf(r);
                    else if (isElement(r, "bgColor")) d.fill.bg = colorOf(r);
                } else if (dxfPart == DxfPart::Border && depth == 5) {
                    BorderSpec &b = d.border;
                    dxfSide = isElement(r, "left") || isElement(r, "start") ? &b.left : isElement(r, "right") || isElement(r, "end") ? &b.right : isElement(r, "top") ? &b.top
                              : isElement(r, "bottom") ? &b.bottom : isElement(r, "diagonal") ? &b.diagonal : nullptr;
                    if (dxfSide) dxfSide->style = borderStyleOf(attribute(r, "style"));
                } else if (dxfPart == DxfPart::Border && depth == 6 && dxfSide && isElement(r, "color")) {
                    dxfSide->color = colorOf(r);
                }
            } else if (depth == 3 && where == Where::Colors) {
                inIndexed = isElement(r, "indexedColors");
            } else if (depth == 4 && where == Where::Colors && inIndexed && isElement(r, "rgbColor")) {
                if (out->indexed.size() < 256) {
                    bool ok = false;
                    const quint32 v = attribute(r, "rgb").trimmed().toUInt(&ok, 16);
                    out->indexed.append(ok ? (v & 0xFFFFFF) : 0);
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (depth == 2) where = Where::Other;
            if (depth == 4 && where == Where::Fills) inGradient = false;
            if (depth == 4 && where == Where::Borders) side = nullptr;
            if (depth == 3) inIndexed = false;
            --depth;
            break;
        default:
            break;
        }
    }
    const QString why = src.problem();
    if (!why.isEmpty()) return fail(why);
    if (!rootSeen) return fail(QCoreApplication::translate("Sheet", "The part is not a list of styles."));
    return true;
}

// ---- the workbook part

bool readWorkbookPart(QIODevice *in, const Limits &limits, WorkbookInfo *out, QString *error)
{
    *out = WorkbookInfo();
    XmlSource src(in, limits);
    QXmlStreamReader &r = src.reader;
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    int depth = 0;
    bool inSheets = false, inNames = false, inName = false, rootSeen = false, inViews = false, viewSeen = false;
    DefinedName name;
    qint64 nameChars = 0;
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            return fail(QCoreApplication::translate("Sheet", "The file begins with instructions that this program does not allow, so it can't be read."));
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) return fail(QCoreApplication::translate("Sheet", "The file has items inside items more deeply than any real file has."));
            if (depth == 1) {
                if (!isElement(r, "workbook")) return fail(QCoreApplication::translate("Sheet", "The part is not a workbook."));
                rootSeen = true;
            } else if (depth == 2 && isElement(r, "workbookPr")) {
                out->dateSystem = parseBool(attribute(r, "date1904").toString()) ? DateSystem::Date1904 : DateSystem::Date1900;
            } else if (depth == 2 && isElement(r, "calcPr")) {
                const QStringView mode = attribute(r, "calcMode");
                out->calcMode = mode == QLatin1String("manual") ? CalcMode::Manual : mode == QLatin1String("autoNoTable") ? CalcMode::AutoNoTable : CalcMode::Auto;
                out->fullCalcOnLoad = parseBool(attribute(r, "fullCalcOnLoad").toString());
                out->calcUnfinished = !attribute(r, "calcCompleted").isEmpty() && !parseBool(attribute(r, "calcCompleted").toString());
                out->iterate = parseBool(attribute(r, "iterate").toString());
                bool okCount = false, okDelta = false;
                const int count = attribute(r, "iterateCount").toInt(&okCount);
                const double delta = attribute(r, "iterateDelta").toDouble(&okDelta);
                if (okCount) out->iterateCount = std::clamp(count, 1, 32767);
                if (okDelta && delta >= 0 && delta < 1e300) out->iterateDelta = delta;
            } else if (depth == 2 && isElement(r, "workbookProtection")) {
                WorkbookProtection &wp = out->protection;
                wp.lockStructure = parseBool(attribute(r, "lockStructure").toString());
                wp.lockWindows = parseBool(attribute(r, "lockWindows").toString());
                wp.password.legacy = attribute(r, "workbookPassword").toString().left(16);
                wp.password.algorithm = attribute(r, "workbookAlgorithmName").toString().left(32);
                wp.password.hash = attribute(r, "workbookHashValue").toString().left(1024);
                wp.password.salt = attribute(r, "workbookSaltValue").toString().left(1024);
                bool spinOk = false;
                const qint64 spin = attribute(r, "workbookSpinCount").toString().toLongLong(&spinOk);
                wp.password.spin = spinOk ? spin : 0;
            } else if (depth == 2 && isElement(r, "bookViews")) {
                inViews = true;
            } else if (depth == 3 && inViews && isElement(r, "workbookView") && !viewSeen) {
                viewSeen = true;
                bool ok = false;
                const int active = attribute(r, "activeTab").toInt(&ok);
                out->activeTab = ok && active >= 0 ? active : 0;
                const int first = attribute(r, "firstSheet").toInt(&ok);
                out->firstSheet = ok && first >= 0 ? first : 0;
            } else if (depth == 2 && isElement(r, "definedNames")) {
                inNames = true;
            } else if (depth == 3 && inNames && isElement(r, "definedName")) {
                if (out->definedNames.size() >= limits.definedNames) return fail(QCoreApplication::translate("Sheet", "The workbook has more names than any real file has."));
                name = DefinedName();
                name.name = attribute(r, "name").toString();
                bool ok = false;
                const int local = attribute(r, "localSheetId").toInt(&ok);
                name.localSheet = ok ? local : -1;
                name.hidden = parseBool(attribute(r, "hidden").toString());
                name.comment = attribute(r, "comment").toString();
                nameChars += name.name.size() + name.comment.size();
                if (nameChars > limits.definedNameChars) return fail(QCoreApplication::translate("Sheet", "The names have more characters than any real file has."));
                inName = true;
            } else if (depth == 2 && isElement(r, "sheets")) {
                inSheets = true;
            } else if (depth == 3 && inSheets && isElement(r, "sheet")) {
                if (out->sheets.size() >= limits.sheets) return fail(QCoreApplication::translate("Sheet", "The workbook has more sheets than any real file has."));
                SheetEntry e;
                e.name = attribute(r, "name").toString();
                if (e.name.size() > kMaxSheetNameChars) return fail(QCoreApplication::translate("Sheet", "A sheet's name is longer than any real file has."));
                e.sheetId = int(attribute(r, "sheetId").toInt());
                const QStringView state = attribute(r, "state");
                e.state = state == QLatin1String("hidden") ? SheetState::Hidden : state == QLatin1String("veryHidden") ? SheetState::VeryHidden : SheetState::Visible;
                const QXmlStreamAttributes attributes = r.attributes();
                for (const QXmlStreamAttribute &a : attributes)
                    if (a.name() == QLatin1String("id") && !a.namespaceUri().isEmpty() && sameNamespace(a.namespaceUri().toString(), ns::R)) e.relId = a.value().toString();
                out->sheets.append(e);
            }
            break;
        case QXmlStreamReader::Characters:
            if (inName) {
                nameChars += r.text().size();
                if (nameChars > limits.definedNameChars) return fail(QCoreApplication::translate("Sheet", "The names have more characters than any real file has."));
                name.ref += r.text();
            }
            break;
        case QXmlStreamReader::EndElement:
            if (depth == 3 && inName) {
                out->definedNames.append(name);
                inName = false;
            }
            if (depth == 2) {
                inSheets = false;
                inNames = false;
                inViews = false;
            }
            --depth;
            break;
        default:
            break;
        }
    }
    const QString why = src.problem();
    if (!why.isEmpty()) return fail(why);
    if (!rootSeen) return fail(QCoreApplication::translate("Sheet", "The part is not a workbook."));
    return true;
}

// ---- a worksheet

namespace {

// What a worksheet reader holds while it reads: the sheet, and the cell in the making.
class SheetReader {
public:
    SheetReader(const ReadOptions &options, Sheet *sheet) : m_options(options), m_limits(options.limits), m_sheet(*sheet) {}
    bool read(QIODevice *in);

private:
    struct Master {
        qint32 text;
        quint32 row, col;
    };
    struct Pending {
        qint32 formula;
        quint32 row, col;
    };

    bool fail(const QString &why)
    {
        m_sheet.error = why;
        return false;
    }
    // Counts `n` more bytes the sheet takes; false, with the sheet's error saying so, once it takes more than the limit.
    bool take(qint64 n)
    {
        m_bytes += n;
        return m_bytes <= m_limits.sheetBytes || fail(QCoreApplication::translate("Sheet", "The sheet would take more memory to hold than any real file's does."));
    }
    // What things cost to hold, as measured (the practice workbooks' largest sheet, and sheets made to be the worst case, read by jcrunchtool: the bytes the
    // process held at its peak against what was counted), rounded up. A cell is 24 bytes in a block that grows by doubling; a string is a slot of 24 in a list
    // that grows by doubling, and a block of the allocator's (its own 24-byte header, the characters twice over and a terminator, and 8 for the allocator,
    // to a multiple of 16, 32 at the least); a shared formula waiting for its first cell is an entry in a table of them (a key, a list, and the list's block),
    // and each cell waiting in it 16 bytes.
    static constexpr qint64 kCellBytes = 32, kFormulaBytes = 24, kRangeBytes = 24, kMasterBytes = 48, kWaitingBytes = 128, kWaitingCellBytes = 16, kSlotBytes = 32;
    // a run of columns, a row kept as not the default, and a merged range (each rounded up from what the structure holds)
    static constexpr qint64 kColumnSpanBytes = 48, kRowSpecBytes = 48, kMergeBytes = 32;
    static constexpr qsizetype kMaxValidations = 20000;
    static constexpr qsizetype kMaxCondRules = 50000;
    static constexpr qint64 kMaxFilterValues = 200000;
    static qint64 stringBytes(qint64 chars) { return kSlotBytes + std::max<qint64>(32, (24 + 2 * chars + 2 + 8 + 15) / 16 * 16); }
    bool startRow(const QXmlStreamReader &r);
    bool startColumns(const QXmlStreamReader &r);
    void startView(const QXmlStreamReader &r);
    void startPane(const QXmlStreamReader &r);
    void startSelection(const QXmlStreamReader &r);
    void endView();
    bool startMerge(const QXmlStreamReader &r);
    Validation readRule(const QXmlStreamReader &r) const;
    CondRule readCondRule(const QXmlStreamReader &r) const;
    bool startCell(const QXmlStreamReader &r);
    bool startFormula(const QXmlStreamReader &r);
    bool endFormula();
    bool endCell();
    void finish();

    const ReadOptions &m_options;
    const Limits &m_limits;
    Sheet &m_sheet;

    // the view in the making
    struct Selection {
        QString pane;
        quint32 row = 0, col = 0;
    };
    QVector<Selection> m_selections;
    QString m_activePane;
    bool m_viewSeen = false;
    bool m_stop = false;             // a row past the one the read was asked to stop at has begun
    bool m_skipCells = false;        // the current row is one whose cells are not kept (ReadOptions::screenRows)
    qint64 m_skipped = 0;            // cells that went by so
    // the place
    quint32 m_row = 0;               // the current row
    bool m_haveRow = false;
    quint32 m_nextCol = 0;           // where a cell with no address goes
    // the cell in the making
    Cell m_cell;
    quint32 m_cellRow = 0, m_cellCol = 0;   // (where m_cell is)
    QString m_type;
    bool m_haveValue = false, m_haveInline = false;
    QString m_value, m_inline, m_formula;
    // the formula in the making
    enum Kind { Normal, Array, Shared, Table } m_kind = Normal;
    qint64 m_si = -1;
    CellRange m_ref;
    bool m_haveRef = false;
    QString m_r1, m_r2;
    bool m_dtr = false;   // a table of one input: its input is a row input cell
    // shared formulas
    QHash<qint64, Master> m_masters;
    QHash<qint64, QVector<Pending>> m_pending;
    qint64 m_cells = 0;
    qint64 m_bytes = 0;              // what the sheet takes so far, as Limits::sheetBytes counts it
};

// Whether the text is written as a cell's address (letters, then digits, with the "$" that may be before each), whether or not it is inside the sheet.
bool looksLikeAddress(QStringView s)
{
    qsizetype i = 0;
    auto run = [&](bool (*is)(QChar)) {
        const qsizetype start = i;
        while (i < s.size() && is(s[i])) ++i;
        return i > start;
    };
    if (i < s.size() && s[i] == QLatin1Char('$')) ++i;
    if (!run([](QChar c) { return c.unicode() < 128 && c.isLetter(); })) return false;
    if (i < s.size() && s[i] == QLatin1Char('$')) ++i;
    return run([](QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; }) && i == s.size();
}

bool SheetReader::startRow(const QXmlStreamReader &r)
{
    bool present = false;
    const QStringView at = attribute(r, "r", &present);
    bool ok = false;
    const qint64 given = present ? at.toLongLong(&ok) : 0;
    if (ok) {
        // A number that is no row of the sheet is refused; text that is no number is as no number (the row follows the one before).
        if (given < 1 || given > qint64(kMaxRows)) return fail(QCoreApplication::translate("Sheet", "A row is outside the sheet."));
        m_row = quint32(given - 1);
    } else {
        // A row with no number follows the one before.
        if (m_haveRow && m_row + 1 >= kMaxRows) return fail(QCoreApplication::translate("Sheet", "A row is outside the sheet."));
        m_row = m_haveRow ? m_row + 1 : 0;
    }
    m_haveRow = true;
    m_nextCol = 0;
    if (m_options.stopAfterRow >= 0 && qint64(m_row) > m_options.stopAfterRow) {
        m_sheet.partial = true;
        m_stop = true;
        return true;
    }
    if (m_options.screenRows > 0) {
        // the rows the view shows: those of a frozen part (which may begin below the first row), and `screenRows` from where the part that scrolls begins
        const SheetView &v = m_sheet.view;
        const qint64 skipTop = v.frozen ? qint64(v.topRow) : 0;
        const qint64 frozenEnd = v.frozen ? skipTop + qint64(v.frozenRows) : 0;
        const qint64 begin = std::max(frozenEnd, v.frozen ? qint64(v.paneRow) : qint64(v.topRow));
        if (qint64(m_row) >= begin + m_options.screenRows) {
            m_sheet.partial = true;
            m_stop = true;
            return true;
        }
        m_skipCells = qint64(m_row) < skipTop || (qint64(m_row) >= frozenEnd && qint64(m_row) < begin);
    }
    // A row that is not as the sheet's default is kept: its height, whether it is hidden, and the format its empty cells have.
    bool hasHeight = false;
    const QStringView ht = attribute(r, "ht", &hasHeight);
    RowSpec spec;
    spec.row = m_row;
    if (hasHeight) {
        bool numeric = false;
        const double v = ht.trimmed().toDouble(&numeric);
        if (numeric && std::isfinite(v) && v >= 0) spec.height = std::min(v, 409.5);
    }
    spec.hidden = parseBool(attribute(r, "hidden").toString());
    spec.customHeight = parseBool(attribute(r, "customHeight").toString());
    spec.customFormat = parseBool(attribute(r, "customFormat").toString());
    bool styleOk = false;
    const uint style = attribute(r, "s").toUInt(&styleOk);
    spec.style = styleOk && spec.customFormat ? style : 0;
    spec.outline = quint8(std::clamp(attribute(r, "outlineLevel").toInt(), 0, 7));
    spec.collapsed = parseBool(attribute(r, "collapsed").toString());
    const bool otherHeight = spec.height >= 0 && std::abs(spec.height - m_sheet.format.defaultRowHeight) > 0.001;
    // (a sheet that hides the rows it does not list needs to know which it lists)
    if (spec.hidden || spec.customFormat || otherHeight || m_sheet.format.zeroHeight || spec.outline > 0 || spec.collapsed) {
        if (m_sheet.stats.rowSpecs >= m_limits.rowSpecs) return fail(QCoreApplication::translate("Sheet", "The sheet has more rows with their own height or format than any real file has."));
        if (!take(kRowSpecBytes)) return false;
        ++m_sheet.stats.rowSpecs;
        m_sheet.rowSpecs.append(spec);
    }
    return true;
}

Validation SheetReader::readRule(const QXmlStreamReader &r) const
{
    Validation v;
    const QString type = attribute(r, "type").toString();
    static const QStringList types = {QStringLiteral("none"), QStringLiteral("whole"), QStringLiteral("decimal"), QStringLiteral("list"), QStringLiteral("date"), QStringLiteral("time"), QStringLiteral("textLength"), QStringLiteral("custom")};
    const int t = int(types.indexOf(type));
    v.type = t >= 0 ? Validation::Type(t) : Validation::Type::Any;
    const QString style = attribute(r, "errorStyle").toString();
    v.style = style == QLatin1String("warning") ? Validation::Style::Warning : style == QLatin1String("information") ? Validation::Style::Information : Validation::Style::Stop;
    static const QStringList ops = {QStringLiteral("between"), QStringLiteral("notBetween"), QStringLiteral("equal"), QStringLiteral("notEqual"), QStringLiteral("greaterThan"), QStringLiteral("lessThan"),
                                    QStringLiteral("greaterThanOrEqual"), QStringLiteral("lessThanOrEqual")};
    const int o = int(ops.indexOf(attribute(r, "operator").toString()));
    v.compare = o >= 0 ? Validation::Compare(o) : Validation::Compare::Between;
    v.allowBlank = parseBool(attribute(r, "allowBlank").toString());
    v.dropDown = !parseBool(attribute(r, "showDropDown").toString());
    v.showError = parseBool(attribute(r, "showErrorMessage").toString());
    v.showInput = parseBool(attribute(r, "showInputMessage").toString());
    v.errorTitle = attribute(r, "errorTitle").toString().left(32);
    v.error = attribute(r, "error").toString().left(255);
    v.promptTitle = attribute(r, "promptTitle").toString().left(32);
    v.prompt = attribute(r, "prompt").toString().left(255);
    for (const QString &part : attribute(r, "sqref").toString().split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
        CellRange range;
        if (parseRangeRef(part, &range) && v.ranges.size() < 64) v.ranges.append(range);
    }
    return v;
}

CondRule SheetReader::readCondRule(const QXmlStreamReader &r) const
{
    CondRule c;
    static const QHash<QString, CondRule::Type> types = {
        {QStringLiteral("cellIs"), CondRule::Type::CellIs}, {QStringLiteral("expression"), CondRule::Type::Expression}, {QStringLiteral("colorScale"), CondRule::Type::ColorScale},
        {QStringLiteral("dataBar"), CondRule::Type::DataBar}, {QStringLiteral("iconSet"), CondRule::Type::IconSet}, {QStringLiteral("top10"), CondRule::Type::Top10},
        {QStringLiteral("uniqueValues"), CondRule::Type::UniqueValues}, {QStringLiteral("duplicateValues"), CondRule::Type::DuplicateValues}, {QStringLiteral("containsText"), CondRule::Type::ContainsText},
        {QStringLiteral("notContainsText"), CondRule::Type::NotContainsText}, {QStringLiteral("beginsWith"), CondRule::Type::BeginsWith}, {QStringLiteral("endsWith"), CondRule::Type::EndsWith},
        {QStringLiteral("containsBlanks"), CondRule::Type::ContainsBlanks}, {QStringLiteral("notContainsBlanks"), CondRule::Type::NotContainsBlanks}, {QStringLiteral("containsErrors"), CondRule::Type::ContainsErrors},
        {QStringLiteral("notContainsErrors"), CondRule::Type::NotContainsErrors}, {QStringLiteral("timePeriod"), CondRule::Type::TimePeriod}, {QStringLiteral("aboveAverage"), CondRule::Type::AboveAverage}};
    c.type = types.value(attribute(r, "type").toString(), CondRule::Type::Expression);
    static const QStringList ops = {QStringLiteral("lessThan"), QStringLiteral("lessThanOrEqual"), QStringLiteral("equal"), QStringLiteral("notEqual"), QStringLiteral("greaterThanOrEqual"),
                                    QStringLiteral("greaterThan"), QStringLiteral("between"), QStringLiteral("notBetween")};
    const int o = int(ops.indexOf(attribute(r, "operator").toString()));
    c.op = o >= 0 ? CondRule::Operator(o) : CondRule::Operator::Equal;
    bool ok = false;
    c.priority = attribute(r, "priority").toInt(&ok);
    if (!ok) c.priority = 1 << 20;
    c.dxf = attribute(r, "dxfId").isEmpty() ? -1 : attribute(r, "dxfId").toInt(&ok);
    if (!ok) c.dxf = -1;
    c.stopIfTrue = parseBool(attribute(r, "stopIfTrue").toString());
    c.text = attribute(r, "text").left(255).toString();
    c.timePeriod = attribute(r, "timePeriod").left(20).toString();
    c.rank = int(std::lround(boundedNumber(attribute(r, "rank"), 1, 1000, 10)));
    c.percent = parseBool(attribute(r, "percent").toString());
    c.bottom = parseBool(attribute(r, "bottom").toString());
    c.above = attribute(r, "aboveAverage").isEmpty() ? true : parseBool(attribute(r, "aboveAverage").toString());
    c.equalAverage = parseBool(attribute(r, "equalAverage").toString());
    c.stdDev = int(std::lround(boundedNumber(attribute(r, "stdDev"), 0, 3, 0)));
    return c;
}

bool SheetReader::startColumns(const QXmlStreamReader &r)
{
    bool ok1 = false, ok2 = false;
    const qint64 min = attribute(r, "min").toLongLong(&ok1), max = attribute(r, "max").toLongLong(&ok2);
    if (++m_sheet.stats.columnSpans > m_limits.columnSpans) return fail(QCoreApplication::translate("Sheet", "The sheet has more runs of columns than any real file has."));
    if (!ok1 || !ok2 || min < 1 || max < min || min > qint64(kMaxColumns)) return true;
    ColumnSpan c;
    c.first = quint32(min - 1);
    c.last = quint32(std::min<qint64>(max, qint64(kMaxColumns)) - 1);
    bool numeric = false;
    const double w = attribute(r, "width").trimmed().toDouble(&numeric);
    c.width = numeric && std::isfinite(w) && w >= 0 ? std::min(w, 255.0) : -1;
    c.hidden = parseBool(attribute(r, "hidden").toString());
    c.customWidth = parseBool(attribute(r, "customWidth").toString());
    bool styleOk = false;
    const uint style = attribute(r, "style").toUInt(&styleOk);
    c.style = styleOk ? style : 0;
    c.outline = quint8(std::min(7, attribute(r, "outlineLevel").toInt()));
    c.collapsed = parseBool(attribute(r, "collapsed").toString());
    c.bestFit = parseBool(attribute(r, "bestFit").toString());
    c.phonetic = parseBool(attribute(r, "phonetic").toString());
    if (c.width < 0 && !c.hidden && c.style == 0 && c.outline == 0 && !c.collapsed && !c.bestFit && !c.phonetic) return true;
    if (!take(kColumnSpanBytes)) return false;
    m_sheet.columns.append(c);
    return true;
}

void SheetReader::startView(const QXmlStreamReader &r)
{
    if (m_viewSeen) return;   // (the first view of a sheet is the one that is shown)
    m_viewSeen = true;
    SheetView &v = m_sheet.view;
    auto flag = [&](const char *name, bool fallback) {
        bool present = false;
        const QStringView value = attribute(r, name, &present);
        return present ? parseBool(value.toString(), fallback) : fallback;
    };
    v.showGridLines = flag("showGridLines", true);
    v.showHeadings = flag("showRowColHeaders", true);
    v.showZeros = flag("showZeros", true);
    v.rightToLeft = flag("rightToLeft", false);
    v.tabSelected = flag("tabSelected", false);
    v.zoom = int(std::lround(boundedZoom(attribute(r, "zoomScale"))));
    {
        bool present = false;
        const QStringView normal = attribute(r, "zoomScaleNormal", &present);
        v.normalZoom = present ? int(std::lround(boundedZoom(normal))) : 0;   // (the kind of view, below, says what none means)
    }
    const QStringView kind = attribute(r, "view");
    v.kind = kind == QLatin1String("pageBreakPreview") ? SheetView::Kind::PageBreaks : kind == QLatin1String("pageLayout") ? SheetView::Kind::PageLayout : SheetView::Kind::Normal;
    if (v.normalZoom == 0) v.normalZoom = v.kind == SheetView::Kind::Normal ? v.zoom : 100;
    quint32 row = 0, col = 0;
    if (parseCellRef(attribute(r, "topLeftCell"), &row, &col)) {
        v.topRow = row;
        v.leftCol = col;
    }
}

void SheetReader::startPane(const QXmlStreamReader &r)
{
    if (!m_viewSeen || m_sheet.view.frozen || !m_activePane.isEmpty()) return;
    SheetView &v = m_sheet.view;
    const QStringView state = attribute(r, "state");
    m_activePane = attribute(r, "activePane").toString();
    if (state != QLatin1String("frozen") && state != QLatin1String("frozenSplit")) return;
    bool ok = false;
    const double x = attribute(r, "xSplit").trimmed().toDouble(&ok), y = attribute(r, "ySplit").trimmed().toDouble(&ok);
    const double cols = std::isfinite(x) && x > 0 ? std::floor(x) : 0, rows = std::isfinite(y) && y > 0 ? std::floor(y) : 0;
    if ((cols == 0 && rows == 0) || cols >= double(kMaxColumns) || rows >= double(kMaxRows)) return;
    v.frozen = true;
    v.frozenCols = quint32(cols);
    v.frozenRows = quint32(rows);
    v.paneRow = v.topRow + v.frozenRows;
    v.paneCol = v.leftCol + v.frozenCols;
    quint32 row = 0, col = 0;
    if (parseCellRef(attribute(r, "topLeftCell"), &row, &col)) {
        v.paneRow = std::max(row, v.frozenRows);
        v.paneCol = std::max(col, v.frozenCols);
    }
}

void SheetReader::startSelection(const QXmlStreamReader &r)
{
    if (!m_viewSeen || m_selections.size() >= 8) return;
    Selection s;
    s.pane = attribute(r, "pane").toString();
    if (s.pane.isEmpty()) s.pane = QStringLiteral("topLeft");
    if (!parseCellRef(attribute(r, "activeCell"), &s.row, &s.col)) return;
    m_selections.append(s);
}

// The cell the sheet was left on is that of the pane that has the cursor.
void SheetReader::endView()
{
    if (m_selections.isEmpty()) return;
    const SheetView &view = m_sheet.view;
    QString want = m_activePane;
    if (want.isEmpty()) want = view.frozen ? (view.frozenRows && view.frozenCols ? QStringLiteral("bottomRight") : view.frozenRows ? QStringLiteral("bottomLeft") : QStringLiteral("topRight"))
                                           : QStringLiteral("topLeft");
    const Selection *chosen = &m_selections.first();
    for (const Selection &s : m_selections)
        if (s.pane == want) {
            chosen = &s;
            break;
        }
    m_sheet.view.activeRow = chosen->row;
    m_sheet.view.activeCol = chosen->col;
    m_selections.clear();
}

bool SheetReader::startMerge(const QXmlStreamReader &r)
{
    ++m_sheet.mergeCount;
    CellRange range;
    if (!parseRangeRef(attribute(r, "ref"), &range) || range.cellCount() < 2) return true;   // (a range of one cell merges nothing)
    if (m_sheet.merges.size() >= m_limits.merges) return fail(QCoreApplication::translate("Sheet", "The sheet has more merged ranges than any real file has."));
    if (!take(kMergeBytes)) return false;
    m_sheet.merges.append(range);
    return true;
}

bool SheetReader::startCell(const QXmlStreamReader &r)
{
    // The three attributes a cell has, in one pass over them.
    bool present = false;
    QStringView at, style, type;
    const QXmlStreamAttributes attributes = r.attributes();
    for (const QXmlStreamAttribute &a : attributes) {
        if (!a.namespaceUri().isEmpty()) continue;
        const QStringView name = a.name();
        if (name.size() != 1) continue;
        if (name[0] == QLatin1Char('r')) {
            at = a.value();
            present = true;
        } else if (name[0] == QLatin1Char('s')) {
            style = a.value();
        } else if (name[0] == QLatin1Char('t')) {
            type = a.value();
        }
    }
    m_cell = Cell();
    quint32 row = 0, col = 0;
    if (present && parseCellRef(at, &row, &col)) {
        m_cellRow = row;
        m_cellCol = col;
    } else if (present && looksLikeAddress(at)) {
        // An address, but of a place past the sheet's last row or column.
        return fail(QCoreApplication::translate("Sheet", "A cell's address is outside the sheet."));
    } else {
        // No address, or one that is no address at all (some writers number cells another way): the cell follows the one before.
        if (m_nextCol >= kMaxColumns) return fail(QCoreApplication::translate("Sheet", "A cell is outside the sheet."));
        m_cellRow = m_row;
        m_cellCol = m_nextCol;
    }
    m_nextCol = m_cellCol + 1;
    bool ok = false;
    const uint styleIndex = style.toUInt(&ok);
    m_cell.style = ok ? styleIndex : 0;
    m_type = type.toString();
    m_haveValue = m_haveInline = false;
    m_value.clear();
    m_inline.clear();
    m_formula.clear();
    m_kind = Normal;
    m_haveRef = false;
    return true;
}

bool SheetReader::startFormula(const QXmlStreamReader &r)
{
    const QStringView kind = attribute(r, "t");
    bool hasSi = false;
    m_kind = kind == QLatin1String("array") ? Array : kind == QLatin1String("shared") ? Shared : kind == QLatin1String("dataTable") ? Table : Normal;
    m_si = -1;
    const QStringView si = attribute(r, "si", &hasSi);
    if (hasSi) {
        bool ok = false;
        const qint64 v = si.toLongLong(&ok);
        if (ok && v >= 0) m_si = v;
    }
    // A shared formula without a number is a formula of its own.
    if (m_kind == Shared && m_si < 0) m_kind = Normal;
    m_haveRef = false;
    bool hasRef = false;
    const QStringView ref = attribute(r, "ref", &hasRef);
    if (hasRef) m_haveRef = parseRangeRef(ref, &m_ref);
    if (m_kind == Table) {
        m_r1 = attribute(r, "r1").toString();
        m_r2 = attribute(r, "r2").toString();
        m_dtr = parseBool(attribute(r, "dtr").toString());
    }
    m_formula.clear();
    return true;
}

bool SheetReader::endFormula()
{
    Formula f;
    m_cell.flags |= HasFormula;
    const bool blank = std::all_of(m_formula.cbegin(), m_formula.cend(), [](QChar c) { return c.isSpace(); });
    const qint64 length = m_formula.size();
    if (length > m_sheet.stats.longestFormula) m_sheet.stats.longestFormula = length;
    qint32 text = -1;
    if (m_kind == Table) {
        m_cell.flags |= DataTable;
        // as the formula of a table is written: the row input cell, then the column input cell; a table of one input has the other empty
        m_formula = QStringLiteral("TABLE(") + (!m_r2.isEmpty() ? m_r1 + QLatin1Char(',') + m_r2 : m_dtr ? m_r1 + QLatin1Char(',') : QLatin1Char(',') + m_r1) + QLatin1Char(')');
        if (!take(stringBytes(m_formula.size()))) return false;
        m_sheet.formulaTexts.append(m_formula);
        text = qint32(m_sheet.formulaTexts.size() - 1);
        m_sheet.stats.expandedFormulaChars += m_formula.size();
    } else if (m_kind == Shared && blank) {
        // A cell of a shared formula: the text is its first cell's, which may not have come yet.
        m_cell.flags |= SharedFormula;
        const auto master = m_masters.constFind(m_si);
        if (master != m_masters.cend()) {
            text = master->text;
            f.dRow = qint32(qint64(m_cellRow) - qint64(master->row));
            f.dCol = qint32(qint64(m_cellCol) - qint64(master->col));
            m_sheet.stats.expandedFormulaChars += m_sheet.formulaTexts[text].size();
        }
    } else if (!blank) {
        if (!take(stringBytes(length))) return false;
        m_sheet.formulaTexts.append(m_formula);
        text = qint32(m_sheet.formulaTexts.size() - 1);
        m_sheet.stats.expandedFormulaChars += length;
        if (m_kind == Array) m_cell.flags |= ArrayFormula;
        if (m_kind == Shared) {
            m_cell.flags |= SharedFormula;
            if (!m_masters.contains(m_si)) {
                if (m_sheet.stats.sharedFormulas >= m_limits.sharedFormulas) return fail(QCoreApplication::translate("Sheet", "The sheet has more shared formulas than any real file has."));
                ++m_sheet.stats.sharedFormulas;
                if (m_haveRef) {
                    if (m_ref.cellCount() > m_limits.sharedRangeCells) return fail(QCoreApplication::translate("Sheet", "A shared formula covers a range larger than any real file has."));
                    m_sheet.stats.largestSharedRange = std::max(m_sheet.stats.largestSharedRange, m_ref.cellCount());
                }
                if (!take(kMasterBytes)) return false;
                m_masters.insert(m_si, {text, m_cellRow, m_cellCol});
                // The cells that came before the first cell of the formula have it now.
                const auto waiting = m_pending.find(m_si);
                if (waiting != m_pending.end()) {
                    for (const Pending &p : std::as_const(*waiting)) {
                        Formula &g = m_sheet.formulas[p.formula];
                        g.text = text;
                        g.dRow = qint32(qint64(p.row) - qint64(m_cellRow));
                        g.dCol = qint32(qint64(p.col) - qint64(m_cellCol));
                        m_sheet.stats.expandedFormulaChars += length;
                    }
                    m_pending.erase(waiting);
                }
            }
        }
    }
    if (m_sheet.stats.expandedFormulaChars > m_limits.expandedFormulaChars) return fail(QCoreApplication::translate("Sheet", "The formulas, all together, are longer than any real file has."));
    f.text = text;
    if (!take(kFormulaBytes)) return false;
    if ((m_kind == Array || m_kind == Table) && m_haveRef) {
        if (!take(kRangeBytes)) return false;
        m_sheet.ranges.append(m_ref);
        f.range = qint32(m_sheet.ranges.size() - 1);
    }
    m_sheet.formulas.append(f);
    m_cell.formula = qint32(m_sheet.formulas.size() - 1);
    if (text < 0 && m_kind == Shared) {
        // Waiting for the first cell of its formula: each such cell is held, and how many different formulas are waited for is bounded as those that have come.
        auto waiting = m_pending.find(m_si);
        if (waiting == m_pending.end()) {
            if (m_pending.size() >= m_limits.sharedFormulas) return fail(QCoreApplication::translate("Sheet", "The sheet has more shared formulas than any real file has."));
            if (!take(kWaitingBytes)) return false;
            waiting = m_pending.insert(m_si, QVector<Pending>());
        } else if (!take(kWaitingCellBytes)) {
            return false;
        }
        waiting->append({m_cell.formula, m_cellRow, m_cellCol});
    }
    return true;
}

bool SheetReader::endCell()
{
    if (m_skipCells) {
        // (a row the view has left above it: its cells are not kept, and not worked out; what a shared formula's first cell tells has been told by now; the read can still be stopped)
        if (++m_skipped % kProgressEvery == 0 && m_options.progress && !m_options.progress(m_cells + m_skipped)) return fail(QCoreApplication::translate("Sheet", "The read was stopped."));
        return true;
    }
    Cell &c = m_cell;
    const QString type = m_type;
    auto number = [&](CellType t) {
        double v = 0;
        if (toNumber(m_value, &v)) {
            c.type = t;
            c.number = v;
        } else {
            c.type = CellType::Error;
            c.number = double(int(CellError::Value));
        }
    };
    bool room = true;
    auto pooled = [&](CellType t, const QString &text) {
        room = take(stringBytes(text.size()));
        c.type = t;
        c.number = double(m_sheet.texts.size());
        m_sheet.texts.append(text);
        if (text.size() > m_sheet.stats.longestString) m_sheet.stats.longestString = text.size();
    };
    if (type == QLatin1String("inlineStr")) {
        if (m_haveInline) {
            decodeEscapes(&m_inline);
            pooled(CellType::InlineString, m_inline);
        }
    } else if (m_haveValue) {
        if (type == QLatin1String("s")) {
            bool ok = false;
            const qint64 index = m_value.trimmed().toLongLong(&ok);
            if (ok && index >= 0 && index < (qint64(1) << 31)) {
                c.type = CellType::SharedString;
                c.number = double(index);
            }
        } else if (type == QLatin1String("str")) {
            decodeEscapes(&m_value);
            pooled(CellType::FormulaString, m_value);
        } else if (type == QLatin1String("b")) {
            const QString v = m_value.trimmed();
            if (v == QLatin1String("1") || v.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0) {
                c.type = CellType::Bool;
                c.number = 1;
            } else if (v == QLatin1String("0") || v.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0) {
                c.type = CellType::Bool;
                c.number = 0;
            } else if (!v.isEmpty()) {
                c.type = CellType::Error;
                c.number = double(int(CellError::Value));
            }
        } else if (type == QLatin1String("e")) {
            c.type = CellType::Error;
            c.number = double(int(errorFromText(QStringView(m_value).trimmed())));
        } else if (type == QLatin1String("d")) {
            double serial = 0;
            if (calc::parseIsoDate(m_value, m_options.dateSystem, &serial) || toNumber(m_value, &serial)) {
                c.type = CellType::Date;
                c.number = serial;
            } else if (!m_value.trimmed().isEmpty()) {
                c.type = CellType::Error;
                c.number = double(int(CellError::Value));
            }
        } else if (!m_value.trimmed().isEmpty()) {
            number(CellType::Number);
        }
    }
    if (c.hasFormula()) {
        ++m_sheet.formulaCells;
        if (c.type != CellType::Empty) ++m_sheet.storedResults;
    }
    if (!room) return false;

    if (m_cells >= m_limits.cellsPerSheet) return fail(QCoreApplication::translate("Sheet", "The sheet has more cells than any real file has."));
    if (!take(kCellBytes)) return false;
    ++m_cells;
    if (c.type != CellType::Empty || c.hasFormula() || c.style != 0) {
        m_sheet.extentRows = std::max(m_sheet.extentRows, m_cellRow + 1);
        m_sheet.extentCols = std::max(m_sheet.extentCols, m_cellCol + 1);
    }
    m_sheet.cells.add(m_cellRow, m_cellCol, c);
    if (m_cells % kProgressEvery == 0 && m_options.progress && !m_options.progress(m_cells)) return fail(QCoreApplication::translate("Sheet", "The read was stopped."));
    return true;
}

void SheetReader::finish() { m_sheet.cells.finish(); }

bool SheetReader::read(QIODevice *in)
{
    XmlSource src(in, m_limits);
    QXmlStreamReader &r = src.reader;
    int depth = 0, phonetic = 0;
    bool rootSeen = false, inData = false, inRow = false, inCell = false, inFormula = false, inValue = false, inInline = false, inText = false, inMerge = false;
    bool inSheetPr = false, inViews = false, inCols = false, inView = false, inValidations = false, inRule = false, inLinks = false;
    bool inHeaderFooter = false;
    int inBreaks = 0;              // 1 while the row breaks are read, 2 for the column breaks
    QString *hfText = nullptr;     // the header or footer text being read
    // conditional formatting: the ranges of the element being read, the rule, where its formula text goes, and (in the extension list) the data bars' extra attributes by their ids
    bool inCond = false, inCondRule = false, inExtension = false;
    QVector<CellRange> condRanges;
    CondRule condRule;
    QString *condText = nullptr;
    int condFormulas = 0;
    QHash<QString, CondRule> extensionBars;   // by id: only what the extension adds to a data bar
    QString extensionBarId;
    CondRule extensionBar;
    bool inExtensionBar = false;
    QString *ruleText = nullptr;   // the formula of the rule being read, while its text is collected
    Validation rule;
    // the filter: the column in the making, and the sort state (written again as it is)
    bool inFilter = false, inFilterColumn = false, filterColumnBad = false;
    FilterColumn filterColumn;
    qint64 filterValues = 0;
    QString sortState;
    // the rules of validation in the extension list (x14): the rule, and where the text of its formula or ranges goes
    bool inX14Rules = false, inX14Rule = false;
    Validation x14Rule;
    QString x14Ranges;
    QString *x14Text = nullptr;
    int x14Which = 1;
    bool ok = true;
    // Whoever asked to be told how far the read has come is told at least once for every so many bytes of the XML, so that a sheet of anything (merged ranges,
    // an extension list) and not only of cells can be stopped.
    qint64 nextReport = kReportEveryBytes;
    while (ok && !m_stop && !r.atEnd()) {
        if (src.guard.bytesPassed() >= nextReport) {
            nextReport = src.guard.bytesPassed() + kReportEveryBytes;
            if (m_options.progress && !m_options.progress(m_cells)) ok = fail(QCoreApplication::translate("Sheet", "The read was stopped."));
            if (!ok) break;
        }
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            ok = fail(QCoreApplication::translate("Sheet", "The file begins with instructions that this program does not allow, so it can't be read."));
            break;
        case QXmlStreamReader::StartElement:
            // (what the extensions hold that the program does not keep: a copy of the sheet is told, so that it can say what it leaves out. A data bar's extension is read into the rule.)
            if (inExtension && depth == 3) {
                const QString kind = r.name().toString();
                const char *note = kind == QLatin1String("sparklineGroups") ? "sparklines" : kind == QLatin1String("dataValidations") || kind == QLatin1String("conditionalFormattings") ? nullptr
                                   : "otherExtensions";
                if (note && !m_sheet.notKept.contains(QLatin1String(note))) m_sheet.notKept.append(QLatin1String(note));
            } else if (inExtension && depth == 5 && isElement(r, "cfRule") && attribute(r, "type") != QLatin1String("dataBar")) {
                if (!m_sheet.notKept.contains(QLatin1String("extendedRules"))) m_sheet.notKept.append(QLatin1String("extendedRules"));
            }
            if (++depth > kMaxXmlDepth) {
                ok = fail(QCoreApplication::translate("Sheet", "The file has items inside items more deeply than any real file has."));
            } else if (depth == 1) {
                if (!(isElement(r, "worksheet") || isElement(r, "macrosheet") || isElement(r, "dialogsheet") || isElement(r, "chartsheet")))
                    ok = fail(QCoreApplication::translate("Sheet", "The part is not a worksheet."));
                rootSeen = true;
            } else if (depth == 2) {
                if (isElement(r, "sheetData")) {
                    inData = true;
                } else if (isElement(r, "dimension")) {
                    parseRangeRef(attribute(r, "ref"), &m_sheet.dimension);
                } else if (isElement(r, "mergeCells")) {
                    inMerge = true;
                } else if (isElement(r, "dataValidations")) {
                    inValidations = true;
                } else if (isElement(r, "conditionalFormatting")) {
                    inCond = true;
                    condRanges.clear();
                    for (const QString &part : attribute(r, "sqref").toString().split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                        CellRange range;
                        if (condRanges.size() < 1000 && parseRangeRef(part, &range)) condRanges.append(range);
                    }
                } else if (isElement(r, "extLst")) {
                    inExtension = true;
                } else if (isElement(r, "autoFilter")) {
                    inFilter = true;
                    m_sheet.autoFilter = AutoFilter();
                    CellRange range;
                    if (parseRangeRef(attribute(r, "ref").toString(), &range)) m_sheet.autoFilter.ref = range;
                } else if (isElement(r, "sheetProtection")) {
                    readProtection(r, &m_sheet.protection);
                    m_sheet.isProtected = m_sheet.protection.enabled;
                } else if (isElement(r, "hyperlinks")) {
                    inLinks = true;
                } else if (isElement(r, "sheetPr")) {
                    inSheetPr = true;
                    m_sheet.transitionEvaluation = parseBool(attribute(r, "transitionEvaluation").toString());
                    m_sheet.transitionEntry = parseBool(attribute(r, "transitionEntry").toString());
                } else if (isElement(r, "pageSetup")) {
                    readPageSetupAttributes(r.attributes(), &m_sheet.page);
                } else if (isElement(r, "pageMargins")) {
                    readPageMarginsAttributes(r.attributes(), &m_sheet.page);
                } else if (isElement(r, "printOptions")) {
                    readPrintOptionsAttributes(r.attributes(), &m_sheet.page);
                } else if (isElement(r, "headerFooter")) {
                    readHeaderFooterAttributes(r.attributes(), &m_sheet.page);
                    inHeaderFooter = true;
                } else if (isElement(r, "legacyDrawingHF")) {
                    for (const QXmlStreamAttribute &a : r.attributes())
                        if (a.name() == QLatin1String("id") && !a.namespaceUri().isEmpty() && sameNamespace(a.namespaceUri().toString(), ns::R)) m_sheet.page.headerFooter.drawingRel = a.value().toString();
                } else if (isElement(r, "rowBreaks")) {
                    inBreaks = 1;
                } else if (isElement(r, "colBreaks")) {
                    inBreaks = 2;
                } else if (isElement(r, "sheetViews")) {
                    inViews = true;
                } else if (isElement(r, "cols")) {
                    inCols = true;
                } else if (isElement(r, "sheetFormatPr")) {
                    SheetFormat &f = m_sheet.format;
                    f.defaultRowHeight = boundedNumber(attribute(r, "defaultRowHeight"), 0, 409.5, 15);
                    f.defaultColWidth = boundedNumber(attribute(r, "defaultColWidth"), 0, 255, 0);
                    f.baseColWidth = int(std::lround(boundedNumber(attribute(r, "baseColWidth"), 0, 255, 8)));
                    f.zeroHeight = parseBool(attribute(r, "zeroHeight").toString());
                    f.customHeight = parseBool(attribute(r, "customHeight").toString());
                    f.outlineLevelRow = quint8(std::clamp(attribute(r, "outlineLevelRow").toInt(), 0, 7));
                    f.outlineLevelCol = quint8(std::clamp(attribute(r, "outlineLevelCol").toInt(), 0, 7));
                }
            } else if (inFilter && depth == 3 && isElement(r, "filterColumn")) {
                inFilterColumn = true;
                filterColumn = FilterColumn();
                bool good = false;
                const int id = attribute(r, "colId").toInt(&good);
                filterColumn.column = good && id >= 0 && id < int(kMaxColumns) ? quint32(id) : 0;
                filterColumnBad = !(good && id >= 0 && id < int(kMaxColumns));
                filterColumn.hideButton = parseBool(attribute(r, "hiddenButton").toString()) || (!attribute(r, "showButton").isEmpty() && !parseBool(attribute(r, "showButton").toString(), true));
            } else if (inFilter && depth == 3 && isElement(r, "sortState")) {
                sortState = QStringLiteral("<sortState");
                for (const QXmlStreamAttribute &a : r.attributes())
                    if (a.namespaceUri().isEmpty()) sortState += QStringLiteral(" %1=\"%2\"").arg(a.name().toString(), a.value().toString().toHtmlEscaped());
                sortState += QLatin1Char('>');
            } else if (inFilter && depth == 4 && !sortState.isEmpty() && sortState.size() < 65536 && isElement(r, "sortCondition")) {
                sortState += QStringLiteral("<sortCondition");
                for (const QXmlStreamAttribute &a : r.attributes())
                    if (a.namespaceUri().isEmpty()) sortState += QStringLiteral(" %1=\"%2\"").arg(a.name().toString(), a.value().toString().toHtmlEscaped());
                sortState += QStringLiteral("/>");
            } else if (inFilterColumn && depth == 4) {
                if (isElement(r, "filters")) {
                    filterColumn.kind = FilterColumn::Kind::Values;
                    filterColumn.blank = parseBool(attribute(r, "blank").toString());
                } else if (isElement(r, "customFilters")) {
                    filterColumn.kind = FilterColumn::Kind::Custom;
                    filterColumn.both = parseBool(attribute(r, "and").toString());
                } else if (isElement(r, "top10")) {
                    filterColumn.kind = FilterColumn::Kind::Top10;
                    filterColumn.top = attribute(r, "top").isEmpty() ? true : parseBool(attribute(r, "top").toString(), true);
                    filterColumn.percent = parseBool(attribute(r, "percent").toString());
                    filterColumn.count = boundedNumber(attribute(r, "val"), 0, 1e6, 10);
                    filterColumn.hasLimit = !attribute(r, "filterVal").isEmpty();
                    filterColumn.limit = boundedNumber(attribute(r, "filterVal"), -1e300, 1e300, 0);
                } else if (isElement(r, "dynamicFilter")) {
                    filterColumn.kind = FilterColumn::Kind::Dynamic;
                    filterColumn.dynamic = attribute(r, "type").left(40).toString();
                    filterColumn.hasDynVal = !attribute(r, "val").isEmpty();
                    filterColumn.dynVal = boundedNumber(attribute(r, "val"), -1e300, 1e300, 0);
                    filterColumn.dynMax = boundedNumber(attribute(r, "maxVal"), -1e300, 1e300, 0);
                } else if (isElement(r, "colorFilter")) {
                    filterColumn.kind = FilterColumn::Kind::Color;
                    bool good = false;
                    const int dxf = attribute(r, "dxfId").toInt(&good);
                    filterColumn.dxf = good ? dxf : -1;
                    // (the cells' own color, unless the file says cellColor="0": then it is the text's)
                    filterColumn.fontColor = !attribute(r, "cellColor").isEmpty() && !parseBool(attribute(r, "cellColor").toString(), true);
                }
            } else if (inFilterColumn && depth == 5) {
                if (isElement(r, "filter") && filterColumn.kind == FilterColumn::Kind::Values) {
                    // (what the listed values take is counted with the rest of what the sheet holds: a sheet whose filter lists more than the memory limit allows is not read)
                    const QStringView val = attribute(r, "val").left(1000);
                    if (!take(kSlotBytes + 2 * val.size() + 48)) {
                        ok = false;
                        break;
                    }
                    if (++filterValues <= kMaxFilterValues) filterColumn.values.append(val.toString());
                } else if (isElement(r, "dateGroupItem") && filterColumn.kind == FilterColumn::Kind::Values) {
                    if (!take(kSlotBytes + 64)) {
                        ok = false;
                        break;
                    }
                    if (++filterValues <= kMaxFilterValues) {
                        FilterColumn::DateGroup g;
                        const QStringView by = attribute(r, "dateTimeGrouping");
                        g.unit = by == QLatin1String("year") ? FilterColumn::DateGroup::Unit::Year : by == QLatin1String("month") ? FilterColumn::DateGroup::Unit::Month
                                 : by == QLatin1String("hour") ? FilterColumn::DateGroup::Unit::Hour : by == QLatin1String("minute") ? FilterColumn::DateGroup::Unit::Minute
                                 : by == QLatin1String("second") ? FilterColumn::DateGroup::Unit::Second : FilterColumn::DateGroup::Unit::Day;
                        g.year = attribute(r, "year").toInt();
                        g.month = attribute(r, "month").toInt();
                        g.day = attribute(r, "day").toInt();
                        g.hour = attribute(r, "hour").toInt();
                        g.minute = attribute(r, "minute").toInt();
                        g.second = attribute(r, "second").toInt();
                        filterColumn.dates.append(g);
                    }
                } else if (isElement(r, "customFilter") && filterColumn.kind == FilterColumn::Kind::Custom && filterColumn.conditions.size() < 2) {
                    FilterColumn::Condition c;
                    const QStringView op = attribute(r, "operator");
                    c.op = op == QLatin1String("notEqual") ? FilterColumn::Op::NotEqual : op == QLatin1String("greaterThan") ? FilterColumn::Op::Greater
                           : op == QLatin1String("greaterThanOrEqual") ? FilterColumn::Op::GreaterEqual : op == QLatin1String("lessThan") ? FilterColumn::Op::Less
                           : op == QLatin1String("lessThanOrEqual") ? FilterColumn::Op::LessEqual : FilterColumn::Op::Equal;
                    c.value = attribute(r, "val").left(255).toString();
                    filterColumn.conditions.append(c);
                }
            } else if (inExtension && depth == 4 && isElement(r, "dataValidations")) {
                inX14Rules = true;
            } else if (inX14Rules && depth == 5 && isElement(r, "dataValidation")) {
                inX14Rule = true;
                x14Rule = readRule(r);
                x14Ranges.clear();
            } else if (inX14Rule && depth == 6 && (isElement(r, "formula1") || isElement(r, "formula2"))) {
                x14Which = isElement(r, "formula1") ? 1 : 2;   // (its text is in the element inside)
            } else if (inX14Rule && depth == 7 && isElement(r, "f")) {
                x14Text = x14Which == 1 ? &x14Rule.formula1 : &x14Rule.formula2;
            } else if (inX14Rule && depth == 6 && isElement(r, "sqref")) {
                x14Text = &x14Ranges;
            } else if (depth == 3 && inMerge && isElement(r, "mergeCell")) {
                ok = startMerge(r);
            } else if (depth == 3 && inLinks && isElement(r, "hyperlink")) {
                readLink(r, &m_sheet.links);
            } else if (depth == 3 && inValidations && isElement(r, "dataValidation")) {
                inRule = true;
                rule = readRule(r);
            } else if (depth == 4 && inRule && (isElement(r, "formula1") || isElement(r, "formula2"))) {
                ruleText = isElement(r, "formula1") ? &rule.formula1 : &rule.formula2;
            } else if (depth == 3 && inCond && isElement(r, "cfRule")) {
                inCondRule = true;
                condFormulas = 0;
                condRule = readCondRule(r);
                condRule.ranges = condRanges;
            } else if (inCondRule && depth == 4) {
                if (isElement(r, "formula")) condText = condFormulas++ == 0 ? &condRule.formula1 : &condRule.formula2;
                else if (isElement(r, "colorScale")) condRule.type = CondRule::Type::ColorScale;
                else if (isElement(r, "dataBar")) {
                    condRule.type = CondRule::Type::DataBar;
                    condRule.minLength = int(std::lround(boundedNumber(attribute(r, "minLength"), 0, 100, 10)));
                    condRule.maxLength = int(std::lround(boundedNumber(attribute(r, "maxLength"), 0, 100, 90)));
                    if (!attribute(r, "showValue").isEmpty()) condRule.showValue = parseBool(attribute(r, "showValue").toString());
                } else if (isElement(r, "iconSet")) {
                    condRule.type = CondRule::Type::IconSet;
                    condRule.iconSet = attribute(r, "iconSet").isEmpty() ? QStringLiteral("3TrafficLights1") : attribute(r, "iconSet").left(40).toString();
                    if (!attribute(r, "showValue").isEmpty()) condRule.showValue = parseBool(attribute(r, "showValue").toString());
                    condRule.reverse = parseBool(attribute(r, "reverse").toString());
                }
            } else if (inCondRule && depth == 5) {
                if (isElement(r, "cfvo") && condRule.values.size() < 8) {
                    CondValue v;
                    const QStringView type = attribute(r, "type");
                    v.type = type == QLatin1String("percent") ? CondValue::Type::Percent : type == QLatin1String("max") ? CondValue::Type::Max : type == QLatin1String("min") ? CondValue::Type::Min
                             : type == QLatin1String("formula") ? CondValue::Type::Formula : type == QLatin1String("percentile") ? CondValue::Type::Percentile : type == QLatin1String("autoMin") ? CondValue::Type::AutoMin
                             : type == QLatin1String("autoMax") ? CondValue::Type::AutoMax : CondValue::Type::Number;
                    v.value = attribute(r, "val").left(1000).toString();
                    v.gte = attribute(r, "gte").isEmpty() ? true : parseBool(attribute(r, "gte").toString());
                    condRule.values.append(v);
                } else if (isElement(r, "color") && condRule.colors.size() < 8) {
                    condRule.colors.append(colorOf(r));
                }
            } else if (inExtension && depth == 6 && isElement(r, "cfRule") && attribute(r, "type") == QLatin1String("dataBar")) {
                extensionBarId = attribute(r, "id").toString();
                extensionBar = CondRule();
                inExtensionBar = !extensionBarId.isEmpty();
            } else if (inExtensionBar && depth == 7 && isElement(r, "dataBar")) {
                extensionBar.minLength = int(std::lround(boundedNumber(attribute(r, "minLength"), 0, 100, 10)));
                extensionBar.maxLength = int(std::lround(boundedNumber(attribute(r, "maxLength"), 0, 100, 90)));
                extensionBar.gradient = attribute(r, "gradient").isEmpty() ? true : parseBool(attribute(r, "gradient").toString());
                extensionBar.border = parseBool(attribute(r, "border").toString());
                extensionBar.negativeSameColor = parseBool(attribute(r, "negativeBarColorSameAsPositive").toString());
                const QStringView axis = attribute(r, "axisPosition");
                extensionBar.axis = axis == QLatin1String("middle") ? CondRule::Axis::Middle : axis == QLatin1String("none") ? CondRule::Axis::None : CondRule::Axis::Automatic;
                extensionBar.rightToLeft = attribute(r, "direction") == QLatin1String("rightToLeft");
            } else if (inExtensionBar && depth == 8) {
                if (isElement(r, "negativeFillColor")) extensionBar.negativeColor = colorOf(r);
                else if (isElement(r, "borderColor")) extensionBar.borderColor = colorOf(r);
                else if (isElement(r, "negativeBorderColor")) extensionBar.negativeBorderColor = colorOf(r);
                else if (isElement(r, "axisColor")) extensionBar.axisColor = colorOf(r);
            } else if (inCondRule && depth == 6 && isElement(r, "id")) {
                condText = &condRule.extId;   // (the extension's x14:id of a data bar)
            } else if (depth == 3 && inCols && isElement(r, "col")) {
                ok = startColumns(r);
            } else if (depth == 3 && inSheetPr && isElement(r, "tabColor")) {
                m_sheet.tabColor = colorOf(r);
            } else if (depth == 3 && inSheetPr && isElement(r, "pageSetUpPr")) {
                readFitToPageAttributes(r.attributes(), &m_sheet.page);
            } else if (depth == 3 && inHeaderFooter) {
                hfText = headerFooterText(&m_sheet.page, r.name());
                if (hfText) hfText->clear();
            } else if (depth == 3 && inBreaks && isElement(r, "brk")) {
                readBreakAttributes(r.attributes(), inBreaks == 1, &m_sheet.page);
            } else if (depth == 3 && inViews && isElement(r, "sheetView")) {
                inView = true;
                startView(r);
            } else if (depth == 4 && inView && isElement(r, "pane")) {
                startPane(r);
            } else if (depth == 4 && inView && isElement(r, "selection")) {
                startSelection(r);
            } else if (depth == 3 && inData && isElement(r, "row")) {
                ok = startRow(r);
                if (ok && m_stop) break;
                inRow = ok;
            } else if (depth == 4 && inRow && isElement(r, "c")) {
                inCell = true;
                ok = startCell(r);
            } else if (depth == 5 && inCell) {
                if (isElement(r, "f")) {
                    inFormula = true;
                    ok = startFormula(r);
                } else if (isElement(r, "v")) {
                    inValue = true;
                    m_haveValue = true;
                    m_value.clear();
                } else if (isElement(r, "is")) {
                    inInline = true;
                    m_haveInline = true;
                    m_inline.clear();
                }
            } else if (inInline && !phonetic) {
                // The text of an inline string is its <t> and the <t> of its runs, as in a shared string.
                if (isElement(r, "rPh") || isElement(r, "phoneticPr")) phonetic = depth;
                else if (isElement(r, "t") && (depth == 6 || depth == 7)) inText = true;
            }
            break;
        case QXmlStreamReader::EndElement:
            if (phonetic == depth) phonetic = 0;
            if (x14Text && ((depth == 7 && isElement(r, "f")) || (depth == 6 && isElement(r, "sqref")))) {
                if (depth == 6) {
                    for (const QString &part : x14Ranges.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                        CellRange range;
                        if (parseRangeRef(part, &range) && x14Rule.ranges.size() < 64) x14Rule.ranges.append(range);
                    }
                }
                x14Text = nullptr;
            } else if (inX14Rule && depth == 5) {
                inX14Rule = false;
                if (!x14Rule.ranges.isEmpty() && m_sheet.validations.size() < kMaxValidations) m_sheet.validations.append(x14Rule);
            } else if (inX14Rules && depth == 4) {
                inX14Rules = false;
            } else if (inFilterColumn && depth == 3 && isElement(r, "filterColumn")) {
                inFilterColumn = false;
                // (a column the sheet does not have is not kept, nor is a second condition for one)
                if (!filterColumnBad && (filterColumn.kind != FilterColumn::Kind::None || filterColumn.hideButton) && m_sheet.autoFilter.columns.size() < int(kMaxColumns) && !m_sheet.autoFilter.columnAt(filterColumn.column))
                    m_sheet.autoFilter.columns.append(filterColumn);
            } else if (inFilter && depth == 3 && isElement(r, "sortState")) {
                sortState += QStringLiteral("</sortState>");
            } else if (inFilter && depth == 2) {
                inFilter = false;
                m_sheet.autoFilter.sortState = sortState;
                inData = inMerge = inSheetPr = inViews = inCols = inValidations = inCond = inExtension = inLinks = false;
            } else if (depth == 5 && inCell) {
                if (inFormula && isElement(r, "f")) {
                    inFormula = false;
                    ok = endFormula();
                } else if (inValue && isElement(r, "v")) {
                    inValue = false;
                } else if (inInline && isElement(r, "is")) {
                    inInline = false;
                }
            } else if (inText && isElement(r, "t")) {
                inText = false;
            } else if (depth == 4 && inCell) {
                inCell = false;
                ok = endCell();
            } else if (condText && ((depth == 4 && inCondRule) || (depth == 6 && inCondRule))) {
                condText = nullptr;
            } else if (inExtensionBar && depth == 6 && isElement(r, "cfRule")) {
                extensionBars.insert(extensionBarId, extensionBar);
                inExtensionBar = false;
            } else if (depth == 3 && inCondRule) {
                inCondRule = false;
                if (m_sheet.conditionalFormats.size() < kMaxCondRules) {
                    if (condRule.type == CondRule::Type::DataBar && condRule.colors.isEmpty()) condRule.colors.append(ColorSpec::fromRgb(0x638EC6));
                    m_sheet.conditionalFormats.append(condRule);
                }
            } else if (depth == 4 && ruleText) {
                ruleText = nullptr;
            } else if (depth == 3 && inRule) {
                inRule = false;
                if (!rule.ranges.isEmpty() && m_sheet.validations.size() < kMaxValidations) m_sheet.validations.append(rule);
            } else if (depth == 3 && inRow) {
                inRow = false;
            } else if (depth == 3 && inView) {
                inView = false;
                endView();
            } else if (depth == 3 && hfText) {
                hfText = nullptr;
            } else if (depth == 2) {
                inData = inMerge = inSheetPr = inViews = inCols = inValidations = inCond = inExtension = inLinks = false;
                inHeaderFooter = false;
                inBreaks = 0;
            }
            --depth;
            break;
        case QXmlStreamReader::Characters:
            if (x14Text) {
                if (x14Text->size() < m_limits.formulaChars) *x14Text += r.text();
            } else if (hfText) {
                const qsizetype room = kMaxHeaderFooterChars - hfText->size();
                if (room > 0) *hfText += r.text().left(room);
            } else if (condText) {
                if (condText->size() < m_limits.formulaChars) *condText += r.text();
            } else if (ruleText) {
                if (ruleText->size() < m_limits.formulaChars) *ruleText += r.text();
            } else if (inFormula) {
                m_formula += r.text();
                if (m_formula.size() > m_limits.formulaChars) ok = fail(QCoreApplication::translate("Sheet", "A formula is longer than any real file has."));
            } else if (inValue) {
                m_value += r.text();
                if (m_value.size() > m_limits.stringChars) ok = fail(QCoreApplication::translate("Sheet", "A string is longer than any real file has."));
            } else if (inText && !phonetic) {
                m_inline += r.text();
                if (m_inline.size() > m_limits.stringChars) ok = fail(QCoreApplication::translate("Sheet", "A string is longer than any real file has."));
            }
            break;
        default:
            break;
        }
    }
    if (ok) {
        const QString why = src.problem();
        if (!why.isEmpty()) ok = fail(why);
        else if (!rootSeen) ok = fail(QCoreApplication::translate("Sheet", "The part is not a worksheet."));
    }
    // the rows kept as not the default are in order of their number (a file may list them in any order; the model's edits count on it)
    if (!std::is_sorted(m_sheet.rowSpecs.begin(), m_sheet.rowSpecs.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; }))
        std::stable_sort(m_sheet.rowSpecs.begin(), m_sheet.rowSpecs.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    // a row that is hidden was hidden by hand, unless a filter that has conditions covers it (it was the filter that hid it)
    {
        const AutoFilter &f = m_sheet.autoFilter;
        const bool filtering = f.ref.valid && f.active();
        for (RowSpec &spec : m_sheet.rowSpecs) {
            spec.filtered = filtering && spec.hidden && spec.row > f.ref.row1 && spec.row <= f.ref.row2;
            spec.hand = spec.hidden && !spec.filtered;
        }
    }
    // a data bar takes what its extension says (the id ties them), and the rules are in the order of their priority
    for (CondRule &c : m_sheet.conditionalFormats) {
        if (c.type != CondRule::Type::DataBar || c.extId.trimmed().isEmpty()) continue;
        const auto found = extensionBars.constFind(c.extId.trimmed());
        if (found == extensionBars.constEnd()) continue;
        c.minLength = found->minLength;
        c.maxLength = found->maxLength;
        c.gradient = found->gradient;
        c.border = found->border;
        c.negativeSameColor = found->negativeSameColor;
        c.axis = found->axis;
        c.rightToLeft = found->rightToLeft;
        c.negativeColor = found->negativeColor;
        c.borderColor = found->borderColor;
        c.negativeBorderColor = found->negativeBorderColor;
        c.axisColor = found->axisColor;
    }
    std::stable_sort(m_sheet.conditionalFormats.begin(), m_sheet.conditionalFormats.end(), [](const CondRule &a, const CondRule &b) { return a.priority < b.priority; });
    m_sheet.stats.xmlBytes = src.guard.bytesPassed();
    m_sheet.stats.largestToken = src.guard.largestToken();
    m_sheet.stats.largestRead = src.guard.largestRead();
    m_sheet.stats.heldBytes = m_bytes;
    finish();
    return ok;
}

} // namespace

bool readSheetPart(QIODevice *in, const ReadOptions &options, Sheet *out)
{
    *out = Sheet();
    SheetReader reader(options, out);
    // The first screen of a sheet that was left scrolled down: the rows above the view are cut down before the parser sees them.
    if (options.screenRows > 0 && options.skipRows) {
        RowSkipDevice skip(in, options);
        const bool ok = reader.read(&skip);
        // (a failure of the part's own stream is the part's)
        if (!ok && out->error.isEmpty() && !in->errorString().isEmpty()) out->error = in->errorString();
        return ok;
    }
    return reader.read(in);
}

// ---- the workbook

bool Workbook::open(const Package &package, QString *error, const Limits &limits)
{
    *this = Workbook();
    m_limits = limits;
    auto fail = [&](const QString &why) {
        *this = Workbook();
        if (error) *error = why;
        return false;
    };
    const QString main = package.mainPart();
    if (main.isEmpty() || !package.hasPart(main)) return fail(QCoreApplication::translate("Sheet", "The file has no workbook part."));
    if (main.endsWith(QLatin1String(".bin"), Qt::CaseInsensitive) || package.contentType(main).contains(QLatin1String("binary"), Qt::CaseInsensitive))
        return fail(QCoreApplication::translate("Sheet", "The workbook is in the binary format, which this program doesn't read."));
    {
        const auto device = package.openPart(main);
        QString why;
        if (!readWorkbookPart(device.get(), limits, &m_info, &why)) return fail(why);
    }
    // Each sheet has a part of its own. A workbook whose sheets share one would have it read again for each, and a few lines of it would be as much work
    // as there are sheets times the size of the part.
    QSet<QString> parts;
    for (SheetEntry &e : m_info.sheets) {
        const Relationship rel = package.relationship(main, e.relId);
        if (rel.id.isEmpty() || rel.external || !package.hasPart(rel.target)) continue;
        if (parts.contains(rel.target.toLower())) return fail(QCoreApplication::translate("Sheet", "Two sheets of the workbook share one part, which no real file has."));
        parts.insert(rel.target.toLower());
        e.part = rel.target;
        if (rel.type.endsWith(QLatin1String("/chartsheet"))) e.kind = SheetKind::Chartsheet;
        else if (rel.type.endsWith(QLatin1String("/dialogsheet"))) e.kind = SheetKind::Dialogsheet;
        else if (rel.type.endsWith(QLatin1String("acrosheet"))) e.kind = SheetKind::Macrosheet;
    }
    m_package = package;
    m_mainPart = main;
    prepareSheetList();
    const QVector<Relationship> strings = package.relationshipsOfType(main, QStringLiteral("/sharedStrings"));
    for (const Relationship &rel : strings) {
        if (rel.external || !package.hasPart(rel.target)) continue;
        const auto device = package.openPart(rel.target);
        readSharedStrings(device.get(), limits, &m_shared, &m_sharedError);
        m_sharedPart = rel.target;
        m_sharedInFile = m_shared.strings.size();
        break;
    }
    // The styles: a part that is damaged keeps the formats read before the damage (stylesError says what it was) and the workbook open.
    const QVector<Relationship> styles = package.relationshipsOfType(main, QStringLiteral("/styles"));
    for (const Relationship &rel : styles) {
        if (rel.external || !package.hasPart(rel.target)) continue;
        const auto device = package.openPart(rel.target);
        readStyles(device.get(), limits, &m_styles, &m_stylesError);
        m_stylesPart = rel.target;
        m_formatsInFile = m_styles.formats.size();
        m_fontsInFile = m_styles.fonts.size();
        m_fillsInFile = m_styles.fills.size();
        m_bordersInFile = m_styles.borders.size();
        m_styleXfsInFile = m_styles.styleXfs.size();
        m_dxfsInFile = m_styles.dxfs.size();
        break;
    }
    // The theme: the colors the styles name by place. A part that is missing or can't be read leaves the usual ones.
    const QVector<Relationship> themes = package.relationshipsOfType(main, QStringLiteral("/theme"));
    for (const Relationship &rel : themes) {
        if (rel.external || !package.hasPart(rel.target)) continue;
        Theme theme;
        if (!readTheme(package.part(rel.target), &theme)) break;
        // (the styles count light 1 and dark 1, then light 2 and dark 2: the other way round from the theme's own order)
        static const Theme::Color order[kThemeColors] = {Theme::Light1, Theme::Dark1, Theme::Light2, Theme::Dark2, Theme::Accent1, Theme::Accent2,
                                                         Theme::Accent3, Theme::Accent4, Theme::Accent5, Theme::Accent6, Theme::Hyperlink, Theme::FollowedHyperlink};
        for (int i = 0; i < kThemeColors; ++i)
            if (theme.colors[order[i]].isValid()) m_styles.theme[i] = theme.colors[order[i]].rgb() & 0xFFFFFF;
        m_styles.majorFont = theme.majorLatin;
        m_styles.minorFont = theme.minorLatin;
        break;
    }
    return true;
}

bool Workbook::openStream(const QByteArray &stream, bool hasMacros, QString *error, const Limits &limits)
{
    *this = Workbook();
    m_limits = limits;
    const auto xls = XlsBook::open(stream, hasMacros, limits, &m_info, &m_shared, &m_styles, error);
    if (!xls) {
        *this = Workbook();
        return false;
    }
    m_xls = xls;
    nameBaseParts();
    return true;
}

void Workbook::nameBaseParts()
{
    // The workbook is saved as a package of XML parts that is written from what was read (see xlsxbase.h): its parts are named from the start, with the strings and cell formats it was
    // read with already in the file, so that what an edit adds goes after them.
    m_mainPart = baseMainPart();
    m_sharedPart = baseSharedStringsPart();
    m_stylesPart = baseStylesPart();
    for (int i = 0; i < m_info.sheets.size(); ++i) {
        m_info.sheets[i].part = baseSheetPart(i);
        m_info.sheets[i].relId = QStringLiteral("rId%1").arg(i + 1);
    }
    m_sharedInFile = m_shared.strings.size();
    m_formatsInFile = m_styles.formats.size();
    prepareSheetList();
}

// The list of sheets as the file has it is also the list as it is now: the names are the file's, and the tabs are in the file's order.
void Workbook::prepareSheetList()
{
    m_info.order.clear();
    m_info.fileOrder.clear();
    for (int i = 0; i < m_info.sheets.size(); ++i) {
        m_info.sheets[i].fileName = m_info.sheets[i].name;
        m_info.order.append(i);
        m_info.fileOrder.append(i);
    }
}

int Workbook::activeTabNow() const
{
    const int place = m_info.activeTab >= 0 && m_info.activeTab < m_info.fileOrder.size() ? m_info.fileOrder[m_info.activeTab] : -1;
    if (place >= 0 && place < m_info.sheets.size() && !m_info.sheets[place].deleted && m_info.sheets[place].state == SheetState::Visible) {
        const int at = int(m_info.order.indexOf(place));
        if (at >= 0) return at;
    }
    for (int i = 0; i < m_info.order.size(); ++i)
        if (m_info.sheets[m_info.order[i]].state == SheetState::Visible) return i;
    return 0;
}

int Workbook::addSheetEntry(const QString &name, SheetState state)
{
    SheetEntry e;
    e.name = name;
    e.state = state;
    e.isNew = true;
    int id = 0;
    for (const SheetEntry &s : m_info.sheets) id = std::max(id, s.sheetId);
    e.sheetId = id + 1;
    m_info.sheets.append(e);
    return int(m_info.sheets.size() - 1);
}

void Workbook::removeLastSheetEntry()
{
    if (m_info.sheets.isEmpty()) return;
    const int last = int(m_info.sheets.size() - 1);
    m_info.order.removeAll(last);
    m_info.sheets.removeLast();
}

void Workbook::setSheetName(int sheet, const QString &name)
{
    if (sheet >= 0 && sheet < m_info.sheets.size()) m_info.sheets[sheet].name = name;
}

void Workbook::setSheetState(int sheet, SheetState state)
{
    if (sheet >= 0 && sheet < m_info.sheets.size()) m_info.sheets[sheet].state = state;
}

void Workbook::setSheetDeleted(int sheet, bool deleted)
{
    if (sheet >= 0 && sheet < m_info.sheets.size()) m_info.sheets[sheet].deleted = deleted;
}

namespace {

QString numberedPart(const QString &pattern, int n) { return pattern.arg(n); }

} // namespace

void Workbook::planSheetPart(int sheet)
{
    if (sheet < 0 || sheet >= m_info.sheets.size() || !m_info.sheets[sheet].part.isEmpty()) return;
    QSet<QString> taken;
    for (const SheetEntry &e : m_info.sheets) taken.insert(e.part.toLower());
    for (const QString &p : m_package.partNames()) taken.insert(p.toLower());
    int n = 1;
    while (taken.contains(numberedPart(QStringLiteral("/xl/worksheets/sheet%1.xml"), n).toLower())) ++n;
    m_info.sheets[sheet].part = numberedPart(QStringLiteral("/xl/worksheets/sheet%1.xml"), n);
    QSet<QString> ids;
    for (const Relationship &r : m_package.relationships(m_mainPart)) ids.insert(r.id.toLower());
    for (const SheetEntry &e : m_info.sheets) ids.insert(e.relId.toLower());
    int k = 1;
    if (isBinary()) {
        // (the package made from a workbook of the older form numbers its sheets, then the styles and the strings)
        int original = 0;
        while (original < m_info.sheets.size() && !m_info.sheets[original].isNew) ++original;
        k = original + 3;
    }
    while (ids.contains(QStringLiteral("rid%1").arg(k))) ++k;
    m_info.sheets[sheet].relId = QStringLiteral("rId%1").arg(k);
}

namespace {

QString tooLargeMessage() { return QCoreApplication::translate("jp::office::doc", "The file is larger than this program can open."); }

// Whether a compound file holds a macro project, by whether the streams a project has are there: one function for the two ways a file is opened.
bool holdsMacroProject(const std::function<bool(const QString &)> &hasStream)
{
    static const char *const paths[] = {"_VBA_PROJECT_CUR/VBA/dir", "_VBA_PROJECT_CUR/VBA/_VBA_PROJECT", "VBA/dir", "VBA/_VBA_PROJECT"};
    for (const char *path : paths)
        if (hasStream(QString::fromLatin1(path))) return true;
    return false;
}

// What a file that is not a workbook of either kind says it is, from its first bytes, in words for the person: a page of the web or an XML spreadsheet saved with the name of a
// workbook; empty if it says neither.
QString plainKindOfText(const QByteArray &bytes)
{
    QByteArray head = bytes.left(1024);
    if (head.startsWith("\xEF\xBB\xBF")) head.remove(0, 3);
    head = head.trimmed().toLower();
    if (head.isEmpty() || head[0] != '<') return QString();
    if (head.startsWith("<html") || head.startsWith("<!doctype html") || head.contains("<html"))
        return QCoreApplication::translate("Sheet", "This file is a web page that was saved with an .xls name. This program can't open that kind of file yet.");
    if (head.contains("<workbook") && head.contains("spreadsheet"))
        return QCoreApplication::translate("Sheet", "This file is a spreadsheet written as plain text and saved with an .xls name. This program can't open that kind of file yet.");
    return QString();
}

// The message for a workbook that asks for a password (an encrypted package, or an encrypted .xls): the same words for both.
QString encryptedMessage() { return QCoreApplication::translate("Sheet", "The workbook is encrypted, and this program can't open an encrypted workbook yet."); }

// What a file that is none of the kinds this program opens says it is, from its first bytes, in words for the person (never empty: a file nothing is known of is said to be that).
QString notAWorkbookMessage(const QByteArray &head, qint64 size)
{
    if (size == 0) return QCoreApplication::translate("Sheet", "The file is empty.");
    auto starts = [&](QByteArrayView magic) { return QByteArrayView(head).startsWith(magic); };
    if (starts("%PDF")) return QCoreApplication::translate("Sheet", "This file is a PDF document, not a workbook.");
    if (starts("\x89PNG") || starts("\xFF\xD8\xFF") || starts("GIF8") || starts(QByteArrayView("II*\0", 4)) || starts(QByteArrayView("MM\0*", 4)))
        return QCoreApplication::translate("Sheet", "This file is a picture, not a workbook.");
    if (starts("{\\rtf")) return QCoreApplication::translate("Sheet", "This file is a text document (.rtf), not a workbook.");
    return QCoreApplication::translate("Sheet", "This file is not a workbook: it does not start like any kind of file this program opens (.xlsx, .xlsm, .xls, .ods, .csv). It may be damaged, or have the wrong name.");
}

// A zip package that could not be read: when it has no directory of its entries to read (the end of a zip is where its directory is, so a copy cut short has none), it is said to be cut
// short; any other message the package gave stays.
QString damagedPackageMessage(const QByteArray &bytes, const QString &why)
{
    QVector<ZipEntry> directory;
    if (!jp::readZipDirectory(bytes, &directory))
        return QCoreApplication::translate("Sheet", "The file is damaged: it starts like a zip package (.xlsx or .xlsm), but its contents can't be read. It may have been cut short when it was downloaded or copied. "
                                                    "If you have the original, copy it again.");
    return why;
}

// A package that is not a workbook, by what its main part is: a text document or a presentation, named so; else empty.
QString otherPackageMessage(const Package &package)
{
    const QString type = package.contentType(package.mainPart());
    if (type.contains(QLatin1String("wordprocessingml"))) return QCoreApplication::translate("Sheet", "This file is a text document (.docx), not a workbook.");
    if (type.contains(QLatin1String("presentationml"))) return QCoreApplication::translate("Sheet", "This file is a presentation (.pptx), not a workbook.");
    return QString();
}

} // namespace

bool Workbook::openBinary(const QByteArray &file, QString *error, const Limits &limits)
{
    if (file.size() > limits.fileBytes) {
        if (error) *error = tooLargeMessage();
        return false;
    }
    if (!startsCompoundFile(file)) {
        // (a stream with no container: the forms before BIFF5, which says so; or not a workbook of this kind at all)
        const bool stream = file.size() >= 4 && quint8(file[0]) == 0x09 && (quint8(file[1]) == 0x00 || quint8(file[1]) == 0x02 || quint8(file[1]) == 0x04 || quint8(file[1]) == 0x08);
        if (!stream) {
            const QString kind = plainKindOfText(file);
            if (!kind.isEmpty()) {
                if (error) *error = kind;
                return false;
            }
        }
        return openStream(file, false, error, limits);
    }
    jp::cfb::File container;
    QString why;
    if (!jp::cfb::read(file, &container, &why)) {
        if (error) *error = QCoreApplication::translate("Sheet", "The file is damaged: %1").arg(why);
        return false;
    }
    QByteArray stream = container.stream(QStringLiteral("Workbook"));
    if (stream.isEmpty()) stream = container.stream(QStringLiteral("Book"));
    if (stream.isEmpty()) {
        if (error) *error = !container.stream(QStringLiteral("EncryptionInfo")).isEmpty() ? encryptedMessage()
                                                                                        : QCoreApplication::translate("Sheet", "This file is a compound file, but it has no workbook in it.");
        return false;
    }
    const bool macros = holdsMacroProject([&](const QString &path) { return !container.stream(path).isEmpty(); });
    return openStream(stream, macros, error, limits);
}

bool Workbook::openFile(const QString &path, QString *error, const Limits &limits)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = file.errorString();
        return false;
    }
    // (a file past the limit is refused before any of it is read)
    if (file.size() > limits.fileBytes) {
        if (error) *error = tooLargeMessage();
        return false;
    }
    const QByteArray head = file.read(16384);
    file.close();
    // (a stream with no container: the forms before BIFF5, and a bare BIFF8 or BIFF5 stream, begin with the record of the start of a workbook)
    const bool raw = head.size() >= 4 && quint8(head[0]) == 0x09 && (quint8(head[1]) == 0x00 || quint8(head[1]) == 0x02 || quint8(head[1]) == 0x04 || quint8(head[1]) == 0x08);
    if (startsCompoundFile(head)) {
        QString why;
        QByteArray stream = jp::cfb::readStream(path, QStringLiteral("Workbook"), &why);
        if (stream.isEmpty()) stream = jp::cfb::readStream(path, QStringLiteral("Book"));
        if (stream.isEmpty()) {
            if (error) {
                if (why.startsWith(QLatin1String("no stream")) || why.isEmpty())
                    *error = !jp::cfb::readStream(path, QStringLiteral("EncryptionInfo")).isEmpty() ? encryptedMessage()
                                                                                                    : QCoreApplication::translate("Sheet", "This file is a compound file, but it has no workbook in it.");
                else
                    *error = QCoreApplication::translate("Sheet", "The file is damaged: %1").arg(why);
            }
            return false;
        }
        const bool macros = holdsMacroProject([&](const QString &stream) { return !jp::cfb::readStream(path, stream).isEmpty(); });
        return openStream(stream, macros, error, limits);
    }
    if (raw) {
        QByteArray whole;
        if (!readWholeFile(path, &whole, error, limits.fileBytes)) return false;
        return openStream(whole, false, error, limits);
    }
    // (a package begins with the signature of its first entry, or of an empty package's end; text that merely begins with the letters PK is not one)
    const bool zip = head.startsWith(QByteArrayView("PK\x03\x04", 4)) || head.startsWith(QByteArrayView("PK\x05\x06", 4)) || head.startsWith(QByteArrayView("PK\x07\x08", 4));
    if (!zip) {
        // (an OpenDocument spreadsheet in its flat XML form, or a text file of delimited values by its name)
        const bool flat = looksLikeFlatOpenDocument(head);
        if (flat || isDelimitedName(path)) {
            QByteArray text;
            if (!readWholeFile(path, &text, error, limits.fileBytes)) return false;
            return flat ? openOpenDocument(text, true, error, limits) : openDelimited(text, path, error, limits);
        }
        const QString kind = plainKindOfText(head);
        if (!kind.isEmpty()) {
            if (error) *error = kind;
            return false;
        }
        // (nothing is known of it: it is said to be what it looks like, and not a damaged workbook)
        if (error) *error = notAWorkbookMessage(head, QFileInfo(path).size());
        return false;
    }
    QByteArray bytes;
    if (!readWholeFile(path, &bytes, error, limits.fileBytes)) return false;
    if (zip && looksLikeOpenDocumentZip(bytes)) return openOpenDocument(bytes, false, error, limits);
    Package package;
    QString why;
    if (!package.readLazy(bytes, &why)) {
        if (error) *error = damagedPackageMessage(bytes, why);
        return false;
    }
    if (!open(package, &why, limits)) {
        if (error) *error = otherPackageMessage(package).isEmpty() ? why : otherPackageMessage(package);
        return false;
    }
    return true;
}

QStringList Workbook::skippedFeatures() const { return m_xls ? m_xls->skipped() : m_foreign ? m_foreign->skipped() : m_tableNotes; }

QStringList Workbook::changedFeatures() const { return m_foreign ? m_foreign->changed() : QStringList(); }

QString Workbook::convertedKind() const { return m_xls ? QStringLiteral(".xls") : m_foreign ? m_foreign->kind() : QString(); }

// ---- changes

qint32 Workbook::addSharedString(const QString &text)
{
    const auto found = m_sharedAddedAt.constFind(text);
    if (found != m_sharedAddedAt.constEnd()) return found.value();
    const qint32 at = qint32(m_shared.strings.size());
    m_shared.strings.append(text);
    m_shared.chars += text.size();
    m_sharedAdded.append(text);
    m_sharedAddedAt.insert(text, at);
    return at;
}

int Workbook::numberFormatId(const QString &code)
{
    for (int id = 0; id <= 58; ++id)
        if (jp::calc::builtinFormatCode(id) == code) return id;
    int highest = 163;
    for (auto it = m_styles.numFmts.constBegin(); it != m_styles.numFmts.constEnd(); ++it) {
        if (it.value() == code) return it.key();
        highest = std::max(highest, it.key());
    }
    const int id = highest + 1;
    m_styles.numFmts.insert(id, code);
    m_styles.chars += code.size();
    m_numFmtsAdded.append({id, code});
    return id;
}

namespace {

quint64 hashOf(const CellFormat &f)
{
    quint64 h = 1469598103934665603ull;
    auto mix = [&h](quint64 v) { h = (h ^ v) * 1099511628211ull; };
    mix(quint64(quint32(f.numFmtId)));
    mix(f.font);
    mix(f.fill);
    mix(f.border);
    mix(quint64(f.horizontal) | quint64(f.vertical) << 8 | quint64(f.wrap) << 16 | quint64(f.shrink) << 17 | quint64(f.locked) << 18 | quint64(f.formulaHidden) << 19 | quint64(f.quotePrefix) << 20
        | quint64(f.indent) << 24 | quint64(f.rotation) << 32 | quint64(f.readingOrder) << 40);
    mix(f.xfId);
    mix(f.parent);
    mix(f.isStyle);
    return h;
}

constexpr qint64 kMaxFontsMade = 100000, kMaxFillsMade = 100000, kMaxBordersMade = 100000, kMaxStylesMade = 100000;

} // namespace

namespace {

quint64 mixed(quint64 h, quint64 v) { return (h ^ v) * 1099511628211ull; }

quint64 hashOf(const ColorSpec &c)
{
    quint64 h = mixed(1469598103934665603ull, quint64(c.kind));
    switch (c.kind) {
    case ColorSpec::Kind::Rgb: return mixed(h, c.rgb);
    case ColorSpec::Kind::Theme: {
        const double tint = c.tint == 0 ? 0.0 : c.tint;   // (-0 and 0 are equal)
        quint64 bits = 0;
        std::memcpy(&bits, &tint, sizeof(bits));
        return mixed(mixed(h, quint64(quint32(c.index))), bits);
    }
    case ColorSpec::Kind::Indexed: return mixed(h, quint64(quint32(c.index)));
    default: return h;
    }
}

quint64 hashOf(const FontSpec &f)
{
    quint64 h = mixed(1469598103934665603ull, quint64(qHash(f.name)));
    quint64 bits = 0;
    const double size = f.size == 0 ? 0.0 : f.size;
    std::memcpy(&bits, &size, sizeof(bits));
    h = mixed(h, bits);
    h = mixed(h, quint64(f.bold) | quint64(f.italic) << 1 | quint64(f.strike) << 2 | quint64(f.underline) << 8 | quint64(f.vertAlign) << 16);
    h = mixed(h, hashOf(f.color));
    h = mixed(h, quint64(quint32(f.family)) | quint64(quint32(f.charset)) << 32);
    return mixed(h, quint64(qHash(f.scheme)));
}

quint64 hashOf(const FillSpec &f)
{
    quint64 h = mixed(1469598103934665603ull, quint64(f.pattern) | quint64(f.gradient) << 8);
    return mixed(mixed(h, hashOf(f.fg)), hashOf(f.bg));
}

quint64 hashOf(const BorderSide &s)
{
    quint64 h = mixed(1469598103934665603ull, quint64(s.style));
    return s.style == BorderStyle::None ? h : mixed(h, hashOf(s.color));   // (a side with no line has no color that matters)
}

quint64 hashOf(const BorderSpec &b)
{
    quint64 h = mixed(1469598103934665603ull, quint64(b.diagonalUp) | quint64(b.diagonalDown) << 1);
    for (const BorderSide *s : {&b.left, &b.right, &b.top, &b.bottom, &b.diagonal}) h = mixed(h, hashOf(*s));
    return h;
}

// The place of the first of the list's items that is equal to `item` (found by its hash; the index is made from the list when first needed), or -1.
template <class T>
int firstEqual(const QVector<T> &list, std::unordered_multimap<quint64, quint32> *index, bool *made, const T &item, qint64 *compared)
{
    if (!*made) {
        index->clear();
        index->reserve(size_t(list.size()) * 2);
        for (int i = 0; i < list.size(); ++i) index->emplace(hashOf(list[i]), quint32(i));
        *made = true;
    }
    const auto range = index->equal_range(hashOf(item));
    quint32 found = 0xFFFFFFFFu;
    for (auto it = range.first; it != range.second; ++it) {
        ++*compared;
        if (list[int(it->second)] == item && it->second < found) found = it->second;
    }
    return found == 0xFFFFFFFFu ? -1 : int(found);
}

} // namespace

quint32 Workbook::fontPlace(const FontSpec &font)
{
    const int found = firstEqual(m_styles.fonts, &m_fontIndex, &m_fontIndexMade, font, &m_placeComparisons);
    if (found >= 0) return quint32(found);
    if (m_styles.fonts.size() >= kMaxFontsMade) {
        m_formatLimitHit = true;
        return 0;
    }
    m_styles.fonts.append(font);
    m_fontsAdded.append(font);
    m_fontIndex.emplace(hashOf(font), quint32(m_styles.fonts.size() - 1));
    return quint32(m_styles.fonts.size() - 1);
}

qint32 Workbook::dxfForColor(const ColorSpec &color, bool font)
{
    if (m_stylesPart.isEmpty() || !color.isSet()) return -1;
    auto colorOf = [&](const DifferentialFormat &d) {
        if (font) return d.fontColor;
        return d.fill.bg.isSet() ? d.fill.bg : d.fill.fg;
    };
    for (qsizetype i = 0; i < m_styles.dxfs.size(); ++i) {
        const DifferentialFormat &d = m_styles.dxfs[i];
        // (a format that is only this color)
        const bool onlyThis = font ? (d.fontColor == color && !d.hasFill && !d.hasBorder && !d.bold && !d.italic && !d.strike && !d.underline && !d.numberFormat)
                                   : (d.hasFill && colorOf(d) == color && !d.fontColor.isSet() && !d.hasBorder && !d.bold && !d.italic && !d.strike && !d.underline && !d.numberFormat);
        if (onlyThis) return qint32(i);
    }
    if (m_styles.dxfs.size() >= 100000) return -1;
    DifferentialFormat d;
    if (font) {
        d.fontColor = color;
    } else {
        d.hasFill = true;
        d.fill.pattern = PatternKind::Solid;
        d.fill.bg = color;
    }
    m_styles.dxfs.append(d);
    m_dxfsAdded.append(d);
    return qint32(m_styles.dxfs.size() - 1);
}

quint32 Workbook::fillPlace(const FillSpec &fill)
{
    const int found = firstEqual(m_styles.fills, &m_fillIndex, &m_fillIndexMade, fill, &m_placeComparisons);
    if (found >= 0) return quint32(found);
    if (m_styles.fills.size() >= kMaxFillsMade) {
        m_formatLimitHit = true;
        return 0;
    }
    m_styles.fills.append(fill);
    m_fillsAdded.append(fill);
    m_fillIndex.emplace(hashOf(fill), quint32(m_styles.fills.size() - 1));
    return quint32(m_styles.fills.size() - 1);
}

quint32 Workbook::borderPlace(const BorderSpec &border)
{
    const int found = firstEqual(m_styles.borders, &m_borderIndex, &m_borderIndexMade, border, &m_placeComparisons);
    if (found >= 0) return quint32(found);
    if (m_styles.borders.size() >= kMaxBordersMade) {
        m_formatLimitHit = true;
        return 0;
    }
    m_styles.borders.append(border);
    m_bordersAdded.append(border);
    m_borderIndex.emplace(hashOf(border), quint32(m_styles.borders.size() - 1));
    return quint32(m_styles.borders.size() - 1);
}

quint32 Workbook::findOrAddFormat(quint32 base, const CellFormat &format)
{
    if (!m_formatIndexMade) {
        m_formatIndex.clear();
        m_formatIndex.reserve(size_t(m_styles.formats.size()) * 2);
        for (int i = 0; i < m_styles.formats.size(); ++i) m_formatIndex.emplace(hashOf(m_styles.formats[i]), quint32(i));
        m_formatIndexMade = true;
    }
    // (a cell format is never made twice: an equal one that the workbook has stands for it, the first of them)
    const auto range = m_formatIndex.equal_range(hashOf(format));
    quint32 found = 0xFFFFFFFFu;
    for (auto it = range.first; it != range.second; ++it)
        if (m_styles.formats[int(it->second)] == format && it->second < found) found = it->second;
    if (found != 0xFFFFFFFFu) return found;
    if (m_styles.formats.size() >= kMaxCellFormatsWritten) {
        m_formatLimitHit = true;
        return base;
    }
    m_styles.formats.append(format);
    m_styles.cellXfs.append(format.numFmtId);
    m_formatsAdded.append({base, format});
    const quint32 at = quint32(m_styles.formats.size() - 1);
    m_formatIndex.emplace(hashOf(format), at);
    return at;
}

quint32 Workbook::formatWithNumberFormat(quint32 base, int numFmtId)
{
    if (m_styles.formats.isEmpty()) return base;
    if (base >= quint32(m_styles.formats.size())) base = 0;
    if (m_styles.formats[int(base)].numFmtId == numFmtId) return base;
    CellFormat f = m_styles.formats[int(base)];
    f.numFmtId = numFmtId;
    return findOrAddFormat(base, f);
}

quint32 Workbook::formatWith(quint32 base, const FormatChange &c)
{
    if (m_styles.formats.isEmpty()) return base;
    if (base >= quint32(m_styles.formats.size())) base = 0;
    CellFormat f = m_styles.formats[int(base)];
    if (c.namedStyle) {
        const quint32 style = *c.namedStyle;
        if (style >= quint32(m_styles.styleXfs.size())) return base;
        const CellFormat &sf = m_styles.styleXfs[int(style)];
        // the look of the style: its number format, font, fill, border, alignment, and protection (the font, fill, and border are places in the workbook's lists, as the style's own are)
        f.numFmtId = sf.numFmtId;
        f.font = sf.font;
        f.fill = sf.fill;
        f.border = sf.border;
        f.horizontal = sf.horizontal;
        f.vertical = sf.vertical;
        f.wrap = sf.wrap;
        f.shrink = sf.shrink;
        f.indent = sf.indent;
        f.rotation = sf.rotation;
        f.readingOrder = sf.readingOrder;
        f.locked = sf.locked;
        f.formulaHidden = sf.formulaHidden;
        f.xfId = style;
    }
    if (c.numberFormat) f.numFmtId = numberFormatId(*c.numberFormat);
    if (c.touchesFont()) {
        FontSpec font = m_styles.font(f.font);
        if (c.fontName && font.name != *c.fontName) {
            font.name = *c.fontName;
            font.scheme.clear();   // (a font chosen by name is no longer the theme's own)
        }
        if (c.fontSize) font.size = *c.fontSize;
        if (c.bold) font.bold = *c.bold;
        if (c.italic) font.italic = *c.italic;
        if (c.strike) font.strike = *c.strike;
        if (c.underline) font.underline = *c.underline;
        if (c.vertAlign) font.vertAlign = *c.vertAlign;
        if (c.fontColor) font.color = *c.fontColor;
        f.font = fontPlace(font);
    }
    if (c.fill) f.fill = fillPlace(*c.fill);
    if (c.touchesBorder()) {
        BorderSpec b = m_styles.border(f.border);
        if (c.left) b.left = *c.left;
        if (c.right) b.right = *c.right;
        if (c.top) b.top = *c.top;
        if (c.bottom) b.bottom = *c.bottom;
        if (c.diagonal) b.diagonal = *c.diagonal;
        if (c.diagonalUp) b.diagonalUp = *c.diagonalUp;
        if (c.diagonalDown) b.diagonalDown = *c.diagonalDown;
        f.border = borderPlace(b);
    }
    if (c.horizontal) f.horizontal = *c.horizontal;
    if (c.vertical) f.vertical = *c.vertical;
    if (c.wrap) f.wrap = *c.wrap;
    if (c.shrink) f.shrink = *c.shrink;
    if (c.indent) f.indent = quint8(std::max(0, std::min(255, *c.indent)));
    if (c.rotation) f.rotation = quint8(std::max(0, std::min(255, *c.rotation)));
    if (c.readingOrder) f.readingOrder = quint8(std::max(0, std::min(2, *c.readingOrder)));
    if (c.locked) f.locked = *c.locked;
    if (c.formulaHidden) f.formulaHidden = *c.formulaHidden;
    if (f == m_styles.formats[int(base)]) return base;
    return findOrAddFormat(base, f);
}

qint32 Workbook::styleNamed(const QString &name)
{
    for (const CellStyleName &n : m_styles.cellStyles)
        if (n.name.compare(name, Qt::CaseInsensitive) == 0) return n.xfId < quint32(m_styles.styleXfs.size()) ? qint32(n.xfId) : -1;
    // (a built-in style the workbook does not have is made, if the file can be given one: it has the lists the styles go in, and room)
    BuiltinStyle def;
    if (!builtinStyle(name, &def) || m_styles.styleXfs.isEmpty() || m_styles.cellStyles.isEmpty() || m_styles.styleXfs.size() >= kMaxStylesMade) return -1;
    FontSpec font = m_styles.font(0);
    if (!def.fontName.isEmpty()) font.name = def.fontName;
    if (!def.scheme.isEmpty()) font.scheme = def.scheme;
    // (a heading is set in the workbook's own heading font, which its theme names)
    if (def.scheme == QLatin1String("major") && !m_styles.majorFont.isEmpty()) font.name = m_styles.majorFont;
    if (def.size > 0) font.size = def.size;
    if (def.bold) font.bold = true;
    if (def.italic) font.italic = true;
    if (def.underline != Underline::None) font.underline = def.underline;
    if (def.fontColor.isSet()) font.color = def.fontColor;
    CellFormat f;
    f.numFmtId = def.numFmtId;
    f.font = fontPlace(font);
    f.fill = fillPlace(def.fill);
    f.border = borderPlace(def.border);
    m_styles.styleXfs.append(f);
    CellStyleName n;
    n.name = def.name;
    n.builtin = def.builtinId;
    n.xfId = quint32(m_styles.styleXfs.size() - 1);
    m_styles.cellStyles.append(n);
    m_stylesAdded.append({f, n});
    return qint32(n.xfId);
}

const QVector<Table> &Workbook::tables() const
{
    if (!m_tablesRead) {
        m_tables = readTables(m_package, m_info, &m_tableNotes);
        m_tablesRead = true;
        m_tablesInFile.clear();
        m_tablePartSet.clear();
        for (const Table &t : m_tables) {
            m_tablesInFile.emplace_back(t.part, t.sheet);
            m_tablePartSet.insert(t.part);
        }
    }
    return m_tables;
}

QVector<CellRange> Workbook::pivotTablesOn(int sheet) const
{
    const auto cached = m_pivots.find(sheet);
    if (cached != m_pivots.end()) return cached->second;
    QVector<CellRange> out;
    if (sheet >= 0 && sheet < m_info.sheets.size() && !m_info.sheets[sheet].part.isEmpty()) {
        const QString sheetPart = m_info.sheets[sheet].part;
        for (const Relationship &rel : m_package.relationshipsOfType(sheetPart, QStringLiteral("/pivotTable"))) {
            if (rel.external) continue;
            const QByteArray bytes = m_package.part(resolvePartName(sheetPart, rel.target));
            CellRange range;
            bool found = false;
            QXmlStreamReader r(bytes);
            while (!r.atEnd() && !found) {
                if (r.readNext() != QXmlStreamReader::StartElement || r.name() != QLatin1String("location")) continue;
                found = parseRangeRef(r.attributes().value(QLatin1String("ref")), &range);
            }
            if (!found) {
                // (not readable: it is taken to be everywhere)
                range.valid = true;
                range.row1 = range.col1 = 0;
                range.row2 = 1048575;
                range.col2 = 16383;
            }
            out.append(range);
        }
    }
    m_pivots[sheet] = out;
    return out;
}

int Workbook::tableHeaderAt(int sheet, quint32 row, quint32 col, int *column) const
{
    const QVector<Table> &all = tables();
    for (int i = 0; i < all.size(); ++i) {
        const Table &t = all[i];
        if (t.sheet != sheet || t.headerRows < 1 || row != t.row1 || col < t.col1 || col > t.col2) continue;
        const int place = int(col - t.col1);
        if (place >= t.columns.size()) continue;
        if (column) *column = place;
        return i;
    }
    return -1;
}

void Workbook::setTableColumnName(int table, int column, const QString &name)
{
    tables();
    if (table < 0 || table >= m_tables.size() || column < 0 || column >= m_tables[table].columns.size()) return;
    Table &t = m_tables[table];
    const QString was = t.columns[column];
    t.columns[column] = name;
    // (the formulas the table's part keeps for its columns follow the name)
    if (was != name)
        for (TableColumnInfo &c : t.info) {
            if (!c.calcFormula.isEmpty()) c.calcFormula = renameColumnInFormula(c.calcFormula, t.name, was, name, true);
            if (!c.totalsFormula.isEmpty()) c.totalsFormula = renameColumnInFormula(c.totalsFormula, t.name, was, name, true);
        }
}

bool Workbook::tablesAllow(const StructEdit &edit, QString *why) const
{
    for (const Table &t : tables()) {
        CellRange where;
        if (tableAfter(edit, t, &where, why) == TableFate::Refused) return false;
    }
    return true;
}

std::vector<TablePlace> Workbook::moveTables(const StructEdit &edit)
{
    std::vector<TablePlace> before;
    tables();
    for (int i = 0; i < m_tables.size(); ++i) {
        Table &t = m_tables[i];
        CellRange where;
        int totals = t.totalsRows;
        if (tableAfter(edit, t, &where, nullptr, &totals) != TableFate::Moved) continue;
        TablePlace place{nullptr, i, t.row1, t.col1, t.row2, t.col2, t.totalsRows};
        const Table was = t;
        // columns inserted inside the table, or deleted from it: the lists of its columns follow
        using K = StructEdit::Kind;
        if (edit.sheet == t.sheet && edit.kind == K::InsertColumns && edit.first > was.col1 && edit.first <= was.col2) {
            const int at = int(edit.first - was.col1);
            for (quint32 k = 0; k < edit.count; ++k) {
                int id = 0;
                for (const TableColumnInfo &c : t.info) id = std::max(id, c.id);
                t.columns.insert(at + int(k), freshTableColumnName(t.columns, int(t.columns.size()) + 1));
                TableColumnInfo ci;
                ci.id = id + 1;
                t.info.insert(at + int(k), ci);
            }
            for (FilterColumn &fc : t.filter.columns)
                if (int(fc.column) >= at) fc.column += edit.count;
            place.whole = std::make_shared<Table>(was);
        } else if (edit.sheet == t.sheet && edit.kind == K::DeleteColumns && edit.first <= was.col2 && edit.first + edit.count - 1 >= was.col1) {
            const int from = int(std::max(edit.first, was.col1) - was.col1), to = int(std::min(edit.first + edit.count - 1, was.col2) - was.col1);
            for (int k = to; k >= from; --k) {
                t.columns.removeAt(k);
                t.info.removeAt(k);
            }
            QVector<FilterColumn> kept;
            for (FilterColumn fc : t.filter.columns) {
                if (int(fc.column) >= from && int(fc.column) <= to) continue;
                if (int(fc.column) > to) fc.column -= quint32(to - from + 1);
                kept.append(fc);
            }
            t.filter.columns = kept;
            place.whole = std::make_shared<Table>(was);
        }
        before.push_back(place);
        t.row1 = where.row1;
        t.col1 = where.col1;
        t.row2 = where.row2;
        t.col2 = where.col2;
        t.totalsRows = totals;
        // a formula a column holds for its cells follows the cells it names (a reference to cells moves with them)
        if (edit.kind != K::Move) {
            QStringList names;
            for (const SheetEntry &se : m_info.sheets) names << se.name;
            for (TableColumnInfo &c : t.info) {
                if (c.calcFormula.isEmpty() || edit.sheet != t.sheet) continue;
                const QString moved = adjustFormula(c.calcFormula, t.sheet, names, edit, nullptr, false);
                if (moved == c.calcFormula) continue;
                if (!before.back().whole) before.back().whole = std::make_shared<Table>(was);
                c.calcFormula = moved;
            }
        }
    }
    return before;
}

void Workbook::putTablesBack(const std::vector<TablePlace> &places)
{
    tables();
    for (const TablePlace &p : places) {
        if (p.table < 0 || p.table >= m_tables.size()) continue;
        Table &t = m_tables[p.table];
        if (p.whole) {
            t = *p.whole;
            continue;
        }
        t.row1 = p.row1;
        t.col1 = p.col1;
        t.row2 = p.row2;
        t.col2 = p.col2;
        t.totalsRows = p.totalsRows;
    }
}

std::map<QString, QByteArray> Workbook::changedTableParts() const
{
    std::map<QString, QByteArray> out;
    if (!m_tablesRead) return out;
    for (const Table &t : m_tables) {
        if (t.fileSignature.isEmpty() || tableSignature(t) == t.fileSignature) continue;
        if (!m_tablePartSet.contains(t.part)) continue;   // (a table made here: addedTables)
        bool ok = false;
        const QByteArray bytes = writeTablePart(t, m_package.part(t.part), &ok);
        if (ok) out[t.part] = bytes;
    }
    return out;
}

void Workbook::setTableAt(int index, const Table &table)
{
    tables();
    if (index >= 0 && index < m_tables.size()) m_tables[index] = table;
}

int Workbook::insertTable(int index, const Table &table)
{
    tables();
    if (index < 0 || index > m_tables.size()) index = int(m_tables.size());
    m_tables.insert(index, table);
    return index;
}

void Workbook::eraseTable(int index)
{
    tables();
    if (index >= 0 && index < m_tables.size()) m_tables.removeAt(index);
}

QString Workbook::freshTablePart() const
{
    tables();
    QSet<QString> taken;
    for (const QString &n : m_package.partNames()) taken.insert(n.toLower());
    for (const Table &t : m_tables) taken.insert(t.part.toLower());
    for (const auto &p : m_tablesInFile) taken.insert(p.first.toLower());
    for (int n = 1;; ++n) {
        const QString part = QStringLiteral("/xl/tables/table%1.xml").arg(n);
        if (!taken.contains(part.toLower())) return part;
    }
}

int Workbook::freshTableId() const
{
    tables();
    int id = 0;
    for (const Table &t : m_tables) id = std::max(id, t.id);
    return id + 1;
}

std::vector<Workbook::TableAdd> Workbook::addedTables() const
{
    std::vector<TableAdd> out;
    if (!m_tablesRead) return out;
    for (const Table &t : m_tables) {
        if (m_tablePartSet.contains(t.part)) continue;
        bool ok = false;
        const QByteArray bytes = writeTablePart(t, t.copyOfPart.isEmpty() ? QByteArray() : m_package.part(t.copyOfPart), &ok, !t.copyOfPart.isEmpty());
        if (ok) out.push_back({t.sheet, t.part, bytes});
    }
    return out;
}

std::vector<Workbook::TableGone> Workbook::removedTables() const
{
    std::vector<TableGone> out;
    if (!m_tablesRead) return out;
    QSet<QString> now;
    for (const Table &t : m_tables) now.insert(t.part);
    for (const auto &p : m_tablesInFile)
        if (!now.contains(p.first)) out.push_back({p.second, p.first});
    return out;
}

void Workbook::savedTo(const Package &package, const SavedCounts &counts)
{
    const int strings = counts.strings, numberFormats = counts.numberFormats, cellFormats = counts.cellFormats;
    m_package = package;
    // (the drawings of header pictures and their images are read again from the file just written)
    {
        QMutexLocker lock(&m_headerImages->mutex);
        m_headerImages->drawings.clear();
        m_headerImages->images.clear();
        m_headerImages->bytes = 0;
    }
    // (the tables stay as they are, with the file's state now what they are; their places are what the steps of Undo name)
    if (m_tablesRead) {
        m_tablesInFile.clear();
        m_tablePartSet.clear();
        for (Table &t : m_tables) {
            m_tablesInFile.emplace_back(t.part, t.sheet);
            m_tablePartSet.insert(t.part);
            t.fileSignature = tableSignature(t);
            t.copyOfPart.clear();
            t.fileColumns = t.columns;
            t.fileRow1 = t.row1;
            t.fileCol1 = t.col1;
            t.fileRow2 = t.row2;
            t.fileCol2 = t.col2;
            t.fileTotalsRows = t.totalsRows;
        }
    }
    m_sharedInFile += strings;
    m_formatsInFile += cellFormats;
    m_fontsInFile += counts.fonts;
    m_fillsInFile += counts.fills;
    m_bordersInFile += counts.borders;
    m_styleXfsInFile += counts.styles;
    m_dxfsInFile += counts.dxfs;
    m_dxfsAdded = m_dxfsAdded.mid(counts.dxfs);
    m_sharedAdded = m_sharedAdded.mid(strings);
    m_numFmtsAdded = m_numFmtsAdded.mid(numberFormats);
    m_formatsAdded = m_formatsAdded.mid(cellFormats);
    m_fontsAdded = m_fontsAdded.mid(counts.fonts);
    m_fillsAdded = m_fillsAdded.mid(counts.fills);
    m_bordersAdded = m_bordersAdded.mid(counts.borders);
    m_stylesAdded = m_stylesAdded.mid(counts.styles);
    m_sharedAddedAt.clear();
    for (int i = 0; i < m_sharedAdded.size(); ++i) m_sharedAddedAt.insert(m_sharedAdded[i], qint32(m_sharedInFile + i));
    // The sheets: the file has them as they are now (in the order of the tabs, by the names they have); one deleted is not in it.
    {
        const int active = activeTabNow();
        for (SheetEntry &e : m_info.sheets) {
            if (e.deleted) {
                e.part.clear();
                e.relId.clear();
                e.fileName.clear();
                continue;
            }
            e.isNew = false;
            e.fileName = e.name;
        }
        m_info.fileOrder = m_info.order;
        m_info.activeTab = active;
        m_info.firstSheet = 0;
    }
}

// The print area and the titles that the names of the workbook give the sheet.
static void applyPrintNames(const WorkbookInfo &info, int index, Sheet *out)
{
    const QString &sheetName = info.sheets[index].name;
    for (const DefinedName &n : info.definedNames) {
        if (n.localSheet != index) continue;
        if (n.name.compare(QLatin1String("_xlnm.Print_Area"), Qt::CaseInsensitive) == 0) readPrintAreaText(n.ref, sheetName, &out->page);
        else if (n.name.compare(QLatin1String("_xlnm.Print_Titles"), Qt::CaseInsensitive) == 0) readPrintTitlesText(n.ref, sheetName, &out->page);
    }
}

bool Workbook::readSheet(int index, Sheet *out, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows) const
{
    *out = Sheet();
    if (index < 0 || index >= m_info.sheets.size()) {
        out->error = QCoreApplication::translate("Sheet", "There is no such sheet.");
        return false;
    }
    if (m_xls || m_foreign) {
        const bool ok = m_xls ? m_xls->readSheet(index, m_limits, progress, out) : m_foreign->readSheet(index, m_limits, progress, stopAfterRow, screenRows, out);
        applyPrintNames(m_info, index, out);
        out->protection.enabled = out->isProtected;   // (the older kinds of file say only whether)
        return ok;
    }
    const SheetEntry &entry = m_info.sheets[index];
    // A sheet of a chart has no cells, nor has one the file names with no part to hold it (a module of macros that is listed as a sheet has no relationship).
    if (entry.kind == SheetKind::Chartsheet && !entry.part.isEmpty()) {
        // (a chart sheet has no cells; what it says of printing is kept: the paper, margins, and header and footer)
        if (const auto device = m_package.openPart(entry.part)) {
            ReadOptions chartOptions;
            chartOptions.limits = m_limits;
            chartOptions.dateSystem = m_info.dateSystem;
            Sheet chart;
            if (readSheetPart(device.get(), chartOptions, &chart)) out->page = chart.page;
        }
        applyPrintNames(m_info, index, out);
        return true;
    }
    if (entry.kind == SheetKind::Chartsheet || entry.relId.isEmpty()) return true;
    if (entry.part.isEmpty()) {
        out->error = QCoreApplication::translate("Sheet", "The sheet's part is not in the file.");
        return false;
    }
    const auto device = m_package.openPart(entry.part);
    if (!device) {
        out->error = QCoreApplication::translate("Sheet", "The sheet's part is not in the file.");
        return false;
    }
    ReadOptions options;
    options.limits = m_limits;
    options.dateSystem = m_info.dateSystem;
    options.progress = progress;
    options.stopAfterRow = stopAfterRow;
    options.screenRows = screenRows;
    const bool ok = readSheetPart(device.get(), options, out);
    // a row that is hidden among the rows of a table that has a filter with conditions was hidden by that filter, not by hand
    for (const Table &t : tables()) {
        if (t.sheet != index || !t.filterButtons || t.headerRows == 0 || t.filter.conditionColumns().isEmpty() || !t.hasData()) continue;
        for (RowSpec &spec : out->rowSpecs)
            if (spec.hidden && spec.row >= t.dataFirst() && qint64(spec.row) <= t.dataLast()) {
                spec.filtered = true;
                spec.hand = false;
            }
    }
    applyPrintNames(m_info, index, out);
    if (!out->page.headerFooter.drawingRel.isEmpty()) readHeaderPictures(entry.part, &out->page.headerFooter);
    if (!out->links.isEmpty()) {
        // the addresses outside the workbook are in the sheet's relationships
        QHash<QString, QString> targets;
        for (const Relationship &rel : m_package.relationships(entry.part))
            if (rel.external && rel.type.endsWith(QLatin1String("/hyperlink"))) targets.insert(rel.id, rel.target);
        QVector<LinkSpec> kept;
        for (LinkSpec &link : out->links) {
            if (!link.relId.isEmpty()) {
                const QString target = targets.value(link.relId);
                if (target.size() > kMaxLinkTarget) continue;
                link.target = target;
                link.relId.clear();
            }
            if (link.target.isEmpty() && link.location.isEmpty()) continue;
            kept.push_back(std::move(link));
        }
        // the links of all the sheets are held to one budget
        qint64 others = 0;
        {
            QMutexLocker locker(&m_linkBudget->lock);
            for (const auto &entry : m_linkBudget->perSheet)
                if (entry.first != index) others += entry.second;
        }
        qint64 mine = 0;
        QVector<LinkSpec> within;
        for (LinkSpec &link : kept) {
            const qint64 cost = qint64(link.target.size() + link.location.size() + link.display.size() + link.tip.size()) * 2 + 64;
            if (others + mine + cost > m_limits.linkBytes) continue;
            mine += cost;
            within.push_back(std::move(link));
        }
        {
            QMutexLocker locker(&m_linkBudget->lock);
            m_linkBudget->perSheet[index] = mine;
        }
        out->links = std::move(within);
    }
    return ok;
}

} // namespace jp::office::sheet
