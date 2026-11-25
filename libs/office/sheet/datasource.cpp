#include "sheet/datasource.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrl>
#include <QHash>
#include <QDate>
#include <QSet>
#include <QStringDecoder>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace jp::office::sheet {

namespace {

QString tooBig(const DataLimits &limits)
{
    return QCoreApplication::translate("jp::office::sheet::DataSource", "The file is larger than %1 MB, which is more than this program reads as data.").arg(limits.maxBytes >> 20);
}

QString clipped(const QString &text, const DataLimits &limits, bool *cut)
{
    if (text.size() <= limits.maxStringChars) return text;
    if (cut) *cut = true;
    return text.left(limits.maxStringChars);
}

// Whether a number as written can be held by a number cell: not too large, and not so small that it becomes zero.
bool numberFits(const QString &text)
{
    bool ok = false;
    const double d = text.toDouble(&ok);
    if (!ok || !std::isfinite(d)) return false;
    if (d != 0) return true;
    for (const QChar ch : text) {
        if (ch == QLatin1Char('e') || ch == QLatin1Char('E')) break;
        if (ch >= QLatin1Char('1') && ch <= QLatin1Char('9')) return false;
    }
    return true;
}

// The columns of a table being made, in the order they first appear, and the rows.
class Builder {
public:
    Builder(const DataLimits &limits, DataRows *out) : m_limits(limits), m_out(out) {}

    void newRow()
    {
        m_out->rows.push_back(QVector<DataCell>());
    }

    // `source` says where the value comes from (the path of keys, or of elements with their namespaces); two sources never share a column. `alt` is a second name to try before a number is added.
    void set(const QString &name, const QString &source, const DataCell &cell, const QString &alt = QString())
    {
        int col = m_index.value(source, -1);
        if (col < 0) {
            if (m_out->header.size() >= m_limits.maxCols) {
                m_overColumns = true;
                return;
            }
            col = m_out->header.size();
            QString shown = name;
            const auto taken = m_nameCol.constFind(shown.toLower());
            if (taken != m_nameCol.cend()) {
                ++m_renamed;
                // (the column that was there first is the one with a prefix: it takes its prefixed name, and the new one keeps the plain one)
                const int other = taken.value();
                const QString otherAlt = m_alts.value(other);
                if (!otherAlt.isEmpty() && otherAlt.toLower() != shown.toLower() && !m_nameCol.contains(otherAlt.toLower()) && (alt.isEmpty() || alt == name)) {
                    m_nameCol.remove(shown.toLower());
                    m_nameCol.insert(otherAlt.toLower(), other);
                    m_out->header[other] = otherAlt;
                } else if (!alt.isEmpty() && alt != name && !m_nameCol.contains(alt.toLower())) {
                    shown = alt;
                } else {
                    for (int n = 2; m_nameCol.contains(shown.toLower()); ++n) shown = name + QString::number(n);
                }
            }
            m_nameCol.insert(shown.toLower(), col);
            m_alts << alt;
            m_index.insert(source, col);
            m_out->header.push_back(shown);
        }
        QVector<DataCell> &row = m_out->rows.last();
        if (row.size() <= col) row.resize(col + 1);
        DataCell c = cell;
        c.text = clipped(c.text, m_limits, &m_cut);
        if (row[col].kind != 0 && c.kind == 's') {
            c.text = row[col].text + QStringLiteral("; ") + c.text;
            c.text = clipped(c.text, m_limits, &m_cut);
        }
        row[col] = c;
    }

    void finish()
    {
        const int n = m_out->header.size();
        for (QVector<DataCell> &row : m_out->rows) row.resize(n);
        if (m_overColumns) m_out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Only the first %1 columns are loaded; the rest are left out.").arg(m_limits.maxCols);
        if (m_cut) m_out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Text longer than %1 characters is cut.").arg(m_limits.maxStringChars);
        if (m_deep) m_out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Items inside items more than %1 levels deep are kept as text.").arg(m_limits.maxDepth);
        if (m_listCut) m_out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "A list of values with more than 10,000 items is cut after the first 10,000.");
        if (m_spreadCut) m_out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "A list of values with more than %1 items fills only the first %1 columns; the rest are left out.").arg(m_limits.arrayColumns);
        if (m_renamed > 0) m_out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "%n column(s) got a new name, because another column had the same name.", nullptr, m_renamed);
        m_out->notes << m_notes;
    }

    void deep() { m_deep = true; }
    void listCut() { m_listCut = true; }
    void spreadCut() { m_spreadCut = true; }
    void note(const QString &text) { m_notes << text; }

