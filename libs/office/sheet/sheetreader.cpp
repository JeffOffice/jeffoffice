#include "sheet/sheetreader.h"

#include "calc/numfmt.h"
#include "sheet/rowskip.h"

#include "office/theme.h"
#include "office/xml.h"
#include "io/zipindex.h"
#include "office/xmlstream.h"

#include <QCoreApplication>
#include <QSet>
#include <QXmlStreamReader>
#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

constexpr qint64 kProgressEvery = 4096;
constexpr qint64 kReportEveryBytes = 65536;

QString translate(const char *text) { return QCoreApplication::translate("Sheet", text); }

// The text a file writes a character in the form _xHHHH_ as it stands for ("_x000D_" is a carriage return); an underscore before an "x" is written
// _x005F_, so that what comes after it is not read again.
void decodeEscapes(QString *s)
{
    int at = s->indexOf(QLatin1String("_x"));
    if (at < 0) return;
    QString out;
    out.reserve(s->size());
    int from = 0;
    while (at >= 0) {
        bool ok = false;
        const uint code = at + 7 <= s->size() && (*s)[at + 6] == QLatin1Char('_') ? s->mid(at + 2, 4).toUInt(&ok, 16) : 0;
        // Four hex digits exactly (toUInt would take a sign or a "0x").
        for (int k = 2; ok && k < 6; ++k)
            if (!(*s)[at + k].isLetterOrNumber() || (*s)[at + k].unicode() > 127) ok = false;
        if (ok) {
            out += QStringView(*s).mid(from, at - from);
            out += QChar(ushort(code));
            from = at + 7;
            at = s->indexOf(QLatin1String("_x"), from);
        } else {
            at = s->indexOf(QLatin1String("_x"), at + 2);
        }
    }
    out += QStringView(*s).mid(from);
    *s = out;
}

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
        if (guard.crowded()) return translate("An element of the XML has more attributes than any real file has.");
        if (guard.tooLong()) return translate("A piece of the XML (a text, a tag with its attributes, a comment) is longer than any real file has.");
        if (guard.notUtf8()) return translate("The XML is not UTF-8, which is the only form this program reads.");
        if (guard.doctype()) return translate("The XML has a DOCTYPE, which is not allowed.");
        if (guard.malformed()) return translate("The XML is damaged: it is not written as XML is.");
        if (guard.sourceFailed()) {
            if (const auto *entry = dynamic_cast<const jp::ZipEntryDevice *>(raw)) {
                if (entry->failure() == jp::ZipFailure::TooLarge) return translate("The part is larger than any real file's part, so it is not read.");
                if (entry->failure() == jp::ZipFailure::OverBudget) return translate("The file's parts, read together, come to more than this program reads.");
            }
            return translate("The part is damaged: its data or its checksum is wrong.");
        }
        if (reader.hasError()) return translate("The XML is damaged: %1").arg(reader.errorString());
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
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            return fail(translate("The XML has a DOCTYPE, which is not allowed."));
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) return fail(translate("The XML is nested too deeply."));
            if (depth == 1) {
                if (!isElement(r, "sst")) return fail(translate("The part is not a list of shared strings."));
                rootSeen = true;
            } else if (depth == 2 && isElement(r, "si")) {
                if (out->strings.size() >= limits.sharedStrings) return fail(translate("The file has more shared strings than any real file has."));
                inItem = true;
                text.clear();
            } else if (inItem && !phonetic) {
                // The text of a string is its <t> and the <t> of its runs; the phonetic runs (rPh) are not text.
                if (isElement(r, "rPh") || isElement(r, "phoneticPr")) phonetic = depth;
                else if (isElement(r, "t") && (depth == 3 || depth == 4)) inText = true;
            }
            break;
        case QXmlStreamReader::EndElement:
            if (phonetic == depth) phonetic = 0;
            if (inText && isElement(r, "t")) inText = false;
            if (depth == 2 && inItem) {
                inItem = false;
                decodeEscapes(&text);
                out->chars += text.size();
                if (out->chars > limits.sharedStringChars) return fail(translate("The shared strings have more characters than any real file has."));
                out->strings.append(text);
                text.clear();
            }
            --depth;
            break;
        case QXmlStreamReader::Characters:
            if (inText && !phonetic) {
                text += r.text();
                if (text.size() > limits.stringChars) return fail(translate("A string is longer than any real file has."));
            }
            break;
        default:
            break;
        }
    }
    const QString why = src.problem();
    if (!why.isEmpty()) return fail(why);
    if (!rootSeen) return fail(translate("The part is not a list of shared strings."));
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
    enum class Where { Other, NumFmts, CellXfs, Fonts, Fills, Borders, Colors };
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
            return fail(translate("The XML has a DOCTYPE, which is not allowed."));
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) return fail(translate("The XML is nested too deeply."));
            if (depth == 1) {
                if (!isElement(r, "styleSheet")) return fail(translate("The part is not a list of styles."));
                rootSeen = true;
            } else if (depth == 2) {
                where = isElement(r, "numFmts")   ? Where::NumFmts
                        : isElement(r, "cellXfs") ? Where::CellXfs
                        : isElement(r, "fonts")   ? Where::Fonts
                        : isElement(r, "fills")   ? Where::Fills
                        : isElement(r, "borders") ? Where::Borders
                        : isElement(r, "colors")  ? Where::Colors
                                                  : Where::Other;
            } else if (depth == 3 && where == Where::NumFmts && isElement(r, "numFmt")) {
                if (++numFmtElements > limits.numFmts) return fail(translate("The file has more number formats than any real file has."));
                bool ok = false;
                const int id = attribute(r, "numFmtId").toInt(&ok);
                const QString code = attribute(r, "formatCode").toString();
                out->chars += code.size();
                if (out->chars > limits.numFmtChars) return fail(translate("The number formats have more characters than any real file has."));
                if (ok) out->numFmts.insert(id, code);
            } else if (depth == 3 && where == Where::CellXfs && isElement(r, "xf")) {
                if (out->cellXfs.size() >= limits.cellXfs) return fail(translate("The file has more cell formats than any real file has."));
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
                out->cellXfs.append(f.numFmtId);
                out->formats.append(f);
            } else if (depth == 4 && where == Where::CellXfs && isElement(r, "alignment") && !out->formats.isEmpty()) {
                CellFormat &f = out->formats.last();
                f.horizontal = hAlignOf(attribute(r, "horizontal"));
                f.vertical = vAlignOf(attribute(r, "vertical"));
                f.wrap = parseBool(attribute(r, "wrapText").toString());
                f.shrink = parseBool(attribute(r, "shrinkToFit").toString());
                f.indent = quint8(std::lround(boundedNumber(attribute(r, "indent"), 0, 255, 0)));
                const double turn = boundedNumber(attribute(r, "textRotation"), 0, 255, 0);
                f.rotation = quint8(turn == 255 ? 255 : turn <= 180 ? std::lround(turn) : 0);
            } else if (depth == 4 && where == Where::CellXfs && isElement(r, "protection") && !out->formats.isEmpty()) {
                const QStringView locked = attribute(r, "locked");
                out->formats.last().locked = locked.isEmpty() ? true : parseBool(locked.toString());
            } else if (depth == 3 && where == Where::Fonts && isElement(r, "font")) {
                if (out->fonts.size() >= limits.fonts) return fail(translate("The file has more fonts than any real file has."));
                out->fonts.append(FontSpec());
            } else if (depth == 4 && where == Where::Fonts && !out->fonts.isEmpty()) {
                FontSpec &f = out->fonts.last();
                if (isElement(r, "b")) f.bold = flagOf(r);
                else if (isElement(r, "i")) f.italic = flagOf(r);
                else if (isElement(r, "strike")) f.strike = flagOf(r);
                else if (isElement(r, "sz")) f.size = boundedNumber(attribute(r, "val"), 1, 409.5, 11);
                else if (isElement(r, "name")) f.name = attribute(r, "val").left(kMaxFontNameChars).toString();
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
                if (out->fills.size() >= limits.fills) return fail(translate("The file has more fills than any real file has."));
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
                if (out->borders.size() >= limits.borders) return fail(translate("The file has more borders than any real file has."));
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
    if (!rootSeen) return fail(translate("The part is not a list of styles."));
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
            return fail(translate("The XML has a DOCTYPE, which is not allowed."));
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) return fail(translate("The XML is nested too deeply."));
            if (depth == 1) {
                if (!isElement(r, "workbook")) return fail(translate("The part is not a workbook."));
                rootSeen = true;
            } else if (depth == 2 && isElement(r, "workbookPr")) {
                out->dateSystem = parseBool(attribute(r, "date1904").toString()) ? DateSystem::Date1904 : DateSystem::Date1900;
            } else if (depth == 2 && isElement(r, "calcPr")) {
                const QStringView mode = attribute(r, "calcMode");
                out->calcMode = mode == QLatin1String("manual") ? CalcMode::Manual : mode == QLatin1String("autoNoTable") ? CalcMode::AutoNoTable : CalcMode::Auto;
                out->fullCalcOnLoad = parseBool(attribute(r, "fullCalcOnLoad").toString());
                out->iterate = parseBool(attribute(r, "iterate").toString());
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
                if (out->definedNames.size() >= limits.definedNames) return fail(translate("The workbook has more names than any real file has."));
                name = DefinedName();
                name.name = attribute(r, "name").toString();
                bool ok = false;
                const int local = attribute(r, "localSheetId").toInt(&ok);
                name.localSheet = ok ? local : -1;
                name.hidden = parseBool(attribute(r, "hidden").toString());
                nameChars += name.name.size();
                if (nameChars > limits.definedNameChars) return fail(translate("The names have more characters than any real file has."));
                inName = true;
            } else if (depth == 2 && isElement(r, "sheets")) {
                inSheets = true;
            } else if (depth == 3 && inSheets && isElement(r, "sheet")) {
                if (out->sheets.size() >= limits.sheets) return fail(translate("The workbook has more sheets than any real file has."));
                SheetEntry e;
                e.name = attribute(r, "name").toString();
                if (e.name.size() > kMaxSheetNameChars) return fail(translate("A sheet's name is longer than any real file has."));
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
                if (nameChars > limits.definedNameChars) return fail(translate("The names have more characters than any real file has."));
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
    if (!rootSeen) return fail(translate("The part is not a workbook."));
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
        return m_bytes <= m_limits.sheetBytes || fail(translate("The sheet would take more memory to hold than any real file's does."));
    }
    // What things cost to hold, as measured (the practice workbooks' largest sheet, and sheets made to be the worst case, read by jcrunchtool: the bytes the
    // process held at its peak against what was counted), rounded up. A cell is 24 bytes in a block that grows by doubling; a string is a slot of 24 in a list
    // that grows by doubling, and a block of the allocator's (its own 24-byte header, the characters twice over and a terminator, and 8 for the allocator,
    // to a multiple of 16, 32 at the least); a shared formula waiting for its first cell is an entry in a table of them (a key, a list, and the list's block),
    // and each cell waiting in it 16 bytes.
    static constexpr qint64 kCellBytes = 32, kFormulaBytes = 24, kRangeBytes = 24, kMasterBytes = 48, kWaitingBytes = 128, kWaitingCellBytes = 16, kSlotBytes = 32;
    // a run of columns, a row kept as not the default, and a merged range (each rounded up from what the structure holds)
    static constexpr qint64 kColumnSpanBytes = 48, kRowSpecBytes = 48, kMergeBytes = 32;
    static qint64 stringBytes(qint64 chars) { return kSlotBytes + std::max<qint64>(32, (24 + 2 * chars + 2 + 8 + 15) / 16 * 16); }
    bool startRow(const QXmlStreamReader &r);
    bool startColumns(const QXmlStreamReader &r);
    void startView(const QXmlStreamReader &r);
    void startPane(const QXmlStreamReader &r);
    void startSelection(const QXmlStreamReader &r);
    void endView();
    bool startMerge(const QXmlStreamReader &r);
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
    QString m_type;
    bool m_haveValue = false, m_haveInline = false;
    QString m_value, m_inline, m_formula;
    // the formula in the making
    enum Kind { Normal, Array, Shared, Table } m_kind = Normal;
    qint64 m_si = -1;
    CellRange m_ref;
    bool m_haveRef = false;
    QString m_r1, m_r2;
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
        if (given < 1 || given > qint64(kMaxRows)) return fail(translate("A row is outside the sheet."));
        m_row = quint32(given - 1);
    } else {
        // A row with no number follows the one before.
        if (m_haveRow && m_row + 1 >= kMaxRows) return fail(translate("A row is outside the sheet."));
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
    const bool otherHeight = spec.height >= 0 && std::abs(spec.height - m_sheet.format.defaultRowHeight) > 0.001;
    // (a sheet that hides the rows it does not list needs to know which it lists)
    if (spec.hidden || spec.customFormat || otherHeight || m_sheet.format.zeroHeight) {
        if (m_sheet.stats.rowSpecs >= m_limits.rowSpecs) return fail(translate("The sheet has more rows with their own height or format than any real file has."));
        if (!take(kRowSpecBytes)) return false;
        ++m_sheet.stats.rowSpecs;
        m_sheet.rowSpecs.append(spec);
    }
    return true;
}

