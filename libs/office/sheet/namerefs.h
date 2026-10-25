#pragma once
// Defined names in the text of a formula: where a name is used, and a name given another name. A name is a word that is not a function (no "(" after it), not a table (no "[" after
// it), and not part of a cell's address; it may have the sheet it belongs to in front ("Data!Rate"). Texts in quotes and sheet names in quotes are passed over.

#include <QString>

#include <functional>

namespace jp::office::sheet {

// The formula with each use of the name `oldName` (any case) written `newName`. `applies` is asked for each use with the sheet written in front of it (empty when none is), and says
// whether that use is of the name that is being renamed (a name that belongs to a sheet hides the workbook's of the same name on that sheet, and is reached from the others by writing
// the sheet in front of it). Everything else in the text is as it was; `changed` says whether anything was.
QString renameNameInFormula(const QString &formula, const QString &oldName, const QString &newName, const std::function<bool(const QString &sheet)> &applies, bool *changed = nullptr);

// Whether the formula uses the name at all (any case, with or without a sheet in front).
bool formulaUsesName(const QString &formula, const QString &name);

// Why a name can't be used, in a sentence for the person; empty when it can. A name is made of letters, digits, underscores, and dots (and a backslash), begins with a letter, an
// underscore, or a backslash, is no longer than 255 characters, and is not an address of a cell ("A1", "R2C3", "XFD1048576", "R", "C").
QString nameProblem(const QString &name);

} // namespace jp::office::sheet
