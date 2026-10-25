#include "grid/gridview.h"

#include "app/icons.h"
#include "core/threadtime.h"
#include "grid/textwalk.h"
#include "sheet/cellref.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

#include <algorithm>
#include <climits>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::cellName;
using jp::office::sheet::columnName;

class GridView::Nav : public NavSource {
public:
    explicit Nav(const GridSheet *sheet) : m_sheet(sheet) {}
    bool hasValue(qint64 row, qint64 col) const override
    {
        const Cell *c = m_sheet->cellAt(quint32(row), quint32(col));
        return c && (c->type != jp::office::sheet::CellType::Empty || c->hasFormula());
    }
    void usedExtent(qint64 *rows, qint64 *cols) const override
    {
        quint32 r = 0, c = 0;
        m_sheet->usedExtent(&r, &c);
        *rows = r;
        *cols = c;
    }

private:
    const GridSheet *m_sheet;
};

GridView::GridView(QWidget *parent) : QAbstractScrollArea(parent)
{
    setObjectName(QStringLiteral("gridView"));
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFrameShape(QFrame::NoFrame);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewport()->setBackgroundRole(QPalette::Window);
    m_painter = std::make_unique<GridPainter>(&m_sheet);
    m_nav = std::make_unique<Nav>(&m_sheet);
    m_conditionalTimer.setSingleShot(true);
    connect(&m_conditionalTimer, &QTimer::timeout, this, [this] { conditionalWork(); });
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        if (!m_syncingBars) scrollTo(horizontalScrollBar()->value(), m_sheet.layout().scrollY());
    });
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
        if (!m_syncingBars) scrollTo(m_sheet.layout().scrollX(), verticalScrollBar()->value());
    });
    m_autoScroll.setInterval(50);
    connect(&m_autoScroll, &QTimer::timeout, this, [this] { dragTo(m_lastMouse); });
    viewport()->setMouseTracking(true);
    // the dashes round copied cells move
    m_copyTimer.setInterval(110);
    connect(&m_copyTimer, &QTimer::timeout, this, [this] {
        m_copyPhase = (m_copyPhase + 1) % 7;
        viewport()->update();
    });
    m_walkTimer.setSingleShot(true);
    connect(&m_walkTimer, &QTimer::timeout, this, [this] {
        if (m_jump) stepJump();
        if (m_statsWalk) stepStats();
    });
    layoutChanged();
}

GridView::~GridView() = default;

// ---- the data, the view, and the scroll

GridView::ViewState GridView::viewState() const
{
    ViewState v;
    v.selection = m_selection;
    v.scrollX = m_sheet.layout().scrollX();
    v.scrollY = m_sheet.layout().scrollY();
    v.frozenRows = m_sheet.layout().frozenRows();
    v.frozenCols = m_sheet.layout().frozenCols();
    v.skipRows = m_sheet.layout().skippedRows();
    v.skipCols = m_sheet.layout().skippedCols();
    v.zoom = zoom();
    v.gridlines = m_gridlines;
    v.headings = m_headings;
    v.valid = true;
    return v;
}

void GridView::setViewState(const ViewState &v)
{
    if (!v.valid) return;
    GridLayout &L = m_sheet.layout();
    m_gridlines = v.gridlines;
    m_headings = v.headings;
    L.setZoom(v.zoom);
    L.setSkipped(v.skipRows, v.skipCols);
    L.setFrozen(v.frozenRows, v.frozenCols);
    L.setScroll(v.scrollX, v.scrollY);
    // (clamped to the sheet it is now: a sheet read whole may have fewer rows than the one the state was made for, never more than a sheet can have)
    m_selection = v.selection;
    m_selection.activeRow = std::min(m_selection.activeRow, L.rows.count() - 1);
    m_selection.activeCol = std::min(m_selection.activeCol, L.cols.count() - 1);
    m_selection.headRow = std::min(m_selection.headRow, L.rows.count() - 1);
    m_selection.headCol = std::min(m_selection.headCol, L.cols.count() - 1);
    m_selection.range.row2 = std::min(m_selection.range.row2, L.rows.count() - 1);
    m_selection.range.col2 = std::min(m_selection.range.col2, L.cols.count() - 1);
    m_painter->resetFonts();
    layoutChanged();
    Q_EMIT zoomChanged(zoom());
    Q_EMIT selectionChanged();
    startStats();
}

void GridView::setData(const GridData &data, bool keepView)
{
    const ViewState kept = viewState();
    m_jump.reset();
    m_statsWalk.reset();
    m_sheet.setData(data);
    m_painter->resetFonts();
    if (data.sheet) {
        m_gridlines = data.sheet->view.showGridLines;
        m_headings = data.sheet->view.showHeadings;
    }
    m_sheet.layout().setViewport(viewport()->size());
    if (keepView) {
        setViewState(kept);
        return;
    }
    Navigator nav(&m_sheet.layout(), m_nav.get());
    const jp::office::sheet::SheetView v = data.sheet ? data.sheet->view : jp::office::sheet::SheetView();
    m_selection = nav.click(qint64(v.activeRow), qint64(v.activeCol));
    layoutChanged();
    Q_EMIT zoomChanged(zoom());
    Q_EMIT selectionChanged();
    startStats();
}

void GridView::layoutChanged()
{
    updateHeaderWidth();
    m_sheet.layout().setViewport(viewport()->size());
    updateScrollBars();
    if (m_overlay) m_overlay->layoutChanged();
    viewport()->update();
    Q_EMIT viewChanged();
}

void GridView::updateHeaderWidth()
{
    GridLayout &L = m_sheet.layout();
    int digits = 3;
    if (m_headings) {
        const PaneView pv = L.pane(Pane::Body);
        const qint64 last = pv.isEmpty() ? L.rows.count() : std::max(pv.lastRow, L.frozenRows());
        digits = std::max(3, int(QString::number(std::min(L.rows.count(), last + 1)).size()));
    }
    if (digits != m_headerDigits || (L.headerWidth() > 0) != m_headings) {
        m_headerDigits = digits;
        L.setHeaders(m_headings, digits * 8 + 14, 20);
    }
}

