// How the sheets print, as the window changes it: the commands of the Page Layout tab (margins, orientation, paper, print area, breaks, titles, scale to fit, sheet options), and the
// Page Setup dialog. Each change is one step of Undo (core/editsession.h: setPageSetup), made on the sheet in view; what the commands show (the check marks, the boxes of the scale) is
// what the sheet says now.

#include "app/crunchwindow.h"

#include "app/pagesetupdialog.h"
#include "app/ribbonbuilder.h"
#include "core/editsession.h"
#include "grid/gridview.h"
#include "sheet/pagesetup.h"

#include <QAction>
#include <QComboBox>
#include <QFileInfo>
#include <QLocale>
#include <QSpinBox>

#include <algorithm>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::PageBreak;
using jp::office::sheet::PageMargins;
using jp::office::sheet::PageOrientation;
using jp::office::sheet::PageSetup;

namespace {

// The three sets of margins of the menu (inches).
PageMargins standardMargins() { return {0.7, 0.7, 0.75, 0.75, 0.3, 0.3}; }
PageMargins wideMargins() { return {1.0, 1.0, 1.0, 1.0, 0.5, 0.5}; }
PageMargins narrowMargins() { return {0.25, 0.25, 0.75, 0.75, 0.3, 0.3}; }

bool sameMargins(const PageMargins &a, const PageMargins &b) { return a == b; }

// The pages across (down) a box can hold: "Automatic" and one to ten.
constexpr int kFitChoices = 10;

} // namespace

