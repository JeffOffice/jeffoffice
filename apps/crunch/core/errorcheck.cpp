#include "core/errorcheck.h"

#include "calc/value.h"
#include "core/formularefs.h"
#include "sheet/calculation.h"

#include <QCoreApplication>
#include <QLocale>
#include <QRegularExpression>

#include <algorithm>
#include <map>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::ErrorKind;
using jp::calc::Value;

namespace {

constexpr qint64 kMaxPatternCells = 200000;

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

} // namespace

std::vector<FormulaProblem> findFormulaProblems(const EditSession &session, int sheet, bool *truncated)
{
    std::vector<FormulaProblem> out;
    if (truncated) *truncated = false;
    const Calculation *calc = session.calculation();
    if (!calc || sheet < 0 || sheet >= calc->sheetCount()) return out;
    bool full = false;
    // the pattern of the formula in a cell (empty for a cell with no formula of its own), kept for the cells looked at
    std::map<quint64, QString> keys;
    auto keyAt = [&](qint64 row, qint64 col) -> const QString & {
        static const QString none;
        if (row < 0 || col < 0 || row >= qint64(kMaxRows) || col >= qint64(kMaxColumns)) return none;
        const quint64 id = (quint64(row) << 20) | quint64(col);
        const auto found = keys.find(id);
        if (found != keys.end()) return found->second;
        const Cell *cell = calc->sheet(sheet).cells.find(quint32(row), quint32(col));
        QString key;
        if (cell && cell->hasFormula() && !(cell->flags & ArrayMember)) {
            const Calculation::CellContent content = calc->content(sheet, quint32(row), quint32(col));
            if (content.isFormula) key = relativeKey(content.formula, quint32(row), quint32(col));
        }
        return keys.emplace(id, key).first->second;
    };
    // (the pattern of every formula is worked out only for sheets of a size that is checked at each change; a bigger sheet is left to the other rules)
    const bool patterns = calc->sheet(sheet).cells.cellCount() <= kMaxPatternCells;
    auto formulaAt = [&](qint64 row, qint64 col) {
        if (row < 0 || col < 0 || row >= qint64(kMaxRows) || col >= qint64(kMaxColumns)) return false;
        const Cell *cell = calc->sheet(sheet).cells.find(quint32(row), quint32(col));
        return cell && cell->hasFormula() && !(cell->flags & ArrayMember);
    };
    auto isInconsistent = [&](quint32 row, quint32 col) {
        if (!patterns) return false;
        const bool sides = formulaAt(row, qint64(col) - 1) && formulaAt(row, qint64(col) + 1), updown = formulaAt(qint64(row) - 1, col) && formulaAt(qint64(row) + 1, col);
        if (!sides && !updown) return false;
        const QString own = keyAt(row, col);
        if (own.isEmpty()) return false;
        if (sides) {
            const QString &left = keyAt(qint64(row), qint64(col) - 1), &right = keyAt(qint64(row), qint64(col) + 1);
            if (!left.isEmpty() && left == right && left != own) return true;
        }
        if (!updown) return false;
        const QString &above = keyAt(qint64(row) - 1, qint64(col)), &below = keyAt(qint64(row) + 1, qint64(col));
        return !above.isEmpty() && above == below && above != own;
    };
    calc->sheet(sheet).cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (full || (c.flags & ArrayMember)) return;
        FormulaProblem p;
        p.sheet = sheet;
        p.row = row;
        p.col = col;
        if (c.hasFormula()) {
            const Value v = session.valueAt(sheet, row, col);
            if (v.isError()) {
                p.kind = FormulaProblem::Kind::ErrorValue;
                p.explanation = errorSentence(v.errorKind());
            } else if (isInconsistent(row, col)) {
                p.kind = FormulaProblem::Kind::InconsistentFormula;
                p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This formula is not like the formulas next to it. The ones on both sides of it are alike, and this one is not.");
            } else {
                return;
            }
        } else if (c.type == CellType::SharedString || c.type == CellType::InlineString) {
            const Value v = session.valueAt(sheet, row, col);
            if (!v.isText()) return;
            if (looksLikeNumber(v.text())) {
                p.kind = FormulaProblem::Kind::NumberAsText;
                p.explanation = QCoreApplication::translate("jp::crunch::ErrorCheck", "This number is kept as text, so formulas may skip it. Retype it, or change its format, to make it a number.");
            } else if (looksLikeTwoDigitYear(v.text())) {
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
    std::sort(out.begin(), out.end(), [](const FormulaProblem &a, const FormulaProblem &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    return out;
}

} // namespace jp::crunch
