// JeffWrite's styles and the tools beside them: the Styles pane, Create Style and Modify Style, Update Style to Match Selection, the Copy Formatting, and Go To (the work on the
// document is core/styleops.h and core/gotoops.h).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/stylespane.h"
#include "app/writedialogs.h"
#include "core/editops.h"
#include "core/styleops.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDockWidget>
#include <QListWidget>
#include <QStatusBar>

namespace jp {

void WriteWindow::createStyleActions()
{
    mk("style.pane", tr("Styles Pane"), "palette", QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_S), [this] { showStylesPane(); });
    mk("style.new", tr("Create a Style…"), "plus", QKeySequence(), [this] { showStyleDialog(true); });
    mk("style.modify", tr("Modify Style…"), "pen-line", QKeySequence(), [this] { showStyleDialog(false); });
    mk("style.update", tr("Update Style to Match Selection"), "refresh-ccw", QKeySequence(), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        QString why;
        if (!write::updateStyleToSelection(*m_doc, m_view->cursor(), &why) && !why.isEmpty()) statusBar()->showMessage(why, 8000);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    QAction *painter = mk("edit.formatPainter", tr("Copy Formatting"), "paintbrush", QKeySequence(), [this] {
        // A second click within the time of a double click keeps the painter on (a double click on the button does the same through its own event).
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (m_painter == PainterMode::Once && now - m_painterPicked < QApplication::doubleClickInterval()) {
            setPainterMode(PainterMode::Locked);
        } else if (m_painter != PainterMode::Off) {
            setPainterMode(PainterMode::Off);
        } else {
            setPainterMode(PainterMode::Once);
            m_painterPicked = now;
        }
        act(QStringLiteral("edit.formatPainter"))->setChecked(m_painter != PainterMode::Off);
    }, true);
    painter->setToolTip(tr("Copy Formatting: click to paint once, double-click to keep painting, and press Esc to stop."));
    mk("edit.copyFormat", tr("Copy Format"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), [this] {
        if (!m_doc) return;
        m_brush = write::copyFormat(*m_doc, m_view->cursor());
        if (m_brush.valid) statusBar()->showMessage(tr("Format copied. Select text and press Ctrl+Shift+V to paste it."), 8000);
    });
    mk("edit.pasteFormat", tr("Paste Format"), "", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V), [this] {
        if (!m_doc) return;
        if (!m_brush.valid) {
            statusBar()->showMessage(tr("There is no format to paste yet. Select text and press Ctrl+Shift+C to copy its format."), 8000);
            return;
        }
        write::pasteFormat(*m_doc, m_view->cursor(), m_brush);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("edit.goTo", tr("Go To…"), "locate-fixed", QKeySequence(Qt::CTRL | Qt::Key_G), [this] {
        if (!m_doc) return;
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        write::goToDialog(this, m_doc.get(), m_view);
        m_view->setFocus(Qt::OtherFocusReason);
    });
}

void WriteWindow::showStylesPane()
{
    if (!m_styles) {
        m_stylesDock = new QDockWidget(tr("Styles"), this);
        m_stylesDock->setObjectName(QStringLiteral("stylesDock"));
        m_stylesDock->setFeatures(QDockWidget::DockWidgetClosable);
        m_styles = new StylesPane(this, m_stylesDock);
        m_stylesDock->setWidget(m_styles);
        addDockWidget(Qt::RightDockWidgetArea, m_stylesDock);
        resizeDocks({m_stylesDock}, {260}, Qt::Horizontal);
    }
    m_stylesDock->show();
    m_stylesDock->raise();
    m_styles->refresh();
    m_styles->list()->setFocus();
}

void WriteWindow::showStyleDialog(bool create)
{
    if (!m_doc) return;
    // (The styles are the text's: a header, footer, or note being edited is left first.)
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    const QString id = create ? QString() : (m_styles && m_styles->isVisible() ? m_styles->chosenId() : write::paragraphStyleAt(*m_doc, m_view->cursor()));
    write::styleDialog(this, m_doc.get(), &m_view->cursor(), id);
    m_view->setFocus(Qt::OtherFocusReason);
    refreshUi();
}

void WriteWindow::setPainterMode(PainterMode mode)
{
    if (mode == m_painter) return;
    if (mode != PainterMode::Off && m_painter == PainterMode::Off) {
        // The look to paint is that of the selection (or of the paragraph the caret is in) when the painter is picked up.
        if (!m_doc) return;
        m_brush = write::copyFormat(*m_doc, m_view->cursor());
        if (!m_brush.valid) return;
    }
    m_painter = mode;
    act(QStringLiteral("edit.formatPainter"))->setChecked(mode != PainterMode::Off);
    m_view->setTextDragAllowed(mode == PainterMode::Off);
    if (mode == PainterMode::Off) statusBar()->clearMessage();
    else statusBar()->showMessage(tr("Copy Formatting: select the text to paint. Press Esc to stop."));
}

void WriteWindow::paintFormatNow()
{
    if (m_painter == PainterMode::Off || !m_doc) return;
    write::pasteFormat(*m_doc, m_view->cursor(), m_brush);
    if (m_painter == PainterMode::Once) setPainterMode(PainterMode::Off);
    m_view->setFocus(Qt::OtherFocusReason);
}

} // namespace jp
