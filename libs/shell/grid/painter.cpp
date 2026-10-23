#include "grid/painter.h"

#include "sheet/cellref.h"

#include <QImage>
#include <QPixmap>

#include <algorithm>
#include <cmath>
#include <memory>

namespace jp::grid {

using jp::office::sheet::CellType;
using jp::office::sheet::columnName;

namespace {

constexpr int kMaxScanSteps = 8192;     // rows or columns stepped over to find those that have size (rows too small to show have none)

// How heavy a line is where two cells' lines meet: the heavier is drawn.
int weight(BorderStyle s)
{
    switch (s) {
    case BorderStyle::None: return 0;
    case BorderStyle::Hair: return 1;
    case BorderStyle::Dotted: return 2;
    case BorderStyle::DashDotDot: return 3;
    case BorderStyle::DashDot: return 4;
    case BorderStyle::Dashed: return 5;
    case BorderStyle::Thin: return 6;
    case BorderStyle::MediumDashDotDot: return 7;
    case BorderStyle::SlantDashDot: return 8;
    case BorderStyle::MediumDashDot: return 9;
    case BorderStyle::MediumDashed: return 10;
    case BorderStyle::Medium: return 11;
    case BorderStyle::Double: return 12;
    case BorderStyle::Thick: return 13;
    }
    return 0;
}

const LineLook kNoLine;

const LineLook &heavier(const LineLook *a, const LineLook *b)
{
    const LineLook &x = a ? *a : kNoLine, &y = b ? *b : kNoLine;
    return weight(y.style) > weight(x.style) ? y : x;
}

// A pattern's 8 by 8 tile: which pixels are the foreground.
bool inPattern(PatternKind kind, int x, int y)
{
    switch (kind) {
    case PatternKind::MediumGray: return ((x + y) & 1) == 0;
    case PatternKind::DarkGray: return !((x & 1) && (y & 1));
    case PatternKind::LightGray: return (x & 1) == 0 && (y & 1) == 0;
    case PatternKind::Gray125: return (x & 3) == 0 && (y & 1) == 0;
    case PatternKind::Gray0625: return (x & 3) == 0 && (y & 3) == 0;
    case PatternKind::DarkHorizontal: return (y & 3) < 2;
    case PatternKind::DarkVertical: return (x & 3) < 2;
    case PatternKind::DarkDown: return ((x - y + 8) & 3) < 2;
    case PatternKind::DarkUp: return ((x + y) & 3) < 2;
    case PatternKind::DarkGrid: return (x & 3) < 2 || (y & 3) < 2;
    case PatternKind::DarkTrellis: return ((x + y) & 3) < 2 || ((x - y + 8) & 3) < 2;
    case PatternKind::LightHorizontal: return (y & 3) == 0;
    case PatternKind::LightVertical: return (x & 3) == 0;
    case PatternKind::LightDown: return ((x - y + 8) & 3) == 0;
    case PatternKind::LightUp: return ((x + y) & 3) == 0;
    case PatternKind::LightGrid: return (x & 3) == 0 || (y & 3) == 0;
    case PatternKind::LightTrellis: return ((x + y) & 3) == 0 || ((x - y + 8) & 3) == 0;
    case PatternKind::None:
    case PatternKind::Solid: return true;
    }
    return true;
}

} // namespace

// ---- measuring text, for the model

class GridPainter::QtMeter : public TextMeter {
public:
    double width(const QString &text, const CellLook &look) const override
    {
        if (text.isEmpty()) return 0;
        const size_t id = look.fontId;
        if (id >= m_entries.size()) m_entries.resize(id + 1);
        std::unique_ptr<Entry> &e = m_entries[id];
        if (!e) {
            QFont f(look.family.isEmpty() ? QFont().family() : look.family);
            f.setPixelSize(std::max(1, int(std::lround(look.pointSize * 96.0 / 72.0 * (look.vertAlign ? 0.65 : 1.0)))));
            f.setBold(look.bold);
            f.setItalic(look.italic);
            f.setHintingPreference(QFont::PreferNoHinting);
            e = std::make_unique<Entry>(f);
        }
        return e->metrics.horizontalAdvance(text);
    }
    void reset() { m_entries.clear(); }

private:
    struct Entry {
        explicit Entry(const QFont &f) : font(f), metrics(font) {}
        QFont font;
        QFontMetricsF metrics;
    };
    mutable std::vector<std::unique_ptr<Entry>> m_entries;
};

GridPainter::GridPainter(GridSheet *sheet) : m_sheet(sheet), m_meter(new QtMeter)
{
    m_sheet->setMeter(m_meter);
}

GridPainter::~GridPainter() { delete m_meter; }

const TextMeter &GridPainter::meter() const { return *m_meter; }

void GridPainter::resetFonts()
{
    m_fontMade.assign(m_fontMade.size(), false);
    m_fontZoom = 0;
    m_meter->reset();
    m_sheet->invalidateTexts();
}

const QFont &GridPainter::fontFor(const CellLook &look, double scale)
{
    const size_t id = look.fontId;
    if (id >= m_fonts.size()) {
        m_fonts.resize(id + 1);
        m_fontMade.resize(id + 1, false);
    }
    if (!m_fontMade[id]) {
        QFont f(look.family.isEmpty() ? QFont().family() : look.family);
        f.setPixelSize(std::max(1, int(std::lround(look.pointSize * 96.0 / 72.0 * m_sheet->layout().zoom() * (look.vertAlign ? 0.65 : 1.0)))));
        f.setBold(look.bold);
        f.setItalic(look.italic);
        f.setStrikeOut(look.strike);
        f.setUnderline(look.underline != Underline::None);
        f.setStyleStrategy(QFont::PreferAntialias);
        f.setHintingPreference(QFont::PreferNoHinting);
        m_fonts[id] = f;
        m_fontMade[id] = true;
    }
    Q_UNUSED(scale);
    return m_fonts[id];
}

QBrush GridPainter::patternBrush(PatternKind kind, QRgb fg, QRgb bg)
{
    const quint64 key = (quint64(kind) << 56) ^ (quint64(fg & 0xFFFFFF) << 28) ^ quint64(bg & 0xFFFFFF);
    const auto it = m_patterns.constFind(key);
    if (it != m_patterns.constEnd()) return it.value();
    QImage tile(8, 8, QImage::Format_RGB32);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) tile.setPixel(x, y, inPattern(kind, x, y) ? (fg | 0xFF000000u) : (bg | 0xFF000000u));
    if (m_patterns.size() > 512) m_patterns.clear();
    const QBrush brush(QPixmap::fromImage(tile));
    m_patterns.insert(key, brush);
    return brush;
}