private:
    DataLimits m_limits;
    DataRows *m_out;
    QHash<QString, int> m_index;
    QHash<QString, int> m_nameCol;   // a column's name in lower case, and where it is
    QStringList m_alts;              // the second name each column could have
    QStringList m_notes;
    int m_renamed = 0;
    bool m_overColumns = false;
    bool m_cut = false;
    bool m_deep = false;
    bool m_listCut = false;
    bool m_spreadCut = false;
};

// ---- JSON ----
// A parser of our own, because the order of the keys in the file is the order of the columns (Qt's keeps them sorted), and because the work is bounded as it goes: the nodes made, the depth, and the length of a text.

struct JNode {
    enum Kind : char { Null, Bool, Number, String, Object, Array } kind = Null;
    bool flag = false;
    bool exactText = false;   // a number with more digits than a cell keeps whole: it is read as text
    QString text;             // a String, or the token of a Number
    std::vector<std::pair<QString, JNode>> members;   // an Object, in the file's order
    std::vector<JNode> items;                         // an Array
};

constexpr int kMaxConnectionChars = 80000;   // (the names of 256 columns, with the file's path and choices)
constexpr int kMaxJsonDepth = 64;
constexpr qint64 kMaxJsonNodes = 4000000;

class JsonParser {
public:
    JsonParser(const QByteArray &bytes, const DataLimits &limits) : m_s(bytes), m_limits(limits) {}
    bool parse(JNode *root)
    {
        skip();
        if (!value(root, 0)) return false;
        skip();
        if (m_i != m_s.size()) return fail();
        return true;
    }
    qint64 where() const { return m_i; }
    bool tooDeep() const { return m_deep; }
    bool tooMany() const { return m_many; }
    int duplicates() const { return m_dups; }
    int outOfRange() const { return m_range; }
    bool badCharacters() const { return m_badChars; }

private:
    bool fail() { return false; }
    bool failAt(qint64 at)
    {
        m_i = at;
        return false;
    }
    void skip()
    {
        while (m_i < m_s.size() && (m_s[m_i] == ' ' || m_s[m_i] == '\t' || m_s[m_i] == '\n' || m_s[m_i] == '\r')) ++m_i;
    }
    bool literal(const char *word)
    {
        const int n = int(strlen(word));
        if (m_s.mid(int(m_i), n) != word) return false;
        m_i += n;
        return true;
    }
    bool value(JNode *out, int depth)
    {
        if (++m_nodes > kMaxJsonNodes) {
            m_many = true;
            return false;
        }
        if (m_i >= m_s.size()) return fail();
        const char c = m_s[m_i];
        if (c == '{') return object(out, depth);
        if (c == '[') return array(out, depth);
        if (c == '"') {
            out->kind = JNode::String;
            return string(&out->text);
        }
        if (c == 't') {
            out->kind = JNode::Bool;
            out->flag = true;
            return literal("true");
        }
        if (c == 'f') {
            out->kind = JNode::Bool;
            return literal("false");
        }
        if (c == 'n') {
            out->kind = JNode::Null;
            return literal("null");
        }
        return number(out);
    }
    bool number(JNode *out)
    {
        // the forms JSON allows: -?(0|[1-9][0-9]*)(.[0-9]+)?([eE][+-]?[0-9]+)?
        const qint64 start = m_i;
        auto digit = [&] { return m_i < m_s.size() && m_s[m_i] >= '0' && m_s[m_i] <= '9'; };
        int digits = 0;
        if (m_i < m_s.size() && m_s[m_i] == '-') ++m_i;
        if (!digit()) return failAt(start);
        if (m_s[m_i] == '0') {
            ++m_i;
            ++digits;
            if (digit()) return failAt(start);
        } else {
            while (digit() && m_i - start <= 64) {
                ++m_i;
                ++digits;
            }
        }
        if (m_i < m_s.size() && m_s[m_i] == '.') {
            ++m_i;
            if (!digit()) return failAt(start);
            while (digit() && m_i - start <= 64) {
                ++m_i;
                ++digits;
            }
        }
        if (m_i < m_s.size() && (m_s[m_i] == 'e' || m_s[m_i] == 'E')) {
            ++m_i;
            if (m_i < m_s.size() && (m_s[m_i] == '+' || m_s[m_i] == '-')) ++m_i;
            if (!digit()) return failAt(start);
            while (digit() && m_i - start <= 64) ++m_i;
        }
        if (m_i - start > 64) return failAt(start);
        out->kind = JNode::Number;
        out->text = QString::fromLatin1(m_s.mid(int(start), int(m_i - start)));
        // (a number too large or too small for a double is kept as written, as text)
        const bool range = !numberFits(out->text);
        if (range) ++m_range;
        out->exactText = digits > 15 || range;
        return true;
    }
    // exactly four hex digits (the text "+1ab" or " 1a" is not a number here)
    bool hex4(ushort *u)
    {
        if (m_i + 4 > m_s.size()) return false;
        ushort v = 0;
        for (int k = 0; k < 4; ++k) {
            const char h = m_s[m_i + k];
            const int d = (h >= '0' && h <= '9') ? h - '0' : (h >= 'a' && h <= 'f') ? h - 'a' + 10 : (h >= 'A' && h <= 'F') ? h - 'A' + 10 : -1;
            if (d < 0) return false;
            v = ushort(v * 16 + d);
        }
        m_i += 4;
        *u = v;
        return true;
    }
    QString decode(const QByteArray &raw)
    {
        QStringDecoder dec(QStringDecoder::Utf8);
        const QString t = dec(raw);
        if (dec.hasError()) m_badChars = true;
        return t;
    }
    bool string(QString *out)
    {
        ++m_i;   // the quote
        QByteArray raw;
        QString text;
        while (true) {
            if (m_i >= m_s.size()) return fail();
            const char c = m_s[m_i++];
            if (c == '"') break;
            if (static_cast<unsigned char>(c) < 0x20) return fail();
            if (c != '\\') {
                raw += c;
            } else {
                if (m_i >= m_s.size()) return fail();
                const char e = m_s[m_i++];
                text += decode(raw);
                raw.clear();
                switch (e) {
                case '"': text += QLatin1Char('"'); break;
                case '\\': text += QLatin1Char('\\'); break;
                case '/': text += QLatin1Char('/'); break;
                case 'b': text += QLatin1Char('\b'); break;
                case 'f': text += QLatin1Char('\f'); break;
                case 'n': text += QLatin1Char('\n'); break;
                case 'r': text += QLatin1Char('\r'); break;
                case 't': text += QLatin1Char('\t'); break;
                case 'u': {
                    ushort u = 0;
                    if (!hex4(&u)) return failAt(m_i - 2);
                    if (u >= 0xD800 && u <= 0xDBFF && m_i + 6 <= m_s.size() && m_s[m_i] == '\\' && m_s[m_i + 1] == 'u') {
                        const qint64 keep = m_i;
                        m_i += 2;
                        ushort low = 0;
                        if (hex4(&low) && low >= 0xDC00 && low <= 0xDFFF) {
                            text += QChar(u);
                            text += QChar(low);
                            break;
                        }
                        m_i = keep;
                    }
                    if (u >= 0xD800 && u <= 0xDFFF) {
                        m_badChars = true;
                        text += QChar(0xFFFD);
                    } else {
                        text += QChar(u);
                    }
                    break;
                }
                default: return fail();
                }
            }
            // (a text far longer than a cell holds is kept to a little more than that: the rest is cut where it is made)
            if (raw.size() + text.size() > 4 * m_limits.maxStringChars + 16) {
                // keep reading to the closing quote without keeping more
                while (m_i < m_s.size() && m_s[m_i] != '"') m_i += (m_s[m_i] == '\\') ? 2 : 1;
                if (m_i >= m_s.size()) return fail();
                ++m_i;
                break;
            }
        }
        text += decode(raw);
        *out = text;
        return true;
    }
    bool object(JNode *out, int depth)
    {
        if (depth >= kMaxJsonDepth) {
            m_deep = true;
            return false;
        }
        out->kind = JNode::Object;
        ++m_i;
        skip();
        if (m_i < m_s.size() && m_s[m_i] == '}') {
            ++m_i;
            return true;
        }
        QHash<QString, int> seen;   // filled only for an object with many keys
        while (true) {
            skip();
            if (m_i >= m_s.size() || m_s[m_i] != '"') return fail();
            QString key;
            if (!string(&key)) return false;
            skip();
            if (m_i >= m_s.size() || m_s[m_i] != ':') return fail();
            ++m_i;
            skip();
            // (a key repeated in one object keeps its place and the last value, as the standard reader does)
            JNode member;
            if (!value(&member, depth + 1)) return false;
            int at = -1;
            if (out->members.size() <= 8) {
                for (size_t k = 0; k < out->members.size() && at < 0; ++k)
                    if (out->members[k].first == key) at = int(k);
            } else {
                if (seen.isEmpty())
                    for (size_t k = 0; k < out->members.size(); ++k) seen.insert(out->members[k].first, int(k));
                at = seen.value(key, -1);
            }
            if (at >= 0) {
                out->members[size_t(at)].second = std::move(member);
                ++m_dups;
            } else {
                if (!seen.isEmpty()) seen.insert(key, int(out->members.size()));
                out->members.emplace_back(key, std::move(member));
            }
            skip();
            if (m_i >= m_s.size()) return fail();
            if (m_s[m_i] == ',') {
                ++m_i;
                continue;
            }
            if (m_s[m_i] == '}') {
                ++m_i;
                return true;
            }
            return fail();
        }
    }
    bool array(JNode *out, int depth)
    {
        if (depth >= kMaxJsonDepth) {
            m_deep = true;
            return false;
        }
        out->kind = JNode::Array;
        ++m_i;
        skip();
        if (m_i < m_s.size() && m_s[m_i] == ']') {
            ++m_i;
            return true;
        }
        while (true) {
            skip();
            out->items.emplace_back();
            if (!value(&out->items.back(), depth + 1)) return false;
            skip();
            if (m_i >= m_s.size()) return fail();
            if (m_s[m_i] == ',') {
                ++m_i;
                continue;
            }
            if (m_s[m_i] == ']') {
                ++m_i;
                return true;
            }
            return fail();
        }
    }
    const QByteArray &m_s;
    DataLimits m_limits;
    qint64 m_i = 0;
    qint64 m_nodes = 0;
    bool m_deep = false, m_many = false, m_badChars = false;
    int m_dups = 0, m_range = 0;
};

