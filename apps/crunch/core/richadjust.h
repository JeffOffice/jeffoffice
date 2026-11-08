#pragma once
// The formatting runs of a cell's text when the text is edited: what is changed is found by the text before and after (the same beginning and the same end are kept), the runs after the
// change move with their text, and a run that has nothing left in it goes.

#include "sheet/sheetmodel.h"

namespace jp::crunch {

void adjustRuns(jp::office::sheet::RichText *rich, const QString &now);

} // namespace jp::crunch