bool SheetReader::startColumns(const QXmlStreamReader &r)
{
    bool ok1 = false, ok2 = false;
    const qint64 min = attribute(r, "min").toLongLong(&ok1), max = attribute(r, "max").toLongLong(&ok2);
    if (++m_sheet.stats.columnSpans > m_limits.columnSpans) return fail(translate("The sheet has more runs of columns than any real file has."));
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
    if (c.width < 0 && !c.hidden && c.style == 0) return true;
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
    if (m_sheet.merges.size() >= m_limits.merges) return fail(translate("The sheet has more merged ranges than any real file has."));
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
        m_cell.row = row;
        m_cell.col = quint16(col);
    } else if (present && looksLikeAddress(at)) {
        // An address, but of a place past the sheet's last row or column.
        return fail(translate("A cell's address is outside the sheet."));
    } else {
        // No address, or one that is no address at all (some writers number cells another way): the cell follows the one before.
        if (m_nextCol >= kMaxColumns) return fail(translate("A cell is outside the sheet."));
        m_cell.row = m_row;
        m_cell.col = quint16(m_nextCol);
    }
    m_nextCol = quint32(m_cell.col) + 1;
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
        m_formula = QStringLiteral("TABLE(") + m_r1 + (m_r2.isEmpty() ? QString() : QStringLiteral(",") + m_r2) + QLatin1Char(')');
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
            f.dRow = qint32(qint64(m_cell.row) - qint64(master->row));
            f.dCol = qint32(qint64(m_cell.col) - qint64(master->col));
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
                if (m_sheet.stats.sharedFormulas >= m_limits.sharedFormulas) return fail(translate("The sheet has more shared formulas than any real file has."));
                ++m_sheet.stats.sharedFormulas;
                if (m_haveRef) {
                    if (m_ref.cellCount() > m_limits.sharedRangeCells) return fail(translate("A shared formula covers a range larger than any real file has."));
                    m_sheet.stats.largestSharedRange = std::max(m_sheet.stats.largestSharedRange, m_ref.cellCount());
                }
                if (!take(kMasterBytes)) return false;
                m_masters.insert(m_si, {text, m_cell.row, m_cell.col});
                // The cells that came before the first cell of the formula have it now.
                const auto waiting = m_pending.find(m_si);
                if (waiting != m_pending.end()) {
                    for (const Pending &p : std::as_const(*waiting)) {
                        Formula &g = m_sheet.formulas[p.formula];
                        g.text = text;
                        g.dRow = qint32(qint64(p.row) - qint64(m_cell.row));
                        g.dCol = qint32(qint64(p.col) - qint64(m_cell.col));
                        m_sheet.stats.expandedFormulaChars += length;
                    }
                    m_pending.erase(waiting);
                }
            }
        }
    }
    if (m_sheet.stats.expandedFormulaChars > m_limits.expandedFormulaChars) return fail(translate("The formulas, all together, are longer than any real file has."));
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
            if (m_pending.size() >= m_limits.sharedFormulas) return fail(translate("The sheet has more shared formulas than any real file has."));
            if (!take(kWaitingBytes)) return false;
            waiting = m_pending.insert(m_si, QVector<Pending>());
        } else if (!take(kWaitingCellBytes)) {
            return false;
        }
        waiting->append({m_cell.formula, m_cell.row, m_cell.col});
    }
    return true;
}

