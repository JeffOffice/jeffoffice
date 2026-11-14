#pragma once
// The sort a filter's range or a table keeps in the file (its `sortState`): the cells that were sorted, without their header, and what each level asked. It is written when a sort is made inside
// the range, read back when the file is opened, and used again by Reapply and by the Sort dialog.

#include "core/sortspec.h"
#include "sheet/sheetmodel.h"

#include <QString>

namespace jp::office::sheet {
class Workbook;
}

namespace jp::crunch {

// The sort state element for a sort of `data` (the cells without the header). A level the file cannot say (a color that no format of the workbook holds and none can be added, a list
// with a comma in an item, a sort by nothing that has a color) makes the whole state empty: a state that does not say what was done is not kept.
QString sortStateXml(const jp::office::sheet::CellRange &data, const SortSpec &spec, jp::office::sheet::Workbook &book);

// A sort state read: the cells it was made on and its levels (`hasHeader` is not in the file: the caller says it from where the cells begin). False if it is not a state, or names a place
// that is not in the cells.
bool parseSortState(const QString &xml, const jp::office::sheet::Styles &styles, jp::office::sheet::CellRange *data, SortSpec *spec);

} // namespace jp::crunch
