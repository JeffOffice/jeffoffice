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
#include "sheet/drawingadd.h"
#include "sheet/drawingsave.h"
#include "sheet/pagesetupxml.h"
#include "sheet/refshift.h"
#include "sheet/sheetreader.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <atomic>
#include <functional>
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
    bool styleKnown = false;    // `style` and `customFormat` are the row's own format (else the file's stays)
    bool customFormat = false;
    quint32 style = 0;
    bool outlineKnown = false;  // `outline` and `collapsed` are the row's own (else the file's stay)
    quint8 outline = 0;
    bool collapsed = false;
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
    // The default sizes (`sheetFormatPr`), when a person changed them: the default height of rows (and whether it is a person's), and the default width of columns (none when 0).
    bool replaceFormat = false;
    SheetFormat format;
    std::vector<RowProps> rows;         // sorted by row
    // The color of the sheet's tab, when a person changed it (none: the sheet has no color).
    bool replaceTabColor = false;
    ColorSpec tabColor;
    // Whether the sheet's tab is the one that is open (`tabSelected` of its first view), when the sheets a person changed make it so (-1: the file's stays, 0: not, 1: it is).
    int tabSelected = -1;
    // The sheet's filter, when a person set it, changed it, or took it off (the element is made from it, in place of the file's, or put where the format has it; an `autoFilter` that is
    // not valid takes the file's away), and whether the sheet says it is filtered (`filterMode` of its `sheetPr`: 1 yes, 0 no, -1 the file's stays).
    bool replaceAutoFilter = false;
    AutoFilter autoFilter;
    int filterMode = -1;
    // For a sheet a person added: the XML it begins as (its view, default sizes, rules, and no cells: the cells, sizes of rows and columns, and merged ranges above are put in it).
    QByteArray skeleton;
    // For a sheet a person copied: the part of the sheet it was copied from (in the file as it was read or last saved), and the edits to rows and columns made to the cells it was copied from
    // before the copy, as the copy's own (its place is `sheet` of the edit). What the copied sheet's file has that the program keeps no model of (its protection, filter, links, page setup, and
    // breaks) is taken from that part and put in, following the edits.
    QString copyFromPart;
    std::vector<StructEdit> copyEdits;
    // Rows of the file in this range (from 0, both ends in it) that the list above does not name are shown: a filter that was changed hid and showed rows, and the file's rows it showed are
    // told so. The rows of the sheet that are hidden in the range are in `rows`.
    bool showRows = false;
    quint32 showFirst = 0, showLast = 0;
    bool isProtected = false;           // for a sheet a person added: it is protected (a copy of a protected sheet has the protection the sheet had)
    // And the sheet's page setup as it is now, when a person changed how it prints (or any of it may differ from the file's): the part is changed where it differs from this and nowhere else.
    bool setPage = false;
    PageSetup page;
    // For a sheet a person put an object on that had no drawing: the id of the relationship from the sheet to the drawing that is made (saving sets it, from SaveChanges::addedObjects), and the
    // sheet is given the element that names it.
    QString drawingRelId;
    bool empty() const { return drawingRelId.isEmpty() && !showRows && !replaceAutoFilter && filterMode < 0 && cells.empty() && results.empty() && !replaceColumns && !replaceMerges && !replaceFormat && !replaceTabColor && tabSelected < 0 && skeleton.isEmpty() && rows.empty() && copyFromPart.isEmpty() && !setPage; }
};

// A column of a table renamed (by a person, in its header): the formulas outside the cells that name it (defined names, validations, conditional formats, charts) follow.
struct ColumnRename {
    QString table, oldName, newName;
    bool operator==(const ColumnRename &o) const { return table == o.table && oldName == o.oldName && newName == o.newName; }
};

// A sheet renamed or deleted by a person (the formulas of the file that name it follow; see structsave.h): the name it had, and for a rename the name it has.
struct SheetOp {
    enum class Kind : quint8 { Rename, Delete } kind = Kind::Rename;
    QString name, newName;
    bool operator==(const SheetOp &o) const { return kind == o.kind && name == o.name && newName == o.newName; }
};

