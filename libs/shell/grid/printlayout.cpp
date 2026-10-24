#include "grid/printlayout.h"

#include "grid/gridsheet.h"

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

// Whether a cell format shows anything by itself: a fill, or a border.
bool formatShows(const Styles &styles, quint32 xf)
{
    const jp::office::sheet::CellFormat &f = styles.format(xf);
    if (styles.fill(f.fill).pattern != jp::office::sheet::PatternKind::None) return true;
    const jp::office::sheet::BorderSpec &b = styles.border(f.border);
    using jp::office::sheet::BorderStyle;
    return b.left.style != BorderStyle::None || b.right.style != BorderStyle::None || b.top.style != BorderStyle::None || b.bottom.style != BorderStyle::None
           || (b.diagonal.style != BorderStyle::None && (b.diagonalUp || b.diagonalDown));
}

// The last row and column of the cells that have something to show, and the objects over them; false when there is nothing.
bool usedExtent(const PrintRequest &q, const Axis &rows, const Axis &cols, quint32 *lastRow, quint32 *lastCol)
{
    bool any = false;
    quint32 r = 0, c = 0;
    std::vector<signed char> shows(q.styles ? size_t(q.styles->formats.size()) : 0, -1);
    q.sheet->cells.forEach([&](const jp::office::sheet::Cell &cell) {
        bool counts = cell.type != CellType::Empty || cell.hasFormula();
        if (!counts && q.styles && cell.style != 0 && cell.style < quint32(shows.size())) {
            signed char &s = shows[cell.style];
            if (s < 0) s = formatShows(*q.styles, cell.style) ? 1 : 0;
            counts = s == 1;
        }
        if (!counts) return;
        any = true;
        r = std::max(r, cell.row);
        c = std::max<quint32>(c, cell.col);
    });
    if (any) {
        // a merged range with its first cell in the block reaches to its last
        for (const CellRange &m : q.sheet->merges)
            if (m.valid && m.row1 <= r && m.col1 <= c) {
                if (q.sheet->cells.find(m.row1, m.col1)) {
                    r = std::max(r, m.row2);
                    c = std::max(c, m.col2);
                }
            }
    }
    for (const ObjectBox &b : q.objects) {
        if (b.isNothing()) continue;
        const qint64 right = qint64(std::min(1e12, b.x + b.w)) - 1, bottom = qint64(std::min(1e12, b.y + b.h)) - 1;
        qint64 oc = cols.indexAt(std::max<qint64>(0, right)), orow = rows.indexAt(std::max<qint64>(0, bottom));
        if (oc < 0) oc = cols.count() - 1;
        if (orow < 0) orow = rows.count() - 1;
        any = true;
        r = std::max<quint32>(r, quint32(orow));
        c = std::max<quint32>(c, quint32(oc));
    }
    *lastRow = r;
    *lastCol = c;
    return any;
}

