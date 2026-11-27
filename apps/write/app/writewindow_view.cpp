// JeffWrite's View tab, the parts beyond the zoom: the Navigation pane and Focus mode.

#include "app/writefindpane.h"
#include "app/writewindow.h"

#include "app/navpane.h"
#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/reviewpane.h"
#include "app/ribbon.h"
#include "app/ruler.h"
#include "core/editops.h"
#include "core/blockmove.h"
#include "core/sectionmove.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QScreen>
#include <QScrollBar>
#include <QActionGroup>
#include <QDockWidget>
#include <QSplitter>
#include <QStatusBar>
#include <QTreeWidget>

namespace jp {

void WriteWindow::createViewExtraActions()
{
    mk("view.navigationPane", tr("Navigation Pane"), "panel-left", QKeySequence(), [this] { showNavigationPane(act("view.navigationPane")->isChecked()); }, true);
    // Page movement: the pages in a column, or side by side in one row.
    auto *movement = new QActionGroup(this);
    movement->addAction(mk("view.pageMovement.vertical", tr("Vertical"), "arrow-up-down", QKeySequence(), [this] { m_view->setSideToSide(false); }, true));
    movement->addAction(mk("view.pageMovement.sideToSide", tr("Side to Side"), "arrow-left-right", QKeySequence(), [this] { m_view->setSideToSide(true); }, true));
    act("view.pageMovement.vertical")->setChecked(true);
    mk("view.readMode", tr("Read Mode"), "book-open", QKeySequence(), [this] { setReadMode(act("view.readMode")->isChecked()); }, true);
    mk("view.split", tr("Split"), "rows-2", QKeySequence(), [this] { setSplit(act("view.split")->isChecked()); }, true);
    // View > Window: another window on the same file, the open windows set in rows or in a stack, and two windows side by side that scroll together.
    mk("win.new", tr("New Window"), "app-window", QKeySequence(), [this] {
        auto *w = new WriteWindow();
        w->setAttribute(Qt::WA_DeleteOnClose);
        if (!m_path.isEmpty()) w->openFile(m_path);
        w->show();
    });
    mk("win.arrange", tr("Arrange All"), "layout-panel-left", QKeySequence(), [] { arrangeAllWindows(); });
    mk("win.cascade", tr("Cascade"), "layers-2", QKeySequence(), [] { cascadeWindows(); });
    mk("view.sideBySide", tr("View Side by Side"), "columns-2", QKeySequence(), [this] { setSideBySide(act("view.sideBySide")->isChecked()); }, true);
    mk("view.resetWindowPosition", tr("Reset Window Position"), "layout-panel-left", QKeySequence(), [this] {
        if (m_sidePartner) {
            const QRect area = screen()->availableGeometry();
            setGeometry(area.left(), area.top(), area.width() / 2, area.height());
            m_sidePartner->setGeometry(area.left() + area.width() / 2, area.top(), area.width() / 2, area.height());
        }
    });
    mk("view.moveSectionUp", tr("Move Section Up"), "arrow-up", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Up), [this] { moveSectionAtCaret(true); });
    mk("view.moveSectionDown", tr("Move Section Down"), "arrow-down", QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_Down), [this] { moveSectionAtCaret(false); });
    mk("view.focus", tr("Focus"), "focus", QKeySequence(Qt::Key_F11), [this] { setFocusMode(!m_focusMode); });
}