bool SheetReader::endCell()
{
    if (m_skipCells) {
        // (a row the view has left above it: its cells are not kept, and not worked out; what a shared formula's first cell tells has been told by now; the read can still be stopped)
        if (++m_skipped % kProgressEvery == 0 && m_options.progress && !m_options.progress(m_cells + m_skipped)) return fail(translate("The read was stopped."));
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

    if (m_cells >= m_limits.cellsPerSheet) return fail(translate("The sheet has more cells than any real file has."));
    if (!take(kCellBytes)) return false;
    ++m_cells;
    if (c.type != CellType::Empty || c.hasFormula() || c.style != 0) {
        m_sheet.extentRows = std::max(m_sheet.extentRows, c.row + 1);
        m_sheet.extentCols = std::max(m_sheet.extentCols, quint32(c.col) + 1);
    }
    m_sheet.cells.add(c);
    if (m_cells % kProgressEvery == 0 && m_options.progress && !m_options.progress(m_cells)) return fail(translate("The read was stopped."));
    return true;
}

void SheetReader::finish() { m_sheet.cells.finish(); }

bool SheetReader::read(QIODevice *in)
{
    XmlSource src(in, m_limits);
    QXmlStreamReader &r = src.reader;
    int depth = 0, phonetic = 0;
    bool rootSeen = false, inData = false, inRow = false, inCell = false, inFormula = false, inValue = false, inInline = false, inText = false, inMerge = false;
    bool inSheetPr = false, inViews = false, inCols = false, inView = false;
    bool ok = true;
    // Whoever asked to be told how far the read has come is told at least once for every so many bytes of the XML, so that a sheet of anything (merged ranges,
    // an extension list) and not only of cells can be stopped.
    qint64 nextReport = kReportEveryBytes;
    while (ok && !m_stop && !r.atEnd()) {
        if (src.guard.bytesPassed() >= nextReport) {
            nextReport = src.guard.bytesPassed() + kReportEveryBytes;
            if (m_options.progress && !m_options.progress(m_cells)) ok = fail(translate("The read was stopped."));
            if (!ok) break;
        }
        switch (r.readNext()) {
        case QXmlStreamReader::DTD:
            ok = fail(translate("The XML has a DOCTYPE, which is not allowed."));
            break;
        case QXmlStreamReader::StartElement:
            if (++depth > kMaxXmlDepth) {
                ok = fail(translate("The XML is nested too deeply."));
            } else if (depth == 1) {
                if (!(isElement(r, "worksheet") || isElement(r, "macrosheet") || isElement(r, "dialogsheet") || isElement(r, "chartsheet")))
                    ok = fail(translate("The part is not a worksheet."));
                rootSeen = true;
            } else if (depth == 2) {
                if (isElement(r, "sheetData")) {
                    inData = true;
                } else if (isElement(r, "dimension")) {
                    parseRangeRef(attribute(r, "ref"), &m_sheet.dimension);
                } else if (isElement(r, "mergeCells")) {
                    inMerge = true;
                } else if (isElement(r, "sheetProtection")) {
                    m_sheet.isProtected = parseBool(attribute(r, "sheet").toString());
                } else if (isElement(r, "sheetPr")) {
                    inSheetPr = true;
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
                }
            } else if (depth == 3 && inMerge && isElement(r, "mergeCell")) {
                ok = startMerge(r);
            } else if (depth == 3 && inCols && isElement(r, "col")) {
                ok = startColumns(r);
            } else if (depth == 3 && inSheetPr && isElement(r, "tabColor")) {
                m_sheet.tabColor = colorOf(r);
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
            if (depth == 5 && inCell) {
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
            } else if (depth == 3 && inRow) {
                inRow = false;
            } else if (depth == 3 && inView) {
                inView = false;
                endView();
            } else if (depth == 2) {
                inData = inMerge = inSheetPr = inViews = inCols = false;
            }
            --depth;
            break;
        case QXmlStreamReader::Characters:
            if (inFormula) {
                m_formula += r.text();
                if (m_formula.size() > m_limits.formulaChars) ok = fail(translate("A formula is longer than any real file has."));
            } else if (inValue) {
                m_value += r.text();
                if (m_value.size() > m_limits.stringChars) ok = fail(translate("A string is longer than any real file has."));
            } else if (inText && !phonetic) {
                m_inline += r.text();
                if (m_inline.size() > m_limits.stringChars) ok = fail(translate("A string is longer than any real file has."));
            }
            break;
        default:
            break;
        }
    }
    if (ok) {
        const QString why = src.problem();
        if (!why.isEmpty()) ok = fail(why);
        else if (!rootSeen) ok = fail(translate("The part is not a worksheet."));
    }
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
    if (main.isEmpty() || !package.hasPart(main)) return fail(translate("The file has no workbook part."));
    if (main.endsWith(QLatin1String(".bin"), Qt::CaseInsensitive) || package.contentType(main).contains(QLatin1String("binary"), Qt::CaseInsensitive))
        return fail(translate("The workbook is in the binary format, which this program doesn't read."));
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
        if (parts.contains(rel.target.toLower())) return fail(translate("Two sheets of the workbook share one part, which no real file has."));
        parts.insert(rel.target.toLower());
        e.part = rel.target;
        if (rel.type.endsWith(QLatin1String("/chartsheet"))) e.kind = SheetKind::Chartsheet;
        else if (rel.type.endsWith(QLatin1String("/dialogsheet"))) e.kind = SheetKind::Dialogsheet;
        else if (rel.type.endsWith(QLatin1String("acrosheet"))) e.kind = SheetKind::Macrosheet;
    }
    m_package = package;
    m_mainPart = main;
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
        break;
    }
    return true;
}

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