void GridView::updateScrollBars()
{
    const GridLayout &L = m_sheet.layout();
    m_syncingBars = true;
    const PaneView body = L.pane(Pane::Body);
    const int pageX = int(std::floor(double(std::max(0, body.rect.width())) / L.zoom())), pageY = int(std::floor(double(std::max(0, body.rect.height())) / L.zoom()));
    verticalScrollBar()->setRange(0, int(std::min<qint64>(L.maxScrollY(), INT_MAX)));
    verticalScrollBar()->setPageStep(std::max(1, pageY));
    verticalScrollBar()->setSingleStep(std::max(1, L.rows.defaultSize()));
    verticalScrollBar()->setValue(int(std::min<qint64>(L.scrollY(), INT_MAX)));
    horizontalScrollBar()->setInvertedAppearance(mirrored());   // (the sheet's first column is at the right)
    horizontalScrollBar()->setRange(0, int(std::min<qint64>(L.maxScrollX(), INT_MAX)));
    horizontalScrollBar()->setPageStep(std::max(1, pageX));
    horizontalScrollBar()->setSingleStep(std::max(1, L.cols.defaultSize()));
    horizontalScrollBar()->setValue(int(std::min<qint64>(L.scrollX(), INT_MAX)));
    m_syncingBars = false;
}

QRect GridView::bodyScrollRect(bool vertical) const
{
    const GridLayout &L = m_sheet.layout();
    const QSize v = viewport()->size();
    const PaneView body = L.pane(Pane::Body);
    // what moves with a scroll in one direction: along it, the part that scrolls (and the headers and the frozen part beside it); across it, everything
    return vertical ? QRect(0, body.rect.top(), v.width(), std::max(0, v.height() - body.rect.top())) : QRect(body.rect.left(), 0, std::max(0, v.width() - body.rect.left()), v.height());
}

void GridView::scrollTo(qint64 x, qint64 y)
{
    GridLayout &L = m_sheet.layout();
    const qint64 ox = L.scrollX(), oy = L.scrollY();
    L.setScroll(x, y);
    if (L.scrollX() == ox && L.scrollY() == oy) return;
    const int digits = m_headerDigits;
    updateHeaderWidth();
    updateScrollBars();
    const double z = L.zoom();
    const double exactX = double(L.scrollX() - ox) * z, exactY = double(L.scrollY() - oy) * z;
    const int dx = int(std::lround(exactX)), dy = int(std::lround(exactY));
    const PaneView body = L.pane(Pane::Body);
    // (the pixels already drawn are the pixels that are wanted when the scroll is a whole number of them at this zoom: every edge is then where it was, that far on; at 100% always)
    const bool exact = digits == m_headerDigits && std::abs(exactX - dx) < 1e-9 && std::abs(exactY - dy) < 1e-9;
    if (exact && dx == 0 && dy != 0 && std::abs(dy) < body.rect.height()) {
        viewport()->scroll(0, -dy, mirror(bodyScrollRect(true)));
        ++m_blits;
    } else if (exact && dy == 0 && dx != 0 && std::abs(dx) < body.rect.width()) {
        viewport()->scroll(mirrored() ? dx : -dx, 0, mirror(bodyScrollRect(false)));
        ++m_blits;
    } else {
        viewport()->update();
        ++m_fullScrolls;
    }
    Q_EMIT viewChanged();
}

void GridView::scrollRows(int rows)
{
    GridLayout &L = m_sheet.layout();
    qint64 top = L.rows.indexAt(L.rows.position(L.frozenRows()) + L.scrollY());
    if (top < 0) top = L.rows.lastVisible();
    for (int i = 0; i < std::abs(rows); ++i) {
        const qint64 next = rows > 0 ? L.rows.nextVisible(top) : L.rows.previousVisible(top);
        if (next < 0 || next >= L.rows.count() || (rows < 0 && next < L.frozenRows())) break;
        top = next;
    }
    scrollTo(L.scrollX(), L.rows.position(top) - L.rows.position(L.frozenRows()));
}

int GridView::wholeRowsInView() const
{
    const GridLayout &L = m_sheet.layout();
    const PaneView body = L.pane(Pane::Body);
    int rows = 0;
    for (qint64 r = body.firstRow; !body.isEmpty() && r <= body.lastRow && rows < 4096; r = L.rows.nextVisible(r))
        if (L.yEdge(r, false) >= body.rect.top() && L.yEdge(r + 1, false) <= body.rect.bottom() + 1) ++rows;
    return std::max(1, rows);
}

void GridView::scrollColumns(int columns)
{
    GridLayout &L = m_sheet.layout();
    qint64 left = L.cols.indexAt(L.cols.position(L.frozenCols()) + L.scrollX());
    if (left < 0) left = L.cols.lastVisible();
    for (int i = 0; i < std::abs(columns); ++i) {
        const qint64 next = columns > 0 ? L.cols.nextVisible(left) : L.cols.previousVisible(left);
        if (next < 0 || next >= L.cols.count() || (columns < 0 && next < L.frozenCols())) break;
        left = next;
    }
    scrollTo(L.cols.position(left) - L.cols.position(L.frozenCols()), L.scrollY());
}

bool GridView::ensureVisible(qint64 row, qint64 col)
{
    const GridLayout &L = m_sheet.layout();
    qint64 x = L.scrollX(), y = L.scrollY();
    if (!L.scrollToShow(row, col, &x, &y)) return false;
    // (the scroll is set by scrollTo, so that the pixels already drawn are moved and not drawn again)
    scrollTo(x, y);
    return true;
}

void GridView::setZoom(double z)
{
    GridLayout &L = m_sheet.layout();
    if (std::abs(L.zoom() - std::min(GridLayout::kMaxZoom, std::max(GridLayout::kMinZoom, z))) < 1e-9) return;
    L.setZoom(z);
    m_painter->resetFonts();
    layoutChanged();
    Q_EMIT zoomChanged(L.zoom());
}

void GridView::setShowGridlines(bool on)
{
    m_gridlines = on;
    viewport()->update();
}

void GridView::setShowHeadings(bool on)
{
    m_headings = on;
    layoutChanged();
}

void GridView::setFrozen(qint64 rows, qint64 cols)
{
    GridLayout &L = m_sheet.layout();
    L.setFrozen(rows, cols);
    layoutChanged();
}

qint64 GridView::firstRowInView() const
{
    const GridLayout &L = m_sheet.layout();
    const qint64 first = L.rows.nextVisible(-1);
    if (first < L.frozenRows()) return first;   // (the frozen rows are the top of the view)
    const qint64 top = L.rows.indexAt(L.rows.position(L.frozenRows()) + L.scrollY());
    return top >= 0 ? top : L.rows.lastVisible();
}

qint64 GridView::firstColInView() const
{
    const GridLayout &L = m_sheet.layout();
    const qint64 first = L.cols.nextVisible(-1);
    if (first < L.frozenCols()) return first;
    const qint64 left = L.cols.indexAt(L.cols.position(L.frozenCols()) + L.scrollX());
    return left >= 0 ? left : L.cols.lastVisible();
}

