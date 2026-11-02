#include "core/formularefs.h"

#include "sheet/cellref.h"

#include <QRegularExpression>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

// Where text in quotes and square brackets are, so that nothing in them is read as a reference.
std::vector<bool> shielded(const QString &t)
{
    std::vector<bool> out(size_t(t.size()), false);
    bool quote = false;
    int depth = 0;
    for (int i = 0; i < t.size(); ++i) {
        const QChar c = t[i];
        if (quote) {
            out[size_t(i)] = true;
            if (c == QLatin1Char('"')) quote = false;   // ("" inside a text closes and opens again: the second quote is shielded as it opens)
        } else if (c == QLatin1Char('"')) {
            quote = true;
            out[size_t(i)] = true;
        } else if (c == QLatin1Char('[')) {
            ++depth;
            out[size_t(i)] = true;
        } else if (c == QLatin1Char(']') && depth > 0) {
            --depth;
            out[size_t(i)] = true;
        } else if (depth > 0) {
            out[size_t(i)] = true;
        }
    }
    return out;
}

bool columnOf(const QString &letters, quint32 *col)
{
    quint32 row = 0;
    return parseCellRef(letters + QLatin1Char('1'), &row, col);
}

} // namespace

std::vector<FormulaRef> findReferences(const QString &text)
{
    // (what comes before a reference, and what after, so that part of a longer word, a 3-D or another workbook's reference, or the cells of a sheet's name are not taken for one; a sheet's
    // name is written plain with the same characters sheetNamePrefix leaves without apostrophes)
    static const QRegularExpression re(QStringLiteral(
        "(?<![\\p{L}\\p{N}_.$\\\\':!\\]])((?:'(?:[^']|'')+'|[\\p{L}_][\\p{L}\\p{N}_.]*)!)?"
        "(\\$?[A-Za-z]{1,3}\\$?[0-9]+(?::\\$?[A-Za-z]{1,3}\\$?[0-9]+)?|\\$?[A-Za-z]{1,3}:\\$?[A-Za-z]{1,3}|\\$?[0-9]+:\\$?[0-9]+)(?![\\p{L}\\p{N}_(.!\\[\\\\])"),
        QRegularExpression::UseUnicodePropertiesOption);
    std::vector<FormulaRef> out;
    const std::vector<bool> skip = shielded(text);
    int pos = 0;
    while (pos < text.size()) {
        const QRegularExpressionMatch m = re.match(text, pos);
        if (!m.hasMatch()) break;
        const int start = int(m.capturedStart(0)), end = start + int(m.capturedLength(0));
        // (a match that touches a text in quotes or a table's brackets is not one: the search goes on just after its start, for it may have run into the text from outside)
        bool touches = false;
        for (int i = start; i < end && !touches; ++i) touches = skip[size_t(i)];
        QString sheet = m.captured(1);
        if (!sheet.isEmpty()) {
            sheet.chop(1);
            if (sheet.startsWith(QLatin1Char('\''))) sheet = sheet.mid(1, sheet.size() - 2).replace(QLatin1String("''"), QLatin1String("'"));
        }
        if (touches || sheet.contains(QLatin1Char(':')) || sheet.contains(QLatin1Char('['))) {   // (a name with a colon is a 3-D reference, with brackets another workbook's)
            pos = start + 1;
            continue;
        }
        pos = end;
        FormulaRef r;
        r.start = start;
        r.length = end - start;
        r.bodyStart = int(m.capturedStart(2));
        r.sheet = sheet;
        const QStringList parts = m.captured(2).split(QLatin1Char(':'));
        bool ok = false;
        if (parts.size() == 1) {
            r.kind = FormulaRef::Kind::Cell;
            ok = parseCellRef(parts[0], &r.row1, &r.col1);
            r.row2 = r.row1;
            r.col2 = r.col1;
        } else {
            const QString a = QString(parts[0]).remove(QLatin1Char('$')), b = QString(parts[1]).remove(QLatin1Char('$'));
            if (a[0].isDigit() && b[0].isDigit()) {
                r.kind = FormulaRef::Kind::Rows;
                const quint32 x = a.toUInt(), y = b.toUInt();
                ok = x >= 1 && y >= 1 && x <= kMaxRows && y <= kMaxRows;
                r.row1 = std::min(x, y) - 1;
                r.row2 = std::max(x, y) - 1;
                r.col1 = 0;
                r.col2 = kMaxColumns - 1;
            } else if (!a[a.size() - 1].isDigit() && !b[b.size() - 1].isDigit()) {
                r.kind = FormulaRef::Kind::Columns;
                quint32 x = 0, y = 0;
                ok = columnOf(a, &x) && columnOf(b, &y);
                r.col1 = std::min(x, y);
                r.col2 = std::max(x, y);
                r.row1 = 0;
                r.row2 = kMaxRows - 1;
            } else {
                r.kind = FormulaRef::Kind::Range;
                quint32 r1 = 0, c1 = 0, r2 = 0, c2 = 0;
                ok = parseCellRef(parts[0], &r1, &c1) && parseCellRef(parts[1], &r2, &c2);
                r.row1 = std::min(r1, r2);
                r.row2 = std::max(r1, r2);
                r.col1 = std::min(c1, c2);
                r.col2 = std::max(c1, c2);
            }
        }
        if (ok) out.push_back(r);
    }
    return out;
}

