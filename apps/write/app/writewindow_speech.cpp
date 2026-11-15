// JeffWrite's Read Aloud and missing fonts (libs/shell/app/readaloud, libs/shell/app/missingfonts): what the shared parts need of a document of one long story and its notes.
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/editops.h"
#include "core/writedoc.h"

#include <QPointer>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>
#include <memory>

namespace jp {

namespace {

struct ReadStories {
    QVector<QPointer<QTextDocument>> stories;   // (the number of a paragraph's story: its place here; a story that is gone is null, and nothing is shown for it)
    bool lit = false;
};

}

jp::ReadAloudHost WriteWindow::readAloudHost()
{
    auto state = std::make_shared<ReadStories>();
    jp::ReadAloudHost host;
    host.text = [this, state] {
        QList<jp::ReadAloudParagraph> out;
        state->stories.clear();
        if (!m_doc) return out;
        const QTextCursor c = m_view->cursor();
        QTextDocument *body = m_doc->story();
        auto add = [&](QTextDocument *d, int from, int to) {
            state->stories << QPointer<QTextDocument>(d);
            out += jp::readAloudParagraphs(d, int(state->stories.size()) - 1, from, to);
        };
        if (c.document() && c.document() != body) {
            // In a note, a header, or a footer: that text alone, from the caret or the selection.
            add(c.document(), c.hasSelection() ? c.selectionStart() : c.position(), c.hasSelection() ? c.selectionEnd() : -1);
            return out;
        }
        if (c.hasSelection()) {
            add(body, c.selectionStart(), c.selectionEnd());
            return out;
        }
        add(body, c.position(), -1);
        // The footnotes as they come: in the order of their references in the text.
        QVector<QPair<int, int>> notes;   // (the place of the first reference, the note)
        for (int id : m_doc->citedFootnotes()) {
            const QVector<int> refs = m_doc->footnoteReferences(id);
            if (!refs.isEmpty() && refs.first() >= c.position()) notes.push_back({refs.first(), id});
        }
        std::sort(notes.begin(), notes.end());
        for (const auto &n : std::as_const(notes))
            if (QTextDocument *d = m_doc->editableFootnote(n.second)) add(d, 0, -1);
        return out;
    };
    // The word is shown as a search match is (a tint), not selected: what is typed is never put over it. A footnote's words are said without a tint.
    host.highlight = [this, state](const jp::ReadAloudParagraph &p, int start, int length) {
        if (p.story < 0 || p.story >= state->stories.size() || !m_doc) return;
        QTextDocument *d = state->stories.at(p.story).data();
        if (!d || d != m_doc->story()) return;
        QTextCursor c(d);
        c.setPosition(std::min(p.offset + start, d->characterCount() - 1));
        m_view->showCursor(c);
        m_view->setFindHighlights({{p.offset + start, p.offset + start + length}});
        state->lit = true;
    };
    host.clear = [this, state] {
        if (!state->lit) return;
        state->lit = false;
        m_view->setFindHighlights({});
    };
    // Any change of the text or the notes, or another document put in the window, ends the reading.
    host.watch = [this](const std::function<void()> &changed) {
        QList<QMetaObject::Connection> c;
        if (!m_doc) return c;
        c << connect(m_doc->story(), &QTextDocument::contentsChange, this, [changed] { changed(); });
        c << connect(m_doc.get(), &write::WriteDoc::historyChanged, this, [changed] { changed(); });
        c << connect(m_doc.get(), &write::WriteDoc::noteChanged, this, [changed] { changed(); });
        c << connect(m_doc.get(), &write::WriteDoc::headerFooterChanged, this, [changed] { changed(); });
        return c;
    };
    return host;
}

jp::MissingFontsHost WriteWindow::missingFontsHost()
{
    jp::MissingFontsHost host;
    host.usage = [this] {
        jp::FontUsage u;
        if (!m_doc) return u;
        // The text (the endnotes are paragraphs of it), then the headers, footers, and footnotes.
        u.addDocument(m_doc->story(), tr("Text"), [this](const QTextBlock &b) { return m_doc->endnoteIdOf(b) ? tr("Endnotes") : tr("Text"); });
        for (QTextDocument *d : m_doc->storiesWithChanges(true)) {
            int section = 0, slot = 0;
            bool footer = false;
            u.addDocument(d, m_doc->footnoteIdOf(d) ? tr("Footnotes") : m_doc->headerFooterSlotOf(d, &section, &footer, &slot) && footer ? tr("Footers") : tr("Headers"));
        }
        return u;
    };
    host.replace = [this](const QString &from, const QString &to) { return replaceMissingFont(from, to); };
    host.refresh = [this] {
        if (m_doc) m_view->refresh();
    };
    return host;
}

// Replace in Document: the one place JeffWrite changes the fonts of the file for the dialog (a shared Replace Fonts can take this place later).
int WriteWindow::replaceMissingFont(const QString &from, const QString &to)
{
    if (!m_doc || refuseIfReadOnly()) return -1;
    const int n = write::replaceFont(*m_doc, from, to);
    refreshNow();
    return n;
}

} // namespace jp
