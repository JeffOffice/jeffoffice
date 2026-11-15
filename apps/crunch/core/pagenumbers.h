#pragma once
// JeffCrunch's side of the Page Number gallery (the designs are the shared gallery's, libs/shell/app/pagenumbergallery.h). A printed header or footer is text only, in a left, a center, and
// a right part, with codes for the page's number (&P) and the count (&N), bold (&B), and a size (&22): so only the designs that are text fit, and a design is the text of the part it
// goes in. Nothing here touches the sheet: the window sets the text it makes.

#include "app/pagenumbergallery.h"

#include <QString>

namespace jp::crunch {

// Whether a design fits a printed header or footer: one of the top, the bottom, or the caret's place whose look is text alone (plain, X of Y, bold).
bool offersPageNumberDesign(const PageNumberDesign &d);

// What a design says, as the codes of a header or footer part: "&P", "Page &P of &N", "&B&22&P".
QString pageNumberCode(const PageNumberDesign &d);

// The text of a header (footer) with the design put in the part it names (left, center, or right). The parts that held only page numbers (their codes and the words round them) are
// emptied, so that the number is not there twice; the parts with other text keep it.
QString withPageNumberDesign(const QString &text, const PageNumberDesign &d);

// The text with the page number and count taken out: the codes &P and &N, and the words "Page" and "of" that are left alone in their part. Other text stays. Empty parts are left out.
QString withoutPageNumbers(const QString &text);

// Whether the text has a page number or count in it.
bool hasPageNumbers(const QString &text);

} // namespace jp::crunch
