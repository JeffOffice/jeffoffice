// The views of a sheet: the cells as they are, and the sheet cut into the pages it prints on (the page breaks view). The view a sheet was left in is kept with the workbook and saved with
// it. In the page breaks view the grid shades what does not print and numbers the pages; a line where a page begins or an edge of the printed cells is dragged, and what the drag asks for
// is one step of Undo like any other change of the page setup (changePage).

#include "app/crunchwindow.h"

#include "core/editsession.h"
#include "core/printjob.h"
#include "grid/gridview.h"
#include "grid/pagelayoutview.h"
#include "sheet/pagesetup.h"

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QSlider>
#include <QStatusBar>
#include <QToolButton>

#include <algorithm>
#include <cmath>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::PageBreak;
using jp::office::sheet::PageSetup;

namespace {

// The zoom the page breaks view begins at, so that several pages show at once.
constexpr double kBreakViewZoom = 0.6;

} // namespace

void CrunchWindow::createViewActions()
{
    auto *group = new QActionGroup(this);
    group->setExclusive(true);
    auto view = [this, group](const QString &id, const QString &text, const QString &icon, int kind) {
        QAction *a = mk(id, text, icon, QKeySequence(), [this, kind] { setViewKind(kind); }, true);
        group->addAction(a);
        m_opsActions << id;
        return a;
    };
    view(QStringLiteral("view.normal"), tr("Normal"), QStringLiteral("table-2"), 0)->setChecked(true);
    view(QStringLiteral("view.pageBreakView"), tr("Page Breaks"), QStringLiteral("scissors-line-dashed"), 1);
}

void CrunchWindow::addViewButtons(QStatusBar *bar)
{
    for (const char *id : {"view.normal", "view.pageLayout", "view.pageBreakView"}) {
        auto *b = new QToolButton();
        b->setDefaultAction(act(QString::fromLatin1(id)));
        b->setAutoRaise(true);
        b->setIconSize(QSize(16, 16));
        b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        bar->addPermanentWidget(b);
    }
}

// The person chose a view for the sheet in view.
void CrunchWindow::setViewKind(int kind)
{
    if (!m_session || !m_loaded || m_sheet < 0) return;
    if (kind == m_viewKind) {
        showViewKind(kind);   // (the buttons say what is shown)
        return;
    }
    if (!commitEdit(CellEditor::Way::Stay)) {
        showViewKind(m_viewKind);
        return;
    }
    if (m_viewKind == 0) m_normalZoom = m_view->zoom();
    const int old = m_viewKind;
    showViewKind(kind);
    // (the cells' zoom is the grid's; the pages have a zoom of their own, in the view of the pages)
    if (kind == 2) m_pageView->setZoom(1.0);
    else if (old == 0) m_view->setZoom(kBreakViewZoom);
    else if (kind == 0) m_view->setZoom(m_normalZoom);
    m_session->setSheetView(m_sheet, kind, int(std::lround(viewZoom() * 100)));
    // the zoom shown at the bottom is the view's
    const double zoom = viewZoom();
    m_zoomLabel->setText(QStringLiteral("%1%").arg(std::lround(zoom * 100)));
    const QSignalBlocker block(m_zoomSlider);
    m_zoomSlider->setValue(int(std::lround(std::log(zoom) / std::log(1.05))));
}

// The sheet now in view is shown in the view it was left in (or the one chosen): the grid, the buttons, and the lines.
void CrunchWindow::showViewKind(int kind)
{
    if (m_chartSheetShown) kind = 0;   // (a chart sheet has no pages of cells to show)
    m_viewKind = kind;
    m_view->setBreakView(kind == 1);
    if (kind != 1) m_view->setBreakPages({}, {}, false);
    if (QAction *a = act(QStringLiteral("view.normal"))) a->setChecked(kind == 0);
    if (QAction *a = act(QStringLiteral("view.pageBreakView"))) a->setChecked(kind == 1);
    if (QAction *a = act(QStringLiteral("view.pageLayout"))) a->setChecked(kind == 2);
    showPageLayout(kind == 2);
    // (the pages' zoom is the sheet's own)
    if (kind == 2 && m_pageView) m_pageView->setZoom(m_session ? m_session->pages().viewZoom(m_sheet) / 100.0 : m_view->zoom());
    if (kind == 1 || kind == 2) armPageBreaks();
    else updatePageBreaks();
}