void CrunchWindow::createPageActions()
{
    auto page = [this](const QString &id, const QString &text, const QString &icon, const std::function<void()> &fn, bool checkable = false) {
        QAction *a = mk(id, text, icon, QKeySequence(), fn, checkable, false);
        m_opsActions << id;
        m_pageActions << id;
        return a;
    };
    page(QStringLiteral("pagesetup.dialog"), tr("Page Setup…"), QStringLiteral("file-text"), [this] { pageSetupDialog(0); });
    page(QStringLiteral("margins.custom"), tr("Custom Margins…"), QString(), [this] { pageSetupDialog(1); });
    page(QStringLiteral("margins.standard"), tr("Standard"), QString(), [this] { changePage(tr("Margins"), [](PageSetup *s) { s->margins = standardMargins(); }); }, true);
    page(QStringLiteral("margins.wide"), tr("Wide"), QString(), [this] { changePage(tr("Margins"), [](PageSetup *s) { s->margins = wideMargins(); }); }, true);
    page(QStringLiteral("margins.narrow"), tr("Narrow"), QString(), [this] { changePage(tr("Margins"), [](PageSetup *s) { s->margins = narrowMargins(); }); }, true);
    page(QStringLiteral("orient.portrait"), tr("Portrait"), QString(), [this] { changePage(tr("Page orientation"), [](PageSetup *s) { s->orientation = PageOrientation::Portrait; }); }, true);
    page(QStringLiteral("orient.landscape"), tr("Landscape"), QString(), [this] { changePage(tr("Page orientation"), [](PageSetup *s) { s->orientation = PageOrientation::Landscape; }); }, true);
    for (const int id : jp::office::sheet::commonPaperSizes()) {
        page(QStringLiteral("size.%1").arg(id), jp::office::sheet::paperSizeName(id), QString(), [this, id] {
            changePage(tr("Paper size"), [id](PageSetup *s) {
                s->paperSize = id;
                s->paperWidth = s->paperHeight = 0;
            });
        }, true);
    }
    page(QStringLiteral("size.more"), tr("More Paper Sizes…"), QString(), [this] { pageSetupDialog(0); });
    page(QStringLiteral("printarea.set"), tr("Set Print Area"), QString(), [this] {
        const CellRange r = selectedRange();
        if (!r.valid) return;
        changePage(tr("Print area"), [r](PageSetup *s) { s->printArea = {r}; });
    });
    page(QStringLiteral("printarea.add"), tr("Add Selection to Print Area"), QString(), [this] {
        const CellRange r = selectedRange();
        if (!r.valid) return;
        changePage(tr("Print area"), [r](PageSetup *s) {
            if (s->printArea.size() < jp::office::sheet::kMaxPrintAreas && !s->printArea.contains(r)) s->printArea.append(r);
        });
    });
    page(QStringLiteral("printarea.clear"), tr("Clear Print Area"), QString(), [this] { changePage(tr("Print area"), [](PageSetup *s) { s->printArea.clear(); }); });
    page(QStringLiteral("breaks.insert"), tr("Insert Page Break"), QString(), [this] {
        const grid::Selection &sel = m_view->selection();
        const qint64 row = sel.range.row1, col = sel.range.col1;
        const bool rows = sel.whole != grid::Selection::Whole::Columns && sel.whole != grid::Selection::Whole::All && row > 0;
        const bool cols = sel.whole != grid::Selection::Whole::Rows && sel.whole != grid::Selection::Whole::All && col > 0;
        if (!rows && !cols) {
            setStatus(tr("A page break goes above a row or left of a column. Select a cell that is not in the first row and column, or a whole row or column."));
            return;
        }
        changePage(tr("Insert page break"), [=](PageSetup *s) {
            auto put = [](QVector<PageBreak> *list, qint64 at, quint32 max) {
                for (PageBreak &b : *list)
                    if (qint64(b.before) == at) {
                        b.manual = true;
                        return;
                    }
                if (list->size() >= jp::office::sheet::kMaxPageBreaks) return;
                PageBreak b;
                b.before = quint32(at);
                b.max = max;
                list->append(b);
                std::sort(list->begin(), list->end(), [](const PageBreak &x, const PageBreak &y) { return x.before < y.before; });
            };
            if (rows) put(&s->rowBreaks, row, 16383);
            if (cols) put(&s->colBreaks, col, 1048575);
        });
    });
    page(QStringLiteral("breaks.remove"), tr("Remove Page Break"), QString(), [this] {
        const grid::Selection &sel = m_view->selection();
        const qint64 row = sel.range.row1, col = sel.range.col1;
        const PageSetup &now = m_session->pages().sheet(m_sheet);
        auto has = [](const QVector<PageBreak> &list, qint64 at) {
            for (const PageBreak &b : list)
                if (qint64(b.before) == at) return true;
            return false;
        };
        const bool rows = sel.whole != grid::Selection::Whole::Columns && has(now.rowBreaks, row);
        const bool cols = sel.whole != grid::Selection::Whole::Rows && has(now.colBreaks, col);
        if (!rows && !cols) {
            setStatus(tr("There is no page break above or to the left of the selected cell."));
            return;
        }
        changePage(tr("Remove page break"), [=](PageSetup *s) {
            if (rows) s->rowBreaks.erase(std::remove_if(s->rowBreaks.begin(), s->rowBreaks.end(), [row](const PageBreak &b) { return qint64(b.before) == row; }), s->rowBreaks.end());
            if (cols) s->colBreaks.erase(std::remove_if(s->colBreaks.begin(), s->colBreaks.end(), [col](const PageBreak &b) { return qint64(b.before) == col; }), s->colBreaks.end());
        });
    });
    page(QStringLiteral("breaks.reset"), tr("Reset All Page Breaks"), QString(), [this] {
        changePage(tr("Reset page breaks"), [](PageSetup *s) {
            s->rowBreaks.clear();
            s->colBreaks.clear();
        });
    });
    page(QStringLiteral("titles.dialog"), tr("Print Titles"), QStringLiteral("frame"), [this] { pageSetupDialog(3); });
    page(QStringLiteral("sheetopt.gridPrint"), tr("Print Gridlines"), QString(), [this] {
        const bool on = act(QStringLiteral("sheetopt.gridPrint"))->isChecked();
        changePage(on ? tr("Print gridlines") : tr("Don't print gridlines"), [on](PageSetup *s) { s->gridLines = on; });
    }, true);
    page(QStringLiteral("sheetopt.headPrint"), tr("Print Headings"), QString(), [this] {
        const bool on = act(QStringLiteral("sheetopt.headPrint"))->isChecked();
        changePage(on ? tr("Print headings") : tr("Don't print headings"), [on](PageSetup *s) { s->headings = on; });
    }, true);
    // the same two switches as the View tab's, with the words this group needs
    QAction *gridView = mk(QStringLiteral("sheetopt.gridView"), tr("Show Gridlines"), QString(), QKeySequence(), [this] { act(QStringLiteral("view.gridlines"))->trigger(); }, true);
    QAction *headView = mk(QStringLiteral("sheetopt.headView"), tr("Show Headings"), QString(), QKeySequence(), [this] { act(QStringLiteral("view.headings"))->trigger(); }, true);
    gridView->setChecked(true);
    headView->setChecked(true);
    connect(act(QStringLiteral("view.gridlines")), &QAction::toggled, gridView, &QAction::setChecked);
    connect(act(QStringLiteral("view.headings")), &QAction::toggled, headView, &QAction::setChecked);
    // File > Print: the Print screen
    mk(QStringLiteral("file.print"), tr("Print"), QStringLiteral("printer"), QKeySequence::Print, [this] { showFileScreen(QStringLiteral("print")); });
}

