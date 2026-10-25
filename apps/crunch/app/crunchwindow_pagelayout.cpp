// The Page Layout view: the sheet shown as the pages it prints on, one below the other, with their margins, rulers, and the header and footer in the margins. The cells stay editable (a click
// chooses a cell, typing and F2 and a double click edit it, in a strip over the cell on its page), and a click on a part of a header or footer opens a strip to type its words, with the
// Header and Footer tab of the ribbon to put in the page number, the date, a picture, and the rest. The window keeps the job of the sheet's pages to draw them; the sliced work that cuts
// them is the page lines' (crunchwindow_print.cpp).

#include "app/crunchwindow.h"

#include "app/pagesetupdialog.h"
#include "app/ribbon.h"
#include "core/editsession.h"
#include "core/printjob.h"
#include "grid/gridview.h"
#include "grid/pagelayoutview.h"
#include "sheet/headerfooter.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QFileDialog>
#include <QFocusEvent>
#include <QLabel>
#include <QSlider>
#include <QKeyEvent>
#include <QLocale>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace jp {

using jp::office::sheet::HeaderFooter;
using jp::office::sheet::PageSetup;

namespace {

// The strip a part of a header or footer is typed in: Escape gives it up, Ctrl and Enter finish it, and so does going to something else (a window of another program, or a menu, does not).
class ZoneEditor : public QPlainTextEdit {
public:
    using QPlainTextEdit::QPlainTextEdit;
    std::function<void(bool)> done;

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Escape) {
            if (done) done(false);
            return;
        }
        if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) && (e->modifiers() & Qt::ControlModifier)) {
            if (done) done(true);
            return;
        }
        QPlainTextEdit::keyPressEvent(e);
    }
    void focusOutEvent(QFocusEvent *e) override
    {
        QPlainTextEdit::focusOutEvent(e);
        if (e->reason() == Qt::ActiveWindowFocusReason || e->reason() == Qt::PopupFocusReason) return;
        if (done) done(true);
    }
};

// The pages of a job, for the view.
class JobPages : public grid::PageLayoutSource {
public:
    JobPages(std::shared_ptr<crunch::PrintJob> job, int sheet) : m_job(std::move(job)), m_sheet(sheet) {}
    int pageCount() const override { return m_job->pageCount(); }
    QSizeF pageSize(int page) const override { return m_job->pageSize(page); }
    QRectF headerBand(int page) const override
    {
        const QRectF s = side(page);
        return QRectF(s.left(), 0, s.width(), m_job->paperOf(page).top);
    }
    QRectF footerBand(int page) const override
    {
        const QRectF s = side(page);
        const jp::grid::PaperBox box = m_job->paperOf(page);
        return QRectF(s.left(), box.height - box.bottom, s.width(), box.bottom);
    }
    void paintPage(QPainter *painter, int page) override { m_job->paint(painter, page); }
    bool cellRect(int page, qint64 row, qint64 col, QRectF *rect) override { return m_job->cellRect(page, row, col, rect); }
    bool cellAt(int page, const QPointF &point, qint64 *row, qint64 *col) override { return m_job->cellAt(page, point, row, col); }
    bool rangeRect(int page, const grid::GridRange &r, QRectF *rect) override { return m_job->rangeRect(page, r.row1, r.col1, r.row2, r.col2, rect); }
    int pageOfCell(qint64 row, qint64 col) override { return m_job->pageOfCell(m_sheet, row, col); }
    crunch::PrintJob *job() const { return m_job.get(); }

private:
    // Where the header and footer sit across the paper: between the margins, or (when they are not lined up with them) from a quarter inch in.
    QRectF side(int page) const
    {
        const jp::grid::PaperBox box = m_job->paperOf(page);
        const PageSetup *setup = m_job->setupOfPage(page);
        const bool aligned = !setup || setup->headerFooter.alignWithMargins;
        const double left = aligned ? box.left : std::min(box.left, 18.0), right = aligned ? box.right : std::min(box.right, 18.0);
        return QRectF(left, 0, std::max(1.0, box.width - left - right), 0);
    }
    std::shared_ptr<crunch::PrintJob> m_job;
    int m_sheet;
};

} // namespace

