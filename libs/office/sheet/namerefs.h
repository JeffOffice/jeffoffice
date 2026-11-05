#pragma once
// Defined names in the text of a formula: where a name is used, and a name given another name. A name is a word that is not a function (no "(" after it), not a table (no "[" after
// it), and not part of a cell's address; it may have the sheet it belongs to in front ("Data!Rate"). Texts in quotes and sheet names in quotes are passed over.

#include <QString>

#include "sheet/cellref.h"

#include <functional>
#include <vector>

namespace jp::office::sheet {

// The formula with each use of the name `oldName` (any case) written `newName`. `applies` is asked for each use with the sheet written in front of it (empty when none is), and says
// whether that use is of the name that is being renamed (a name that belongs to a sheet hides the workbook's of the same name on that sheet, and is reached from the others by writing
// the sheet in front of it). Everything else in the text is as it was; `changed` says whether anything was.
QString renameNameInFormula(const QString &formula, const QString &oldName, const QString &newName, const std::function<bool(const QString &sheet)> &applies, bool *changed = nullptr, bool wideText = false);

// Whether the formula uses the name at all (any case, with or without a sheet in front).
bool formulaUsesName(const QString &formula, const QString &name);

// A name that stands for a block of cells on a sheet, to be put in place of the references to those cells.
struct NameTarget {
    QString name;
    QString sheet;   // the sheet the cells are on (any case)
    CellRange cells;
};
// The formula with each reference that is exactly the cells of one of the names written as the name ("$A$1", "A1", "Data!A1:B2", and "'My Data'!$A$1:$B$2" alike; a reference with no sheet
// in front is of `formulaSheet`). Text in quotes, table columns, parts of longer ranges ("A1:C3" when the name is A1), function names, and everything else are as they were; of two names
// for the same cells the first in the list is used. `count`, if given, is how many references were replaced.
QString applyNamesInFormula(const QString &formula, const QString &formulaSheet, const std::vector<NameTarget> &names, int *count = nullptr);

// Why a name can't be used, in a sentence for the person; empty when it can. A name is made of letters, digits, underscores, and dots (and a backslash), begins with a letter, an
// underscore, or a backslash, is no longer than 255 characters, and is not an address of a cell ("A1", "R2C3", "XFD1048576", "R", "C").
QString nameProblem(const QString &name);

} // namespace jp::office::sheet