struct JsonFlags {
    int duplicates = 0, outOfRange = 0;
    bool badChars = false;
};

bool parseJson(const QByteArray &bytes, const DataLimits &limits, JNode *root, QString *why, JsonFlags *flags = nullptr)
{
    if (bytes.size() > limits.maxBytes) {
        if (why) *why = tooBig(limits);
        return false;
    }
    // (a byte order mark is not part of the text)
    const QByteArray body = bytes.startsWith("\xEF\xBB\xBF") ? bytes.mid(3) : bytes;
    JsonParser parser(body, limits);
    if (!parser.parse(root)) {
        if (why) {
            if (parser.tooDeep()) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file has items inside items more deeply than any real file has.");
            else if (parser.tooMany()) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file has more values than this program reads as data.");
            else *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file is not valid JSON (near byte %1).").arg(parser.where());
        }
        return false;
    }
    if (flags) {
        flags->duplicates = parser.duplicates();
        flags->outOfRange = parser.outOfRange();
        flags->badChars = parser.badCharacters();
    }
    return true;
}

bool holdsObjects(const JNode &a)
{
    for (size_t i = 0; i < a.items.size() && i < 8; ++i)
        if (a.items[i].kind == JNode::Object) return true;
    return false;
}

void findJsonLists(const JNode &v, const QString &path, int depth, const DataLimits &limits, QStringList *out)
{
    if (out->size() >= limits.maxLists) return;
    if (v.kind == JNode::Array) {
        if (holdsObjects(v)) out->push_back(path);
        return;
    }
    if (v.kind == JNode::Object && depth < 4)
        for (const auto &m : v.members) findJsonLists(m.second, path + QLatin1Char('.') + m.first, depth + 1, limits, out);
}

