// Tests of libs/office/sheet and of the lazy package reader. Every workbook they read is built in memory (never a real-world file).
// This is what the test classes (tst_sheet_*.cpp) share: building a workbook, and reading one sheet's XML.
#pragma once

#include "io/zip.h"
#include "office/package.h"
#include "sheet/cellref.h"
#include "calc/dates.h"
#include "sheet/sheetmodel.h"
#include "sheet/sheetreader.h"

#include <QBuffer>
#include <QtEndian>

#include <QtTest>

#include <memory>

#include "../testrunner.h"

using namespace jp::office;
using namespace jp::office::sheet;
using namespace jp::calc;

namespace sheettest {

extern const QString kMainNs;
extern const QString kRelNs;

// A worksheet part: the rows (as XML) in its sheetData, and whatever follows it. `head` goes before sheetData (a dimension).
QByteArray worksheet(const QString &rows, const QString &after = QString(), const QString &head = QString());
// A shared strings part: each item is the inside of one <si>.
QByteArray sharedStringsXml(const QStringList &items);
// A workbook as an .xlsx: one sheet part per entry of `sheets` (named Sheet1, Sheet2, ...), the shared strings if given,
// and `workbookPr` and `calcPr` as the attributes of those elements.
QByteArray buildXlsx(const QVector<QByteArray> &sheets, const QByteArray &sharedStrings = QByteArray(), const QString &workbookPr = QString(),
                     const QString &calcPr = QString());

// Reads the XML as a sheet part.
bool readSheetXml(const QByteArray &xml, Sheet *out, const ReadOptions &options = ReadOptions());

// The cell at the A1 reference, or null.
const Cell *at(const Sheet &sheet, const QString &ref);
// What the cell holds, in one string: "n:1.5" a number, "s:text" a shared string, "i:text" an inline string, "t:text" the text of a formula, "b:1" or
// "b:0" a boolean, "e:#DIV/0!" an error, "d:45366" a date, "-" an empty cell, and "none" no cell.
QString show(const Sheet &sheet, const SharedStrings &shared, const QString &ref);

} // namespace sheettest

using namespace sheettest;
