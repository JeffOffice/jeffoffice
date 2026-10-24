#include "core/clipdata.h"

#include <QBuffer>
#include <QDataStream>
#include <QRegularExpression>

namespace jp::crunch {

using jp::office::sheet::Calculation;
using jp::office::sheet::CellRange;
using jp::calc::Value;

namespace {

constexpr quint32 kMagic = 0x4A434C50;   // "JCLP"
constexpr quint32 kVersion = 1;

void writeValue(QDataStream &s, const Value &v)
{
    s << qint32(v.kind());
    switch (v.kind()) {
    case Value::Kind::Number: s << v.number(); break;
    case Value::Kind::Bool: s << qint32(v.boolean() ? 1 : 0); break;
    case Value::Kind::Error: s << qint32(int(v.errorKind())); break;
    case Value::Kind::Text: s << v.text(); break;
    default: break;
    }
}

bool readValue(QDataStream &s, Value *out)
{
    qint32 kind = 0;
    s >> kind;
    switch (Value::Kind(kind)) {
    case Value::Kind::Number: {
        double n = 0;
        s >> n;
        *out = Value::number(n);
        break;
    }
    case Value::Kind::Bool: {
        qint32 b = 0;
        s >> b;
        *out = Value::boolean(b != 0);
        break;
    }
    case Value::Kind::Error: {
        qint32 e = 0;
        s >> e;
        *out = Value::error(jp::calc::ErrorKind(e));
        break;
    }
    case Value::Kind::Text: {
        QString t;
        s >> t;
        *out = Value::text(t);
        break;
    }
    default: *out = Value(); break;
    }
    return s.status() == QDataStream::Ok;
}

} // namespace

QByteArray ClipData::toBytes() const
{
    QByteArray bytes;
    QDataStream s(&bytes, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << kMagic << kVersion << token << serial << qint32(sheet) << source.row1 << source.col1 << source.row2 << source.col2 << rows << cols << cut << quint32(cells.size());
    for (const ClipCell &c : cells) {
        s << c.dRow << c.dCol << c.content.exists << c.content.isFormula << c.content.formula << c.content.style << c.formatCode << c.content.hasStored;
        writeValue(s, c.content.value);
        writeValue(s, c.shown);
        if (c.content.hasStored) writeValue(s, c.content.stored);
    }
    s << quint32(merges.size());
    for (const CellRange &m : merges) s << m.row1 << m.col1 << m.row2 << m.col2;
    return bytes;
}

bool ClipData::fromBytes(const QByteArray &bytes, ClipData *out)
{
    // (what the bytes say is not believed: the sizes are held to the grid and to the bytes there are, before anything is made from them)
    QDataStream s(bytes);
    s.setVersion(QDataStream::Qt_6_0);
    quint32 magic = 0, version = 0, count = 0;
    qint32 sheetIndex = 0;
    ClipData d;
    s >> magic >> version;
    if (magic != kMagic || version != kVersion) return false;
    s >> d.token >> d.serial >> sheetIndex >> d.source.row1 >> d.source.col1 >> d.source.row2 >> d.source.col2 >> d.rows >> d.cols >> d.cut >> count;
    d.sheet = sheetIndex;
    d.source.valid = true;
    if (s.status() != QDataStream::Ok || sheetIndex < 0) return false;
    if (d.rows == 0 || d.cols == 0 || d.rows > jp::calc::kGridRows || d.cols > jp::calc::kGridCols) return false;
    if (d.source.row1 > d.source.row2 || d.source.col1 > d.source.col2 || d.source.row2 >= jp::calc::kGridRows || d.source.col2 >= jp::calc::kGridCols) return false;
    // each cell takes at least this many bytes (its place, flags, an empty formula and format code, and two values), and there can be no more cells than the block has
    constexpr qint64 kLeastCellBytes = 28, kLeastMergeBytes = 16;
    const qint64 left = bytes.size() - s.device()->pos();
    if (qint64(count) > left / kLeastCellBytes || qint64(count) > qint64(d.rows) * qint64(d.cols)) return false;
    for (quint32 i = 0; i < count; ++i) {
        ClipCell c;
        s >> c.dRow >> c.dCol >> c.content.exists >> c.content.isFormula >> c.content.formula >> c.content.style >> c.formatCode >> c.content.hasStored;
        if (s.status() != QDataStream::Ok || c.dRow >= d.rows || c.dCol >= d.cols) return false;
        if (!readValue(s, &c.content.value) || !readValue(s, &c.shown)) return false;
        if (c.content.hasStored && !readValue(s, &c.content.stored)) return false;
        d.cells.push_back(std::move(c));
    }
    quint32 merges = 0;
    s >> merges;
    if (s.status() != QDataStream::Ok || qint64(merges) > (bytes.size() - s.device()->pos()) / kLeastMergeBytes) return false;
    for (quint32 i = 0; i < merges; ++i) {
        CellRange m;
        s >> m.row1 >> m.col1 >> m.row2 >> m.col2;
        m.valid = true;
        if (s.status() != QDataStream::Ok || m.row1 > m.row2 || m.col1 > m.col2 || m.row2 >= d.rows || m.col2 >= d.cols) return false;
        d.merges.push_back(m);
    }
    if (s.status() != QDataStream::Ok) return false;
    *out = std::move(d);
    return true;
}

QVector<QStringList> parseTabSeparated(const QString &text)
{
    QVector<QStringList> rows;
    QStringList row;
    QString field;
    bool quoted = false, any = false;
    qint64 total = 0;   // the cells made so far: past the most there can be, the rest is not read
    const int n = int(text.size());
    for (int i = 0; i < n && total < kMaxClipCells; ++i) {
        const QChar c = text[i];
        if (quoted) {
            if (c == QLatin1Char('"')) {
                if (i + 1 < n && text[i + 1] == QLatin1Char('"')) {
                    field += QLatin1Char('"');
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field += c;
            }
            continue;
        }
        if (c == QLatin1Char('"') && field.isEmpty()) {
            quoted = true;
            any = true;
        } else if (c == QLatin1Char('\t')) {
            row << field;
            ++total;
            field.clear();
            any = true;
        } else if (c == QLatin1Char('\n') || c == QLatin1Char('\r')) {
            if (c == QLatin1Char('\r') && i + 1 < n && text[i + 1] == QLatin1Char('\n')) ++i;
            row << field;
            ++total;
            rows.append(row);
            row.clear();
            field.clear();
            any = false;
        } else {
            field += c;
            any = true;
        }
    }
    // (a text that ends with a line break has no row after it)
    if (any || !field.isEmpty() || !row.isEmpty()) {
        row << field;
        rows.append(row);
    }
    return rows;
}

namespace {

QString decodeEntities(QString s)
{
    s.replace(QStringLiteral("&nbsp;"), QStringLiteral(" ")).replace(QStringLiteral("&lt;"), QStringLiteral("<")).replace(QStringLiteral("&gt;"), QStringLiteral(">")).replace(QStringLiteral("&quot;"), QStringLiteral("\""))
        .replace(QStringLiteral("&apos;"), QStringLiteral("'")).replace(QStringLiteral("&#39;"), QStringLiteral("'"));
    static const QRegularExpression numeric(QStringLiteral("&#(x?)([0-9a-fA-F]+);"));
    QString out;
    int at = 0;
    QRegularExpressionMatchIterator it = numeric.globalMatch(s);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out += s.mid(at, m.capturedStart() - at);
        bool ok = false;
        const uint code = m.captured(2).toUInt(&ok, m.captured(1).isEmpty() ? 10 : 16);
        if (ok && code > 0 && code < 0x110000) out += QString::fromUcs4(reinterpret_cast<const char32_t *>(&code), 1);
        at = m.capturedEnd();
    }
    out += s.mid(at);
    out.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    return out;
}

} // namespace

QVector<QStringList> parseHtmlTable(const QString &html)
{
    QVector<QStringList> rows;
    // the first table: its rows and their cells (a table inside a cell is read as text)
    static const QRegularExpression tableStart(QStringLiteral("<table\\b"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression rowRe(QStringLiteral("<tr\\b[^>]*>(.*?)</tr\\s*>"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression cellRe(QStringLiteral("<t([dh])\\b([^>]*)>(.*?)(?=</t[dh]\\s*>|<t[dh]\\b|</tr)"), QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression tag(QStringLiteral("<[^>]*>"));
    static const QRegularExpression breakRe(QStringLiteral("<br\\s*/?>"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression span(QStringLiteral("colspan\\s*=\\s*[\"']?(\\d+)"), QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch start = tableStart.match(html);
    if (!start.hasMatch()) return rows;
    const int end = int(html.indexOf(QStringLiteral("</table"), start.capturedStart(), Qt::CaseInsensitive));
    const QString body = html.mid(start.capturedStart(), end < 0 ? -1 : end - start.capturedStart());
    QRegularExpressionMatchIterator rit = rowRe.globalMatch(body);
    qint64 total = 0;   // the cells made, those that a span of columns stands for among them: past the most there can be, the rest is not read
    while (rit.hasNext() && total < kMaxClipCells) {
        const QRegularExpressionMatch rm = rit.next();
        QStringList row;
        QRegularExpressionMatchIterator cit = cellRe.globalMatch(rm.captured(1));
        while (cit.hasNext() && total < kMaxClipCells) {
            const QRegularExpressionMatch cm = cit.next();
            QString text = cm.captured(3);
            text.replace(breakRe, QStringLiteral("\n"));
            text.remove(tag);
            text = decodeEntities(text);
            text.replace(QRegularExpression(QStringLiteral("[ \\t\\r\\n]+")), QStringLiteral(" "));
            text = text.trimmed();
            row << text;
            ++total;
            const QRegularExpressionMatch sp = span.match(cm.captured(2));
            if (sp.hasMatch()) {
                const qint64 pad = std::min<qint64>(std::min(sp.captured(1).toInt(), 256) - 1, kMaxClipCells - total);   // (counted before they are made)
                for (qint64 k = 0; k < pad; ++k) row << QString();
                total += std::max<qint64>(0, pad);
            }
        }
        if (!row.isEmpty()) rows.append(row);
    }
    return rows;
}

} // namespace jp::crunch
