// A workbook written as an OpenDocument spreadsheet (.ods) the way the other spreadsheet program writes one: a package of content.xml (the sheets, with the styles the cells use as
// automatic styles, every property explicit), styles.xml (the default look), settings.xml (the cell each sheet was left on, frozen panes, the zoom), meta.xml, and the manifest.
// Formulas are written in OpenFormula with their results. The content is written as it is made, in pieces, so a large sheet takes the memory its package does and not the memory its
// XML would; a document is never made past the size the reader accepts.

#include "sheet/oldsave.h"

#include "calc/formula.h"
#include "calc/numfmt.h"
#include "io/zip.h"
#include "sheet/odsformat.h"
#include "sheet/headerfooter.h"
#include "sheet/odsformula.h"
#include "sheet/oldsavecommon.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <map>

namespace jp::office::sheet {

namespace {

std::atomic<qint64> g_coveredWork{0}, g_coveredLookups{0};

// The columns of a row that merged ranges cover, as separate stretches in order: a column is found by halving, never by going through the stretches. Every look at a stretch and every
// question is counted (for a test of cost), here and nowhere else, so that no search can be written beside this one and go uncounted.
class CoveredRuns {
public:
    void set(std::vector<std::pair<quint32, quint32>> stretches)
    {
        std::sort(stretches.begin(), stretches.end());
        m_runs.clear();
        for (const auto &c : stretches) {
            if (!m_runs.empty() && c.first <= m_runs.back().second + 1) m_runs.back().second = std::max(m_runs.back().second, c.second);
            else m_runs.push_back(c);
        }
    }
    const std::vector<std::pair<quint32, quint32>> &runs() const { return m_runs; }
    bool contains(quint32 col) const
    {
        ++g_coveredLookups;
        size_t lo = 0, hi = m_runs.size();   // the first stretch that ends at or after the column
        while (lo < hi) {
            ++g_coveredWork;
            const size_t mid = (lo + hi) / 2;
            if (m_runs[mid].second < col) lo = mid + 1;
            else hi = mid;
        }
        return lo < m_runs.size() && m_runs[lo].first <= col;
    }

private:
    std::vector<std::pair<quint32, quint32>> m_runs;
};

// The XML of the document body as it is made: kept deflated (lightly) in pieces of about a megabyte, so that a very large sheet does not hold its whole XML in memory (the document's
// head is not known until the sheets are done, and must come first), and written out to the file in the same pieces.
class Spool {
public:
    void add(const QByteArray &bytes)
    {
        if (bytes.isEmpty()) return;
        m_raw += bytes.size();
        m_pending += bytes;
        if (m_pending.size() >= (1 << 20)) seal();
    }
    qint64 size() const { return m_raw; }
    // Writes everything to the entry that is being streamed, and lets go of it as it goes.
    void writeTo(jp::ZipWriter *zip)
    {
        seal();
        while (!m_parts.isEmpty()) {
            const QByteArray raw = qUncompress(m_parts.takeFirst());
            zip->write(raw.constData(), raw.size());
        }
        m_raw = 0;
    }

private:
    void seal()
    {
        if (m_pending.isEmpty()) return;
        m_parts.append(qCompress(m_pending, 1));
        m_pending.clear();
    }
    QList<QByteArray> m_parts;
    QByteArray m_pending;
    qint64 m_raw = 0;
};

constexpr qint64 kMaxContentBytes = qint64(700) << 20;

QString esc(const QString &s)
{
    QString out;
    out.reserve(s.size() + 8);
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s[i];
        const ushort u = c.unicode();
        if (u == '&') out += QLatin1String("&amp;");
        else if (u == '<') out += QLatin1String("&lt;");
        else if (u == '>') out += QLatin1String("&gt;");
        else if (u == '"') out += QLatin1String("&quot;");
        else if (u < 0x20 && u != '\t' && u != '\n' && u != '\r') continue;   // (a control character cannot be in a document)
        else if (c.isHighSurrogate()) {
            if (i + 1 < s.size() && s[i + 1].isLowSurrogate()) {
                out += c;
                out += s[++i];
            }
        } else if (c.isLowSurrogate() || u == 0xFFFE || u == 0xFFFF) continue;
        else out += c;
    }
    return out;
}

QString pt(double v) { return QString::number(v, 'f', 3) + QLatin1String("pt"); }

QString rgbText(quint32 rgb) { return QStringLiteral("#%1").arg(rgb & 0xFFFFFF, 6, 16, QLatin1Char('0')).toUpper(); }

// The widest digit of the default font in whole pixels: the same count as the reader's (odsstyles/ods.cpp digitWidthOf), so a column's width in characters comes back as it went.
int digitWidthOf(const FontSpec &f)
{
    const QString n = f.name.toLower();
    auto has = [&](const char *part) { return n.contains(QLatin1String(part)); };
    double factor = 0.55;
    if (has("calibri") || has("carlito")) factor = 0.507;
    else if (has("verdana")) factor = 0.636;
    else if (has("tahoma")) factor = 0.546;
    else if (has("courier") || has("mono")) factor = 0.6;
    else if (has("consolas")) factor = 0.55;
    else if (has("arial") || has("helvetica") || has("liberation sans") || has("arimo")) factor = 0.556;
    else if (has("times") || has("liberation serif") || has("tinos")) factor = 0.5;
    const double em = f.size * 96.0 / 72.0;
    return int(std::min(64L, std::max(3L, std::lround(em * factor))));
}

// A sheet's name as it is written before the dot in a reference.
QString quoteSheet(const QString &name)
{
    bool plain = !name.isEmpty();
    for (const QChar c : name)
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'))) plain = false;
    if (plain && !name.at(0).isDigit()) return name;
    QString q = name;
    q.replace(QLatin1Char('\''), QLatin1String("''"));
    return QLatin1Char('\'') + q + QLatin1Char('\'');
}

const char *kNamespaces =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
    " xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
    " xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\" xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\""
    " xmlns:calcext=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0\" xmlns:tableooo=\"http://openoffice.org/2009/table\""
    " xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\"";

class OdsWriter {
public:
    OdsWriter(const Workbook &book, CopyReport *report, const std::function<bool()> &cancelled) : m_book(book), m_report(report), m_cancelled(cancelled), m_features(book, CopyKind::Ods) {}

    bool run(QByteArray *out, QString *error)
    {
        m_written = oldsave::sheetsToWrite(m_book, kMaxSheets, 255, m_report);
        for (size_t i = 0; i < m_written.size(); ++i) {
            m_placeToWritten.insert(m_written[i].place, int(i));
            m_sheetNames << m_written[i].name;
            m_nameOfFormula.insert(m_written[i].formulaName, m_written[i].name);
        }
        m_digit = digitWidthOf(m_book.styles().font(0));
        jp::ZipWriter zip;
        zip.add(QStringLiteral("mimetype"), QByteArray("application/vnd.oasis.opendocument.spreadsheet"), false);
        // content.xml is written in pieces; its automatic styles are not known until the sheets have been read, so the sheets are written to a part of their own first
        const QString globalNames = names();
        if (!writeBody(error)) return false;
        QByteArray head;
        {
            QString h = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-content") + QLatin1String(kNamespaces) + QStringLiteral(" office:version=\"1.2\">");
            h += QStringLiteral("<office:scripts/>");
            h += fontFaces();
            h += QStringLiteral("<office:automatic-styles>") + m_dataStyles + m_styleXml + QStringLiteral("</office:automatic-styles>");
            h += QStringLiteral("<office:body><office:spreadsheet>");
            h += QStringLiteral("<table:calculation-settings table:case-sensitive=\"false\" table:search-criteria-must-apply-to-whole-cell=\"true\" table:automatic-find-labels=\"false\" table:use-regular-expressions=\"false\" table:use-wildcards=\"true\">");
            h += QStringLiteral("<table:null-date table:value-type=\"date\" table:date-value=\"%1\"/>").arg(m_book.info().dateSystem == jp::calc::DateSystem::Date1904 ? QStringLiteral("1904-01-01") : QStringLiteral("1899-12-30"));
            if (m_book.info().iterate)
                h += QStringLiteral("<table:iteration table:status=\"enable\" table:steps=\"%1\" table:maximum-difference=\"%2\"/>").arg(m_book.info().iterateCount).arg(m_book.info().iterateDelta);
            h += QStringLiteral("</table:calculation-settings>");
            head = h.toUtf8();
        }
        QByteArray tail = globalNames.toUtf8() + "</office:spreadsheet></office:body></office:document-content>";
        zip.beginStream(QStringLiteral("content.xml"), true);
        zip.write(head.constData(), head.size());
        m_body.writeTo(&zip);
        zip.write(tail.constData(), tail.size());
        zip.endStream();
        zip.add(QStringLiteral("styles.xml"), stylesXml().toUtf8(), true);
        zip.add(QStringLiteral("settings.xml"), settingsXml().toUtf8(), true);
        zip.add(QStringLiteral("meta.xml"), (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-meta") + QLatin1String(kNamespaces)
                                              + QStringLiteral(" office:version=\"1.2\"><office:meta><meta:generator>JeffCrunch</meta:generator></office:meta></office:document-meta>")).toUtf8(), true);
        zip.add(QStringLiteral("META-INF/manifest.xml"),
                QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">"
                           "<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\"/>"
                           "<manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/><manifest:file-entry manifest:full-path=\"styles.xml\" manifest:media-type=\"text/xml\"/>"
                           "<manifest:file-entry manifest:full-path=\"settings.xml\" manifest:media-type=\"text/xml\"/><manifest:file-entry manifest:full-path=\"meta.xml\" manifest:media-type=\"text/xml\"/>"
                           "</manifest:manifest>"),
                true);
        *out = zip.finish();
        if (m_report) {
            m_report->notKept = m_features.result();
            if (m_formulasAsValues > 0) m_report->notKept << QCoreApplication::translate("Sheet", "%1 formulas the format cannot hold (they keep the value they came to)").arg(m_formulasAsValues);
            if (m_tablesAsValues > 0) m_report->notKept << QCoreApplication::translate("Sheet", "what-if data tables (their cells keep the numbers they hold)");
            m_report->formulas = m_formulas;
            m_report->formulasAsValues = m_formulasAsValues;
            m_report->cells = m_cells;
            m_report->sheets = qint64(m_written.size());
            m_report->trimmed << m_trimmed;
            m_report->formulaReasons = oldsave::topReasons(m_reasons);
            std::sort(m_report->notKept.begin(), m_report->notKept.end());
        }
        return true;
    }

