#pragma once
// AutoComplete for the text of cells: while a person types in a cell, the rest of an entry above or below it in the same column is offered when only one entry starts with what was typed.
// Only texts are offered (not numbers, dates, formulas, or texts that read as numbers), and only from the block of cells next to the one being typed in (up and down to the first empty cell).

#include <QString>
#include <QStringList>

namespace jp::crunch {

class EditSession;

// The whole text to offer for `typed`, or an empty text for none: the one entry of `column` (any case) that starts with `typed` and is longer, when the entries that start with it are all that
// one (a text that differs only in its capitals counts as the same entry; the first is offered). Nothing is offered for a formula (a text that starts with "=") or for nothing typed.
QString autoCompleted(const QStringList &column, const QString &typed);

// The texts the cells in the column of (sheet, row, col) hold, from the block of cells that touches it (up and down to the first empty cell, at most `kAutoCompleteReach` cells each way); the
// cell itself is left out.
constexpr int kAutoCompleteReach = 5000;
QStringList columnTextsAround(const EditSession &session, int sheet, quint32 row, quint32 col);

} // namespace jp::crunch
