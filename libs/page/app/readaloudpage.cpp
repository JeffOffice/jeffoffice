#include "app/readaloudpage.h"
#include "app/editor.h"

#include <QSet>
#include <QTextCursor>
#include <QTextDocument>

#include <memory>

namespace jp {

namespace {

struct ReadState {
    QVector<FindStory> stories;
    bool lit = false;
    bool moving = false;   // (the reading itself is turning the page: that is not a change of the document)
};

QTextDocument *wordsOf(const Document &doc, const FindStory &s) { return s.art ? s.art.get() : doc.storyDoc(s.storyId); }

bool sameStory(const FindStory &s, const Editor::TextTarget &t)
{
    return s.storyId == t.storyId && s.row == t.row && s.col == t.col && s.half == t.half;
}

}

ReadAloudHost pageReadAloudHost(Editor *ed, const FindScope &scope, const std::function<void(const FindStory &)> &reveal)
{
    auto state = std::make_shared<ReadState>();
    ReadAloudHost host;
    host.text = [ed, scope, state] {
        QList<ReadAloudParagraph> out;
        const Document &doc = *ed->doc();
        state->stories = findStories(doc, scope);
        const QVector<FindStory> &all = state->stories;
        int first = 0, from = 0;
        QSet<QString> only;   // the objects selected: nothing else is read
        if (ed->isEditingText()) {
            int at = -1;
            for (int i = 0; i < all.size() && at < 0; ++i)
                if (sameStory(all.at(i), ed->textTarget())) at = i;
            if (at >= 0) {
                const QTextCursor c = ed->cursor();
                // The words selected are all that is read.
                if (c.hasSelection()) return readAloudParagraphs(wordsOf(doc, all.at(at)), at, c.selectionStart(), c.selectionEnd());
                first = at;
                from = c.position();
            }
        } else if (!ed->selection().isEmpty()) {
            for (const QString &id : ed->selection()) only.insert(id);
        } else {
            first = all.size();
            for (int i = 0; i < all.size(); ++i)
                if (all.at(i).master.isEmpty() && all.at(i).page >= ed->currentPage()) {
                    first = i;
                    break;
                }
        }
        for (int i = first; i < all.size(); ++i) {
            if (!only.isEmpty() && !only.contains(all.at(i).itemId)) continue;
            out += readAloudParagraphs(wordsOf(doc, all.at(i)), i, i == first ? from : 0);
        }
        return out;
    };
    host.highlight = [ed, state, reveal](const ReadAloudParagraph &p, int start, int length) {
        if (p.story < 0 || p.story >= state->stories.size()) return;
        const FindStory &s = state->stories.at(p.story);
        if (s.art || !wordsOf(*ed->doc(), s)) return;   // (a Text Art's words are not drawn as text: nothing to tint)
        state->moving = true;
        if (reveal) reveal(s);
        if (!s.master.isEmpty()) ed->setMasterView(s.master);
        else if (s.page >= 0) ed->setCurrentPage(s.page);
        state->moving = false;
        ed->setReadHighlight(s.storyId, p.offset + start, p.offset + start + length);
        state->lit = true;
    };
    host.clear = [ed, state] {
        if (!state->lit) return;
        state->lit = false;
        ed->clearReadHighlight();
    };
    host.watch = [ed, state](const std::function<void()> &changed) {
        QList<QMetaObject::Connection> c;
        c << QObject::connect(ed, &Editor::changed, ed, [changed, state] {
            if (!state->moving) changed();
        });
        c << QObject::connect(ed, &Editor::documentReplaced, ed, [changed] { changed(); });
        return c;
    };
    return host;
}

} // namespace jp
