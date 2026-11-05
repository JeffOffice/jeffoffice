#pragma once
// Data Validation editing: the rules a sheet has, changed so that one rule covers some cells (and no other rule does), or none does. The rules are kept as the file writes them: the limits,
// choices, or formula are the text of the file's formulas (no leading =), as seen from the first cell of the first range of the rule.

#include "sheet/sheetmodel.h"

#include <QVector>
#include <optional>

namespace jp::crunch {

// The cells of `from` that are not in `taken`, as up to four blocks (none when `taken` covers all of it).
QVector<jp::office::sheet::CellRange> subtractRange(const jp::office::sheet::CellRange &from, const jp::office::sheet::CellRange &taken);

// `rules` with the cells of `where` taken out of every rule (a rule left with no cells is dropped), and `rule` (if any) added for them, last. A rule of type Any with no messages is no rule:
// it takes the cells' rules away and adds none.
QVector<jp::office::sheet::Validation> withRuleOn(const QVector<jp::office::sheet::Validation> &rules, const jp::office::sheet::CellRange &where, const std::optional<jp::office::sheet::Validation> &rule);

// The limit a person typed for a rule of this type, as the file writes it: for a list, "a,b,c" typed is written in quotes and "=Sheet2!A1:A5" is the reference; a number stays; for a date or
// time, a date or time typed (as the computer writes them, or 2026-11-04 and 13:30) is its serial number; "=..." is a formula. Empty with `ok` false when it can't be read.
QString validationLimitText(jp::office::sheet::Validation::Type type, const QString &typed, bool *ok);
// The reverse, for showing a rule's limit in the dialog.
QString validationLimitShown(jp::office::sheet::Validation::Type type, const QString &stored);

} // namespace jp::crunch