void CrunchWindow::pageRibbonParts(RibbonParts &parts)
{
    parts.launchers[QStringLiteral("pageSetup")] = [this] { pageSetupDialog(0); };
    auto fitBox = [this](bool across) {
        auto *box = new QComboBox;
        box->setObjectName(across ? QStringLiteral("fitWidthBox") : QStringLiteral("fitHeightBox"));
        box->setAccessibleName(across ? tr("Pages across to fit the sheet to") : tr("Pages down to fit the sheet to"));
        box->setToolTip(across ? tr("How many pages wide the sheet may be when it is shrunk to fit") : tr("How many pages tall the sheet may be when it is shrunk to fit"));
        box->addItem(tr("Automatic"), 0);
        for (int n = 1; n <= kFitChoices; ++n) box->addItem(n == 1 ? tr("1 page") : tr("%1 pages").arg(n), n);
        box->setMinimumWidth(96);
        (across ? m_fitWidthBox : m_fitHeightBox) = box;
        connect(box, &QComboBox::activated, this, [this, across](int i) {
            if (m_updatingPage || !m_session) return;
            const int n = (across ? m_fitWidthBox : m_fitHeightBox)->itemData(i).toInt();
            changePage(tr("Scale to fit"), [=](PageSetup *s) {
                // (a sheet that is not being fitted shows Automatic on both sides, whatever numbers the file keeps for them)
                if (!(s->fitToPage && (s->fitToWidth > 0 || s->fitToHeight > 0))) s->fitToWidth = s->fitToHeight = 0;
                if (across) s->fitToWidth = n;
                else s->fitToHeight = n;
                // a fit with an automatic side and an automatic side is the scale as it was: the fit is on when either side is asked for
                s->fitToPage = s->fitToWidth > 0 || s->fitToHeight > 0;
                if (!s->fitToPage) {
                    s->fitToWidth = 1;
                    s->fitToHeight = 1;
                }
            });
        });
        return box;
    };
    parts.widgets[QStringLiteral("fitWidth")] = [fitBox]() -> QWidget * { return fitBox(true); };
    parts.widgets[QStringLiteral("fitHeight")] = [fitBox]() -> QWidget * { return fitBox(false); };
    parts.widgets[QStringLiteral("scalePercent")] = [this]() -> QWidget * {
        auto *box = new QSpinBox;
        box->setObjectName(QStringLiteral("scaleBox"));
        box->setRange(jp::office::sheet::kMinPageScale, jp::office::sheet::kMaxPageScale);
        box->setSuffix(QStringLiteral(" %"));
        box->setValue(100);
        box->setKeyboardTracking(false);
        box->setAccessibleName(tr("Scale in percent"));
        box->setToolTip(tr("How big the cells print, in percent of full size. It works when Width and Height are both Automatic."));
        m_scaleBox = box;
        connect(box, &QSpinBox::valueChanged, this, [this](int v) {
            if (m_updatingPage || !m_session) return;
            changePage(tr("Scale"), [v](PageSetup *s) {
                s->scale = v;
                s->fitToPage = false;
            });
        });
        return box;
    };
}

void CrunchWindow::connectPageSetup()
{
    connect(m_session.get(), &crunch::EditSession::pageSetupChanged, this, [this](int sheet) {
        if (sheet == m_sheet) updatePageActions();
        if (m_breaksArmed) schedulePageBreaks();
    });
    // the lines where the pages begin follow the cells (the sizes, the rows and columns inserted, and what is typed are what the pages are cut by)
    connect(m_session.get(), &crunch::EditSession::sizesChanged, this, [this] {
        if (m_breaksArmed) schedulePageBreaks();
    });
    connect(m_session.get(), &crunch::EditSession::cellsMoved, this, [this] {
        if (m_breaksArmed) schedulePageBreaks();
    });
    // (and what is typed: a cell past the last one the pages ended at makes another page)
    connect(m_session.get(), &crunch::EditSession::cellsChanged, this, [this](int sheet, const std::vector<jp::grid::ValueStore::Item> &) {
        if (m_breaksArmed && sheet == m_sheet) schedulePageBreaks();
    });
    connect(m_session->undoStack(), &QUndoStack::indexChanged, this, [this] { updatePageActions(); });
    updatePageActions();
}

