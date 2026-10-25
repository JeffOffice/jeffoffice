#include "sheet/structref.h"

#include "sheet/refshift.h"

#include <algorithm>

namespace jp::office::sheet {

namespace {

// The index after the "]" that closes the "[" at `i` (a ' escapes the character after it), or -1.
int closeOf(const QString &s, int i)
{
    int depth = 0;
    for (int j = i; j < s.size(); ++j) {
        const QChar c = s[j];
        if (c == QLatin1Char('\'') && j + 1 < s.size()) {
            ++j;
            continue;
        }
        if (c == QLatin1Char('[')) ++depth;
        else if (c == QLatin1Char(']') && --depth == 0) return j + 1;
    }
    return -1;
}

bool isNameChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); }

struct Term {
    enum class Kind { Column, Special } kind = Kind::Column;
    QString name;   // a column's name, or the special item's words as written without the "#" (All, Data, Headers, Totals, This Row)
};

// `inside` read into the terms it holds, in order; a ':' between two column groups is a range (the second term has `rangeFromPrevious`).
struct Parsed {
    bool at = false;                // "@": the formula's row
    std::vector<Term> terms;
    std::vector<bool> rangeFromPrevious;
    bool ok = true;
};

Term termOf(const QString &raw)
{
    Term t;
    if (raw.startsWith(QLatin1Char('#'))) {
        t.kind = Term::Kind::Special;
        t.name = raw.mid(1).trimmed();
    } else {
        t.name = unescapeTableColumn(raw);
    }
    return t;
}

Parsed parseInside(const QString &inside)
{
    Parsed p;
    int i = 0;
    const int n = int(inside.size());
    auto skip = [&] {
        while (i < n && inside[i] == QLatin1Char(' ')) ++i;
    };
    skip();
    if (i < n && inside[i] == QLatin1Char('@')) {
        p.at = true;
        ++i;
        skip();
        if (i >= n) return p;
    }
    if (i < n && inside[i] != QLatin1Char('[')) {
        // a single name (or special item) written in the one pair of brackets
        const QString raw = inside.mid(i).trimmed();
        if (raw.isEmpty()) return p;
        p.terms.push_back(termOf(raw));
        p.rangeFromPrevious.push_back(false);
        return p;
    }
    bool range = false;
    while (i < n) {
        skip();
        if (i >= n) break;
        if (inside[i] == QLatin1Char('[')) {
            const int end = closeOf(inside, i);
            if (end < 0) {
                p.ok = false;
                return p;
            }
            p.terms.push_back(termOf(inside.mid(i + 1, end - i - 2)));
            p.rangeFromPrevious.push_back(range);
            range = false;
            i = end;
        } else if (inside[i] == QLatin1Char(',')) {
            ++i;
        } else if (inside[i] == QLatin1Char(':')) {
            range = true;
            ++i;
        } else {
            p.ok = false;
            return p;
        }
    }
    return p;
}

} // namespace

bool splitStructuredRef(const QString &text, QString *table, QString *inside)
{
    const int open = int(text.indexOf(QLatin1Char('[')));
    if (open < 0 || !text.endsWith(QLatin1Char(']'))) return false;
    const int end = closeOf(text, open);
    if (end != text.size()) return false;
    QString name = text.left(open).trimmed();
    for (const QChar c : name)
        if (!isNameChar(c)) return false;
    *table = name;
    *inside = text.mid(open + 1, text.size() - open - 2);
    return true;
}