// ---- drawing a cell's parts

void GridPainter::drawFill(QPainter *p, const QRect &rect, const CellLook &look, const PaintOptions &)
{
    if (look.pattern == PatternKind::None) return;
    ++m_stats.fills;
    if (look.pattern == PatternKind::Solid) {
        p->fillRect(rect, QColor::fromRgba(look.fg));
        return;
    }
    // (the pattern begins at the cell's corner, so it is the same wherever the cell is on the screen)
    p->setBrushOrigin(rect.topLeft());
    p->fillRect(rect, patternBrush(look.pattern, look.fg, look.bg));
}

// A border on the edge that begins at (x, y) and goes `length` pixels, down (vertical) or right. A line's own pixels lie on the side of the edge before it (the last pixel of
// the cell above or to the left, where a gridline is), thicker ones reaching over it.
void GridPainter::drawEdge(QPainter *p, const LineLook &line, int x, int y, int length, bool vertical)
{
    if (line.style == BorderStyle::None || length <= 0) return;
    ++m_stats.borderSegments;
    const QColor color = QColor::fromRgba(line.color);
    int thickness = 1;
    int first = -1;      // where its first pixel is, from the edge
    QList<qreal> dashes;
    bool two = false;
    switch (line.style) {
    case BorderStyle::Thin: break;
    case BorderStyle::Hair: dashes = {1, 1}; break;
    case BorderStyle::Dotted: dashes = {1, 1}; break;
    case BorderStyle::Dashed: dashes = {3, 1}; break;
    case BorderStyle::DashDot: dashes = {3, 1, 1, 1}; break;
    case BorderStyle::DashDotDot: dashes = {3, 1, 1, 1, 1, 1}; break;
    case BorderStyle::Medium: thickness = 2; break;
    case BorderStyle::MediumDashed: thickness = 2; dashes = {3, 1}; break;
    case BorderStyle::MediumDashDot: thickness = 2; dashes = {3, 1, 1, 1}; break;
    case BorderStyle::MediumDashDotDot: thickness = 2; dashes = {3, 1, 1, 1, 1, 1}; break;
    case BorderStyle::SlantDashDot: thickness = 2; dashes = {3, 1, 1, 1}; break;
    case BorderStyle::Thick: thickness = 3; first = -2; break;
    case BorderStyle::Double: two = true; thickness = 1; break;
    case BorderStyle::None: return;
    }
    auto rect = [&](int offset, int width) { return vertical ? QRect(x + offset, y, width, length) : QRect(x, y + offset, length, width); };
    if (two) {
        p->fillRect(rect(-2, 1), color);
        p->fillRect(rect(0, 1), color);
        return;
    }
    if (thickness == 2) first = -1;
    if (dashes.isEmpty()) {
        p->fillRect(rect(first, thickness), color);
        return;
    }
    // a dashed line: dashes of the pattern's lengths along the edge
    int at = 0;
    size_t step = 0;
    while (at < length) {
        const int run = int(dashes[int(step % size_t(dashes.size()))]);
        if (step % 2 == 0) {
            const int len = std::min(run, length - at);
            p->fillRect(vertical ? QRect(x + first, y + at, thickness, len) : QRect(x + at, y + first, len, thickness), color);
        }
        at += run;
        ++step;
    }
}

