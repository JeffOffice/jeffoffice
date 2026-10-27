#include "grid/printlayout.h"

#include "grid/gridsheet.h"
#include "grid/look.h"

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::CellType;
using jp::office::sheet::kMaxColumns;
using jp::office::sheet::kMaxMarginInches;
using jp::office::sheet::kMaxPageScale;
using jp::office::sheet::kMinPageScale;
using jp::office::sheet::kMaxRows;
using jp::office::sheet::PageOrder;

double columnPoints(double fileWidth, int maxDigitWidth) { return GridSheet::pointsForPixels(GridSheet::columnPixels(fileWidth, maxDigitWidth)); }

double rowPoints(double heightInPoints) { return GridSheet::pointsForPixels(GridSheet::rowPixels(heightInPoints)); }

int printDigitPixels(const Styles &styles, int dpi)
{
    // (a workbook with no fonts has the usual one: the screen takes 7 pixels for it, and 46 is what that comes to on paper)
    if (styles.fonts.isEmpty()) return int(std::max<long>(1, std::lround(11.0 * double(dpi) / 72.0 * digitWidthFactor(QStringLiteral("Calibri")))));
    const jp::office::sheet::FontSpec &f = styles.font(0);
    const double em = std::max(1.0, f.size) * double(dpi) / 72.0;
    return int(std::max<long>(1, std::lround(em * digitWidthFactor(f.name))));
}

double printColumnPoints(double fileWidth, int digitPixels, int dpi)
{
    if (!(fileWidth > 0) || digitPixels <= 0) return 0;
    const double px = std::floor((256.0 * fileWidth + std::floor(128.0 / digitPixels)) / 256.0 * digitPixels);
    return px * 72.0 / double(dpi);
}

Axis printColumnAxis(const Sheet &sheet, const Styles &styles, int fine, int dpi)
{
    using jp::office::sheet::ColumnSpan;
    fine = std::clamp(fine, 1, 64);
    const int mdw = LookTable(&styles).maxDigitWidth();
    const int mdwp = printDigitPixels(styles, dpi);
    auto pixelsOf = [&](double fileWidth) { return int(std::lround(printColumnPoints(fileWidth, mdwp, dpi) / kPointsPerPixel * fine)); };
    // the default width of a column: the file's, or the one its base width makes (in whole eighths of pixels of the screen, as the screen has it)
    double defaultWidth = sheet.format.defaultColWidth;
    if (!(defaultWidth > 0)) defaultWidth = std::floor(double((sheet.format.baseColWidth * mdw + 5 + 7) / 8 * 8) / std::max(1, mdw) * 256.0) / 256.0;
    Axis cols(qint64(jp::office::sheet::kMaxColumns), std::max(1, pixelsOf(defaultWidth)));
    std::vector<ColumnSpan> runs(sheet.columns.begin(), sheet.columns.end());
    std::stable_sort(runs.begin(), runs.end(), [](const ColumnSpan &a, const ColumnSpan &b) { return a.first < b.first; });
    std::vector<Axis::Place> places;
    quint32 next = 0;
    for (const ColumnSpan &run : runs) {
        const quint32 first = std::max(run.first, next);
        if (first > run.last || first >= jp::office::sheet::kMaxColumns) continue;
        const quint32 last = std::min(run.last, jp::office::sheet::kMaxColumns - 1);
        next = last + 1;
        const int size = run.hidden ? 0 : run.width >= 0 ? pixelsOf(run.width) : -1;
        if (size >= 0)
            for (quint32 c = first; c <= last; ++c) places.push_back({qint64(c), size});
    }
    cols.assign(std::move(places));
    cols.total();
    return cols;
}

// The row axis built a step at a time: the specs are copied, put in order of row if they are not (a merge sort that goes a few steps at a call), the last of any row given twice kept, the
// places made, and the axis worked out.
struct RowAxisTask::Impl {
    using RowSpec = jp::office::sheet::RowSpec;
    enum class Stage { Copy, Chunks, Merge, Unique, Emit, Build, Done };
    static constexpr size_t kChunk = 64;

    const Sheet *sheet;
    int fine;
    int defaultRow;
    Axis rows;
    Stage stage = Stage::Copy;
    std::vector<RowSpec> specs, spare;
    size_t at = 0, keep = 0;
    bool sorted = true;
    size_t width = kChunk, base = 0, l = 0, mid = 0, r = 0, end = 0;
    bool inPair = false;
    bool zeroHeight = false;
    qint64 nextRow = 0, lastRow = 0;
    size_t listed = 0;