// Why the commands that move a section can't work now, empty where they can (in Outline view they move the selected heading, as the same keys always did); `heading` gets the position of the
// heading of the section the caret is in.
QString WriteWindow::sectionMoveBlocker(int *heading) const
{
    if (heading) *heading = -1;
    if (!m_doc) return QString();
    if (m_outlineOn) return isReadOnly() ? QCoreApplication::translate("AppWindow", "This file is open for reading only. Use Save a Copy, or Save As, to make a copy you can change.") : QString();
    if (m_doc->protection() && m_doc->protection()->enforced) return QCoreApplication::translate("jp::write::BlockMove", "This document is protected, so a section can't be moved.");
    if (isReadOnly()) return QCoreApplication::translate("AppWindow", "This file is open for reading only. Use Save a Copy, or Save As, to make a copy you can change.");
    // The caret is in the text of the document: not in a header, a footer, a note, a box of text, a comment, or the endnotes that are set after the text.
    const bool inComment = m_review && focusWidget() && m_review->isAncestorOf(focusWidget());
    if (m_view->inHeaderFooter() || m_view->inNote() || m_view->editingShapeText() || inComment || m_view->cursor().document() != m_doc->story() || write::inEndnoteWords(m_view->cursor()))
        return tr("Sections can be moved only from the main text of the document, not from a header, footer, note, text box, or comment.");
    const int at = write::headingAtOrAbove(*m_doc, m_view->cursor().position());
    if (at < 0) return tr("Put the caret in a heading, or under one, to move its section.");
    if (heading) *heading = at;
    return QString();
}

// The two commands are off, and say why in their tooltips, where they would refuse.
void WriteWindow::refreshSectionMoveUi()
{
    const QString why = m_doc ? sectionMoveBlocker() : QString();
    for (const char *id : {"view.moveSectionUp", "view.moveSectionDown"}) {
        QAction *a = act(QString::fromLatin1(id));
        if (!a) continue;
        const QString name = QString(a->text()).remove(QLatin1Char('&'));
        a->setEnabled(m_doc && why.isEmpty());
        a->setToolTip(jp::plainToolTip(why.isEmpty() ? tr("%1 (%2)").arg(name, a->shortcut().toString(QKeySequence::NativeText)) : why));
        a->setStatusTip(jp::plainToolTip(why));
    }
}

void WriteWindow::moveSectionAtCaret(bool up)
{
    if (!m_doc) return;
    if (m_outlineOn) {
        act(up ? "outline.moveUp" : "outline.moveDown")->trigger();
        return;
    }
    int heading = -1;
    const QString blocked = sectionMoveBlocker(&heading);
    if (!blocked.isEmpty()) {
        statusBar()->showMessage(blocked, 6000);
        return;
    }
    const int wasAnchor = m_view->cursor().anchor(), wasPosition = m_view->cursor().position();   // (numbers: a cursor follows the edit, and the move collapses it)
    write::SectionRange range;
    if (!write::sectionAt(*m_doc, heading, &range)) return;
    QString why;
    int movedTo = -1;
    if (!write::moveSectionBy(*m_doc, heading, up, &why, &movedTo)) {
        if (!why.isEmpty()) statusBar()->showMessage(why, 6000);
        return;
    }
    // The caret (and the selection) stays at the same place in the same text.
    m_view->showCursor(write::cursorAfterMove(m_doc->story(), wasAnchor, wasPosition, range.first, range.end - range.first, movedTo));
    if (m_nav) m_nav->rescan();
}

void WriteWindow::showNavigationPane(bool show)
{
    if (show && !m_nav) {
        m_navDock = new QDockWidget(tr("Navigation"), this);
        m_navDock->setObjectName(QStringLiteral("navigationDock"));
        m_navDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_nav = new NavigationPane(this, m_navDock);
        m_navDock->setWidget(m_nav);
        addDockWidget(Qt::LeftDockWidgetArea, m_navDock);
        resizeDocks({m_navDock}, {260}, Qt::Horizontal);
        connect(m_navDock, &QDockWidget::visibilityChanged, this, [this](bool visible) {
            act("view.navigationPane")->setChecked(visible);
            if (visible && m_nav) m_nav->refresh();
        });
    }
    if (!m_navDock) return;
    m_navDock->setVisible(show);
    act("view.navigationPane")->setChecked(show);
    if (show) {
        m_nav->refresh();
        m_nav->headings()->setFocus();
    } else {
        m_view->setFocus(Qt::OtherFocusReason);
    }
}

