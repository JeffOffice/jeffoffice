// References > Insert Footnote (Ctrl+Alt+F) and Insert Endnote (Ctrl+Alt+D): a reference to a new note where the caret is, and the caret in the note (at the foot of the page, or after the text).

#include "app/writewindow.h"

#include "app/pageview.h"
#include "core/editops.h"

#include <QAction>

namespace jp {

void WriteWindow::createNoteActions()
{
    mk("refs.footnote", tr("Insert Footnote"), "superscript", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F), [this] {
        // (A header, a footer, or a note takes no note.)
        if (m_doc && !m_view->inHeaderFooter() && !m_view->inNote()) {
            QTextCursor c = m_view->cursor();
            if (write::insertFootnote(*m_doc, c)) m_view->setCursor(c);
        }
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("refs.endnote", tr("Insert Endnote"), "list-end", QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_D), [this] {
        // (A header, a footer, or a note takes no note.)
        if (m_doc && !m_view->inHeaderFooter() && !m_view->inNote()) {
            QTextCursor c = m_view->cursor();
            if (write::insertEndnote(*m_doc, c)) m_view->setCursor(c);
        }
        m_view->setFocus(Qt::OtherFocusReason);
    });
}

} // namespace jp
