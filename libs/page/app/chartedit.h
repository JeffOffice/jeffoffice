#pragma once
// Charts in the editor: Insert > Chart and the Chart tab's changes, each one step that undoes, and the small dialogs they ask with (the words of a title, the data editor). The
// presentation and publishing programs have these controls from here; the word processor asks with the same dialogs. The chart itself is core/chartobject.h's.

#include "app/editor.h"
#include "core/chartobject.h"

#include <QString>
#include <QWidget>
#include <functional>

namespace jp {

// A chart of the kind with the sample numbers, in the middle of the page or slide being edited (at the size a page's width allows), selected; one step. Returns its id, or nothing.
QString insertChart(Editor *ed, office::sheet::ChartForm form);
// The selected object if it is a chart object, else null.
PictureItem *selectedChart(Editor *ed);
// Changes the selected chart as one step named `label`; false (and no step) when it is no chart that can be changed or `edit` changes nothing.
bool changeSelectedChart(Editor *ed, const QString &label, const std::function<bool(office::Chart &)> &edit);
// What the selected chart is (not valid when none is selected).
office::sheet::ChartInfo selectedChartInfo(Editor *ed);

// Ask for the words of the chart's title (`text` in and out); false when cancelled.
bool askChartTitle(QWidget *parent, QString *text);
// Ask for the words of an axis's title (empty for none); false when cancelled.
bool askChartAxisTitle(QWidget *parent, bool values, QString *text);
// The data editor: a table with the words under the points in its first column, the names of the series in its first row, and numbers in the rest. False when cancelled; when it is
// accepted with a cell that is not a number, `notice` says so and the data is left as it was (false too).
bool askChartData(QWidget *parent, office::sheet::ChartData *data, QString *notice);

} // namespace jp
