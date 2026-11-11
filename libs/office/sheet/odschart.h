#pragma once
// The charts of a sheet in an OpenDocument spreadsheet: a frame holding a `draw:object` whose sub-document (`Object N/content.xml`) is a `chart:chart`. They are read into the shared chart model
// (office/chart.h) and written from it, so a chart is what the .xlsx and .xls chart code makes of it: a chart part. See odsdraw.h for the frame, its place, and the limits that go with it.

#include "office/chart.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

#include "sheet/odsdraw.h"
#include "sheet/odsstyles.h"

#include <QHash>

#include <functional>

namespace jp::office::sheet::odschart {

constexpr int kMaxChartsPerSheet = 200;
constexpr int kMaxSeries = 255;
constexpr int kMaxPoints = 100000;               // to a series
constexpr qint64 kMaxPointsInChart = 400000;     // in all the series of a chart
constexpr qint64 kMaxPartBytes = qint64(32) << 20;

// ---- reading
// The chart of an OpenDocument chart sub-document (the bytes of `Object N/content.xml`), as the model. `store` has the styles of the document (odsstyles.h; see parseStyles); `renames` the sheets whose
// names were changed (as the reader of the spreadsheet keeps them). False, with a message, if it is not XML, is past a limit, or holds no chart. `notes` gets what was left out or cut, in words.
bool chartFromOdf(const QByteArray &content, const ods::StyleStore &store, const QHash<QString, QString> *renames, jp::office::Chart *out, QString *error, QStringList *notes);
// The styles a chart document defines, for chartFromOdf (the same reading the spreadsheet's own styles have). False, with a message, if the XML is damaged or past a limit.
bool parseStyles(const QByteArray &content, const Limits &limits, ods::StyleStore *store, QString *error);

// ---- writing
// The chart as the bytes of `Object N/content.xml`, `widthEmu` and `heightEmu` its size. `notes` gets what the chart has that the document cannot say, or that was cut to the limits, in words.
QByteArray chartToOdf(const jp::office::Chart &chart, qint64 widthEmu, qint64 heightEmu, QStringList *notes);

} // namespace jp::office::sheet::odschart
