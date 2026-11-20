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

// The names of the sheets that were changed to be valid in the .xlsx form (by the name the document gave); a reference to one is written with the new name. Set for the length of one
// conversion by the callers below.
thread_local const QHash<QString, QString> *t_renames = nullptr;
// The numbers of the other files' workbooks, when the caller keeps them.
thread_local const std::function<int(const QString &)> *t_links = nullptr;

QString renamed(const QString &sheet)
{
    if (!t_renames) return sheet;
    const auto it = t_renames->constFind(sheet);
    return it == t_renames->constEnd() ? sheet : it.value();
}

QString sheetPrefix(const QString &sheet)
{
    if (sheet.isEmpty()) return QString();
    const QString name = renamed(sheet);
    return (needsQuotes(name) ? quoted(name) : name) + QLatin1Char('!');
}

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
    const QString first = renamed(a.sheet), last = renamed(b.sheet);
    const QString names = first + QLatin1Char(':') + last;
    const QString prefix = (needsQuotes(first) || needsQuotes(last) ? quoted(names) : names) + QLatin1Char('!');
    *out = prefix + a.cell + (a.cell == b.cell ? QString() : QLatin1Char(':') + b.cell);
    return true;
}

// ---- the names of functions: the two tables, one each way

struct NamePair {
    const char *odf;     // as the standard writes it
    const char *model;   // as the .xlsx form writes it (without the prefix the newer ones carry)
};

// Names that are not the same in the two forms. The first of each pair that has the model's name twice is the name the model has back.
const NamePair kNames[] = {
    {"ERROR.TYPE", "ERROR.TYPE"},      {"ERRORTYPE", "ERROR.TYPE"},         // (the standard has the dot, an older form of it has none)
    {"FORMULA", "FORMULATEXT"},        {"CEILING", "CEILING.MATH"},         {"FLOOR", "FLOOR.MATH"},        // (the standard's CEILING and FLOOR take a mode: the model's .MATH forms do)
    {"CHISQDIST", "CHISQ.DIST"},       {"CHISQINV", "CHISQ.INV"},
    {"LEGACY.CHIDIST", "CHIDIST"},     {"LEGACY.CHIINV", "CHIINV"},         {"LEGACY.FDIST", "FDIST"},      {"LEGACY.FINV", "FINV"},
    {"LEGACY.NORMSDIST", "NORMSDIST"}, {"LEGACY.NORMSINV", "NORMSINV"},
};

const char *const kPrefixes[] = {"COM.MICROSOFT.", "ORG.OPENOFFICE.", "ORG.LIBREOFFICE."};
// The add-ins of the standard's first form: COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEDATE is EDATE.
const char *const kAddIns[] = {"COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GET", "COM.SUN.STAR.SHEET.ADDIN.DATEFUNCTIONS.GET", "COM.SUN.STAR.SHEET.ADDIN.FINANCIAL.GET"};