    Impl(const Sheet &s, int f)
        : sheet(&s), fine(std::clamp(f, 1, 64)), defaultRow(std::max(1, sizeFor(s.format.defaultRowHeight > 0 ? s.format.defaultRowHeight : 15))), rows(qint64(jp::office::sheet::kMaxRows), defaultRow)
    {
        specs.reserve(size_t(s.rowSpecs.size()));
    }
    int sizeFor(double points) const { return int(std::lround(std::clamp(points, 0.0, 1000.0) / kPointsPerPixel * fine)); }
    int sizeOf(const RowSpec &s) const { return s.hidden ? 0 : s.height >= 0 ? sizeFor(s.height) : defaultRow; }

    bool step(Work &w)
    {
        const auto before = [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; };
        for (;;) {
            if (stage == Stage::Done) return true;
            if (w.spent()) return false;
            switch (stage) {
            case Stage::Copy:
                if (at < size_t(sheet->rowSpecs.size())) {
                    const RowSpec &s = sheet->rowSpecs[qsizetype(at++)];
                    if (!specs.empty() && s.row < specs.back().row) sorted = false;
                    specs.push_back(s);
                    w.add();
                } else {
                    at = 0;
                    stage = sorted ? Stage::Unique : Stage::Chunks;
                    keep = 0;
                }
                break;
            case Stage::Chunks:
                if (at < specs.size()) {
                    const size_t to = std::min(specs.size(), at + kChunk);
                    std::stable_sort(specs.begin() + std::ptrdiff_t(at), specs.begin() + std::ptrdiff_t(to), before);
                    w.add(qint64(kChunk) * 6);
                    at = to;
                } else {
                    stage = Stage::Merge;
                    base = 0;
                    inPair = false;
                    spare.clear();
                    spare.reserve(specs.size());
                }
                break;
            case Stage::Merge:
                if (width >= specs.size()) {
                    at = 0;
                    stage = Stage::Unique;
                    break;
                }
                if (base >= specs.size()) {
                    specs.swap(spare);
                    spare.clear();
                    width *= 2;
                    base = 0;
                    inPair = false;
                    break;
                }
                if (!inPair) {
                    l = base;
                    mid = std::min(specs.size(), base + width);
                    r = mid;
                    end = std::min(specs.size(), base + 2 * width);
                    inPair = true;
                }
                // (the left run before the right when they are equal: the sort is a stable one)
                while (spare.size() < end && !w.spent()) {
                    if (l < mid && (r >= end || !before(specs[r], specs[l]))) spare.push_back(specs[l++]);
                    else spare.push_back(specs[r++]);
                    w.add();
                }
                if (spare.size() >= end) {
                    inPair = false;
                    base = end;
                }
                break;
            case Stage::Unique:
                // a row the file gives twice has its last
                if (at < specs.size()) {
                    if (keep > 0 && specs[keep - 1].row == specs[at].row) specs[keep - 1] = specs[at];
                    else specs[keep++] = specs[at];
                    ++at;
                    w.add();
                } else {
                    specs.resize(keep);
                    spare = std::vector<RowSpec>();
                    zeroHeight = sheet->format.zeroHeight && !specs.empty();
                    if (zeroHeight) {
                        // the rows the file does not list are hidden, and the sheet ends at its last listed row
                        lastRow = qint64(specs.back().row);
                        rows.setCount(lastRow + 1);
                        rows.beginAssign(size_t(lastRow + 1));
                    } else {
                        rows.beginAssign(specs.size());
                    }
                    at = 0;
                    nextRow = 0;
                    stage = Stage::Emit;
                }
                break;
            case Stage::Emit:
                if (zeroHeight ? nextRow <= lastRow : at < specs.size()) {
                    if (zeroHeight) {
                        if (at < specs.size() && qint64(specs[at].row) == nextRow) rows.appendPlace({nextRow, sizeOf(specs[at++])});
                        else rows.appendPlace({nextRow, 0});
                        ++nextRow;
                    } else {
                        rows.appendPlace({qint64(specs[at].row), sizeOf(specs[at])});
                        ++at;
                    }
                    w.add();
                } else {
                    stage = Stage::Build;
                }
                break;
            case Stage::Build:
                if (rows.buildSome(w)) stage = Stage::Done;
                break;
            case Stage::Done: break;
            }
        }
    }
};

