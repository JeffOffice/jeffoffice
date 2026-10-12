#include "app/notespane.h"

#include "app/editor.h"
#include "core/deck.h"
#include "text/storyio.h"

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
    if (!d || !m_typed) return;
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

void NotesPane::focusOutEvent(QFocusEvent *e)
{
    commit();
    QPlainTextEdit::focusOutEvent(e);
}

} // namespace jp