// Focus: the ribbon, the ruler, the status bar, and the panes out of the way, the window full screen; the same command (or Esc) brings them back.
void WriteWindow::setFocusMode(bool on)
{
    if (m_focusMode == on) return;
    if (!on && m_readMode) {
        setReadMode(false);
        return;
    }
    m_focusMode = on;
    syncVerticalRuler();
    if (on) {
        m_focusWas.fullScreen = isFullScreen();
        m_focusWas.ribbon = m_ribbon->isVisible();
        m_focusWas.ruler = m_ruler->isVisible();
        m_focusWas.status = statusBar()->isVisible();
        m_focusWas.docks.clear();
        for (QDockWidget *d : findChildren<QDockWidget *>())
            if (d->isVisible()) {
                m_focusWas.docks << d;
                d->hide();
            }
        m_ribbon->hide();
        m_ruler->hide();
        statusBar()->hide();
        showFullScreen();
    } else {
        if (!m_focusWas.fullScreen) showNormal();
        m_ribbon->setVisible(m_focusWas.ribbon);
        m_ruler->setVisible(m_focusWas.ruler);
        statusBar()->setVisible(m_focusWas.status);
        for (const QPointer<QDockWidget> &d : std::as_const(m_focusWas.docks))
            if (d) d->show();
        m_focusWas.docks.clear();
    }
    act("view.focus")->setChecked(on);
    m_view->setFocus(Qt::OtherFocusReason);
}

// Read Mode: the window as Focus makes it, the pages in a row each fitted to the window, and the text read only; what was there before comes back when it ends.
void WriteWindow::syncVerticalRuler()
{
    if (m_vruler) m_vruler->setVisible(act("view.verticalRuler")->isChecked() && !m_outlineOn && !m_focusMode);
}

void WriteWindow::setReadMode(bool on)
{
    if (m_readMode == on) return;
    if (on && !m_doc) {
        act("view.readMode")->setChecked(false);
        return;
    }
    if (m_find) m_find->cancelSearch();   // (a search ends before the way the document may be changed does)
    if (on) {
        setOutlineMode(false);
        m_readWas.readOnly = isReadOnly();
        m_readWas.sideToSide = m_view->sideToSide();
        m_readWas.zoomMode = int(m_view->zoomMode());
        m_readWas.zoom = m_view->zoom();
        m_readMode = true;
        m_view->setStyleAreaSuppressed(true);
        setFocusMode(true);
        setReadOnly(true);   // (the window's own read-only state: every command, the Find pane, and the boxes refuse a change as they do in a document open for reading)
        m_view->setSideToSide(true);
        m_view->setZoomMode(write::PageView::ZoomMode::OnePage);
    } else {
        m_readMode = false;
        m_view->setStyleAreaSuppressed(false);
        setFocusMode(false);
        setReadOnly(m_readWas.readOnly || m_readOnlyByProtection);
        m_view->setSideToSide(m_readWas.sideToSide);
        m_view->setZoomMode(write::PageView::ZoomMode(m_readWas.zoomMode));
        if (m_view->zoomMode() == write::PageView::ZoomMode::Percent) m_view->setZoom(m_readWas.zoom);
    }
    act("view.readMode")->setChecked(on);
    act("view.pageMovement.sideToSide")->setChecked(m_view->sideToSide());
    act("view.pageMovement.vertical")->setChecked(!m_view->sideToSide());
    refreshUi();
}

QList<WriteWindow *> WriteWindow::writeWindows()
{
    QList<WriteWindow *> out;
    for (AppWindow *w : AppWindow::windows())
        if (auto *ww = qobject_cast<WriteWindow *>(w); ww && ww->isVisible()) out << ww;
    return out;
}

