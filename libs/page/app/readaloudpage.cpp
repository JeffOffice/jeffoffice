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
};

QTextDocument *wordsOf(const Document &doc, const FindStory &s) { return s.art ? s.art.get() : doc.storyDoc(s.storyId); }

bool sameStory(const FindStory &s, const Editor::TextTarget &t)
{
    return s.storyId == t.storyId && s.row == t.row && s.col == t.col && s.half == t.half;
}

}

ReadAloudHost pageReadAloudHost(Editor *ed, const FindScope &scope, const std::function<void(const FindStory &, const QTextCursor &)> &show)
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
    host.highlight = [ed, state, show](const ReadAloudParagraph &p, int start, int length) {
        if (p.story < 0 || p.story >= state->stories.size()) return;
        const FindStory &s = state->stories.at(p.story);
        QTextDocument *words = wordsOf(*ed->doc(), s);
        if (!words) return;
        QTextCursor c(words);
        c.setPosition(p.offset + start);
        c.setPosition(p.offset + start + length, QTextCursor::KeepAnchor);
        if (show) show(s, c);
        else showFindMatch(ed, s, c);
        state->lit = true;
    };
    host.clear = [ed, state] {
        if (!state->lit) return;
        state->lit = false;
        if (!ed->isEditingText()) return;
        QTextCursor c = ed->cursor();
        c.clearSelection();
        ed->setCursor(c);
    };
    return host;
}

} // namespace jp