const JNode *jsonAt(const JNode &root, const QString &path)
{
    const JNode *v = &root;
    if (path == QLatin1String("$")) return v;
    if (!path.startsWith(QLatin1String("$."))) return nullptr;
    // keys may hold dots: the longest key that matches is taken at each step
    QString rest = path.mid(2);
    while (!rest.isEmpty()) {
        if (v->kind != JNode::Object) return nullptr;
        const JNode *next = nullptr;
        for (int cut = int(rest.size()); cut > 0 && !next; --cut) {
            if (cut < rest.size() && rest.at(cut) != QLatin1Char('.')) continue;
            const QString key = rest.left(cut);
            for (const auto &m : v->members)
                if (m.first == key) {
                    next = &m.second;
                    rest = cut < rest.size() ? rest.mid(cut + 1) : QString();
                    break;
                }
        }
        if (!next) return nullptr;
        v = next;
    }
    return v;
}

QString compact(const JNode &v)
{
    switch (v.kind) {
    case JNode::Null: return QStringLiteral("null");
    case JNode::Bool: return v.flag ? QStringLiteral("true") : QStringLiteral("false");
    case JNode::Number: return v.text;
    case JNode::String: {
        QString q = v.text;
        q.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char('"'), QLatin1String("\\\""));
        return QLatin1Char('"') + q + QLatin1Char('"');
    }
    case JNode::Array: {
        QStringList parts;
        for (const JNode &i : v.items) parts << compact(i);
        return QLatin1Char('[') + parts.join(QLatin1Char(',')) + QLatin1Char(']');
    }
    case JNode::Object: {
        QStringList parts;
        for (const auto &m : v.members) parts << compact(JNode{JNode::String, false, false, m.first, {}, {}}) + QLatin1Char(':') + compact(m.second);
        return QLatin1Char('{') + parts.join(QLatin1Char(',')) + QLatin1Char('}');
    }
    }
    return QString();
}