void GridView::freezeAt(qint64 row, qint64 col)
{
    GridLayout &L = m_sheet.layout();
    // where the view begins now (before what is frozen is changed)
    const qint64 viewTop = firstRowInView(), viewLeft = firstColInView();
    const bool rowsFrozen = row > viewTop, colsFrozen = col > viewLeft;
    L.setFrozen(0, 0);
    L.setSkipped(0, 0);
    L.setSkipped(rowsFrozen ? viewTop : 0, colsFrozen ? viewLeft : 0);
    L.setFrozen(rowsFrozen ? row : 0, colsFrozen ? col : 0);
    // the part that scrolls begins at the cell (the frozen part ends there), or where the view was when nothing is frozen in that direction
    const qint64 bodyRow = rowsFrozen ? row : viewTop, bodyCol = colsFrozen ? col : viewLeft;
    L.setScroll(L.cols.position(bodyCol) - L.cols.position(L.frozenCols()), L.rows.position(bodyRow) - L.rows.position(L.frozenRows()));
    layoutChanged();
}

void GridView::freezeTopRow()
{
    const GridLayout &L = m_sheet.layout();
    const qint64 keptCols = L.frozenCols() > L.skippedCols() ? L.frozenCols() : 0;
    freezeAt(L.rows.nextVisible(firstRowInView()), keptCols);
}

void GridView::freezeFirstColumn()
{
    const GridLayout &L = m_sheet.layout();
    const qint64 keptRows = L.frozenRows() > L.skippedRows() ? L.frozenRows() : 0;
    freezeAt(keptRows, L.cols.nextVisible(firstColInView()));
}

void GridView::unfreeze() { freezeAt(0, 0); }

void GridView::invalidateValues()
{
    m_sheet.invalidateTexts();
    viewport()->update();
    startStats();
}

bool GridView::valuesChanged(int sheetIndex, const std::vector<ValueStore::Item> &items)
{
    if (sheetIndex != m_sheet.data().sheetIndex) return false;
    const GridLayout &L = m_sheet.layout();
    struct Part {
        Pane pane;
        PaneView view;
    };
    std::vector<Part> parts;
    for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const PaneView v = L.pane(p);
        if (!v.isEmpty()) parts.push_back({p, v});
    }
    const GridRange &sel = m_selection.range;
    bool active = false, inSelection = false, all = false;
    int visible = 0;
    QRegion dirty;
    for (const ValueStore::Item &it : items) {
        if (it.sheet != sheetIndex) continue;
        m_sheet.invalidateText(it.row, it.col);
        if (qint64(it.row) == m_selection.activeRow && qint64(it.col) == m_selection.activeCol) active = true;
        if (qint64(it.row) >= sel.row1 && qint64(it.row) <= sel.row2 && qint64(it.col) >= sel.col1 && qint64(it.col) <= sel.col2) inSelection = true;
        if (all) continue;
        for (const Part &p : parts) {
            if (qint64(it.row) < p.view.firstRow || qint64(it.row) > p.view.lastRow || qint64(it.col) < p.view.firstCol || qint64(it.col) > p.view.lastCol) continue;
            // (the strip of the part that the row crosses: text may run into the cells beside it)
            if (++visible > 200) {
                all = true;
                break;
            }
            dirty += L.rangeRectIn(p.pane, it.row, p.view.firstCol, it.row, p.view.lastCol) & p.view.rect;
        }
    }
    if (m_sheet.hasConditionalFormats() && !items.empty()) {
        // (a rule of conditional formatting can make any cell look different when a value changes: the figures it keeps are let go, the texts are made again as they are drawn, and
        // the whole of what is shown is drawn again)
        m_sheet.invalidateLooks();
        all = true;
    }
    if (all) viewport()->update();
    else if (!dirty.isEmpty()) viewport()->update(mirror(dirty));
    if (inSelection) startStats();
    return active;
}

// ---- the selection

void GridView::setSelection(const Selection &s) { setSelectionInternal(s); }

void GridView::selectCell(qint64 row, qint64 col)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.click(row, col));
}

void GridView::selectRange(qint64 row1, qint64 col1, qint64 row2, qint64 col2)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.extendTo(nav.click(row1, col1), row2, col2));
}

void GridView::selectRows(qint64 from, qint64 to)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.selectRows(from, to), false);
}

void GridView::selectColumns(qint64 from, qint64 to)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.selectColumns(from, to), false);
}

void GridView::selectAll()
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.selectAll(m_selection), false);
}

void GridView::showActive()
{
    // the corner that moved is the one to keep in view: the head when the selection is extended
    const bool extended = !m_selection.isOneCell() || m_selection.whole != Selection::Whole::None;
    if (m_selection.whole == Selection::Whole::Rows || m_selection.whole == Selection::Whole::All) return;
    if (m_selection.whole == Selection::Whole::Columns) return;
    if (extended && (m_selection.headRow != m_selection.activeRow || m_selection.headCol != m_selection.activeCol)) ensureVisible(m_selection.headRow, m_selection.headCol);
    else ensureVisible(m_selection.activeRow, m_selection.activeCol);
}

void GridView::setSelectionInternal(const Selection &s, bool keepActiveInView)
{
    const Selection old = m_selection;
    if (s == old) return;
    m_selection = s;
    if (keepActiveInView) showActive();
    // draw again what the old selection covered, what the new one covers, and the headers
    const GridLayout &L = m_sheet.layout();
    QRegion dirty;
    const Selection *both[] = {&old, &m_selection};
    for (const Selection *sel : both)
        for (Pane pane : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
            const QRect r = L.rangeRectIn(pane, sel->range.row1, sel->range.col1, sel->range.row2, sel->range.col2).adjusted(-2, -2, 2, 2) & L.pane(pane).rect;
            if (!r.isEmpty()) dirty += r;
        }
    dirty += QRect(0, 0, viewport()->width(), L.headerHeight());
    dirty += QRect(0, 0, L.headerWidth(), viewport()->height());
    viewport()->update(mirror(dirty));
    Q_EMIT selectionChanged();
    startStats();
    if (m_drag == Drag::None) Q_EMIT selectionSettled();
}

QString GridView::selectionName() const
{
    return cellName(quint32(std::max<qint64>(0, m_selection.activeRow)), quint32(std::max<qint64>(0, m_selection.activeCol)));
}

