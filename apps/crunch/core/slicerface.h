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

struct SlicerFace {
    QString caption, style;
    int columns = 1;
    qint64 rowHeight = 0;    // a button's height, in the file's units (a 12700th of a point); 0: the file does not say
    bool showCaption = true;
    bool filtered = false;   // the table shows only some of the items
    qint64 itemCount = 0;    // the buttons the slicer has in all
    std::vector<SlicerButton> buttons;
};

} // namespace jp::crunch