void GridPainter::drawText(QPainter *p, const QRect &cell, const QRect &box, const QRect &area, const CellLook &look, const Shown &sh, const PaintOptions &)
{
    const GridLayout &L = m_sheet->layout();
    const double z = L.zoom();
    QFont font = fontFor(look, z);
    if (font.pixelSize() < kMinFontPixels) return;
    const int pad = std::max(1, int(std::lround(kCellPadding * z))), indent = int(std::lround(look.indent * 9 * z));
    const bool left = sh.align != HAlign::Right && sh.align != HAlign::Center && sh.align != HAlign::CenterContinuous, right = sh.align == HAlign::Right;
    QRect r = box.adjusted(pad + (left ? indent : 0), 1, -(pad + (right ? indent : 0)), -1);
    if (r.width() <= 0 || r.height() <= 0) return;
    int flags = left ? Qt::AlignLeft : right ? Qt::AlignRight : Qt::AlignHCenter;
    flags |= look.vertical == VAlign::Top || look.vertical == VAlign::Justify || look.vertical == VAlign::Distributed ? Qt::AlignTop : look.vertical == VAlign::Center ? Qt::AlignVCenter : Qt::AlignBottom;
    QString text = sh.text;
    const double deviceWidth = sh.width * z;
    bool clip = deviceWidth > r.width() || r.height() < font.pixelSize() + 2;
    if (look.shrink && !look.wrap && deviceWidth > r.width() && deviceWidth > 0) {
        font.setPixelSize(std::max(1, int(std::floor(font.pixelSize() * r.width() / deviceWidth))));
        if (font.pixelSize() < kMinFontPixels) return;
        clip = true;
    }
    if (look.wrap) {
        flags |= Qt::TextWordWrap;
        clip = true;
    } else {
        flags |= Qt::TextSingleLine;
    }
    // "*x" in a number's format: the text is spread with x to fill the cell
    if (sh.kind == Kind::Number && sh.fillAt >= 0 && sh.fillAt <= text.size() && !sh.hashes) {
        const double charWidth = std::max(1.0, m_meter->width(QString(sh.fillChar), look) * z);
        const int n = int((r.width() - deviceWidth) / charWidth);
        if (n > 0) text = text.left(sh.fillAt) + QString(n, sh.fillChar) + text.mid(sh.fillAt);
    }
    p->setFont(font);
    p->setPen(QColor::fromRgba(sh.color ? sh.color : look.textColor));
    ++m_stats.textRuns;
    m_stats.textChars += text.size();
    if (sh.hashes) ++m_stats.hashed;
    if (look.rotation != 0 && look.rotation != 255) {
        // turned: about the middle of the cell, in a box turned with it
        const int turn = look.rotation <= 90 ? look.rotation : -(look.rotation - 90);
        p->save();
        p->setClipRect(cell & area);
        p->translate(QPointF(cell.center()) + QPointF(0.5, 0.5));
        p->rotate(-turn);
        const bool sideways = std::abs(turn) > 45;
        const QRect turned(-(sideways ? cell.height() : cell.width()) / 2, -(sideways ? cell.width() : cell.height()) / 2, sideways ? cell.height() : cell.width(),
                           sideways ? cell.width() : cell.height());
        p->drawText(turned, Qt::AlignCenter | Qt::TextSingleLine | Qt::TextDontClip, text);
        p->restore();
        return;
    }
    if (look.rotation == 255) {
        // upright, one letter under another
        QString stacked;
        for (QChar c : text) stacked += (stacked.isEmpty() ? QString() : QStringLiteral("\n")) + c;
        p->setClipRect(cell & area);
        p->drawText(cell, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip, stacked);
        p->setClipRect(area);
        return;
    }
    if (clip) p->setClipRect(box & area);
    p->drawText(r, flags | (clip ? 0 : Qt::TextDontClip), text);
    if (clip) p->setClipRect(area);
}

// ---- the parts of the grid

struct GridPainter::PaneWork {
    std::vector<Span> rows, cols;
    std::vector<const Cell *> cells;           // rows by columns, those beside the ones drawn included
    std::vector<const CellLook *> looks;       // the same, null for the plain look
    std::vector<char> covered;                 // under a merged range
    std::vector<const Cell *> leftCandidate, rightCandidate;   // by row: the nearest cell with something in it outside the columns drawn, on each side
    size_t C = 0;
};