QString GridView::rangeName() const
{
    const GridRange &r = m_selection.range;
    if (m_selection.whole == Selection::Whole::All) return QStringLiteral("A1:%1").arg(cellName(quint32(r.row2), quint32(r.col2)));
    if (m_selection.whole == Selection::Whole::Columns) return r.col1 == r.col2 ? QStringLiteral("%1:%1").arg(columnName(quint32(r.col1))) : QStringLiteral("%1:%2").arg(columnName(quint32(r.col1)), columnName(quint32(r.col2)));
    if (m_selection.whole == Selection::Whole::Rows) return QStringLiteral("%1:%2").arg(r.row1 + 1).arg(r.row2 + 1);
    if (m_selection.isOneCell()) return cellName(quint32(r.row1), quint32(r.col1));
    return QStringLiteral("%1:%2").arg(cellName(quint32(r.row1), quint32(r.col1)), cellName(quint32(r.row2), quint32(r.col2)));
}

QString GridView::activeCellText() const
{
    const Cell *cell = m_sheet.cellAt(quint32(m_selection.activeRow), quint32(m_selection.activeCol));
    if (!cell || !m_sheet.data().sheet) return QString();
    using jp::calc::Value;
    if (cell->hasFormula() || (cell->flags & jp::office::sheet::ArrayMember)) {
        const QString f = displayFormula(m_sheet.data().sheet->formulaText(*cell));
        if (!f.isEmpty()) return (cell->flags & jp::office::sheet::ArrayFormula) ? QStringLiteral("{=%1}").arg(f) : QStringLiteral("=%1").arg(f);
    }
    const Value v = m_sheet.valueOf(*cell);
    switch (v.kind()) {
    case Value::Kind::Text: return v.text();
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    case Value::Kind::Number: {
        // (a date, a time, or a percentage shows as its format makes it; any other number as it is stored)
        const CellLook &look = m_sheet.looks().look(cell->style);
        if (look.hasFormat && (look.format.kind() == jp::calc::FormatKind::Date || look.format.code().contains(QLatin1Char('%')))) {
            const jp::calc::Formatted f = look.format.format(v.number(), m_sheet.data().dateSystem, 0);   // (as wide as it is: the bar has no cell to fit)
            if (!f.failed) return f.text.trimmed();
        }
        return jp::calc::numberToText(v.number());
    }
    default: return QString();
    }
}

QString GridView::selectionAsText(bool *tooMany) const
{
    if (tooMany) *tooMany = false;
    const std::unique_ptr<TextWalk> walk = copyWalk();
    if (!walk) return QString();
    if (walk->tooMany()) {
        if (tooMany) *tooMany = true;
        return QString();
    }
    walk->step(0);
    return walk->text();
}

std::unique_ptr<TextWalk> GridView::copyWalk() const
{
    GridRange r = m_selection.range;
    quint32 usedRows = 0, usedCols = 0;
    m_sheet.usedExtent(&usedRows, &usedCols);
    // (rows and columns past the last cell hold nothing: a whole column is its used rows)
    r.row2 = std::min<qint64>(r.row2, std::max<qint64>(0, qint64(usedRows) - 1));
    r.col2 = std::min<qint64>(r.col2, std::max<qint64>(0, qint64(usedCols) - 1));
    if (r.row2 < r.row1 || r.col2 < r.col1) return nullptr;
    return std::make_unique<TextWalk>(&m_sheet, r);
}

// ---- the numbers of the selection, and the walk for the edge of the data

void GridView::startStats()
{
    m_statsWalk.reset();
    m_statsText.clear();
    const GridSheet &sheet = m_sheet;
    GridRange r = m_selection.range;
    quint32 usedRows = 0, usedCols = 0;
    sheet.usedExtent(&usedRows, &usedCols);
    const bool single = m_selection.isOneCell();
    // (only the cells the sheet has can hold anything: a whole column is added up as far as its last row)
    r.row2 = std::min<qint64>(r.row2, qint64(usedRows) - 1);
    r.col2 = std::min<qint64>(r.col2, qint64(usedCols) - 1);
    if (single || r.row2 < r.row1 || r.col2 < r.col1) {
        Q_EMIT statsChanged(QString());
        return;
    }
    m_statsWalk = std::make_unique<StatsWalk>(&m_sheet, r);
    stepStats();
}

void GridView::stepStats()
{
    if (!m_statsWalk) return;
    QElapsedTimer t;
    t.start();
    bool done = false;
    while (!done && t.elapsed() < 4) done = m_statsWalk->step(StatsWalk::kWorkPerSlice);
    if (!done) {
        m_walkTimer.start(0);
        return;
    }
    m_statsText = jp::grid::statsText(m_statsWalk->stats());
    m_statsWalk.reset();
    Q_EMIT statsChanged(m_statsText);
}

void GridView::startJump(int dRow, int dCol, bool extend)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    m_jump = std::make_unique<JumpWalk>(nav.jump(m_selection, dRow, dCol, extend));
    m_jumpExtend = extend;
    m_jumpFrom = m_selection;
    stepJump();
}

void GridView::stepJump()
{
    if (!m_jump) return;
    QElapsedTimer t;
    t.start();
    bool done = false;
    while (!done && t.elapsed() < 6) done = m_jump->step(20000);
    if (!done) {
        m_walkTimer.start(0);
        return;
    }
    Navigator nav(&m_sheet.layout(), m_nav.get());
    const qint64 row = m_jump->row(), col = m_jump->col();
    m_jump.reset();
    setSelectionInternal(nav.arrive(m_jumpFrom, row, col, m_jumpExtend));
}

// ---- painting

PaintOptions GridView::paintOptions() const
{
    PaintOptions o;
    o.gridlines = m_gridlines;
    o.headers = m_headings;
    o.selection = m_cellsEnabled ? &m_selection : nullptr;
    o.fillHandle = m_cellsEnabled;
    o.fillPreview = m_drag == Drag::Fill && !(m_fillExtent == m_fillSource) ? &m_fillExtent : nullptr;
    o.copyRange = hasCopyBorder() ? &m_copyBorder : nullptr;
    o.copyPhase = m_copyPhase;
    o.dropDown = dropDownRectInLayout();
    if (m_drag == Drag::Resize) {
        o.resizeGuide = m_resizeEdge + int(std::lround((m_resizeSize - m_resizeSize0) * m_sheet.layout().zoom()));
        o.resizeGuideColumn = m_resizeColumn;
    }
    o.accent = uiAccent();
    o.headerBackground = uiPanel();
    o.headerText = uiText();
    o.headerLine = uiLine();
    QColor selected = o.accent;
    selected.setAlpha(uiDark() ? 90 : 55);
    o.headerSelected = QColor(uiPanel());
    o.headerSelected = QColor((o.headerBackground.red() * (255 - selected.alpha()) + selected.red() * selected.alpha()) / 255,
                              (o.headerBackground.green() * (255 - selected.alpha()) + selected.green() * selected.alpha()) / 255,
                              (o.headerBackground.blue() * (255 - selected.alpha()) + selected.blue() * selected.alpha()) / 255);
    o.backdrop = uiWindow();
    QFont f = font();
    f.setPixelSize(11);
    o.headerFont = f;
    return o;
}