// Takes the hidden rows and columns off the ends of a block; false if no row or no column of it shows.
bool trimHidden(const Axis &rows, const Axis &cols, CellRange *b)
{
    const qint64 r1 = rows.nextVisible(qint64(b->row1) - 1), r2 = rows.previousVisible(qint64(b->row2) + 1);
    const qint64 c1 = cols.nextVisible(qint64(b->col1) - 1), c2 = cols.previousVisible(qint64(b->col2) + 1);
    if (r1 > qint64(b->row2) || r2 < qint64(b->row1) || r1 > r2 || c1 > qint64(b->col2) || c2 < qint64(b->col1) || c1 > c2) return false;
    b->row1 = quint32(r1);
    b->row2 = quint32(r2);
    b->col1 = quint32(c1);
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

std::vector<CellRange> blocksOf(const PrintRequest &q, const Axis &rows, const Axis &cols)
{
    std::vector<CellRange> blocks;
    if (!q.sheet) return blocks;
    // Whole rows (or whole columns) are printed as far as the sheet has anything: a print area of $1:$1048576 is not a million rows of paper.
    bool usedKnown = false, anyUsed = false;
    quint32 usedRow = 0, usedCol = 0;
    auto wholeLines = [&](CellRange b, CellRange *out) {
        const bool wholeColumns = b.row1 == 0 && qint64(b.row2) >= rows.count() - 1, wholeRows = b.col1 == 0 && qint64(b.col2) >= cols.count() - 1;
        if (wholeColumns || wholeRows) {
            if (!usedKnown) {
                usedKnown = true;
                anyUsed = usedExtent(q, rows, cols, &usedRow, &usedCol);
            }
            if (!anyUsed) return false;
            if (wholeColumns) b.row2 = std::min(b.row2, usedRow);
            if (wholeRows) b.col2 = std::min(b.col2, usedCol);
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
    } else {
        quint32 lastRow = 0, lastCol = 0;
        if (usedExtent(q, rows, cols, &lastRow, &lastCol)) {
            CellRange all;
            all.valid = true;
            all.row1 = 0;
            all.col1 = 0;
            all.row2 = std::min<quint32>(lastRow, quint32(rows.count() - 1));
            all.col2 = std::min<quint32>(lastCol, quint32(cols.count() - 1));
            blocks.push_back(all);
        }
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
    double extent = 0;              // pixels
};

// Cuts [first, last] of an axis into the bands that fit `capacity` pixels (less the titles' extent for a band after them), a new one at each break. Gives how many bands there are; the first
// `keep` of them are put in `out`.
qint64 cutAxis(const Axis &axis, qint64 first, qint64 last, double capacity, const Titles &titles, const std::vector<qint64> &breaks, qint64 keep, std::vector<Band> *out)
{
    qint64 bands = 0;
    qint64 pos = axis.nextVisible(first - 1);
    while (pos <= last && pos < axis.count()) {
        const bool titled = titles.last >= 0 && pos > titles.last;
        const double cap = std::max(1.0, capacity - (titled ? titles.extent : 0.0));
        qint64 limit = last;
        const auto next = std::upper_bound(breaks.begin(), breaks.end(), pos);
        if (next != breaks.end()) limit = std::min(limit, *next - 1);
        const double start = double(axis.position(pos));
        const qint64 reach = qint64(std::floor(start + cap + 1e-6));
        const qint64 row = axis.indexAt(reach);
        qint64 end = row < 0 ? axis.count() - 1 : row - 1;
        end = std::min(std::max(end, pos), limit);
        if (out && bands < keep) out->push_back({pos, end, titled});
        ++bands;
        pos = axis.nextVisible(end);
    }
    return bands;
}

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
};

// The capacity of a page in the grid's pixels at the scale (a fraction).
double capacityOf(double points, double scale, double headingPixels) { return points / kPointsPerPixel / scale - headingPixels; }

// How many bands the block needs each way at the scale (the breaks are not looked at).
void bandCounts(const Context &c, const CellRange &b, double scale, qint64 *rowBands, qint64 *colBands)
{
    *rowBands = cutAxis(*c.rows, b.row1, b.row2, capacityOf(c.contentH, scale, c.headings ? kColumnHeadingPixels : 0), c.titleRows, {}, 0, nullptr);
    *colBands = cutAxis(*c.cols, b.col1, b.col2, capacityOf(c.contentW, scale, c.headings ? kRowHeadingPixels : 0), c.titleCols, {}, 0, nullptr);
}

} // namespace

std::vector<CellRange> printedBlocks(const PrintRequest &request)
{
    if (!request.rows || !request.cols) return {};
    return blocksOf(request, withoutSkips(request.rows), withoutSkips(request.cols));
}

SheetPrint paginate(const PrintRequest &request)
{
    SheetPrint out;
    if (!request.sheet || !request.rows || !request.cols) return out;
    const Axis rows = withoutSkips(request.rows), cols = withoutSkips(request.cols);
    const PageSetup &setup = request.setup;
    out.paper = paperBox(setup);
    out.areas = blocksOf(request, rows, cols);
    out.headings = setup.headings;
    int percent = std::clamp(setup.scale, kMinPageScale, kMaxPageScale);
    if (out.areas.empty()) {
        out.scalePercent = percent;
        return out;
    }
    Context c;
    c.rows = &rows;
    c.cols = &cols;
    c.contentW = out.paper.content().width();
    c.contentH = out.paper.content().height();
    c.headings = setup.headings;
    c.titleRows = titlesOf(rows, setup.titles.hasRows, setup.titles.row1, setup.titles.row2);
    c.titleCols = titlesOf(cols, setup.titles.hasCols, setup.titles.col1, setup.titles.col2);

    // ---- the scale
    const bool fit = setup.fitToPage && (setup.fitToWidth > 0 || setup.fitToHeight > 0);
    if (fit) {
        auto fits = [&](int pct) {
            for (const CellRange &b : out.areas) {
                qint64 r = 0, k = 0;
                bandCounts(c, b, pct / 100.0, &r, &k);
                if ((setup.fitToHeight > 0 && r > setup.fitToHeight) || (setup.fitToWidth > 0 && k > setup.fitToWidth)) return false;
            }
            return true;
        };
        const int ceiling = std::clamp(request.fitCeiling, kMinPageScale, kMaxPageScale);
        out.fitted = true;
        if (fits(ceiling)) {
            percent = ceiling;
        } else if (!fits(kMinPageScale)) {
            percent = kMinPageScale;
            out.tooSmall = true;
        } else {
            int lo = kMinPageScale, hi = ceiling;   // fits at lo, not at hi
            while (hi - lo > 1) {
                const int mid = (lo + hi) / 2;
                if (fits(mid)) lo = mid;
                else hi = mid;
            }
            percent = lo;
        }
    }
    out.scalePercent = percent;
    const double scale = percent / 100.0;

    // ---- the pages
    const qint64 room = std::max(1, request.maxPages);
    int areaIndex = 0;
    for (const CellRange &b : out.areas) {
        const std::vector<qint64> rowBreaks = fit ? std::vector<qint64>() : breaksIn(setup.rowBreaks, b.row1, b.row2);
        const std::vector<qint64> colBreaks = fit ? std::vector<qint64>() : breaksIn(setup.colBreaks, b.col1, b.col2);
        std::vector<Band> rowBands, colBands;
        const qint64 nr = cutAxis(rows, b.row1, b.row2, capacityOf(c.contentH, scale, c.headings ? kColumnHeadingPixels : 0), c.titleRows, rowBreaks, room, &rowBands);
        const qint64 nc = cutAxis(cols, b.col1, b.col2, capacityOf(c.contentW, scale, c.headings ? kRowHeadingPixels : 0), c.titleCols, colBreaks, room, &colBands);
        out.pagesNeeded += nr * nc;
        auto add = [&](qint64 ri, qint64 ci) {
            if (qint64(out.pages.size()) >= room) {
                out.truncated = true;
                return false;
            }
            PageCut p;
            p.area = areaIndex;
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
        };
        bool more = true;
        if (setup.pageOrder == PageOrder::OverThenDown) {
            for (qint64 ri = 0; more && ri < qint64(rowBands.size()); ++ri)
                for (qint64 ci = 0; more && ci < qint64(colBands.size()); ++ci) more = add(ri, ci);
        } else {
            for (qint64 ci = 0; more && ci < qint64(colBands.size()); ++ci)
                for (qint64 ri = 0; more && ri < qint64(rowBands.size()); ++ri) more = add(ri, ci);
        }
        if (nr * nc > 0 && nr * nc > room && qint64(out.pages.size()) >= room) out.truncated = true;
        ++areaIndex;
    }
    return out;
}

} // namespace jp::grid