RowAxisTask::RowAxisTask(const Sheet &sheet, int fine) : d(std::make_unique<Impl>(sheet, fine)) {}
RowAxisTask::~RowAxisTask() = default;
bool RowAxisTask::step(Work &work) { return d->step(work); }
Axis RowAxisTask::take() { return d->rows; }

Axis printRowAxis(const Sheet &sheet, int fine)
{
    RowAxisTask task(sheet, fine);
    Work all;
    task.step(all);
    return task.take();
}

namespace {

constexpr double kMinContent = 36;   // the least a page can have to print on, in points, whatever the margins say

// One side pair of margins held so that the page between them is no smaller than kMinContent.
void holdMargins(double length, double *a, double *b)
{
    *a = std::clamp(*a, 0.0, kMaxMarginInches * 72);
    *b = std::clamp(*b, 0.0, kMaxMarginInches * 72);
    const double room = std::max(0.0, length - kMinContent);
    if (*a + *b > room) {
        const double both = *a + *b;
        *a = *a / both * room;
        *b = *b / both * room;
    }
}

} // namespace

PaperBox paperBox(const PageSetup &setup)
{
    PaperBox b;
    jp::office::sheet::pageSizePoints(setup, &b.width, &b.height);
    b.width = std::clamp(b.width, 72.0, 14400.0);
    b.height = std::clamp(b.height, 72.0, 14400.0);
    b.left = setup.margins.left * 72;
    b.right = setup.margins.right * 72;
    b.top = setup.margins.top * 72;
    b.bottom = setup.margins.bottom * 72;
    holdMargins(b.width, &b.left, &b.right);
    holdMargins(b.height, &b.top, &b.bottom);
    b.header = std::clamp(setup.margins.header * 72, 0.0, b.height);
    b.footer = std::clamp(setup.margins.footer * 72, 0.0, b.height);
    return b;
}

namespace {

// Whether a cell format shows anything by itself: a fill (any pattern, white or not), or a border on any side (the original counts these and nothing else of a format: not a number format, an
// alignment, a font, a color of text, wrapping, protection, nor a note, a rule of conditional formatting, a validation, nor the width or height of its column or row).
bool formatShows(const Styles &styles, quint32 xf)
{
    const jp::office::sheet::CellFormat &f = styles.format(xf);
    if (styles.fill(f.fill).pattern != jp::office::sheet::PatternKind::None) return true;
    const jp::office::sheet::BorderSpec &b = styles.border(f.border);
    using jp::office::sheet::BorderStyle;
    return b.left.style != BorderStyle::None || b.right.style != BorderStyle::None || b.top.style != BorderStyle::None || b.bottom.style != BorderStyle::None
           || (b.diagonal.style != BorderStyle::None && (b.diagonalUp || b.diagonalDown));
}

// The last row and column of the cells that have something to show, and the objects over them, found a step at a time (see Work): first the cells, a row at a time from the last row up, then
// the merged ranges, then the objects. `any` is false when there is nothing.
class UsedScan {
public:
    UsedScan(const PrintRequest &q, const Axis &rows, const Axis &cols) : m_q(q), m_rows(rows), m_cols(cols), m_shows(q.styles ? size_t(q.styles->formats.size()) : 0, -1) {}