private:
    const Workbook &m_book;
    CopyReport *m_report;
    std::function<bool()> m_cancelled;
    Spool m_body;
    QSet<quint64> m_arraysAsValues;   // (the first cells of the arrays of this sheet that are written as values)
    int m_sinceLook = 0;
    bool m_stopped = false;
    // Whether the save is to stop: the caller is asked once in a few thousand cells, not only between rows.
    bool stopNow()
    {
        if (!m_cancelled) return false;
        if (m_stopped) return true;
        if (++m_sinceLook < 2048) return false;
        m_sinceLook = 0;
        m_stopped = m_cancelled();
        return m_stopped;
    }
    oldsave::Features m_features;
    std::vector<oldsave::Written> m_written;
    QHash<int, int> m_placeToWritten;
    QStringList m_sheetNames;
    QHash<QString, QString> m_nameOfFormula;
    QStringList m_trimmed;
    QMap<QString, qint64> m_reasons;
    qint64 m_cells = 0, m_formulas = 0, m_formulasAsValues = 0, m_tablesAsValues = 0, m_textTotal = 0;
    bool m_printNames = false;
    int m_digit = 7;
    // the styles
    QString m_dataStyles, m_styleXml;
    QHash<QString, QString> m_dataStyleOfCode;   // the name of the data style for a format code ("" for none)
    QHash<quint32, QString> m_cellStyleOf;
    QHash<QString, QString> m_columnStyleOf, m_rowStyleOf;
    QStringList m_tableStyleXml;
    QStringList m_fonts;
    QHash<QString, int> m_fontSet;
    int m_dataCount = 0, m_badFormats = 0;
    qint64 m_cellsCut = 0;
    struct SheetViewInfo {
        QString name;
        SheetView view;
        int place = -1;
    };
    QVector<SheetViewInfo> m_views;

    void trim(const QString &what) { m_trimmed << what; }

    // ---- styles

    QString fontFaceFor(const QString &name)
    {
        if (!m_fontSet.contains(name)) {
            m_fontSet.insert(name, 1);
            m_fonts << name;
        }
        return name;
    }
    QString fontFaces() const
    {
        QString x = QStringLiteral("<office:font-face-decls>");
        for (const QString &f : m_fonts) x += QStringLiteral("<style:font-face style:name=\"%1\" svg:font-family=\"%1\"/>").arg(esc(f));
        return x + QStringLiteral("</office:font-face-decls>");
    }

    QString dataStyleFor(const QString &code)
    {
        const auto it = m_dataStyleOfCode.constFind(code);
        if (it != m_dataStyleOfCode.cend()) return it.value();
        QString name;
        if (!code.isEmpty() && code != QLatin1String("General")) {
            OdfDataStyles styles;
            const QString base = QStringLiteral("N%1").arg(++m_dataCount);
            if (m_dataCount < 60000 && odfDataStyles(code, base, &styles)) {
                m_dataStyles += styles.xml;
                name = styles.mainName;
            } else {
                ++m_badFormats;
            }
        }
        m_dataStyleOfCode.insert(code, name);
        return name;
    }

    static QString borderText(const BorderSide &s, const QString &color)
    {
        if (s.style == BorderStyle::None) return QStringLiteral("none");
        double width = 0.75;
        QString line = QStringLiteral("solid");
        switch (s.style) {
        case BorderStyle::Hair: width = 0.2; break;
        case BorderStyle::Thin: break;
        case BorderStyle::Medium: width = 1.5; break;
        case BorderStyle::Thick: width = 3; break;
        case BorderStyle::Double: width = 1.5; line = QStringLiteral("double"); break;
        case BorderStyle::Dotted: line = QStringLiteral("dotted"); break;
        case BorderStyle::Dashed: line = QStringLiteral("dashed"); break;
        case BorderStyle::MediumDashed: case BorderStyle::MediumDashDot: case BorderStyle::MediumDashDotDot: width = 1.5; line = QStringLiteral("dashed"); break;
        case BorderStyle::DashDot: case BorderStyle::DashDotDot: case BorderStyle::SlantDashDot: line = QStringLiteral("dashed"); break;
        default: break;
        }
        return QStringLiteral("%1pt %2 %3").arg(width).arg(line, color);
    }

    // Whether a cell format is the look a cell has when it names none: the workbook's usual font, no fill or border, nothing set for the alignment, and General.
    bool isPlain(const CellFormat &f) const
    {
        const Styles &st = m_book.styles();
        return f.font == 0 && st.fill(f.fill).pattern == PatternKind::None && st.border(f.border) == BorderSpec() && f.horizontal == HAlign::General && f.vertical == VAlign::Bottom && !f.wrap && !f.shrink
               && f.indent == 0 && f.rotation == 0 && f.locked && !f.formulaHidden && f.language.isEmpty() && !f.noProof && (f.numFmtId == 0 || st.formatCode(quint32(&f - st.formats.constData()), QLocale::c()) == QLatin1String("General"));
    }

    // The automatic style for a cell format, made once (empty for the plain look).
    QString cellStyleFor(quint32 xf)
    {
        const auto it = m_cellStyleOf.constFind(xf);
        if (it != m_cellStyleOf.cend()) return it.value();
        const Styles &st = m_book.styles();
        const CellFormat &f = st.format(xf);
        if (xf == 0 && isPlain(f)) {
            m_cellStyleOf.insert(xf, QString());
            return QString();
        }
        const QString name = QStringLiteral("ce%1").arg(m_cellStyleOf.size());
        const QString code = st.formatCode(xf, QLocale::c());
        const QString data = dataStyleFor(code);
        QString x = QStringLiteral("<style:style style:name=\"%1\" style:family=\"table-cell\" style:parent-style-name=\"Default\"").arg(name);
        if (!data.isEmpty()) x += QStringLiteral(" style:data-style-name=\"%1\"").arg(data);
        x += QStringLiteral(">");
        // the cell
        const FillSpec &fill = st.fill(f.fill);
        const BorderSpec &b = st.border(f.border);
        QString cell;
        if (fill.pattern != PatternKind::None) {
            bool automatic = false;
            const quint32 rgb = st.resolve(fill.fg, 0xFFFFFFFFu);
            automatic = rgb == 0xFFFFFFFFu;
            if (!automatic) cell += QStringLiteral(" fo:background-color=\"%1\"").arg(rgbText(rgb));
        }
        auto color = [&](const BorderSide &s) {
            const quint32 rgb = st.resolve(s.color, 0xFFFFFFFFu);
            return rgbText(rgb == 0xFFFFFFFFu ? 0 : rgb);
        };
        cell += QStringLiteral(" fo:border-left=\"%1\" fo:border-right=\"%2\" fo:border-top=\"%3\" fo:border-bottom=\"%4\"")
                    .arg(borderText(b.left, color(b.left)), borderText(b.right, color(b.right)), borderText(b.top, color(b.top)), borderText(b.bottom, color(b.bottom)));
        if (b.diagonal.style != BorderStyle::None) {
            if (b.diagonalUp) cell += QStringLiteral(" style:diagonal-bl-tr=\"%1\"").arg(borderText(b.diagonal, color(b.diagonal)));
            if (b.diagonalDown) cell += QStringLiteral(" style:diagonal-tl-br=\"%1\"").arg(borderText(b.diagonal, color(b.diagonal)));
        }
        cell += f.vertical == VAlign::Top || f.vertical == VAlign::Justify ? QStringLiteral(" style:vertical-align=\"top\"")
                : f.vertical == VAlign::Center ? QStringLiteral(" style:vertical-align=\"middle\"") : QStringLiteral(" style:vertical-align=\"bottom\"");
        cell += f.wrap ? QStringLiteral(" fo:wrap-option=\"wrap\"") : QStringLiteral(" fo:wrap-option=\"no-wrap\"");
        if (f.shrink) cell += QStringLiteral(" style:shrink-to-fit=\"true\"");
        if (f.rotation == 255) cell += QStringLiteral(" style:direction=\"ttb\"");
        else if (f.rotation > 0 && f.rotation <= 90) cell += QStringLiteral(" style:rotation-angle=\"%1\" style:rotation-align=\"none\"").arg(f.rotation);
        else if (f.rotation > 90 && f.rotation <= 180) cell += QStringLiteral(" style:rotation-angle=\"%1\" style:rotation-align=\"none\"").arg(450 - f.rotation);
        cell += QStringLiteral(" style:cell-protect=\"%1\"").arg(f.locked ? (f.formulaHidden ? QStringLiteral("protected formula-hidden") : QStringLiteral("protected")) : (f.formulaHidden ? QStringLiteral("formula-hidden") : QStringLiteral("none")));
        cell += f.horizontal == HAlign::General ? QStringLiteral(" style:text-align-source=\"value-type\" style:repeat-content=\"false\"") : QStringLiteral(" style:text-align-source=\"fix\" style:repeat-content=\"false\"");
        x += QStringLiteral("<style:table-cell-properties%1/>").arg(cell);
        // the paragraph
        QString para;
        switch (f.horizontal) {
        case HAlign::Left: case HAlign::Fill: para += QStringLiteral(" fo:text-align=\"start\""); break;
        case HAlign::Center: case HAlign::CenterContinuous: para += QStringLiteral(" fo:text-align=\"center\""); break;
        case HAlign::Right: para += QStringLiteral(" fo:text-align=\"end\""); break;
        case HAlign::Justify: case HAlign::Distributed: para += QStringLiteral(" fo:text-align=\"justify\""); break;
        case HAlign::General: break;
        }
        if (f.indent > 0) para += QStringLiteral(" fo:margin-left=\"%1pt\"").arg(f.indent * 10);
        x += QStringLiteral("<style:paragraph-properties%1/>").arg(para);
        // the text
        const FontSpec &font = st.font(f.font);
        QString fn = st.fontNameOf(font);
        if (fn.isEmpty()) fn = QStringLiteral("Arial");
        fontFaceFor(fn);
        QString text = QStringLiteral(" style:font-name=\"%1\" fo:font-family=\"%1\" fo:font-size=\"%2pt\"").arg(esc(fn)).arg(font.size);
        text += font.bold ? QStringLiteral(" fo:font-weight=\"bold\"") : QStringLiteral(" fo:font-weight=\"normal\"");
        text += font.italic ? QStringLiteral(" fo:font-style=\"italic\"") : QStringLiteral(" fo:font-style=\"normal\"");
        if (font.underline != Underline::None) {
            text += QStringLiteral(" style:text-underline-style=\"solid\" style:text-underline-width=\"auto\" style:text-underline-color=\"font-color\"");
            if (font.underline == Underline::Double || font.underline == Underline::DoubleAccounting) text += QStringLiteral(" style:text-underline-type=\"double\"");
        }
        if (font.strike) text += QStringLiteral(" style:text-line-through-style=\"solid\"");
        if (font.vertAlign == 1) text += QStringLiteral(" style:text-position=\"super 58%\"");
        else if (font.vertAlign == 2) text += QStringLiteral(" style:text-position=\"sub 58%\"");
        const quint32 fc = st.resolve(font.color, 0xFFFFFFFFu);
        if (fc != 0xFFFFFFFFu) text += QStringLiteral(" fo:color=\"%1\"").arg(rgbText(fc));
        // the language of the text, and "zxx" (with no country) for text that is not checked
        if (f.noProof) text += QStringLiteral(" fo:language=\"zxx\" fo:country=\"none\"");
        else if (!f.language.isEmpty()) {
            text += QStringLiteral(" fo:language=\"%1\"").arg(esc(f.language.section(QLatin1Char('-'), 0, 0)));
            if (f.language.contains(QLatin1Char('-'))) text += QStringLiteral(" fo:country=\"%1\"").arg(esc(f.language.section(QLatin1Char('-'), 1, 1)));
        }
        x += QStringLiteral("<style:text-properties%1/>").arg(text);
        x += QStringLiteral("</style:style>");
        m_cellStyleOf.insert(xf, name);
        m_styleXml += x;
        return name;
    }

    QString columnStyleFor(double widthPoints, bool breakBefore = false)
    {
        const QString key = QString::number(widthPoints, 'f', 3) + (breakBefore ? QLatin1String("b") : QLatin1String(""));
        const auto it = m_columnStyleOf.constFind(key);
        if (it != m_columnStyleOf.cend()) return it.value();
        const QString name = QStringLiteral("co%1").arg(m_columnStyleOf.size() + 1);
        m_styleXml += QStringLiteral("<style:style style:name=\"%1\" style:family=\"table-column\"><style:table-column-properties fo:break-before=\"%3\" style:column-width=\"%2pt\"/></style:style>").arg(name, QString::number(widthPoints, 'f', 3), breakBefore ? QStringLiteral("page") : QStringLiteral("auto"));
        m_columnStyleOf.insert(key, name);
        return name;
    }

    QString rowStyleFor(double heightPoints, bool custom, bool breakBefore = false)
    {
        const QString key = QString::number(heightPoints, 'f', 3) + (custom ? QLatin1String("c") : QLatin1String("o")) + (breakBefore ? QLatin1String("b") : QLatin1String(""));
        const auto it = m_rowStyleOf.constFind(key);
        if (it != m_rowStyleOf.cend()) return it.value();
        const QString name = QStringLiteral("ro%1").arg(m_rowStyleOf.size() + 1);
        m_styleXml += QStringLiteral("<style:style style:name=\"%1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"%2pt\" fo:break-before=\"%4\" style:use-optimal-row-height=\"%3\"/></style:style>")
                          .arg(name, QString::number(heightPoints, 'f', 3), custom ? QStringLiteral("false") : QStringLiteral("true"), breakBefore ? QStringLiteral("page") : QStringLiteral("auto"));
        m_rowStyleOf.insert(key, name);
        return name;
    }

    QString stylesXml()
    {
        const Styles &st = m_book.styles();
        const FontSpec &font = st.font(0);
        QString fn = st.fontNameOf(font);
        if (fn.isEmpty()) fn = QStringLiteral("Arial");
        QString x = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-styles") + QLatin1String(kNamespaces) + QStringLiteral(" office:version=\"1.2\">");
        x += QStringLiteral("<office:font-face-decls><style:font-face style:name=\"%1\" svg:font-family=\"%1\"/></office:font-face-decls>").arg(esc(fn));
        x += QStringLiteral("<office:styles><style:default-style style:family=\"table-cell\"><style:table-cell-properties style:decimal-places=\"2\"/><style:paragraph-properties style:tab-stop-distance=\"36pt\"/>"
                            "<style:text-properties style:font-name=\"%1\" fo:font-size=\"%2pt\" fo:language=\"en\" fo:country=\"US\"/></style:default-style><style:style style:name=\"Default\" style:family=\"table-cell\"/>").arg(esc(fn)).arg(font.size);
        x += QStringLiteral("</office:styles><office:automatic-styles>") + m_pageLayouts + QStringLiteral("</office:automatic-styles><office:master-styles>") + m_masterPages + QStringLiteral("</office:master-styles></office:document-styles>");
        return x;
    }

    QString settingsXml() const
    {
        auto item = [](const QString &name, const char *type, const QString &value) { return QStringLiteral("<config:config-item config:name=\"%1\" config:type=\"%2\">%3</config:config-item>").arg(name, QLatin1String(type), esc(value)); };
        QString x = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<office:document-settings") + QLatin1String(kNamespaces)
                    + QStringLiteral(" office:version=\"1.2\"><office:settings><config:config-item-set config:name=\"ooo:view-settings\"><config:config-item-map-indexed config:name=\"Views\"><config:config-item-map-entry>");
        x += item(QStringLiteral("ViewId"), "string", QStringLiteral("view1"));
        x += QStringLiteral("<config:config-item-map-named config:name=\"Tables\">");
        // (the active sheet is numbered among all the book's: the first written at or after it, or the last)
        int active = std::max(0, int(m_views.size()) - 1);
        for (int i = 0; i < m_views.size(); ++i)
            if (m_views[i].place >= m_book.info().activeTab) {
                active = i;
                break;
            }
        for (const SheetViewInfo &v : m_views) {
            x += QStringLiteral("<config:config-item-map-entry config:name=\"%1\">").arg(esc(v.name));
            const SheetView &s = v.view;
            x += item(QStringLiteral("CursorPositionX"), "int", QString::number(s.activeCol));
            x += item(QStringLiteral("CursorPositionY"), "int", QString::number(s.activeRow));
            // (a split that is not held has its bars at a place in pixels, a twentieth of a point being a fifteenth of a pixel; the mode is 1 for it and 2 for held rows and columns)
            const bool cut = !s.frozen && s.split && (s.splitX > 0 || s.splitY > 0);
            x += item(QStringLiteral("HorizontalSplitMode"), "short", cut && s.splitX > 0 ? QStringLiteral("1") : s.frozen && s.frozenCols > 0 ? QStringLiteral("2") : QStringLiteral("0"));
            x += item(QStringLiteral("VerticalSplitMode"), "short", cut && s.splitY > 0 ? QStringLiteral("1") : s.frozen && s.frozenRows > 0 ? QStringLiteral("2") : QStringLiteral("0"));
            x += item(QStringLiteral("HorizontalSplitPosition"), "int", cut ? QString::number(s.splitX / 15) : s.frozen ? QString::number(s.leftCol + s.frozenCols) : QStringLiteral("0"));
            x += item(QStringLiteral("VerticalSplitPosition"), "int", cut ? QString::number(s.splitY / 15) : s.frozen ? QString::number(s.topRow + s.frozenRows) : QStringLiteral("0"));
            x += item(QStringLiteral("ActiveSplitRange"), "short", cut ? (s.splitX > 0 && s.splitY > 0 ? QStringLiteral("3") : s.splitY > 0 ? QStringLiteral("2") : QStringLiteral("1"))
                                                                     : s.frozen ? (s.frozenCols > 0 && s.frozenRows > 0 ? QStringLiteral("3") : s.frozenRows > 0 ? QStringLiteral("2") : QStringLiteral("1")) : QStringLiteral("2"));
            x += item(QStringLiteral("PositionLeft"), "int", QString::number(s.leftCol));
            x += item(QStringLiteral("PositionRight"), "int", QString::number(cut ? s.paneCol : s.frozen ? std::max(s.paneCol, s.leftCol + s.frozenCols) : 0));
            x += item(QStringLiteral("PositionTop"), "int", QString::number(s.topRow));
            x += item(QStringLiteral("PositionBottom"), "int", QString::number(cut ? s.paneRow : s.frozen ? std::max(s.paneRow, s.topRow + s.frozenRows) : 0));
            x += item(QStringLiteral("ZoomType"), "short", QStringLiteral("0"));
            x += item(QStringLiteral("ZoomValue"), "int", QString::number(std::max(10, std::min(400, s.normalZoom > 0 ? s.normalZoom : s.zoom))));
            x += QStringLiteral("</config:config-item-map-entry>");
        }
        x += QStringLiteral("</config:config-item-map-named>");
        x += item(QStringLiteral("ActiveTable"), "string", m_views.isEmpty() ? QString() : m_views[active].name);
        x += item(QStringLiteral("ShowGrid"), "boolean", !m_views.isEmpty() && !m_views[active].view.showGridLines ? QStringLiteral("false") : QStringLiteral("true"));
        x += item(QStringLiteral("ShowZeroValues"), "boolean", !m_views.isEmpty() && !m_views[active].view.showZeros ? QStringLiteral("false") : QStringLiteral("true"));
        x += QStringLiteral("</config:config-item-map-entry></config:config-item-map-indexed></config:config-item-set></office:settings></office:document-settings>");
        return x;
    }

    // ---- page styles

    static QString cm(double inches) { return QString::number(std::max(0.0, inches) * 2.54, 'f', 3) + QLatin1String("cm"); }

    // The paper in inches (width, height) for the format's paper numbers that are in common use; letter for any other.
    static void paperInches(int paper, double w, double h, double *width, double *height)
    {
        struct Paper { int id; double w, h; };
        static const Paper papers[] = {{1, 8.5, 11}, {2, 8.5, 11}, {3, 11, 17}, {5, 8.5, 14}, {7, 7.25, 10.5}, {8, 297 / 25.4, 420 / 25.4}, {9, 210 / 25.4, 297 / 25.4}, {10, 210 / 25.4, 297 / 25.4},
                                       {11, 148 / 25.4, 210 / 25.4}, {12, 250 / 25.4, 353 / 25.4}, {13, 182 / 25.4, 257 / 25.4}, {14, 8.5, 13}, {18, 8.5, 11}, {66, 420 / 25.4, 594 / 25.4}};
        *width = 8.5;
        *height = 11;
        for (const Paper &p : papers)
            if (p.id == paper) {
                *width = p.w;
                *height = p.h;
                return;
            }
        if (w > 0 && h > 0) {
            *width = w / 72.0;
            *height = h / 72.0;
        }
    }

    // One region of a header or footer: the paragraphs of a part, with its fields.
    static QString region(const HfPart &part, const QString &name)
    {
        if (part.isEmpty()) return QString();
        QString x = QStringLiteral("<style:%1>").arg(name);
        for (const HfLine &line : part.lines) {
            x += QStringLiteral("<text:p>");
            for (const HfRun &run : line) {
                switch (run.field) {
                case HfField::None: x += esc(run.text); break;
                case HfField::Page: x += QStringLiteral("<text:page-number>1</text:page-number>"); break;
                case HfField::Pages: x += QStringLiteral("<text:page-count>1</text:page-count>"); break;
                case HfField::Date: x += QStringLiteral("<text:date/>"); break;
                case HfField::Time: x += QStringLiteral("<text:time/>"); break;
                case HfField::File: x += QStringLiteral("<text:file-name text:display=\"name-and-extension\"/>"); break;
                case HfField::Path: x += QStringLiteral("<text:file-name text:display=\"path\"/>"); break;
                case HfField::Sheet: x += QStringLiteral("<text:sheet-name/>"); break;
                case HfField::Picture: break;
                }
            }
            x += QStringLiteral("</text:p>");
        }
        return x + QStringLiteral("</style:%1>").arg(name);
    }

    static QString headerOrFooter(const QString &code, const QString &tag)
    {
        const HfText t = parseHeaderFooter(code);
        if (t.isEmpty()) return QString();
        return QStringLiteral("<style:%1>").arg(tag) + region(t.left, QStringLiteral("region-left")) + region(t.center, QStringLiteral("region-center")) + region(t.right, QStringLiteral("region-right"))
               + QStringLiteral("</style:%1>").arg(tag);
    }

    // The master page (and its page layout) for how a sheet prints, made once for each distinct setup; its name.
    QString masterPageFor(const PageSetup &p)
    {
        double pw = 8.5, ph = 11;
        paperInches(p.paperSize, p.paperWidth, p.paperHeight, &pw, &ph);
        const bool landscape = p.orientation == PageOrientation::Landscape;
        if (landscape) std::swap(pw, ph);
        const HeaderFooter &h = p.headerFooter;
        const QString header = headerOrFooter(h.oddHeader, QStringLiteral("header")), footer = headerOrFooter(h.oddFooter, QStringLiteral("footer"));
        const QString headerFirst = h.differentFirst ? headerOrFooter(h.firstHeader, QStringLiteral("header-first")) : QString(), footerFirst = h.differentFirst ? headerOrFooter(h.firstFooter, QStringLiteral("footer-first")) : QString();
        const QString headerEven = h.differentOddEven ? headerOrFooter(h.evenHeader, QStringLiteral("header-left")) : QString(), footerEven = h.differentOddEven ? headerOrFooter(h.evenFooter, QStringLiteral("footer-left")) : QString();
        const PageMargins &m = p.margins;
        QString layout = QStringLiteral("<style:page-layout-properties fo:page-width=\"%1\" fo:page-height=\"%2\" style:print-orientation=\"%3\"").arg(cm(pw), cm(ph), landscape ? QStringLiteral("landscape") : QStringLiteral("portrait"));
        const bool hasHeader = !header.isEmpty() || !headerFirst.isEmpty() || !headerEven.isEmpty(), hasFooter = !footer.isEmpty() || !footerFirst.isEmpty() || !footerEven.isEmpty();
        layout += QStringLiteral(" fo:margin-top=\"%1\" fo:margin-bottom=\"%2\" fo:margin-left=\"%3\" fo:margin-right=\"%4\"").arg(cm(hasHeader ? m.header : m.top), cm(hasFooter ? m.footer : m.bottom), cm(m.left), cm(m.right));
        if (p.fitToPage) layout += QStringLiteral(" style:scale-to-X=\"%1\" style:scale-to-Y=\"%2\"").arg(p.fitToWidth).arg(p.fitToHeight);
        else if (p.scale != 100) layout += QStringLiteral(" style:scale-to=\"%1%\"").arg(p.scale);
        layout += QStringLiteral(" style:table-centering=\"%1\"").arg(p.centerHorizontally && p.centerVertically ? QStringLiteral("both") : p.centerHorizontally ? QStringLiteral("horizontal") : p.centerVertically ? QStringLiteral("vertical") : QStringLiteral("none"));
        QString print = QStringLiteral("charts drawings objects zero-values");
        if (p.headings) print += QStringLiteral(" headers");
        if (p.gridLines) print += QStringLiteral(" grid");
        if (p.comments != PrintComments::None) print += QStringLiteral(" annotations");
        layout += QStringLiteral(" style:print=\"%1\" style:print-page-order=\"%2\"").arg(print, p.pageOrder == PageOrder::OverThenDown ? QStringLiteral("ltr") : QStringLiteral("ttb"));
        if (p.useFirstPageNumber) layout += QStringLiteral(" style:first-page-number=\"%1\"").arg(p.firstPageNumber);
        layout += QStringLiteral("/>");
        // (the header's height is what the top margin has beyond the header's own: the way the other program reads the margins of a sheet)
        // (a page with no header or footer still names them, empty: the other program gives a page that names none a header of its own height)
        layout += QStringLiteral("<style:header-style><style:header-footer-properties fo:min-height=\"%1\" fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-bottom=\"0cm\"/></style:header-style>").arg(hasHeader ? cm(m.top - m.header) : QStringLiteral("0cm"));
        layout += QStringLiteral("<style:footer-style><style:header-footer-properties fo:min-height=\"%1\" fo:margin-left=\"0cm\" fo:margin-right=\"0cm\" fo:margin-top=\"0cm\"/></style:footer-style>").arg(hasFooter ? cm(m.bottom - m.footer) : QStringLiteral("0cm"));
        QString master = header + (headerEven.isEmpty() && h.differentOddEven ? QStringLiteral("<style:header-left style:display=\"false\"/>") : QString()) + headerEven + headerFirst
                         + footer + (footerEven.isEmpty() && h.differentOddEven ? QStringLiteral("<style:footer-left style:display=\"false\"/>") : QString()) + footerEven + footerFirst;
        if (!h.differentOddEven) master = header + headerFirst + footer + footerFirst;
        const QString key = layout + QLatin1Char('|') + master;
        const auto it = m_masterOf.constFind(key);
        if (it != m_masterOf.cend()) return it.value();
        const int n = m_masterOf.size() + 1;
        const QString name = n == 1 ? QStringLiteral("Default") : QStringLiteral("Master%1").arg(n);
        m_pageLayouts += QStringLiteral("<style:page-layout style:name=\"Mpm%1\">%2</style:page-layout>").arg(n).arg(layout);
        m_masterPages += QStringLiteral("<style:master-page style:name=\"%1\" style:page-layout-name=\"Mpm%2\">%3</style:master-page>").arg(name).arg(n).arg(master);
        m_masterOf.insert(key, name);
        return name;
    }
    QHash<QString, QString> m_masterOf;
    QString m_pageLayouts, m_masterPages;

    // ---- names

    // A reference as the document writes it inside brackets.
    QString odfRef(const jp::calc::RefLit &r, const QString &s1, const QString &s2, bool hasSheet, bool area) const
    {
        auto sheetPart = [&](const QString &name) { return QLatin1Char('$') + quoteSheet(m_nameOfFormula.value(name, name)) + QLatin1Char('.'); };
        auto cell = [&](quint32 row, quint32 col, bool rowAbs, bool colAbs) {
            return QString(colAbs ? QStringLiteral("$") : QString()) + jp::calc::columnLetters(col) + (rowAbs ? QStringLiteral("$") : QString()) + QString::number(row + 1);
        };
        QString out;
        const QString first = hasSheet ? sheetPart(s1) : QStringLiteral(".");
        if (r.flags & jp::calc::kRefWholeCols) {
            out = first + QString(r.flags & jp::calc::kRefCol1Abs ? QStringLiteral("$") : QString()) + jp::calc::columnLetters(r.col1) + QLatin1Char(':') + (hasSheet ? (s2.isEmpty() ? sheetPart(s1) : sheetPart(s2)) : QStringLiteral("."))
                  + QString(r.flags & jp::calc::kRefCol2Abs ? QStringLiteral("$") : QString()) + jp::calc::columnLetters(r.col2);
        } else if (r.flags & jp::calc::kRefWholeRows) {
            out = first + QString(r.flags & jp::calc::kRefRow1Abs ? QStringLiteral("$") : QString()) + QString::number(r.row1 + 1) + QLatin1Char(':') + (hasSheet ? (s2.isEmpty() ? sheetPart(s1) : sheetPart(s2)) : QStringLiteral("."))
                  + QString(r.flags & jp::calc::kRefRow2Abs ? QStringLiteral("$") : QString()) + QString::number(r.row2 + 1);
        } else {
            out = first + cell(r.row1, r.col1, r.flags & jp::calc::kRefRow1Abs, r.flags & jp::calc::kRefCol1Abs);
            if (area || !s2.isEmpty()) out += QLatin1Char(':') + (hasSheet ? (s2.isEmpty() ? sheetPart(s1) : sheetPart(s2)) : QStringLiteral(".")) + cell(r.row2, r.col2, r.flags & jp::calc::kRefRow2Abs, r.flags & jp::calc::kRefCol2Abs);
        }
        return out;
    }

    // The formula in OpenFormula ("of:=..."): references in brackets, arguments joined by semicolons, the names of the standard's functions. False if the text has something it cannot say.
    bool toOpenFormula(const QString &text, QString *out) const
    {
        std::vector<jp::calc::ReferenceAt> refs;
        jp::calc::ParseOptions opts;
        opts.limits = jp::calc::ParseLimits::grid();
        if (!jp::calc::findReferences(text, opts, &refs, false)) return false;
        std::sort(refs.begin(), refs.end(), [](const jp::calc::ReferenceAt &a, const jp::calc::ReferenceAt &b) { return a.start < b.start; });
        QString res;
        size_t next = 0;
        int i = 0;
        // each '(' is a call of a function or a plain parenthesis; a comma in a plain one is the union of references
        QVector<bool> callStack;
        int brace = 0;
        const int n = text.size();
        while (i < n) {
            if (next < refs.size() && refs[next].start == i) {
                const jp::calc::ReferenceAt &r = refs[next++];
                if (r.afterWorkbook) return false;
                res += QLatin1Char('[') + odfRef(r.ref, r.sheet1, r.sheet2, !r.sheet1.isEmpty(), r.ref.row1 != r.ref.row2 || r.ref.col1 != r.ref.col2 || (r.ref.flags & jp::calc::kRefArea)) + QLatin1Char(']');
                i = r.end;
                continue;
            }
            const QChar c = text[i];
            if (c == QLatin1Char('"')) {
                int j = i + 1;
                for (;;) {
                    if (j >= n) return false;
                    if (text[j] == QLatin1Char('"')) {
                        if (j + 1 < n && text[j + 1] == QLatin1Char('"')) {
                            j += 2;
                            continue;
                        }
                        break;
                    }
                    ++j;
                }
                res += text.mid(i, j - i + 1);
                i = j + 1;
            } else if (c == QLatin1Char('\'')) {
                return false;   // (a quoted name that is not a reference: a name of a sheet in a place this cannot follow)
            } else if (c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\')) {
                int j = i;
                while (j < n && (text[j].isLetterOrNumber() || text[j] == QLatin1Char('_') || text[j] == QLatin1Char('.') || text[j] == QLatin1Char('\\'))) ++j;
                const QString word = text.mid(i, j - i);
                if (j < n && text[j] == QLatin1Char('(')) {
                    res += odfFunctionName(word);
                } else if (word.compare(QLatin1String("TRUE"), Qt::CaseInsensitive) == 0 || word.compare(QLatin1String("FALSE"), Qt::CaseInsensitive) == 0) {
                    res += word.toUpper() + QStringLiteral("()");
                } else {
                    res += word;
                }
                i = j;
            } else if (c == QLatin1Char('(')) {
                const bool call = i > 0 && (text[i - 1].isLetterOrNumber() || text[i - 1] == QLatin1Char('_') || text[i - 1] == QLatin1Char('.'));
                callStack.append(call);
                res += c;
                ++i;
            } else if (c == QLatin1Char(')')) {
                if (!callStack.isEmpty()) callStack.removeLast();
                res += c;
                ++i;
            } else if (c == QLatin1Char('{')) {
                ++brace;
                res += c;
                ++i;
            } else if (c == QLatin1Char('}')) {
                --brace;
                res += c;
                ++i;
            } else if (c == QLatin1Char(',')) {
                if (brace > 0) res += QLatin1Char(';');
                else if (!callStack.isEmpty() && !callStack.last()) res += QLatin1Char('~');
                else res += QLatin1Char(';');
                ++i;
            } else if (c == QLatin1Char(';') && brace > 0) {
                res += QLatin1Char('|');
                ++i;
            } else if (c == QLatin1Char(' ')) {
                // a space between two references is their intersection
                int j = i;
                while (j < n && text[j] == QLatin1Char(' ')) ++j;
                const QChar before = res.isEmpty() ? QChar() : res[res.size() - 1];
                if ((before == QLatin1Char(']') || before == QLatin1Char(')')) && j < n && (text[j] == QLatin1Char('[') || next < refs.size())) res += QLatin1Char('!');
                i = j;
            } else if (c == QLatin1Char('[')) {
                return false;
            } else {
                res += c;
                ++i;
            }
        }
        *out = QStringLiteral("of:=") + res;
        return true;
    }

    QString names()
    {
        // the names of the workbook, and those of one sheet (inside its table)
        QString global;
        QHash<int, QString> local;
        for (const DefinedName &d : m_book.info().definedNames) {
            if (d.name.startsWith(QLatin1String("_xlnm."), Qt::CaseInsensitive)) {
                if (d.name.compare(QLatin1String("_xlnm.Print_Titles"), Qt::CaseInsensitive) == 0) m_printNames = true;
                continue;
            }
            int writtenSheet = -1;
            if (d.localSheet >= 0) {
                const auto at = m_placeToWritten.constFind(d.localSheet);
                if (at == m_placeToWritten.cend()) continue;
                writtenSheet = at.value();
            }
            QString xml;
            const QString base = m_sheetNames.isEmpty() ? QString() : QLatin1Char('$') + quoteSheet(m_sheetNames[std::max(0, writtenSheet)]) + QStringLiteral(".$A$1");
            // a name for cells
            std::vector<jp::calc::ReferenceAt> refs;
            jp::calc::ParseOptions opts;
            opts.limits = jp::calc::ParseLimits::grid();
            bool isRange = false;
            if (jp::calc::findReferences(d.ref, opts, &refs, false) && refs.size() == 1 && refs[0].start == 0 && refs[0].end == d.ref.size() && !refs[0].sheet1.isEmpty() && !refs[0].afterWorkbook) {
                const jp::calc::ReferenceAt &r = refs[0];
                QString address = odfRef(r.ref, r.sheet1, r.sheet2, true, true);
                if (r.ref.row1 == r.ref.row2 && r.ref.col1 == r.ref.col2 && r.sheet2.isEmpty()) {
                    // (a cell: the address says the range of one cell, as the other program writes it)
                    address = odfRef(r.ref, r.sheet1, r.sheet2, true, false);
                }
                xml = QStringLiteral("<table:named-range table:name=\"%1\" table:base-cell-address=\"%2\" table:cell-range-address=\"%3\"/>").arg(esc(d.name), esc(base), esc(address));
                isRange = true;
            }
            if (!isRange) {
                QString f;
                if (!toOpenFormula(d.ref, &f)) continue;
                xml = QStringLiteral("<table:named-expression table:name=\"%1\" table:base-cell-address=\"%2\" table:expression=\"%3\"/>").arg(esc(d.name), esc(base), esc(f.mid(4)));
            }
            if (writtenSheet < 0) global += xml;
            else local[writtenSheet] += xml;
        }
        m_localNames = local;
        return global.isEmpty() ? QString() : QStringLiteral("<table:named-expressions>") + global + QStringLiteral("</table:named-expressions>");
    }
    QHash<int, QString> m_localNames;
    bool m_explicitDefault = false;     // the sheet being written gives some columns or rows a look of their own
    QHash<quint64, QString> m_linkOf;   // the address a cell of the sheet being written links to, by row and column

    // ---- the sheets

    bool writeBody(QString *error)
    {
        for (size_t i = 0; i < m_written.size(); ++i) {
            if (m_cancelled && m_cancelled()) {
                *error = QCoreApplication::translate("Sheet", "The save was stopped.");
                return false;
            }
            if (!writeSheet(int(i), &m_body, error)) return false;
            if (m_body.size() > kMaxContentBytes) {
                trim(QCoreApplication::translate("Sheet", "the sheets after %1 (the document would be larger than any program reads)").arg(m_written[i].name));
                break;
            }
        }
        if (m_badFormats > 0) trim(QCoreApplication::translate("Sheet", "%1 number formats OpenDocument can't say (they show as General)").arg(m_badFormats));
        if (m_cellsCut > 0) trim(QCoreApplication::translate("Sheet", "%1 cells past the limit on cells in a copy").arg(m_cellsCut));
        return true;
    }

    bool writeSheet(int index, Spool *spool, QString *error)
    {
        const oldsave::Written &w = m_written[size_t(index)];
        Sheet sheet;
        if (w.place >= 0) {
            m_book.readSheet(w.place, &sheet);
            if (!sheet.error.isEmpty() && sheet.cells.cellCount() == 0 && sheet.error.contains(QLatin1String("not in the file"))) {
                *error = oldsave::sheetNote(w.name, sheet.error);
                return false;
            }
            m_features.note(sheet);
        }
        m_views.append({w.name, sheet.view, w.place});
        const SheetEntry entry = m_book.info().sheets.value(w.place);
        QString tableStyle = QStringLiteral("ta%1").arg(index + 1);
        {
            QString props;
            if (entry.state != SheetState::Visible) props += QStringLiteral(" table:display=\"false\"");
            if (sheet.view.rightToLeft) props += QStringLiteral(" style:writing-mode=\"rl-tb\"");
            if (sheet.tabColor.isSet()) {
                const quint32 rgb = m_book.styles().resolve(sheet.tabColor, 0xFFFFFFFFu);
                if (rgb != 0xFFFFFFFFu) props += QStringLiteral(" tableooo:tab-color=\"%1\"").arg(rgbText(rgb));
            }
            m_styleXml += QStringLiteral("<style:style style:name=\"%1\" style:family=\"table\" style:master-page-name=\"%3\"><style:table-properties%2/></style:style>").arg(tableStyle, props, masterPageFor(sheet.page));
        }
        QString head = QStringLiteral("<table:table table:name=\"%1\" table:style-name=\"%2\"").arg(esc(w.name), tableStyle);
        if (sheet.isProtected) head += QStringLiteral(" table:protected=\"true\"");
        head += QStringLiteral(">");
        // the used area
        qint64 cellsPastCap = 0;
        const std::vector<PlacedCell> cells = oldsave::sortedCells(sheet, std::max<qint64>(0, kMaxCopyCells - m_cells) + 2000000, &cellsPastCap);
        m_cellsCut += cellsPastCap;
        m_linkOf.clear();
        {
            qint64 spread = 0;
            for (const LinkSpec &l : sheet.links) {
                if (!l.range.valid || (l.target.isEmpty() && l.location.isEmpty()) || l.range.cellCount() > 10000 || spread + l.range.cellCount() > 200000) continue;
                spread += l.range.cellCount();
                QString href = l.target;
                if (href.isEmpty()) {
                    // a place in this workbook: "Sheet2!B5" is "#Sheet2.B5"
                    QString loc = l.location;
                    const int bang = int(loc.lastIndexOf(QLatin1Char('!')));
                    if (bang >= 0) loc = loc.left(bang).remove(QLatin1Char('\'')) + QLatin1Char('.') + loc.mid(bang + 1);
                    href = QLatin1Char('#') + loc;
                }
                for (quint32 r = l.range.row1; r <= l.range.row2; ++r)
                    for (quint32 c = l.range.col1; c <= l.range.col2; ++c) m_linkOf.insert((quint64(r) << 20) | c, href);
            }
        }
        quint32 lastCol = 0;
        for (const PlacedCell &pc : cells) lastCol = std::max(lastCol, pc.col);
        // the merged ranges
        std::vector<CellRange> merges;
        qint64 mergesDropped = 0, coveredCells = 0;
        for (const CellRange &m : sheet.merges) {
            if (!m.valid || m.cellCount() < 2) continue;
            if (m.cellCount() > 1000000 || coveredCells + m.cellCount() > 5000000) {
                ++mergesDropped;
                continue;
            }
            coveredCells += m.cellCount();
            merges.push_back(m);
        }
        std::sort(merges.begin(), merges.end(), [](const CellRange &a, const CellRange &b) { return a.row1 != b.row1 ? a.row1 < b.row1 : a.col1 < b.col1; });
        if (mergesDropped > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 merged ranges too large to write").arg(mergesDropped)));
        for (const CellRange &m : merges) lastCol = std::max(lastCol, m.col2);
        // columns
        QString columns;
        {
            QVector<ColumnSpan> cols = sheet.columns;
            std::stable_sort(cols.begin(), cols.end(), [](const ColumnSpan &a, const ColumnSpan &b) { return a.first < b.first; });
            QSet<quint32> columnBreaks;
            for (const PageBreak &b : sheet.page.colBreaks)
                if (b.manual && b.before < 16384) columnBreaks.insert(b.before);
            const double defaultChars = sheet.format.defaultColWidth > 0 ? sheet.format.defaultColWidth : (sheet.format.baseColWidth > 0 ? sheet.format.baseColWidth + 0.71 : 8.43);
            auto widthPt = [&](double chars) { return chars * m_digit * 72.0 / 96.0; };
            for (const ColumnSpan &c : cols) lastCol = std::max<quint32>(lastCol, std::min<quint32>(c.last, 16383));
            quint32 at = 0;
            auto gap = [&](quint32 until) {
                if (until > at) {
                    columns += QStringLiteral("<table:table-column table:style-name=\"%1\" table:number-columns-repeated=\"%2\"/>").arg(columnStyleFor(widthPt(defaultChars))).arg(until - at);
                    at = until;
                }
            };
            for (const ColumnSpan &c : cols) {
                if (c.first < at) continue;
                const quint32 last = std::min<quint32>(c.last, 16383);
                if (c.first > lastCol && c.first >= at && !(c.hidden || c.style)) continue;
                gap(c.first);
                const double chars = c.width >= 0 ? c.width : defaultChars;
                QString x = QStringLiteral("<table:table-column table:style-name=\"%1\"").arg(columnStyleFor(widthPt(chars)));
                if (last > c.first) x += QStringLiteral(" table:number-columns-repeated=\"%1\"").arg(last - c.first + 1);
                if (c.hidden) x += QStringLiteral(" table:visibility=\"collapse\"");
                if (c.style && !cellStyleFor(c.style).isEmpty()) x += QStringLiteral(" table:default-cell-style-name=\"%1\"").arg(cellStyleFor(c.style));
                columns += x + QStringLiteral("/>");
                at = last + 1;
            }
            gap(lastCol + 1);
        }
        // print areas: the ranges the sheet prints (the head of the table is complete with them, and goes out with the columns before the rows)
        {
            QString print;
            if (sheet.page.hasPrintArea()) {
                for (const CellRange &r : sheet.page.printArea) {
                    if (!r.valid || r.row2 > 1048575 || r.col2 > 16383) continue;
                    print += (print.isEmpty() ? QString() : QStringLiteral(" ")) + quoteSheet(w.name) + QLatin1Char('.') + cellName(r.row1, r.col1) + QLatin1Char(':') + quoteSheet(w.name) + QLatin1Char('.') + cellName(r.row2, r.col2);
                }
            }
            if (!print.isEmpty()) head.insert(head.size() - 1, QStringLiteral(" table:print-ranges=\"%1\"").arg(esc(print)));
        }
        spool->add(head.toUtf8() + columns.toUtf8());
        QMap<quint32, RowSpec> specs;
        for (const RowSpec &s : sheet.placedRowSpecs()) specs.insert(s.row, s);
        m_explicitDefault = false;
        for (const ColumnSpan &c : sheet.columns)
            if (c.style && !cellStyleFor(c.style).isEmpty()) m_explicitDefault = true;
        for (const RowSpec &r : specs)
            if (r.customFormat && !cellStyleFor(r.style).isEmpty()) m_explicitDefault = true;
        // the cells of a row, with the merged ranges in it
        std::map<quint64, int> regionOf;
        struct Region {
            CellRange range;
        };
        std::vector<Region> regions;
        qint64 regionCells = 0;
        m_arraysAsValues.clear();
        for (const PlacedCell &pc : cells) {
            if (!(pc.cell.flags & (ArrayFormula | DataTable))) continue;
            const CellRange r = sheet.formulaRange(pc.cell);
            // (one array may have a million cells, and all of a sheet's together two million, as in the .xls copy; past that, or with no range of its own, it is written as the values it has)
            if (!r.valid || r.row1 != pc.row || r.col1 != pc.col || r.cellCount() > 1000000 || regionCells + r.cellCount() > 2000000) {
                if (pc.cell.flags & ArrayFormula) {
                    m_arraysAsValues.insert((quint64(pc.row) << 16) | pc.col);
                    ++m_formulasAsValues;
                    ++m_reasons[QCoreApplication::translate("Sheet", "an array formula with more cells than a copy writes")];
                }
                continue;
            }
            regionCells += r.cellCount();
            regions.push_back({r});
            if (pc.cell.flags & DataTable) {
                ++m_tablesAsValues;
                regions.back().range.valid = false;
                continue;
            }
            for (quint32 rr = r.row1; rr <= r.row2; ++rr)
                for (quint32 cc = r.col1; cc <= r.col2; ++cc) regionOf[(quint64(rr) << 16) | cc] = int(regions.size()) - 1;
        }
        // the rows
        QString rowsXml;
        {
            // the rows that matter: those with cells, with a size, with part of a merged range, or at the top of a page
            QVector<quint32> rowList;
            QSet<quint32> rowBreakSet;
            {
                QSet<quint32> seen;
                for (const PlacedCell &pc : cells)
                    if (seen.size() < 20000000 && !seen.contains(pc.row)) {
                        seen.insert(pc.row);
                        rowList.append(pc.row);
                    }
                for (auto it = specs.cbegin(); it != specs.cend(); ++it)
                    if (!seen.contains(it.key())) {
                        seen.insert(it.key());
                        rowList.append(it.key());
                    }
                for (const CellRange &m : merges)
                    for (quint32 r = m.row1; r <= m.row2; ++r)
                        if (!seen.contains(r)) {
                            seen.insert(r);
                            rowList.append(r);
                        }
                for (const PageBreak &b : sheet.page.rowBreaks)
                    if (b.manual && b.before < 1048576) {
                        rowBreakSet.insert(b.before);
                        if (!seen.contains(b.before)) {
                            seen.insert(b.before);
                            rowList.append(b.before);
                        }
                    }
                std::sort(rowList.begin(), rowList.end());
            }
            // the cells by row
            QHash<quint32, QPair<size_t, size_t>> spanOfRow;
            for (size_t i = 0; i < cells.size();) {
                size_t j = i;
                while (j < cells.size() && cells[j].row == cells[i].row) ++j;
                spanOfRow.insert(cells[i].row, {i, j});
                i = j;
            }
            const double defaultHeight = sheet.format.defaultRowHeight > 0 ? sheet.format.defaultRowHeight : 15;
            quint32 nextRow = 0;
            size_t mergeStart = 0;   // the merges that begin at or before the row being written, in order
            std::vector<size_t> active;
            auto emptyRows = [&](quint32 until) {
                if (until > nextRow) {
                    rowsXml += QStringLiteral("<table:table-row table:style-name=\"%1\" table:number-rows-repeated=\"%2\"><table:table-cell/></table:table-row>").arg(rowStyleFor(defaultHeight, false)).arg(until - nextRow);
                    nextRow = until;
                }
            };
            for (qsizetype ri = 0; ri < rowList.size(); ++ri) {
                if (m_cancelled && ri % 2048 == 0 && m_cancelled()) {
                    *error = QCoreApplication::translate("Sheet", "The save was stopped.");
                    return false;
                }
                const quint32 row = rowList[ri];
                if (rowsXml.size() > (1 << 19)) {
                    spool->add(rowsXml.toUtf8());
                    rowsXml.clear();
                }
                if (spool->size() > kMaxContentBytes) {
                    trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "the rows from %1 on (the document would be larger than any program reads)").arg(row + 1)));
                    break;
                }
                emptyRows(row);
                while (mergeStart < merges.size() && merges[mergeStart].row1 <= row) active.push_back(mergeStart++);
                active.erase(std::remove_if(active.begin(), active.end(), [&](size_t k) { return merges[k].row2 < row; }), active.end());
                // what the row has, in order of column: cells, and covered runs
                struct Item {
                    quint32 col;
                    const PlacedCell *cell;   // null for a covered run
                    quint32 span;             // the columns it covers
                    quint32 rowSpan;          // rows (a merged range's first cell)
                    quint32 colSpan;
                    bool member = false;      // a cell inside an array's matrix, other than its first
                };
                std::vector<Item> items;
                std::vector<std::pair<quint32, quint32>> covered;   // columns covered in this row
                for (size_t k : active) {
                    const CellRange &m = merges[k];
                    if (row == m.row1) {
                        if (m.col1 < m.col2) covered.push_back({m.col1 + 1, m.col2});
                    } else {
                        covered.push_back({m.col1, m.col2});
                    }
                }
                CoveredRuns coveredRuns;
                coveredRuns.set(covered);
                covered = coveredRuns.runs();
                auto isCovered = [&](quint32 col) { return coveredRuns.contains(col); };
                const auto sp = spanOfRow.constFind(row);
                if (sp != spanOfRow.cend())
                    for (size_t k = sp->first; k < sp->second; ++k) {
                        if (stopNow()) {
                            *error = QCoreApplication::translate("Sheet", "The save was stopped.");
                            return false;
                        }
                        const PlacedCell &pc = cells[k];
                        if (pc.col > 16383 || isCovered(pc.col)) continue;
                        if (m_cells >= kMaxCopyCells) {
                            ++m_cellsCut;
                            continue;
                        }
                        const Cell &c = pc.cell;
                        if (c.type == CellType::Empty && !c.hasFormula() && c.style == 0) continue;
                        const auto ro = regionOf.find((quint64(pc.row) << 16) | pc.col);
                        const bool member = ro != regionOf.end() && !(regions[size_t(ro->second)].range.row1 == pc.row && regions[size_t(ro->second)].range.col1 == pc.col);
                        items.push_back({pc.col, &pc, 1, 1, 1, member});
                    }
                for (size_t k : active) {
                    const CellRange &m = merges[k];
                    if (row != m.row1) continue;
                    // the first cell of a merged range: it may have no cell of its own
                    bool found = false;
                    for (Item &it : items)
                        if (it.col == m.col1) {
                            it.rowSpan = m.row2 - m.row1 + 1;
                            it.colSpan = m.col2 - m.col1 + 1;
                            found = true;
                        }
                    if (!found) items.push_back({m.col1, nullptr, 1, m.row2 - m.row1 + 1, m.col2 - m.col1 + 1, false});
                }
                for (const auto &c : covered) items.push_back({c.first, nullptr, 0, 0, c.second - c.first + 1, false});
                std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.col < b.col; });
                // the row
                QString rx = QStringLiteral("<table:table-row");
                const auto spec = specs.constFind(row);
                if (spec != specs.cend()) {
                    const double h = spec->height >= 0 ? spec->height : defaultHeight;
                    rx += QStringLiteral(" table:style-name=\"%1\"").arg(rowStyleFor(h, spec->customHeight, rowBreakSet.contains(row)));
                    if (spec->hidden) rx += QStringLiteral(" table:visibility=\"collapse\"");
                    if (spec->customFormat && !cellStyleFor(spec->style).isEmpty()) rx += QStringLiteral(" table:default-cell-style-name=\"%1\"").arg(cellStyleFor(spec->style));
                } else {
                    rx += QStringLiteral(" table:style-name=\"%1\"").arg(rowStyleFor(defaultHeight, false, rowBreakSet.contains(row)));
                }
                rx += QStringLiteral(">");
                quint32 col = 0;
                for (const Item &it : items) {
                    if (it.col < col) continue;
                    if (it.col > col) {
                        rx += it.col - col == 1 ? QStringLiteral("<table:table-cell/>") : QStringLiteral("<table:table-cell table:number-columns-repeated=\"%1\"/>").arg(it.col - col);
                        col = it.col;
                    }
                    if (!it.cell && it.span == 0) {
                        rx += it.colSpan == 1 ? QStringLiteral("<table:covered-table-cell/>") : QStringLiteral("<table:covered-table-cell table:number-columns-repeated=\"%1\"/>").arg(it.colSpan);
                        col += it.colSpan;
                        continue;
                    }
                    rx += cellXml(sheet, it.cell, it.rowSpan, it.colSpan, it.member);
                    col += 1;
                }
                rx += QStringLiteral("</table:table-row>");
                rowsXml += rx;
                nextRow = row + 1;
            }
        }
        spool->add(rowsXml.toUtf8());
        QString tail;
        const QString localNames = m_localNames.value(index);
        if (!localNames.isEmpty()) tail += QStringLiteral("<table:named-expressions>") + localNames + QStringLiteral("</table:named-expressions>");
        spool->add(tail.toUtf8() + "</table:table>");
        return true;
    }

    // ---- cells

    static QString textParagraphs(const QString &text)
    {
        // each line a paragraph; runs of spaces and tabs as the standard writes them
        QString out;
        QString normal = text;
        normal.replace(QLatin1String("\r\n"), QLatin1String("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));   // (a line break is a new paragraph, whichever way the text had it)
        const QStringList lines = normal.split(QLatin1Char('\n'));
        for (const QString &line0 : lines) {
            QString line = line0;
            line.remove(QLatin1Char('\r'));
            QString p = QStringLiteral("<text:p>");
            int i = 0;
            while (i < line.size()) {
                const QChar c = line[i];
                if (c == QLatin1Char(' ')) {
                    int j = i;
                    while (j < line.size() && line[j] == QLatin1Char(' ')) ++j;
                    const int run = j - i;
                    if (i == 0 || run > 1) {
                        // the first of a run is a plain space unless the run is at the start
                        if (i > 0) {
                            p += QStringLiteral(" ");
                            if (run > 1) p += QStringLiteral("<text:s text:c=\"%1\"/>").arg(run - 1);
                        } else {
                            p += QStringLiteral("<text:s text:c=\"%1\"/>").arg(run);
                        }
                    } else {
                        p += QStringLiteral(" ");
                    }
                    i = j;
                } else if (c == QLatin1Char('\t')) {
                    p += QStringLiteral("<text:tab/>");
                    ++i;
                } else {
                    int j = i;
                    while (j < line.size() && line[j] != QLatin1Char(' ') && line[j] != QLatin1Char('\t')) ++j;
                    p += esc(line.mid(i, j - i));
                    i = j;
                }
            }
            out += p + QStringLiteral("</text:p>");
        }
        return out;
    }

    static QString errorName(int error) { return errorText(CellError(error)); }

    QString shownText(const Cell &c, quint32 xf) const
    {
        const Styles &st = m_book.styles();
        if (c.type == CellType::Bool) return c.number != 0 ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        const QString code = st.formatCode(xf, QLocale::c());
        if (code.isEmpty() || code == QLatin1String("General")) return QString::number(c.number, 'g', 15);
        const jp::calc::NumberFormat f(code);
        if (!f.isValid()) return QString::number(c.number, 'g', 15);
        const jp::calc::Formatted r = f.format(c.number, m_book.info().dateSystem);
        return r.failed ? QString::number(c.number, 'g', 15) : r.text;
    }

    // A cell of the row: `covered` for a cell inside the matrix of an array formula (it holds a value and no formula).
    QString cellXml(const Sheet &sheet, const PlacedCell *pc, quint32 rowSpan, quint32 colSpan, bool covered)
    {
        QString x = covered ? QStringLiteral("<table:covered-table-cell") : QStringLiteral("<table:table-cell");
        if (!pc) {
            if (rowSpan > 1) x += QStringLiteral(" table:number-rows-spanned=\"%1\"").arg(rowSpan);
            if (colSpan > 1) x += QStringLiteral(" table:number-columns-spanned=\"%1\"").arg(colSpan);
            return x + QStringLiteral("/>");
        }
        const Cell &c = pc->cell;
        ++m_cells;
        const QString styleName = cellStyleFor(c.style);
        // (a cell with no style of its own takes the look of its column or row: one that is to be plain says so, where the sheet gives columns or rows a look)
        if (!styleName.isEmpty()) x += QStringLiteral(" table:style-name=\"%1\"").arg(styleName);
        else if (m_explicitDefault) x += QStringLiteral(" table:style-name=\"Default\"");
        if (rowSpan > 1) x += QStringLiteral(" table:number-rows-spanned=\"%1\"").arg(rowSpan);
        if (colSpan > 1) x += QStringLiteral(" table:number-columns-spanned=\"%1\"").arg(colSpan);
        // the formula
        if (covered) {
            // (a cell of an array's matrix has the value, and the formula is the first cell's)
        } else if ((c.flags & ArrayFormula) && m_arraysAsValues.contains((quint64(pc->row) << 16) | pc->col)) {
            // (an array too large for the copy: its value only; counted when it was found)
        } else if (c.hasFormula() && !(c.flags & DataTable)) {
            QString f;
            const QString text = sheet.formulaText(c);
            if (text.isEmpty() || !toOpenFormula(text, &f)) {
                ++m_reasons[text.isEmpty() ? QStringLiteral("no text") : QStringLiteral("a reference or name OpenFormula cannot say")];
                ++m_formulasAsValues;
            } else {
                x += QStringLiteral(" table:formula=\"%1\"").arg(esc(f));
                if (c.flags & ArrayFormula) {
                    const CellRange r = sheet.formulaRange(c);
                    if (r.valid) x += QStringLiteral(" table:number-matrix-columns-spanned=\"%1\" table:number-matrix-rows-spanned=\"%2\"").arg(r.col2 - r.col1 + 1).arg(r.row2 - r.row1 + 1);
                }
                ++m_formulas;
            }
        } else if (c.flags & DataTable) {
            ++m_formulasAsValues;
        }
        // the value
        QString paragraphs;
        switch (c.type) {
        case CellType::Empty: break;
        case CellType::Number:
        case CellType::Date: {
            if (!std::isfinite(c.number)) {
                // (a number that is not one is the error the other form gives it)
                x += QStringLiteral(" office:value-type=\"string\" office:string-value=\"\" calcext:value-type=\"error\"");
                paragraphs = QStringLiteral("<text:p>#NUM!</text:p>");
                break;
            }
            const QString lit = QString::number(c.number, 'g', 17);
            x += QStringLiteral(" office:value-type=\"float\" office:value=\"%1\" calcext:value-type=\"float\"").arg(lit);
            paragraphs = QStringLiteral("<text:p>") + esc(shownText(c, c.style)) + QStringLiteral("</text:p>");
            break;
        }
        case CellType::Bool:
            x += QStringLiteral(" office:value-type=\"boolean\" office:boolean-value=\"%1\" calcext:value-type=\"boolean\"").arg(c.number != 0 ? QStringLiteral("true") : QStringLiteral("false"));
            paragraphs = QStringLiteral("<text:p>") + shownText(c, c.style) + QStringLiteral("</text:p>");
            break;
        case CellType::Error:
            x += QStringLiteral(" office:value-type=\"string\" office:string-value=\"\" calcext:value-type=\"error\"");
            paragraphs = QStringLiteral("<text:p>") + esc(errorName(int(c.number))) + QStringLiteral("</text:p>");
            break;
        case CellType::SharedString:
        case CellType::InlineString:
        case CellType::FormulaString: {
            bool cut = false;
            QString text = oldsave::cutText(oldsave::textOf(sheet, m_book.sharedStrings(), c), &cut);
            if (cut) ++m_textsCut;
            m_textTotal += text.size();
            if (m_textTotal > kMaxCopyTextTotal) text.clear();
            x += QStringLiteral(" office:value-type=\"string\" calcext:value-type=\"string\"");
            paragraphs = textParagraphs(text);
            break;
        }
        }
        if (!covered && !m_linkOf.isEmpty() && (c.type != CellType::Empty)) {
            const auto link = m_linkOf.constFind((quint64(pc->row) << 20) | pc->col);
            if (link != m_linkOf.cend()) {
                // the text of the cell is the text of the link
                const QString open = QStringLiteral("<text:a xlink:href=\"%1\" xlink:type=\"simple\">").arg(esc(link.value()));
                if (paragraphs.startsWith(QLatin1String("<text:p>"))) {
                    paragraphs.replace(QLatin1String("<text:p>"), QStringLiteral("<text:p>") + open);
                    paragraphs.replace(QLatin1String("</text:p>"), QStringLiteral("</text:a></text:p>"));
                }
            }
        }
        const QString closing = covered ? QStringLiteral("</table:covered-table-cell>") : QStringLiteral("</table:table-cell>");
        if (c.type == CellType::Empty) return x + QStringLiteral("/>");
        return x + QStringLiteral(">") + paragraphs + closing;
    }

public:
    qint64 m_textsCut = 0;
    QStringList textsTrimmed() const { return m_textsCut > 0 ? QStringList{QCoreApplication::translate("Sheet", "%1 texts cut to 32,767 characters").arg(m_textsCut)} : QStringList(); }
};

} // namespace

qint64 odsCoveredWorkForTest() { return g_coveredWork.load(); }
qint64 odsCoveredLookupsForTest() { return g_coveredLookups.load(); }

bool saveOds(const Workbook &book, QByteArray *out, CopyReport *report, QString *error, const std::function<bool()> &cancelled)
{
    QString message;
    OdsWriter w(book, report, cancelled);
    const bool ok = w.run(out, &message);
    if (ok && report) report->trimmed << w.textsTrimmed();
    if (error) *error = message;
    return ok && message.isEmpty();
}

} // namespace jp::office::sheet