quint32 Workbook::formatWithNumberFormat(quint32 base, int numFmtId)
{
    if (m_styles.formats.isEmpty()) return base;
    if (base >= quint32(m_styles.formats.size())) base = 0;
    if (m_styles.formats[int(base)].numFmtId == numFmtId) return base;
    // (one made before for the same cell format and number format stands for it)
    for (int i = 0; i < m_formatsAdded.size(); ++i)
        if (m_formatsAdded[i].base == base && m_formatsAdded[i].numFmtId == numFmtId) return quint32(m_formatsInFile + i);
    if (m_styles.formats.size() >= kMaxCellXfs) return base;
    CellFormat f = m_styles.formats[int(base)];
    f.numFmtId = numFmtId;
    m_styles.formats.append(f);
    m_styles.cellXfs.append(numFmtId);
    m_formatsAdded.append({base, numFmtId});
    return quint32(m_styles.formats.size() - 1);
}

const QVector<Table> &Workbook::tables() const
{
    if (!m_tablesRead) {
        m_tables = readTables(m_package, m_info);
        m_tablesRead = true;
    }
    return m_tables;
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
    m_tables[table].columns[column] = name;
}

std::map<QString, QByteArray> Workbook::changedTableParts() const
{
    std::map<QString, QByteArray> out;
    if (!m_tablesRead) return out;
    for (const Table &t : m_tables) {
        if (t.columns == t.fileColumns) continue;
        QByteArray renamed;
        if (renameTableXml(m_package.part(t.part), t.fileColumns, t.columns, t.name, &renamed)) out[t.part] = renamed;
    }
    return out;
}

