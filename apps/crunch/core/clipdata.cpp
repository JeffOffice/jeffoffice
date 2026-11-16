#include "core/clipdata.h"

#include "sheet/notes.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDate>
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
    // (after the merges, so that a reader that stops there reads the rest as it always did)
    s << quint32(notes.size());
    for (const ClipNote &n : notes) s << n.dRow << n.dCol << n.text << n.author << n.shown;
    s << quint32(columnWidths.size());
    for (const double w : columnWidths) s << w;
    // (after the widths, for the same reason)
    s << quint32(threads.size());
    for (const ClipThread &t : threads) {
        s << t.dRow << t.dCol << t.shown << quint32(t.comments.size());
        for (const ClipThreadComment &c : t.comments) s << c.author << c.userId << c.providerId << c.personId << c.time << c.text << c.done << c.parent;
    }
    // (after the threads, for the same reason)
    s << quint32(rowOffsets.size());
    for (const quint32 o : rowOffsets) s << o;
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
    if (!s.atEnd()) {
        quint32 notes = 0, widths = 0;
        s >> notes;
        constexpr qint64 kLeastNoteBytes = 17;
        if (s.status() != QDataStream::Ok || qint64(notes) > (bytes.size() - s.device()->pos()) / kLeastNoteBytes || qint64(notes) > qint64(d.rows) * qint64(d.cols)) return false;
        for (quint32 i = 0; i < notes; ++i) {
            ClipNote n;
            s >> n.dRow >> n.dCol >> n.text >> n.author >> n.shown;
            if (s.status() != QDataStream::Ok || n.dRow >= d.rows || n.dCol >= d.cols) return false;
            d.notes.push_back(std::move(n));
        }
        s >> widths;
        if (s.status() != QDataStream::Ok || widths > d.cols || qint64(widths) * 8 > bytes.size() - s.device()->pos()) return false;
        for (quint32 i = 0; i < widths; ++i) {
            double w = 0;
            s >> w;
            d.columnWidths.push_back(w);
        }
        if (s.status() != QDataStream::Ok) return false;
    }
    if (!s.atEnd()) {
        quint32 threads = 0;
        s >> threads;
        constexpr qint64 kLeastThreadBytes = 13;
        if (s.status() != QDataStream::Ok || qint64(threads) > (bytes.size() - s.device()->pos()) / kLeastThreadBytes || qint64(threads) > qint64(d.rows) * qint64(d.cols)) return false;
        qint64 total = 0;
        for (quint32 i = 0; i < threads; ++i) {
            ClipThread t;
            quint32 count = 0;
            s >> t.dRow >> t.dCol >> t.shown >> count;
            if (s.status() != QDataStream::Ok || t.dRow >= d.rows || t.dCol >= d.cols || count > quint32(jp::office::sheet::kMaxThreadComments) || (total += count) > jp::office::sheet::kMaxNotes) return false;
            for (quint32 k = 0; k < count; ++k) {
                ClipThreadComment c;
                s >> c.author >> c.userId >> c.providerId >> c.personId >> c.time >> c.text >> c.done >> c.parent;
                if (s.status() != QDataStream::Ok || c.parent < -1 || c.parent >= qint32(count)) return false;
                t.comments.push_back(std::move(c));
            }
            d.threads.push_back(std::move(t));
        }
    }
    if (!s.atEnd()) {
        quint32 offsets = 0;
        s >> offsets;
        if (s.status() != QDataStream::Ok || (offsets != 0 && offsets != d.rows) || qint64(offsets) * 4 > bytes.size() - s.device()->pos()) return false;
        for (quint32 i = 0; i < offsets; ++i) {
            quint32 o = 0;
            s >> o;
            if (s.status() != QDataStream::Ok || qint64(d.source.row1) + o > d.source.row2) return false;
            d.rowOffsets.push_back(o);
        }
    }
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

