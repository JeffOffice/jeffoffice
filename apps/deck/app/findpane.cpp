#include "app/findpane.h"

#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/outlineview.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

FindPane::FindPane(DeckWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
{
    setObjectName(QStringLiteral("findPane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(6, 6, 6, 6);
    // Each label names its box for screen readers (its buddy).
    auto *findLabel = new QLabel(tr("Find what:"), this);
    v->addWidget(findLabel);
    m_find = new QLineEdit(this);
    m_find->setObjectName(QStringLiteral("findWhat"));
    findLabel->setBuddy(m_find);
    v->addWidget(m_find);
    auto *replaceLabel = new QLabel(tr("Replace with:"), this);
    v->addWidget(replaceLabel);
    m_replace = new QLineEdit(this);
    m_replace->setObjectName(QStringLiteral("replaceWith"));
    replaceLabel->setBuddy(m_replace);
    v->addWidget(m_replace);
    m_case = new QCheckBox(tr("Match case"), this);
    m_whole = new QCheckBox(tr("Find whole words only"), this);
    m_notes = new QCheckBox(tr("Search speaker notes"), this);
    m_notes->setChecked(true);
    v->addWidget(m_case);
    v->addWidget(m_whole);
    v->addWidget(m_notes);
    auto *row = new QHBoxLayout();
    auto *next = new QPushButton(tr("Find Next"), this);
    auto *rep = new QPushButton(tr("Replace"), this);
    auto *all = new QPushButton(tr("Replace All"), this);
    row->addWidget(next);
    row->addWidget(rep);
    row->addWidget(all);
    v->addLayout(row);
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("findStatus"));
    m_status->setWordWrap(true);
    v->addWidget(m_status);
    v->addStretch(1);
    connect(next, &QPushButton::clicked, this, [this] { findNext(); });
    connect(m_find, &QLineEdit::returnPressed, this, [this] { findNext(); });
    connect(rep, &QPushButton::clicked, this, [this] { replaceOne(); });
    connect(all, &QPushButton::clicked, this, [this] { replaceAll(); });
    setMinimumWidth(240);
}

void FindPane::focusBox(bool replace)
{
    QLineEdit *box = replace ? m_replace : m_find;
    box->setFocus(Qt::OtherFocusReason);
    box->selectAll();
}

QString FindPane::status() const { return m_status->text(); }

QTextDocument::FindFlags FindPane::flags() const
{
    QTextDocument::FindFlags f;
    if (m_case->isChecked()) f |= QTextDocument::FindCaseSensitively;
    if (m_whole->isChecked()) f |= QTextDocument::FindWholeWords;
    return f;
}

FindScope FindPane::scope(int firstPage) const
{
    FindScope s;
    s.masters = false;   // the layouts' prompts are not the presentation's words
    s.notes = m_notes->isChecked();
    s.firstPage = firstPage;
    return s;
}

// Whether the last match was in the notes of the slide being edited and their cursor has stayed where it ended.
bool FindPane::notesMatchShown() const
{
    Editor *ed = m_win->editor();
    if (m_noteStory.isEmpty() || ed->isEditingText()) return false;
    const Document *d = ed->doc();
    const int page = ed->currentPage();
    return page >= 0 && page < d->pages.size() && d->pages[page]->notesStoryId == m_noteStory && m_win->notesPane()->textCursor().selectionEnd() == m_noteEnd;
}

void FindPane::showMatch(const FindStory &story, const QTextCursor &found)
{
    if (m_win->view() == DeckWindow::View::Sorter) m_win->setView(DeckWindow::View::Normal);
    Editor *ed = m_win->editor();
    showFindMatch(ed, story, found);
    m_noteStory.clear();
    if (!story.notes) return;
    // The notes are a box under the slide, not part of it: open them and select the match there.
    ed->endTextEdit();
    ed->clearSelection();
    NotesPane *notes = m_win->notesPane();
    if (notes->isHidden()) m_win->act(QStringLiteral("view.notes"))->trigger();
    notes->reload();
    QTextCursor c = notes->textCursor();
    c.setPosition(found.selectionStart());
    c.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
    notes->setTextCursor(c);
    notes->ensureCursorVisible();
    m_noteStory = story.storyId;
    m_noteEnd = found.selectionEnd();
}

bool FindPane::findNext()
{
    Editor *ed = m_win->editor();
    const QString q = m_find->text();
    if (q.isEmpty()) return false;
    // What was typed in the notes or the outline is on the slides first.
    m_win->notesPane()->commit();
    if (m_win->outlineView()) m_win->outlineView()->commit();
    const Document *d = ed->doc();
    if (d->pages.isEmpty()) return false;
    const QVector<FindStory> all = findStories(*d, scope(std::clamp(ed->currentPage(), 0, int(d->pages.size()) - 1)));
    int start = 0;
    int pos = 0;
    if (ed->isEditingText()) {
        for (int i = 0; i < all.size(); ++i)
            if (all[i].storyId == ed->textTarget().storyId) { start = i; break; }
        pos = ed->cursor().selectionEnd();
    } else if (notesMatchShown()) {
        for (int i = 0; i < all.size(); ++i)
            if (all[i].storyId == m_noteStory) { start = i; break; }
        pos = m_noteEnd;
    }
    int story = 0;
    QTextCursor found;
    if (!all.isEmpty() && findInStories(*d, all, q, flags(), start, pos, &story, &found)) {
        showMatch(all[story], found);
        m_status->clear();
        return true;
    }
    m_status->setText(tr("JeffDeck finished searching the presentation. \"%1\" was not found.").arg(q));
    return false;
}

void FindPane::replaceOne()
{
    Editor *ed = m_win->editor();
    const Qt::CaseSensitivity cs = m_case->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    if (ed->isEditingText() && ed->cursor().hasSelection() && ed->cursor().selectedText().compare(m_find->text(), cs) == 0) {
        ed->change(tr("Replace"), [&] { ed->cursor().insertText(m_replace->text()); });
        ed->textEdited();
    } else if (notesMatchShown()) {
        NotesPane *notes = m_win->notesPane();
        const QTextCursor sel = notes->textCursor();
        if (sel.hasSelection() && sel.selectedText().compare(m_find->text(), cs) == 0) {
            const int start = sel.selectionStart();
            ed->change(tr("Replace"), [&] {
                QTextCursor c(ed->doc()->storyDoc(m_noteStory));
                c.setPosition(start);
                c.setPosition(sel.selectionEnd(), QTextCursor::KeepAnchor);
                c.insertText(m_replace->text());
            });
            // The notes show the new words with the cursor after them, where the next search goes on from.
            QTextCursor after = notes->textCursor();
            after.setPosition(start + int(m_replace->text().size()));
            notes->setTextCursor(after);
            m_noteEnd = after.position();
        }
    }
    findNext();
}

void FindPane::replaceAll()
{
    Editor *ed = m_win->editor();
    const QString q = m_find->text();
    if (q.isEmpty()) return;
    m_win->notesPane()->commit();
    if (m_win->outlineView()) m_win->outlineView()->commit();
    ed->endTextEdit();
    m_noteStory.clear();
    int n = 0;
    ed->change(tr("Replace All"), [&] { n = replaceInStories(*ed->doc(), findStories(*ed->doc(), scope(0)), q, flags(), m_replace->text()); });
    m_status->setText(n == 0   ? tr("JeffDeck made no replacements. \"%1\" was not found.").arg(q)
                      : n == 1 ? tr("JeffDeck made 1 replacement.")
                               : tr("JeffDeck made %1 replacements.").arg(n));
}

} // namespace jp
