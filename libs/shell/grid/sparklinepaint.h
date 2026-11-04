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

// `values` has a NaN for a cell that holds no number. `groupLow` and `groupHigh` are the lowest and highest of the group's numbers (for an axis the group shares). `dates`, when the group has a date
// axis and there is one date for each value, spaces the points by them; else they are evenly spaced. The whole cell is the room (what is drawn is held to the painter's clip, which is
// narrowed to the cell); the line and the markers are sized by the cell's height. The picture keeps clear of the cell's edges: about an eighth of its height above and below, and 3 pixels at the sides.
void paintSparkline(QPainter *p, const QRect &cell, const jp::office::sheet::SparklineGroup &group, const SparklineColors &colors, const QVector<double> &values, double groupLow, double groupHigh,
                    const QVector<double> &dates = QVector<double>());

} // namespace jp::grid