DataCell scalarCell(const JNode &v)
{
    switch (v.kind) {
    case JNode::Bool: return DataCell{v.flag ? QStringLiteral("TRUE") : QStringLiteral("FALSE"), 'b'};
    case JNode::Number: return DataCell{v.text, v.exactText ? 's' : 'n'};
    case JNode::String: return DataCell{v.text, 's'};
    default: return DataCell();
    }
}

void flattenJson(Builder &b, const QString &key, const QString &src, const JNode &v, int depth, ArrayMode mode, const DataLimits &limits)
{
    switch (v.kind) {
    case JNode::Null:
    case JNode::Bool:
    case JNode::Number:
    case JNode::String: b.set(key, src, scalarCell(v)); break;
    case JNode::Object: {
        if (depth >= limits.maxDepth) {
            b.deep();
            b.set(key, src, DataCell{compact(v), 's'});
            break;
        }
        for (const auto &m : v.members) flattenJson(b, key + QLatin1Char('.') + m.first, src + QLatin1Char('\x1f') + m.first, m.second, depth + 1, mode, limits);
        break;
    }
    case JNode::Array: {
        bool plain = true;
        for (size_t i = 0; i < v.items.size() && plain; ++i) plain = v.items[i].kind != JNode::Object && v.items[i].kind != JNode::Array;
        if (!plain) {
            b.set(key, src, DataCell{compact(v), 's'});
            break;
        }
        if (mode == ArrayMode::Columns) {
            if (int(v.items.size()) > limits.arrayColumns) b.spreadCut();
            for (size_t i = 0; i < v.items.size() && int(i) < limits.arrayColumns; ++i) flattenJson(b, key + QLatin1Char('.') + QString::number(i + 1), src + QLatin1Char('\x1e') + QString::number(i + 1), v.items[i], depth + 1, mode, limits);
            break;
        }
        QStringList parts;
        if (v.items.size() > 10000) b.listCut();
        for (size_t i = 0; i < v.items.size() && i < 10000; ++i) parts << scalarCell(v.items[i]).text;
        b.set(key, src, DataCell{parts.join(QStringLiteral("; ")), 's'});
        break;
    }
    }
}

// ---- XML ----

bool refuseXml(QXmlStreamReader &xml, QString *why)
{
    if (why) *why = xml.hasError() ? QCoreApplication::translate("jp::office::sheet::DataSource", "The file is not valid XML (line %1).").arg(xml.lineNumber())
                                   : QCoreApplication::translate("jp::office::sheet::DataSource", "The file has a document type or entities, which this program does not read.");
    return false;
}

}   // namespace