void GridView::paintEvent(QPaintEvent *e)
{
    QElapsedTimer t;
    t.start();
    const qint64 cpu0 = jp::threadCpuMicros();
    QPainter p(viewport());
    m_painter->paint(&p, mirror(e->rect()), paintOptions());
    paintPageBreaks(&p, e->rect());
    paintOverlay(&p, e->rect());
    paintMarks(&p, e->rect());
    paintNotes(&p, e->rect());
    p.end();
    if (m_paintTimes.size() < 100000) {
        m_paintTimes.push_back(double(t.nsecsElapsed()) / 1e6);
        m_paintCpuTimes.push_back(double(jp::threadCpuMicros() - cpu0) / 1e3);
    }
    afterPaint();
}

// What the rules of conditional formatting left after a paint is done in slices between the paints, and what stopped one altogether is said once.
void GridView::afterPaint()
{
    const CondFormats &rules = m_sheet.conditional();
    if (rules.wantsMore() && !m_conditionalTimer.isActive()) m_conditionalTimer.start(0);
    const QString notice = rules.takeNotice();
    if (!notice.isEmpty()) Q_EMIT noticeRaised(notice);
}

void GridView::conditionalWork()
{
    CondFormats &rules = m_sheet.conditional();
    const bool more = rules.step(3000);
    // (what was shown plain while the figures were not ready is made again, and drawn again)
    m_sheet.invalidateLooks();
    viewport()->update();
    if (more) m_conditionalTimer.start(0);
}

void GridView::setPageBreaks(std::vector<PageBreakLine> rows, std::vector<PageBreakLine> cols)
{
    auto order = [](const PageBreakLine &a, const PageBreakLine &b) { return a.before < b.before; };
    std::stable_sort(rows.begin(), rows.end(), order);
    std::stable_sort(cols.begin(), cols.end(), order);
    m_breakRows = std::move(rows);
    m_breakCols = std::move(cols);
    viewport()->update();
}

// The lines are found by the rows and columns each part of the grid shows, so a sheet with a hundred thousand of them costs the few on the screen.
void GridView::paintPageBreaks(QPainter *p, const QRect &clip)
{
    if (m_breakView) paintBreakView(p, clip);
    if (m_breakRows.empty() && m_breakCols.empty()) return;
    const GridLayout &L = m_sheet.layout();
    const QColor ink = uiDark() ? QColor(0x7F, 0xA8, 0xFF) : QColor(0x1F, 0x3F, 0xA8);
    p->save();
    p->setRenderHint(QPainter::Antialiasing, false);
    for (const Pane pane : {Pane::Body, Pane::Left, Pane::Top, Pane::Corner}) {
        const PaneView pv = L.pane(pane);
        const QRect area = pv.rect & clip;
        if (pv.isEmpty() || area.isEmpty()) continue;
        const bool fr = pane == Pane::Corner || pane == Pane::Top, fc = pane == Pane::Corner || pane == Pane::Left;
        p->setClipRect(area);
        auto draw = [&](const PageBreakLine &line, bool horizontal) {
            QPen pen(ink, line.manual ? 2 : 1, line.manual ? Qt::SolidLine : Qt::DashLine);
            pen.setCosmetic(true);
            p->setPen(pen);
            if (horizontal) {
                const int y = L.yEdge(line.before, fr), x0 = L.xEdge(line.from, fc), x1 = L.xEdge(line.to + 1, fc);
                p->drawLine(x0, y, x1, y);
            } else {
                const int x = L.xEdge(line.before, fc), y0 = L.yEdge(line.from, fr), y1 = L.yEdge(line.to + 1, fr);
                p->drawLine(x, y0, x, y1);
            }
        };
        auto first = [](const std::vector<PageBreakLine> &v, qint64 at) {
            return std::lower_bound(v.begin(), v.end(), at, [](const PageBreakLine &l, qint64 value) { return l.before < value; });
        };
        for (auto it = first(m_breakRows, pv.firstRow); it != m_breakRows.end() && it->before <= pv.lastRow + 1; ++it) draw(*it, true);
        for (auto it = first(m_breakCols, pv.firstCol); it != m_breakCols.end() && it->before <= pv.lastCol + 1; ++it) draw(*it, false);
    }
    p->restore();
}

void GridView::resizeEvent(QResizeEvent *e)
{
    QAbstractScrollArea::resizeEvent(e);
    layoutChanged();
}

// What an input method (for Chinese, Japanese, and Korean, or accents made of two keys) finishes writing starts an entry in the cell; what it is still composing is not shown here.
void GridView::inputMethodEvent(QInputMethodEvent *e)
{
    if (m_cellsEnabled && !e->commitString().isEmpty()) Q_EMIT typingStarted(e->commitString());
    e->accept();
}

QVariant GridView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImEnabled) return true;
    return QAbstractScrollArea::inputMethodQuery(query);
}

void GridView::focusInEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusInEvent(e);
}

void GridView::focusOutEvent(QFocusEvent *e)
{
    QAbstractScrollArea::focusOutEvent(e);
}

// ---- the mouse

