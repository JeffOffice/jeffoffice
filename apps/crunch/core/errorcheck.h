#pragma once
// Error Checking: the cells of a sheet that may be wrong. A formula that comes to an error, a number that is kept as text, a formula that differs from the ones on both sides of it (which agree with each other), a date kept as text with a year of two digits, and a formula in a cell that is not locked. Each has a sentence that says what is the matter. What a person
// chose to ignore is the window's to keep.

#include "core/editsession.h"

#include <QString>
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

// The problems of one sheet in order of row and then column; at most kMaxFormulaProblems (`truncated` says more were left out).
std::vector<FormulaProblem> findFormulaProblems(const EditSession &session, int sheet, bool *truncated = nullptr);

} // namespace jp::crunch