void CrunchWindow::createPageLayoutActions()
{
    QAction *layout = mk(QStringLiteral("view.pageLayout"), tr("Page Layout"), QStringLiteral("file-text"), QKeySequence(), [this] { setViewKind(2); }, true);
    if (QAction *normal = act(QStringLiteral("view.normal")); normal && normal->actionGroup()) normal->actionGroup()->addAction(layout);
    m_opsActions << QStringLiteral("view.pageLayout");
    // the Header and Footer tab: what is put in the part being typed in, and the choices that are the whole header's
    struct Code {
        const char *id, *text, *icon, *code;
    };
    static const Code codes[] = {{"hf.pageNumber", QT_TR_NOOP("This Page's Number"), "hash", "&P"}, {"hf.pageCount", QT_TR_NOOP("Pages in All"), "files", "&N"},
                                 {"hf.date", QT_TR_NOOP("Date Printed"), "calendar", "&D"},  {"hf.time", QT_TR_NOOP("Time Printed"), "clock", "&T"},
                                 {"hf.filePath", QT_TR_NOOP("Folder"), "folder", "&Z"},   {"hf.fileName", QT_TR_NOOP("Workbook File"), "file-text", "&F"},
                                 {"hf.sheetName", QT_TR_NOOP("Sheet's Name"), "table-2", "&A"}};
    for (const Code &c : codes) {
        const QString code = QString::fromLatin1(c.code);
        mk(QString::fromLatin1(c.id), tr(c.text), QString::fromLatin1(c.icon), QKeySequence(), [this, code] { insertHeaderCode(code); });
    }
    mk(QStringLiteral("hf.picture"), tr("Add Picture"), QStringLiteral("image"), QKeySequence(), [this] { insertHeaderPicture(); });
    auto flag = [this](const QString &id, const QString &text, const QString &undo, bool HeaderFooter::*member) {
        mk(id, text, QString(), QKeySequence(), [this, id, undo, member] {
            const bool on = act(id)->isChecked();
            changePage(undo, [member, on](PageSetup *s) { s->headerFooter.*member = on; });
        }, true);
        m_opsActions << id;
    };
    flag(QStringLiteral("hf.differentFirst"), tr("First Page on Its Own"), tr("Header and footer of the first page"), &HeaderFooter::differentFirst);
    flag(QStringLiteral("hf.differentOddEven"), tr("Odd and Even on Their Own"), tr("Header and footer of odd and even pages"), &HeaderFooter::differentOddEven);
    flag(QStringLiteral("hf.scaleWithDoc"), tr("Grow with the Cells"), tr("Header and footer size"), &HeaderFooter::scaleWithDoc);
    flag(QStringLiteral("hf.alignWithMargins"), tr("Line Up with Margins"), tr("Header and footer alignment"), &HeaderFooter::alignWithMargins);
}

// The view of the pages is made with the window, hidden until the view is chosen.
void CrunchWindow::buildPageLayoutView(QWidget *parent, QVBoxLayout *into)
{
    m_pageView = new grid::PageLayoutView(parent);
    m_pageView->setObjectName(QStringLiteral("pageLayoutView"));
    m_pageView->setAccessibleName(tr("Pages"));
    m_pageView->setAccessibleDescription(tr("The sheet as the pages it prints on. Click a cell to choose it; click the top or bottom margin of a page to type its header or footer."));
    m_pageView->setMetric(QLocale().measurementSystem() == QLocale::MetricSystem);
    m_pageView->hide();
    into->addWidget(m_pageView, 1);
    connect(m_pageView, &grid::PageLayoutView::cellPressed, this, [this](int, qint64 row, qint64 col, Qt::KeyboardModifiers mods) {
        if (!commitEdit(CellEditor::Way::Stay)) return;
        endZoneEdit(true);
        const grid::Selection &sel = m_view->selection();
        if (mods & Qt::ShiftModifier) {
            m_view->selectRange(std::min<qint64>(sel.activeRow, row), std::min<qint64>(sel.activeCol, col), std::max<qint64>(sel.activeRow, row), std::max<qint64>(sel.activeCol, col));
        } else {
            m_pressRow = row;
            m_pressCol = col;
            m_view->selectCell(row, col);
        }
    });
    connect(m_pageView, &grid::PageLayoutView::cellDragged, this, [this](int, qint64 row, qint64 col) {
        m_view->selectRange(std::min(m_pressRow, row), std::min(m_pressCol, col), std::max(m_pressRow, row), std::max(m_pressCol, col));
    });
    connect(m_pageView, &grid::PageLayoutView::cellReleased, this, [this] { Q_EMIT m_view->selectionSettled(); });
    connect(m_pageView, &grid::PageLayoutView::cellDoubleClicked, this, [this](int, qint64 row, qint64 col) {
        if (!commitEdit(CellEditor::Way::Stay)) return;
        m_view->selectCell(row, col);
        Q_EMIT m_view->editRequested();
    });
    connect(m_pageView, &grid::PageLayoutView::zoneClicked, this, &CrunchWindow::zoneClicked);
    // the keys are the cells': the grid, hidden behind, does what they say
    connect(m_pageView, &grid::PageLayoutView::keyPressed, this, [this](QKeyEvent *e) {
        QCoreApplication::sendEvent(m_view, e);
    });
    connect(m_pageView, &grid::PageLayoutView::viewChanged, this, [this] { placeEditor(); });
    connect(m_pageView, &grid::PageLayoutView::zoomChanged, this, [this](double z) {
        placeEditor();
        m_zoomLabel->setText(QStringLiteral("%1%").arg(std::lround(z * 100)));
        const QSignalBlocker block(m_zoomSlider);
        m_zoomSlider->setValue(int(std::lround(std::log(z) / std::log(1.05))));
        if (m_viewKind == 2 && m_session && m_sheet >= 0) m_session->setSheetView(m_sheet, 2, int(std::lround(z * 100)));
    });
    connect(m_view, &grid::GridView::selectionChanged, this, [this] { syncPageViewSelection(); });
}

