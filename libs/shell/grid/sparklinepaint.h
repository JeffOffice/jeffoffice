#pragma once
// Draws one sparkline into the room of its cell: a line, columns, or wins and losses, from the numbers it reads. Only the drawing is here; which cell has a sparkline and what it reads
// is the model's (GridSheet::sparklineAt, sparklineValues).

#include "sheet/sparkline.h"

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QVector>

namespace jp::grid {

// The colors of a group, resolved (the file's theme colors and palette numbers made into colors).
struct SparklineColors {
    QColor series, negative, axis, markers, first, last, high, low;
};

// `values` has a NaN for a cell that holds no number. `groupLow` and `groupHigh` are the lowest and highest of the group's numbers (for an axis the group shares). `scale` is the size of the
// window's zoom (1 at 100%): the width of the line and the size of the markers follow it.
void paintSparkline(QPainter *p, const QRect &cell, const jp::office::sheet::SparklineGroup &group, const SparklineColors &colors, const QVector<double> &values, double groupLow, double groupHigh,
                    double scale = 1);

} // namespace jp::grid
