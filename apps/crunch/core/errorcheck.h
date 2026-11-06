#pragma once
// Error Checking: the cells of a sheet that may be wrong. A formula that comes to an error, a number that is kept as text, a formula that differs from the ones on both sides of it (which agree with each other), a date kept as text with a year of two digits, and a formula in a cell that is not locked. Each has a sentence that says what is the matter. What a person
// chose to ignore is the window's to keep.

#include "core/editsession.h"

#include <QString>
#include <map>
#include <vector>

namespace jp::crunch {

struct FormulaProblem {
    enum class Kind : quint8 { ErrorValue, NumberAsText, InconsistentFormula, TwoDigitYear, UnlockedFormula } kind = Kind::ErrorValue;
    int sheet = 0;
    quint32 row = 0, col = 0;
    QString explanation;
    bool operator==(const FormulaProblem &o) const { return kind == o.kind && sheet == o.sheet && row == o.row && col == o.col; }
};

constexpr size_t kMaxFormulaProblems = 5000;

// The most cells a sheet holds for the rule of inconsistent formulas to be run on it.
constexpr qint64 kMaxPatternCells = 2000000;

// The problems of one sheet in order of row and then column; at most kMaxFormulaProblems (`truncated` says more were left out). `patterns` includes the rule that looks at neighboring formulas, which
// takes longer: a window that asks after every change leaves it out and runs an InconsistentScan in slices.
std::vector<FormulaProblem> findFormulaProblems(const EditSession &session, int sheet, bool *truncated = nullptr, bool patterns = true);

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