QString functionName(const QString &written) { return modelFunctionName(written); }

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
            } else if (c == QLatin1Char(':')) {
                // the range operator between two operands outside brackets, as in [.A2]:INDEX(...): the same operator, kept as it is
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
        QString text;
        if (inner.startsWith(QLatin1Char('\'')) && inner.contains(QLatin1String("'#"))) {
            if (!t_links) return false;
            QString url;
            int k = 1;
            for (; k < inner.size(); ++k) {
                if (inner[k] == QLatin1Char('\'')) {
                    if (k + 1 < inner.size() && inner[k + 1] == QLatin1Char('\'')) ++k;
                    else break;
                }
                url += inner[k];
            }
            if (k + 1 >= inner.size() || inner[k + 1] != QLatin1Char('#') || url.isEmpty() || !referenceText(inner.mid(k + 2), &text)) return false;
            const int number = (*t_links)(url);
            if (number < 1) return false;
            const QString mark = QLatin1Char('[') + QString::number(number) + QLatin1Char(']');
            text = text.startsWith(QLatin1Char('\'')) ? QLatin1Char('\'') + mark + text.mid(1) : mark + text;
        } else if (!referenceText(inner, &text)) {
            return false;
        }
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
            // a function: the call is one operand, begun at its name (the table of values for several inputs, which the other format has no function for, keeps its values as constants)
            if (functionName(word) == QLatin1String("MULTIPLE.OPERATIONS")) return false;
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

namespace {
struct RenameScope {
    explicit RenameScope(const QHash<QString, QString> *renames) : old(t_renames) { t_renames = renames; }
    ~RenameScope() { t_renames = old; }
    const QHash<QString, QString> *old;
};
} // namespace

QString odfFormulaToText(const QString &formula, bool *ok, const QHash<QString, QString> *sheetNames, const std::function<int(const QString &)> *links)
{
    RenameScope scope(sheetNames);
    struct LinkScope {
        explicit LinkScope(const std::function<int(const QString &)> *l) : old(t_links) { t_links = l; }
        ~LinkScope() { t_links = old; }
        const std::function<int(const QString &)> *old;
    } linkScope(links);
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

QString odfRangeToText(const QString &address, bool *ok, const QHash<QString, QString> *sheetNames)
{
    RenameScope scope(sheetNames);
    QString text;
    const QString a = address.trimmed();
    const bool good = !a.isEmpty() && referenceText(a, &text);
    if (ok) *ok = good;
    return good ? text : QString();
}

QString modelFunctionName(const QString &odfName)
{
    QString name = odfName.toUpper();
    for (const char *add : kAddIns)
        if (name.startsWith(QLatin1String(add))) return name.mid(int(qstrlen(add)));
    // a name with a prefix of the standard's own is the name without it, and the forms the .xlsx format has (CEILING.XCL) are the plain names
    for (const NamePair &pair : kNames)
        if (name == QLatin1String(pair.odf)) return QLatin1String(pair.model);
    for (const char *p : kPrefixes)
        if (name.startsWith(QLatin1String(p))) {
            name = name.mid(int(qstrlen(p)));
            if (name.endsWith(QLatin1String(".XCL"))) return name.chopped(4);
            return name;
        }
    return name;
}

QString odfFunctionName(const QString &modelName)
{
    const QString name = modelName.toUpper();
    // the forms the .xlsx format has, of the two that the standard's own forms differ from
    if (name == QLatin1String("CEILING")) return QStringLiteral("COM.MICROSOFT.CEILING.XCL");
    if (name == QLatin1String("FLOOR")) return QStringLiteral("COM.MICROSOFT.FLOOR.XCL");
    for (const NamePair &pair : kNames)
        if (name == QLatin1String(pair.model) && QLatin1String(pair.odf) != QLatin1String("ERRORTYPE")) return QLatin1String(pair.odf);
    // a function the .xlsx form added after its first edition is written with a prefix in the standard's form
    if (prefixedFunctionNames().contains(name)) return QStringLiteral("COM.MICROSOFT.") + name;
    return name;
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

namespace jp::office::sheet {

QString odfLinkAddress(const QString &target)
{
    QString t = target;
    t.replace(QLatin1Char('\\'), QLatin1Char('/'));
    int scheme = 0;
    while (scheme < t.size() && ((t[scheme].isLetterOrNumber() && t[scheme].unicode() < 128) || t[scheme] == QLatin1Char('+') || t[scheme] == QLatin1Char('-') || t[scheme] == QLatin1Char('.'))) ++scheme;
    const bool schemed = scheme >= 2 && scheme < t.size() && t[scheme] == QLatin1Char(':') && t[0].isLetter();
    QString prefix;
    if (!schemed) {
        const bool drive = t.size() >= 3 && t[0].isLetter() && t[0].unicode() < 128 && t[1] == QLatin1Char(':') && t[2] == QLatin1Char('/');
        if (drive) prefix = QStringLiteral("file:///");
        else if (t.startsWith(QLatin1String("//"))) {
            prefix = QStringLiteral("file:");
        } else if (t.startsWith(QLatin1Char('/'))) {
            prefix = QStringLiteral("file://");
        }
    }
    const auto hex = [](QChar c) { return c.unicode() < 128 && (c.isDigit() || (c.toLower() >= QLatin1Char('a') && c.toLower() <= QLatin1Char('f'))); };
    QString out = prefix;
    for (int i = 0; i < t.size(); ++i) {
        const QChar c = t[i];
        const ushort u = c.unicode();
        const bool plain = (u < 128 && (c.isLetterOrNumber() || QLatin1String("-._~/:@!$&'()*+,;=").contains(c))) || u >= 128 || (schemed && c == QLatin1Char('?'));
        // (a percent sign that begins an escape of an address that has them already is kept)
        const bool escape = c == QLatin1Char('%') && schemed && i + 2 < t.size() && hex(t[i + 1]) && hex(t[i + 2]);
        if (plain || escape) {
            out += c;
            continue;
        }
        const QByteArray bytes = QString(c).toUtf8();
        for (const char b : bytes) out += QLatin1Char('%') + QString::number(uchar(b), 16).toUpper().rightJustified(2, QLatin1Char('0'));
    }
    return out;
}

int OdsLinkTable::add(const QString &target, const QString &address)
{
    const auto it = m_byAddress.constFind(address);
    if (it != m_byAddress.constEnd()) return it.value();
    if (m_targets.size() >= kMost) return 0;
    m_targets.append(target);
    m_byAddress.insert(address, int(m_targets.size()));
    return int(m_targets.size());
}

void OdsLinkTable::addNumbered(int number, const QString &target, const QString &address)
{
    if (number < 1 || number > kMost) return;
    while (m_targets.size() < number) m_targets.append(QString());
    m_targets[number - 1] = target;
    if (!m_byAddress.contains(address)) m_byAddress.insert(address, number);
}

int OdsLinkTable::numberOf(const QString &address) const
{
    m_examined.fetch_add(1, std::memory_order_relaxed);
    return m_byAddress.value(address, 0);
}

} // namespace jp::office::sheet