QStringList dataJsonLists(const QByteArray &bytes, const DataLimits &limits, QString *why)
{
    JNode root;
    if (!parseJson(bytes, limits, &root, why)) return {};
    QStringList lists;
    if (root.kind == JNode::Array) {
        if (holdsObjects(root)) lists << QStringLiteral("$");
    } else {
        findJsonLists(root, QStringLiteral("$"), 0, limits, &lists);
    }
    if (lists.isEmpty() && why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file holds no list of items with named values to load.");
    return lists;
}

bool dataReadJson(const QByteArray &bytes, const QString &path, ArrayMode mode, int maxRows, const DataLimits &limits, DataRows *out, QString *why)
{
    JNode root;
    JsonFlags flags;
    if (!parseJson(bytes, limits, &root, why, &flags)) return false;
    const JNode *list = jsonAt(root, path);
    if (!list || list->kind != JNode::Array) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "That list is not in the file.");
        return false;
    }
    const int cap = maxRows > 0 ? std::min(maxRows, limits.maxRows) : limits.maxRows;
    *out = DataRows();
    Builder b(limits, out);
    int taken = 0;
    for (size_t i = 0; i < list->items.size() && taken < cap; ++i) {
        const JNode &item = list->items[i];
        b.newRow();
        ++taken;
        if (item.kind == JNode::Object) {
            for (const auto &m : item.members) flattenJson(b, m.first, m.first, m.second, 1, mode, limits);
        } else {
            flattenJson(b, QStringLiteral("value"), QStringLiteral("value"), item, 1, mode, limits);
        }
    }
    if (flags.duplicates > 0) b.note(QCoreApplication::translate("jp::office::sheet::DataSource", "The file repeats %n name(s) inside items; the last value of each is kept.", nullptr, flags.duplicates));
    if (flags.outOfRange > 0) b.note(QCoreApplication::translate("jp::office::sheet::DataSource", "%n number(s) are too large or too small for a cell, so they are kept as text, as written.", nullptr, flags.outOfRange));
    if (flags.badChars) b.note(QCoreApplication::translate("jp::office::sheet::DataSource", "Some characters in the file could not be read and are shown as a replacement mark."));
    b.finish();
    if (maxRows <= 0 && int(list->items.size()) > cap) out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Only the first %1 rows are loaded; the rest are left out.").arg(cap);
    if (out->header.isEmpty()) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The list has no values to load.");
        return false;
    }
    return true;
}

QStringList dataXmlLists(const QByteArray &bytes, const DataLimits &limits, QString *why)
{
    if (bytes.size() > limits.maxBytes) {
        if (why) *why = tooBig(limits);
        return {};
    }
    QXmlStreamReader xml(bytes);
    QStringList stack;
    QVector<QHash<QString, int>> counts;
    QSet<QString> seen;
    QStringList lists;
    while (!xml.atEnd()) {
        const auto t = xml.readNext();
        if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference || t == QXmlStreamReader::Invalid) {
            refuseXml(xml, why);
            return {};
        }
        if (t == QXmlStreamReader::StartElement) {
            if (stack.size() >= limits.maxXmlDepth) {
                if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file has items inside items more deeply than any real file has.");
                return {};
            }
            const QString path = (stack.isEmpty() ? QString() : stack.last()) + QLatin1Char('/') + xml.name().toString();
            if (!counts.isEmpty()) {
                int &n = counts.last()[path];
                if (++n == 2 && !seen.contains(path) && lists.size() < limits.maxLists) {
                    seen.insert(path);
                    lists << path;
                }
                if (counts.last().size() > 4096) {
                    if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file has more kinds of items than any real file has.");
                    return {};
                }
            }
            stack.push_back(path);
            counts.push_back(QHash<QString, int>());
        } else if (t == QXmlStreamReader::EndElement) {
            stack.pop_back();
            counts.pop_back();
        }
    }
    if (xml.hasError()) {
        refuseXml(xml, why);
        return {};
    }
    if (lists.isEmpty() && why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file holds no item that repeats, so there are no rows to load.");
    return lists;
}