void GridView::dragTo(const QPoint &pos)
{
    if (m_drag == Drag::None) return;
    GridLayout &L = m_sheet.layout();
    const PaneView body = L.pane(Pane::Body);
    // a point outside the body is as far as the edge, and the view scrolls toward it
    QPoint p = pos;
    const QRect inner = body.rect.isEmpty() ? viewport()->rect() : body.rect;
    int scrollX = 0, scrollY = 0;
    if (pos.x() < inner.left()) scrollX = -1;
    else if (pos.x() > inner.right()) scrollX = 1;
    if (pos.y() < inner.top()) scrollY = -1;
    else if (pos.y() > inner.bottom()) scrollY = 1;
    if (m_drag != Drag::Rows) p.setX(std::max(inner.left(), std::min(pos.x(), inner.right())));
    if (m_drag != Drag::Columns) p.setY(std::max(inner.top(), std::min(pos.y(), inner.bottom())));
    if (scrollY && m_drag != Drag::Columns) scrollRows(scrollY);
    if (scrollX && m_drag != Drag::Rows) scrollColumns(scrollX);
    const Hit h = L.hit(p);
    Navigator nav(&L, m_nav.get());
    if (m_drag == Drag::Fill && h.kind == Hit::Kind::Cell) {
        // along one row or one column: the way the cell under the mouse is farthest from the block
        const GridRange &src = m_fillSource;
        const qint64 down = h.row > src.row2 ? h.row - src.row2 : 0, up = h.row < src.row1 ? src.row1 - h.row : 0;
        const qint64 right = h.col > src.col2 ? h.col - src.col2 : 0, left = h.col < src.col1 ? src.col1 - h.col : 0;
        const qint64 most = std::max({down, up, right, left});
        GridRange extent = src;
        if (most > 0) {
            if (down == most) extent.row2 = h.row;
            else if (up == most) extent.row1 = h.row;
            else if (right == most) extent.col2 = h.col;
            else extent.col1 = h.col;
        }
        if (!(extent == m_fillExtent)) {
            m_fillExtent = extent;
            viewport()->update();
        }
    } else if (m_drag == Drag::Cells && h.kind == Hit::Kind::Cell) setSelectionInternal(nav.extendTo(m_selection, h.row, h.col), false);
    else if (m_drag == Drag::Rows && h.row >= 0) setSelectionInternal(nav.extendRows(m_selection, h.row), false);
    else if (m_drag == Drag::Columns && h.col >= 0) setSelectionInternal(nav.extendColumns(m_selection, h.col), false);
    if (scrollX || scrollY) {
        if (!m_autoScroll.isActive()) m_autoScroll.start();
    } else {
        m_autoScroll.stop();
    }
}

void GridView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_clickGuard && !m_clickGuard()) return;
    setFocus(Qt::MouseFocusReason);
    if (m_breakView && e->button() == Qt::LeftButton && m_cellsEnabled && breakPress(mirror(e->pos()))) return;
    if (overlayContext(e) || overlayPress(e) || !m_cellsEnabled) return;
    m_jump.reset();
    const GridLayout &L = m_sheet.layout();
    const QPoint lp = mirror(e->pos());
    const Hit h = L.hit(lp);
    Navigator nav(&m_sheet.layout(), m_nav.get());
    const bool shift = e->modifiers() & Qt::ShiftModifier;
    if (e->button() == Qt::RightButton) {
        // a right click inside the selection keeps it; outside, it selects what is under it
        if (h.kind == Hit::Kind::Cell && !m_selection.range.contains(h.row, h.col)) setSelectionInternal(nav.click(h.row, h.col));
        else if (h.kind == Hit::Kind::RowHeader && !(m_selection.whole == Selection::Whole::Rows && h.row >= m_selection.range.row1 && h.row <= m_selection.range.row2)) setSelectionInternal(nav.selectRows(h.row, h.row));
        else if (h.kind == Hit::Kind::ColumnHeader && !(m_selection.whole == Selection::Whole::Columns && h.col >= m_selection.range.col1 && h.col <= m_selection.range.col2)) setSelectionInternal(nav.selectColumns(h.col, h.col));
        Q_EMIT contextMenuRequested(e->globalPosition().toPoint());
        return;
    }
    if (e->button() != Qt::LeftButton) return;
    if (headerButtonPress(lp)) return;
    if (dropDownRectInLayout().contains(lp)) {
        Q_EMIT dropDownRequested();
        return;
    }
    bool borderColumn = false;
    qint64 borderIndex = -1;
    if (borderAtInLayout(lp, &borderColumn, &borderIndex)) {
        const GridLayout &LL = m_sheet.layout();
        m_drag = Drag::Resize;
        m_resizeColumn = borderColumn;
        m_resizeIndex = borderIndex;
        m_resizeFrom = borderColumn ? lp.x() : lp.y();
        m_resizeSize0 = m_resizeSize = borderColumn ? LL.cols.size(borderIndex) : LL.rows.size(borderIndex);
        m_resizeEdge = borderColumn ? LL.xEdge(borderIndex + 1, borderIndex < LL.frozenCols()) : LL.yEdge(borderIndex + 1, borderIndex < LL.frozenRows());
        m_lastMouse = lp;
        viewport()->update();
        return;
    }
    if (fillHandleRectInLayout().contains(lp)) {
        m_drag = Drag::Fill;
        m_fillSource = m_fillExtent = m_selection.range;
        m_fillCtrl = e->modifiers() & Qt::ControlModifier;
        m_lastMouse = lp;
        return;
    }
    switch (h.kind) {
    case Hit::Kind::Cell:
        m_drag = Drag::Cells;   // (before the selection is set: it is settled when the button is let go)
        m_pressRow = h.kind == Hit::Kind::Cell ? h.row : -1;
        m_pressCol = h.kind == Hit::Kind::Cell ? h.col : -1;
        setSelectionInternal(shift ? nav.extendTo(m_selection, h.row, h.col) : nav.click(h.row, h.col), false);
        break;
    case Hit::Kind::RowHeader:
        m_drag = Drag::Rows;
        setSelectionInternal(shift ? nav.extendRows(m_selection, h.row) : nav.selectRows(h.row, h.row), false);
        break;
    case Hit::Kind::ColumnHeader:
        m_drag = Drag::Columns;
        setSelectionInternal(shift ? nav.extendColumns(m_selection, h.col) : nav.selectColumns(h.col, h.col), false);
        break;
    case Hit::Kind::Corner: setSelectionInternal(nav.selectAll(m_selection), false); break;
    case Hit::Kind::None: break;
    }
    m_lastMouse = lp;
}

void GridView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_drag == Drag::None && overlayMove(e)) return;
    const QPoint lp = mirror(e->pos());
    if (breakMove(lp)) return;
    m_lastMouse = lp;
    if (m_drag == Drag::Resize) {
        const int at = m_resizeColumn ? lp.x() : lp.y();
        m_resizeSize = std::clamp(m_resizeSize0 + int(std::lround((at - m_resizeFrom) / m_sheet.layout().zoom())), 0, Axis::kMaxSize);
        viewport()->update();
        return;
    }
    if (e->buttons() & Qt::LeftButton) {
        dragTo(lp);
        return;
    }
    bool column = false;
    qint64 index = -1;
    if (m_breakView) {
        const BreakGrab g = breakGrabAt(e->pos());
        if (g.kind != BreakGrab::Kind::None) {
            const bool vertical = g.kind == BreakGrab::Kind::Line ? g.column : g.side >= 2;
            viewport()->setCursor(vertical ? Qt::SizeHorCursor : Qt::SizeVerCursor);
            return;
        }
    }
    if (borderAtInLayout(lp, &column, &index)) viewport()->setCursor(column ? Qt::SplitHCursor : Qt::SplitVCursor);
    else viewport()->setCursor(fillHandleRectInLayout().contains(lp) ? Qt::CrossCursor : Qt::ArrowCursor);
    const Hit over = m_sheet.layout().hit(lp);
    const qint64 hr = over.kind == Hit::Kind::Cell ? over.row : -1, hc = over.kind == Hit::Kind::Cell ? over.col : -1;
    if (hr != m_hoverRow || hc != m_hoverCol) {
        m_hoverRow = hr;
        m_hoverCol = hc;
        Q_EMIT hoverCellChanged(hr, hc);
    }
}