    bool step(Work &work)
    {
        if (m_phase == 0) {
            qint64 steps = 0;
            const qint64 room = std::max<qint64>(1, work.limit - work.used);
            const bool done = m_q.sheet->cells.forEachRowTailSome(&m_cursor, room, &steps, [&](quint32 row, quint32 col, const jp::office::sheet::Cell &cell) {
                forgetWhatWasFound();
                bool counts = cell.type != CellType::Empty || cell.hasFormula();
                if (!counts && m_q.styles && cell.style != 0 && cell.style < quint32(m_shows.size())) {
                    signed char &s = m_shows[cell.style];
                    if (s < 0) s = formatShows(*m_q.styles, cell.style) ? 1 : 0;
                    counts = s == 1;
                }
                if (!counts) return false;
                any = true;
                lastRow = std::max(lastRow, row);
                lastCol = std::max<quint32>(lastCol, col);
                return true;
            });
            forgetWhatWasFound();   // (the walk began again and found nothing)
            work.add(steps);
            if (!done) return false;
            m_phase = 1;
        }
        if (m_phase == 1) {
            // a merged range of more than one cell counts, where it is and whether or not its first cell has anything (measured against the original: a merge of empty cells far out is printed to its last cell,
            // a merge of one cell is not)
            while (m_at < size_t(m_q.sheet->merges.size())) {
                if (work.spent()) return false;
                const CellRange &m = m_q.sheet->merges[qsizetype(m_at++)];
                work.add();
                if (m.valid && (m.row2 > m.row1 || m.col2 > m.col1)) {
                    any = true;
                    lastRow = std::max(lastRow, m.row2);
                    lastCol = std::max(lastCol, m.col2);
                }
            }
            m_phase = 2;
            m_at = 0;
        }
        while (m_at < m_q.objects.size()) {
            if (work.spent()) return false;
            const ObjectBox &b = m_q.objects[m_at++];
            work.add();
            if (b.isNothing()) continue;
            const qint64 right = qint64(std::min(1e12, (b.x + b.w) * m_q.colFine)) - 1, bottom = qint64(std::min(1e12, (b.y + b.h) * m_q.rowFine)) - 1;
            qint64 oc = m_cols.indexAt(std::max<qint64>(0, right)), orow = m_rows.indexAt(std::max<qint64>(0, bottom));
            if (oc < 0) oc = m_cols.count() - 1;
            if (orow < 0) orow = m_rows.count() - 1;
            any = true;
            lastRow = std::max<quint32>(lastRow, quint32(orow));
            lastCol = std::max<quint32>(lastCol, quint32(oc));
        }
        return true;
    }

    bool any = false;
    quint32 lastRow = 0, lastCol = 0;

private:
    // The rows or columns were edited between two steps, so the walk began again from the end: the last row and column it had found are of the sheet as it was.
    void forgetWhatWasFound()
    {
        if (!m_cursor.restarted) return;
        m_cursor.restarted = false;
        any = false;
        lastRow = lastCol = 0;
    }
    const PrintRequest &m_q;
    const Axis &m_rows, &m_cols;
    std::vector<signed char> m_shows;
    jp::office::sheet::CellStore::RowTailCursor m_cursor;
    int m_phase = 0;
    size_t m_at = 0;
};

// Takes the hidden rows and columns off the ends of a block; false if no row or no column of it shows.
bool trimHidden(const Axis &rows, const Axis &cols, CellRange *b)
{
    const qint64 r1 = rows.nextVisible(qint64(b->row1) - 1), r2 = rows.previousVisible(qint64(b->row2) + 1);
    const qint64 c1 = cols.nextVisible(qint64(b->col1) - 1), c2 = cols.previousVisible(qint64(b->col2) + 1);
    if (r1 > qint64(b->row2) || r2 < qint64(b->row1) || r1 > r2 || c1 > qint64(b->col2) || c2 < qint64(b->col1) || c1 > c2) return false;
    // (the hidden rows and columns at the start stay in the block: a manual break among them still begins a page, and cutAxis steps over them to the first that shows)
    b->row2 = quint32(r2);
    b->col2 = quint32(c2);
    return true;
}

Axis withoutSkips(const Axis *a)
{
    Axis copy = *a;
    copy.setSkipped(0);
    return copy;
}

CellRange held(const CellRange &r, const Axis &rows, const Axis &cols)
{
    CellRange out = r;
    out.row2 = std::min<quint32>(out.row2, quint32(std::max<qint64>(0, rows.count() - 1)));
    out.col2 = std::min<quint32>(out.col2, quint32(std::max<qint64>(0, cols.count() - 1)));
    out.valid = r.valid && out.row1 <= out.row2 && out.col1 <= out.col2;
    return out;
}

// What the scan for the extent of the used cells found, once it was needed.
struct UsedState {
    bool known = false, any = false;
    quint32 row = 0, col = 0;
    bool wanted = false;   // blocksOf could not go on without it (the blocks it gave are not the blocks)
};

std::vector<CellRange> blocksOf(const PrintRequest &q, const Axis &rows, const Axis &cols, UsedState *used)
{
    std::vector<CellRange> blocks;
    if (!q.sheet) return blocks;
    // Whole rows (or whole columns) are printed as far as the sheet has anything: a print area of $1:$1048576 is not a million rows of paper.
    auto wholeLines = [&](CellRange b, CellRange *out) {
        const bool wholeColumns = b.row1 == 0 && qint64(b.row2) >= rows.count() - 1, wholeRows = b.col1 == 0 && qint64(b.col2) >= cols.count() - 1;
        if (wholeColumns || wholeRows) {
            if (!used->known) {
                used->wanted = true;
                return false;
            }
            if (!used->any) return false;
            if (wholeColumns) b.row2 = std::min(b.row2, used->row);
            if (wholeRows) b.col2 = std::min(b.col2, used->col);
        }
        *out = b;
        return true;
    };
    if (q.onlySelection) {
        CellRange s = held(q.selection, rows, cols);
        if (s.valid && wholeLines(s, &s)) blocks.push_back(s);
    } else if (q.setup.hasPrintArea() && !q.ignorePrintArea) {
        for (const CellRange &a : q.setup.printArea) {
            CellRange h = held(a, rows, cols);
            if (h.valid && wholeLines(h, &h)) blocks.push_back(h);
        }
    } else if (!used->known) {
        used->wanted = true;
        return blocks;
    } else if (used->any) {
        CellRange all;
        all.valid = true;
        all.row1 = 0;
        all.col1 = 0;
        all.row2 = std::min<quint32>(used->row, quint32(rows.count() - 1));
        all.col2 = std::min<quint32>(used->col, quint32(cols.count() - 1));
        blocks.push_back(all);
    }
    std::vector<CellRange> shown;
    for (CellRange b : blocks)
        if (trimHidden(rows, cols, &b)) shown.push_back(b);
    return shown;
}

// One page's share of an axis: the rows (or columns) of a band, and whether the titles go with it.
struct Band {
    qint64 first, last;
    bool titled;
};

struct Titles {
    qint64 first = -1, last = -1;   // -1: none
    double extent = 0;              // the axis's units
};

// Cuts [first, last] of an axis into the bands that fit `capacity` pixels (less the titles' extent for a band after them), a new one at each break, a band at a time (see Work). The number of
// bands is `bands`; the first `keep` of them are put in `out`.
class BandCutter {
public:
    BandCutter(const Axis &axis, qint64 first, qint64 last, double capacity, const Titles &titles, std::vector<qint64> breaks, qint64 keep, std::vector<Band> *out)
        : m_axis(&axis), m_first(first), m_last(last), m_capacity(capacity), m_titles(titles), m_breaks(std::move(breaks)), m_keep(keep), m_out(out)
    {
    }