namespace {

// Reads one element's content (the start tag was just read) into columns named from `key` (the names without prefixes); `qkey` is the same with the prefixes, tried when two columns would share a name, and `src` names the
// source with namespaces, so that two different sources never share a column.
bool readXmlElement(QXmlStreamReader &xml, Builder &b, const QString &key, const QString &qkey, const QString &src, int depth, const DataLimits &limits, QString *why)
{
    auto join = [](const QString &base, const QString &name, QChar sep) { return base.isEmpty() ? name : base + sep + name; };
    const QXmlStreamAttributes attrs = xml.attributes();
    for (const QXmlStreamAttribute &a : attrs)
        b.set(join(key, a.name().toString(), QLatin1Char('.')), join(src, QLatin1Char('@') + a.namespaceUri().toString() + QLatin1Char('|') + a.name().toString(), QLatin1Char('\x1f')),
              DataCell{a.value().toString(), 's'}, join(qkey, a.qualifiedName().toString(), QLatin1Char('.')));
    QString text;
    bool children = false;
    while (!xml.atEnd()) {
        const auto t = xml.readNext();
        if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference || t == QXmlStreamReader::Invalid) return refuseXml(xml, why);
        if (t == QXmlStreamReader::Characters || t == QXmlStreamReader::EntityReference) {
            if (text.size() <= limits.maxStringChars) text += xml.text().toString();
        } else if (t == QXmlStreamReader::StartElement) {
            children = true;
            if (depth >= limits.maxXmlDepth) {
                if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file has items inside items more deeply than any real file has.");
                return false;
            }
            const QString sub = join(key, xml.name().toString(), QLatin1Char('.'));
            const QString qsub = join(qkey, xml.qualifiedName().toString(), QLatin1Char('.'));
            const QString ssub = join(src, xml.namespaceUri().toString() + QLatin1Char('|') + xml.name().toString(), QLatin1Char('\x1f'));
            if (depth + 1 > limits.maxDepth) {
                b.deep();
                b.set(sub, ssub, DataCell{xml.readElementText(QXmlStreamReader::IncludeChildElements), 's'}, qsub);
                if (xml.hasError()) return refuseXml(xml, why);
                continue;
            }
            if (!readXmlElement(xml, b, sub, qsub, ssub, depth + 1, limits, why)) return false;
        } else if (t == QXmlStreamReader::EndElement) {
            break;
        }
    }
    if (xml.hasError()) return refuseXml(xml, why);
    const QString trimmed = text.trimmed();
    // (the text of an element that also holds elements: in a column named for the element, or "text" at the top)
    if (!children && !trimmed.isEmpty()) b.set(key.isEmpty() ? QStringLiteral("text") : key, src.isEmpty() ? QStringLiteral("#text") : src, DataCell{trimmed, 's'}, qkey);
    else if (children && !trimmed.isEmpty()) b.set(key.isEmpty() ? QStringLiteral("text") : key + QStringLiteral(".text"), src.isEmpty() ? QStringLiteral("#text") : src + QStringLiteral("\x1f#text"), DataCell{trimmed, 's'}, key.isEmpty() ? QString() : qkey + QStringLiteral(".text"));
    return true;
}

}   // namespace

bool dataReadXml(const QByteArray &bytes, const QString &path, int maxRows, const DataLimits &limits, DataRows *out, QString *why)
{
    if (bytes.size() > limits.maxBytes) {
        if (why) *why = tooBig(limits);
        return false;
    }
    *out = DataRows();
    Builder b(limits, out);
    const int cap = maxRows > 0 ? std::min(maxRows, limits.maxRows) : limits.maxRows;
    QXmlStreamReader xml(bytes);
    QStringList stack;
    bool more = false;
    while (!xml.atEnd()) {
        const auto t = xml.readNext();
        if (t == QXmlStreamReader::DTD || t == QXmlStreamReader::EntityReference || t == QXmlStreamReader::Invalid) {
            refuseXml(xml, why);
            return false;
        }
        if (t == QXmlStreamReader::StartElement) {
            if (stack.size() >= limits.maxXmlDepth) {
                if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file has items inside items more deeply than any real file has.");
                return false;
            }
            const QString here = (stack.isEmpty() ? QString() : stack.last()) + QLatin1Char('/') + xml.name().toString();
            if (here == path) {
                if (out->rows.size() >= cap) {
                    more = true;
                    break;
                }
                b.newRow();
                if (!readXmlElement(xml, b, QString(), QString(), QString(), 0, limits, why)) return false;
                continue;   // its end tag was read
            }
            stack.push_back(here);
        } else if (t == QXmlStreamReader::EndElement) {
            if (!stack.isEmpty()) stack.pop_back();
        }
    }
    if (!more && xml.hasError()) {
        refuseXml(xml, why);
        return false;
    }
    b.finish();
    if (maxRows <= 0 && more) out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Only the first %1 rows are loaded; the rest are left out.").arg(cap);
    if (out->rows.isEmpty() || out->header.isEmpty()) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "That item is not in the file, or holds no values to load.");
        return false;
    }
    return true;
}

bool dataPathIsRemote(const QString &path)
{
    const QString p = path.trimmed();
    if (p.startsWith(QLatin1String("\\\\")) || p.startsWith(QLatin1String("//"))) return true;
    static const QRegularExpression scheme(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]+:"));
    const QRegularExpressionMatch m = scheme.match(p);
    // (one letter and a colon is a drive; a longer word and a colon is an address)
    return m.hasMatch();
}

