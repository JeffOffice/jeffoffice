#include "core/errorcheck.h"

#include "calc/value.h"
#include "core/formularefs.h"
#include "sheet/calculation.h"

#include <QCoreApplication>
#include <QLocale>
#include <QRegularExpression>

#include <algorithm>
#include <map>
#include <set>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::ErrorKind;
using jp::calc::Value;

namespace {

QString errorSentence(ErrorKind e)
{
    const char *c = "jp::crunch::ErrorCheck";
    switch (e) {
    case ErrorKind::Null: return QCoreApplication::translate(c, "The formula gives an error: the two blocks of cells it names have no cell in common.");
    case ErrorKind::Div0: return QCoreApplication::translate(c, "The formula gives an error: it divides by zero or by an empty cell.");
    case ErrorKind::Value: return QCoreApplication::translate(c, "The formula gives an error: a value is of the wrong kind, such as text where a number is needed.");
    case ErrorKind::Ref: return QCoreApplication::translate(c, "The formula gives an error: it points to a cell that is not there.");
    case ErrorKind::Name: return QCoreApplication::translate(c, "The formula gives an error: it uses a name or function that is not known.");
    case ErrorKind::Num: return QCoreApplication::translate(c, "The formula gives an error: a number in it is not allowed, or is too large or too small.");
    case ErrorKind::NA: return QCoreApplication::translate(c, "The formula gives an error: a value it looks for is not available.");
    case ErrorKind::Spill: return QCoreApplication::translate(c, "The formula gives an error: its results do not fit, because cells where they would go are not empty.");
    default: return QCoreApplication::translate(c, "The formula gives an error.");
    }
}

bool looksLikeNumber(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty()) return false;
    bool ok = false;
    QLocale::c().toDouble(t, &ok);
    if (!ok) QLocale().toDouble(t, &ok);
    return ok;
}

// A date kept as text with a year of two digits: 3/4/24, 3-4-24, or 3.4.24.
bool looksLikeTwoDigitYear(const QString &text)
{
    // (a quick look first: the text is at most 10 characters and holds a separator)
    if (text.size() < 6 || text.size() > 12 || (!text.contains(QLatin1Char('/')) && !text.contains(QLatin1Char('.')) && !text.contains(QLatin1Char('-')))) return false;
    static const QRegularExpression date(QStringLiteral("^\\s*\\d{1,2}([/.-])\\d{1,2}\\1\\d{2}\\s*$"));
    return date.match(text).hasMatch();
}

// The formula with each reference written as how far it is from the cell (R[1]C[-2]) where it moves with the cell, so that two cells with the same pattern have the same key.
QString relativeKey(const QString &formula, quint32 row, quint32 col)
{
    const QString text = formula.startsWith(QLatin1Char('=')) ? formula.mid(1) : formula;
    static const QRegularExpression end(QStringLiteral("^(\\$?)([A-Za-z]*)(\\$?)(\\d*)$"));
    QString key;
    int at = 0;
    for (const FormulaRef &ref : findReferences(text)) {
        key += text.mid(at, ref.start - at).toUpper();
        key += text.mid(ref.start, ref.bodyStart - ref.start).toUpper();
        const QString body = text.mid(ref.bodyStart, ref.start + ref.length - ref.bodyStart);
        const QStringList ends = body.split(QLatin1Char(':'));
        for (int i = 0; i < ends.size(); ++i) {
            const QRegularExpressionMatch m = end.match(ends[i]);
            const quint32 r = i == 0 ? ref.row1 : ref.row2, c = i == 0 ? ref.col1 : ref.col2;
            if (i > 0) key += QLatin1Char(':');
            if (!m.hasMatch()) {
                key += ends[i];
                continue;
            }
            if (!m.captured(4).isEmpty()) key += m.captured(3).isEmpty() ? QStringLiteral("R[%1]").arg(qint64(r) - qint64(row)) : QStringLiteral("R%1").arg(r + 1);
            if (!m.captured(2).isEmpty()) key += m.captured(1).isEmpty() ? QStringLiteral("C[%1]").arg(qint64(c) - qint64(col)) : QStringLiteral("C%1").arg(c + 1);
        }
        at = ref.start + ref.length;
    }
    return key + text.mid(at).toUpper();
}