    qint64 bands = 0;

    bool step(Work &w)
    {
        const Axis &axis = *m_axis;
        const std::vector<qint64> &breaks = m_breaks;
        if (!m_started) {
            m_started = true;
            m_pos = axis.nextVisible(m_first - 1);
            // Breaks among the hidden rows at the start each close a page too, the first of them one of rows that are all hidden: a blank page.
            if (!breaks.empty() && m_pos <= m_last && m_pos < axis.count()) {
                const auto lo = std::upper_bound(breaks.begin(), breaks.end(), m_first);   // (a break at the first row begins nothing before it)
                const auto hi = std::upper_bound(lo, breaks.end(), m_pos);
                for (auto it = lo; it != hi; ++it) {
                    const qint64 from = it == lo ? m_first : *(it - 1);
                    if (m_out && bands < m_keep) m_out->push_back({from, *it - 1, m_titles.last >= 0 && m_titles.extent * 2 <= m_capacity && from > m_titles.last});
                    ++bands;
                }
                w.add(hi - lo);
            }
        }
        while (m_pos <= m_last && m_pos < axis.count()) {
            if (w.spent()) return false;
            w.add();
            // (titles that take more than half a page leave no room for the sheet: they are not repeated, rather than one row a page under a page of titles)
            const bool titled = m_titles.last >= 0 && m_titles.extent * 2 <= m_capacity && m_pos > m_titles.last;
            const double cap = std::max(1.0, m_capacity - (titled ? m_titles.extent : 0.0));
            qint64 limit = m_last;
            const auto next = std::upper_bound(breaks.begin(), breaks.end(), m_pos);
            if (next != breaks.end()) limit = std::min(limit, *next - 1);
            const double start = double(axis.position(m_pos));
            const qint64 reach = qint64(std::floor(start + cap + 1e-6));
            const qint64 row = axis.indexAt(reach);
            qint64 end = row < 0 ? axis.count() - 1 : row - 1;
            end = std::min(std::max(end, m_pos), limit);
            if (m_out && bands < m_keep) m_out->push_back({m_pos, end, titled});
            ++bands;
            const qint64 nextPos = axis.nextVisible(end);
            // Rows that are all hidden between two manual breaks still make a page (a blank one, with its header and footer and the titles): the breaks that fall in the hidden rows between this
            // band and the next each close a page. (The common programs do so: a sheet filtered down to a few rows of a list that has a break every page keeps all those pages.)
            if (!breaks.empty() && nextPos <= m_last && nextPos < axis.count()) {
                const auto lo = std::upper_bound(breaks.begin(), breaks.end(), end);
                const auto hi = std::upper_bound(lo, breaks.end(), nextPos);
                for (auto it = lo; it != hi && it + 1 != hi; ++it) {
                    if (m_out && bands < m_keep) m_out->push_back({*it, *(it + 1) - 1, m_titles.last >= 0 && m_titles.extent * 2 <= m_capacity && *it > m_titles.last});
                    ++bands;
                }
                w.add(hi - lo);
            }
            m_pos = nextPos;
        }
        return true;
    }

private:
    const Axis *m_axis;
    qint64 m_first, m_last;
    double m_capacity;
    Titles m_titles;
    std::vector<qint64> m_breaks;
    qint64 m_keep;
    std::vector<Band> *m_out;
    bool m_started = false;
    qint64 m_pos = 0;
};

std::vector<qint64> breaksIn(const QVector<jp::office::sheet::PageBreak> &list, qint64 first, qint64 last)
{
    std::vector<qint64> out;
    for (const jp::office::sheet::PageBreak &b : list)
        if (b.manual && qint64(b.before) > first && qint64(b.before) <= last) out.push_back(qint64(b.before));
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

Titles titlesOf(const Axis &axis, bool has, quint32 first, quint32 last)
{
    Titles t;
    if (!has) return t;
    const qint64 a = std::min<qint64>(first, last), b = std::min<qint64>(std::max<qint64>(first, last), axis.count() - 1);
    if (a > b) return t;
    const double extent = double(axis.position(b + 1) - axis.position(a));
    if (extent <= 0) return t;
    t.first = a;
    t.last = b;
    t.extent = extent;
    return t;
}

struct Context {
    const Axis *rows, *cols;
    Titles titleRows, titleCols;
    double contentW, contentH;   // pixels at 100% that a page holds at scale 1, less the headings
    bool headings;
    int rowFine = 1, colFine = 1;
};

// The capacity of a page in the axis's units (`fine` to the grid's pixel) at the scale (a fraction); the headings are in pixels.
double capacityOf(double points, double scale, double headingPixels, int fine) { return (points / kPointsPerPixel / scale - headingPixels) * fine; }

} // namespace

namespace {

// The blocks that print, found with no limit on the work.
std::vector<CellRange> blocksNow(const PrintRequest &q, const Axis &rows, const Axis &cols)
{
    UsedState used;
    std::vector<CellRange> blocks = blocksOf(q, rows, cols, &used);
    if (!used.wanted) return blocks;
    UsedScan scan(q, rows, cols);
    Work all;
    scan.step(all);
    used.known = true;
    used.any = scan.any;
    used.row = scan.lastRow;
    used.col = scan.lastCol;
    return blocksOf(q, rows, cols, &used);
}

} // namespace

std::vector<CellRange> printedBlocks(const PrintRequest &request)
{
    if (!request.rows || !request.cols) return {};
    return blocksNow(request, withoutSkips(request.rows), withoutSkips(request.cols));
}

struct PaginateTask::Impl {
    enum class Stage { Blocks, Scan, Fit, Pages, Done };
    // the stages of the fit: the whole sheet at the ceiling, at the least, and then between them
    enum class Fit { Ceiling, Least, Between, Done };