// A defined name given another name by a person (the formulas of the file that use it follow; see structsave.h): the names it had and has, the place of the sheet it belongs to (-1 for the
// workbook's), and the places of the sheets that had a name of the same text of their own then (a use of the workbook's name on such a sheet is of the sheet's name).
struct NameRename {
    QString oldName, newName;
    int scope = -1;
    std::vector<int> shadowed;
    bool operator==(const NameRename &o) const { return oldName == o.oldName && newName == o.newName && scope == o.scope && shadowed == o.shadowed; }
};

// A defined name a person made, changed, or took away. It is found in the file by the name it had and the place of its sheet (`hadOld`: it is in the file; else it is added at the end of the
// list); `remove` takes it away; else it is as `name`, `text`, `comment`, and `hidden` say, and `place` is where it belongs (-1: the workbook).
struct NameChange {
    bool hadOld = false;
    QString oldName;
    int oldPlace = -1;
    bool remove = false;
    QString name, text, comment;
    int place = -1;
    bool hidden = false;
};

// Cells put in another order in place (a sort): in the cells of the sheet from `first` on that are in the lines (rows, or columns with `columns`) of `lines`, line `i` of the range
// went to line `to[i]` of it. The notes and links on the cells go with them. `after` is how many of the edits to rows, columns, and cells (SaveChanges::edits) had been made when it was.
struct CellPermute {
    int sheet = 0;
    bool columns = false;
    quint32 first = 0;                  // the first line it covers
    quint32 otherFirst = 0, otherLast = 0;   // the cells of the lines that it moves: these columns (rows, with `columns`)
    std::vector<quint32> to;            // the line each of them went to (as a line number, not counted from `first`)
    size_t after = 0;
};