QVector<QStringList> splitDelimitedText(const QString &text, const jp::office::sheet::CsvImport &choices)
{
    using jp::office::sheet::CsvColumn;
    QVector<QStringList> rows;
    QStringList row;
    QString field;
    bool quoted = false, inQuote = false, wasSeparator = false, any = false;
    qint64 made = 0, line = 0;
    const QString seps = choices.separators.isEmpty() ? QStringLiteral("\t") : choices.separators;
    auto endField = [&] {
        row << field;
        field.clear();
        quoted = false;
    };
    auto endRow = [&] {
        endField();
        if (line++ >= choices.firstRow) {
            QStringList out;
            for (qsizetype c = 0; c < row.size(); ++c) {
                const CsvColumn k = choices.columnAt(c);
                if (k.kind == CsvColumn::Kind::Skip) continue;
                QString f = row[c];
                if (k.kind == CsvColumn::Kind::Text && !f.isEmpty()) f = QLatin1Char('\'') + f;
                else if (k.kind == CsvColumn::Kind::Date && !f.isEmpty()) {
                    const QString order = k.dateOrder.isEmpty() ? QStringLiteral("MDY") : k.dateOrder;
                    const QStringList parts = f.trimmed().split(QRegularExpression(QStringLiteral("[-/.\\s]+")));
                    if (parts.size() == 3) {
                        bool ok[3] = {false, false, false};
                        const int a = parts[0].toInt(&ok[0]), b = parts[1].toInt(&ok[1]), c3 = parts[2].toInt(&ok[2]);
                        if (ok[0] && ok[1] && ok[2]) {
                            int y = c3, m = a, d = b;
                            if (order == QLatin1String("DMY")) { d = a; m = b; y = c3; }
                            else if (order == QLatin1String("YMD")) { y = a; m = b; d = c3; }
                            if (y < 100) y += y < 30 ? 2000 : 1900;
                            const QDate date(y, m, d);
                            if (date.isValid()) f = date.toString(Qt::ISODate);
                        }
                    }
                }
                out << f;
            }
            made += out.size();
            rows << out;
        }
        row.clear();
        any = false;
    };
    if (choices.fixedWidth) {
        // columns of fixed width: each line is cut at the places chosen, and the pieces lose the spaces at their ends
        QVector<int> breaks = choices.breaks;
        std::sort(breaks.begin(), breaks.end());
        qsizetype pos = 0;
        while (pos < text.size() && made <= kMaxClipCells) {
            qsizetype end = pos;
            while (end < text.size() && text[end] != QLatin1Char('\r') && text[end] != QLatin1Char('\n')) ++end;
            const QString lineText = text.mid(pos, end - pos);
            pos = end;
            if (pos < text.size() && text[pos] == QLatin1Char('\r') && pos + 1 < text.size() && text[pos + 1] == QLatin1Char('\n')) ++pos;
            ++pos;
            qsizetype from = 0;
            for (qsizetype k = 0; k <= breaks.size(); ++k) {
                const qsizetype to = k < breaks.size() ? qsizetype(breaks[k]) : lineText.size();
                const QString piece = from < lineText.size() ? lineText.mid(from, std::max<qsizetype>(0, to - from)).trimmed() : QString();
                if (k < breaks.size()) row << piece;
                else field = piece;
                from = std::max(from, to);
            }
            any = true;
            endRow();
        }
        return rows;
    }
    for (qsizetype i = 0; i < text.size() && made <= kMaxClipCells; ++i) {
        const QChar ch = text[i];
        if (inQuote) {
            if (ch == choices.quote) {
                if (i + 1 < text.size() && text[i + 1] == choices.quote) {
                    field += ch;
                    ++i;
                } else inQuote = false;
            } else field += ch;
            continue;
        }
        if (!choices.quote.isNull() && ch == choices.quote && field.isEmpty() && !quoted) {
            inQuote = quoted = any = true;
            wasSeparator = false;
            continue;
        }
        if (ch == QLatin1Char('\r') || ch == QLatin1Char('\n')) {
            if (ch == QLatin1Char('\r') && i + 1 < text.size() && text[i + 1] == QLatin1Char('\n')) ++i;
            endRow();
            wasSeparator = false;
            continue;
        }
        if (seps.contains(ch)) {
            if (!(choices.mergeSeparators && wasSeparator)) endField();
            wasSeparator = true;
            any = true;
            continue;
        }
        wasSeparator = false;
        any = true;
        field += ch;
    }
    if (any || !field.isEmpty() || !row.isEmpty()) endRow();
    return rows;
}

