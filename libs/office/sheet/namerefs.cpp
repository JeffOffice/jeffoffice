#include "sheet/namerefs.h"

#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace jp::office::sheet {

namespace {

bool wordChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); }
bool wordStart(QChar c) { return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\'); }

// The end (after the closing quote) of the text in quotes that begins at `i`; a quote is doubled to stand for itself.
int quotedEnd(const QString &s, int i)
{
    const QChar q = s[i];
    int j = i + 1;
    while (j < s.size()) {
        if (s[j] == q) {
            if (j + 1 < s.size() && s[j + 1] == q) {
                j += 2;
                continue;
            }
            return j + 1;
        }
        ++j;
    }
    return int(s.size());
}

int nextNonSpace(const QString &s, int i)
{
    while (i < s.size() && s[i] == QLatin1Char(' ')) ++i;
    return i;
}

// Calls `use(start, end, sheet)` for each word of the formula that can be a name, with the sheet written in front of it (empty for none).
template <typename F>
void eachNameUse(const QString &s, F use)
{
    const int n = int(s.size());
    int i = 0;
    QString sheet;
    bool external = false;   // a workbook's number in brackets was just read: what follows is another workbook's
    int sheetEnd = -1;   // where the "!" of a sheet that was just read is
    while (i < n) {
        const QChar c = s[i];
        if (c == QLatin1Char('"')) {
            i = quotedEnd(s, i);
            continue;
        }
        if (c == QLatin1Char('\'')) {
            const int end = quotedEnd(s, i);
            if (end < n && s[end] == QLatin1Char('!')) {
                sheet = s.mid(i + 1, end - i - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
                sheetEnd = end;
            }
            i = end;
            continue;
        }
        if (c == QLatin1Char('[')) {
            // a table's columns, or a workbook in brackets: passed over, brackets inside brackets and all
            external = i == 0 || !(wordChar(s[i - 1]) || s[i - 1] == QLatin1Char(']'));
            int depth = 0;
            while (i < n) {
                if (s[i] == QLatin1Char('[')) ++depth;
                else if (s[i] == QLatin1Char(']') && --depth == 0) {
                    ++i;
                    break;
                }
                ++i;
            }
            continue;
        }
        if (external && c == QLatin1Char('!')) {
            // "[1]!Total": a name of another workbook
            ++i;
            while (i < n && wordChar(s[i])) ++i;
            external = false;
            continue;
        }
        if (c.isDigit()) {
            while (i < n && wordChar(s[i])) ++i;
            continue;
        }
        if (!wordStart(c)) {
            if (c != QLatin1Char('!') && c != QLatin1Char('$')) {
                sheet.clear();
                external = false;
            }
            ++i;
            continue;
        }
        const int start = i;
        while (i < n && wordChar(s[i])) ++i;
        const int next = nextNonSpace(s, i);
        if (next < n && s[next] == QLatin1Char('!')) {
            sheet = s.mid(start, i - start);
            sheetEnd = next;
            i = next;
            continue;
        }
        external = false;
        const bool qualified = sheetEnd >= 0 && start == sheetEnd + 1;
        const QString own = qualified ? sheet : QString();
        sheet.clear();
        sheetEnd = -1;
        if (next < n && (s[next] == QLatin1Char('(') || s[next] == QLatin1Char('['))) continue;   // a function or a table
        // part of an address: "$A$1", a column "A:A", a row
        if (start > 0 && s[start - 1] == QLatin1Char('$')) continue;
        if (i < n && s[i] == QLatin1Char('$')) continue;
        if (next < n && s[next] == QLatin1Char(':')) continue;
        int back = start - 1;
        while (back >= 0 && s[back] == QLatin1Char(' ')) --back;
        if (back >= 0 && s[back] == QLatin1Char(':')) continue;
        use(start, i, own);
    }
}

} // namespace

QString renameNameInFormula(const QString &formula, const QString &oldName, const QString &newName, const std::function<bool(const QString &)> &applies, bool *changed)
{
    if (changed) *changed = false;
    if (formula.size() < oldName.size() || oldName.isEmpty()) return formula;
    struct Hit {
        int start, end;
    };
    std::vector<Hit> hits;
    eachNameUse(formula, [&](int start, int end, const QString &sheet) {
        if (end - start != oldName.size() || formula.mid(start, end - start).compare(oldName, Qt::CaseInsensitive) != 0) return;
        if (applies && !applies(sheet)) return;
        hits.push_back({start, end});
    });
    if (hits.empty()) return formula;
    QString out;
    out.reserve(formula.size() + int(hits.size()) * std::max<int>(0, int(newName.size()) - int(oldName.size())));
    int at = 0;
    for (const Hit &h : hits) {
        out += formula.mid(at, h.start - at) + newName;
        at = h.end;
    }
    out += formula.mid(at);
    if (changed) *changed = out != formula;
    return out;
}

bool formulaUsesName(const QString &formula, const QString &name)
{
    bool found = false;
    eachNameUse(formula, [&](int start, int end, const QString &) {
        if (end - start == name.size() && formula.mid(start, end - start).compare(name, Qt::CaseInsensitive) == 0) found = true;
    });
    return found;
}

QString nameProblem(const QString &name)
{
    if (name.isEmpty()) return QCoreApplication::translate("jp::office::sheet::Names", "A name needs at least one character.");
    if (name.size() > 255) return QCoreApplication::translate("jp::office::sheet::Names", "A name can have at most 255 characters.");
    if (name.startsWith(QLatin1String("_xlnm."), Qt::CaseInsensitive)) return QCoreApplication::translate("jp::office::sheet::Names", "Names that begin with _xlnm. belong to the program's own use, such as print areas. Choose another.");
    if (!wordStart(name[0])) return QCoreApplication::translate("jp::office::sheet::Names", "A name begins with a letter, an underscore, or a backslash.");
    for (const QChar c : name) {
        if (c == QLatin1Char(' ')) return QCoreApplication::translate("jp::office::sheet::Names", "A name can't have a space in it. Use an underscore or a dot instead.");
        if (!wordChar(c)) return QCoreApplication::translate("jp::office::sheet::Names", "A name can have only letters, digits, underscores, and dots in it.");
    }
    quint32 row = 0, col = 0;
    static const QRegularExpression rc(QStringLiteral("^(R(\\d+)?(C(\\d+)?)?|C(\\d+)?)$"), QRegularExpression::CaseInsensitiveOption);
    if (parseCellRef(name, &row, &col) || rc.match(name).hasMatch())
        return QCoreApplication::translate("jp::office::sheet::Names", "That is the address of a cell, so it can't be a name. Add a letter or an underscore to it.");
    // (the other program refuses the whole file for a name that begins as an address of the other style does, as R2D2 does: R, C, or RC and a number, and then something more)
    static const QRegularExpression rcMore(QStringLiteral("^(R\\d+|C\\d+|RC\\d+)."), QRegularExpression::CaseInsensitiveOption);
    if (rcMore.match(name).hasMatch())
        return QCoreApplication::translate("jp::office::sheet::Names", "A name that begins with R or C and a number can be taken for a row or a column. Add an underscore before it.");
    return QString();
}

} // namespace jp::office::sheet