void GridPainter::paintPane(QPainter *p, Pane which, const QRect &clip, const PaintOptions &o)
{
    const GridLayout &L = m_sheet->layout();
    const PaneView pv = L.pane(which);
    const QRect area = pv.rect & clip;
    if (area.isEmpty()) return;
    ++m_stats.panes;
    p->setClipRect(area);
    p->fillRect(area, o.backdrop);
    if (pv.isEmpty()) return;
    const bool frozenRows = which == Pane::Corner || which == Pane::Top, frozenCols = which == Pane::Corner || which == Pane::Left;
    const Axis &rowsAxis = L.rows, &colsAxis = L.cols;
    PaneWork w;
    // ---- the rows and columns drawn: those that have pixels and are in the clip, with the one beside each end for the borders they share
    auto collect = [&](const Axis &axis, qint64 first, qint64 last, bool frozen, bool vertical, int lo, int hi, int most, std::vector<Span> *out) {
        int steps = 0;
        for (qint64 i = first; i <= last && i < axis.count(); i = axis.nextVisible(i)) {
            if (++steps > kMaxScanSteps) {
                m_stats.truncated = true;
                break;
            }
            const int a = vertical ? L.yEdge(i, frozen) : L.xEdge(i, frozen), b = vertical ? L.yEdge(i + 1, frozen) : L.xEdge(i + 1, frozen);
            if (b <= lo) continue;
            if (a > hi) break;
            if (b <= a) continue;
            if (int(out->size()) >= most) {
                m_stats.truncated = true;
                break;
            }
            out->push_back({i, a, b - a, false});
        }
    };
    collect(colsAxis, pv.firstCol, pv.lastCol, frozenCols, false, area.left(), area.right(), kMaxCols, &w.cols);
    const int colsCount = int(std::max<size_t>(1, w.cols.size()));
    collect(rowsAxis, pv.firstRow, pv.lastRow, frozenRows, true, area.top(), area.bottom(), int(std::min<qint64>(kMaxRows, std::max<qint64>(1, kMaxCells / colsCount))), &w.rows);
    if (w.rows.empty() || w.cols.empty()) return;
    m_stats.rowsVisited += qint64(w.rows.size());
    m_stats.colsVisited += qint64(w.cols.size());
    const Span firstRow = w.rows.front(), lastRow = w.rows.back(), firstCol = w.cols.front(), lastCol = w.cols.back();
    auto ring = [](qint64 index) { return Span{index, 0, 0, true}; };
    const qint64 rowBefore = rowsAxis.previousVisible(firstRow.index), rowAfter = rowsAxis.nextVisible(lastRow.index);
    const qint64 colBefore = colsAxis.previousVisible(firstCol.index), colAfter = colsAxis.nextVisible(lastCol.index);
    if (rowBefore >= 0) w.rows.insert(w.rows.begin(), ring(rowBefore));
    if (rowAfter < rowsAxis.count()) w.rows.push_back(ring(rowAfter));
    if (colBefore >= 0) w.cols.insert(w.cols.begin(), ring(colBefore));
    if (colAfter < colsAxis.count()) w.cols.push_back(ring(colAfter));
    const size_t R = w.rows.size(), C = w.cols.size();
    w.C = C;
    const size_t r0 = rowBefore >= 0 ? 1 : 0, r1 = R - (rowAfter < rowsAxis.count() ? 1 : 0);   // the rows drawn: [r0, r1)
    const size_t c0 = colBefore >= 0 ? 1 : 0, c1 = C - (colAfter < colsAxis.count() ? 1 : 0);
    // ---- the paper under the cells, and the gridlines on it
    p->fillRect(QRect(firstCol.start, firstRow.start, lastCol.start + lastCol.size - firstCol.start, lastRow.start + lastRow.size - firstRow.start), o.paper);
    if (o.gridlines) {
        const int x0 = firstCol.start, x1 = lastCol.start + lastCol.size, y0 = firstRow.start, y1 = lastRow.start + lastRow.size;
        for (size_t i = r0; i < r1; ++i) {
            p->fillRect(QRect(x0, w.rows[i].start + w.rows[i].size - 1, x1 - x0, 1), o.gridColor);
            ++m_stats.gridLines;
        }
        for (size_t j = c0; j < c1; ++j) {
            p->fillRect(QRect(w.cols[j].start + w.cols[j].size - 1, y0, 1, y1 - y0), o.gridColor);
            ++m_stats.gridLines;
        }
    }
    // ---- the cells: found a row at a time, with those on either side that text may spill from
    const bool scrollingCols = !frozenCols;
    w.cells.assign(R * C, nullptr);
    w.leftCandidate.assign(R, nullptr);
    w.rightCandidate.assign(R, nullptr);
    const qint64 marginLeft = scrollingCols ? kMaxSpill : 0, marginRight = scrollingCols ? kMaxSpill : 0;
    const bool wide = m_sheet->hasImplicitStyles();
    for (size_t i = 0; i < R; ++i) {
        const qint64 row = w.rows[i].index;
        const bool inMargin = scrollingCols && m_stats.cellsLooked < kMaxCells * 3;
        const qint64 lo = inMargin ? std::max<qint64>(0, w.cols.front().index - marginLeft) : w.cols.front().index;
        const qint64 hi = inMargin ? std::min<qint64>(colsAxis.count() - 1, w.cols.back().index + marginRight) : w.cols.back().index;
        const qint64 leftEdge = w.cols[c0].index, rightEdge = w.cols[c1 - 1].index;
        m_sheet->data().sheet->cells.forEachInRange(quint32(row), quint32(lo), quint32(row), quint32(hi), [&](const Cell &cell) {
            ++m_stats.cellsLooked;
            const qint64 col = cell.col;
            const auto it = std::lower_bound(w.cols.begin(), w.cols.end(), col, [](const Span &s, qint64 v) { return s.index < v; });
            if (it != w.cols.end() && it->index == col) w.cells[i * C + size_t(it - w.cols.begin())] = &cell;
            const bool holds = cell.type != CellType::Empty || cell.hasFormula();
            if (holds && scrollingCols) {
                if (col < leftEdge && !colsAxis.isHidden(col)) w.leftCandidate[i] = &cell;                       // (the last one met is the nearest)
                else if (col > rightEdge && !w.rightCandidate[i] && !colsAxis.isHidden(col)) w.rightCandidate[i] = &cell;
            }
            return true;
        });
    }
    // ---- merged ranges that cross what is drawn, and the cells under them
    std::vector<int> merges;
    if (L.merges.count() > 0) {
        if (!L.merges.crossing(quint32(firstRow.index), quint32(firstCol.index), quint32(lastRow.index), quint32(lastCol.index), kMaxMerges, &merges)) m_stats.truncated = true;
        w.covered.assign(R * C, 0);
        for (int id : merges) {
            const jp::office::sheet::CellRange &m = L.merges.range(id);
            const auto rb = std::lower_bound(w.rows.begin() + std::ptrdiff_t(r0), w.rows.begin() + std::ptrdiff_t(r1), qint64(m.row1), [](const Span &s, qint64 v) { return s.index < v; });
            const auto cb = std::lower_bound(w.cols.begin() + std::ptrdiff_t(c0), w.cols.begin() + std::ptrdiff_t(c1), qint64(m.col1), [](const Span &s, qint64 v) { return s.index < v; });
            for (auto ri = rb; ri != w.rows.begin() + std::ptrdiff_t(r1) && ri->index <= m.row2; ++ri)
                for (auto ci = cb; ci != w.cols.begin() + std::ptrdiff_t(c1) && ci->index <= m.col2; ++ci) {
                    if (m_stats.mergeMarks >= kMaxMergeMarks) {
                        m_stats.truncated = true;
                        goto marked;
                    }
                    ++m_stats.mergeMarks;
                    w.covered[size_t(ri - w.rows.begin()) * C + size_t(ci - w.cols.begin())] = 1;
                }
        }
    }
marked:
    // ---- the looks, by cell (a cell with the first format, and no cell, has the plain look unless its row or column has a format)
    w.looks.assign(R * C, nullptr);
    bool anyFill = false, anyBorder = false;
    const LookTable &looks = m_sheet->looks();
    for (size_t i = 0; i < R; ++i)
        for (size_t j = 0; j < C; ++j) {
            const Cell *c = w.cells[i * C + j];
            quint32 xf = c ? c->style : 0;
            if (!c && wide && !w.rows[i].ring && !w.cols[j].ring) xf = m_sheet->styleOf(quint32(w.rows[i].index), quint32(w.cols[j].index), nullptr);
            if (xf == 0) continue;
            const CellLook &look = looks.look(xf);
            w.looks[i * C + j] = &look;
            anyFill = anyFill || look.hasFill();
            anyBorder = anyBorder || look.hasBorder();
        }
    // ---- each row: fills, borders, text
    qint64 textBudget = kMaxTextRuns - m_stats.textRuns;
    auto spillOfCell = [&](const Cell &cell, const CellLook &look, const Shown &sh, qint64 row) {
        const double width = colsAxis.size(cell.col);
        const bool centerContinuous = sh.align == HAlign::CenterContinuous;
        return spillOf(sh, look, width, [&](int step) -> double {
            const qint64 col = qint64(cell.col) + step;
            if (col < 0 || col >= colsAxis.count()) return -1;
            if ((col < L.frozenCols()) != (qint64(cell.col) < L.frozenCols())) return -1;   // (not across the edge of the frozen columns)
            const double w2 = colsAxis.size(col);
            if (w2 == 0) return 0;
            if (++m_stats.spillProbes > kMaxSpillProbes) return -1;
            const Cell *other = m_sheet->cellAt(quint32(row), quint32(col));
            if (other && (other->type != CellType::Empty || other->hasFormula())) return -1;
            if (L.merges.count() > 0 && L.merges.at(quint32(row), quint32(col)) >= 0) return -1;
            if (centerContinuous) {
                const quint32 xf = other ? other->style : m_sheet->styleOf(quint32(row), quint32(col), nullptr);
                if (looks.look(xf).horizontal != HAlign::CenterContinuous) return -1;
            }
            return w2;
        });
    };
    for (size_t i = r0; i < r1; ++i) {
        const Span &rs = w.rows[i];
        // fills
        if (anyFill)
            for (size_t j = c0; j < c1; ++j) {
                const CellLook *look = w.looks[i * C + j];
                if (!look || !look->hasFill() || (!w.covered.empty() && w.covered[i * C + j])) continue;
                drawFill(p, QRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size), *look, o);
            }
        // borders: the lines between the cells of this row, and above it
        if (anyBorder) {
            for (size_t k = c0; k <= c1; ++k) {
                if (!w.covered.empty() && ((k > 0 && k - 1 >= c0 && w.covered[i * C + k - 1] && k < C && w.covered[i * C + k]) )) continue;   // inside a merged range
                const CellLook *a = k > 0 ? w.looks[i * C + k - 1] : nullptr, *b = k < C ? w.looks[i * C + k] : nullptr;
                const LineLook &line = heavier(a ? &a->right : nullptr, b ? &b->left : nullptr);
                const int x = k < c1 ? w.cols[k].start : w.cols[c1 - 1].start + w.cols[c1 - 1].size;
                drawEdge(p, line, x, rs.start, rs.size, true);
            }
            for (size_t j = c0; j < c1; ++j) {
                if (!w.covered.empty() && w.covered[i * C + j] && i > 0 && w.covered[(i - 1) * C + j]) continue;
                const CellLook *above = i > 0 ? w.looks[(i - 1) * C + j] : nullptr, *here = w.looks[i * C + j];
                drawEdge(p, heavier(above ? &above->bottom : nullptr, here ? &here->top : nullptr), w.cols[j].start, rs.start, w.cols[j].size, false);
                if (here && here->diagonal.style != BorderStyle::None && (here->diagonalUp || here->diagonalDown)) {
                    p->save();
                    p->setClipRect(QRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size) & area);
                    p->setPen(QColor::fromRgba(here->diagonal.color));
                    if (here->diagonalUp) p->drawLine(w.cols[j].start, rs.start + rs.size - 1, w.cols[j].start + w.cols[j].size - 1, rs.start);
                    if (here->diagonalDown) p->drawLine(w.cols[j].start, rs.start, w.cols[j].start + w.cols[j].size - 1, rs.start + rs.size - 1);
                    p->restore();
                    ++m_stats.borderSegments;
                }
            }
        }
        // the text of this row; then that of the cell beside the columns drawn whose text runs into them
        for (size_t j = c0; j < c1 && textBudget > 0; ++j) {
            const Cell *cell = w.cells[i * C + j];
            if (!cell || (!w.covered.empty() && w.covered[i * C + j])) continue;
            if (cell->type == CellType::Empty && !cell->hasFormula()) continue;
            const Shown sh = m_sheet->shown(cell->row, cell->col, *cell);
            if (sh.kind == Kind::Empty && !sh.hashes) continue;
            const CellLook &look = looks.look(cell->style);
            const Spill sp = spillOfCell(*cell, look, sh, rs.index);
            QRect box(w.cols[j].start, rs.start, w.cols[j].size, rs.size);
            if (sp.left + sp.right > 0) {
                const int x0 = L.xEdge(qint64(cell->col) - sp.left, frozenCols), x1 = L.xEdge(qint64(cell->col) + sp.right + 1, frozenCols);
                box = QRect(x0, rs.start, x1 - x0, rs.size);
            }
            drawText(p, QRect(w.cols[j].start, rs.start, w.cols[j].size, rs.size), box, area, look, sh, o);
            --textBudget;
        }
        if (textBudget <= 0 && !m_stats.truncated) m_stats.truncated = true;
        for (const Cell *cand : {w.leftCandidate[i], w.rightCandidate[i]}) {
            if (!cand || textBudget <= 0) continue;
            const Shown sh = m_sheet->shown(cand->row, cand->col, *cand);
            if (sh.kind != Kind::Text || sh.hashes) continue;
            const CellLook &look = looks.look(cand->style);
            const Spill sp = spillOfCell(*cand, look, sh, rs.index);
            if (sp.left + sp.right == 0) continue;
            const int cx0 = L.xEdge(cand->col, frozenCols), cx1 = L.xEdge(qint64(cand->col) + 1, frozenCols);
            const int x0 = L.xEdge(qint64(cand->col) - sp.left, frozenCols), x1 = L.xEdge(qint64(cand->col) + sp.right + 1, frozenCols);
            const QRect box(x0, rs.start, x1 - x0, rs.size);
            if (!box.intersects(area)) continue;
            drawText(p, QRect(cx0, rs.start, cx1 - cx0, rs.size), box, area, look, sh, o);
            --textBudget;
        }
    }
    // the edge below the last row
    if (anyBorder && r1 > r0)
        for (size_t j = c0; j < c1; ++j) {
            const CellLook *here = w.looks[(r1 - 1) * C + j], *below = r1 < R ? w.looks[r1 * C + j] : nullptr;
            const Span &rs = w.rows[r1 - 1];
            drawEdge(p, heavier(here ? &here->bottom : nullptr, below ? &below->top : nullptr), w.cols[j].start, rs.start + rs.size, w.cols[j].size, false);
        }
    // ---- merged ranges, each as one cell: its fill (over the gridlines inside it, but not those that bound it), its borders from the cells at its corners, and its first cell's text
    for (int id : merges) {
        const jp::office::sheet::CellRange &m = L.merges.range(id);
        const QRect rect = L.rangeRectIn(which, m.row1, m.col1, m.row2, m.col2);
        if (rect.isEmpty() || !rect.intersects(area)) continue;
        ++m_stats.mergesDrawn;
        const Cell *anchor = m_sheet->cellAt(m.row1, m.col1);
        const quint32 xf = anchor ? anchor->style : m_sheet->styleOf(m.row1, m.col1, nullptr);
        const CellLook &look = looks.look(xf);
        if (look.hasFill()) drawFill(p, rect, look, o);
        else p->fillRect(rect.adjusted(0, 0, o.gridlines ? -1 : 0, o.gridlines ? -1 : 0), o.paper);
        if (look.hasBorder() || xf == 0) {
            auto lookOf = [&](quint32 r, quint32 c) -> const CellLook * {
                const Cell *x = m_sheet->cellAt(r, c);
                return x && x->style ? &looks.look(x->style) : nullptr;
            };
            const CellLook *tl = lookOf(m.row1, m.col1), *tr = lookOf(m.row1, m.col2), *bl = lookOf(m.row2, m.col1), *br = lookOf(m.row2, m.col2);
            drawEdge(p, heavier(tl ? &tl->left : nullptr, bl ? &bl->left : nullptr), rect.left(), rect.top(), rect.height(), true);
            drawEdge(p, heavier(tr ? &tr->right : nullptr, br ? &br->right : nullptr), rect.right() + 1, rect.top(), rect.height(), true);
            drawEdge(p, heavier(tl ? &tl->top : nullptr, tr ? &tr->top : nullptr), rect.left(), rect.top(), rect.width(), false);
            drawEdge(p, heavier(bl ? &bl->bottom : nullptr, br ? &br->bottom : nullptr), rect.left(), rect.bottom() + 1, rect.width(), false);
        }
        if (anchor && textBudget > 0) {
            const Shown sh = m_sheet->shown(anchor->row, anchor->col, *anchor);
            if (sh.kind != Kind::Empty || sh.hashes) {
                drawText(p, rect, rect, area, look, sh, o);
                --textBudget;
            }
        }
    }
    paintSelection(p, which, area, o);
    p->setClipping(false);
}