    PrintRequest q;
    Axis rows, cols;
    SheetPrint out;
    Context c;
    Stage stage = Stage::Blocks;
    UsedState used;
    std::unique_ptr<UsedScan> scan;
    bool fit = false;
    int percent = 100;
    double scale = 1;

    // the fit
    Fit fitStage = Fit::Ceiling;
    int ceiling = 100, lo = 0, hi = 0, probing = 0;
    size_t probeArea = 0;
    int probeAxis = 0;
    qint64 probeRows = 0;
    std::unique_ptr<BandCutter> probe;

    // the pages
    qint64 room = 0;
    size_t area = 0;
    int pageStage = 0;
    std::vector<Band> rowBands, colBands;
    std::unique_ptr<BandCutter> rowCut, colCut;
    qint64 emitted = 0;

    explicit Impl(const PrintRequest &request) : q(request)
    {
        if (!q.sheet || !q.rows || !q.cols) {
            stage = Stage::Done;
            return;
        }
        rows = withoutSkips(q.rows);
        cols = withoutSkips(q.cols);
        const PageSetup &setup = q.setup;
        out.paper = paperBox(setup);
        out.headings = setup.headings;
        percent = std::clamp(setup.scale, kMinPageScale, kMaxPageScale);
        c.rows = &rows;
        c.cols = &cols;
        c.contentW = out.paper.content().width();
        c.contentH = out.paper.content().height();
        c.headings = setup.headings;
        c.rowFine = std::clamp(q.rowFine, 1, 64);
        c.colFine = std::clamp(q.colFine, 1, 64);
        c.titleRows = titlesOf(rows, setup.titles.hasRows, setup.titles.row1, setup.titles.row2);
        c.titleCols = titlesOf(cols, setup.titles.hasCols, setup.titles.col1, setup.titles.col2);
        fit = setup.fitToPage && (setup.fitToWidth > 0 || setup.fitToHeight > 0);
        room = std::max(1, q.maxPages);
    }

