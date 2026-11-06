#pragma once
// Review > Check Accessibility: what in a workbook is hard for some readers (or for a screen reader) to use. A list, each with where it is, so that the window can go there.

#include "core/editsession.h"

#include <QString>
#include <vector>

namespace jp::crunch {

class SheetObjects;

struct AccessIssue {
    enum Kind { MissingAltText, MergedCells, DefaultSheetName, TableWithoutHeader, LowContrast, BlankLayoutCells, ReadingOrder };
    enum Level { Error, Warning, Tip };
    Kind kind = MissingAltText;
    Level level = Warning;
    int sheet = 0;
    quint32 row = 0, col = 0;   // where to go
    QString message;
};

constexpr int kMaxAccessIssues = 500;

// Whether a screen reader would meet the objects of a sheet (listed in the order they are read, which is the order they are stacked) in another order than a person reads the
// sheet, top to bottom and then left to right. Returns the place in the list of the first one that comes too early or too late (-1 if the order is fine).
int firstObjectOutOfReadingOrder(const std::vector<std::pair<quint32, quint32>> &rowsAndColumns);

// The problems found, sheet by sheet (at most `most`). `objects` (may be null) gives the charts and pictures. A sheet that is deleted is not looked at.
std::vector<AccessIssue> checkAccessibility(const EditSession &session, SheetObjects *objects, int most = kMaxAccessIssues);

} // namespace jp::crunch
