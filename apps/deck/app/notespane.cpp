#include "app/notespane.h"

#include "app/editor.h"
#include "core/deck.h"
#include "text/storyio.h"

#include "app/settings.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QPointer>
#include <QTextBlock>
#include <QTextDocument>

namespace jp {

NotesPane::NotesPane(Editor *ed, QWidget *parent) : QPlainTextEdit(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("notesPane"));
    setAccessibleName(tr("Notes"));
    setPlaceholderText(tr("Click to add notes"));
    m_idle.setSingleShot(true);
    m_idle.setInterval(700);
    connect(&m_idle, &QTimer::timeout, this, &NotesPane::commit);
    // Grammar as you type, as in the slides' text: the check is of this pane's own text, which is not in the slide until it is saved.
    m_grammar = std::make_unique<GrammarCheck>(document());
    m_grammar->setEnabled(grammarAsYouType());
    connect(m_grammar.get(), &GrammarCheck::changed, this, &NotesPane::markGrammar);
    connect(&GrammarSettingNotifier::get(), &GrammarSettingNotifier::changed, this, [this](bool on) { m_grammar->setEnabled(on); });
    connect(this, &QPlainTextEdit::textChanged, this, &NotesPane::markGrammar);
    connect(this, &QPlainTextEdit::textChanged, this, [this] {
        if (m_loading) return;
        m_typed = true;
        m_idle.start();
    });
    reload();
}

void NotesPane::reload()
{
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (!d || i < 0 || i >= d->pages.size()) return;
    const Page &slide = *d->pages[i];
    const QString text = deck::notesText(*d, slide);
    if (slide.id == m_pageId && (m_typed || text == toPlainText())) return;
    m_idle.stop();
    m_typed = false;
    m_loading = true;
    m_pageId = slide.id;
    setPlainText(text);
    m_loading = false;
}

void NotesPane::commit()
{
    m_idle.stop();
    Document *d = m_ed->doc();
    if (!d || !m_typed || m_ed->readOnly()) return;   // (a presentation open for reading only is not changed by what is typed)
    m_typed = false;
    const int i = d->pageIndexOf(m_pageId);
    if (i < 0) return;
    Page &slide = *d->pages[i];
    if (deck::notesText(*d, slide) == toPlainText()) return;
    m_ed->beginChange(tr("Notes"));
    if (slide.notesStoryId.isEmpty() || !d->storyDoc(slide.notesStoryId)) slide.notesStoryId = d->createStory();
    setStoryText(d->storyDoc(slide.notesStoryId), toPlainText());
    m_ed->endChange();
}

QVector<GrammarCheck::Issue> NotesPane::grammarIssues() const { return m_grammar->issuesIn(0, document()->characterCount()); }

void NotesPane::markGrammar()
{
    QList<QTextEdit::ExtraSelection> marks;
    for (const GrammarCheck::Issue &g : grammarIssues()) {
        QTextEdit::ExtraSelection x;
        x.cursor = QTextCursor(document());
        x.cursor.setPosition(g.start);
        x.cursor.setPosition(g.end, QTextCursor::KeepAnchor);
        x.format.setUnderlineStyle(QTextCharFormat::DotLine);
        x.format.setUnderlineColor(QColor(30, 90, 220));   // (the blue of the slides' text)
        marks << x;
    }
    setExtraSelections(marks);
}

// The message, the suggestions (the first in bold; each replaces the issue's words), Ignore This Issue, and Ignore All, for the issue at a position.
bool NotesPane::addGrammarSuggestions(QMenu &menu, int position)
{
    if (isReadOnly() || m_ed->readOnly() || !m_grammar->enabled()) return false;
    const GrammarCheck::Issue issue = m_grammar->issueAt(position);
    if (!issue.valid()) return false;
    menu.addAction(issue.message)->setEnabled(false);
    QPointer<NotesPane> where(this);
    bool first = true;
    for (const QString &sgg : issue.suggestions) {
        QAction *a = menu.addAction(sgg.isEmpty() ? tr("(Delete)") : sgg, &menu, [where, issue, sgg] {
            if (!where) return;
            QTextCursor c(where->document());
            c.setPosition(issue.start);
            c.setPosition(issue.end, QTextCursor::KeepAnchor);
            c.insertText(sgg);
        });
        if (first) {
            QFont f = a->font();
            f.setBold(true);
            a->setFont(f);
            first = false;
        }
    }
    menu.addAction(tr("Ignore This Issue"), &menu, [where, issue] {
        if (where) where->m_grammar->ignoreHere(issue);
    });
    menu.addAction(tr("Ignore All"), &menu, [where, issue] {
        if (where) where->m_grammar->ignoreAll(issue);
    });
    menu.addSeparator();
    return true;
}

void NotesPane::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);
    addGrammarSuggestions(menu, cursorForPosition(e->pos()).position());
    QMenu *standard = createStandardContextMenu(e->pos());
    standard->setParent(&menu);
    menu.addActions(standard->actions());
    menu.exec(e->globalPos());
}

void NotesPane::focusOutEvent(QFocusEvent *e)
{
    commit();
    QPlainTextEdit::focusOutEvent(e);
}

} // namespace jp