struct SaveChanges {
    std::vector<CellPermute> permutes;
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
        // With `full`, the new cell format is `format`, whole (its number format, font, fill, border, alignment, and protection; the places are those of the file as it will be written),
        // and the copy of `base` has only what the file says besides (a link to a named style, say) kept. Without, it is `base` with the number format `numFmtId`.
        bool full = false;
        CellFormat format;
    };
    QVector<NumberFormat> newNumberFormats;
    QVector<CellFormatAdded> newCellFormats;
    qint64 cellFormatsBefore = 0;
    // And fonts, fills, and borders added (each list begins at the place the `...Before` says), and named styles with their formats among the style formats (cellStyleXfs).
    QVector<FontSpec> newFonts;
    QVector<FillSpec> newFills;
    QVector<BorderSpec> newBorders;
    qint64 fontsBefore = 0, fillsBefore = 0, bordersBefore = 0;
    // And differential formats added (for the colors a filter asks for), at the end of the file's `dxfs`.
    QVector<DifferentialFormat> newDxfs;
    struct StyleAdded {
        CellFormat format;
        CellStyleName name;
    };
    QVector<StyleAdded> newStyles;
    qint64 styleXfsBefore = 0;
    // Parts to be written anew whole, by the part's name (the table parts whose columns were renamed).
    std::map<QString, QByteArray> replacedParts;
    // Rows, columns, and cells inserted and deleted since the file was read, in the order they were made. Every part of the file that has cells in it, or names them, is changed by them
    // (see structsave.h) before the cells above are put in, which are in the places the edits leave.
    std::vector<StructEdit> edits;
    // And the columns of tables renamed since, in order.
    std::vector<ColumnRename> renames;
    // And the defined names given other names since, in order, and the names a person made, changed, or took away (written into the workbook's list of names).
    std::vector<NameRename> nameRenames;
    std::vector<NameChange> nameChanges;
    // How the workbook calculates, when a person changed it (the workbook's calcPr is made to say so): the mode, whether circular references are worked out by going round them, and how.
    bool calcChanged = false;
    CalcMode calcMode = CalcMode::Auto;
    bool iterate = false;
    int iterateCount = 100;
    double iterateDelta = 0.001;
    // Chart parts whose caches (the numbers and texts a chart stores for the cells it reads) are made again, by the part's name: for each reference as the part writes it (its c:f), what
    // the cells hold now. A cache that holds that already is left alone (drawingsave.h).
    std::map<QString, ChartCaches> chartCaches;
    // Drawing objects a person deleted: for each drawing part (by its name), the places of their elements among the children of the part's root (DrawingObject::ordinal). What they leave
    // unused (the relationships of the drawing, and the chart or picture parts nothing else in the package relates to, with their own relationships and types) goes with them.
    std::map<QString, std::vector<int>> removedObjects;
    // Two-cell anchors whose places are written again, by the drawing part and the place of the element (DrawingObject::ordinal): an object that keeps its size when cells are resized (editAs
    // "oneCell" or "absolute") is where its size and place now put it. Applied before the objects are taken away (the places are the part's as it was).
    std::map<QString, std::map<int, DrawingAnchor>> anchors;
    // Objects a person named: for each drawing part (by its name), the new name of the object at a place (DrawingObject::ordinal). Applied with the anchors, before objects are taken away.
    std::map<QString, std::map<int, QString>> objectNames;
    // Objects a person put on sheets (a chart, a picture, a shape): by the sheet's place, what the drawing part gets and the parts it relates to (drawingadd.h). A sheet with no drawing is given one.
    std::map<int, AddedDrawing> addedObjects;
    // Drawings whose objects are drawn in another order: for each drawing part (by its name), the order of all the children of its root as they are after what is above is done (the objects taken
    // away gone, those added at the end): the child at place k is the one this says (drawingsave.h, reorderDrawingChildren).
    std::map<QString, std::vector<int>> drawingOrder;
    // Parts to be written anew whole, made by the thread that writes the file (replacedParts' kind, for what takes time to make: a chart part from a model).
    std::map<QString, std::function<QByteArray()>> producedParts;
    // Ask the program that opens the file to work every formula out again (a result that could not be worked out here is stale).
    bool fullCalcOnLoad = false;
    // The sheets: when a person added, deleted, renamed, moved, hid, or showed one (the workbook's own list is then made from the Workbook's: names, states, and the order of the tabs, with the
    // parts and relationships of the new sheets it has planned), and the renames and deletes in the order made (everything in the file that names a sheet follows them).
    bool sheetsChanged = false;
    std::vector<SheetOp> sheetOps;
    int activeTab = 0;                           // the sheet that is open, by its place in the order of the tabs
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
// The workbook part with its list of sheets made from the workbook's own (names, shown or hidden, the order of the tabs, those added and deleted), the names that belong to a sheet
// following it, and the open tab where it is now.
bool patchWorkbookSheets(const QByteArray &xml, const Workbook &book, QByteArray *out, QString *error = nullptr);
// [Content_Types].xml without the Override of a part, and a relationships part without the link of a type (whose Type ends with the given words); false (nothing changed) if there is none.
bool removeContentTypeOverride(const QByteArray &xml, const QString &partName, QByteArray *out);
bool removeRelationshipOfType(const QByteArray &xml, const QString &typeSuffix, QByteArray *out);
// A relationships part without the links with these ids; false (nothing changed) if there is none of them.
bool removeRelationshipsById(const QByteArray &xml, const QStringList &ids, QByteArray *out);
// The workbook part with fullCalcOnLoad="1" on its calcPr (made if there is none).
bool setFullCalcOnLoad(const QByteArray &xml, QByteArray *out);
// The workbook part with the defined names changed as the list says (names not in it are left byte for byte as they are). `order` is the places of the sheets in the order of the tabs, which the
// file's `localSheetId` counts by (the part is as patchWorkbookSheets left it). False (nothing changed) if the part is not in a form this changes.
bool patchDefinedNames(const QByteArray &xml, const std::vector<NameChange> &changes, const QVector<int> &order, QByteArray *out);
// The workbook part with its calcPr saying how the workbook calculates (made if there is none).
bool patchCalcPr(const QByteArray &xml, CalcMode mode, bool iterate, int iterateCount, double iterateDelta, QByteArray *out);
// The hidden name that other programs keep for the range of a sheet's filter (`_xlnm._FilterDatabase`, of the sheet at `localSheetId` in the workbook's list of sheets) made, changed, or taken
// away: `range` is the range as a name writes it ("'Sheet1'!$A$1:$D$9"), or empty to take the name away.
struct FilterName {
    int localSheetId = 0;
    QString range;
};
bool patchFilterNames(const QByteArray &xml, const std::vector<FilterName> &names, QByteArray *out, QString *error = nullptr);

} // namespace jp::office::sheet