QString DataConnection::text() const
{
    auto enc = [](const QString &v) { return QString::fromLatin1(v.toUtf8().toPercentEncoding()); };
    QString text = QStringLiteral("jo-data:1?kind=%1&path=%2&list=%3&mode=%4&text=%5").arg(enc(kind), enc(path), enc(list), mode == ArrayMode::Columns ? QStringLiteral("columns") : QStringLiteral("join"), asText ? QStringLiteral("1") : QStringLiteral("0"));
    if (!columns.isEmpty()) {
        QStringList names;
        for (const QString &c : columns) names << enc(c);
        const QString all = enc(names.join(QLatin1Char(',')));
        if (text.size() + all.size() + 6 <= kMaxConnectionChars) text += QStringLiteral("&cols=") + all;
    }
    return text;
}

bool DataConnection::parse(const QString &text, DataConnection *out)
{
    if (!text.startsWith(QLatin1String("jo-data:1?")) || text.size() > kMaxConnectionChars) return false;
    DataConnection c;
    const QStringList parts = text.mid(10).split(QLatin1Char('&'));
    for (const QString &part : parts) {
        const int eq = int(part.indexOf(QLatin1Char('=')));
        if (eq < 0) return false;
        const QString key = part.left(eq);
        const QString value = QUrl::fromPercentEncoding(part.mid(eq + 1).toLatin1());
        if (key == QLatin1String("kind")) c.kind = value;
        else if (key == QLatin1String("path")) c.path = value;
        else if (key == QLatin1String("list")) c.list = value;
        else if (key == QLatin1String("mode")) c.mode = value == QLatin1String("columns") ? ArrayMode::Columns : ArrayMode::Join;
        else if (key == QLatin1String("text")) c.asText = value == QLatin1String("1");
        else if (key == QLatin1String("cols")) {
            for (const QString &name : value.split(QLatin1Char(','))) c.columns << QUrl::fromPercentEncoding(name.toLatin1());
            if (c.columns.size() > 4096) return false;
        }
    }
    if (c.kind != QLatin1String("json") && c.kind != QLatin1String("xml") && c.kind != QLatin1String("workbook")) return false;
    if (c.path.isEmpty()) return false;
    *out = c;
    return true;
}

QVector<char> dataColumnKinds(const DataRows &table, bool asText)
{
    QVector<char> kinds(table.header.size(), 't');
    if (asText) return kinds;
    static const QRegularExpression number(QStringLiteral("^-?(0|[1-9][0-9]*)(\\.[0-9]+)?([eE][+-]?[0-9]+)?$"));
    static const QRegularExpression date(QStringLiteral("^[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$"));
    for (int c = 0; c < kinds.size(); ++c) {
        bool anyN = false, allN = true, allB = true, allD = true, any = false;
        for (const QVector<DataCell> &row : table.rows) {
            const DataCell cell = row.value(c);
            if (cell.kind == 0 && cell.text.isEmpty()) continue;
            any = true;
            const bool numeric = cell.kind == 'n' || (cell.kind == 's' && number.match(cell.text).hasMatch() && cell.text.size() <= 16 && numberFits(cell.text));
            allN = allN && numeric && (cell.kind == 'n' || !cell.text.startsWith(QLatin1Char('0')) || cell.text.startsWith(QLatin1String("0.")) || cell.text == QLatin1String("0"));
            anyN = anyN || numeric;
            allB = allB && (cell.kind == 'b' || (cell.kind == 's' && (cell.text.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0 || cell.text.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0)));
            allD = allD && cell.kind == 's' && date.match(cell.text).hasMatch() && QDate::fromString(cell.text, Qt::ISODate).isValid();
        }
        if (!any) continue;
        if (allB) kinds[c] = 'b';
        else if (allN) kinds[c] = 'n';
        else if (allD) kinds[c] = 'd';
        (void)anyN;
    }
    return kinds;
}

bool dataReadFile(const QString &path, const DataLimits &limits, const std::function<bool()> &cancelled, QByteArray *bytes, QString *why)
{
    auto fail = [&](const QString &w) {
        if (why) *why = w;
        return false;
    };
    if (dataPathIsRemote(path)) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "Only a file on this computer can be loaded, not a web address or a file on another computer."));
    const QFileInfo info(path);
    if (!info.exists()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The file is not there."));
    if (!info.isFile()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "That is not a file."));
    if (info.size() > limits.maxBytes) return fail(tooBig(limits));
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The file can't be opened."));
    QByteArray all;
    while (!f.atEnd()) {
        if (cancelled && cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::DataSource", "The read was stopped."));
        const QByteArray piece = f.read(1 << 20);
        if (piece.isEmpty()) break;
        if (all.size() + piece.size() > limits.maxBytes) return fail(tooBig(limits));
        all += piece;
    }
    *bytes = all;
    return true;
}

}   // namespace jp::office::sheet
