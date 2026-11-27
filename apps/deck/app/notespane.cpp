#include "app/notespane.h"

#include "app/editor.h"
#include "app/proofingdialogs.h"
#include "core/deck.h"

#include "app/settings.h"

#include <QMenu>
#include <QPointer>
#include <QTextDocument>

namespace jp {

NotesPane::NotesPane(Editor *ed, QWidget *parent) : StoryEdit(ed, parent), m_ed(ed)
{
    setObjectName(QStringLiteral("notesPane"));
    setAccessibleName(tr("Notes"));
    setPlaceholderText(tr("Click to add notes"));
    // Grammar as you type, as in the slides' text: the check is of the story shown, which it is told of as the story changes.
    m_grammar = std::make_unique<GrammarCheck>(document());
    m_grammar->setEnabled(grammarAsYouType());
    connect(m_grammar.get(), &GrammarCheck::changed, this, &NotesPane::markGrammar);
    connect(&GrammarSettingNotifier::get(), &GrammarSettingNotifier::changed, this, [this](bool on) { m_grammar->setEnabled(on); });
    connect(this, &StoryEdit::storyShown, this, [this] {
        m_grammar->setStory(document());
        markGrammar();
    });
    connect(this, &QTextEdit::textChanged, this, &NotesPane::markGrammar);
    reload();
}

// (the grammar check goes with this view's own members, before the base class's destructor lets the document go: nothing of this view's own may be called by then)
NotesPane::~NotesPane()
{
    disconnect(this, nullptr, this, nullptr);
    if (m_grammar) disconnect(m_grammar.get(), nullptr, this, nullptr);
}

void NotesPane::reload()
{
    const Document *d = m_ed->doc();
    const int i = m_ed->currentPage();
    if (!d || i < 0 || i >= d->pages.size()) return;
    const Page &slide = *d->pages[i];
    m_pageId = slide.id;
    setStory(slide.notesStoryId);
    if (hasFocus()) takeTarget();   // (the caret is in the notes of this slide: the commands act on them)
}

// A slide with no notes yet gets its story as the first words are put in (no step of its own: the typing is the step, and undone, the notes are as empty as they were).
QString NotesPane::makeStory()
{
    Document *d = m_ed->doc();
    const int i = d ? d->pageIndexOf(m_pageId) : -1;
    if (i < 0) return QString();
    Page &slide = *d->pages[i];
    if (slide.notesStoryId.isEmpty() || !d->storyDoc(slide.notesStoryId)) slide.notesStoryId = d->createStory();
    return slide.notesStoryId;
}

void NotesPane::commit() { flushTyping(); }

QVector<GrammarCheck::Issue> NotesPane::grammarIssues() const { return m_grammar->issuesIn(0, document()->characterCount()); }

void NotesPane::markGrammar()
{
    QVector<Mark> marks;
    for (const GrammarCheck::Issue &g : grammarIssues())
        marks.push_back(Mark{g.start, g.end, QColor(30, 90, 220), QTextCharFormat::DotLine});   // (the blue of the slides' text)
    setExtraMarks(marks);
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
            if (!where || !where->takeTarget()) return;
            where->m_ed->editText([&](QTextCursor &c) {
                c.setPosition(issue.start);
                c.setPosition(issue.end, QTextCursor::KeepAnchor);
                c.insertText(sgg);
            });
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

void NotesPane::fillMenu(QMenu &menu, int position)
{
    if (!isReadOnly() && !m_ed->readOnly() && isTarget()) addSpellingSuggestions(menu, m_ed);
    addGrammarSuggestions(menu, position);
    if (m_fill) m_fill(menu);
}

} // namespace jp
