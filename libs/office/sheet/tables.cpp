#include "sheet/tables.h"

#include "sheet/sheetreader.h"

#include <QRegularExpression>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp::office::sheet {

namespace {

constexpr int kMaxTables = 20000;
constexpr int kMaxTableColumns = 16384;
constexpr int kMaxColumnNameChars = 255;

QString nameOf(const QXmlStreamAttributes &a, const char *name) { return a.value(QLatin1String(name)).toString(); }

bool isTableElement(const QXmlStreamReader &r, const char *name) { return r.name() == QLatin1String(name); }

// ---- names in formulas

// The name inside brackets, with the escapes taken out (' before [ ] # and ').
QString unescapeColumn(const QString &raw)
{
    QString out;
    out.reserve(raw.size());
    for (int i = 0; i < raw.size(); ++i) {
        if (raw[i] == QLatin1Char('\'') && i + 1 < raw.size()) ++i;
        out += raw[i];
    }
    return out;
}

QString escapeColumn(const QString &name)
{
    QString out;
    out.reserve(name.size() + 4);
    for (const QChar c : name) {
        if (c == QLatin1Char('[') || c == QLatin1Char(']') || c == QLatin1Char('#') || c == QLatin1Char('\''))
            out += QLatin1Char('\'');
        out += c;
    }
    return out;
}

// A name that is not one a bracket can hold alone (it has a character that needs the whole of it in brackets).
bool needsDoubleBrackets(const QString &name)
{
    static const QString special = QStringLiteral(",:.[]#'\"{}$^&*+=-<>/\t\n\r");
    for (const QChar c : name)
        if (special.contains(c)) return true;
    return false;
}

// The index after the "]" that closes the "[" at `i` (a ' escapes the character after it), or -1.
int closeOfBracket(const QString &s, int i)
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

// The structured reference that begins at the "[" at `at` (its text, and the name before it, were found by the caller): its columns renamed.
struct Rename {
    const QString &oldName, &newName;
    bool changed = false;
};

QString renameInBracket(const QString &group, const Rename &rename, bool *changed)
{
    // `group` is the whole "[...]" (the outer brackets included). The inside is a column ("Name", "@Name"), a special item ("#Headers", "#This Row"), or groups of those,
    // each in its brackets and joined by commas and colons ("[#Headers],[Name]", "@[Name]", "[Name]:[Next]").
    const QString inner = group.mid(1, group.size() - 2);
    auto same = [&](const QString &raw) { return unescapeColumn(raw).compare(rename.oldName, Qt::CaseInsensitive) == 0; };
    if (!inner.contains(QLatin1Char('['))) {
        // a single bracket: a column name, with "@" for the row's own cell, or a special item
        if (inner.startsWith(QLatin1Char('#'))) return group;
        const bool at = inner.startsWith(QLatin1Char('@'));
        const QString name = at ? inner.mid(1) : inner;
        if (!same(name)) return group;
        *changed = true;
        const QString escaped = escapeColumn(rename.newName);
        const QString mark = at ? QStringLiteral("@") : QString();
        if (needsDoubleBrackets(rename.newName)) return QLatin1Char('[') + mark + QStringLiteral("[") + escaped + QStringLiteral("]]");
        return QLatin1Char('[') + mark + escaped + QLatin1Char(']');
    }
    // groups inside: each "[...]" that is a column
    QString out = QStringLiteral("[");
    int i = 0;
    while (i < inner.size()) {
        if (inner[i] == QLatin1Char('[')) {
            const int end = closeOfBracket(inner, i);
            if (end < 0) {
                out += inner.mid(i);
                break;
            }
            const QString piece = inner.mid(i, end - i);
            const QString name = piece.mid(1, piece.size() - 2);
            if (!name.startsWith(QLatin1Char('#')) && same(name)) {
                out += QLatin1Char('[') + escapeColumn(rename.newName) + QLatin1Char(']');
                *changed = true;
            } else {
                out += piece;
            }
            i = end;
        } else {
            out += inner[i++];
        }
    }
    out += QLatin1Char(']');
    return out;
}

} // namespace