StructResult structuredArea(const Table &t, const QString &inside, qint64 formulaRow, CellRange *out)
{
    const Parsed p = parseInside(inside);
    if (!p.ok) return StructResult::Ref;
    bool all = false, data = false, headers = false, totals = false, thisRow = p.at;
    int firstCol = -1, lastCol = -1;
    int previousColumn = -1;
    for (size_t i = 0; i < p.terms.size(); ++i) {
        const Term &term = p.terms[i];
        if (term.kind == Term::Kind::Special) {
            const QString w = term.name;
            if (w.compare(QLatin1String("All"), Qt::CaseInsensitive) == 0) all = true;
            else if (w.compare(QLatin1String("Data"), Qt::CaseInsensitive) == 0) data = true;
            else if (w.compare(QLatin1String("Headers"), Qt::CaseInsensitive) == 0) headers = true;
            else if (w.compare(QLatin1String("Totals"), Qt::CaseInsensitive) == 0) totals = true;
            else if (w.compare(QLatin1String("This Row"), Qt::CaseInsensitive) == 0) thisRow = true;
            else return StructResult::Ref;
            continue;
        }
        const int column = t.columnNamed(term.name);
        if (column < 0) return StructResult::Ref;
        if (p.rangeFromPrevious[i] && previousColumn >= 0) {
            firstCol = std::min({firstCol, previousColumn, column});
            lastCol = std::max({lastCol, previousColumn, column});
        } else {
            firstCol = firstCol < 0 ? column : std::min(firstCol, column);
            lastCol = lastCol < 0 ? column : std::max(lastCol, column);
        }
        previousColumn = column;
    }
    if (firstCol < 0) {
        firstCol = 0;
        lastCol = int(t.columns.size()) - 1;
    }
    out->valid = true;
    out->col1 = t.col1 + quint32(firstCol);
    out->col2 = t.col1 + quint32(lastCol);
    if (thisRow) {
        if (formulaRow < qint64(t.dataFirst()) || formulaRow > t.dataLast()) return StructResult::Value;
        out->row1 = out->row2 = quint32(formulaRow);
        return StructResult::Ok;
    }
    if (all) {
        out->row1 = t.row1;
        out->row2 = t.row2;
        return StructResult::Ok;
    }
    if (!headers && !totals) data = true;
    if (headers && t.headerRows == 0) return StructResult::Ref;
    if (totals && t.totalsRows == 0) return StructResult::Ref;
    if (headers && totals && !data) return StructResult::Ref;
    if (data && !t.hasData() && !headers && !totals) return StructResult::Ref;
    qint64 top, bottom;
    top = headers ? t.row1 : data ? qint64(t.dataFirst()) : t.row2;
    bottom = totals ? qint64(t.row2) : data ? t.dataLast() : qint64(t.row1);
    if (top > bottom) return StructResult::Ref;
    out->row1 = quint32(top);
    out->row2 = quint32(bottom);
    return StructResult::Ok;
}

namespace {
QString bracketed(const QString &column, bool inner)
{
    const QString name = escapeTableColumn(column);
    if (inner) return QLatin1Char('[') + name + QLatin1Char(']');
    return tableColumnNeedsDoubleBrackets(column) ? QStringLiteral("[[") + name + QStringLiteral("]]") : QLatin1Char('[') + name + QLatin1Char(']');
}
} // namespace

QString structuredColumnRef(const QString &table, const QString &column) { return table + bracketed(column, false); }

QString structuredRowRef(const QString &table, const QString &column, bool inside)
{
    if (inside) return tableColumnNeedsDoubleBrackets(column) ? QStringLiteral("[@[") + escapeTableColumn(column) + QStringLiteral("]]") : QStringLiteral("[@") + escapeTableColumn(column) + QLatin1Char(']');
    return table + QStringLiteral("[[#This Row],[") + escapeTableColumn(column) + QStringLiteral("]]");
}

QString structuredSpecialRef(const QString &table, const QString &item, const QString &column)
{
    if (column.isEmpty()) return table + QStringLiteral("[") + item + QLatin1Char(']');
    return table + QStringLiteral("[[") + item + QStringLiteral("],[") + escapeTableColumn(column) + QStringLiteral("]]");
}

namespace {

// Calls `f(start, end, tableName, inside)` for each structured reference of the formula, outside texts in quotes and sheet names in apostrophes; `f` gives the text that replaces the reference
// (from `start`, which includes the name before the bracket, to `end`), or a null string to leave it.
template <class F>
QString mapReferences(const QString &formula, F f)
{
    if (!formula.contains(QLatin1Char('['))) return formula;
    QString out;
    out.reserve(formula.size() + 16);
    const int n = int(formula.size());
    int i = 0;
    while (i < n) {
        const QChar c = formula[i];
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            int j = i + 1;
            while (j < n) {
                if (formula[j] == c) {
                    if (j + 1 < n && formula[j + 1] == c) {
                        j += 2;
                        continue;
                    }
                    break;
                }
                ++j;
            }
            out += formula.mid(i, std::min(n, j + 1) - i);
            i = j + 1;
            continue;
        }
        if (c == QLatin1Char('[')) {
            const int end = closeOf(formula, i);
            if (end < 0) {
                out += formula.mid(i);
                break;
            }
            int nameStart = int(out.size());
            while (nameStart > 0 && isNameChar(out[nameStart - 1])) --nameStart;
            const QString name = out.mid(nameStart);
            const QString inside = formula.mid(i + 1, end - i - 2);
            const QString replaced = f(name, inside);
            if (replaced.isNull()) {
                out += formula.mid(i, end - i);
            } else {
                out.truncate(nameStart);
                out += replaced;
            }
            i = end;
            continue;
        }
        out += c;
        ++i;
    }
    return out;
}

} // namespace

