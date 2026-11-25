#pragma once
// Error Checking: the cells of a sheet that may be wrong. A formula that comes to an error, a number that is kept as text, a formula that differs from the ones on both sides of it (which agree with each other), a date kept as text with a year of two digits, a formula in a cell that is not locked, a formula that leaves out a number just beside the cells it adds up, and a formula that reads an empty cell. Each has a sentence that says what is the matter. What a person
// chose to ignore is the window's to keep.

#include "core/editsession.h"

#include <QString>
#include <map>
#include <vector>

namespace jp::crunch {

struct FormulaProblem {
    enum class Kind : quint8 { ErrorValue, NumberAsText, InconsistentFormula, TwoDigitYear, UnlockedFormula, OmittedCells, EmptyReference } kind = Kind::ErrorValue;
    int sheet = 0;
    quint32 row = 0, col = 0;
    QString explanation;
    QString fixLabel, fixedFormula;   // for a problem a click can mend: what the button says, and the formula to put in the cell
    bool operator==(const FormulaProblem &o) const { return kind == o.kind && sheet == o.sheet && row == o.row && col == o.col; }
};

constexpr size_t kMaxFormulaProblems = 5000;

// The rules a person can turn off (Options > Formulas), one bit each by the problem's kind; the rule of empty cells is off until it is asked for.
constexpr quint32 errorRuleBit(FormulaProblem::Kind kind) { return quint32(1) << unsigned(kind); }
constexpr quint32 kAllErrorRules = (quint32(1) << 7) - 1;
constexpr quint32 kDefaultErrorRules = kAllErrorRules & ~errorRuleBit(FormulaProblem::Kind::EmptyReference);

// The most formulas of a sheet looked at, for the two rules that read what a formula refers to.
constexpr qint64 kMaxReferenceChecks = 200000;
// And the most references (and ranges looked through for one) read in one pass over a sheet: past it, the cells still to come get no finding from those two rules in that pass, and the next
// pass (after a change) starts again from the top, so a sheet of very long formulas goes quiet for them instead of freezing the window.
constexpr qint64 kMaxReferenceWork = 300000;

// The most cells a sheet holds for the rule of inconsistent formulas to be run on it.
constexpr qint64 kMaxPatternCells = 2000000;

// The problems of one sheet in order of row and then column; at most kMaxFormulaProblems (`truncated` says more were left out). `patterns` includes the rule that looks at neighboring formulas, which
// takes longer: a window that asks after every change leaves it out and runs an InconsistentScan in slices.
std::vector<FormulaProblem> findFormulaProblems(const EditSession &session, int sheet, bool *truncated = nullptr, bool patterns = true, quint32 rules = kDefaultErrorRules, qint64 *referenceWork = nullptr);

// The rule of inconsistent formulas, a few cells at a time: `step` looks at the next `cells` formulas and says whether it is through them all. `found` is what it has found so far.
class InconsistentScan {
public:
    InconsistentScan(const EditSession &session, int sheet);
    bool step(int cells);
    const std::vector<FormulaProblem> &found() const { return m_found; }
    int sheet() const { return m_sheet; }

private:
    const QString &keyAt(qint64 row, qint64 col);
    bool formulaAt(qint64 row, qint64 col) const;
    const jp::office::sheet::Calculation *m_calc = nullptr;
    int m_sheet = 0;
    std::vector<quint64> m_places;
    size_t m_next = 0;
    std::map<quint64, QString> m_keys;
    std::vector<FormulaProblem> m_found;
};

// The base problems with the inconsistent ones added (a cell with another problem keeps that one).
std::vector<FormulaProblem> mergeInconsistent(std::vector<FormulaProblem> base, const std::vector<FormulaProblem> &found, bool *truncated = nullptr);

} // namespace jp::crunch
