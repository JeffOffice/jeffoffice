#pragma once
// A slicer as the sheet draws it and is clicked on (core/sheetslicers.h): what EditSession::slicerFace says of it, with no part of the file in it.

#include <QString>
#include <QtGlobal>

#include <vector>

namespace jp::crunch {

struct SlicerButton {
    QString text;
    qint32 place = 0;        // the item's place among the field's items (what a click names)
    bool selected = false;   // the table shows it (all of them do when nothing is filtered)
    bool noData = false;     // no record has it
};

// Why a slicer cannot be used here: it filters a table (not a pivot table), or what it filters is not in the file.
enum class SlicerWhy : quint8 { None, Table, Missing };

struct SlicerFace {
    SlicerWhy unusable = SlicerWhy::None;
    QString caption, style;
    int columns = 1;
    qint64 rowHeight = 0;    // a button's height, in the file's units (a 12700th of a point); 0: the file does not say
    bool showCaption = true;
    bool filtered = false;   // the table shows only some of the items
    qint64 itemCount = 0;    // the buttons the slicer has in all
    qint64 chosenCount = 0;  // how many of them the table shows (all of them when nothing is filtered), among all the buttons and not only those made
    std::vector<SlicerButton> buttons;
    // How the picture is made, not what the file says: a slicer in the window scrolls (its scroll bar is drawn when the buttons do not fit), one printed does not (its last row says how many are left out);
    // `first` is the place among the buttons of the first made (a row start); `focus` the button the keyboard is on (-1: none).
    bool scrolls = false;
    bool printed = false;    // drawn for a page (nothing can scroll there)
    qint64 first = 0;
    qint64 focus = -1;
};

} // namespace jp::crunch
