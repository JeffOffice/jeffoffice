#pragma once
// Dialog boxes of a mail merge that every app with one shares.

#include "core/document.h"

class QWidget;

namespace jp {

class Editor;

// The Mail Merge Recipients window on a list (with `typeNew`, the New Address List window: a list with a first empty row, and the list is offered for saving as a CSV file where it is accepted).
// Returns whether the list was accepted; `m` is then what the person made of it (the rows they checked, in the order of the table).
bool editRecipientList(QWidget *p, MergeSource &m, bool typeNew);

// Format Merge Field: how the merge field at the editing caret (or the ones selected) shows its value: as the list holds it, a number or a percent with 0 to 4 decimal places, a currency
// amount, a date, or a time (text/mergeformat.h), with a sample of what the choice makes. Returns whether a format was set (one step of Undo).
bool mergeFieldFormatDialog(QWidget *p, Editor *ed);

} // namespace jp