void GridView::leaveEvent(QEvent *e)
{
    QAbstractScrollArea::leaveEvent(e);
    if (m_hoverRow >= 0) {
        m_hoverRow = m_hoverCol = -1;
        Q_EMIT hoverCellChanged(-1, -1);
    }
}

bool GridView::borderAt(const QPoint &p, bool *column, qint64 *index) const { return borderAtInLayout(mirror(p), column, index); }

bool GridView::borderAtInLayout(const QPoint &p, bool *column, qint64 *index) const
{
    if (!m_headings) return false;
    const GridLayout &L = m_sheet.layout();
    constexpr int kGrab = 4;
    const bool inColumns = p.y() >= 0 && p.y() < L.headerHeight() && p.x() >= L.headerWidth();
    const bool inRows = p.x() >= 0 && p.x() < L.headerWidth() && p.y() >= L.headerHeight();
    if (!inColumns && !inRows) return false;
    // the columns (rows) a few pixels either side of the point: the one whose end is near it
    for (const int d : {0, -kGrab, kGrab}) {
        const QPoint q = inColumns ? QPoint(p.x() + d, p.y()) : QPoint(p.x(), p.y() + d);
        const Hit h = L.hit(q);
        const qint64 at = inColumns ? (h.kind == Hit::Kind::ColumnHeader ? h.col : -1) : (h.kind == Hit::Kind::RowHeader ? h.row : -1);
        if (at < 0) continue;
        const int edge = inColumns ? L.xEdge(at + 1, at < L.frozenCols()) : L.yEdge(at + 1, at < L.frozenRows());
        const int here = inColumns ? p.x() : p.y();
        if (std::abs(here - edge) <= kGrab) {
            *column = inColumns;
            *index = at;
            return true;
        }
    }
    return false;
}

void GridView::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_drag == Drag::None && overlayRelease(e)) return;
    if (breakRelease()) return;
    if (m_drag == Drag::Resize) {
        m_drag = Drag::None;
        viewport()->update();
        if (m_resizeSize != m_resizeSize0) Q_EMIT resizeRequested(m_resizeColumn, m_resizeIndex, m_resizeSize);
        return;
    }
    const bool fill = m_drag == Drag::Fill;
    const bool selecting = m_drag == Drag::Cells || m_drag == Drag::Rows || m_drag == Drag::Columns;
    const bool clicking = m_drag == Drag::Cells && e && e->button() == Qt::LeftButton && m_pressRow >= 0;
    const GridRange source = m_fillSource, extent = m_fillExtent;
    const bool ctrl = m_fillCtrl || (e && (e->modifiers() & Qt::ControlModifier));
    m_drag = Drag::None;
    m_autoScroll.stop();
    if (fill) {
        viewport()->update();
        if (!(extent == source)) Q_EMIT fillRequested(source, extent, ctrl);
    }
    if (selecting) Q_EMIT selectionSettled();
    if (clicking) {
        const Hit up = m_sheet.layout().hit(mirror(e->pos()));
        if (up.kind == Hit::Kind::Cell && up.row == m_pressRow && up.col == m_pressCol) Q_EMIT cellClicked(up.row, up.col, e->modifiers());
    }
    m_pressRow = m_pressCol = -1;
}

QRect GridView::fillHandleRect() const { return mirror(fillHandleRectInLayout()); }

QRect GridView::fillHandleRectInLayout() const
{
    if (m_selection.whole != Selection::Whole::None) return QRect();
    const QRect corner = cellRectInLayout(m_selection.range.row2, m_selection.range.col2);
    if (corner.isEmpty()) return QRect();
    return QRect(corner.right() - 4, corner.bottom() - 4, 9, 9);
}

void GridView::setDropDownCell(qint64 row, qint64 col)
{
    if (row == m_dropRow && col == m_dropCol) return;
    m_dropRow = row;
    m_dropCol = col;
    viewport()->update();
}

QRect GridView::dropDownCellRect() const { return mirror(m_dropRow < 0 ? QRect() : cellRectInLayout(m_dropRow, m_dropCol)); }

QRect GridView::dropDownRect() const { return mirror(dropDownRectInLayout()); }

QRect GridView::dropDownRectInLayout() const
{
    if (m_dropRow < 0 || m_dropRow != m_selection.activeRow || m_dropCol != m_selection.activeCol) return QRect();
    const QRect cell = m_dropRow < 0 ? QRect() : cellRectInLayout(m_dropRow, m_dropCol);
    if (cell.isEmpty() || cell.width() < 30 || cell.height() < 12) return QRect();
    const int size = std::min(cell.height() - 2, 17);
    return QRect(cell.right() - size, cell.bottom() - size, size, size);
}

void GridView::setCopyBorder(const GridRange &range)
{
    m_copyBorder = range;
    m_copyPhase = 0;
    m_copyTimer.start();
    viewport()->update();
}

void GridView::clearCopyBorder()
{
    if (!hasCopyBorder()) return;
    m_copyBorder = GridRange{-1, -1, -1, -1};
    m_copyTimer.stop();
    viewport()->update();
}

void GridView::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (overlayDoubleClick(e)) return;
    if (e->button() != Qt::LeftButton || !m_cellsEnabled) return;
    bool column = false;
    qint64 index = -1;
    const QPoint lp = mirror(e->pos());
    if (borderAtInLayout(lp, &column, &index)) {
        m_drag = Drag::None;
        Q_EMIT fitRequested(column, index);
        return;
    }
    const Hit h = m_sheet.layout().hit(lp);
    if (h.kind == Hit::Kind::Cell) Q_EMIT editRequested();
}

QRect GridView::cellRect(qint64 row, qint64 col) const { return mirror(cellRectInLayout(row, col)); }

QRect GridView::cellRectInLayout(qint64 row, qint64 col) const
{
    const GridLayout &L = m_sheet.layout();
    GridRange r{row, col, row, col};
    const int m = row >= 0 && col >= 0 ? L.merges.at(quint32(row), quint32(col)) : -1;
    if (m >= 0) {
        const jp::office::sheet::CellRange &range = L.merges.range(m);
        r = GridRange{qint64(range.row1), qint64(range.col1), qint64(range.row2), qint64(range.col2)};
    }
    for (Pane p : {Pane::Corner, Pane::Top, Pane::Left, Pane::Body}) {
        const PaneView v = L.pane(p);
        if (v.isEmpty() || row < v.firstRow || row > v.lastRow || col < v.firstCol || col > v.lastCol) continue;
        const QRect rect = L.rangeRectIn(p, r.row1, r.col1, r.row2, r.col2);
        return rect & v.rect;
    }
    return QRect();
}