    double capacityRows(double sc) const { return capacityOf(c.contentH, sc, c.headings ? kColumnHeadingPixels : 0, c.rowFine); }
    double capacityCols(double sc) const { return capacityOf(c.contentW, sc, c.headings ? kRowHeadingPixels : 0, c.colFine); }

    // Whether the sheet fits the pages asked for at the percent (the blocks cut each way, the breaks not looked at); false until the answer is known.
    bool probeStep(Work &w, int pct, bool *fits)
    {
        const PageSetup &setup = q.setup;
        while (probeArea < out.areas.size()) {
            const CellRange &b = out.areas[probeArea];
            if (!probe) {
                probe = probeAxis == 0 ? std::make_unique<BandCutter>(rows, b.row1, b.row2, capacityRows(pct / 100.0), c.titleRows, std::vector<qint64>(), 0, nullptr)
                                       : std::make_unique<BandCutter>(cols, b.col1, b.col2, capacityCols(pct / 100.0), c.titleCols, std::vector<qint64>(), 0, nullptr);
            }
            if (!probe->step(w)) return false;
            const qint64 n = probe->bands;
            probe.reset();
            if (probeAxis == 0) {
                probeRows = n;
                probeAxis = 1;
            } else {
                probeAxis = 0;
                ++probeArea;
                if ((setup.fitToHeight > 0 && probeRows > setup.fitToHeight) || (setup.fitToWidth > 0 && n > setup.fitToWidth)) {
                    *fits = false;
                    probeArea = 0;
                    return true;
                }
            }
        }
        *fits = true;
        probeArea = 0;
        return true;
    }

    // The scale: the percent that was asked, or the largest that fits the pages asked for.
    bool fitStep(Work &w)
    {
        for (;;) {
            bool fits = false;
            switch (fitStage) {
            case Fit::Ceiling:
                if (!probeStep(w, ceiling, &fits)) return false;
                if (fits) {
                    percent = ceiling;
                    fitStage = Fit::Done;
                } else {
                    fitStage = Fit::Least;
                }
                break;
            case Fit::Least:
                if (!probeStep(w, kMinPageScale, &fits)) return false;
                if (!fits) {
                    percent = kMinPageScale;
                    out.tooSmall = true;
                    fitStage = Fit::Done;
                } else {
                    lo = kMinPageScale;   // fits at lo, not at hi
                    hi = ceiling;
                    probing = 0;
                    fitStage = Fit::Between;
                }
                break;
            case Fit::Between:
                if (hi - lo <= 1) {
                    percent = lo;
                    fitStage = Fit::Done;
                    break;
                }
                if (!probeStep(w, (lo + hi) / 2, &fits)) return false;
                if (fits) lo = (lo + hi) / 2;
                else hi = (lo + hi) / 2;
                break;
            case Fit::Done: return true;
            }
        }
    }

    bool add(qint64 ri, qint64 ci, qint64 nr, qint64 nc, size_t areaIndex)
    {
        if (qint64(out.pages.size()) >= room) {
            out.truncated = true;
            return false;
        }
        PageCut p;
        p.area = int(areaIndex);
        p.row1 = rowBands[size_t(ri)].first;
        p.row2 = rowBands[size_t(ri)].last;
        p.col1 = colBands[size_t(ci)].first;
        p.col2 = colBands[size_t(ci)].last;
        if (rowBands[size_t(ri)].titled) {
            p.titleRow1 = c.titleRows.first;
            p.titleRow2 = c.titleRows.last;
        }
        if (colBands[size_t(ci)].titled) {
            p.titleCol1 = c.titleCols.first;
            p.titleCol2 = c.titleCols.last;
        }
        p.rowBand = int(ri);
        p.colBand = int(ci);
        p.rowBands = int(std::min<qint64>(nr, 1 << 30));
        p.colBands = int(std::min<qint64>(nc, 1 << 30));
        out.pages.push_back(p);
        return true;
    }

