#include "sheet/ods.h"

#include "calc/dates.h"
#include "io/zipindex.h"
#include "office/package.h"
#include "office/xmlstream.h"
#include "sheet/cellref.h"
#include "sheet/odsformula.h"
#include "sheet/odsstyles.h"
#include "sheet/xmlscan.h"

#include <QBuffer>
#include <QDate>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>
#include <vector>

namespace jp::office::sheet {

namespace {

QString say(const char *text) { return QCoreApplication::translate("jp::office::sheet::Ods", text); }

constexpr QLatin1String NS_TABLE("urn:oasis:names:tc:opendocument:xmlns:table:1.0");
constexpr QLatin1String NS_OFFICE("urn:oasis:names:tc:opendocument:xmlns:office:1.0");
constexpr QLatin1String NS_TEXT("urn:oasis:names:tc:opendocument:xmlns:text:1.0");
constexpr QLatin1String NS_STYLE("urn:oasis:names:tc:opendocument:xmlns:style:1.0");
constexpr QLatin1String NS_FO("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
constexpr QLatin1String NS_NUMBER("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0");
constexpr QLatin1String NS_SVG("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
constexpr QLatin1String NS_DRAW("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
constexpr QLatin1String NS_CONFIG("urn:oasis:names:tc:opendocument:xmlns:config:1.0");
constexpr QLatin1String NS_SCRIPT("urn:oasis:names:tc:opendocument:xmlns:script:1.0");
constexpr QLatin1String NS_CALCEXT("urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0");
constexpr QLatin1String NS_TABLEOOO("http://openoffice.org/2009/table");
constexpr QLatin1String NS_LOEXT("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0");

// All the properties (attributes of the elements of style properties) of all the styles together (tens of thousands are found in real documents).
constexpr qint64 kMaxOdsProperties = 4000000;

const char kMimeSpreadsheet[] = "application/vnd.oasis.opendocument.spreadsheet";

QString attr(const QXmlStreamAttributes &a, QLatin1String ns, QLatin1String name) { return a.value(ns, name).toString(); }
bool hasAttr(const QXmlStreamAttributes &a, QLatin1String ns, QLatin1String name) { return a.hasAttribute(ns, name); }

QString guardMessage(const jp::office::XmlGuardDevice &guard)
{
    if (guard.crowded()) return say("An element of the XML has more attributes than any real file has.");
    if (guard.tooLong()) return say("A piece of the XML (a text, a tag with its attributes, a comment) is longer than any real file has.");
    if (guard.notUtf8()) return say("The XML is not UTF-8, which is the only form this program reads.");
    if (guard.doctype()) return say("The XML has a DOCTYPE, which is not allowed.");
    if (guard.malformed()) return say("The XML is damaged: it is not written as XML is.");
    if (guard.sourceFailed()) return say("The file is damaged, or its parts come to more than this program reads.");
    return QString();
}

// ---- what the first pass finds

struct TableInfo {
    QString name;
    QString styleName;
    bool isProtected = false;
};

struct NamedItem {
    QString name;
    QString text;    // the address of a range, or the expression
    bool isRange = false;
    int sheet = -1;
};

struct Found {
    ods::StyleStore store;
    QVector<TableInfo> tables;
    QString tableQName;
    QVector<NamedItem> names;
    QHash<QString, QHash<QString, QString>> sheetViews;   // by sheet name: the items of its view settings
    QHash<QString, QString> viewItems;                    // the view's own (ActiveTable, ShowGrid)
    bool charts = false, drawings = false, comments = false, pivots = false, macros = false, conditional = false, validation = false;
    QString nullDate = QStringLiteral("1899-12-30");
    bool iterate = false;
};

class DocumentParser {
public:
    DocumentParser(Found *found, const Limits &limits) : m_f(*found), m_limits(limits) {}

    bool parse(QIODevice *device, QString *error)
    {
        jp::office::XmlGuardDevice guard(device, kMaxXmlTokenBytes);
        guard.open(QIODevice::ReadOnly);
        QXmlStreamReader xml(&guard);
        while (!xml.atEnd()) {
            switch (xml.readNext()) {
            case QXmlStreamReader::StartElement:
                if (++m_depth > kMaxOdsDepth) return fail(error, say("The XML is nested more deeply than any real file's."));
                if (!start(xml, error)) return false;
                break;
            case QXmlStreamReader::EndElement:
                end(xml);
                --m_depth;
                break;
            case QXmlStreamReader::Characters:
                if (m_capture && m_text.size() < 65536) m_text += xml.text();
                break;
            case QXmlStreamReader::DTD:
            case QXmlStreamReader::EntityReference: return fail(error, say("The XML has a DOCTYPE, which is not allowed."));
            default: break;
            }
        }
        if (guard.refused()) return fail(error, guardMessage(guard));
        if (xml.hasError()) return fail(error, say("The XML is damaged: %1").arg(xml.errorString()));
        return true;
    }

private:
    bool fail(QString *error, const QString &message)
    {
        if (error) *error = message;
        return false;
    }

    bool start(const QXmlStreamReader &x, QString *error)
    {
        const QStringView ns = x.namespaceUri(), name = x.name();
        if (ns == NS_TEXT) return true;
        const QXmlStreamAttributes a = x.attributes();
        if (ns == NS_TABLE) return startTable(x, a, name, error);
        if (ns == NS_STYLE) return startStyle(a, name, error);
        if (ns == NS_NUMBER) {
            startDataStyle(a, name);
            return true;
        }
        if (ns == NS_OFFICE) {
            if (name == QLatin1String("spreadsheet")) m_spreadsheetDepth = m_depth;
            else if (name == QLatin1String("annotation")) m_f.comments = true;
            else if (name == QLatin1String("scripts")) m_inScripts = m_depth;
            else if (name == QLatin1String("font-face-decls")) return true;
            if (m_inScripts && m_depth > m_inScripts) m_f.macros = true;
            return true;
        }
        if (ns == NS_DRAW) {
            if (name == QLatin1String("object") || name == QLatin1String("object-ole")) m_f.charts = true;
            else if (name == QLatin1String("image") || name == QLatin1String("frame") || name == QLatin1String("custom-shape") || name == QLatin1String("rect") || name == QLatin1String("ellipse")
                     || name == QLatin1String("line") || name == QLatin1String("polygon") || name == QLatin1String("polyline") || name == QLatin1String("path") || name == QLatin1String("g")
                     || name == QLatin1String("control") || name == QLatin1String("caption") || name == QLatin1String("circle") || name == QLatin1String("connector"))
                m_f.drawings = true;
            return true;
        }
        if (ns == NS_CONFIG) return startConfig(a, name);
        if (ns == NS_CALCEXT) {
            if (name == QLatin1String("conditional-format") || name == QLatin1String("conditional-formats")) m_f.conditional = true;
            return true;
        }
        if (ns == NS_SCRIPT) m_f.macros = true;
        return true;
    }

    bool startTable(const QXmlStreamReader &x, const QXmlStreamAttributes &a, QStringView name, QString *error)
    {
        if (name == QLatin1String("table-cell") || name == QLatin1String("table-row") || name == QLatin1String("covered-table-cell") || name == QLatin1String("table-column")) return true;
        if (name == QLatin1String("table")) {
            if (m_spreadsheetDepth == 0 || m_depth != m_spreadsheetDepth + 1) return true;
            if (m_f.tables.size() >= m_limits.sheets) return fail(error, say("The workbook has more sheets than any real file has."));
            TableInfo t;
            t.name = attr(a, NS_TABLE, QLatin1String("name"));
            t.styleName = attr(a, NS_TABLE, QLatin1String("style-name"));
            t.isProtected = attr(a, NS_TABLE, QLatin1String("protected")) == QLatin1String("true");
            if (m_f.tables.isEmpty()) m_f.tableQName = x.qualifiedName().toString();
            m_f.tables.append(t);
            m_inTable = true;
        } else if (name == QLatin1String("named-range") || name == QLatin1String("named-expression")) {
            if (m_f.names.size() >= m_limits.definedNames) return fail(error, say("The workbook has more names than any real file has."));
            NamedItem n;
            n.name = attr(a, NS_TABLE, QLatin1String("name"));
            n.isRange = name == QLatin1String("named-range");
            n.text = n.isRange ? attr(a, NS_TABLE, QLatin1String("cell-range-address")) : attr(a, NS_TABLE, QLatin1String("expression"));
            n.sheet = m_inTable ? int(m_f.tables.size()) - 1 : -1;
            if (!n.name.isEmpty()) m_f.names.append(n);
        } else if (name == QLatin1String("null-date")) {
            const QString d = attr(a, NS_TABLE, QLatin1String("date-value"));
            if (!d.isEmpty()) m_f.nullDate = d;
        } else if (name == QLatin1String("iteration")) {
            m_f.iterate = attr(a, NS_TABLE, QLatin1String("status")) == QLatin1String("enable");
        } else if (name == QLatin1String("content-validation") || name == QLatin1String("content-validations")) {
            m_f.validation = true;
        } else if (name == QLatin1String("data-pilot-table") || name == QLatin1String("data-pilot-tables")) {
            m_f.pivots = true;
        } else if (name == QLatin1String("conditional-format") || name == QLatin1String("conditional-formats")) {
            m_f.conditional = true;
        }
        return true;
    }

    // ---- styles

    static QString propertyPrefix(QStringView name)
    {
        if (name == QLatin1String("table-cell-properties")) return QStringLiteral("cell");
        if (name == QLatin1String("text-properties")) return QStringLiteral("text");
        if (name == QLatin1String("paragraph-properties")) return QStringLiteral("para");
        if (name == QLatin1String("table-column-properties")) return QStringLiteral("col");
        if (name == QLatin1String("table-row-properties")) return QStringLiteral("row");
        if (name == QLatin1String("table-properties")) return QStringLiteral("table");
        return QString();
    }

    void addProperties(const QXmlStreamAttributes &a, const QString &prefix, ods::Props *props)
    {
        for (const QXmlStreamAttribute &at : a) {
            if (props->size() > 4000 || ++m_properties > kMaxOdsProperties) break;
            props->insert(prefix + QLatin1Char(':') + ods::canonicalName(at.namespaceUri().toString(), at.name().toString()), at.value().toString());
        }
    }

    bool startStyle(const QXmlStreamAttributes &a, QStringView name, QString *error)
    {
        if (name == QLatin1String("style") || name == QLatin1String("default-style")) {
            m_style = ods::StyleDef();
            m_style.family = attr(a, NS_STYLE, QLatin1String("family"));
            m_style.name = attr(a, NS_STYLE, QLatin1String("name"));
            m_style.parent = attr(a, NS_STYLE, QLatin1String("parent-style-name"));
            m_style.dataStyle = attr(a, NS_STYLE, QLatin1String("data-style-name"));
            m_isDefaultStyle = name == QLatin1String("default-style");
            m_inStyle = true;
            if (m_f.store.styleCount() >= kMaxOdsStyles) return fail(error, say("The file has more styles than any real file has."));
            return true;
        }
        if (name == QLatin1String("font-face")) {
            const QString face = attr(a, NS_STYLE, QLatin1String("name"));
            QString family = attr(a, NS_SVG, QLatin1String("font-family"));
            if (!face.isEmpty()) m_f.store.addFont(face, family.isEmpty() ? face : family);
            return true;
        }
        if (name == QLatin1String("map") && m_inData) {
            ods::DataStyle::Map map;
            map.condition = attr(a, NS_STYLE, QLatin1String("condition"));
            map.applyStyle = attr(a, NS_STYLE, QLatin1String("apply-style-name"));
            if (m_data.maps.size() < 8) m_data.maps.append(map);
            return true;
        }
        if (name == QLatin1String("text-properties") && m_inData) {
            quint32 rgb = 0;
            if (ods::colorOf(attr(a, NS_FO, QLatin1String("color")), &rgb)) m_data.color = rgb;
            return true;
        }
        const QString prefix = propertyPrefix(name);
        if (!prefix.isEmpty() && m_inStyle) addProperties(a, prefix, &m_style.props);
        return true;
    }

    // ---- data styles

    void startDataStyle(const QXmlStreamAttributes &a, QStringView name)
    {
        using ods::DataPart;
        if (name.endsWith(QLatin1String("-style")) && !m_inData) {
            m_data = ods::DataStyle();
            m_data.name = attr(a, NS_STYLE, QLatin1String("name"));
            m_data.kind = name.left(name.size() - 6).toString();
            m_data.truncateOnOverflow = attr(a, NS_NUMBER, QLatin1String("truncate-on-overflow")) != QLatin1String("false");
            m_inData = ++m_dataStyles <= kMaxOdsStyles;
            return;
        }
        if (!m_inData || m_data.parts.size() > 1000) return;
        auto integer = [&](const char *n, int fallback) {
            bool ok = false;
            const int v = attr(a, NS_NUMBER, QLatin1String(n)).toInt(&ok);
            return ok ? std::max(0, std::min(v, 100)) : fallback;
        };
        auto isLong = [&] { return attr(a, NS_NUMBER, QLatin1String("style")) == QLatin1String("long"); };
        DataPart p;
        if (name == QLatin1String("number")) {
            p.kind = DataPart::Number;
            p.hasDecimals = hasAttr(a, NS_NUMBER, QLatin1String("decimal-places"));
            p.decimals = integer("decimal-places", 0);
            bool blankOk = false;
            const int blank = attr(a, NS_LOEXT, QLatin1String("max-blank-integer-digits")).toInt(&blankOk);
            p.maxBlank = blankOk ? std::max(0, std::min(blank, 30)) : 0;
            p.minDecimals = hasAttr(a, NS_NUMBER, QLatin1String("min-decimal-places")) ? integer("min-decimal-places", 0) : -1;
            p.minInteger = integer("min-integer-digits", 1);
            p.grouping = attr(a, NS_NUMBER, QLatin1String("grouping")) == QLatin1String("true");
            bool ok = false;
            const int factor = attr(a, NS_NUMBER, QLatin1String("display-factor")).toInt(&ok);
            p.displayFactor = ok && factor >= 1 ? factor : 1;
        } else if (name == QLatin1String("scientific-number")) {
            p.kind = DataPart::Scientific;
            p.decimals = integer("decimal-places", 0);
            p.minInteger = integer("min-integer-digits", 1);
            p.minExponent = integer("min-exponent-digits", 1);
            p.forcedExponentSign = attr(a, NS_NUMBER, QLatin1String("forced-exponent-sign")) != QLatin1String("false");
        } else if (name == QLatin1String("fraction")) {
            p.kind = DataPart::Fraction;
            p.minInteger = integer("min-integer-digits", 0);
            p.minNumerator = integer("min-numerator-digits", 1);
            p.minDenominator = integer("min-denominator-digits", 1);
            p.denominatorValue = integer("denominator-value", 0);
        } else if (name == QLatin1String("text") || name == QLatin1String("currency-symbol") || name == QLatin1String("fill-character")) {
            p.kind = name == QLatin1String("text") ? DataPart::Text : name == QLatin1String("fill-character") ? DataPart::FillCharacter : DataPart::CurrencySymbol;
            const QString blank = attr(a, NS_LOEXT, QLatin1String("blank-width-char"));
            if (!blank.isEmpty()) p.blankWidth = blank[0];
            m_capture = true;
            m_text.clear();
            m_pendingText = true;
            m_data.parts.append(p);
            return;
        } else if (name == QLatin1String("embedded-text")) {
            m_capture = true;
            m_text.clear();
            m_embeddedAt = integer("position", 0);
            m_pendingEmbedded = true;
            return;
        } else if (name == QLatin1String("day")) {
            p.kind = DataPart::Day;
            p.longStyle = isLong();
        } else if (name == QLatin1String("month")) {
            p.kind = DataPart::Month;
            p.longStyle = isLong();
            p.textual = attr(a, NS_NUMBER, QLatin1String("textual")) == QLatin1String("true");
        } else if (name == QLatin1String("year")) {
            p.kind = DataPart::Year;
            p.longStyle = isLong();
        } else if (name == QLatin1String("era")) {
            p.kind = DataPart::Era;
        } else if (name == QLatin1String("day-of-week")) {
            p.kind = DataPart::DayOfWeek;
            p.longStyle = isLong();
        } else if (name == QLatin1String("week-of-year")) {
            p.kind = DataPart::WeekOfYear;
        } else if (name == QLatin1String("quarter")) {
            p.kind = DataPart::Quarter;
        } else if (name == QLatin1String("hours")) {
            p.kind = DataPart::Hours;
            p.longStyle = isLong();
        } else if (name == QLatin1String("minutes")) {
            p.kind = DataPart::Minutes;
            p.longStyle = isLong();
        } else if (name == QLatin1String("seconds")) {
            p.kind = DataPart::Seconds;
            p.longStyle = isLong();
            p.decimals = integer("decimal-places", 0);
        } else if (name == QLatin1String("am-pm")) {
            p.kind = DataPart::AmPm;
        } else if (name == QLatin1String("text-content")) {
            p.kind = DataPart::TextContent;
        } else if (name == QLatin1String("boolean")) {
            p.kind = DataPart::Boolean;
        } else {
            return;
        }
        m_data.parts.append(p);
    }

    // ---- settings

    bool startConfig(const QXmlStreamAttributes &a, QStringView name)
    {
        const QString cname = attr(a, NS_CONFIG, QLatin1String("name"));
        if (name == QLatin1String("config-item-set") || name == QLatin1String("config-item-map-indexed") || name == QLatin1String("config-item-map-named") || name == QLatin1String("config-item-map-entry")) {
            if (m_config.size() < 16) m_config.append({name.toString(), cname});
            else m_config.append({QString(), QString()});
            return true;
        }
        if (name == QLatin1String("config-item")) {
            m_configItem = cname;
            m_capture = true;
            m_text.clear();
            m_inConfigItem = true;
        }
        return true;
    }

    // The sheet whose view settings the path is in ("" outside the Tables of a view).
    QString configSheet() const
    {
        bool inView = false, inTables = false;
        QString sheet;
        for (const auto &e : m_config) {
            if (e.first == QLatin1String("config-item-set")) inView = e.second == QLatin1String("ooo:view-settings");
            else if (e.first == QLatin1String("config-item-map-named") && inView) inTables = e.second == QLatin1String("Tables");
            else if (e.first == QLatin1String("config-item-map-entry") && inTables) sheet = e.second;
        }
        return sheet;
    }
    bool configInViewEntry() const
    {
        bool inView = false;
        int entries = 0;
        for (const auto &e : m_config) {
            if (e.first == QLatin1String("config-item-set")) inView = e.second == QLatin1String("ooo:view-settings");
            else if (e.first == QLatin1String("config-item-map-entry") && inView) ++entries;
        }
        return inView && entries == 1;
    }

    void end(const QXmlStreamReader &x)
    {
        const QStringView ns = x.namespaceUri(), name = x.name();
        if (ns == NS_STYLE) {
            if ((name == QLatin1String("style") || name == QLatin1String("default-style")) && m_inStyle) {
                if (m_isDefaultStyle) {
                    ods::Props p = m_style.props;
                    if (!m_style.dataStyle.isEmpty()) p.insert(QStringLiteral("data-style"), m_style.dataStyle);
                    m_f.store.setDefault(m_style.family, p);
                } else if (!m_style.name.isEmpty()) {
                    m_f.store.addStyle(m_style);
                }
                m_inStyle = false;
            }
        } else if (ns == NS_NUMBER) {
            if (m_pendingText && (name == QLatin1String("text") || name == QLatin1String("currency-symbol") || name == QLatin1String("fill-character"))) {
                if (!m_data.parts.isEmpty()) m_data.parts.last().text = m_text;
                m_pendingText = false;
                m_capture = false;
            } else if (m_pendingEmbedded && name == QLatin1String("embedded-text")) {
                m_pendingEmbedded = false;
                m_capture = false;
            } else if (m_inData && name.endsWith(QLatin1String("-style"))) {
                if (!m_data.name.isEmpty()) m_f.store.addDataStyle(m_data);
                m_inData = false;
            }
        } else if (ns == NS_CONFIG) {
            if (name == QLatin1String("config-item")) {
                if (m_inConfigItem) {
                    const QString sheet = configSheet();
                    if (!sheet.isEmpty() && (m_f.sheetViews.size() < 4000 || m_f.sheetViews.contains(sheet))) {
                        auto &items = m_f.sheetViews[sheet];
                        if (items.size() < 200) items.insert(m_configItem, m_text);
                    } else if (configInViewEntry() && m_f.viewItems.size() < 200) {
                        m_f.viewItems.insert(m_configItem, m_text);
                    }
                }
                m_inConfigItem = false;
                m_capture = false;
            } else if ((name == QLatin1String("config-item-set") || name == QLatin1String("config-item-map-indexed") || name == QLatin1String("config-item-map-named")
                        || name == QLatin1String("config-item-map-entry"))
                       && !m_config.isEmpty()) {
                m_config.removeLast();
            }
        } else if (ns == NS_TABLE) {
            if (name == QLatin1String("table") && m_inTable && m_depth == m_spreadsheetDepth + 1) m_inTable = false;
        } else if (ns == NS_OFFICE) {
            if (name == QLatin1String("scripts")) m_inScripts = 0;
            else if (name == QLatin1String("spreadsheet")) m_spreadsheetDepth = 0;
        }
    }

    Found &m_f;
    Limits m_limits;
    int m_depth = 0;
    int m_spreadsheetDepth = 0;
    int m_inScripts = 0;
    bool m_inTable = false;
    bool m_capture = false;
    QString m_text;
    // a style being read
    ods::StyleDef m_style;
    bool m_inStyle = false, m_isDefaultStyle = false;
    qint64 m_properties = 0;   // all the properties of all the styles
    qint64 m_dataStyles = 0;
    // a data style being read
    ods::DataStyle m_data;
    bool m_inData = false, m_pendingText = false, m_pendingEmbedded = false;
    int m_embeddedAt = 0;
    // the settings path
    QVector<QPair<QString, QString>> m_config;
    QString m_configItem;
    bool m_inConfigItem = false;
};

// ---- where each sheet is in content.xml

struct Range {
    qsizetype from = 0, to = 0;
};

// The byte ranges of the elements of this name that are not inside one another, in the XML held whole: found by searching, so that a sheet is read by itself, from a piece of the
// bytes, with no need to read the sheets before it again. Empty, with `ok` false, if the start tags and end tags do not pair up.
QVector<Range> findElements(const QByteArray &data, const QByteArray &qname, bool *ok)
{
    QVector<Range> ranges;
    *ok = true;
    const QByteArray startNeedle = "<" + qname, endNeedle = "</" + qname;
    qsizetype pos = 0, nextStart = -2, nextEnd = -2, rangeStart = 0;
    int depth = 0;
    const QByteArrayView view(data);
    auto boundary = [&](qsizetype at, qsizetype len) {
        if (at + len >= data.size()) return false;
        const char c = data[at + len];
        return c == ' ' || c == '>' || c == '/' || c == '\n' || c == '\r' || c == '\t';
    };
    for (;;) {
        if (nextStart != -1 && nextStart < pos) {
            nextStart = view.indexOf(startNeedle, pos);
            while (nextStart >= 0 && !boundary(nextStart, startNeedle.size())) nextStart = view.indexOf(startNeedle, nextStart + 1);
        }
        if (nextEnd != -1 && nextEnd < pos) {
            nextEnd = view.indexOf(endNeedle, pos);
            while (nextEnd >= 0 && !boundary(nextEnd, endNeedle.size())) nextEnd = view.indexOf(endNeedle, nextEnd + 1);
        }
        if (nextStart < 0 && nextEnd < 0) break;
        if (nextStart >= 0 && (nextEnd < 0 || nextStart < nextEnd)) {
            const qsizetype te = xmlscan::tagEnd(view, nextStart);
            if (te < 0) {
                *ok = false;
                return {};
            }
            const bool selfClosing = data[te - 2] == '/';
            if (depth == 0) rangeStart = nextStart;
            if (selfClosing) {
                if (depth == 0) ranges.append({rangeStart, te});
            } else {
                ++depth;
            }
            pos = te;
        } else {
            const qsizetype te = xmlscan::tagEnd(view, nextEnd);
            if (te < 0 || depth == 0) {
                *ok = false;
                return {};
            }
            if (--depth == 0) ranges.append({rangeStart, te});
            pos = te;
        }
    }
    if (depth != 0) *ok = false;
    return ranges;
}

// The sheets found by searching for their tags in the XML held whole, when it is written as the standard's own programs write it: `<table:table ` to `</table:table>`. A start tag
// or an end tag in any other form (a line break after the name, a space in the end tag) is not found, and if that leaves the tags that are found not in pairs this gives false, so that the
// whole document is read by the parser instead.
bool findTablesFast(const QByteArray &data, QVector<Range> *ranges)
{
    const QByteArrayView view(data);
    qsizetype pos = 0, rangeStart = 0;
    int depth = 0;
    qsizetype nextStart = -2, nextEnd = -2;
    const QByteArrayView startNeedle("<table:table "), endNeedle("</table:table>");
    for (;;) {
        if (nextStart != -1 && nextStart < pos) nextStart = view.indexOf(startNeedle, pos);
        if (nextEnd != -1 && nextEnd < pos) nextEnd = view.indexOf(endNeedle, pos);
        if (nextStart < 0 && nextEnd < 0) break;
        if (nextStart >= 0 && (nextEnd < 0 || nextStart < nextEnd)) {
            const qsizetype te = xmlscan::tagEnd(view, nextStart);
            if (te < 0) return false;
            if (depth == 0) rangeStart = nextStart;
            if (data[te - 2] == '/') {
                if (depth == 0) ranges->append({rangeStart, te});
            } else {
                ++depth;
            }
            pos = te;
        } else {
            if (depth == 0) return false;
            pos = nextEnd + endNeedle.size();
            if (--depth == 0) ranges->append({rangeStart, pos});
        }
    }
    return depth == 0;
}

// The document with every sheet cut down to its start tag and what the first pass needs of it: the names defined inside it and a mark for each thing the program does not read that it
// holds (a drawing, a note, a pivot table, a validation, a conditional format). The first pass reads the styles and the head and tail of the document with the parser, and does not read
// the million rows of a large sheet, which are read when the sheet is.
bool skeletonOf(const QByteArray &data, QByteArrayView rootTag, const QVector<Range> &ranges, QByteArray *out)
{
    auto declares = [&](const char *prefix, QLatin1String uri) {
        bool found = false;
        const QByteArrayView value = xmlscan::attribute(rootTag, QByteArray("xmlns:") + prefix, &found);
        return found && QLatin1String(value.constData(), value.size()) == uri;
    };
    struct Mark {
        const char *needle;
        const char *element;
        bool usable;
        bool seen;
    };
    Mark marks[] = {{"<draw:object", "<draw:object/>", declares("draw", NS_DRAW), false},
                    {"<draw:image", "<draw:image/>", declares("draw", NS_DRAW), false},
                    {"<draw:frame", "<draw:frame/>", declares("draw", NS_DRAW), false},
                    {"<draw:custom-shape", "<draw:custom-shape/>", declares("draw", NS_DRAW), false},
                    {"<office:annotation", "<office:annotation/>", declares("office", NS_OFFICE), false},
                    {"<table:data-pilot-table", "<table:data-pilot-table/>", true, false},
                    {"<table:content-validation", "<table:content-validation/>", true, false},
                    {"<table:conditional-format", "<table:conditional-format/>", true, false},
                    {"<calcext:conditional-format", "<calcext:conditional-format/>", declares("calcext", NS_CALCEXT), false}};
    const QByteArrayView view(data);
    qsizetype pos = 0;
    out->reserve(data.size() / 16 + 1024);
    for (const Range &r : ranges) {
        out->append(data.constData() + pos, r.from - pos);
        const qsizetype te = xmlscan::tagEnd(view, r.from);
        out->append(data.constData() + r.from, te - r.from);
        if (data[te - 2] != '/') {
            const QByteArrayView inside = view.mid(te, r.to - te);
            const qsizetype names = inside.indexOf(QByteArrayView("<table:named-expressions"));
            if (names >= 0) {
                const qsizetype endNames = inside.indexOf(QByteArrayView("</table:named-expressions>"), names);
                if (endNames >= 0) out->append(inside.constData() + names, endNames + qsizetype(qstrlen("</table:named-expressions>")) - names);
            }
            for (Mark &m : marks)
                if (m.usable && !m.seen && inside.indexOf(QByteArrayView(m.needle)) >= 0) {
                    m.seen = true;
                    out->append(m.element);
                }
            out->append("</table:table>");
        }
        pos = r.to;
    }
    out->append(data.constData() + pos, data.size() - pos);
    return true;
}

// A device that gives three pieces of bytes one after another, with no copy: the root's start tag, one sheet's element, and the root's end tag.
class SliceDevice : public QIODevice {
public:
    SliceDevice(const QByteArray &head, QByteArrayView body, const QByteArray &tail) : m_head(head), m_body(body), m_tail(tail) { open(QIODevice::ReadOnly); }
    bool isSequential() const override { return true; }

protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        qint64 done = 0;
        auto take = [&](QByteArrayView piece, qint64 &at) {
            const qint64 n = std::min<qint64>(maxlen - done, piece.size() - at);
            if (n > 0) {
                memcpy(data + done, piece.constData() + at, size_t(n));
                at += n;
                done += n;
            }
        };
        take(m_head, m_headAt);
        if (m_headAt >= m_head.size()) take(m_body, m_bodyAt);
        if (m_headAt >= m_head.size() && m_bodyAt >= m_body.size()) take(m_tail, m_tailAt);
        return done;
    }
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    QByteArrayView m_head, m_body, m_tail;
    qint64 m_headAt = 0, m_bodyAt = 0, m_tailAt = 0;
};

// The day number of an ISO date or date and time (2024-03-15, 2024-03-15T10:30:00.5), counted from the document's null date: what the document means by the number, in whatever
// calendar the date is in, so that its day 0 (1899-12-30) is 0.
bool dateSerialFrom(const QString &text, const QDate &nullDate, double *serial)
{
    static const QRegularExpression re(QStringLiteral("^(-?[0-9]{4,6})-([0-9]{2})-([0-9]{2})(?:T([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.[0-9]+)?)?(?:Z|[+-][0-9]{2}:[0-9]{2})?$"));
    const QRegularExpressionMatch m = re.match(text.trimmed());
    if (!m.hasMatch()) return false;
    const QDate date(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
    if (!date.isValid() || !nullDate.isValid()) return false;
    double day = double(date.toJulianDay() - nullDate.toJulianDay());
    if (m.hasCaptured(4)) day += (m.captured(4).toInt() * 3600.0 + m.captured(5).toInt() * 60.0 + m.captured(6).toDouble() + m.captured(7).toDouble()) / 86400.0;
    *serial = day;
    return true;
}

struct ColProps {
    double widthPt = -1;
};
struct RowProps {
    double heightPt = -1;
    bool optimal = false;
};

struct SheetShape {
    QString name;
    SheetState state = SheetState::Visible;
    bool isProtected = false;
    bool rightToLeft = false;
    ColorSpec tabColor;
};

} // namespace

struct OdsBook::Impl {
    QByteArray content, head, tail;
    QVector<Range> slices;
    Found found;
    QVector<SheetShape> shapes;
    QHash<QString, quint32> xfOf;
    QHash<quint32, QString> nameOfXf;   // a style that has the format (any one)
    DateSystem dateSystem = DateSystem::Date1900;
    QDate nullDate = QDate(1899, 12, 30);
    int maxDigitWidth = 7;
    Limits limits;
};

OdsBook::OdsBook() = default;
OdsBook::~OdsBook() = default;

bool looksLikeOpenDocumentZip(const QByteArray &zip)
{
    QVector<jp::ZipEntry> entries;
    if (!jp::readZipDirectory(zip, &entries)) return false;
    bool content = false;
    for (const jp::ZipEntry &e : std::as_const(entries)) {
        if (e.name == QLatin1String("[Content_Types].xml")) return false;
        if (e.name == QLatin1String("content.xml")) content = true;
    }
    return content;
}

bool looksLikeFlatOpenDocument(const QByteArray &head)
{
    QByteArray h = head.left(16384);
    if (h.startsWith("\xEF\xBB\xBF")) h.remove(0, 3);
    const qsizetype root = h.indexOf("<office:document");
    if (root < 0 || h.indexOf("<office:document-content") == root) return false;
    return h.indexOf("opendocument.spreadsheet", root) >= 0;
}

namespace {

// The widest digit of the default font, in whole pixels, as the grid counts a column's width in characters (look.cpp).
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

} // namespace

std::shared_ptr<const OdsBook> OdsBook::open(const QByteArray &file, bool flat, const Limits &limits, WorkbookInfo *info, SharedStrings *shared, Styles *styles, QString *error)
{
    auto fail = [&](const QString &message) -> std::shared_ptr<const OdsBook> {
        if (error) *error = message;
        return nullptr;
    };
    std::shared_ptr<OdsBook> book(new OdsBook);
    book->m_flat = flat;
    book->m = std::make_shared<Impl>();
    Impl &m = *book->m;
    m.limits = limits;
    Package package;
    QStringList parts;   // the names of the parts that hold styles and settings, in the order they are read
    if (flat) {
        m.content = file;
    } else {
        QString why;
        if (!package.readLazyOpenDocument(file, &why)) return fail(why);
        const QByteArray mime = package.part(QStringLiteral("/mimetype")).trimmed();
        if (!mime.isEmpty() && mime != kMimeSpreadsheet && mime != QByteArray(kMimeSpreadsheet) + "-template") {
            if (mime.contains("opendocument.text")) return fail(say("This file is an OpenDocument text document, not a spreadsheet."));
            if (mime.contains("opendocument.presentation")) return fail(say("This file is an OpenDocument presentation, not a spreadsheet."));
            if (mime.contains("opendocument.graphics")) return fail(say("This file is an OpenDocument drawing, not a spreadsheet."));
            return fail(say("This file is an OpenDocument file, but not a spreadsheet."));
        }
        // a document that is encrypted holds parts that are not XML
        const QByteArray manifest = package.part(QStringLiteral("/META-INF/manifest.xml"));
        if (manifest.contains("encryption-data")) return fail(say("This spreadsheet is protected with a password. Open it in the program that made it, save a copy without the password, and open that."));
        if (!package.hasPart(QStringLiteral("/content.xml"))) return fail(say("The file has no sheets in it."));
        const qint64 size = package.partSize(QStringLiteral("/content.xml"));
        if (size > limits.odsContentBytes) return fail(say("The spreadsheet is larger than this program reads."));
        m.content = package.part(QStringLiteral("/content.xml"));
        if (m.content.isEmpty()) return fail(say("The file is damaged, or its parts come to more than this program reads."));
        for (const QString &name : {QStringLiteral("/Scripts/"), QStringLiteral("/Basic/")}) {
            for (const QString &p : package.partNames())
                if (p.startsWith(name)) {
                    m.found.macros = true;
                    break;
                }
        }
        for (const QString &p : package.partNames()) {
            if (p.endsWith(QLatin1String("/content.xml")) && p != QLatin1String("/content.xml")) m.found.charts = true;
            if (p.startsWith(QLatin1String("/Pictures/")) && !p.endsWith(QLatin1Char('/'))) m.found.drawings = true;
        }
    }
    // ---- the first pass: styles, settings, and content (the sheets' names, the names, the notes of what is skipped)
    DocumentParser parser(&m.found, limits);
    if (!flat) {
        for (const QString &name : {QStringLiteral("/styles.xml"), QStringLiteral("/settings.xml")}) {
            if (!package.hasPart(name)) continue;
            const QByteArray bytes = package.part(name);
            QBuffer buffer;
            buffer.setData(bytes);
            buffer.open(QIODevice::ReadOnly);
            QString why;
            if (!parser.parse(&buffer, &why)) return fail(why);
        }
    }
    // (the document's root, and the sheets: when the XML is written as the standard's own programs write it, each sheet is found by its tags and read by itself, when it is asked for)
    xmlscan::Tag root;
    if (!xmlscan::nextTag(m.content, 0, &root) || root.closing || root.selfClosing) return fail(say("The file is damaged: it has no document in it."));
    {
        const QByteArrayView rootTag = QByteArrayView(m.content).mid(root.start, root.end - root.start);
        bool tableNs = false;
        const QByteArrayView prefixTable = xmlscan::attribute(rootTag, "xmlns:table", &tableNs);
        QVector<Range> found;
        QByteArray skeleton;
        const bool fast = tableNs && QLatin1String(prefixTable.constData(), prefixTable.size()) == NS_TABLE && findTablesFast(m.content, &found) && skeletonOf(m.content, rootTag, found, &skeleton);
        QBuffer buffer;
        buffer.setData(fast ? skeleton : m.content);
        buffer.open(QIODevice::ReadOnly);
        QString why;
        if (!parser.parse(&buffer, &why)) return fail(why);
        if (fast) m.slices = found;
    }
    if (m.found.tables.isEmpty()) return fail(say("The file has no sheets in it."));
    // ---- where each sheet is
    {
        const QByteArray qname = m.found.tableQName.toUtf8();
        if (m.slices.isEmpty()) {
            bool ok = false;
            m.slices = findElements(m.content, qname, &ok);
            if (!ok) m.slices.clear();
        }
        if (m.slices.size() != m.found.tables.size()) return fail(say("The file is damaged: its sheets cannot be told apart."));
        m.head = m.content.mid(root.start, root.end - root.start);
        m.tail = "</" + QByteArray(root.name.constData(), root.name.size()) + ">";
        // (the sheet's own name, as the element has it, must be the one the parser found)
        for (int i = 0; i < m.slices.size(); ++i) {
            const qsizetype te = xmlscan::tagEnd(m.content, m.slices[i].from);
            const QByteArrayView tag = QByteArrayView(m.content).mid(m.slices[i].from, te - m.slices[i].from);
            bool found = false;
            const QByteArrayView name = xmlscan::attribute(tag, QByteArray(m.found.tableQName.left(m.found.tableQName.indexOf(QLatin1Char(':')) + 1).toUtf8() + "name"), &found);
            if (found && xmlscan::unescape(name) != m.found.tables[i].name) return fail(say("The file is damaged: its sheets cannot be told apart."));
        }
    }
    // ---- the workbook
    *info = WorkbookInfo();
    *shared = SharedStrings();
    *styles = Styles();
    const ods::StyleStore &store = m.found.store;
    {
        QString why;
        if (!store.buildCellFormats(limits, styles, &m.xfOf, &why)) return fail(why);
    }
    m.maxDigitWidth = styles->fonts.isEmpty() ? 7 : digitWidthOf(styles->fonts[0]);
    for (auto it = m.xfOf.cbegin(); it != m.xfOf.cend(); ++it) {
        const auto old = m.nameOfXf.constFind(it.value());
        if (old == m.nameOfXf.constEnd() || it.key() < old.value()) m.nameOfXf.insert(it.value(), it.key());
    }
    // the date system: the day the numbers count from
    const QString nullDate = m.found.nullDate.left(10);
    m.dateSystem = nullDate == QLatin1String("1904-01-01") ? DateSystem::Date1904 : DateSystem::Date1900;
    const QDate parsedNull = QDate::fromString(nullDate, QStringLiteral("yyyy-MM-dd"));
    if (parsedNull.isValid()) m.nullDate = parsedNull;
    info->dateSystem = m.dateSystem;
    info->iterate = m.found.iterate;
    for (int i = 0; i < m.found.tables.size(); ++i) {
        const TableInfo &t = m.found.tables[i];
        SheetEntry e;
        e.name = t.name.isEmpty() ? QStringLiteral("Sheet%1").arg(i + 1) : t.name.left(kMaxSheetNameChars);
        e.sheetId = i + 1;
        SheetShape shape;
        shape.name = e.name;
        shape.isProtected = t.isProtected;
        const ods::Props p = store.resolve(QStringLiteral("table"), t.styleName);
        if (p.value(QStringLiteral("table:table:display")) == QLatin1String("false")) e.state = SheetState::Hidden;
        shape.state = e.state;
        shape.rightToLeft = p.value(QStringLiteral("table:style:writing-mode")).startsWith(QLatin1String("rl"));
        quint32 rgb = 0;
        if (ods::colorOf(p.value(QStringLiteral("table:tableooo:tab-color")), &rgb)) {
            shape.tabColor.kind = ColorSpec::Kind::Rgb;
            shape.tabColor.rgb = rgb;
        }
        info->sheets.append(e);
        m.shapes.append(shape);
    }
    // names
    for (const NamedItem &n : std::as_const(m.found.names)) {
        bool ok = false;
        const QString ref = n.isRange ? odfRangeToText(n.text, &ok) : odfFormulaToText(n.text, &ok);
        if (!ok) continue;
        DefinedName d;
        d.name = n.name;
        d.ref = ref;
        d.localSheet = n.sheet;
        info->definedNames.append(d);
    }
    // the sheet that was open
    const QString active = m.found.viewItems.value(QStringLiteral("ActiveTable"));
    for (int i = 0; i < info->sheets.size(); ++i)
        if (info->sheets[i].name == active) info->activeTab = i;
    if (info->activeTab < info->sheets.size() && info->sheets[info->activeTab].state != SheetState::Visible) info->activeTab = 0;
    // what is not read
    if (m.found.charts) book->m_skipped << say("charts");
    if (m.found.drawings) book->m_skipped << say("pictures and other drawing objects");
    if (m.found.pivots) book->m_skipped << say("pivot tables");
    if (m.found.comments) book->m_skipped << say("comments");
    if (m.found.macros) book->m_skipped << say("macros");
    if (m.found.conditional) book->m_skipped << say("conditional formats");
    if (m.found.validation) book->m_skipped << say("data validation");
    return book;
}

// ---- one sheet

namespace {

struct Proto {
    quint32 repeat = 1;
    quint32 xf = 0;
    CellType type = CellType::Empty;
    double number = 0;
    QString text;
    QVector<QPair<qsizetype, QString>> spans;   // where the style of the text changes: the character, and the text style from there
    QString formula;
    bool hasFormula = false;
    bool formulaFailed = false;
    quint32 colSpan = 1, rowSpan = 1;
    quint32 matrixCols = 0, matrixRows = 0;
    bool covered = false;
    qint32 textIndex = -1;       // once its text is in the sheet
    qint32 formulaTextIndex = -1;
    bool hasValue() const { return type != CellType::Empty || hasFormula; }
};

struct ColRun {
    quint32 first = 0, count = 0;
    QString style;
    quint32 xf = 0;
    bool hidden = false;
};

struct RowRun {
    quint32 first = 0, count = 0;
    QString style;
    bool hidden = false;
    bool hasXf = false;
    quint32 xf = 0;
};

class SheetReader {
public:
    SheetReader(const OdsBook::Impl &m, int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out)
        : m_m(m), m_index(index), m_limits(limits), m_progress(progress), m_s(*out)
    {
        m_stopAfter = stopAfterRow;
        if (screenRows > 0 && (m_stopAfter < 0 || screenRows - 1 < m_stopAfter)) m_stopAfter = screenRows - 1;
    }

    bool run()
    {
        const Range range = m_m.slices[m_index];
        SliceDevice slice(m_m.head, QByteArrayView(m_m.content).mid(range.from, range.to - range.from), m_m.tail);
        jp::office::XmlGuardDevice guard(&slice, kMaxXmlTokenBytes);
        guard.open(QIODevice::ReadOnly);
        QXmlStreamReader xml(&guard);
        m_xml = &xml;
        m_guard = &guard;
        bool ok = read();
        if (ok && guard.refused()) ok = fail(guardMessage(guard));
        if (ok && !m_stopped && xml.hasError()) ok = fail(say("The XML is damaged: %1").arg(xml.errorString()));
        finish();
        return ok && m_s.error.isEmpty();
    }

private:
    bool fail(const QString &message)
    {
        if (m_s.error.isEmpty()) m_s.error = message;
        return false;
    }
    // The document ends where it should not: the guard says why, if it ended it.
    bool damaged() { return fail(m_guard && m_guard->refused() ? guardMessage(*m_guard) : say("The XML is damaged.")); }

    static bool is(QStringView ns, QStringView name, QLatin1String wantNs, QLatin1String wantName) { return ns == wantNs && name == wantName; }

    // Skips the element just begun, to its end, with its nesting bounded.
    bool skipElement()
    {
        int depth = 1;
        while (depth > 0 && !m_xml->atEnd()) {
            switch (m_xml->readNext()) {
            case QXmlStreamReader::StartElement:
                if (++depth + m_depth > kMaxOdsDepth) return fail(say("The XML is nested more deeply than any real file's."));
                break;
            case QXmlStreamReader::EndElement: --depth; break;
            default: break;
            }
        }
        return depth == 0;
    }

    bool read()
    {
        QXmlStreamReader &xml = *m_xml;
        m_depth = 0;
        bool inTable = false;
        while (!xml.atEnd()) {
            const QXmlStreamReader::TokenType t = xml.readNext();
            if (t == QXmlStreamReader::EndElement) {
                --m_depth;
                continue;
            }
            if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference) return fail(say("The XML has a DOCTYPE, which is not allowed."));
            if (t != QXmlStreamReader::StartElement) continue;
            if (++m_depth > kMaxOdsDepth) return fail(say("The XML is nested more deeply than any real file's."));
            const QStringView ns = xml.namespaceUri(), name = xml.name();
            if (ns == NS_TABLE && name == QLatin1String("table") && !inTable) {
                inTable = true;
                continue;
            }
            if (!inTable) continue;
            if (ns != NS_TABLE) {
                // (drawings, forms, and notes of the sheet are not read)
                if (!skipElementCounted()) return false;
                continue;
            }
            if (name == QLatin1String("table-column")) {
                if (!column(xml.attributes())) return false;
                if (!skipElementCounted()) return false;
            } else if (name == QLatin1String("table-row")) {
                if (!row()) return false;
                if (m_stopped) return true;
            } else if (name == QLatin1String("table-columns") || name == QLatin1String("table-header-columns") || name == QLatin1String("table-column-group") || name == QLatin1String("table-header-rows")
                       || name == QLatin1String("table-rows") || name == QLatin1String("table-row-group")) {
                continue;   // (a group: its columns and rows are the sheet's)
            } else {
                if (!skipElementCounted()) return false;
            }
        }
        return true;
    }

    // Skips the element just begun (its start was counted in m_depth, and its end is counted here).
    bool skipElementCounted()
    {
        const bool ok = skipElement();
        --m_depth;
        return ok;
    }

    // ---- columns

    bool column(const QXmlStreamAttributes &a)
    {
        bool ok = false;
        qint64 repeat = attr(a, NS_TABLE, QLatin1String("number-columns-repeated")).toLongLong(&ok);
        if (!ok || repeat < 1) repeat = 1;
        if (m_nextCol >= kMaxColumns) return true;
        ColRun run;
        run.first = m_nextCol;
        run.count = quint32(std::min<qint64>(repeat, qint64(kMaxColumns) - m_nextCol));
        run.style = attr(a, NS_TABLE, QLatin1String("style-name"));
        const QString vis = attr(a, NS_TABLE, QLatin1String("visibility"));
        run.hidden = vis == QLatin1String("collapse") || vis == QLatin1String("filter");
        const QString cell = attr(a, NS_TABLE, QLatin1String("default-cell-style-name"));
        run.xf = cell.isEmpty() ? m_m.xfOf.value(QString()) : m_m.xfOf.value(cell, m_m.xfOf.value(QString()));
        m_nextCol += run.count;
        if (qint64(m_colRuns.size()) >= m_limits.columnSpans) return fail(say("The sheet has more columns defined than any real file has."));
        m_colRuns.push_back(run);
        return true;
    }

    void ensureColumnTable()
    {
        if (!m_colXf.isEmpty()) return;
        m_colXf.fill(m_m.xfOf.value(QString()), int(kMaxColumns));
        for (const ColRun &r : m_colRuns)
            for (quint32 c = r.first; c < r.first + r.count; ++c) m_colXf[int(c)] = r.xf;
    }

    // ---- rows

    bool row()
    {
        QXmlStreamReader &xml = *m_xml;
        const QXmlStreamAttributes a = xml.attributes();
        ensureColumnTable();
        bool ok = false;
        qint64 repeat = attr(a, NS_TABLE, QLatin1String("number-rows-repeated")).toLongLong(&ok);
        if (!ok || repeat < 1) repeat = 1;
        const QString style = attr(a, NS_TABLE, QLatin1String("style-name"));
        const QString vis = attr(a, NS_TABLE, QLatin1String("visibility"));
        const bool hidden = vis == QLatin1String("collapse") || vis == QLatin1String("filter");
        const qint64 rowIndex = m_nextRow;
        if (rowIndex >= qint64(kMaxRows)) {
            // (a row past the last a sheet has: nothing in it is read, and nothing of it is said unless it holds something)
            m_rowsPast = true;
            return skipElementCounted();
        }
        repeat = std::min<qint64>(repeat, qint64(kMaxRows) - rowIndex);
        // the cells of the row
        m_row.clear();
        quint32 col = 0;
        for (;;) {
            if (xml.atEnd()) return damaged();
            const QXmlStreamReader::TokenType t = xml.readNext();
            if (t == QXmlStreamReader::EndElement) {
                --m_depth;
                break;
            }
            if (t != QXmlStreamReader::StartElement) continue;
            if (++m_depth > kMaxOdsDepth) return fail(say("The XML is nested more deeply than any real file's."));
            const QStringView ns = xml.namespaceUri(), name = xml.name();
            if (ns == NS_TABLE && (name == QLatin1String("table-cell") || name == QLatin1String("covered-table-cell"))) {
                Proto p;
                if (!cell(name == QLatin1String("covered-table-cell"), col, &p)) return false;
                const quint32 here = col;
                col = quint32(std::min<quint64>(quint64(col) + p.repeat, quint64(kMaxColumns) + 1));   // (a row of more cells than a sheet has stops counting at its edge)
                if (here < kMaxColumns) m_row.push_back(std::move(p));
            } else if (!skipElementCounted()) {
                return false;
            }
        }
        m_nextRow = rowIndex + repeat;
        return finishRow(rowIndex, repeat, style, hidden);
    }

    // The row (and its repeats) is over: its cells go into the sheet.
    bool finishRow(qint64 rowIndex, qint64 repeat, const QString &style, bool hidden)
    {
        // cells that are only a look, in a run that reaches the end of the row, are the look of the row
        RowRun run;
        run.first = quint32(rowIndex);
        run.count = quint32(repeat);
        run.style = style;
        run.hidden = hidden;
        quint32 col = 0;
        // (the columns each prototype covers)
        std::vector<std::pair<quint32, Proto *>> placed;
        placed.reserve(m_row.size());
        for (Proto &p : m_row) {
            placed.push_back({col, &p});
            col += p.repeat;
        }
        qint64 rowsToMake = repeat;
        bool stopHere = false;
        if (m_stopAfter >= 0 && rowIndex + repeat - 1 >= m_stopAfter) {
            rowsToMake = std::max<qint64>(0, m_stopAfter - rowIndex + 1);
            stopHere = true;
        }
        // the cells that are made for each row
        struct Piece {
            quint32 col;
            Proto *p;
            quint32 count;
        };
        std::vector<Piece> pieces;
        for (const auto &[first, p] : placed) {
            if (first >= kMaxColumns) break;
            const quint32 count = std::min<quint32>(p->repeat, kMaxColumns - first);
            if (!p->hasValue() && p->colSpan <= 1 && p->rowSpan <= 1) {
                // an empty cell has the look of its column unless it says otherwise; a run of them to the end of the row is the row's look
                if (first + count >= kMaxColumns && count >= 64) {
                    if (!run.hasXf && p->xf != 0) {
                        run.hasXf = true;
                        run.xf = p->xf;
                    }
                    continue;
                }
                bool differs = false;
                for (quint32 c = first; c < first + count && !differs; ++c) differs = m_colXf[int(c)] != p->xf;
                if (!differs) continue;
            }
            pieces.push_back({first, p, count});
        }
        if (qint64(m_rowRuns.size()) < m_limits.rowSpecs + 1) m_rowRuns.push_back(run);
        for (qint64 r = 0; r < rowsToMake && !pieces.empty(); ++r) {
            for (const Piece &piece : pieces)
                if (!emit(quint32(rowIndex + r), piece.col, *piece.p, piece.count, quint32(r))) return false;
        }
        if (stopHere) {
            m_stopped = true;
            m_s.partial = true;
        }
        return true;
    }

    // ---- cells

    bool cell(bool covered, quint32 col, Proto *out)
    {
        QXmlStreamReader &xml = *m_xml;
        const QXmlStreamAttributes a = xml.attributes();
        Proto &p = *out;
        p.covered = covered;
        bool ok = false;
        qint64 repeat = attr(a, NS_TABLE, QLatin1String("number-columns-repeated")).toLongLong(&ok);
        if (!ok || repeat < 1) repeat = 1;
        p.repeat = quint32(std::min<qint64>(repeat, 1 << 20));
        const QString styleName = attr(a, NS_TABLE, QLatin1String("style-name"));
        const quint32 colDefault = m_colXf[int(std::min<quint32>(col, kMaxColumns - 1))];
        p.xf = styleName.isEmpty() ? colDefault : m_m.xfOf.value(styleName, m_m.xfOf.value(QString()));
        if (!covered) {
            const qint64 cs = attr(a, NS_TABLE, QLatin1String("number-columns-spanned")).toLongLong(), rs = attr(a, NS_TABLE, QLatin1String("number-rows-spanned")).toLongLong();
            p.colSpan = quint32(std::max<qint64>(1, std::min<qint64>(cs, kMaxColumns)));
            p.rowSpan = quint32(std::max<qint64>(1, std::min<qint64>(rs, kMaxRows)));
        }
        const QString formula = attr(a, NS_TABLE, QLatin1String("formula"));
        const QString valueType = attr(a, NS_OFFICE, QLatin1String("value-type"));
        const QString calcType = attr(a, NS_CALCEXT, QLatin1String("value-type"));
        // the text of the cell, and its runs
        QString text;
        QVector<QString> spanStack;
        QVector<QPair<qsizetype, QString>> spans;
        int paragraphs = 0, depth = 0, skipDepth = 0;
        bool inParagraph = false, tooLong = false;
        int paragraphDepth = 0;
        for (;;) {
            if (xml.atEnd()) return damaged();
            const QXmlStreamReader::TokenType t = xml.readNext();
            if (t == QXmlStreamReader::StartElement) {
                ++depth;
                if (++m_depth > kMaxOdsDepth) return fail(say("The XML is nested more deeply than any real file's."));
                if (skipDepth) continue;
                const QStringView ns = xml.namespaceUri(), name = xml.name();
                if (ns == NS_TEXT) {
                    if (name == QLatin1String("p") || name == QLatin1String("h")) {
                        if (paragraphs++ > 0) text += QLatin1Char('\n');
                        inParagraph = true;
                        paragraphDepth = depth;
                    } else if (name == QLatin1String("span")) {
                        spanStack.append(attr(xml.attributes(), NS_TEXT, QLatin1String("style-name")));
                        if (spans.size() < 100000) spans.append({text.size(), spanStack.last()});
                    } else if (name == QLatin1String("s")) {
                        bool good = false;
                        const int n = attr(xml.attributes(), NS_TEXT, QLatin1String("c")).toInt(&good);
                        text += QString(good ? std::max(1, std::min(n, 10000)) : 1, QLatin1Char(' '));
                    } else if (name == QLatin1String("tab")) {
                        text += QLatin1Char('\t');
                    } else if (name == QLatin1String("line-break")) {
                        text += QLatin1Char('\n');
                    } else if (name == QLatin1String("note") || name == QLatin1String("note-body")) {
                        skipDepth = depth;
                    }
                } else {
                    skipDepth = depth;   // (a note, a drawing, or anything else in the cell is not read)
                }
                if (text.size() > m_limits.stringChars) tooLong = true;
            } else if (t == QXmlStreamReader::EndElement) {
                --m_depth;
                if (depth == 0) break;
                if (skipDepth) {
                    if (depth == skipDepth) skipDepth = 0;
                } else if (xml.namespaceUri() == NS_TEXT) {
                    if (xml.name() == QLatin1String("span") && !spanStack.isEmpty()) {
                        spanStack.removeLast();
                        if (spans.size() < 100000) spans.append({text.size(), spanStack.isEmpty() ? QString() : spanStack.last()});
                    } else if ((xml.name() == QLatin1String("p") || xml.name() == QLatin1String("h")) && depth == paragraphDepth) {
                        inParagraph = false;
                    }
                }
                --depth;
            } else if (t == QXmlStreamReader::Characters) {
                if (!skipDepth && inParagraph && !tooLong) {
                    text += xml.text();
                    if (text.size() > m_limits.stringChars) tooLong = true;
                }
            } else if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference) {
                return fail(say("The XML has a DOCTYPE, which is not allowed."));
            }
        }
        if (tooLong) return fail(say("A text in this file is longer than any real file's."));
        // ---- the value
        auto number = [&](const QString &v, double *x) {
            bool good = false;
            *x = v.toDouble(&good);
            return good;
        };
        if (calcType == QLatin1String("error")) {
            p.type = CellType::Error;
            bool known = false;
            CellError e = errorFromText(text.trimmed(), &known);
            if (!known) e = text.startsWith(QLatin1String("Err:50")) || text.startsWith(QLatin1String("Err:51")) ? CellError::Value : CellError::Value;
            p.number = double(int(e));
        } else if (valueType == QLatin1String("float") || valueType == QLatin1String("percentage") || valueType == QLatin1String("currency")) {
            double v = 0;
            if (number(attr(a, NS_OFFICE, QLatin1String("value")), &v)) {
                if (std::isfinite(v)) {
                    p.type = CellType::Number;
                    p.number = v;
                } else {
                    p.type = CellType::Error;
                    p.number = double(int(CellError::Num));
                }
            }
        } else if (valueType == QLatin1String("date")) {
            double serial = 0;
            if (dateSerialFrom(attr(a, NS_OFFICE, QLatin1String("date-value")), m_m.nullDate, &serial)) {
                p.type = CellType::Number;
                p.number = serial;
            } else {
                p.type = CellType::InlineString;
                p.text = attr(a, NS_OFFICE, QLatin1String("date-value"));
            }
        } else if (valueType == QLatin1String("time")) {
            static const QRegularExpression re(QStringLiteral("^(-)?P(?:([0-9]+)D)?(?:T(?:([0-9]+)H)?(?:([0-9]+)M)?(?:([0-9]+(?:\\.[0-9]*)?)S)?)?$"));
            const QRegularExpressionMatch m = re.match(attr(a, NS_OFFICE, QLatin1String("time-value")));
            if (m.hasMatch()) {
                const double days = m.captured(2).toDouble(), h = m.captured(3).toDouble(), mi = m.captured(4).toDouble(), s = m.captured(5).toDouble();
                p.type = CellType::Number;
                p.number = (m.captured(1).isEmpty() ? 1 : -1) * (days + h / 24 + mi / 1440 + s / 86400);
            }
        } else if (valueType == QLatin1String("boolean")) {
            const QString v = attr(a, NS_OFFICE, QLatin1String("boolean-value"));
            p.type = CellType::Bool;
            p.number = v == QLatin1String("true") ? 1 : 0;
        } else if (valueType == QLatin1String("string") || (valueType.isEmpty() && !text.isEmpty())) {
            p.type = CellType::InlineString;
            p.text = hasAttr(a, NS_OFFICE, QLatin1String("string-value")) ? attr(a, NS_OFFICE, QLatin1String("string-value")) : text;
            if (p.text.isEmpty() && formula.isEmpty()) p.type = CellType::Empty;
            else if (!formula.isEmpty()) p.type = CellType::FormulaString;
            // (the runs of formatting inside the text, if it is the text of the cell)
            if (!hasAttr(a, NS_OFFICE, QLatin1String("string-value"))) p.spans = spans;
        }
        // ---- the formula
        if (!formula.isEmpty()) {
            bool good = false;
            const QString converted = odfFormulaToText(formula, &good);
            if (good && qint64(converted.size()) <= m_limits.formulaChars) {
                p.hasFormula = true;
                p.formula = converted;
                if (p.type == CellType::InlineString) p.type = CellType::FormulaString;
                if (p.type == CellType::Empty && valueType == QLatin1String("string")) p.type = CellType::FormulaString;
                p.matrixCols = quint32(std::max<qint64>(0, attr(a, NS_TABLE, QLatin1String("number-matrix-columns-spanned")).toLongLong()));
                p.matrixRows = quint32(std::max<qint64>(0, attr(a, NS_TABLE, QLatin1String("number-matrix-rows-spanned")).toLongLong()));
            } else {
                p.formulaFailed = true;
                ++m_s.stats.unreadFormulas;
            }
        }
        return true;
    }

    // ---- making cells

    bool add(Cell c, qint64 extra)
    {
        if (++m_cells > m_limits.cellsPerSheet) return fail(say("The sheet has more cells than any real file has."));
        m_held += 32 + extra;
        if (m_held > m_limits.sheetBytes) return fail(say("The sheet is larger than any real file's."));
        m_s.cells.add(c);
        m_s.extentRows = std::max(m_s.extentRows, c.row + 1);
        m_s.extentCols = std::max(m_s.extentCols, quint32(c.col) + 1);
        if (m_progress && m_cells % 4096 == 0 && !m_progress(m_cells)) return fail(say("The read was stopped."));
        return true;
    }

    qint32 richFontIndex(const FontSpec &f)
    {
        for (int i = 0; i < m_s.richFonts.size(); ++i) {
            const FontSpec &g = m_s.richFonts[i];
            if (g.name == f.name && g.size == f.size && g.bold == f.bold && g.italic == f.italic && g.strike == f.strike && g.underline == f.underline && g.vertAlign == f.vertAlign
                && g.color.kind == f.color.kind && g.color.rgb == f.color.rgb)
                return i;
        }
        m_s.richFonts.append(f);
        return qint32(m_s.richFonts.size() - 1);
    }

    // The text of a prototype in the sheet's texts (once, however many cells it makes), with the runs of formatting it has.
    void putText(Proto *p)
    {
        if (p->textIndex >= 0) return;
        p->textIndex = qint32(m_s.texts.size());
        m_s.texts.append(p->text);
        m_s.stats.longestString = std::max<qint64>(m_s.stats.longestString, p->text.size());
        if (p->spans.isEmpty()) return;
        // the runs: the font of each stretch of the text, if it differs from the cell's
        const ods::StyleStore &store = m_m.found.store;
        RichRuns runs;
        bool differs = false;
        const ods::Props cellProps = store.resolve(QStringLiteral("table-cell"), m_m.nameOfXf.value(p->xf));
        const FontSpec cellFont = store.fontOf(cellProps);
        if (p->spans.first().first > 0) runs.append({0, RichRun::kCellFont});
        for (const auto &span : std::as_const(p->spans)) {
            RichRun r;
            r.start = quint32(span.first);
            if (span.second.isEmpty()) {
                r.font = RichRun::kCellFont;
            } else {
                ods::Props merged = cellProps;
                const ods::Props sp = store.resolve(QStringLiteral("text"), span.second);
                for (auto it = sp.cbegin(); it != sp.cend(); ++it) merged.insert(it.key(), it.value());
                const FontSpec f = store.fontOf(merged);
                if (f.name == cellFont.name && f.size == cellFont.size && f.bold == cellFont.bold && f.italic == cellFont.italic && f.strike == cellFont.strike && f.underline == cellFont.underline
                    && f.vertAlign == cellFont.vertAlign && f.color.rgb == cellFont.color.rgb && f.color.kind == cellFont.color.kind) {
                    r.font = RichRun::kCellFont;
                } else {
                    r.font = quint32(richFontIndex(f));
                    differs = true;
                }
            }
            if (!runs.isEmpty() && runs.last().start == r.start) runs.last() = r;
            else runs.append(r);
        }
        if (differs && m_s.richTexts.size() < 200000) m_s.richTexts.insert(p->textIndex, runs);
    }

    // The cells a prototype makes in a row: `count` of them from `col` (its repeats), in the row `row`, which is the `rowCopy`-th copy of a repeated row.
    bool emit(quint32 row, quint32 col, Proto &p, quint32 count, quint32 rowCopy)
    {
        for (quint32 k = 0; k < count; ++k) {
            Cell c;
            c.row = row;
            c.col = quint16(col + k);
            c.style = p.xf;
            qint64 extra = 0;
            switch (p.type) {
            case CellType::InlineString:
            case CellType::FormulaString: {
                const bool first = p.textIndex < 0;
                putText(&p);
                c.type = p.type;
                c.number = double(p.textIndex);
                if (first) extra = 32 + p.text.size() * 2;
                break;
            }
            case CellType::Empty: break;
            default:
                c.type = p.type;
                c.number = p.number;
                break;
            }
            if (p.hasFormula) {
                c.flags |= HasFormula;
                const quint32 copy = rowCopy + k;
                if (p.formulaTextIndex < 0) {
                    p.formulaTextIndex = qint32(m_s.formulaTexts.size());
                    m_s.formulaTexts.append(p.formula);
                    m_s.stats.longestFormula = std::max<qint64>(m_s.stats.longestFormula, p.formula.size());
                }
                Formula f;
                f.text = p.formulaTextIndex;
                f.dRow = qint32(rowCopy);
                f.dCol = qint32(k);
                if (p.matrixCols > 0 && p.matrixRows > 0 && k == 0 && rowCopy == 0) {
                    c.flags |= ArrayFormula;
                    CellRange range;
                    range.row1 = row;
                    range.col1 = col;
                    range.row2 = std::min<quint32>(row + p.matrixRows - 1, kMaxRows - 1);
                    range.col2 = std::min<quint32>(col + p.matrixCols - 1, kMaxColumns - 1);
                    range.valid = true;
                    m_s.ranges.append(range);
                    f.range = qint32(m_s.ranges.size() - 1);
                } else if (copy > 0) {
                    c.flags |= SharedFormula;
                }
                m_s.formulas.append(f);
                c.formula = qint32(m_s.formulas.size() - 1);
                ++m_s.formulaCells;
                if (c.type != CellType::Empty) ++m_s.storedResults;
                extra += 48;
            }
            if (!add(c, extra)) return false;
            // a merged cell: once, for the first of its copies
            if ((p.colSpan > 1 || p.rowSpan > 1) && k == 0 && rowCopy == 0 && !p.covered) {
                CellRange r;
                r.row1 = row;
                r.col1 = col;
                r.row2 = std::min<quint32>(row + p.rowSpan - 1, kMaxRows - 1);
                r.col2 = std::min<quint32>(col + p.colSpan - 1, kMaxColumns - 1);
                r.valid = true;
                ++m_s.mergeCount;
                if (m_s.merges.size() < m_limits.merges) m_s.merges.append(r);
            }
        }
        return true;
    }

    // ---- the end of the sheet

    void finish()
    {
        m_s.cells.finish();
        // columns: the width most of them have is the sheet's, the others have their own
        {
            QHash<qint64, qint64> widths;
            for (const ColRun &r : m_colRuns) widths[qint64(std::llround(widthOf(r) * 100))] += r.count;
            qint64 best = -1, bestCount = 0;
            for (auto it = widths.cbegin(); it != widths.cend(); ++it)
                if (it.value() > bestCount) {
                    bestCount = it.value();
                    best = it.key();
                }
            const double defaultPt = best >= 0 ? double(best) / 100 : -1;
            if (defaultPt > 0) m_s.format.defaultColWidth = defaultPt * 96.0 / 72.0 / m_m.maxDigitWidth;
            for (const ColRun &r : m_colRuns) {
                const double w = widthOf(r);
                const bool differs = w > 0 && std::abs(w - defaultPt) > 0.01;
                if (!differs && !r.hidden && r.xf == 0) continue;
                ColumnSpan span;
                span.first = r.first;
                span.last = r.first + r.count - 1;
                span.hidden = r.hidden;
                span.style = r.xf;
                if (w > 0) {
                    span.width = w * 96.0 / 72.0 / m_m.maxDigitWidth;
                    span.customWidth = true;
                }
                if (!m_s.columns.isEmpty()) {
                    ColumnSpan &last = m_s.columns.last();
                    if (last.last + 1 == span.first && last.hidden == span.hidden && last.style == span.style && last.width == span.width && last.customWidth == span.customWidth) {
                        last.last = span.last;
                        continue;
                    }
                }
                m_s.columns.append(span);
            }
        }
        // rows: the height most have is the sheet's
        {
            QHash<qint64, qint64> heights;
            for (const RowRun &r : m_rowRuns) heights[qint64(std::llround(heightOf(r) * 100))] += r.count;
            qint64 best = -1, bestCount = 0;
            for (auto it = heights.cbegin(); it != heights.cend(); ++it)
                if (it.value() > bestCount && it.key() > 0) {
                    bestCount = it.value();
                    best = it.key();
                }
            const double defaultPt = best > 0 ? double(best) / 100 : 15;
            m_s.format.defaultRowHeight = defaultPt;
            for (const RowRun &r : m_rowRuns) {
                const double h = heightOf(r);
                const bool differs = h > 0 && std::abs(h - defaultPt) > 0.01;
                if (!differs && !r.hidden && !(r.hasXf && r.xf != 0)) continue;
                for (quint32 k = 0; k < r.count; ++k) {
                    if (qint64(m_s.rowSpecs.size()) >= m_limits.rowSpecs) {
                        fail(say("The sheet has more rows set apart than any real file has."));
                        break;
                    }
                    RowSpec spec;
                    spec.row = r.first + k;
                    spec.hidden = r.hidden;
                    if (h > 0) {
                        spec.height = h;
                        spec.customHeight = !optimalHeight(r);
                    }
                    if (r.hasXf && r.xf != 0) {
                        spec.customFormat = true;
                        spec.style = r.xf;
                    }
                    m_s.rowSpecs.append(spec);
                }
            }
        }
        if (m_s.extentRows > 0) {
            m_s.dimension.row1 = 0;
            m_s.dimension.col1 = 0;
            m_s.dimension.row2 = m_s.extentRows - 1;
            m_s.dimension.col2 = m_s.extentCols - 1;
            m_s.dimension.valid = true;
        }
        m_s.isProtected = m_m.shapes[m_index].isProtected;
        m_s.tabColor = m_m.shapes[m_index].tabColor;
        m_s.view.rightToLeft = m_m.shapes[m_index].rightToLeft;
        view();
        m_s.stats.heldBytes = m_held;
        m_s.stats.xmlBytes = 0;
    }

    double widthOf(const ColRun &r) const
    {
        double pt = -1;
        const ods::Props p = m_m.found.store.resolve(QStringLiteral("table-column"), r.style);
        if (ods::lengthPoints(p.value(QStringLiteral("col:style:column-width")), &pt) && pt > 0) return pt;
        return -1;
    }

    double heightOf(const RowRun &r) const
    {
        double pt = -1;
        const ods::Props p = m_m.found.store.resolve(QStringLiteral("table-row"), r.style);
        if (ods::lengthPoints(p.value(QStringLiteral("row:style:row-height")), &pt) && pt > 0) return pt;
        return -1;
    }

    bool optimalHeight(const RowRun &r) const
    {
        return m_m.found.store.resolve(QStringLiteral("table-row"), r.style).value(QStringLiteral("row:style:use-optimal-row-height")) == QLatin1String("true");
    }

    // How the sheet was left on the screen, from the settings.
    void view()
    {
        SheetView &v = m_s.view;
        const QHash<QString, QString> items = m_m.found.sheetViews.value(m_m.shapes[m_index].name);
        auto integer = [&](const char *name, qint64 fallback) {
            bool ok = false;
            const qint64 x = items.value(QLatin1String(name)).toLongLong(&ok);
            return ok ? x : fallback;
        };
        v.tabSelected = m_m.found.viewItems.value(QStringLiteral("ActiveTable")) == m_m.shapes[m_index].name;
        const qint64 zoom = integer("ZoomValue", 100);
        v.zoom = int(std::max<qint64>(10, std::min<qint64>(400, zoom)));
        v.activeCol = quint32(std::max<qint64>(0, std::min<qint64>(integer("CursorPositionX", 0), kMaxColumns - 1)));
        v.activeRow = quint32(std::max<qint64>(0, std::min<qint64>(integer("CursorPositionY", 0), kMaxRows - 1)));
        const qint64 left = std::max<qint64>(0, integer("PositionLeft", 0)), top = std::max<qint64>(0, integer("PositionTop", 0));
        v.leftCol = quint32(std::min<qint64>(left, kMaxColumns - 1));
        v.topRow = quint32(std::min<qint64>(top, kMaxRows - 1));
        const bool hFixed = integer("HorizontalSplitMode", 0) == 2, vFixed = integer("VerticalSplitMode", 0) == 2;
        if (hFixed || vFixed) {
            const qint64 cols = hFixed ? std::max<qint64>(0, integer("HorizontalSplitPosition", 0) - left) : 0;
            const qint64 rows = vFixed ? std::max<qint64>(0, integer("VerticalSplitPosition", 0) - top) : 0;
            if (cols > 0 || rows > 0) {
                v.frozen = true;
                v.frozenCols = quint32(std::min<qint64>(cols, kMaxColumns - 1));
                v.frozenRows = quint32(std::min<qint64>(rows, kMaxRows - 1));
                v.paneCol = quint32(std::min<qint64>(std::max<qint64>(integer("PositionRight", left + cols), left + cols), kMaxColumns - 1));
                v.paneRow = quint32(std::min<qint64>(std::max<qint64>(integer("PositionBottom", top + rows), top + rows), kMaxRows - 1));
            }
        }
        const QString grid = m_m.found.viewItems.value(QStringLiteral("ShowGrid"));
        v.showGridLines = grid != QLatin1String("false");
        v.showZeros = m_m.found.viewItems.value(QStringLiteral("ShowZeroValues")) != QLatin1String("false");
    }

    const OdsBook::Impl &m_m;
    int m_index;
    Limits m_limits;
    std::function<bool(qint64)> m_progress;
    Sheet &m_s;
    QXmlStreamReader *m_xml = nullptr;
    jp::office::XmlGuardDevice *m_guard = nullptr;
    qint64 m_stopAfter = -1;
    bool m_stopped = false, m_rowsPast = false;
    int m_depth = 0;
    quint32 m_nextCol = 0;
    qint64 m_nextRow = 0;
    qint64 m_cells = 0, m_held = 0;
    std::vector<ColRun> m_colRuns;
    std::vector<RowRun> m_rowRuns;
    QVector<quint32> m_colXf;
    std::vector<Proto> m_row;
};

} // namespace

bool OdsBook::readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, qint64 stopAfterRow, qint64 screenRows, Sheet *out) const
{
    *out = Sheet();
    if (index < 0 || index >= m->slices.size()) {
        out->error = say("There is no such sheet.");
        return false;
    }
    out->format.defaultRowHeight = 15;
    SheetReader reader(*m, index, limits, progress, stopAfterRow, screenRows, out);
    return reader.run();
}

bool Workbook::openOpenDocument(const QByteArray &file, bool flat, QString *error, const Limits &limits)
{
    *this = Workbook();
    m_limits = limits;
    const auto book = OdsBook::open(file, flat, limits, &m_info, &m_shared, &m_styles, error);
    if (!book) {
        *this = Workbook();
        return false;
    }
    m_foreign = book;
    nameBaseParts();
    return true;
}

} // namespace jp::office::sheet