void GridPainter::paintSelection(QPainter *p, Pane which, const QRect &area, const PaintOptions &o)
{
    if (!o.selection) return;
    const GridLayout &L = m_sheet->layout();
    const Selection &s = *o.selection;
    const QRect range = L.rangeRectIn(which, s.range.row1, s.range.col1, s.range.row2, s.range.col2);
    if (range.isEmpty() || !range.intersects(area)) return;
    QRect active = L.rangeRectIn(which, s.activeRow, s.activeCol, s.activeRow, s.activeCol);
    const int merge = L.merges.at(quint32(s.activeRow), quint32(s.activeCol));
    if (merge >= 0) {
        const jp::office::sheet::CellRange &m = L.merges.range(merge);
        active = L.rangeRectIn(which, m.row1, m.col1, m.row2, m.col2);
    }
    p->setClipRect(area);
    if (!(range == active)) {
        QColor tint = o.accent;
        tint.setAlpha(40);
        p->setClipRegion(QRegion(range & area).subtracted(active));
        p->fillRect(range, tint);
        p->setClipRect(area);
    }
    // the border, two pixels, inside the range's edge
    p->fillRect(QRect(range.left(), range.top(), range.width(), 2), o.accent);
    p->fillRect(QRect(range.left(), range.bottom() - 1, range.width(), 2), o.accent);
    p->fillRect(QRect(range.left(), range.top(), 2, range.height()), o.accent);
    p->fillRect(QRect(range.right() - 1, range.top(), 2, range.height()), o.accent);
}