// The cell `row`, `col` holds a number that was typed (no formula).
bool typedNumberAt(const EditSession &session, const Calculation *calc, int sheet, qint64 row, qint64 col)
{
    if (row < 0 || col < 0 || row >= qint64(kMaxRows) || col >= qint64(kMaxColumns)) return false;
    const std::optional<Cell> cell = calc->sheet(sheet).cells.find(quint32(row), quint32(col));
    return cell && !cell->hasFormula() && !(cell->flags & ArrayMember) && session.valueAt(sheet, quint32(row), quint32(col)).isNumber();
}

// The same reference with the end that is now at `from` (a row or column of the sheet, from 0) moved to `line`: the part of the text that names that end is rewritten. Which end of the
// text it is does not matter (K4:K2 has its first end below its second).
QString movedEnd(const QString &body, bool columns, quint32 from, quint32 line)
{
    static const QRegularExpression end(QStringLiteral("^(\\$?)([A-Za-z]*)(\\$?)(\\d*)$"));
    QStringList ends = body.split(QLatin1Char(':'));
    if (ends.size() != 2) return QString();
    for (QString &one : ends) {
        const QRegularExpressionMatch m = end.match(one);
        if (!m.hasMatch()) return QString();
        if (columns ? m.captured(2).toUpper() != columnName(from) : m.captured(4).toLongLong() - 1 != qint64(from)) continue;
        one = columns ? m.captured(1) + columnName(line) + m.captured(3) + m.captured(4) : m.captured(1) + m.captured(2) + m.captured(3) + QString::number(line + 1);
        return ends.join(QLatin1Char(':'));
    }
    return QString();
}

// The functions that total or summarize the cells of a range, the only ones whose range a neighboring number is likely to have been left out of (the table's categories are too wide: they
// hold INDEX, MATCH, and OFFSET, which take a range as a table to look in, and adding a row to it changes what they answer).
bool summarizesRange(const QString &name)
{
    static const std::set<QString> names = {QStringLiteral("SUM"), QStringLiteral("AVERAGE"), QStringLiteral("AVERAGEA"), QStringLiteral("COUNT"), QStringLiteral("COUNTA"), QStringLiteral("COUNTBLANK"),
                                            QStringLiteral("MIN"), QStringLiteral("MAX"), QStringLiteral("MINA"), QStringLiteral("MAXA"), QStringLiteral("PRODUCT"), QStringLiteral("SUBTOTAL"),
                                            QStringLiteral("AGGREGATE"), QStringLiteral("MEDIAN"), QStringLiteral("MODE"), QStringLiteral("STDEV"), QStringLiteral("STDEVP"), QStringLiteral("VAR"),
                                            QStringLiteral("VARP"), QStringLiteral("SUMSQ"), QStringLiteral("GEOMEAN"), QStringLiteral("HARMEAN")};
    return names.count(name) > 0;
}

// For each reference (in order): the name of the function it is a plain argument of (a reference alone between the brackets and commas), or an empty string. One pass over the text.
std::vector<QString> functionsOf(const QString &text, const std::vector<FormulaRef> &refs)
{
    std::vector<QString> out(refs.size());
    QString masked = text;
    bool quoted = false;
    for (QChar &ch : masked) {   // (text in quotes is not looked at)
        if (ch == QLatin1Char('"')) quoted = !quoted;
        else if (quoted) ch = QLatin1Char(' ');
    }
    std::vector<int> open;   // where each bracket that is still open begins
    size_t next = 0;
    for (int i = 0; i <= masked.size() && next < refs.size(); ++i) {
        while (next < refs.size() && refs[next].start == i) {
            const FormulaRef &ref = refs[next];
            int before = ref.start - 1, after = ref.end();
            while (before >= 0 && masked[before].isSpace()) --before;
            while (after < masked.size() && masked[after].isSpace()) ++after;
            const bool alone = before >= 0 && (masked[before] == QLatin1Char('(') || masked[before] == QLatin1Char(',')) && after < masked.size() && (masked[after] == QLatin1Char(',') || masked[after] == QLatin1Char(')'));
            if (alone && !open.empty()) {
                int n = open.back();
                int from = n;
                while (from > 0 && (masked[from - 1].isLetterOrNumber() || masked[from - 1] == QLatin1Char('.') || masked[from - 1] == QLatin1Char('_'))) --from;
                out[next] = masked.mid(from, n - from).toUpper();
            }
            ++next;
        }
        if (i == masked.size()) break;
        if (masked[i] == QLatin1Char('(')) open.push_back(i);
        else if (masked[i] == QLatin1Char(')') && !open.empty()) open.pop_back();
    }
    return out;
}

