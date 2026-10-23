#include "sheet/cellref.h"

#include <algorithm>
#include <cstring>

namespace jp::office::sheet {

namespace {

// 1 to 3 letters, in either case, as a 0-based column; false if they aren't, or the column is past the grid.
bool columnFromLetters(QStringView letters, quint32 *col)
{
    if (letters.isEmpty() || letters.size() > 3) return false;
    quint32 v = 0;
    for (const QChar ch : letters) {
        ushort c = ch.unicode();
        if (c >= 'a' && c <= 'z') c = ushort(c - 32);
        if (c < 'A' || c > 'Z') return false;
        v = v * 26 + (c - 'A' + 1);
    }
    if (v < 1 || v > kMaxColumns) return false;
    *col = v - 1;
    return true;
}

// Digits as a 1-based row inside the grid.
bool rowFromDigits(QStringView digits, quint32 *row)
{
    if (digits.isEmpty() || digits.size() > 7) return false;
    quint32 v = 0;
    for (const QChar ch : digits) {
        if (ch.unicode() < '0' || ch.unicode() > '9') return false;
        v = v * 10 + (ch.unicode() - '0');
    }
    if (v < 1 || v > kMaxRows) return false;
    *row = v - 1;
    return true;
}

// A cell reference as written: its parts and whether each is pinned by a "$".
struct Ref {
    quint32 row = 0, col = 0;
    bool rowAbs = false, colAbs = false;
};

// "$B$3", "b3": false if the whole text isn't one.
bool parseRef(QStringView text, Ref *ref)
{
    *ref = Ref();
    qsizetype i = 0;
    if (i < text.size() && text[i] == QLatin1Char('$')) {
        ref->colAbs = true;
        ++i;
    }
    const qsizetype letters = i;
    while (i < text.size() && text[i].isLetter() && text[i].unicode() < 128) ++i;
    if (!columnFromLetters(text.mid(letters, i - letters), &ref->col)) return false;
    if (i < text.size() && text[i] == QLatin1Char('$')) {
        ref->rowAbs = true;
        ++i;
    }
    return rowFromDigits(text.mid(i), &ref->row);
}

// A column alone ("$B", "c"), as in a reference to whole columns.
bool parseColumnOnly(QStringView text, quint32 *col, bool *absolute)
{
    *absolute = !text.isEmpty() && text[0] == QLatin1Char('$');
    return columnFromLetters(*absolute ? text.mid(1) : text, col);
}

// ---- the pieces a formula's text is made of

bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\');
}

bool isWordStart(QChar c) { return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\') || c == QLatin1Char('$'); }

bool isDigit(QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; }

// The end of the text in quotes that starts at `i` (a quote, which is doubled to stand for itself inside).
int endOfQuoted(const QString &s, int i)
{
    const QChar quote = s[i];
    for (++i; i < s.size(); ++i)
        if (s[i] == quote) {
            if (i + 1 < s.size() && s[i + 1] == quote) {
                ++i;
                continue;
            }
            return i + 1;
        }
    return int(s.size());
}

// The end of the square brackets that open at `i`, which may hold more of them (a table's columns); an apostrophe makes the next character plain.
int endOfBrackets(const QString &s, int i)
{
    int depth = 0;
    for (; i < s.size(); ++i) {
        if (s[i] == QLatin1Char('\'') && i + 1 < s.size()) {
            ++i;
        } else if (s[i] == QLatin1Char('[')) {
            ++depth;
        } else if (s[i] == QLatin1Char(']') && --depth == 0) {
            return i + 1;
        }
    }
    return int(s.size());
}

// The end of an error written as a literal ("#REF!", "#N/A", "#DIV/0!") that starts at `i`.
int endOfError(const QString &s, int i)
{
    ++i;
    while (i < s.size() && (s[i].isLetterOrNumber() || s[i] == QLatin1Char('/') || s[i] == QLatin1Char('_'))) ++i;
    if (i < s.size() && (s[i] == QLatin1Char('!') || s[i] == QLatin1Char('?'))) ++i;
    return i;
}

// The end of a number that starts at `i`: digits, a fraction, an exponent.
int endOfNumber(const QString &s, int i)
{
    while (i < s.size() && isDigit(s[i])) ++i;
    if (i < s.size() && s[i] == QLatin1Char('.')) {
        ++i;
        while (i < s.size() && isDigit(s[i])) ++i;
    }
    if (i < s.size() && (s[i] == QLatin1Char('E') || s[i] == QLatin1Char('e'))) {
        int j = i + 1;
        if (j < s.size() && (s[j] == QLatin1Char('+') || s[j] == QLatin1Char('-'))) ++j;
        if (j < s.size() && isDigit(s[j])) {
            while (j < s.size() && isDigit(s[j])) ++j;
            i = j;
        }
    }
    return i;
}

// The end of a word (a name, a reference, a function) that starts at `i`.
int endOfWord(const QString &s, int i)
{
    while (i < s.size() && (isWordChar(s[i]) || s[i] == QLatin1Char('$'))) ++i;
    return i;
}

QString refText(const Ref &r, qint64 row, qint64 col)
{
    return QString(r.colAbs ? "$" : "") + columnName(quint32(col)) + (r.rowAbs ? "$" : "") + QString::number(row + 1);
}

} // namespace

bool parseCellRef(QStringView text, quint32 *row, quint32 *col)
{
    Ref r;
    if (!parseRef(text, &r)) return false;
    *row = r.row;
    *col = r.col;
    return true;
}

bool parseRangeRef(QStringView text, CellRange *out)
{
    *out = CellRange();
    const qsizetype colon = text.indexOf(QLatin1Char(':'));
    Ref a, b;
    if (colon < 0) {
        if (!parseRef(text, &a)) return false;
        b = a;
    } else if (!parseRef(text.left(colon), &a) || !parseRef(text.mid(colon + 1), &b)) {
        return false;
    }
    out->row1 = std::min(a.row, b.row);
    out->row2 = std::max(a.row, b.row);
    out->col1 = std::min(a.col, b.col);
    out->col2 = std::max(a.col, b.col);
    out->valid = true;
    return true;
}

QString columnName(quint32 col)
{
    if (col >= kMaxColumns) return {};
    QString name;
    for (quint32 n = col + 1; n > 0; n = (n - 1) / 26) name.prepend(QChar(ushort('A' + (n - 1) % 26)));
    return name;
}

QString cellName(quint32 row, quint32 col) { return columnName(col) + QString::number(qint64(row) + 1); }

QString rangeName(const CellRange &range)
{
    if (!range.valid) return {};
    if (range.row1 == range.row2 && range.col1 == range.col2) return cellName(range.row1, range.col1);
    return cellName(range.row1, range.col1) + QLatin1Char(':') + cellName(range.row2, range.col2);
}

QString shiftFormula(const QString &formula, int rows, int cols)
{
    if (rows == 0 && cols == 0) return formula;
    const QString &s = formula;
    const int n = int(s.size());
    QString out;
    out.reserve(n + 8);
    const QString refError = QStringLiteral("#REF!");
    auto inGrid = [](qint64 row, qint64 col) { return row >= 0 && row < qint64(kMaxRows) && col >= 0 && col < qint64(kMaxColumns); };
    auto moved = [&](const Ref &r, qint64 *row, qint64 *col) {
        *row = r.rowAbs ? qint64(r.row) : qint64(r.row) + rows;
        *col = r.colAbs ? qint64(r.col) : qint64(r.col) + cols;
    };
    // Not a reference when a name, function, sheet, or table goes on from it.
    auto endsRef = [&](int end) { return !(end < n && (s[end] == QLatin1Char('(') || s[end] == QLatin1Char('!') || s[end] == QLatin1Char('['))); };

    // Whole rows ("1:3", "$2:$5") at `i`: the text that stands for them moved, and where they end; false if what is there isn't whole rows.
    auto wholeRows = [&](int i, int *stop, QString *text) {
        int j = i;
        const bool abs1 = s[j] == QLatin1Char('$');
        if (abs1) ++j;
        const int d1 = j;
        while (j < n && isDigit(s[j])) ++j;
        if (j == d1 || j >= n || s[j] != QLatin1Char(':')) return false;
        int k = j + 1;
        const bool abs2 = k < n && s[k] == QLatin1Char('$');
        if (abs2) ++k;
        const int d2 = k;
        while (k < n && isDigit(s[k])) ++k;
        quint32 r1 = 0, r2 = 0;
        if (k == d2 || (k < n && (isWordChar(s[k]) || s[k] == QLatin1Char('(') || s[k] == QLatin1Char('[')))) return false;
        if (!rowFromDigits(QStringView(s).mid(d1, j - d1), &r1) || !rowFromDigits(QStringView(s).mid(d2, k - d2), &r2)) return false;
        const qint64 n1 = abs1 ? qint64(r1) : qint64(r1) + rows, n2 = abs2 ? qint64(r2) : qint64(r2) + rows;
        if (n1 >= 0 && n1 < qint64(kMaxRows) && n2 >= 0 && n2 < qint64(kMaxRows))
            *text = QString(abs1 ? "$" : "") + QString::number(n1 + 1) + ':' + (abs2 ? "$" : "") + QString::number(n2 + 1);
        else
            *text = refError;
        *stop = k;
        return true;
    };
    // A reference, a range of cells, or whole columns in the word at `i` (ending at `end`): the text that stands for it moved, and where it ends.
    auto reference = [&](int i, int end, int *stop, QString *text) {
        if (!endsRef(end)) return false;
        const QStringView word = QStringView(s).mid(i, end - i);
        Ref a;
        if (parseRef(word, &a)) {
            // "A1:B2": both ends move together, and either leaving the grid loses the whole.
            Ref b = a;
            bool area = false;
            *stop = end;
            if (end + 1 < n && s[end] == QLatin1Char(':') && isWordStart(s[end + 1])) {
                const int end2 = endOfWord(s, end + 1);
                if (endsRef(end2) && parseRef(QStringView(s).mid(end + 1, end2 - end - 1), &b)) {
                    area = true;
                    *stop = end2;
                } else {
                    b = a;
                }
            }
            qint64 ra, ca, rb = 0, cb = 0;
            moved(a, &ra, &ca);
            if (area) moved(b, &rb, &cb);
            if (!inGrid(ra, ca) || (area && !inGrid(rb, cb)))
                *text = refError;
            else
                *text = refText(a, ra, ca) + (area ? QString(QLatin1Char(':')) + refText(b, rb, cb) : QString());
            return true;
        }
        quint32 colA = 0, colB = 0;
        bool absA = false, absB = false;
        if (end + 1 < n && s[end] == QLatin1Char(':') && parseColumnOnly(word, &colA, &absA)) {
            // whole columns: "A:C", "$A:$B"
            const int end2 = endOfWord(s, end + 1);
            if (endsRef(end2) && parseColumnOnly(QStringView(s).mid(end + 1, end2 - end - 1), &colB, &absB)) {
                const qint64 na = absA ? qint64(colA) : qint64(colA) + cols, nb = absB ? qint64(colB) : qint64(colB) + cols;
                if (na >= 0 && na < qint64(kMaxColumns) && nb >= 0 && nb < qint64(kMaxColumns))
                    *text = QString(absA ? "$" : "") + columnName(quint32(na)) + ':' + (absB ? "$" : "") + columnName(quint32(nb));
                else
                    *text = refError;
                *stop = end2;
                return true;
            }
        }
        return false;
    };

    int i = 0;
    while (i < n) {
        const QChar c = s[i];
        int end = i + 1;
        QString text;
        int stop = 0;
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            end = endOfQuoted(s, i);
        } else if (c == QLatin1Char('[')) {
            end = endOfBrackets(s, i);
        } else if (c == QLatin1Char('#')) {
            end = endOfError(s, i);
        } else if ((isDigit(c) || c == QLatin1Char('$')) && wholeRows(i, &stop, &text)) {
            out += text;
            i = stop;
            continue;
        } else if (isDigit(c) || (c == QLatin1Char('.') && i + 1 < n && isDigit(s[i + 1]))) {
            end = endOfNumber(s, i);
        } else if (isWordStart(c)) {
            end = endOfWord(s, i);
            if (reference(i, end, &stop, &text)) {
                out += text;
                i = stop;
                continue;
            }
        }
        out += QStringView(s).mid(i, end - i);
        i = end;
    }
    return out;
}