void WriteWindow::arrangeAllWindows()
{
    const QList<WriteWindow *> wins = writeWindows();
    if (wins.isEmpty()) return;
    const QRect area = wins.first()->screen()->availableGeometry();
    const int each = area.width() / int(wins.size());
    for (int i = 0; i < wins.size(); ++i) {
        wins[i]->showNormal();
        wins[i]->setGeometry(area.left() + i * each, area.top(), each, area.height());
    }
}

void WriteWindow::cascadeWindows()
{
    int k = 0;
    for (WriteWindow *w : writeWindows()) {
        w->showNormal();
        w->move(40 + k * 30, 40 + k * 30);
        w->raise();
        ++k;
    }
}

void WriteWindow::setSideBySide(bool on)
{
    // The windows that were tied are let go first; a window is tied to one other.
    auto untie = [](WriteWindow *w) {
        if (!w) return;
        QObject::disconnect(w->m_sideScroll[0]);
        QObject::disconnect(w->m_sideScroll[1]);
        w->m_sidePartner = nullptr;
        if (w->act(QStringLiteral("view.sideBySide"))->isChecked()) w->act(QStringLiteral("view.sideBySide"))->setChecked(false);
    };
    if (!on) {
        WriteWindow *other = m_sidePartner;
        untie(this);
        untie(other);
        refreshViewUi();
        if (other) other->refreshViewUi();
        return;
    }
    WriteWindow *partner = nullptr;
    for (WriteWindow *w : writeWindows())
        if (w != this) {
            partner = w;
            break;
        }
    if (!partner) {
        act(QStringLiteral("view.sideBySide"))->setChecked(false);
        statusBar()->showMessage(tr("Open a second window to view two documents side by side."), 5000);
        return;
    }
    untie(m_sidePartner);
    WriteWindow *old = partner->m_sidePartner;
    untie(old);
    untie(this);
    act(QStringLiteral("view.sideBySide"))->setChecked(true);
    partner->act(QStringLiteral("view.sideBySide"))->setChecked(true);
    m_sidePartner = partner;
    partner->m_sidePartner = this;
    // Each window's scrolling moves the other's, by the same share of the way down.
    auto follow = [](WriteWindow *from, WriteWindow *to, int which) {
        QScrollBar *a = from->m_view->verticalScrollBar(), *b = to->m_view->verticalScrollBar();
        auto guard = std::make_shared<bool>(false);
        from->m_sideScroll[which] = QObject::connect(a, &QScrollBar::valueChanged, to, [a, b, guard](int value) {
            if (*guard) return;
            *guard = true;
            b->setValue(a->maximum() > 0 ? int(qint64(value) * b->maximum() / a->maximum()) : value);
            *guard = false;
        });
    };
    follow(this, partner, 0);
    follow(partner, this, 1);
    const QRect area = screen()->availableGeometry();
    for (WriteWindow *w : {this, partner}) w->showNormal();
    setGeometry(area.left(), area.top(), area.width() / 2, area.height());
    partner->setGeometry(area.left() + area.width() / 2, area.top(), area.width() / 2, area.height());
    refreshViewUi();
    partner->refreshViewUi();
}

void WriteWindow::setSplit(bool on)
{
    if (on && !m_doc) {
        act("view.split")->setChecked(false);
        return;
    }
    if (on == m_split->isVisible()) return;
    if (on) {
        m_split->setDocument(m_doc.get());
        m_split->show();
        // The two panes share the height; the second begins where the first is.
        m_splitter->setSizes({m_splitter->height() / 2, m_splitter->height() / 2});
        m_split->goToPage(m_view->topPage());
    } else {
        m_split->hide();
        m_split->setDocument(nullptr);
    }
    act("view.split")->setChecked(on);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::refreshViewUi()
{
    if (m_nav && m_navDock && m_navDock->isVisible()) m_nav->refresh();
    // (Reset Window Position puts a pair of side-by-side windows back: with none it has nothing to do.)
    act(QStringLiteral("view.resetWindowPosition"))->setEnabled(m_sidePartner != nullptr);
    refreshSectionMoveUi();
}

} // namespace jp
