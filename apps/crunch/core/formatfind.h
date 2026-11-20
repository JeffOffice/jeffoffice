#pragma once
// A format to look for: the fields of a FormatChange that are set must be what the cell's format has (the fields that are not set are not looked at). It is made from a cell (every field
// set), or from what the Format Cells dialog changed.

#include "sheet/styles.h"

#include <QString>

#include <unordered_map>

namespace jp::crunch {

// Every field of the cell format at `xf`, as a change (what a cell with that format would be given to look the same).
jp::office::sheet::FormatChange fullFormatChange(const jp::office::sheet::Styles &styles, quint32 xf);
// The same for a format that is not among the cell formats (a named style's).
jp::office::sheet::FormatChange fullFormatChange(const jp::office::sheet::Styles &styles, const jp::office::sheet::CellFormat &format);
bool formatMatches(const jp::office::sheet::Styles &styles, const jp::office::sheet::CellFormat &format, const jp::office::sheet::FormatChange &want);
// Whether the cell format at `xf` has every field the change sets.
bool formatMatches(const jp::office::sheet::Styles &styles, quint32 xf, const jp::office::sheet::FormatChange &want);
// The fields of `change` that a cell with format `cell` takes when the look of the style it follows goes from `styleBefore` to what `change` says: those in which the cell has what the style
// had. A field the cell has another value in is the cell's own (an override), and stays.
jp::office::sheet::FormatChange inheritedFields(const jp::office::sheet::Styles &styles, const jp::office::sheet::CellFormat &cell, const jp::office::sheet::CellFormat &styleBefore,
                                                const jp::office::sheet::FormatChange &change);
// The fields in which two looks differ, with the values of `after`.
jp::office::sheet::FormatChange changedFields(const jp::office::sheet::Styles &styles, const jp::office::sheet::CellFormat &before, const jp::office::sheet::CellFormat &after);
jp::office::sheet::FormatChange changedFields(const jp::office::sheet::Styles &styles, const jp::office::sheet::FormatChange &before, const jp::office::sheet::FormatChange &after);
// Whether cell formats match a format to look for, each distinct format worked out once (a sheet of a million cells has a few hundred).
class FormatMatcher {
public:
    FormatMatcher(const jp::office::sheet::Styles &styles, const jp::office::sheet::FormatChange &want);
    bool matches(quint32 xf);
    // How many cell formats were worked out (the others were answered from what was).
    qint64 resolutions() const { return m_resolutions; }

private:
    const jp::office::sheet::Styles &m_styles;
    jp::office::sheet::FormatChange m_want;
    std::unordered_map<quint32, bool> m_seen;
    qint64 m_resolutions = 0;
};
// A look with every color that names the theme or the palette given as the color it shows in `source`, and the font's scheme left out (the font is its name): what another workbook takes
// when its theme is not the same.
jp::office::sheet::FormatChange withColorsResolved(const jp::office::sheet::Styles &source, jp::office::sheet::FormatChange look);
// The workbook's theme as a line of text (its twelve colors and its two fonts): two workbooks with the same text show a theme color alike.
QString themeKeyOf(const jp::office::sheet::Styles &styles);
// Whether the change sets nothing.
bool formatIsEmpty(const jp::office::sheet::FormatChange &change);
// The fields the change sets, as a line of words for the dialog ("Font bold, Fill, Number format"), at most `limit` characters.
QString describeFormat(const jp::office::sheet::FormatChange &change);

} // namespace jp::crunch