QStringList formulaFunctions(const QString &formula)
{
    QStringList names;
    const QString &s = formula;
    const int n = int(s.size());
    int i = 0;
    while (i < n) {
        const QChar c = s[i];
        int end = i + 1;
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            end = endOfQuoted(s, i);
        } else if (c == QLatin1Char('[')) {
            end = endOfBrackets(s, i);
        } else if (c == QLatin1Char('#')) {
            end = endOfError(s, i);
        } else if (isDigit(c) || (c == QLatin1Char('.') && i + 1 < n && isDigit(s[i + 1]))) {
            end = endOfNumber(s, i);
        } else if (isWordStart(c)) {
            end = endOfWord(s, i);
            if (end < n && s[end] == QLatin1Char('(')) {
                QString name = s.mid(i, end - i);
                // what a newer function is written with in a file is not its name
                for (bool stripped = true; stripped;) {
                    stripped = false;
                    for (const char *prefix : {"_xlfn.", "_xlws.", "_xll."})
                        if (name.startsWith(QLatin1String(prefix), Qt::CaseInsensitive)) {
                            name.remove(0, int(strlen(prefix)));
                            stripped = true;
                        }
                }
                if (!name.isEmpty()) names << name.toUpper();
            }
        }
        i = end;
    }
    return names;
}

} // namespace jp::office::sheet