// What the job cut the sheet in view into, for the grid to shade and number.
void CrunchWindow::sendBreakPages(const crunch::PrintJob &job, const PageSetup &setup)
{
    const jp::grid::SheetPrint *print = job.printOf(m_sheet);
    if (!print) return;
    std::vector<grid::GridRange> areas;
    for (const CellRange &a : print->areas) areas.push_back({qint64(a.row1), qint64(a.col1), qint64(a.row2), qint64(a.col2)});
    std::vector<grid::GridView::BreakPage> pages;
    const int first = job.firstPageOf(m_sheet);
    pages.reserve(print->pages.size());
    for (size_t i = 0; i < print->pages.size(); ++i) {
        const jp::grid::PageCut &c = print->pages[i];
        grid::GridView::BreakPage p;
        p.row1 = c.row1;
        p.row2 = c.row2;
        p.col1 = c.col1;
        p.col2 = c.col2;
        p.number = first >= 0 ? job.numberOf(first + int(i)) : int(i) + 1;
        pages.push_back(p);
    }
    // (a sheet fitted to pages uses no breaks of its own: its lines are not moved)
    m_view->setBreakPages(std::move(areas), std::move(pages), !setup.fitToPage && m_session && m_session->canEdit());
}

// A line where a page begins was dragged to `to` (-1: off the printed cells, to be taken away).
void CrunchWindow::breakLineMoved(bool column, qint64 from, qint64 to)
{
    changePage(to < 0 ? tr("Remove page break") : tr("Move page break"), [=](PageSetup *s) {
        QVector<PageBreak> &list = column ? s->colBreaks : s->rowBreaks;
        for (qsizetype i = list.size(); i-- > 0;)
            if (qint64(list[i].before) == from) list.removeAt(i);
        if (to < 0) return;
        for (PageBreak &b : list)
            if (qint64(b.before) == to) {
                b.manual = true;
                return;
            }
        if (list.size() >= jp::office::sheet::kMaxPageBreaks) return;
        PageBreak b;
        b.before = quint32(to);
        b.max = column ? 1048575 : 16383;
        list.append(b);
        std::sort(list.begin(), list.end(), [](const PageBreak &x, const PageBreak &y) { return x.before < y.before; });
    });
}

// An edge of a block of printed cells was dragged: the print area follows (the cells that printed of themselves become the area, with that edge moved).
void CrunchWindow::printEdgeMoved(int area, int side, qint64 to)
{
    const std::vector<grid::GridRange> shown = m_view->breakAreas();
    if (area < 0 || size_t(area) >= shown.size() || to < 0) return;
    changePage(tr("Print area"), [=](PageSetup *s) {
        QVector<CellRange> areas;
        if (size_t(s->printArea.size()) == shown.size()) {
            areas = s->printArea;
        } else {
            for (const grid::GridRange &g : shown) {
                CellRange r;
                r.row1 = quint32(g.row1);
                r.col1 = quint32(g.col1);
                r.row2 = quint32(g.row2);
                r.col2 = quint32(g.col2);
                r.valid = true;
                areas.append(r);
            }
        }
        CellRange &r = areas[area];
        switch (side) {
        case 0: r.row1 = quint32(to); break;
        case 1: r.row2 = quint32(to); break;
        case 2: r.col1 = quint32(to); break;
        default: r.col2 = quint32(to); break;
        }
        if (r.row1 > r.row2 || r.col1 > r.col2) return;
        s->printArea = areas;
    });
}

} // namespace jp