void GridView::moveAfterEnter(bool up)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.enter(m_selection, up));
}

void GridView::moveAfterTab(bool back)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.tab(m_selection, back));
}

void GridView::moveAfterArrow(int dRow, int dCol)
{
    Navigator nav(&m_sheet.layout(), m_nav.get());
    setSelectionInternal(nav.move(m_selection, dRow, dCol, false));
}

void GridView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const int steps = e->angleDelta().y() / 120;
        if (steps != 0) setZoom(std::round((zoom() + steps * 0.1) * 10) / 10);
        e->accept();
        return;
    }
    const double z = zoom();
    if (!e->pixelDelta().isNull()) {
        scrollTo(m_sheet.layout().scrollX() + (mirrored() ? 1 : -1) * qint64(std::lround(e->pixelDelta().x() / z)), m_sheet.layout().scrollY() - qint64(std::lround(e->pixelDelta().y() / z)));
    } else {
        const int steps = e->angleDelta().y() / 120;
        const int across = e->angleDelta().x() / 120;
        if ((e->modifiers() & Qt::ShiftModifier) && steps != 0) scrollColumns(-steps * 3);
        else if (steps != 0) scrollRows(-steps * 3);
        if (across != 0) scrollColumns((mirrored() ? across : -across) * 3);
    }
    e->accept();
}

// ---- the keys

bool GridView::keyBeginsAnEntry(Qt::KeyboardModifiers m, const QString &text, bool macKeyboard)
{
    const bool ctrl = m & Qt::ControlModifier, alt = m & Qt::AltModifier, meta = m & Qt::MetaModifier;
    // (on a Mac, Qt gives Cmd as Control and Ctrl as Meta, and Option alone gives the characters of the layout's second level: é, @, a bracket; Cmd or Ctrl with anything is a shortcut.
    // Elsewhere AltGr arrives as Ctrl and Alt together, and Alt alone is a shortcut)
    const bool characterKey = macKeyboard ? !ctrl && !meta : ((ctrl && alt && !meta) || (!ctrl && !alt && !meta));
    return characterKey && !text.isEmpty() && text.front().isPrint();
}

void GridView::keyPressEvent(QKeyEvent *e)
{
    if (m_overlay && m_overlay->key(e)) {
        e->accept();
        return;
    }
    if (!m_cellsEnabled) {
        e->ignore();
        return;
    }
    const bool shift = e->modifiers() & Qt::ShiftModifier, ctrl = e->modifiers() & Qt::ControlModifier;
    const bool plain = !(e->modifiers() & (Qt::AltModifier | Qt::MetaModifier));
    if (m_jump) {
        // any key ends a search for the edge that is still going
        m_jump.reset();
        if (e->key() == Qt::Key_Escape) return;
    }
    if (e->key() == Qt::Key_Down && (e->modifiers() & Qt::AltModifier) && !dropDownRect().isEmpty()) {
        Q_EMIT dropDownRequested();
        return;
    }
    if (e->key() == Qt::Key_Escape && hasCopyBorder()) {
        clearCopyBorder();
        Q_EMIT copyBorderCleared();
        return;
    }
    Navigator nav(&m_sheet.layout(), m_nav.get());
    auto go = [&](const Selection &s) { setSelectionInternal(s); };
    // a character that prints starts an entry
    auto typed = [&]() {
#ifdef Q_OS_MACOS
        constexpr bool kMacKeyboard = true;
#else
        constexpr bool kMacKeyboard = false;
#endif
        if (!keyBeginsAnEntry(e->modifiers(), e->text(), kMacKeyboard)) return false;
        Q_EMIT typingStarted(e->text());
        e->accept();
        return true;
    };
    if (plain && !ctrl) {
        if (e->key() == Qt::Key_F2 && !shift) {
            Q_EMIT editRequested();
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Delete) {
            Q_EMIT clearRequested();
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Backspace) {
            Q_EMIT typingStarted(QString());
            e->accept();
            return;
        }
    }
    int dRow = 0, dCol = 0;
    switch (e->key()) {
    case Qt::Key_Left: dCol = mirrored() ? 1 : -1; break;   // (in a sheet that reads right to left the next column is to the left)
    case Qt::Key_Right: dCol = mirrored() ? -1 : 1; break;
    case Qt::Key_Up: dRow = -1; break;
    case Qt::Key_Down: dRow = 1; break;
    default: break;
    }
    if ((dRow || dCol) && plain) {
        if (ctrl) startJump(dRow, dCol, shift);
        else go(nav.move(m_selection, dRow, dCol, shift));
        e->accept();
        return;
    }
    if (!plain) {
        QAbstractScrollArea::keyPressEvent(e);
        return;
    }
    switch (e->key()) {
    case Qt::Key_Home: go(nav.home(m_selection, ctrl, shift)); break;
    case Qt::Key_End:
        if (ctrl) go(nav.end(m_selection, shift));
        else QAbstractScrollArea::keyPressEvent(e);
        return;
    case Qt::Key_PageDown:
    case Qt::Key_PageUp: {
        // a screen of rows: the sheet goes that far, and the cursor with it (it stays where it was in the window)
        const int direction = e->key() == Qt::Key_PageDown ? 1 : -1, rows = wholeRowsInView();
        scrollRows(direction * rows);
        go(nav.page(m_selection, direction, rows, shift));
        break;
    }
    case Qt::Key_Tab: go(nav.tab(m_selection, false)); break;
    case Qt::Key_Backtab: go(nav.tab(m_selection, true)); break;
    case Qt::Key_Return:
    case Qt::Key_Enter: go(nav.enter(m_selection, shift)); break;
    case Qt::Key_Space:
        if (ctrl) go(nav.selectWholeColumns(m_selection));
        else if (shift) go(nav.selectWholeRows(m_selection));
        else if (!typed()) QAbstractScrollArea::keyPressEvent(e);
        return;
    case Qt::Key_A:
        if (ctrl) go(nav.selectAll(m_selection));
        else if (!typed()) QAbstractScrollArea::keyPressEvent(e);
        return;
    default:
        if (!typed()) QAbstractScrollArea::keyPressEvent(e);
        return;
    }
    e->accept();
}

} // namespace jp::grid