QString renameColumnInFormula(const QString &formula, const QString &table, const QString &oldName, const QString &newName, bool insideTable)
{
    if (!formula.contains(QLatin1Char('['))) return formula;
    const Rename rename{oldName, newName};
    QString out;
    out.reserve(formula.size() + 8);
    const int n = int(formula.size());
    for (int i = 0; i < n;) {
        const QChar c = formula[i];
        if (c == QLatin1Char('"')) {
            // a text, with doubled quotes inside
            int j = i + 1;
            while (j < n) {
                if (formula[j] == QLatin1Char('"')) {
                    if (j + 1 < n && formula[j + 1] == QLatin1Char('"')) {
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
        if (c == QLatin1Char('\'')) {
            // a sheet's name in quotes
            int j = i + 1;
            while (j < n) {
                if (formula[j] == QLatin1Char('\'')) {
                    if (j + 1 < n && formula[j + 1] == QLatin1Char('\'')) {
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
            // a structured reference: the name just before it says the table (none: the table the formula is in)
            int nameStart = int(out.size());
            while (nameStart > 0 && isNameChar(out[nameStart - 1])) --nameStart;
            const QString before = out.mid(nameStart);
            const int end = closeOfBracket(formula, i);
            if (end < 0) {
                out += formula.mid(i);
                break;
            }
            const bool ours = before.isEmpty() ? insideTable : before.compare(table, Qt::CaseInsensitive) == 0;
            // (a bracket group right after another, as in "Table1[[a]:[b]]", is read whole by closeOfBracket; "Table1[a]:Table1[b]" is two references)
            if (ours) {
                bool changed = false;
                out += renameInBracket(formula.mid(i, end - i), rename, &changed);
            } else {
                out += formula.mid(i, end - i);
            }
            i = end;
            continue;
        }
        out += c;
        ++i;
    }
    return out;
}

QString uniqueColumnName(const QString &typedIn, const QStringList &columns, int column, bool *tooLong)
{
    if (tooLong) *tooLong = false;
    QString name = typedIn.trimmed();
    if (name.startsWith(QLatin1Char('\'')) && name.size() > 1) name = name.mid(1);
    if (name.isEmpty()) return {};
    if (name.size() > kMaxColumnNameChars) {
        if (tooLong) *tooLong = true;
        return {};
    }
    auto taken = [&](const QString &candidate) {
        for (int i = 0; i < columns.size(); ++i)
            if (i != column && columns[i].compare(candidate, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    if (!taken(name)) return name;
    for (int n = 2;; ++n) {
        const QString candidate = name.left(kMaxColumnNameChars - 6) + QString::number(n);
        if (!taken(candidate)) return candidate;
    }
}

QVector<Table> readTables(const Package &package, const WorkbookInfo &info)
{
    QVector<Table> tables;
    for (int sheet = 0; sheet < info.sheets.size(); ++sheet) {
        const SheetEntry &entry = info.sheets[sheet];
        if (entry.kind != SheetKind::Worksheet || entry.part.isEmpty()) continue;
        for (const Relationship &rel : package.relationshipsOfType(entry.part, QStringLiteral("/table"))) {
            if (rel.external || tables.size() >= kMaxTables) continue;
            const QString partName = resolvePartName(entry.part, rel.target);
            const QByteArray bytes = package.part(partName);
            if (bytes.isEmpty()) continue;
            Table t;
            t.part = partName;
            t.sheet = sheet;
            QXmlStreamReader r(bytes);
            QString displayName, name;
            QString ref;
            int headerRows = 1;
            while (!r.atEnd()) {
                if (r.readNext() != QXmlStreamReader::StartElement) continue;
                if (isTableElement(r, "table")) {
                    const QXmlStreamAttributes a = r.attributes();
                    name = nameOf(a, "name");
                    displayName = nameOf(a, "displayName");
                    ref = nameOf(a, "ref");
                    const QString hr = nameOf(a, "headerRowCount");
                    if (!hr.isEmpty()) headerRows = hr.toInt();
                } else if (isTableElement(r, "tableColumn")) {
                    if (t.columns.size() < kMaxTableColumns) t.columns.append(nameOf(r.attributes(), "name"));
                }
            }
            if (r.hasError() || t.columns.isEmpty()) continue;
            CellRange range;
            if (!parseRangeRef(ref, &range)) continue;
            t.name = displayName.isEmpty() ? name : displayName;
            t.row1 = range.row1;
            t.col1 = range.col1;
            t.row2 = range.row2;
            t.col2 = range.col2;
            t.headerRows = headerRows > 0 ? 1 : 0;
            t.fileColumns = t.columns;
            tables.append(t);
        }
    }
    return tables;
}

bool renameTableXml(const QByteArray &xml, const QStringList &fileNames, const QStringList &names, const QString &table, QByteArray *out)
{
    const QString text = QString::fromUtf8(xml);
    if (!text.contains(QLatin1String("<table"))) return false;
    auto escapeText = [](const QString &s) {
        QString o = s;
        o.replace(QLatin1Char('&'), QStringLiteral("&amp;")).replace(QLatin1Char('<'), QStringLiteral("&lt;")).replace(QLatin1Char('>'), QStringLiteral("&gt;"));
        return o;
    };
    auto escapeAttribute = [&](const QString &s) { return escapeText(s).replace(QLatin1Char('"'), QStringLiteral("&quot;")); };
    auto unescapeXml = [](const QString &s) {
        QString o = s;
        o.replace(QStringLiteral("&lt;"), QStringLiteral("<")).replace(QStringLiteral("&gt;"), QStringLiteral(">")).replace(QStringLiteral("&quot;"), QStringLiteral("\"")).replace(QStringLiteral("&apos;"), QStringLiteral("'")).replace(QStringLiteral("&amp;"), QStringLiteral("&"));
        return o;
    };
    // the formulas the part holds for its columns follow the renames
    auto renamedFormula = [&](QString formula) {
        for (int i = 0; i < fileNames.size() && i < names.size(); ++i)
            if (fileNames[i] != names[i]) formula = renameColumnInFormula(formula, table, fileNames[i], names[i], true);
        return formula;
    };
    QString result;
    result.reserve(text.size() + 64);
    static const QRegularExpression tag(QStringLiteral("<tableColumn\\b[^>]*>"));
    static const QRegularExpression nameAttribute(QStringLiteral("(\\s)name=\"([^\"]*)\""));
    static const QRegularExpression formulaElement(QStringLiteral("(<(?:calculatedColumnFormula|totalsRowFormula)\\b[^>]*>)([^<]*)(</)"));
    int at = 0, column = 0;
    QRegularExpressionMatchIterator it = tag.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        result += text.mid(at, m.capturedStart() - at);
        QString element = m.captured();
        if (column < names.size() && column < fileNames.size() && names[column] != fileNames[column]) {
            const QRegularExpressionMatch a = nameAttribute.match(element);
            if (a.hasMatch()) element.replace(a.capturedStart(), a.capturedLength(), a.captured(1) + QStringLiteral("name=\"") + escapeAttribute(names[column]) + QLatin1Char('"'));
        }
        result += element;
        at = m.capturedEnd();
        ++column;
    }
    result += text.mid(at);
    // (the formulas)
    QString final;
    final.reserve(result.size());
    at = 0;
    QRegularExpressionMatchIterator fit = formulaElement.globalMatch(result);
    while (fit.hasNext()) {
        const QRegularExpressionMatch m = fit.next();
        final += result.mid(at, m.capturedStart() - at);
        final += m.captured(1) + escapeText(renamedFormula(unescapeXml(m.captured(2)))) + m.captured(3);
        at = m.capturedEnd();
    }
    final += result.mid(at);
    *out = final.toUtf8();
    return true;
}

} // namespace jp::office::sheet