void GridPainter::paintHeaders(QPainter *p, const QRect &clip, const PaintOptions &o)
{
    const GridLayout &L = m_sheet->layout();
    const int hw = L.headerWidth(), hh = L.headerHeight();
    if (hw == 0 || hh == 0) return;
    QFont font = o.headerFont;
    if (font.pixelSize() <= 0) font.setPixelSize(11);
    p->setFont(font);
    const Selection *sel = o.selection;
    auto colSelected = [&](qint64 c) { return sel && c >= sel->range.col1 && c <= sel->range.col2; };
    auto rowSelected = [&](qint64 r) { return sel && r >= sel->range.row1 && r <= sel->range.row2; };
    // the corner
    const QRect corner(0, 0, hw, hh);
    if (corner.intersects(clip)) {
        p->setClipRect(corner & clip);
        p->fillRect(corner, o.headerBackground);
        p->fillRect(QRect(hw - 1, 0, 1, hh), o.headerLine);
        p->fillRect(QRect(0, hh - 1, hw, 1), o.headerLine);
        ++m_stats.headersDrawn;
    }
    for (Pane which : {Pane::Left, Pane::Body}) {
        // the letters: along the top, over the columns of this part
        const PaneView pv = L.pane(which);
        const bool frozen = which == Pane::Left;
        const QRect band(pv.rect.left(), 0, pv.rect.width(), hh);
        if (band.width() > 0 && band.intersects(clip)) {
            const QRect area = band & clip;
            p->setClipRect(area);
            p->fillRect(area, o.headerBackground);
            int steps = 0, drawn = 0;
            for (qint64 c = pv.firstCol; !pv.isEmpty() && c <= pv.lastCol && c < L.cols.count() && drawn < kMaxCols && ++steps < kMaxScanSteps; c = L.cols.nextVisible(c)) {
                const int a = L.xEdge(c, frozen), b = L.xEdge(c + 1, frozen);
                if (b <= area.left()) continue;
                if (a > area.right()) break;
                if (b <= a) continue;
                const QRect cell(a, 0, b - a, hh);
                p->fillRect(cell, colSelected(c) ? o.headerSelected : o.headerBackground);
                p->fillRect(QRect(b - 1, 0, 1, hh), o.headerLine);
                p->fillRect(QRect(a, hh - 1, b - a, 1), o.headerLine);
                if (cell.width() >= 12) {
                    p->setPen(o.headerText);
                    p->drawText(cell, Qt::AlignCenter | Qt::TextSingleLine, columnName(quint32(c)));
                }
                ++m_stats.headersDrawn;
                ++drawn;
            }
        }
    }
    for (Pane which : {Pane::Top, Pane::Body}) {
        // the numbers: down the side, beside the rows of this part
        const PaneView pv = L.pane(which);
        const bool frozen = which == Pane::Top;
        const QRect band(0, pv.rect.top(), hw, pv.rect.height());
        if (band.height() > 0 && band.intersects(clip)) {
            const QRect area = band & clip;
            p->setClipRect(area);
            p->fillRect(area, o.headerBackground);
            int steps = 0, drawn = 0;
            for (qint64 r = pv.firstRow; !pv.isEmpty() && r <= pv.lastRow && r < L.rows.count() && drawn < kMaxRows && ++steps < kMaxScanSteps; r = L.rows.nextVisible(r)) {
                const int a = L.yEdge(r, frozen), b = L.yEdge(r + 1, frozen);
                if (b <= area.top()) continue;
                if (a > area.bottom()) break;
                if (b <= a) continue;
                const QRect cell(0, a, hw, b - a);
                p->fillRect(cell, rowSelected(r) ? o.headerSelected : o.headerBackground);
                p->fillRect(QRect(hw - 1, a, 1, b - a), o.headerLine);
                p->fillRect(QRect(0, b - 1, hw, 1), o.headerLine);
                if (cell.height() >= 10) {
                    p->setPen(o.headerText);
                    p->drawText(cell, Qt::AlignCenter | Qt::TextSingleLine, QString::number(r + 1));
                }
                ++m_stats.headersDrawn;
                ++drawn;
            }
        }
    }
    p->setClipping(false);
}

void GridPainter::paint(QPainter *p, const QRect &clip, const PaintOptions &o)
{
    m_stats = PaintStats();
    const qint64 textsBefore = m_sheet->textsMade(), looksBefore = m_sheet->looks().made();
    m_sheet->looks().trim();
    const double zoom = m_sheet->layout().zoom();
    if (zoom != m_fontZoom) {
        m_fontMade.assign(m_fontMade.size(), false);
        m_fontZoom = zoom;
    }
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);
    p->setRenderHint(QPainter::SmoothPixmapTransform, false);
    for (Pane which : {Pane::Body, Pane::Left, Pane::Top, Pane::Corner}) paintPane(p, which, clip, o);
    if (o.headers) paintHeaders(p, clip, o);
    p->restore();
    m_stats.textsMade = m_sheet->textsMade() - textsBefore;
    m_stats.looksMade = m_sheet->looks().made() - looksBefore;
}

} // namespace jp::grid
