#include "sheet/odsnotes.h"

#include "sheet/cellref.h"

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {
constexpr double kEmuPerPoint = 12700.0;
constexpr double kEmuPerPx = 9525.0;
constexpr double kMostPoints = 1e9;   // (a sheet's whole height is under 1.1e8 points: a place past this is not one)

qint64 emuOfPoints(double points) { return qint64(std::llround(points * kEmuPerPoint)); }

int clampTo(qint64 v, int most) { return int(std::clamp<qint64>(v, -most, most)); }
} // namespace

bool odsLengthPoints(const QString &text, double *points)
{
    const QString s = text.trimmed();
    qsizetype i = s.size();
    while (i > 0 && s[i - 1].isLetter()) --i;
    if (i == 0 || i == s.size() || s.size() - i > 2) return false;
    const QString unit = s.mid(i);
    double per = 0;
    if (unit == QLatin1String("pt")) per = 1;
    else if (unit == QLatin1String("cm")) per = 72.0 / 2.54;
    else if (unit == QLatin1String("mm")) per = 72.0 / 25.4;
    else if (unit == QLatin1String("in")) per = 72;
    else if (unit == QLatin1String("pc")) per = 12;
    else if (unit == QLatin1String("px")) per = 0.75;
    else return false;
    bool ok = false;
    const double n = s.left(i).trimmed().toDouble(&ok);
    if (!ok || !std::isfinite(n)) return false;
    const double v = n * per;
    if (!std::isfinite(v) || std::abs(v) > kMostPoints) return false;
    *points = v;
    return true;
}

NoteBox noteBoxFromOds(double x, double y, double width, double height, quint32 row, quint32 col, const odsdraw::Grid &grid)
{
    NoteBox box;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || !std::isfinite(height)) return box;
    if (x < 0 || y < 0 || !(width > 0) || !(height > 0) || x > kMostPoints || y > kMostPoints || width > kMostPoints || height > kMostPoints) return box;
    auto corner = [&](double px, double py, int *cellRow, int *cellCol, int *offY, int *offX) {
        quint32 r = 0, c = 0;
        qint64 oy = 0, ox = 0;
        grid.columnAt(emuOfPoints(px), &c, &ox);
        grid.rowAt(emuOfPoints(py), &r, &oy);
        *cellCol = clampTo(qint64(c) - qint64(col), kMaxNoteBoxCells);
        *cellRow = clampTo(qint64(r) - qint64(row), kMaxNoteBoxCells);
        *offX = clampTo(std::llround(double(ox) / kEmuPerPx), kMaxNoteBoxPixels);
        *offY = clampTo(std::llround(double(oy) / kEmuPerPx), kMaxNoteBoxPixels);
    };
    corner(x, y, &box.row1, &box.col1, &box.y1, &box.x1);
    corner(x + width, y + height, &box.row2, &box.col2, &box.y2, &box.x2);
    box.set = true;
    return box;
}

bool noteBoxToOds(const NoteBox &placed, quint32 row, quint32 col, const odsdraw::Grid &grid, double *x, double *y, double *width, double *height)
{
    if (!placed.set) return false;
    const NoteBox box = noteBoxInsideSheet(placed, row, col);
    auto corner = [&](int dRow, int dCol, int offY, int offX, double *px, double *py) {
        const qint64 c = std::clamp<qint64>(qint64(col) + dCol, 0, kMaxColumns - 1), r = std::clamp<qint64>(qint64(row) + dRow, 0, kMaxRows - 1);
        *px = double(grid.columnStart(quint32(c))) / kEmuPerPoint + double(std::max(0, offX)) * kEmuPerPx / kEmuPerPoint;
        *py = double(grid.rowStart(quint32(r))) / kEmuPerPoint + double(std::max(0, offY)) * kEmuPerPx / kEmuPerPoint;
    };
    double x2 = 0, y2 = 0;
    corner(box.row1, box.col1, box.y1, box.x1, x, y);
    corner(box.row2, box.col2, box.y2, box.x2, &x2, &y2);
    *width = x2 - *x;
    *height = y2 - *y;
    return *width > 0 && *height > 0;
}

} // namespace jp::office::sheet
