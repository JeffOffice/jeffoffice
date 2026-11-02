#pragma once
// Pivot tables: a pivot cache (the list a table is made from: xl/pivotCache/pivotCacheDefinitionN.xml with its records) and a pivot table
// (xl/pivotTables/pivotTableN.xml: which fields are rows, columns, filters, and values), and the cells the table shows, worked out from the two.
//
// A file stores the cells a table shows in the sheet, and the table's rowItems and colItems (the lines of each axis); working them out here is
// what Refresh does after the cache is read again from the source, and what a change of layout does. A table nobody changed is never written
// again: its parts are kept as the file has them.

#include "calc/dates.h"

#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
#include <memory>
#include <vector>

namespace jp::office::sheet {

// What the engine will take in, bounded. A file that has more is refused (the cells stay as it stored them).
constexpr int kPivotMaxFields = 2000;
constexpr qint64 kPivotMaxItemsPerField = 1000000;
constexpr qint64 kPivotMaxRecords = 5000000;
constexpr qint64 kPivotMaxRecordCells = 60000000;   // records times fields
constexpr qint64 kPivotMaxOutputCells = 4000000;
constexpr qint64 kPivotMaxLeaves = 8000000;
constexpr qint64 kPivotMaxItemsPerTable = 2000000;   // the items of all the fields of one table together
constexpr int kPivotMaxDataFields = 1000;
constexpr qint64 kPivotMaxRows = 1048576;            // lines of the rows' axis, and of the columns' (a sheet's rows and columns)
constexpr qint64 kPivotMaxCols = 16384;
// What a part of a pivot table or cache may be once inflated (a table's, a cache definition's, the records').
constexpr qint64 kPivotMaxTablePartBytes = qint64(128) << 20;
constexpr qint64 kPivotMaxDefinitionBytes = qint64(256) << 20;
constexpr qint64 kPivotMaxRecordsBytes = qint64(2) << 30;
constexpr int kPivotMaxXmlDepth = 64;
// For the tests: other limits for the parts a pivot book takes in (0 for the usual ones).
void pivotSetPartLimitsForTest(qint64 table, qint64 definition, qint64 records);

struct PivotValue {
    enum Kind : quint8 { Missing, Number, Text, Bool, Error, Date };
    Kind kind = Missing;
    double number = 0;   // a number, a day count (Date), a bool (0 or 1)
    QString text;        // text, or the error as the file spells it
    bool isNumeric() const { return kind == Number || kind == Date; }
    bool operator==(const PivotValue &o) const { return kind == o.kind && number == o.number && text == o.text; }
};

struct PivotCacheField {
    QString name;
    int numFmtId = 0;
    QVector<PivotValue> items;   // the distinct values: the shared items first, then those only the records have
    int sharedCount = 0;         // the shared items the file lists
    bool hasShared = false;      // the file lists shared items (`items` begins with them)
    QVector<qint32> records;     // for each record, the place of its value in `items`
    QString sharedAttrs;         // the attributes of <sharedItems> (kept for writing: containsBlank and the like are made again when written)
};

struct PivotCache {
    QString sourceType = QStringLiteral("worksheet");
    QString sheet, ref, name;    // the source: a range on a sheet, or a table or a name
    QVector<PivotCacheField> fields;
    qint64 recordCount = 0;
    bool saveData = true, refreshOnLoad = false;
    QString unsupported;         // why the cache can't be worked from or read again (grouped or calculated fields, ...); empty when it can
    QVector<QVector<qint32>> remap;   // after buildPivotCache: for each field of the previous cache, where each of its shared items went
    QVector<int> fromField;      // after buildPivotCache: for each field, the place of the previous cache's field of the same name (-1: new)
};

struct PivotAxisItem {
    QString t;                   // "" for a line of data; default, grand, sum, countA, avg, max, min, product, count, stdDev, stdDevP, var, varP, blank
    int r = 0;                   // how many of the fields' items are those of the line before
    int i = 0;                   // which data field (when the values are on this axis)
    QVector<int> x;              // the places of the items in each field's list, from field r on
    bool labelOnly = false;      // (not in the file) a line that names an item and shows no values: the line of an item whose group is under it
};

struct PivotTableItem {
    int x = -1;                  // the place in the cache field's shared items (for a data item)
    QString t;                   // "" for an item; default for the subtotal line, or sum, avg, ...
    bool hidden = false, missing = false;
    bool showDetails = true;     // false: collapsed, the items under it are not shown
    QString n;                   // the name given to it
};

struct PivotField {
    QString name;                // the name given to the field here (empty: the cache's)
    QString axis;                // axisRow, axisCol, axisPage, or empty
    bool dataField = false;
    bool defaultSubtotal = true;
    QStringList subtotals;       // the custom subtotals (sum, countA, avg, ...)
    int numFmtId = 0;
    QString sortType = QStringLiteral("manual");
    bool showAll = false;
    bool subtotalTop = true;
    bool compact = true, outline = true;
    bool insertBlankRow = false;
    QVector<PivotTableItem> items;
    bool hasItems = false;
    int sortByData = -1;         // the field is sorted by the values of this data field (autoSortScope)
    int src = -1;                // the place of its element in the part the table was read from (-1: a field made here)
};

struct PivotDataField {
    QString name;
    int fld = 0;
    QString subtotal = QStringLiteral("sum");
    QString showDataAs = QStringLiteral("normal");
    int numFmtId = 0;
};

struct PivotPageField {
    int fld = 0;
    int item = -1;               // the place in the field's items; -1 for all
};

struct PivotTable {
    QString name;
    int cacheId = 0;
    QString ref;                 // location
    int firstHeaderRow = 1, firstDataRow = 1, firstDataCol = 1;
    int rowPageCount = 0, colPageCount = 0;
    bool dataOnRows = false;
    QString dataCaption = QStringLiteral("Values");
    QString grandTotalCaption = QStringLiteral("Grand Total");
    QString rowHeaderCaption, colHeaderCaption;
    QString missingCaption, errorCaption;
    bool showMissing = true, showError = false;
    bool rowGrandTotals = true, colGrandTotals = true;
    bool compact = true, outline = false;
    bool mergeItem = false;
    bool gridDropZones = false;
    QVector<PivotField> fields;
    QVector<int> rowFields, colFields;   // a field's place; -2 for the values
    QVector<PivotPageField> pageFields;
    QVector<PivotDataField> dataFields;
    QVector<PivotAxisItem> rowItems, colItems;   // as the file has them
    QString unsupported;         // why the table can't be worked out
};

// What one cell of the table shows.
struct PivotCell {
    enum Kind : quint8 { Empty, Number, Text, Bool, Error };
    Kind kind = Empty;
    double number = 0;
    QString text;
    int numFmtId = -1;           // the format the table gives it (-1: none)
    bool isValue = false;        // a cell of the data, not a label
    bool operator==(const PivotCell &o) const { return kind == o.kind && number == o.number && text == o.text; }
};

struct PivotGrid {
    int rows = 0, cols = 0;
    QVector<PivotCell> cells;    // rows * cols, by row
    int firstHeaderRow = 1, firstDataRow = 1, firstDataCol = 1;
    QVector<PivotAxisItem> rowItems, colItems;
    QVector<PivotCell> pageLabels;   // the page fields: a field's caption and its value, as two cells each, in order
    const PivotCell &at(int r, int c) const { return cells[qsizetype(r) * cols + c]; }
    PivotCell &at(int r, int c) { return cells[qsizetype(r) * cols + c]; }
};

// ---- reading
// A cache from its definition and its records (empty when the table doesn't keep them: it is then not usable). False, with `why`, when a part is not
// well formed or is past a bound.
bool readPivotCache(const QByteArray &definition, const QByteArray &records, jp::calc::DateSystem dates, PivotCache *out, QString *why);
bool readPivotTable(const QByteArray &xml, PivotTable *out, QString *why);
// Only where a cache's data is (the source type, sheet, range, and name), from the head of its definition: cheap, whatever else the part holds.
bool readPivotCacheSource(const QByteArray &definition, PivotCache *out);
// Only the cells a table covers, read by streaming: for a table that can't be read whole.
bool readPivotTableLocation(const QByteArray &xml, QString *ref);

// ---- working out
// The cells the table shows. The table's `location` is not read, the grid has the size the table needs. False, with `why`, when the table or the
// cache has something this does not work out (a grouped field, a "show values as"), or is past a bound.
struct PivotOptions {
    // How a number shows in text (the "7/1/2001" of "7/1/2001 Total"): by a number format's id; empty for the plain number.
    std::function<QString(double value, int numFmtId, int gridRow, int gridCol)> format;
};
bool computePivot(const PivotTable &table, const PivotCache &cache, PivotGrid *out, QString *why, const PivotOptions &options = PivotOptions());

// ---- reading the list again from the sheet
// What a cell of the source holds, by row and column.
using PivotSourceCell = std::function<PivotValue(quint32 row, quint32 col)>;
struct PivotSource {
    PivotSourceCell cell;
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;   // the range: its first row has the names of the fields
    QVector<int> numFmtIds;                            // the number format of each column's first cell of data (empty: those the cache had)
    QSet<int> axisFields;                              // the fields (by their place in the cache being replaced) a table has on an axis or a page: their items are shared
};
// A new cache from the cells of a range. A field of the same name in `previous` keeps the order of its shared items that remain, new ones are put after them;
// `remap` of the result says where each shared item of the previous cache went (-1: gone). False, with `why`, past a bound or for a source with no fields.
bool buildPivotCache(const PivotSource &source, const PivotCache &previous, PivotCache *out, QString *why);
// Makes the table fit a cache that was read again: items that are gone are taken out, those new are added (at the end, or in order when the field is sorted),
// fields that are new or gone are matched by name, and a filter on an item that is gone is All. The lines of the axes are made again by computePivot.
void matchPivotItems(PivotTable *table, const PivotCache &previous, const PivotCache &fresh);
// The layout rules a person's changes keep: the values (-2) are on an axis when there is more than one, once; and `dataOnRows` says which.
void pivotNormalizeLayout(PivotTable *table);
// Makes sure each field on an axis or a page has shared items in the cache and a list of items in the table (all the field's items, in order, and the subtotal line). True if the cache changed.
bool pivotPrepareFields(PivotTable *table, PivotCache *cache);
// Whether the field holds numbers only (blanks apart): a field that is checked in goes to the values if it does, else to the rows.
bool pivotFieldIsNumeric(const PivotCache &cache, int field);
// "Sum" for sum, "Count Numbers" for countNums, and so on; and the name a value gets, "Sum of Amount", made different from every field's and every value's name by spaces after it.
QString pivotFunctionName(const QString &function);
QString pivotDataFieldName(const PivotTable &table, const PivotCache &cache, int field, const QString &function);
// Sets each field's axis and data marks from the lists of row, column, page, and value fields.
void pivotSyncRoles(PivotTable *table);
// Puts the text of a cell value in the order a sorted field has: numbers first (in number order), then text (without regard to case), then errors, then the blank.
bool pivotValueLess(const PivotValue &a, const PivotValue &b);

// ---- writing (a part is made from the one the file has: what the program doesn't model is kept)
QByteArray writePivotCacheDefinition(const QByteArray &original, const PivotCache &cache, qint64 recordCount, double refreshedDate, jp::calc::DateSystem dates);
QByteArray writePivotCacheRecords(const PivotCache &cache, jp::calc::DateSystem dates);
// The table's definition with its location, fields, lines, and values as the table has them; the original's other parts stay. `structureChanged` drops the formats that name the fields' items.
QByteArray writePivotTable(const QByteArray &original, const PivotTable &table, bool structureChanged);
// Puts the lines and the place of the header of a worked-out grid in the table (what writePivotTable writes), and the grid's place at `row`, `col`.
void pivotApplyGrid(PivotTable *table, const PivotGrid &grid, quint32 row, quint32 col);

// The file's way of writing a character XML can't hold (_xHHHH_), and of writing an underscore that starts the same: the text a file's string is, and the string a text is written as.
QString pivotDecodeXString(const QString &s);
QString pivotEncodeXString(const QString &s);
// How deep the elements of a part nest (at most the limit counted; a part that is not well formed is taken to be as deep as it is).
int pivotXmlDepth(const QByteArray &xml, int limit);
// The data fields of `old` that are in `now` (by name, else by field and function): the places in `now` of the data fields of `old`, -1 for one that is gone; and the sorts by value follow them.
void pivotRemapDataFields(const PivotTable &old, PivotTable *now);
// Checks that the parts of a pivot table, its cache, and the records are as the schema has them: the order of the elements, the counts, the places an index points to, the marks of the shared
// items, and the dates. `records` may be empty. False, with the first thing wrong in `why`.
bool pivotCheckShape(const QByteArray &table, const QByteArray &definition, const QByteArray &records, QString *why);

// A cache definition with its source (the sheet and the range) put in, and nothing else of it changed.
QByteArray patchPivotCacheSource(const QByteArray &definition, const QString &sheet, const QString &ref);

// "A1:C9" for the cells of a grid placed at a row and column.
QString pivotRangeRef(quint32 row1, quint32 col1, quint32 row2, quint32 col2);

} // namespace jp::office::sheet