double CrunchWindow::viewZoom() const { return m_viewKind == 2 && m_pageView ? m_pageView->zoom() : m_view->zoom(); }

void CrunchWindow::setViewZoom(double zoom)
{
    if (m_viewKind == 2 && m_pageView) m_pageView->setZoom(zoom);
    else m_view->setZoom(zoom);
}

void CrunchWindow::syncPageViewSelection()
{
    if (m_viewKind != 2 || !m_pageView) return;
    const grid::Selection &sel = m_view->selection();
    m_pageView->setSelection(sel.range, sel.activeRow, sel.activeCol);
    if (sel.activeRow >= 0) m_pageView->ensureCellVisible(sel.activeRow, sel.activeCol);
    placeEditor();
}

// The pages are shown (or not): the view takes the grid's place, the strip for a cell is over the pages, and the keys the view gets go to the grid.
void CrunchWindow::showPageLayout(bool on)
{
    if (!m_pageView) return;
    if (on == m_pageView->isVisible()) return;
    endZoneEdit(true);
    m_view->setVisible(!on);
    m_pageView->setVisible(on);
    m_view->setFocusProxy(on ? m_pageView : nullptr);
    m_cellEditor->setParent(on ? m_pageView->viewport() : m_view->viewport());
    m_cellEditor->hide();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Header and Footer"), false);
    if (on) {
        syncPageViewSelection();
        m_pageView->setFocus(Qt::OtherFocusReason);
    } else {
        dropLayoutJob();
        m_view->setFocus(Qt::OtherFocusReason);
    }
    if (m_edit.active) placeEditor();
}

void CrunchWindow::dropLayoutJob()
{
    if (m_pageView) m_pageView->setSource(nullptr);
    m_layoutPages.reset();
    m_layoutJob.reset();
    m_layoutSheet = -1;
}

// The job that cut the sheet in view into pages is the view's to draw them from; the one before it is let go of once the view has the new.
void CrunchWindow::takeLayoutJob(std::unique_ptr<crunch::PrintJob> job)
{
    if (!m_pageView || !job) return;
    std::shared_ptr<crunch::PrintJob> shared(std::move(job));
    auto pages = std::make_shared<JobPages>(shared, m_sheet);
    m_pageView->setSource(pages.get());
    m_layoutPages = pages;
    m_layoutJob = shared;
    m_layoutSheet = m_sheet;
    syncPageViewSelection();
}

// ---- the header and footer, typed in place

