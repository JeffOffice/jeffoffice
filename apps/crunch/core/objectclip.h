#pragma once
// An object over the cells (a chart) on the clipboard, for Copy, Cut, and Paste: the chart as a chart part, its size, the sheet it was on, and which workbook it came from. Pasted in the workbook
// it came from, the chart reads the cells it read (on the sheet it read them, wherever it is put). Pasted in another, it keeps the numbers it showed and reads no cells: a reference to a
// workbook that is not the one open would read the wrong cells.
//
// Bounded: what is on a clipboard is a stranger's; a chart part is read as any is (office/chart.h's limits), and no more than kMaxObjectClipBytes is looked at.

#include "office/chart.h"
#include "office/theme.h"

#include <QByteArray>
#include <QString>

namespace jp::crunch {

constexpr char kObjectMime[] = "application/x-jeffcrunch-object";
constexpr qint64 kMaxObjectClipBytes = qint64(4) << 20;   // (the largest chart part of the practice workbooks is 1.8 MB)

struct ObjectClip {
    QString kind;             // "chart"
    QString origin;           // which workbook it was copied in (the session's token)
    QString sheetName;        // the sheet it was on
    double width = 0, height = 0;   // its size in the grid's units
    QByteArray chartXml;      // the chart part

    bool valid() const { return kind == QLatin1String("chart") && !chartXml.isEmpty(); }
    QByteArray toBytes() const;
    static bool fromBytes(const QByteArray &bytes, ObjectClip *out);
    // The chart a paste makes. `sameBook`: it reads what it read; else it keeps the numbers and words it showed (the references are taken off). False if the part is not a chart.
    bool chart(const jp::office::Theme &theme, bool sameBook, jp::office::Chart *out) const;
};

// The chart with the cells it reads let go: what it shows is what it holds.
void dropReferences(jp::office::Chart *chart);

} // namespace jp::crunch
