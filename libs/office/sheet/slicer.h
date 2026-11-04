// Slicers and timelines: the buttons and the slider that filter a pivot table, as a file keeps them. A slicer cache part says which field of which table it filters and which items are chosen;
// the slicer part says how it looks (its caption, columns, and style). This reads both, and puts a cache's choice on a table's items.
#pragma once

#include "sheet/pivot.h"

#include <map>

#include <QByteArray>
#include <QSet>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

class Workbook;

constexpr qint64 kMaxSlicerItems = kPivotMaxItemsPerField;   // the items of one slicer (it has at most one for each item of its field)
constexpr int kMaxSlicerTables = 5000;                       // the tables one slicer filters
constexpr int kMaxSlicersPerPart = 10000;                    // the slicers (or timelines) one part lists

struct SlicerItem {
    qint32 x = 0;            // the place of the item among the field's shared items
    bool selected = false;
    bool noData = false;     // no record has it now
};

struct SlicerCache {
    bool timeline = false;
    QString part;                                    // (set by the workbook) the part it was read from
    QString name, sourceName;                        // the cache's name, and the field it filters
    struct Connection {
        int tabId = -1;                              // the sheet the table is on (as the file numbers sheets)
        QString name;                                // the table's name
    };
    QVector<Connection> tables;                      // the tables it filters
    int pivotCacheId = -1;                           // (a slicer) the cache of the data it reads items from
    QVector<SlicerItem> items;                       // (a slicer) its items, in the order it shows them
    QString sortOrder;                               // (a slicer) ascending, descending, or "" for the data's own
    bool hideNoData = false;                         // (a slicer) the items with no data are not shown
    QString startDate, endDate, level;               // (a timeline) the chosen span, as the file writes dates, and the level (years, quarters, months, or days)
};

// A slicer or timeline as it sits on a sheet.
struct SlicerView {
    bool timeline = false;
    QString name, cache, caption, style;
    int columns = 1;                                 // (a slicer) the columns of buttons
    int rowHeight = 0;                               // (a slicer) a button's height, in the file's units (a 12700th of a point)
    bool showCaption = true;
};

// False if the part is not a slicer cache or a timeline cache; `out` is filled as far as it could be read.
bool readSlicerCache(const QByteArray &xml, SlicerCache *out);
// The slicers of a slicers part, or the timelines of a timelines part.
bool readSlicerViews(const QByteArray &xml, QVector<SlicerView> *out);

// Puts a slicer's choice on a table's field: the items it does not choose are hidden, as the checks of the items are (nothing chosen is everything chosen). False if the field is not the slicer's.
bool pivotApplySlicer(PivotTable *table, const PivotCache &cache, int field, const SlicerCache &slicer);

// The items of a table's field that show now, by their place among the cache field's shared items (all of them when the table lists no items of the field): a slicer's choice is the table's.
QSet<qint32> pivotShownItems(const PivotTable &table, const PivotCache &cache, int field);
// Whether two choices are the same to a slicer: none chosen and all chosen are the same (nothing is filtered).
bool slicerSameChoice(const QSet<qint32> &a, const QSet<qint32> &b, qint64 itemCount);
// The part of a slicer cache with its items chosen as `selected`: only the `s` attributes of the items change, the rest of the part is as it was (byte for byte). Empty for a part that is not
// UTF-8 (a byte order mark of UTF-16, or another encoding declared), which is left as it is.
QByteArray patchSlicerSelection(const QByteArray &xml, const QSet<qint32> &selected);

// ---- a workbook's slicers
// How many slicer, timeline, and slicer-list parts have been read, in all (for tests: a part is read once however many relationships name it).
int slicerPartsParsed();
// The tables a slicer cache filters, found by name, and the field of each that the slicer reads (by the cache field's name): pairs of (the table's place, the field's place).
QVector<std::pair<int, int>> slicerTargets(const Workbook &book, const SlicerCache &slicer);
// The slicer parts the tables' items change: for each slicer cache whose choice (the items its first table shows) is not the file's, the part with the choice put in, by the part's name.
std::map<QString, QByteArray> slicerPartPatches(const Workbook &book);

} // namespace jp::office::sheet