// Which cells a formula's references cover, asked one cell at a time. Ranges of one column (and single cells) and ranges of one row are each kept as sorted, joined stretches, so a question
// is a binary search; the few ranges of several rows and columns are looked through, and each one looked at is counted in `work`.
class Covering {
public:
    Covering(const std::vector<FormulaRef> &refs, qint64 *work) : m_work(work)
    {
        for (const FormulaRef &ref : refs) {
            if (!ref.sheet.isEmpty()) continue;
            if (ref.col1 == ref.col2) m_tall[ref.col1].push_back({ref.row1, ref.row2});
            else if (ref.row1 == ref.row2) m_wide[ref.row1].push_back({ref.col1, ref.col2});
            else m_blocks.push_back(&ref);
        }
        for (auto *group : {&m_tall, &m_wide})
            for (auto &[line, stretches] : *group) {
                std::sort(stretches.begin(), stretches.end());
                std::vector<std::pair<quint32, quint32>> joined;
                for (const auto &st : stretches) {
                    if (!joined.empty() && qint64(st.first) <= qint64(joined.back().second) + 1) joined.back().second = std::max(joined.back().second, st.second);
                    else joined.push_back(st);
                }
                stretches = std::move(joined);
            }
        *m_work += qint64(refs.size());
    }
    bool covers(qint64 r, qint64 c) const
    {
        if (r < 0 || c < 0 || r >= qint64(kMaxRows) || c >= qint64(kMaxColumns)) return false;
        if (stretchHolds(m_tall, quint32(c), quint32(r)) || stretchHolds(m_wide, quint32(r), quint32(c))) return true;
        for (const FormulaRef *ref : m_blocks) {
            ++*m_work;
            if (qint64(ref->row1) <= r && r <= qint64(ref->row2) && qint64(ref->col1) <= c && c <= qint64(ref->col2)) return true;
        }
        return false;
    }

private:
    using Stretches = std::map<quint32, std::vector<std::pair<quint32, quint32>>>;
    static bool stretchHolds(const Stretches &group, quint32 line, quint32 at)
    {
        const auto found = group.find(line);
        if (found == group.end()) return false;
        const auto &v = found->second;
        const auto it = std::upper_bound(v.begin(), v.end(), at, [](quint32 x, const std::pair<quint32, quint32> &st) { return x < st.first; });
        return it != v.begin() && std::prev(it)->second >= at;
    }
    Stretches m_tall, m_wide;
    std::vector<const FormulaRef *> m_blocks;
    qint64 *m_work;
};

// The two rules that read what a formula refers to. Adds what it finds (at most one of each rule) to `found`. `work` counts the references looked at.
void referenceProblems(const EditSession &session, const Calculation *calc, int sheet, quint32 row, quint32 col, quint32 rules, qint64 *work, std::vector<FormulaProblem> *found)
{
    const Calculation::CellContent content = calc->content(sheet, row, col, false);
    if (!content.isFormula) return;
    const QString text = content.formula.startsWith(QLatin1Char('=')) ? content.formula.mid(1) : content.formula;
    const std::vector<FormulaRef> refs = findReferences(text);
    ++*work;
    // (a reference with the sheet's name in front is of another sheet or of this one by name: only those without are looked at)
    FormulaProblem base;
    base.sheet = sheet;
    base.row = row;
    base.col = col;
    if (rules & errorRuleBit(FormulaProblem::Kind::OmittedCells)) {
        const Covering covering(refs, work);
        const std::vector<QString> functions = functionsOf(text, refs);
        for (size_t k = 0; k < refs.size(); ++k) {
            const FormulaRef &ref = refs[k];
            if (!ref.sheet.isEmpty() || ref.kind != FormulaRef::Kind::Range || !summarizesRange(functions[k])) continue;
            const bool tall = ref.col1 == ref.col2 && ref.row2 > ref.row1, wide = ref.row1 == ref.row2 && ref.col2 > ref.col1;
            if (!tall && !wide) continue;
            bool hit = false;
            // (the cell just before the first, and just after the last, of the cells the formula adds up)
            for (const bool last : {false, true}) {
                const qint64 r = tall ? (last ? qint64(ref.row2) + 1 : qint64(ref.row1) - 1) : qint64(ref.row1);
                const qint64 c = wide ? (last ? qint64(ref.col2) + 1 : qint64(ref.col1) - 1) : qint64(ref.col1);
                if ((r == qint64(row) && c == qint64(col)) || covering.covers(r, c) || !typedNumberAt(session, calc, sheet, r, c)) continue;
                const QString body = text.mid(ref.bodyStart, ref.start + ref.length - ref.bodyStart);
                const QString moved = tall ? movedEnd(body, false, last ? ref.row2 : ref.row1, quint32(r)) : movedEnd(body, true, last ? ref.col2 : ref.col1, quint32(c));
                FormulaProblem p = base;
                p.kind = FormulaProblem::Kind::OmittedCells;
                p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This formula leaves out a number that is next to the cells it uses. Include that cell, or ignore this if it is meant.");
                if (!moved.isEmpty()) {
                    p.fixLabel = QCoreApplication::translate("jp::crunch::ErrorCheck", "Include the Cell");
                    p.fixedFormula = QLatin1Char('=') + text.left(ref.bodyStart) + moved + text.mid(ref.start + ref.length);
                }
                found->push_back(p);
                hit = true;
                break;
            }
            if (hit) break;
        }
    }
    if (rules & errorRuleBit(FormulaProblem::Kind::EmptyReference)) {
        for (const FormulaRef &ref : refs) {
            ++*work;
            if (!ref.sheet.isEmpty() || ref.kind != FormulaRef::Kind::Cell || (ref.row1 == row && ref.col1 == col)) continue;
            if (!session.valueAt(sheet, ref.row1, ref.col1).isEmpty()) continue;
            FormulaProblem p = base;
            p.kind = FormulaProblem::Kind::EmptyReference;
            p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This formula uses a cell that is empty, so it counts as zero or as no text. Fill the cell, or ignore this if it is meant.");
            found->push_back(p);
            break;
        }
    }
}

} // namespace

