// The status bar's place and proofing pieces: the caret's line and column on its page, and how many spelling and grammar errors the document has, which a click goes to the next of.

#include "app/writewindow.h"

#include "app/grammarcheck.h"
#include "app/pageview.h"
#include "core/spellcheck.h"
#include "core/writedoc.h"

#include <QLabel>
#include <QTextCursor>
#include <QToolButton>

namespace jp {

namespace {
constexpr int kGrammarSliceMs = 30;   // the time the grammar check may take, in all, to find the next issue on a click
}

// The caret's place on its page; nothing where the caret is not in the page's own text.
void WriteWindow::updateCaretPlace()
{
    if (!m_placeLabel) return;
    int line = 0, column = 0;
    if (m_doc && m_view->caretLineColumn(&line, &column)) m_placeLabel->setText(tr("Line %1, Column %2").arg(line).arg(column));
    else m_placeLabel->clear();
}

// The count of errors from what the checks already know (nothing is checked or counted here): the spelling total the check keeps, and the grammar issues of the paragraphs it has answered.
void WriteWindow::updateProofing()
{
    if (!m_proofLabel) return;
    const write::SpellCheck *spelling = m_doc ? m_view->spelling() : nullptr;
    const bool spellingOn = spelling && spelling->enabled() && m_view->spellingEnabled();
    const GrammarCheck *grammar = m_doc ? m_view->grammar() : nullptr;
    const bool grammarOn = grammar && grammar->enabled() && m_view->grammarEnabled();
    if (!spellingOn && !grammarOn) {
        m_proofLabel->setText(QString());
        m_proofLabel->hide();
        return;
    }
    const int n = (spellingOn ? spelling->knownMisspellings() : 0) + (grammarOn ? grammar->knownIssues() : 0);
    m_proofLabel->setText(spellingOn && !spelling->counted() ? tr("%1+ proofing errors").arg(n) : tr("%n proofing error(s)", nullptr, n));
    m_proofLabel->show();
}

// Selects the next error after the caret (the nearer of the next misspelled word and the next grammar issue), going round to the top at the end of the text.
void WriteWindow::goToNextProofingError()
{
    if (!m_doc || m_view->inHeaderFooter() || m_view->inNote()) return;
    write::SpellCheck *spelling = m_view->spelling();
    GrammarCheck *grammar = m_view->grammar();
    const bool spellingOn = spelling && spelling->enabled() && m_view->spellingEnabled();
    const bool grammarOn = grammar && grammar->enabled() && m_view->grammarEnabled();
    const int caret = m_view->cursor().selectionEnd();
    QPair<int, int> found(-1, -1);
    for (const int from : {caret, 0}) {
        QPair<int, int> word(-1, -1);
        if (spellingOn) word = spelling->nextMisspelling(from);
        GrammarCheck::Issue issue;
        int next = from;
        if (grammarOn && grammar->search(from, kGrammarSliceMs, &issue, &next) == GrammarCheck::Step::Found && (word.first < 0 || issue.start < word.first)) word = qMakePair(issue.start, issue.end);
        if (word.first >= 0) {
            found = word;
            break;
        }
    }
    if (found.first < 0) {
        statusBar()->showMessage(tr("There are no spelling or grammar errors in this document."), 5000);
        return;
    }
    QTextCursor c(m_doc->story());
    c.setPosition(found.first);
    c.setPosition(found.second, QTextCursor::KeepAnchor);
    m_view->setCursor(c);
    m_view->setFocus(Qt::OtherFocusReason);
}

// The checks of a document just set: the spelling check counts as it goes, and what it, the grammar check, and the spelling being on or off say is shown after a pause.
void WriteWindow::watchProofing()
{
    if (!m_doc) {
        updateProofing();
        return;
    }
    if (write::SpellCheck *spelling = m_view->spelling()) {
        spelling->setCounting(true);
        connect(spelling, &write::SpellCheck::countChanged, &m_proofTimer, qOverload<>(&QTimer::start));
    }
    if (GrammarCheck *grammar = m_view->grammar()) connect(grammar, &GrammarCheck::changed, &m_proofTimer, qOverload<>(&QTimer::start));
    m_proofTimer.start();
}

} // namespace jp
