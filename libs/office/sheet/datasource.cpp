#include "sheet/datasource.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QXmlStreamReader>

#include <algorithm>

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

// The columns of a table being made, in the order they first appear, and the rows.
class Builder {
public:
    Builder(const DataLimits &limits, DataTable *out) : m_limits(limits), m_out(out) {}

    void newRow()
    {
        m_out->rows.push_back(QVector<DataCell>());
    }

    void set(const QString &key, const DataCell &cell)
    {
        int col = m_index.value(key, -1);
        if (col < 0) {
            if (m_out->header.size() >= m_limits.maxCols) {
                m_overColumns = true;
                return;
            }
            col = m_out->header.size();
            m_index.insert(key, col);
            m_out->header.push_back(key);
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
    }

    void deep() { m_deep = true; }

private:
    DataLimits m_limits;
    DataTable *m_out;
    QHash<QString, int> m_index;
    bool m_overColumns = false;
    bool m_cut = false;
    bool m_deep = false;
};

// ---- JSON ----

bool parseJson(const QByteArray &bytes, const DataLimits &limits, QJsonDocument *doc, QString *why)
{
    if (bytes.size() > limits.maxBytes) {
        if (why) *why = tooBig(limits);
        return false;
    }
    QJsonParseError err;
    *doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file is not valid JSON (near byte %1).").arg(err.offset);
        return false;
    }
    return true;
}

bool holdsObjects(const QJsonArray &a)
{
    for (int i = 0; i < a.size() && i < 8; ++i)
        if (a.at(i).isObject()) return true;
    return false;
}

void findJsonLists(const QJsonValue &v, const QString &path, int depth, const DataLimits &limits, QStringList *out)
{
    if (out->size() >= limits.maxLists) return;
    if (v.isArray()) {
        if (holdsObjects(v.toArray())) out->push_back(path);
        return;
    }
    if (v.isObject() && depth < 4) {
        const QJsonObject o = v.toObject();
        QStringList keys = o.keys();
        for (const QString &k : keys) findJsonLists(o.value(k), path + QLatin1Char('.') + k, depth + 1, limits, out);
    }
}

QJsonValue jsonAt(const QJsonDocument &doc, const QString &path)
{
    QJsonValue v = doc.isArray() ? QJsonValue(doc.array()) : QJsonValue(doc.object());
    if (path == QLatin1String("$")) return v;
    if (!path.startsWith(QLatin1String("$."))) return QJsonValue::Undefined;
    // keys may hold dots: take the longest key that matches at each step
    QString rest = path.mid(2);
    while (!rest.isEmpty()) {
        if (!v.isObject()) return QJsonValue::Undefined;
        const QJsonObject o = v.toObject();
        bool found = false;
        for (int cut = rest.size(); cut > 0 && !found; --cut) {
            if (cut < rest.size() && rest.at(cut) != QLatin1Char('.')) continue;
            const QString key = rest.left(cut);
            if (o.contains(key)) {
                v = o.value(key);
                rest = cut < rest.size() ? rest.mid(cut + 1) : QString();
                found = true;
            }
        }
        if (!found) return QJsonValue::Undefined;
    }
    return v;
}

QString numberText(double d)
{
    if (d == double(qint64(d)) && std::abs(d) < 1e15) return QString::number(qint64(d));
    return QString::number(d, 'g', 15);
}

QString compact(const QJsonValue &v)
{
    if (v.isArray()) return QString::fromUtf8(QJsonDocument(v.toArray()).toJson(QJsonDocument::Compact));
    return QString::fromUtf8(QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact));
}

