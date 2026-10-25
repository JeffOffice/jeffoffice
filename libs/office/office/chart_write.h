#pragma once
// Writing a chart: the model of chart.h as a chart part (c:chartSpace), so that a chart a person makes or changes is saved and opens the same here and in other programs. What the model
// holds is written (kinds, series and where they read, titles, axes, legend, labels, colors); what it does not hold is not, so a chart part is only written over by this when
// chartIsFullyModeled says the part has nothing the model would lose. A chart made from the model reads back as the model (the same kinds, series, references, and text).

#include "office/chart.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::office {

// The chart part (c:chartSpace) for the model. The numbers and words each series stores are the model's (the cells' contents when the chart was last given them), the references are
// written as they are (c:f). A series without a reference is written as literal data.
QByteArray writeChartPart(const Chart &chart);

// Whether a chart part holds only what writeChartPart writes, or writes back from what the model kept as the file wrote it (gradients and effects, the labels and looks of single points, legend
// entries, date axis units, the part's links): decided by writing what is read and reading it again. Not a kind of chart that is not drawn, a picture fill, a trend line, error bars, or 3-D.
// If not, `why` names the first thing that is not (in the part's own words, for a message). Chart parts that are too deep or too large for xml.h's limits are not.
bool chartIsFullyModeled(const QByteArray &chartPartXml, QString *why = nullptr);

// Everything chartIsFullyModeled finds that the model would lose (the first only, if `firstOnly`), in the order it looks: for a count of what to carry next.
QStringList chartLosses(const QByteArray &chartPartXml, bool firstOnly = false);

// A chart with nothing in it but what the theme gives every chart (the colors that series take in turn, the text, the background), for a chart to be made from.
Chart blankChart(const Theme &theme);

} // namespace jp::office
