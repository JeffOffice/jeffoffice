#pragma once
// A format to look for: the fields of a FormatChange that are set must be what the cell's format has (the fields that are not set are not looked at). It is made from a cell (every field
// set), or from what the Format Cells dialog changed.

#include "sheet/styles.h"

#include <QString>

namespace jp::crunch {

// Every field of the cell format at `xf`, as a change (what a cell with that format would be given to look the same).
jp::office::sheet::FormatChange fullFormatChange(const jp::office::sheet::Styles &styles, quint32 xf);
// The same for a format that is not among the cell formats (a named style's).
jp::office::sheet::FormatChange fullFormatChange(const jp::office::sheet::Styles &styles, const jp::office::sheet::CellFormat &format);
bool formatMatches(const jp::office::sheet::Styles &styles, const jp::office::sheet::CellFormat &format, const jp::office::sheet::FormatChange &want);
// Whether the cell format at `xf` has every field the change sets.
bool formatMatches(const jp::office::sheet::Styles &styles, quint32 xf, const jp::office::sheet::FormatChange &want);
// Whether the change sets nothing.
bool formatIsEmpty(const jp::office::sheet::FormatChange &change);
// The fields the change sets, as a line of words for the dialog ("Font bold, Fill, Number format"), at most `limit` characters.
QString describeFormat(const jp::office::sheet::FormatChange &change);

} // namespace jp::crunch