void flattenJson(Builder &b, const QString &key, const QJsonValue &v, int depth, ArrayMode mode, const DataLimits &limits)
{
    switch (v.type()) {
    case QJsonValue::Null:
    case QJsonValue::Undefined: b.set(key, DataCell()); break;
    case QJsonValue::Bool: b.set(key, DataCell{v.toBool() ? QStringLiteral("TRUE") : QStringLiteral("FALSE"), 'b'}); break;
    case QJsonValue::Double: b.set(key, DataCell{numberText(v.toDouble()), 'n'}); break;
    case QJsonValue::String: b.set(key, DataCell{v.toString(), 's'}); break;
    case QJsonValue::Object: {
        if (depth >= limits.maxDepth) {
            b.deep();
            b.set(key, DataCell{compact(v), 's'});
            break;
        }
        const QJsonObject o = v.toObject();
        for (auto it = o.begin(); it != o.end(); ++it) flattenJson(b, key + QLatin1Char('.') + it.key(), it.value(), depth + 1, mode, limits);
        break;
    }
    case QJsonValue::Array: {
        const QJsonArray a = v.toArray();
        bool plain = true;
        for (int i = 0; i < a.size() && plain; ++i) plain = !a.at(i).isObject() && !a.at(i).isArray();
        if (!plain) {
            b.set(key, DataCell{compact(v), 's'});
            break;
        }
        if (mode == ArrayMode::Columns) {
            for (int i = 0; i < a.size() && i < limits.arrayColumns; ++i) flattenJson(b, key + QLatin1Char('.') + QString::number(i + 1), a.at(i), depth + 1, mode, limits);
            break;
        }
        QStringList parts;
        for (int i = 0; i < a.size() && i < 10000; ++i) {
            const QJsonValue e = a.at(i);
            parts << (e.isDouble() ? numberText(e.toDouble()) : e.isBool() ? (e.toBool() ? QStringLiteral("TRUE") : QStringLiteral("FALSE")) : e.toString());
        }
        b.set(key, DataCell{parts.join(QStringLiteral("; ")), 's'});
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
    QJsonDocument doc;
    if (!parseJson(bytes, limits, &doc, why)) return {};
    QStringList lists;
    if (doc.isArray()) {
        if (holdsObjects(doc.array())) lists << QStringLiteral("$");
    } else {
        findJsonLists(QJsonValue(doc.object()), QStringLiteral("$"), 0, limits, &lists);
    }
    if (lists.isEmpty() && why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "The file holds no list of items with named values to load.");
    return lists;
}

bool dataReadJson(const QByteArray &bytes, const QString &path, ArrayMode mode, int maxRows, const DataLimits &limits, DataTable *out, QString *why)
{
    QJsonDocument doc;
    if (!parseJson(bytes, limits, &doc, why)) return false;
    const QJsonValue list = jsonAt(doc, path);
    if (!list.isArray()) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::DataSource", "That list is not in the file.");
        return false;
    }
    const QJsonArray a = list.toArray();
    const int cap = maxRows > 0 ? std::min(maxRows, limits.maxRows) : limits.maxRows;
    *out = DataTable();
    Builder b(limits, out);
    int taken = 0;
    for (int i = 0; i < a.size() && taken < cap; ++i) {
        const QJsonValue item = a.at(i);
        b.newRow();
        ++taken;
        if (item.isObject()) {
            const QJsonObject o = item.toObject();
            for (auto it = o.begin(); it != o.end(); ++it) flattenJson(b, it.key(), it.value(), 1, mode, limits);
        } else {
            flattenJson(b, QStringLiteral("value"), item, 1, mode, limits);
        }
    }
    b.finish();
    if (maxRows <= 0 && a.size() > cap) out->notes << QCoreApplication::translate("jp::office::sheet::DataSource", "Only the first %1 rows are loaded; the rest are left out.").arg(cap);
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

// Reads one element's content (the start tag was just read) into columns named from `key`.
bool readXmlElement(QXmlStreamReader &xml, Builder &b, const QString &key, int depth, const DataLimits &limits, QString *why)
{
    const QXmlStreamAttributes attrs = xml.attributes();
    for (const QXmlStreamAttribute &a : attrs) b.set(key.isEmpty() ? a.name().toString() : key + QLatin1Char('.') + a.name().toString(), DataCell{a.value().toString(), 's'});
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
            const QString name = xml.name().toString();
            const QString sub = key.isEmpty() ? name : key + QLatin1Char('.') + name;
            if (depth + 1 > limits.maxDepth) {
                b.deep();
                b.set(sub, DataCell{xml.readElementText(QXmlStreamReader::IncludeChildElements), 's'});
                if (xml.hasError()) return refuseXml(xml, why);
                continue;
            }
            if (!readXmlElement(xml, b, sub, depth + 1, limits, why)) return false;
        } else if (t == QXmlStreamReader::EndElement) {
            break;
        }
    }
    if (xml.hasError()) return refuseXml(xml, why);
    const QString trimmed = text.trimmed();
    if (!children && !trimmed.isEmpty()) b.set(key.isEmpty() ? QStringLiteral("text") : key, DataCell{trimmed, 's'});
    else if (children && !trimmed.isEmpty() && !key.isEmpty()) b.set(key + QStringLiteral(".text"), DataCell{trimmed, 's'});
    return true;
}

}   // namespace

bool dataReadXml(const QByteArray &bytes, const QString &path, int maxRows, const DataLimits &limits, DataTable *out, QString *why)
{
    if (bytes.size() > limits.maxBytes) {
        if (why) *why = tooBig(limits);
        return false;
    }
    *out = DataTable();
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
                if (!readXmlElement(xml, b, QString(), 0, limits, why)) return false;
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

}   // namespace jp::office::sheet