const QString &InconsistentScan::keyAt(qint64 row, qint64 col)
{
    static const QString none;
    if (row < 0 || col < 0 || row >= qint64(kMaxRows) || col >= qint64(kMaxColumns)) return none;
    const quint64 id = (quint64(row) << 20) | quint64(col);
    const auto found = m_keys.find(id);
    if (found != m_keys.end()) return found->second;
    const std::optional<Cell> cell = m_calc->sheet(m_sheet).cells.find(quint32(row), quint32(col));
    QString key;
    if (cell && cell->hasFormula() && !(cell->flags & ArrayMember)) {
        const Calculation::CellContent content = m_calc->content(m_sheet, quint32(row), quint32(col), false);
        if (content.isFormula) key = relativeKey(content.formula, quint32(row), quint32(col));
    }
    return m_keys.emplace(id, key).first->second;
}

bool InconsistentScan::formulaAt(qint64 row, qint64 col) const
{
    if (row < 0 || col < 0 || row >= qint64(kMaxRows) || col >= qint64(kMaxColumns)) return false;
    const std::optional<Cell> cell = m_calc->sheet(m_sheet).cells.find(quint32(row), quint32(col));
    return cell && cell->hasFormula() && !(cell->flags & ArrayMember);
}

InconsistentScan::InconsistentScan(const EditSession &session, int sheet) : m_sheet(sheet)
{
    m_calc = session.calculation();
    if (!m_calc || sheet < 0 || sheet >= m_calc->sheetCount() || m_calc->sheet(sheet).cells.cellCount() > kMaxPatternCells) return;
    // (the places of the formulas, in order of row and then column: the slices go through them)
    m_calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (c.hasFormula() && !(c.flags & ArrayMember)) m_places.push_back((quint64(row) << 20) | quint64(col));
    });
    std::sort(m_places.begin(), m_places.end());
}

bool InconsistentScan::step(int cells)
{
    for (; m_next < m_places.size() && cells > 0; ++m_next, --cells) {
        const qint64 row = qint64(m_places[m_next] >> 20), col = qint64(m_places[m_next] & 0xFFFFF);
        const bool sides = formulaAt(row, col - 1) && formulaAt(row, col + 1), updown = formulaAt(row - 1, col) && formulaAt(row + 1, col);
        if (!sides && !updown) continue;
        const QString own = keyAt(row, col);
        if (own.isEmpty()) continue;
        bool odd = false;
        if (sides) {
            const QString &left = keyAt(row, col - 1), &right = keyAt(row, col + 1);
            odd = !left.isEmpty() && left == right && left != own;
        }
        if (!odd && updown) {
            const QString &above = keyAt(row - 1, col), &below = keyAt(row + 1, col);
            odd = !above.isEmpty() && above == below && above != own;
        }
        if (!odd || m_found.size() >= kMaxFormulaProblems) continue;
        FormulaProblem p;
        p.kind = FormulaProblem::Kind::InconsistentFormula;
        p.sheet = m_sheet;
        p.row = quint32(row);
        p.col = quint32(col);
        p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This formula is not like the formulas next to it. The ones on both sides of it are alike, and this one is not.");
        m_found.push_back(p);
    }
    return m_next >= m_places.size();
}

