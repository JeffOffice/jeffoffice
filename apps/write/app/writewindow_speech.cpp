// JeffWrite's Read Aloud and missing fonts (libs/shell/app/readaloud, libs/shell/app/missingfonts): what the shared parts need of a document of one long story and its notes.
#include "app/writewindow.h"
#include "app/pageview.h"
#include "core/editops.h"
#include "core/writedoc.h"

#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>
#include <memory>

namespace jp {

namespace {

struct ReadStories {
    QVector<QTextDocument *> stories;   // (the number of a paragraph's story: its place here)
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
            state->stories << d;
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
    host.highlight = [this, state](const jp::ReadAloudParagraph &p, int start, int length) {
        if (p.story < 0 || p.story >= state->stories.size() || !m_doc) return;
        QTextCursor c(state->stories.at(p.story));
        c.setPosition(p.offset + start);
        c.setPosition(p.offset + start + length, QTextCursor::KeepAnchor);
        m_view->showCursor(c);
        state->lit = true;
    };
    host.clear = [this, state] {
        if (!state->lit) return;
        state->lit = false;
        QTextCursor c = m_view->cursor();
        c.clearSelection();
        m_view->setCursor(c);
    };
    return host;
}

jp::MissingFontsHost WriteWindow::missingFontsHost()
{
    jp::MissingFontsHost host;
    host.usage = [this] {
        jp::FontUsage u;
        if (!m_doc) return u;
        u.addDocument(m_doc->story(), tr("Text"));
        for (int s = 0; s < m_doc->sectionCount(); ++s)
            for (const bool footer : {false, true})
                for (int slot = 0; slot < 3; ++slot)
                    if (m_doc->hasOwnHeaderFooter(s, footer, slot))
                        if (QTextDocument *d = m_doc->headerFooterStory(s, footer, slot)) u.addDocument(d, footer ? tr("Footers") : tr("Headers"));
        return u;
    };
    host.replace = [this](const QString &from, const QString &to) {
        if (!m_doc || refuseIfReadOnly()) return -1;
        const int n = write::replaceFont(*m_doc, from, to);
        refreshNow();
        return n;
    };
    host.refresh = [this] {
        if (m_doc) m_view->refresh();
    };
    return host;
}

} // namespace jp
