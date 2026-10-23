// References > Insert Footnote (Ctrl+Alt+F): a reference to a new note where the caret is, and the caret in the note at the foot of the page.

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
}

} // namespace jp