bool guessDelimitedText(const QString &text, jp::office::sheet::CsvImport *choices, bool strict)
{
    if (text.contains(QLatin1Char('\t'))) return false;
    // the first lines whole: a cut line would count wrong, and a line end inside quotes is no end of a line
    QString head = text.left(kGuessBytes);
    if (head.size() < text.size()) {
        bool inQuote = false;
        qsizetype cut = -1;
        for (qsizetype i = 0; i < head.size(); ++i) {
            const QChar ch = head[i];
            if (ch == QLatin1Char('"')) inQuote = !inQuote;
            else if (!inQuote && (ch == QLatin1Char('\n') || ch == QLatin1Char('\r'))) cut = i;
        }
        if (cut <= 0) return false;
        head.truncate(cut);
    }
    const int minFields = strict ? 3 : 2;
    // by a mark: the same count of fields on every row (a blank line is no row; a line end inside a quoted field is part of the field)
    for (const QChar mark : {QLatin1Char(';'), QLatin1Char('|'), QLatin1Char(',')}) {
        if (!head.contains(mark)) continue;
        jp::office::sheet::CsvImport c;
        c.separators = QString(mark);
        QVector<QStringList> table;
        for (const QStringList &row : splitDelimitedText(head, c)) {
            if (row.size() == 1 && row.first().trimmed().isEmpty()) continue;
            table << row;
            if (table.size() >= kGuessLines) break;
        }
        bool same = table.size() >= 2 && table.first().size() >= minFields;
        for (const QStringList &row : std::as_const(table))
            if (row.size() != table.first().size()) same = false;
        if (!same) continue;
        if (strict) {
            // no column that is empty on every row ("int x = 1;" lines, a list with a comma at the end of each line)
            for (qsizetype col = 0; col < table.first().size(); ++col) {
                bool any = false;
                for (const QStringList &row : std::as_const(table)) any = any || !row[col].trimmed().isEmpty();
                if (!any) same = false;
            }
            // numbers with their thousands marks (1,234 and 5,678) are not fields
            if (mark == QLatin1Char(',')) {
                static const QRegularExpression grouped(QStringLiteral("^\\s*-?\\d{1,3}(,\\d{3})+\\s*$"));
                bool all = true;
                for (const QStringList &row : std::as_const(table)) all = all && grouped.match(row.join(QLatin1Char(','))).hasMatch();
                if (all) same = false;
            }
            if (!same) continue;
        }
        choices->separators = c.separators;
        choices->mergeSeparators = false;
        choices->fixedWidth = false;
        choices->breaks.clear();
        if (mark == QLatin1Char(';')) {
            static const QRegularExpression commaNumber(QStringLiteral("^-?\\d+,\\d+$")), dotNumber(QStringLiteral("^-?\\d+\\.\\d+$"));
            bool comma = false, dot = false;
            for (const QStringList &row : std::as_const(table))
                for (const QString &f : row) {
                    comma = comma || commaNumber.match(f.trimmed()).hasMatch();
                    dot = dot || dotNumber.match(f.trimmed()).hasMatch();
                }
            if (comma && !dot) choices->decimalMark = QLatin1Char(',');
        }
        return true;
    }
    // columns of fixed width: gaps of spaces that every line has
    QStringList kept;
    for (const QString &l : head.split(QRegularExpression(QStringLiteral("\r\n|\r|\n"))))
        if (!l.trimmed().isEmpty() && kept.size() < kGuessLines) kept << l;
    if (kept.size() < 3) return false;
    qsizetype longest = 0;
    for (QString &l : kept) {
        while (l.endsWith(QLatin1Char(' '))) l.chop(1);
        longest = std::max(longest, l.size());
    }
    if (longest > 4096) return false;
    QVector<bool> blank(longest, true);
    for (const QString &l : std::as_const(kept))
        for (qsizetype p = 0; p < l.size(); ++p)
            if (l[p] != QLatin1Char(' ')) blank[p] = false;
    QVector<int> starts;
    int gaps = 0;
    bool inData = false;
    for (qsizetype p = 0; p < longest; ++p) {
        if (!blank[p] && !inData) {
            if (p > 0 && !starts.isEmpty()) ++gaps;
            starts << int(p);
        }
        inData = !blank[p];
    }
    if (gaps < 2 || starts.size() > 256) return false;
    // each column holds something on at least half the lines
    for (qsizetype k = 0; k < starts.size(); ++k) {
        const int from = starts[k], to = k + 1 < starts.size() ? starts[k + 1] : int(longest);
        int filled = 0;
        for (const QString &l : std::as_const(kept))
            if (!l.mid(from, to - from).trimmed().isEmpty()) ++filled;
        if (filled * 2 < kept.size()) return false;
    }
    choices->fixedWidth = true;
    choices->separators.clear();
    choices->breaks.clear();
    for (qsizetype k = 1; k < starts.size(); ++k) choices->breaks << starts[k];
    return true;
}