std::vector<FormulaProblem> findFormulaProblems(const EditSession &session, int sheet, bool *truncated, bool patterns, quint32 rules, qint64 *work)
{
    std::vector<FormulaProblem> out;
    if (truncated) *truncated = false;
    const Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return out;
    bool full = false;
    qint64 referenceChecks = 0, referenceWork = 0;
    const bool readsReferences = rules & (errorRuleBit(FormulaProblem::Kind::OmittedCells) | errorRuleBit(FormulaProblem::Kind::EmptyReference));
    calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (full || (c.flags & ArrayMember)) return;
        FormulaProblem p;
        p.sheet = sheet;
        p.row = row;
        p.col = col;
        if (c.hasFormula()) {
            // (a cell can have more than one finding, so the two rules that read references are reached for a cell that is not locked too; a cell that gives an error has that one only)
            std::vector<FormulaProblem> here;
            const Value v = session.valueAt(sheet, row, col);
            if (v.isError()) {
                if (!(rules & errorRuleBit(FormulaProblem::Kind::ErrorValue))) return;
                p.kind = FormulaProblem::Kind::ErrorValue;
                p.explanation = errorSentence(v.errorKind());
                here.push_back(p);
            } else {
                if (!session.styleIsLocked(c.style) && (rules & errorRuleBit(FormulaProblem::Kind::UnlockedFormula))) {
                    p.kind = FormulaProblem::Kind::UnlockedFormula;
                    p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This formula is in a cell that is not locked, so it can be changed even when the sheet is protected. Lock the cell to keep the formula safe.");
                    here.push_back(p);
                }
                // (past the budget, the cells that are left get no reference findings in this pass)
                if (readsReferences && referenceChecks < kMaxReferenceChecks && referenceWork < kMaxReferenceWork) {
                    ++referenceChecks;
                    referenceProblems(session, calc, sheet, row, col, rules, &referenceWork, &here);
                }
            }
            for (const FormulaProblem &one : here) {
                if (out.size() >= kMaxFormulaProblems) {
                    full = true;
                    if (truncated) *truncated = true;
                    return;
                }
                out.push_back(one);
            }
            return;
        } else if (c.type == CellType::SharedString || c.type == CellType::InlineString) {
            const Value v = session.valueAt(sheet, row, col);
            if (!v.isText()) return;
            if (looksLikeNumber(v.text()) && (rules & errorRuleBit(FormulaProblem::Kind::NumberAsText))) {
                p.kind = FormulaProblem::Kind::NumberAsText;
                p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This number is kept as text, so formulas may skip it. Retype it, or change its format, to make it a number.");
            } else if (looksLikeTwoDigitYear(v.text()) && (rules & errorRuleBit(FormulaProblem::Kind::TwoDigitYear))) {
                p.kind = FormulaProblem::Kind::TwoDigitYear;
                p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This date is kept as text and its year has two digits, so it may be read as the wrong century. Retype it with four digits.");
            } else {
                return;
            }
        } else {
            return;
        }
        if (out.size() >= kMaxFormulaProblems) {
            full = true;
            if (truncated) *truncated = true;
            return;
        }
        out.push_back(p);
    });
    if (patterns && (rules & errorRuleBit(FormulaProblem::Kind::InconsistentFormula))) {
        InconsistentScan scan(session, sheet);
        while (!scan.step(1 << 20)) {}
        out = mergeInconsistent(std::move(out), scan.found(), truncated);
    }
    std::stable_sort(out.begin(), out.end(), [](const FormulaProblem &a, const FormulaProblem &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    if (work) *work = referenceWork;
    return out;
}

std::vector<FormulaProblem> mergeInconsistent(std::vector<FormulaProblem> base, const std::vector<FormulaProblem> &found, bool *truncated)
{
    // (a cell that has another problem keeps that one)
    std::set<quint64> have;
    for (const FormulaProblem &p : base) have.insert((quint64(p.row) << 20) | p.col);
    for (const FormulaProblem &p : found) {
        if (have.count((quint64(p.row) << 20) | p.col)) continue;
        if (base.size() >= kMaxFormulaProblems) {
            if (truncated) *truncated = true;
            break;
        }
        base.push_back(p);
    }
    std::sort(base.begin(), base.end(), [](const FormulaProblem &a, const FormulaProblem &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    return base;
}

} // namespace jp::crunch
