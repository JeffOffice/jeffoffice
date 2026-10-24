#include "sheet/odsformula.h"

#include "calc/formula.h"
#include "sheet/entry.h"

#include <QRegularExpression>
#include <QStringList>

#include <vector>

namespace jp::office::sheet {

namespace {

bool looksLikeCellName(const QString &name)
{
    static const QRegularExpression cell(QStringLiteral("^[A-Za-z]{1,3}[0-9]+$"));
    static const QRegularExpression rc(QStringLiteral("^[Rr][0-9]*[Cc][0-9]*$"));
    return cell.match(name).hasMatch() || rc.match(name).hasMatch();
}

bool needsQuotes(const QString &name)
{
    static const QRegularExpression plain(QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$"));
    if (!plain.match(name).hasMatch()) return true;
    const QString up = name.toUpper();
    return up == QLatin1String("TRUE") || up == QLatin1String("FALSE") || looksLikeCellName(name);
}

QString quoted(const QString &name)
{
    QString q = name;
    q.replace(QLatin1Char('\''), QLatin1String("''"));
    return QLatin1Char('\'') + q + QLatin1Char('\'');
}

QString sheetPrefix(const QString &sheet) { return sheet.isEmpty() ? QString() : (needsQuotes(sheet) ? quoted(sheet) : sheet) + QLatin1Char('!'); }

// One end of a reference: [$]sheet.cell, with the sheet perhaps in quotes or left out (".A1").
struct Part {
    QString sheet;
    QString cell;
    bool hasSheet = false;
};

bool validCellPart(const QString &cell)
{
    static const QRegularExpression a1(QStringLiteral("^\\$?[A-Za-z]{1,3}\\$?[0-9]+$"));
    static const QRegularExpression col(QStringLiteral("^\\$?[A-Za-z]{1,3}$"));
    static const QRegularExpression row(QStringLiteral("^\\$?[0-9]+$"));
    return a1.match(cell).hasMatch() || col.match(cell).hasMatch() || row.match(cell).hasMatch() || cell == QLatin1String("#REF!");
}

bool parsePart(const QString &text, Part *out)
{
    QString p = text;
    int i = 0;
    if (p.startsWith(QLatin1Char('$')) && p.size() > 1 && (p[1] == QLatin1Char('\'') || p[1].isLetter() || p[1] == QLatin1Char('_'))) i = 1;   // (the sheet is absolute: no matter here)
    QString sheet;
    if (i < p.size() && p[i] == QLatin1Char('\'')) {
        ++i;
        for (;;) {
            if (i >= p.size()) return false;
            if (p[i] == QLatin1Char('\'')) {
                if (i + 1 < p.size() && p[i + 1] == QLatin1Char('\'')) {
                    sheet += QLatin1Char('\'');
                    i += 2;
                    continue;
                }
                ++i;
                break;
            }
            sheet += p[i++];
        }
        if (i >= p.size() || p[i] != QLatin1Char('.')) return false;
        ++i;
        out->hasSheet = true;
    } else {
        const int dot = int(p.indexOf(QLatin1Char('.'), i));
        if (dot < 0) return false;
        sheet = p.mid(i, dot - i);
        i = dot + 1;
        out->hasSheet = !sheet.isEmpty();
    }
    out->sheet = sheet;
    out->cell = p.mid(i);
    return validCellPart(out->cell);
}

// Splits at the colons that are not inside a quoted sheet name.
QStringList splitOutsideQuotes(const QString &text, QChar sep)
{
    QStringList parts;
    QString cur;
    bool q = false;
    for (QChar c : text) {
        if (c == QLatin1Char('\'')) q = !q;
        if (c == sep && !q) {
            parts << cur;
            cur.clear();
        } else {
            cur += c;
        }
    }
    parts << cur;
    return parts;
}

// The inside of a reference in brackets, as the model writes it.
bool referenceText(const QString &inner, QString *out)
{
    if (inner == QLatin1String("#REF!") || inner == QLatin1String(".#REF!")) {
        *out = QStringLiteral("#REF!");
        return true;
    }
    const QStringList ends = splitOutsideQuotes(inner, QLatin1Char(':'));
    if (ends.size() < 1 || ends.size() > 2) return false;
    Part a, b;
    if (!parsePart(ends[0], &a)) return false;
    if (ends.size() == 1) {
        *out = sheetPrefix(a.sheet) + a.cell;
        return true;
    }
    if (!parsePart(ends[1], &b)) return false;
    if (!b.hasSheet || b.sheet == a.sheet) {
        *out = sheetPrefix(a.sheet) + a.cell + QLatin1Char(':') + b.cell;
        return true;
    }
    // two sheets: a range through them (the same cells in each)
    const QString names = a.sheet + QLatin1Char(':') + b.sheet;
    const QString prefix = (needsQuotes(a.sheet) || needsQuotes(b.sheet) ? quoted(names) : names) + QLatin1Char('!');
    *out = prefix + a.cell + (a.cell == b.cell ? QString() : QLatin1Char(':') + b.cell);
    return true;
}

// The names of functions that differ between the two formats (after the prefixes are taken off).
QString renamedFunction(const QString &name)
{
    static const struct {
        const char *odf;
        const char *model;
    } table[] = {{"FORMULA", "FORMULATEXT"}, {"MULTIPLE.OPERATIONS", "MULTIPLE.OPERATIONS"}};
    for (const auto &t : table)
        if (name == QLatin1String(t.odf)) return QLatin1String(t.model);
    return name;
}

QString functionName(const QString &written)
{
    QString name = written.toUpper();
    static const char *const prefixes[] = {"COM.MICROSOFT.", "ORG.OPENOFFICE.", "ORG.LIBREOFFICE.", "LEGACY."};
    for (const char *p : prefixes)
        if (name.startsWith(QLatin1String(p))) {
            name = name.mid(int(qstrlen(p)));
            break;
        }
    return renamedFunction(name);
}

bool identStart(QChar c) { return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\'); }
bool identPart(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); }

class Converter {
public:
    explicit Converter(const QString &f) : m_f(f) {}

    bool run(QString *result)
    {
        m_levels.push_back(Level());
        const int n = int(m_f.size());
        while (m_i < n) {
            const QChar c = m_f[m_i];
            if (c.isSpace()) {
                ++m_i;
                continue;
            }
            if (c == QLatin1Char('"')) {
                if (!text()) return false;
            } else if (c == QLatin1Char('[')) {
                if (!reference()) return false;
            } else if (c.isDigit() || (c == QLatin1Char('.') && m_i + 1 < n && m_f[m_i + 1].isDigit())) {
                number();
            } else if (c == QLatin1Char('{')) {
                if (!array()) return false;
            } else if (c == QLatin1Char('#')) {
                if (!errorLiteral()) return false;
            } else if (identStart(c)) {
                if (!name()) return false;
            } else if (c == QLatin1Char('(')) {
                closeUnion();
                m_levels.back().operandStart = m_out.size();
                const qsizetype start = m_out.size();
                m_out += c;
                m_levels.push_back(Level());
                m_levels.back().parentStart = start;
                ++m_i;
            } else if (c == QLatin1Char(')')) {
                if (m_levels.size() < 2) return false;
                closeUnion();
                const qsizetype start = m_levels.back().parentStart;
                m_levels.pop_back();
                m_out += c;
                // the group is one operand of the level outside it (a call, with its name)
                m_levels.back().operandStart = m_levels.back().callStart >= 0 ? m_levels.back().callStart : start;
                m_levels.back().callStart = -1;
                ++m_i;
            } else if (c == QLatin1Char(';')) {
                closeUnion();
                m_out += QLatin1Char(',');
                ++m_i;
            } else if (c == QLatin1Char('~')) {
                if (!m_levels.back().unionOpen) {
                    m_out.insert(m_levels.back().operandStart, QLatin1Char('('));
                    m_levels.back().unionOpen = true;
                }
                m_out += QLatin1Char(',');
                ++m_i;
            } else if (c == QLatin1Char('!')) {
                m_out += QLatin1Char(' ');
                ++m_i;
            } else if (QStringLiteral("+-*/^&=<>%").contains(c)) {
                closeUnion();
                m_out += c;
                ++m_i;
            } else {
                return false;
            }
        }
        if (m_levels.size() != 1) return false;
        closeUnion();
        *result = m_out;
        return !m_out.isEmpty();
    }

private:
    struct Level {
        qsizetype operandStart = 0;
        bool unionOpen = false;
        qsizetype parentStart = 0;   // where the "(" that opened this level is
        qsizetype callStart = -1;    // where the name of the function this level is the arguments of begins
    };

    void closeUnion()
    {
        if (m_levels.back().unionOpen) {
            m_out += QLatin1Char(')');
            m_levels.back().unionOpen = false;
        }
    }

    bool text()
    {
        const int n = int(m_f.size());
        const qsizetype start = m_out.size();
        m_out += m_f[m_i++];
        while (m_i < n) {
            const QChar c = m_f[m_i++];
            m_out += c;
            if (c == QLatin1Char('"')) {
                if (m_i < n && m_f[m_i] == QLatin1Char('"')) {
                    m_out += m_f[m_i++];
                    continue;
                }
                m_levels.back().operandStart = start;
                return true;
            }
        }
        return false;
    }

    bool reference()
    {
        const int n = int(m_f.size());
        int j = m_i + 1;
        bool q = false;
        while (j < n && (q || m_f[j] != QLatin1Char(']'))) {
            if (m_f[j] == QLatin1Char('\'')) q = !q;
            ++j;
        }
        if (j >= n) return false;
        const QString inner = m_f.mid(m_i + 1, j - m_i - 1).trimmed();
        // another file's cells: 'file:///x.ods'#$Sheet1.A1
        if (inner.startsWith(QLatin1Char('\'')) && inner.contains(QLatin1String("'#"))) return false;
        QString text;
        if (!referenceText(inner, &text)) return false;
        m_levels.back().operandStart = m_out.size();
        m_out += text;
        m_i = j + 1;
        return true;
    }

    void number()
    {
        const int n = int(m_f.size());
        const qsizetype start = m_out.size();
        while (m_i < n && (m_f[m_i].isDigit() || m_f[m_i] == QLatin1Char('.'))) m_out += m_f[m_i++];
        if (m_i < n && (m_f[m_i] == QLatin1Char('E') || m_f[m_i] == QLatin1Char('e'))) {
            int k = m_i + 1;
            if (k < n && (m_f[k] == QLatin1Char('+') || m_f[k] == QLatin1Char('-'))) ++k;
            if (k < n && m_f[k].isDigit()) {
                while (m_i < k) m_out += m_f[m_i++];
                while (m_i < n && m_f[m_i].isDigit()) m_out += m_f[m_i++];
            }
        }
        m_levels.back().operandStart = start;
    }

    bool array()
    {
        const int n = int(m_f.size());
        const qsizetype start = m_out.size();
        m_out += m_f[m_i++];
        while (m_i < n) {
            const QChar c = m_f[m_i];
            if (c == QLatin1Char('"')) {
                // a text in the array
                m_out += m_f[m_i++];
                while (m_i < n) {
                    const QChar d = m_f[m_i++];
                    m_out += d;
                    if (d == QLatin1Char('"')) {
                        if (m_i < n && m_f[m_i] == QLatin1Char('"')) {
                            m_out += m_f[m_i++];
                            continue;
                        }
                        break;
                    }
                }
                continue;
            }
            ++m_i;
            if (c == QLatin1Char('}')) {
                m_out += c;
                m_levels.back().operandStart = start;
                return true;
            }
            if (c == QLatin1Char(';')) m_out += QLatin1Char(',');
            else if (c == QLatin1Char('|')) m_out += QLatin1Char(';');
            else if (!c.isSpace()) m_out += c;
        }
        return false;
    }

    bool errorLiteral()
    {
        static const char *const errors[] = {"#NULL!", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#N/A"};
        for (const char *e : errors) {
            if (m_f.mid(m_i, int(qstrlen(e))).compare(QLatin1String(e), Qt::CaseInsensitive) == 0) {
                m_levels.back().operandStart = m_out.size();
                m_out += QLatin1String(e);
                m_i += int(qstrlen(e));
                return true;
            }
        }
        return false;
    }

    bool name()
    {
        const int n = int(m_f.size());
        int j = m_i;
        while (j < n && identPart(m_f[j])) ++j;
        const QString word = m_f.mid(m_i, j - m_i);
        int k = j;
        while (k < n && m_f[k].isSpace()) ++k;
        if (k < n && m_f[k] == QLatin1Char('(')) {
            // a function: the call is one operand, begun at its name
            closeUnion();
            const qsizetype start = m_out.size();
            m_out += functionName(word) + QLatin1Char('(');
            m_levels.push_back(Level());
            m_levels.back().parentStart = start;
            // (the level outside remembers where the call began, for when it ends)
            m_levels[m_levels.size() - 2].callStart = start;
            m_i = k + 1;
            return true;
        }
        m_levels.back().operandStart = m_out.size();
        m_out += word;
        m_i = j;
        return true;
    }

    const QString &m_f;
    QString m_out;
    int m_i = 0;
    std::vector<Level> m_levels;
};

} // namespace

QString odfFormulaToText(const QString &formula, bool *ok)
{
    auto fail = [&]() {
        if (ok) *ok = false;
        return QString();
    };
    QString f = formula.trimmed();
    static const QRegularExpression prefix(QStringLiteral("^([A-Za-z]+):="));
    const QRegularExpressionMatch m = prefix.match(f);
    if (m.hasMatch()) {
        const QString ns = m.captured(1).toLower();
        f = f.mid(int(m.capturedLength()));
        if (ns == QLatin1String("msoxl")) {
            // the other format's own form: as the model keeps it
            if (ok) *ok = !f.isEmpty();
            return f;
        }
        if (ns != QLatin1String("of") && ns != QLatin1String("oooc")) return fail();
    } else if (f.startsWith(QLatin1Char('='))) {
        f = f.mid(1);
    }
    if (f.trimmed().isEmpty()) return fail();
    QString out;
    Converter c(f);
    if (!c.run(&out)) return fail();
    out = toFileFormula(out);
    // (a text that the engine cannot read is not kept as a formula)
    jp::calc::ParseOptions po;
    if (!jp::calc::parseFormula(out, po).ok()) return fail();
    if (ok) *ok = true;
    return out;
}

QString odfRangeToText(const QString &address, bool *ok)
{
    QString text;
    const QString a = address.trimmed();
    const bool good = !a.isEmpty() && referenceText(a, &text);
    if (ok) *ok = good;
    return good ? text : QString();
}

bool odfCellOf(const QString &text, quint32 *row, quint32 *col)
{
    static const QRegularExpression cell(QStringLiteral("^\\$?([A-Za-z]{1,3})\\$?([0-9]+)$"));
    const QRegularExpressionMatch m = cell.match(text.trimmed());
    if (!m.hasMatch()) return false;
    quint32 c = 0;
    for (const QChar ch : m.captured(1).toUpper()) c = c * 26 + quint32(ch.unicode() - 'A' + 1);
    bool good = false;
    const quint32 r = m.captured(2).toUInt(&good);
    if (!good || r == 0 || c == 0 || c > 16384 || r > 1048576) return false;
    *row = r - 1;
    *col = c - 1;
    return true;
}

} // namespace jp::office::sheet