void Workbook::savedTo(const Package &package, int strings, int numberFormats, int cellFormats)
{
    m_package = package;
    // (the tables are read again from the file just written, which has the names they have now)
    m_tablesRead = false;
    m_tables.clear();
    m_sharedInFile += strings;
    m_formatsInFile += cellFormats;
    m_sharedAdded = m_sharedAdded.mid(strings);
    m_numFmtsAdded = m_numFmtsAdded.mid(numberFormats);
    m_formatsAdded = m_formatsAdded.mid(cellFormats);
    m_sharedAddedAt.clear();
    for (int i = 0; i < m_sharedAdded.size(); ++i) m_sharedAddedAt.insert(m_sharedAdded[i], qint32(m_sharedInFile + i));
}

bool Workbook::readSheet(int index, Sheet *out, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows) const
{
    *out = Sheet();
    if (index < 0 || index >= m_info.sheets.size()) {
        out->error = translate("There is no such sheet.");
        return false;
    }
    const SheetEntry &entry = m_info.sheets[index];
    // A sheet of a chart has no cells, nor has one the file names with no part to hold it (a module of macros that is listed as a sheet has no relationship).
    if (entry.kind == SheetKind::Chartsheet || entry.relId.isEmpty()) return true;
    if (entry.part.isEmpty()) {
        out->error = translate("The sheet's part is not in the file.");
        return false;
    }
    const auto device = m_package.openPart(entry.part);
    if (!device) {
        out->error = translate("The sheet's part is not in the file.");
        return false;
    }
    ReadOptions options;
    options.limits = m_limits;
    options.dateSystem = m_info.dateSystem;
    options.progress = progress;
    options.stopAfterRow = stopAfterRow;
    options.screenRows = screenRows;
    return readSheetPart(device.get(), options, out);
}

} // namespace jp::office::sheet