    bool pagesStep(Work &w)
    {
        const PageSetup &setup = q.setup;
        while (area < out.areas.size()) {
            const CellRange &b = out.areas[area];
            if (w.spent()) return false;
            if (pageStage == 0) {
                const std::vector<qint64> rowBreaks = fit ? std::vector<qint64>() : breaksIn(setup.rowBreaks, b.row1, b.row2);
                const std::vector<qint64> colBreaks = fit ? std::vector<qint64>() : breaksIn(setup.colBreaks, b.col1, b.col2);
                w.add(qint64(rowBreaks.size() + colBreaks.size()));
                rowBands.clear();
                colBands.clear();
                rowCut = std::make_unique<BandCutter>(rows, b.row1, b.row2, capacityRows(scale), c.titleRows, rowBreaks, room, &rowBands);
                colCut = std::make_unique<BandCutter>(cols, b.col1, b.col2, capacityCols(scale), c.titleCols, colBreaks, room, &colBands);
                pageStage = 1;
            }
            if (pageStage == 1) {
                if (!rowCut->step(w)) return false;
                pageStage = 2;
            }
            if (pageStage == 2) {
                if (!colCut->step(w)) return false;
                out.pagesNeeded += rowCut->bands * colCut->bands;
                emitted = 0;
                pageStage = 3;
            }
            const qint64 nr = rowCut->bands, nc = colCut->bands;
            const qint64 rn = qint64(rowBands.size()), cn = qint64(colBands.size());
            bool more = true;
            while (more && emitted < rn * cn) {
                if (w.spent()) return false;
                const bool overFirst = setup.pageOrder == PageOrder::OverThenDown;
                const qint64 ri = overFirst ? emitted / cn : emitted % rn, ci = overFirst ? emitted % cn : emitted / rn;
                more = add(ri, ci, nr, nc, area);
                ++emitted;
                w.add();
            }
            if (nr * nc > 0 && nr * nc > room && qint64(out.pages.size()) >= room) out.truncated = true;
            rowCut.reset();
            colCut.reset();
            rowBands.clear();
            colBands.clear();
            pageStage = 0;
            ++area;
        }
        return true;
    }

    bool step(Work &w)
    {
        for (;;) {
            switch (stage) {
            case Stage::Done: return true;
            case Stage::Blocks: {
                if (w.spent()) return false;
                out.areas = blocksOf(q, rows, cols, &used);
                if (used.wanted) {
                    scan = std::make_unique<UsedScan>(q, rows, cols);
                    stage = Stage::Scan;
                    break;
                }
                if (out.areas.empty()) {
                    out.scalePercent = percent;
                    stage = Stage::Done;
                    break;
                }
                ceiling = std::clamp(q.fitCeiling, kMinPageScale, kMaxPageScale);
                out.fitted = fit;
                stage = fit ? Stage::Fit : Stage::Pages;
                if (!fit) scale = percent / 100.0;
                break;
            }
            case Stage::Scan:
                if (!scan->step(w)) return false;
                used.known = true;
                used.wanted = false;
                used.any = scan->any;
                used.row = scan->lastRow;
                used.col = scan->lastCol;
                scan.reset();
                stage = Stage::Blocks;
                break;
            case Stage::Fit:
                if (!fitStep(w)) return false;
                out.scalePercent = percent;
                scale = percent / 100.0;
                stage = Stage::Pages;
                break;
            case Stage::Pages:
                out.scalePercent = percent;
                if (!pagesStep(w)) return false;
                stage = Stage::Done;
                break;
            }
        }
    }
};

PaginateTask::PaginateTask(const PrintRequest &request) : d(std::make_unique<Impl>(request)) {}
PaginateTask::~PaginateTask() = default;
bool PaginateTask::step(Work &work) { return d->step(work); }
SheetPrint PaginateTask::take() { return std::move(d->out); }

SheetPrint paginate(const PrintRequest &request)
{
    PaginateTask task(request);
    Work all;
    task.step(all);
    return task.take();
}

} // namespace jp::grid
