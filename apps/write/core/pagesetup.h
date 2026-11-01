#pragma once
// The Page Setup dialog's work on the document: a section's margins, orientation, paper size, and layout (how the section starts, different first page, different odd and even
// pages), applied to the section the cursor is in or to the whole document, as one step that undoes. Only what the person changed is applied, so a change to the margins of every
// section leaves each section's own paper and orientation as it was.

#include "core/writedoc.h"

namespace jp::write {

// The shortest side of paper, the longest, and the least width and height of text a page keeps (points).
constexpr double kLeastPaper = 36, kMostPaper = 2160, kLeastText = 36;

// What the dialog shows of a section, in points.
struct SectionSetup {
    double paperWidth = 612, paperHeight = 792;   // upright (a landscape page is the same paper turned: the sides are not swapped here)
    bool landscape = false;
    double top = 72, bottom = 72, left = 72, right = 72, gutter = 0;   // the gutter is added to the left margin
    double header = 36, footer = 36;                                   // from the edge of the paper
    QString start = QStringLiteral("nextPage");                        // nextPage, continuous, oddPage, or evenPage
    bool differentFirstPage = false;
    bool differentOddEven = false;                                     // the document's
    bool operator==(const SectionSetup &o) const;
};

enum class PageSetupScope { ThisSection, WholeDocument };

// The page setup a section has now (`oddEven` is the document's different odd and even pages).
SectionSetup pageSetupOf(const office::wml::SectionProps &props, bool oddEven);
// The setup held to what a page can hold: paper from kLeastPaper to kMostPaper, each margin at least 0 and the margins together leaving kLeastText of the page, and the
// header and footer distances within the page. Numbers that are not numbers are the defaults.
SectionSetup normalized(SectionSetup setup);
// Applies what differs between `before` and `after` to the sections the scope names (the section `section` for the one, every section for the other), as one step. Returns how
// many sections it changed (the document's odd and even setting is counted as a change of the last).
int applyPageSetup(WriteDoc &doc, int section, PageSetupScope scope, const SectionSetup &before, const SectionSetup &after);

} // namespace jp::write
