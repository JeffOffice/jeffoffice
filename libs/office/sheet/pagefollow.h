#pragma once
// A sheet's page setup after rows, columns, or cells of the workbook are inserted or deleted: the breaks a person put in, the print area, and the titles move with the cells they are at,
// exactly as saving moves them in the file (structsave.h): a break is before a row or column and moves with it, one before a row that is deleted goes, a print area grows when rows are
// inserted inside it and shrinks when rows in it are deleted, and a title row or column that is deleted is no longer one. It is the same rewriting of references that defined names get
// (refshift.h), so what is shown here is what a reopened file holds.

#include "sheet/pagesetup.h"
#include "sheet/refshift.h"

#include <QStringList>

namespace jp::office::sheet {

// `ownSheet` is the place of the sheet the setup is for, and `sheetNames` the names of all the workbook's sheets, in order.
void followStructEdit(PageSetup *setup, const StructEdit &edit, int ownSheet, const QStringList &sheetNames);

// Every edit in turn.
void followStructEdits(PageSetup *setup, const std::vector<StructEdit> &edits, int ownSheet, const QStringList &sheetNames);

} // namespace jp::office::sheet
