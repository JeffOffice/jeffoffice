// Slicers and timelines: the buttons and the slider that filter a pivot table, as a file keeps them. A slicer cache part says which field of which table it filters and which items are chosen;
// the slicer part says how it looks (its caption, columns, and style). This reads both, and puts a cache's choice on a table's items.
#pragma once

#include "sheet/pivot.h"

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

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

} // namespace jp::office::sheet