void CrunchWindow::zoneClicked(int page, bool header, int zone)
{
    if (m_viewKind != 2 || !m_session || !m_layoutPages || !commitEdit(CellEditor::Way::Stay)) return;
    endZoneEdit(true);
    QString why;
    if (!m_session->canEdit(&why)) {
        notice(tr("Header and Footer"), why);
        return;
    }
    auto *pages = static_cast<JobPages *>(m_layoutPages.get());
    // (what the sheet says now, not what the pages were cut with: the pages follow a change a moment after it)
    const PageSetup now = m_session->pages().sheet(m_sheet);
    const HeaderFooter &hf = now.headerFooter;
    const bool first = hf.differentFirst && pages->job()->isFirstOfSheet(page);
    const bool even = !first && hf.differentOddEven && pages->job()->numberOf(page) % 2 == 0;
    const QString &text = header ? (first ? hf.firstHeader : even ? hf.evenHeader : hf.oddHeader) : (first ? hf.firstFooter : even ? hf.evenFooter : hf.oddFooter);
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(text, &parts[0], &parts[1], &parts[2]);
    const QRect r = m_pageView->zoneRect(page, header, zone);
    if (r.isEmpty()) return;
    m_zone = ZoneEdit();
    m_zone.page = page;
    m_zone.header = header;
    m_zone.zone = zone;
    m_zone.form = first ? 2 : even ? 1 : 0;
    auto *editor = new ZoneEditor(m_pageView->viewport());
    editor->setObjectName(QStringLiteral("headerFooterZoneEditor"));
    editor->setAccessibleName(header ? tr("Header text") : tr("Footer text"));
    editor->setAccessibleDescription(tr("The words of this part of the header or footer. The buttons of the Header and Footer tab put in the page number, the date, and more. Ctrl and Enter finish; Escape gives up."));
    editor->setPlainText(parts[zone]);
    editor->setTabChangesFocus(true);
    editor->done = [this](bool keep) { endZoneEdit(keep); };
    // (at least a line or two high and wide enough to type in, within the view)
    const QRect vp = m_pageView->viewport()->rect();
    const int w = std::max(r.width(), 160), h = std::max(r.height(), 44);
    int x = std::min(r.left(), std::max(0, vp.width() - w)), y = header ? r.top() : std::max(0, r.bottom() - h + 1);
    editor->setGeometry(x, y, w, h);
    editor->show();
    editor->setFocus(Qt::OtherFocusReason);
    editor->moveCursor(QTextCursor::End);
    m_zoneEditor = editor;
    if (m_ribbon) {
        m_ribbon->setContextVisible(QStringLiteral("Header and Footer"), true);
        if (RibbonTab *tab = m_ribbon->tab(QStringLiteral("Header and Footer"))) m_ribbon->showTab(tab);
    }
    updateHeaderFooterActions();
}

// The strip is finished: what was typed becomes the header (or footer) as one step of Undo, or is given up.
void CrunchWindow::endZoneEdit(bool keep)
{
    if (!m_zoneEditor) return;
    QPlainTextEdit *editor = m_zoneEditor;
    m_zoneEditor = nullptr;
    const QString typed = editor->toPlainText();
    editor->hide();
    editor->deleteLater();
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Header and Footer"), false);
    const ZoneEdit zone = m_zone;
    m_zone = ZoneEdit();
    if (!keep || !m_session) {
        if (m_viewKind == 2 && m_pageView) m_pageView->setFocus(Qt::OtherFocusReason);
        return;
    }
    const PageSetup now = m_session->pages().sheet(m_sheet);
    const HeaderFooter &hf = now.headerFooter;
    const QString current = zone.header ? (zone.form == 2 ? hf.firstHeader : zone.form == 1 ? hf.evenHeader : hf.oddHeader) : (zone.form == 2 ? hf.firstFooter : zone.form == 1 ? hf.evenFooter : hf.oddFooter);
    QString parts[3];
    jp::office::sheet::splitHeaderFooter(current, &parts[0], &parts[1], &parts[2]);
    parts[zone.zone] = typed;
    const QString made = jp::office::sheet::composeHeaderFooter(parts[0], parts[1], parts[2]).left(jp::office::sheet::kMaxHeaderFooterChars);
    const bool pictureNeeded = zone.hasPicture && jp::office::sheet::headerFooterHasPicture(made, zone.zone == 0 ? QLatin1Char('L') : zone.zone == 1 ? QLatin1Char('C') : QLatin1Char('R'));
    if (made != current || pictureNeeded) {
        QString place = QString(zone.zone == 0 ? QLatin1Char('L') : zone.zone == 1 ? QLatin1Char('C') : QLatin1Char('R')) + (zone.header ? QLatin1Char('H') : QLatin1Char('F')) + (zone.form == 2 ? QStringLiteral("F") : zone.form == 1 ? QStringLiteral("E") : QString());
        changePage(zone.header ? tr("Header") : tr("Footer"), [&](PageSetup *s) {
            QString *text = zone.header ? (zone.form == 2 ? &s->headerFooter.firstHeader : zone.form == 1 ? &s->headerFooter.evenHeader : &s->headerFooter.oddHeader)
                                        : (zone.form == 2 ? &s->headerFooter.firstFooter : zone.form == 1 ? &s->headerFooter.evenFooter : &s->headerFooter.oddFooter);
            *text = made;
            if (pictureNeeded) {
                for (qsizetype i = s->headerFooter.pictures.size(); i-- > 0;)
                    if (s->headerFooter.pictures[i].place.compare(place, Qt::CaseInsensitive) == 0) s->headerFooter.pictures.removeAt(i);
                if (s->headerFooter.pictures.size() < jp::office::sheet::kMaxHeaderPictures) {
                    jp::office::sheet::HeaderPicture p = zone.picture;
                    p.place = place;
                    s->headerFooter.pictures.append(p);
                }
            }
            jp::office::sheet::dropUnusedHeaderPictures(&s->headerFooter);
        });
    }
    if (m_viewKind == 2 && m_pageView) m_pageView->setFocus(Qt::OtherFocusReason);
}