bool textImportFromOptions(const QString &separator, const QString &encoding, const QString &decimal, jp::office::sheet::CsvImport *out, QString *error)
{
    using jp::office::sheet::CsvEncoding;
    auto fail = [&](const QString &what) {
        if (error) *error = what;
        return false;
    };
    jp::office::sheet::CsvImport c;
    // (a null text is an option that was not given; an empty one was given, and is refused)
    if (!separator.isNull() && separator.isEmpty()) return fail(QCoreApplication::translate("jp::crunch::TextOptions", "The separator is empty. Use comma, semicolon, tab, space, bar, or one character."));
    if (!separator.isNull()) {
        const QString k = separator.toLower();
        if (k == QLatin1String("comma")) c.separators = QStringLiteral(",");
        else if (k == QLatin1String("semicolon")) c.separators = QStringLiteral(";");
        else if (k == QLatin1String("tab") || separator == QLatin1String("\\t")) c.separators = QStringLiteral("\t");
        else if (k == QLatin1String("space")) c.separators = QStringLiteral(" ");
        else if (k == QLatin1String("bar") || k == QLatin1String("pipe")) c.separators = QStringLiteral("|");
        else if (separator.size() == 1 && separator != QLatin1String("\"") && (separator.at(0).category() != QChar::Other_Control || separator.at(0).unicode() == 9) && separator.at(0).unicode() != 0x2028 && separator.at(0).unicode() != 0x2029) c.separators = separator;
        else return fail(QCoreApplication::translate("jp::crunch::TextOptions", "The separator \"%1\" is not comma, semicolon, tab, space, bar, or one character.").arg(separator));
    }
    if (!encoding.isNull()) {
        const QString k = encoding.toLower().remove(QLatin1Char('-')).remove(QLatin1Char('_')).remove(QLatin1Char(' '));
        c.encodingSet = true;
        auto legacy = [&](int page) {
            c.encoding = CsvEncoding::Legacy;
            c.codePage = page;
        };
        static const QRegularExpression windows(QStringLiteral("^(?:windows|cp|win)?(125[0-4]|932|936|949|950)$")), iso(QStringLiteral("^iso8859(\\d{1,2})$"));
        if (k == QLatin1String("utf8")) c.encoding = CsvEncoding::Utf8;
        else if (k == QLatin1String("utf8bom")) c.encoding = CsvEncoding::Utf8Bom;
        else if (k == QLatin1String("utf16") || k == QLatin1String("utf16le")) c.encoding = CsvEncoding::Utf16Le;
        else if (k == QLatin1String("utf16be")) c.encoding = CsvEncoding::Utf16Be;
        else if (k == QLatin1String("latin1")) legacy(28591);
        else if (k == QLatin1String("macroman")) legacy(10000);
        else if (const auto m = windows.match(k); m.hasMatch()) legacy(m.captured(1).toInt());
        else if (const auto m2 = iso.match(k); m2.hasMatch() && QList<int>{1, 2, 5, 7, 9, 15}.contains(m2.captured(1).toInt())) legacy(28590 + m2.captured(1).toInt());
        else return fail(QCoreApplication::translate("jp::crunch::TextOptions", "The encoding \"%1\" is not one JeffCrunch reads text in. Use utf-8, utf-8-bom, utf-16le, utf-16be, latin1, mac-roman, windows-1252 (or another page), or iso-8859-N.").arg(encoding));
    }
    if (!decimal.isNull()) {
        const QString k = decimal.toLower();
        if (k == QLatin1String(".") || k == QLatin1String("point")) c.decimalMark = QLatin1Char('.');
        else if (k == QLatin1String(",") || k == QLatin1String("comma")) c.decimalMark = QLatin1Char(',');
        else return fail(QCoreApplication::translate("jp::crunch::TextOptions", "The decimal mark \"%1\" is not a point or a comma.").arg(decimal));
    }
    if (!c.separators.isEmpty() && !c.decimalMark.isNull() && c.separators.contains(c.decimalMark))
        return fail(QCoreApplication::translate("jp::crunch::TextOptions", "The separator and the decimal mark are the same character. Choose different ones."));
    *out = c;
    return true;
}

} // namespace jp::crunch
