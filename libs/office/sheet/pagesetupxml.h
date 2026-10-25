#pragma once
// The page setup in a sheet's XML: reading it back from bytes, making the elements for a setup, and patching a sheet's part (and the workbook's names) so that only what differs from
// the setup is changed: an attribute whose value is what the model says is left as the file wrote it, an element that says nothing different is not touched, a missing element is put
// where the format puts it, and every byte of everything else is passed on as it was. The same code writes the elements of a workbook that is written from the model.

#include "sheet/pagesetup.h"

#include <QByteArray>
#include <vector>

namespace jp::office::sheet {

// The setup that the elements printOptions, pageMargins, pageSetup, headerFooter, rowBreaks, and colBreaks in the bytes (what follows a sheet's cells, or the whole part) say; the setup of
// no elements for those that are not there.
PageSetup pageSetupOfXml(const QByteArray &xml);
// Whether the bytes (what comes before a sheet's cells, or the whole part) ask for fitting to pages (sheetPr/pageSetUpPr).
bool fitToPageOfXml(const QByteArray &xml);

// The elements the setup needs, in the order the format has them, each only if it says something other than the format's default (the margins, which the format wants whole, when they
// are not the default); `prefix` goes before each name ("x:" or nothing).
QByteArray pageSetupElementsXml(const PageSetup &setup, const QByteArray &prefix);
// sheetPr with pageSetUpPr, if the setup fits to pages (for a sheet written whole; `tab` is the rest of sheetPr's children, to come before it).
QByteArray fitToPageXml(const PageSetup &setup, const QByteArray &prefix);

// What follows a sheet's cells (from after "</sheetData>" to the end), changed to say what `now` says of the printing: see above. Gives the bytes as they were if there is nothing to change.
QByteArray patchPageSetupTail(const QByteArray &after, const PageSetup &now, const QByteArray &prefix);
// What comes before a sheet's cells, with the fit to pages of sheetPr as `now` says.
QByteArray patchFitToPageHead(const QByteArray &head, const PageSetup &now);

// The names of a workbook that hold the print area and titles of a sheet.
struct PrintNameChange {
    int sheet = 0;
    QString sheetName;
    PageSetup setup;   // what the sheet's print area and titles are now
};
// The workbook part with the names `_xlnm.Print_Area` and `_xlnm.Print_Titles` of each sheet as the changes say: a name whose references are what the setup says is left as it is, one that
// differs is written again, one the setup no longer has is taken out, and one it has newly is added. False (nothing changed) if the part is not in a form this changes.
bool patchPrintNames(const QByteArray &xml, const std::vector<PrintNameChange> &changes, QByteArray *out);

} // namespace jp::office::sheet