void CrunchWindow::insertHeaderCode(const QString &code)
{
    if (!m_zoneEditor) {
        setStatus(tr("Click the top or bottom margin of a page in the Page Layout view first, then choose what to put in."));
        return;
    }
    m_zoneEditor->insertPlainText(code);
    m_zoneEditor->setFocus(Qt::OtherFocusReason);
}

// A picture from a file goes in the part being typed in: it is kept with the strip and put in the sheet's setup with the text.
void CrunchWindow::insertHeaderPicture()
{
    if (!m_zoneEditor) {
        setStatus(tr("Click the top or bottom margin of a page in the Page Layout view first, then choose what to put in."));
        return;
    }
    QPlainTextEdit *editor = m_zoneEditor;
    const QString path = m_pictureAsker ? m_pictureAsker() : QFileDialog::getOpenFileName(this, tr("Choose a Picture"), QString(), tr("Pictures (*.png *.jpg *.jpeg *.gif *.bmp)"));
    if (m_zoneEditor != editor) return;   // (the strip was finished while the picture was being picked)
    editor->setFocus(Qt::OtherFocusReason);
    if (path.isEmpty()) return;
    jp::office::sheet::HeaderPicture picture;
    QString why;
    if (!loadHeaderPicture(path, &picture, &why)) {
        notice(tr("Picture"), why);
        return;
    }
    m_zone.picture = picture;
    m_zone.hasPicture = true;
    const QChar part = m_zone.zone == 0 ? QLatin1Char('L') : m_zone.zone == 1 ? QLatin1Char('C') : QLatin1Char('R');
    if (!jp::office::sheet::headerFooterHasPicture(jp::office::sheet::composeHeaderFooter(part == QLatin1Char('L') ? editor->toPlainText() : QString(), part == QLatin1Char('C') ? editor->toPlainText() : QString(),
                                                                                         part == QLatin1Char('R') ? editor->toPlainText() : QString()),
                                                   part))
        editor->insertPlainText(QStringLiteral("&G"));
}

// The choices of the Header and Footer tab show the sheet's setup.
void CrunchWindow::updateHeaderFooterActions()
{
    const bool ready = m_session && m_loaded && m_sheet >= 0 && m_sheet < m_session->pages().sheetCount();
    const HeaderFooter hf = ready ? m_session->pages().sheet(m_sheet).headerFooter : HeaderFooter();
    auto set = [this](const char *id, bool on) {
        if (QAction *a = act(QString::fromLatin1(id))) {
            const QSignalBlocker block(a);
            a->setChecked(on);
        }
    };
    set("hf.differentFirst", hf.differentFirst);
    set("hf.differentOddEven", hf.differentOddEven);
    set("hf.scaleWithDoc", hf.scaleWithDoc);
    set("hf.alignWithMargins", hf.alignWithMargins);
}

} // namespace jp