namespace {

// One end of a reference as written ($B$2, C, $3): its letters and digits, and whether the column and the row are fixed (the "$" before digits alone is the row's).
struct Part {
    QString letters, digits;
    bool col = false, row = false;
    bool ok = false;
};

Part parsePart(const QString &p)
{
    static const QRegularExpression re(QStringLiteral("^(\\$?)([A-Za-z]*)(\\$?)([0-9]*)$"));
    Part out;
    const QRegularExpressionMatch m = re.match(p);
    if (!m.hasMatch()) return out;
    out.ok = true;
    out.letters = m.captured(2);
    out.digits = m.captured(4);
    if (out.letters.isEmpty()) {
        out.row = !m.captured(1).isEmpty() || !m.captured(3).isEmpty();
    } else {
        out.col = !m.captured(1).isEmpty();
        out.row = !m.captured(3).isEmpty();
    }
    return out;
}

QString writePart(const Part &p, bool col, bool row)
{
    QString s;
    if (!p.letters.isEmpty()) s += (col ? QStringLiteral("$") : QString()) + p.letters;
    if (!p.digits.isEmpty()) s += (row ? QStringLiteral("$") : QString()) + p.digits;
    return s;
}

} // namespace

QString nextFixedForm(const QString &body)
{
    const QStringList parts = body.split(QLatin1Char(':'));
    const Part first = parsePart(parts[0]);
    if (!first.ok) return body;
    const bool hasCol = !first.letters.isEmpty(), hasRow = !first.digits.isEmpty();
    bool col = first.col, row = first.row;
    if (hasCol && !hasRow) col = !col;
    else if (hasRow && !hasCol) row = !row;
    else if (!col && !row) col = row = true;       // A1 -> $A$1
    else if (col && row) col = false;              // $A$1 -> A$1
    else if (!col && row) std::swap(col, row);     // A$1 -> $A1
    else col = row = false;                        // $A1 -> A1
    QStringList out;
    for (const QString &p : parts) {
        const Part part = parsePart(p);
        if (!part.ok) return body;
        out << writePart(part, col, row);
    }
    return out.join(QLatin1Char(':'));
}

QString refitBody(const QString &oldBody, const QString &newBody)
{
    if (!oldBody.contains(QLatin1Char('$'))) return newBody;
    const QStringList olds = oldBody.split(QLatin1Char(':')), news = newBody.split(QLatin1Char(':'));
    QStringList out;
    for (int i = 0; i < news.size(); ++i) {
        const Part o = parsePart(olds[std::min<int>(i, int(olds.size()) - 1)]), n = parsePart(news[i]);
        if (!o.ok || !n.ok) return newBody;
        out << writePart(n, o.col, o.row);
    }
    return out.join(QLatin1Char(':'));
}

QString referenceText(FormulaRef::Kind kind, quint32 row1, quint32 col1, quint32 row2, quint32 col2)
{
    const quint32 r1 = std::min(row1, row2), r2 = std::max(row1, row2), c1 = std::min(col1, col2), c2 = std::max(col1, col2);
    if (kind == FormulaRef::Kind::Columns) return columnName(c1) + QLatin1Char(':') + columnName(c2);
    if (kind == FormulaRef::Kind::Rows) return QString::number(r1 + 1) + QLatin1Char(':') + QString::number(r2 + 1);
    if (r1 == r2 && c1 == c2) return cellName(r1, c1);
    return cellName(r1, c1) + QLatin1Char(':') + cellName(r2, c2);
}

QVector<quint32> referenceColors()
{
    // blue, red, purple, green, orange, teal, pink, brown: each reads on a white sheet and on a dark one
    return {0xFF3B82F6, 0xFFEF4444, 0xFFA855F7, 0xFF22A06B, 0xFFF59E0B, 0xFF14B8C4, 0xFFEC4899, 0xFFB7791F};
}

} // namespace jp::crunch
