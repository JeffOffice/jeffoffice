// Insert > Content Control: the menu that puts a plain text box, a drop-down list, a date picker, or a check box in the text, and the Properties box of the one the caret is in; and what a
// click on a list or a date shows (core/contentcontrolops.h).

#include "app/controldialogs.h"
#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/contentcontrolops.h"

#include <QAction>
#include <QMenu>
#include <QStatusBar>

namespace jp {

void WriteWindow::createControlActions()
{
    auto insert = [this](write::ControlKind kind) {
        if (!m_doc || refuseIfReadOnly()) return;
        QString why;
        if (!write::insertContentControl(*m_doc, m_view->cursor(), kind, &why) && !why.isEmpty()) statusBar()->showMessage(why, 6000);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    };
    mk("insert.control.text", tr("Plain Text"), "text-cursor-input", QKeySequence(), [insert] { insert(write::ControlKind::PlainText); });
    mk("insert.control.dropDown", tr("Drop-Down List"), "list", QKeySequence(), [insert] { insert(write::ControlKind::DropDown); });
    mk("insert.control.date", tr("Date Picker"), "calendar", QKeySequence(), [insert] { insert(write::ControlKind::Date); });
    mk("insert.control.checkBox", tr("Check Box"), "square-check", QKeySequence(), [insert] { insert(write::ControlKind::CheckBox); });
    mk("insert.control.properties", tr("Properties…"), "settings-2", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        const write::ContentControl c = write::contentControlAt(*m_doc, m_view->cursor().hasSelection() ? m_view->cursor().selectionStart() + 1 : m_view->cursor().position());
        if (!c.valid()) return;
        write::contentControlDialog(this, m_doc.get(), c.start);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    connect(m_view, &write::PageView::controlChoiceWanted, this, [this](int start, const QPoint &global) {
        if (!m_doc || refuseIfReadOnly()) return;
        QMenu *menu = write::controlChoicesMenu(this, m_doc.get(), start, [this] {
            m_view->setFocus(Qt::OtherFocusReason);
            m_view->viewport()->update();
        });
        if (menu) menu->popup(global);
    });
}

void WriteWindow::refreshControlUi()
{
    const bool body = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    for (const char *id : {"insert.control.text", "insert.control.dropDown", "insert.control.date", "insert.control.checkBox"}) act(QString::fromLatin1(id))->setEnabled(body);
    bool inControl = false;
    if (body) {
        const QTextCursor &c = m_view->cursor();
        const write::ContentControl ctl = write::contentControlAt(*m_doc, c.hasSelection() ? c.selectionStart() + 1 : c.position());
        inControl = ctl.valid() && ctl.editable;
    }
    act("insert.control.properties")->setEnabled(inControl);
}

} // namespace jp
