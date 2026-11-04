#pragma once
// Dialog boxes of a mail merge that every app with one shares.

#include "core/document.h"

class QWidget;

namespace jp {

// The Mail Merge Recipients window on a list (with `typeNew`, the New Address List window: a list with a first empty row, and the list is offered for saving as a CSV file where it is accepted).
// Returns whether the list was accepted; `m` is then what the person made of it (the rows they checked, in the order of the table).
bool editRecipientList(QWidget *p, MergeSource &m, bool typeNew);

} // namespace jp