QString renameTableInFormula(const QString &formula, const QString &oldName, const QString &newName)
{
    return mapReferences(formula, [&](const QString &name, const QString &inside) -> QString {
        if (name.compare(oldName, Qt::CaseInsensitive) != 0) return QString();
        return newName + QLatin1Char('[') + inside + QLatin1Char(']');
    });
}

QString structuredRefsToCells(const QString &formula, const Table &t, const QString &tableSheetName, const QString &formulaSheetName, qint64 formulaRow, bool insideTable)
{
    return mapReferences(formula, [&](const QString &name, const QString &inside) -> QString {
        if (name.isEmpty() ? !insideTable : name.compare(t.name, Qt::CaseInsensitive) != 0) return QString();
        CellRange r;
        if (structuredArea(t, inside, formulaRow, &r) != StructResult::Ok) return QStringLiteral("#REF!");
        const QString prefix = tableSheetName.compare(formulaSheetName, Qt::CaseInsensitive) == 0 ? QString() : sheetNamePrefix(tableSheetName);
        // (a reference to the formula's own row is relative, so that the formula can be filled down; the rest are fixed)
        const bool own = parseInside(inside).at || inside.contains(QLatin1String("#This Row"), Qt::CaseInsensitive);
        const QString d = own ? QString() : QStringLiteral("$");
        const QString first = d + columnName(r.col1) + (own ? QString() : d) + QString::number(r.row1 + 1);
        if (r.row1 == r.row2 && r.col1 == r.col2) return prefix + first;
        return prefix + first + QLatin1Char(':') + d + columnName(r.col2) + (own ? QString() : d) + QString::number(r.row2 + 1);
    });
}

QString fileFormOfStructuredRefs(const QString &formula, const QString &tableAtCell)
{
    return mapReferences(formula, [&](const QString &name, const QString &inside) -> QString {
        const QString trimmed = inside.trimmed();
        if (!trimmed.startsWith(QLatin1Char('@'))) return QString();
        const QString table = name.isEmpty() ? tableAtCell : name;
        if (table.isEmpty()) return QString();
        const QString rest = trimmed.mid(1).trimmed();
        if (rest.isEmpty()) return table + QStringLiteral("[#This Row]");
        if (rest.startsWith(QLatin1Char('['))) return table + QStringLiteral("[[#This Row],") + rest + QLatin1Char(']');
        return table + QStringLiteral("[[#This Row],[") + rest + QStringLiteral("]]");
    });
}

QString typedFormOfStructuredRefs(const QString &formula, const QString &tableAtCell)
{
    if (tableAtCell.isEmpty()) return formula;
    return mapReferences(formula, [&](const QString &name, const QString &inside) -> QString {
        if (name.compare(tableAtCell, Qt::CaseInsensitive) != 0) return QString();
        static const QString head = QStringLiteral("[#This Row],");
        const QString trimmed = inside.trimmed();
        if (!trimmed.startsWith(head, Qt::CaseInsensitive)) return QString();
        const QString rest = trimmed.mid(head.size()).trimmed();
        if (!rest.startsWith(QLatin1Char('[')) || closeOf(rest, 0) != rest.size()) return QString();   // (one group: a single column)
        const QString column = rest.mid(1, rest.size() - 2);
        return tableColumnNeedsDoubleBrackets(unescapeTableColumn(column)) ? QStringLiteral("[@[") + column + QStringLiteral("]]") : QStringLiteral("[@") + column + QLatin1Char(']');
    });
}

} // namespace jp::office::sheet