void CrunchWindow::updatePageActions()
{
    if (m_updatingPage) return;
    m_updatingPage = true;
    const bool ready = m_session && m_loaded && m_sheet >= 0 && m_sheet < m_session->pages().sheetCount();
    const PageSetup s = ready ? m_session->pages().sheet(m_sheet) : PageSetup();
    const bool open = ready && m_session->canEdit() && !m_chartSheetShown;
    auto check = [this](const QString &id, bool on) {
        if (QAction *a = act(id)) a->setChecked(on);
    };
    check(QStringLiteral("margins.standard"), sameMargins(s.margins, standardMargins()));
    check(QStringLiteral("margins.wide"), sameMargins(s.margins, wideMargins()));
    check(QStringLiteral("margins.narrow"), sameMargins(s.margins, narrowMargins()));
    check(QStringLiteral("orient.portrait"), !jp::office::sheet::isLandscape(s));
    check(QStringLiteral("orient.landscape"), jp::office::sheet::isLandscape(s));
    for (const int id : jp::office::sheet::commonPaperSizes()) check(QStringLiteral("size.%1").arg(id), s.paperSize == id);
    check(QStringLiteral("sheetopt.gridPrint"), s.gridLines);
    check(QStringLiteral("sheetopt.headPrint"), s.headings);
    if (QAction *a = act(QStringLiteral("printarea.clear"))) a->setEnabled(open && s.hasPrintArea());
    const bool fit = s.fitToPage && (s.fitToWidth > 0 || s.fitToHeight > 0);
    auto setFit = [&](QComboBox *box, int pages) {
        if (!box) return;
        int at = box->findData(pages);
        if (at < 0) {
            box->addItem(pages == 1 ? tr("1 page") : tr("%1 pages").arg(pages), pages);
            at = box->count() - 1;
        }
        box->setCurrentIndex(at);
        box->setEnabled(open);
    };
    setFit(m_fitWidthBox, fit ? s.fitToWidth : 0);
    setFit(m_fitHeightBox, fit ? s.fitToHeight : 0);
    if (m_scaleBox) {
        m_scaleBox->setValue(fit ? 100 : s.scale);
        m_scaleBox->setEnabled(open && !fit);
    }
    m_updatingPage = false;
    if (m_breaksArmed) schedulePageBreaks();
}

void CrunchWindow::changePage(const QString &text, const std::function<void(PageSetup *)> &change)
{
    if (!m_session || !commitEdit(CellEditor::Way::Stay)) return;
    if (m_session->pages().sheetCount() <= m_sheet || m_sheet < 0) return;
    PageSetup s = m_session->pages().sheet(m_sheet);
    change(&s);
    const crunch::EditSession::Outcome o = m_session->setPageSetup(m_sheet, s, text);
    if (!o.ok) notice(tr("Page Setup"), o.problem);
    updatePageActions();
}

void CrunchWindow::pageSetupDialog(int tab)
{
    if (!m_session || !commitEdit(CellEditor::Way::Stay)) return;
    if (m_sheet < 0 || m_sheet >= m_session->pages().sheetCount()) return;
    QString why;
    if (!m_session->canEdit(&why)) {
        notice(tr("Page Setup"), why);
        return;
    }
    PageSetupRequest request;
    request.setup = m_session->pages().sheet(m_sheet);
    request.sheetName = m_loaded->book->info().sheets.value(m_sheet).name;
    request.fileName = m_path.isEmpty() ? displayName() : QFileInfo(m_path).fileName();
    request.selectionText = m_view->rangeName();
    request.tab = tab;
    request.metric = QLocale().measurementSystem() == QLocale::MetricSystem;
    if (m_pageSetupAsker) {
        if (!m_pageSetupAsker(&request)) return;
    } else {
        PageSetupDialog dialog(request, this);
        if (dialog.exec() != QDialog::Accepted) return;
        request.setup = dialog.setup();
    }
    changePage(tr("Page Setup"), [&](PageSetup *s) { *s = request.setup; });
}

} // namespace jp
