#pragma once
// Saving a workbook as .xlsx or .xlsm without writing it again: the new file is the old file's ZIP with only what changed put in anew. Every part the person did not change
// (the other sheets, the styles when no format was added, the theme, drawings, the VBA project, everything the program does not read) is copied as it is in the old file, compressed
// bytes and all, so it is the same to the last bit. A sheet that has changed is the old sheet's XML with the changed cells put in their places (and nothing else of it touched: its
// conditional formats, validations, drawings, and the rest are as they were); the shared strings and styles parts get what was added at their ends; and the calculation chain,
// which lists the formulas in the order a program last worked them out and which a program refuses to open when it names a cell that no longer holds one, is left out (the other
// program makes it again when it opens the file).
//
// The changes are handed over as plain data (`SaveChanges`), taken from the workbook when the save begins, so that the file can be made on a thread of its own while the window goes
// on: nothing here reads the workbook's cells.

#include "office/package.h"
#include "sheet/drawingsave.h"
#include "sheet/pagesetupxml.h"
#include "sheet/refshift.h"
#include "sheet/sheetreader.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <atomic>
#include <map>
#include <vector>

namespace jp::office::sheet {

// A value as a cell of the new file holds it.
struct SavedValue {
    enum class Kind : quint8 { Empty, Number, Text, Bool, Error } kind = Kind::Empty;
    double number = 0;          // a number; 1 or 0 for a bool; the error's place (CellError) for an error
    QString text;
    qint32 sharedString = -1;   // a text: its place in the shared strings if it has one (the file is given them), else it is written in the cell
};

// A cell the person changed, put in whole: what it holds, its format, and its formula.
struct SavedCell {
    quint32 row = 0;
    quint16 col = 0;
    quint32 style = 0;          // the place of its cell format
    SavedValue value;           // the value, or a formula's result
    bool isFormula = false;
    QString formula;            // as the file writes it, with no "=" in front
    bool hasResult = true;      // a formula's result is known (else the cell has the formula and no value)
    QString arrayRef;           // the first cell of an array formula: the cells it covers ("C3:C5"), else empty
};

// The result of a formula that the person did not change, worked out again after a change to what it reads.
struct SavedResult {
    quint32 row = 0;
    quint16 col = 0;
    SavedValue value;
};

// And the height of a row, or that it is hidden.
struct RowProps {
    quint32 row = 0;
    double height = -1;         // points; -1: the file's stays
    bool hidden = false;
    bool customHeight = false;
};

struct SheetChanges {
    std::vector<SavedCell> cells;       // sorted by row, then column
    std::vector<SavedResult> results;   // sorted the same way
    // The runs of columns the sheet has now (their widths, whether they are hidden, and what else the file says of them), when the file's `cols` is to be written from them: columns
    // inserted or deleted or sized by a person. Sorted, none twice.
    bool replaceColumns = false;
    std::vector<ColumnSpan> columns;
    // And the merged ranges, when a paste or a cut and paste changed them.
    bool replaceMerges = false;
    std::vector<CellRange> merges;
    std::vector<RowProps> rows;         // sorted by row
    // And the sheet's page setup as it is now, when a person changed how it prints (or any of it may differ from the file's): the part is changed where it differs from this and nowhere else.
    bool setPage = false;
    PageSetup page;
    bool empty() const { return cells.empty() && results.empty() && !replaceColumns && !replaceMerges && rows.empty() && !setPage; }
};

// A column of a table renamed (by a person, in its header): the formulas outside the cells that name it (defined names, validations, conditional formats, charts) follow.
struct ColumnRename {
    QString table, oldName, newName;
    bool operator==(const ColumnRename &o) const { return table == o.table && oldName == o.oldName && newName == o.newName; }
};

struct SaveChanges {
    std::map<int, SheetChanges> sheets;           // by the sheet's place in the workbook
    // The print area and titles of the sheets whose page setup may differ from the file's: the workbook's names are changed where they differ from these.
    std::vector<PrintNameChange> printNames;
    // Strings added to the end of the shared strings (the first is at the place `sharedStringsBefore`), and the number of strings the file has now.
    QStringList newSharedStrings;
    qint64 sharedStringsBefore = 0;
    // Number formats and cell formats added to the styles: each cell format is an old one with another number format (the first is at the place `cellFormatsBefore`).
    struct NumberFormat {
        int id = 0;
        QString code;
    };
    struct CellFormatAdded {
        quint32 base = 0;       // the cell format it is a copy of
        int numFmtId = 0;
    };
    QVector<NumberFormat> newNumberFormats;
    QVector<CellFormatAdded> newCellFormats;
    qint64 cellFormatsBefore = 0;
    // Parts to be written anew whole, by the part's name (the table parts whose columns were renamed).
    std::map<QString, QByteArray> replacedParts;
    // Rows, columns, and cells inserted and deleted since the file was read, in the order they were made. Every part of the file that has cells in it, or names them, is changed by them
    // (see structsave.h) before the cells above are put in, which are in the places the edits leave.
    std::vector<StructEdit> edits;
    // And the columns of tables renamed since, in order.
    std::vector<ColumnRename> renames;
    // Chart parts whose caches (the numbers and texts a chart stores for the cells it reads) are made again, by the part's name: for each reference as the part writes it (its c:f), what
    // the cells hold now. A cache that holds that already is left alone (drawingsave.h).
    std::map<QString, ChartCaches> chartCaches;
    // Drawing objects a person deleted: for each drawing part (by its name), the places of their elements among the children of the part's root (DrawingObject::ordinal). What they leave
    // unused (the relationships of the drawing, and the chart or picture parts nothing else in the package relates to, with their own relationships and types) goes with them.
    std::map<QString, std::vector<int>> removedObjects;
    // Two-cell anchors whose places are written again, by the drawing part and the place of the element (DrawingObject::ordinal): an object that keeps its size when cells are resized (editAs
    // "oneCell" or "absolute") is where its size and place now put it. Applied before the objects are taken away (the places are the part's as it was).
    std::map<QString, std::map<int, DrawingAnchor>> anchors;
    // Ask the program that opens the file to work every formula out again (a result that could not be worked out here is stale).
    bool fullCalcOnLoad = false;
};

struct SaveReport {
    int sheetsRewritten = 0;
    int partsCopied = 0;                 // copied as they were
    int partsChanged = 0;                // and written anew (sheets, shared strings, styles, content types, relationships, workbook)
    qint64 cellsChanged = 0, resultsChanged = 0;
    bool calcChainDropped = false;
    QStringList changedParts;            // the names, as the file has them
};

// Makes the new file's bytes from the old (the lazy package `book` was opened from) and the changes. `cancel`, if given, is looked at as the sheets are copied. False, with a message, if
// the file cannot be made (a sheet is not XML this can change, the package is not lazy, the workbook has no part to put a change in).
bool saveXlsx(const Workbook &book, const SaveChanges &changes, QByteArray *out, QString *error = nullptr, SaveReport *report = nullptr, const std::atomic<bool> *cancel = nullptr);

// Writes the bytes to `path` without touching a file that is already there until they are all written: they go to a file of their own in the same folder, which then takes the place
// of the old (renamed over it). False, with a message, if anything failed; the old file is then as it was.
bool writeFileAtomically(const QString &path, const QByteArray &bytes, QString *error = nullptr);

// ---- the parts' changes, one by one (saveXlsx puts them together; they are public for the tests)

// A sheet's XML with the changes put in. `in` is read to its end and the new XML written to `out` (a ZipWriter stream, or any sink); false, with a message, if it cannot be made.
class PartSink {
public:
    virtual ~PartSink() = default;
    virtual void write(const char *data, qsizetype size) = 0;
};
bool patchSheet(QIODevice *in, PartSink *out, const SheetChanges &changes, bool hasSharedStrings, QString *error = nullptr, const std::atomic<bool> *cancel = nullptr);
// The shared strings part with the strings added at its end (and its counts raised).
bool patchSharedStrings(const QByteArray &xml, const QStringList &added, QByteArray *out, QString *error = nullptr);
// The styles part with the number formats and cell formats added.
bool patchStyles(const QByteArray &xml, const SaveChanges &changes, QByteArray *out, QString *error = nullptr);
// [Content_Types].xml without the Override of a part, and a relationships part without the link of a type (whose Type ends with the given words); false (nothing changed) if there is none.
bool removeContentTypeOverride(const QByteArray &xml, const QString &partName, QByteArray *out);
bool removeRelationshipOfType(const QByteArray &xml, const QString &typeSuffix, QByteArray *out);
// A relationships part without the links with these ids; false (nothing changed) if there is none of them.
bool removeRelationshipsById(const QByteArray &xml, const QStringList &ids, QByteArray *out);
// The workbook part with fullCalcOnLoad="1" on its calcPr (made if there is none).
bool setFullCalcOnLoad(const QByteArray &xml, QByteArray *out);

} // namespace jp::office::sheet
