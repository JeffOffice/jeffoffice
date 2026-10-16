#include "app/findreplace.h"

#include "app/editor.h"
#include "core/document.h"

#include <QSet>
#include <algorithm>

namespace jp {

QVector<FindStory> findStories(const Document &doc, const FindScope &scope)
{
    // Every story once, in page order, using the first frame of each chain.
    QVector<FindStory> out;
    QSet<QString> seen;
    const Document *d = &doc;
    auto visit = [&](const ItemList &items, int page, const QString &master) {
        walkItems(items, [&](const ItemPtr &it) {
            if (it->type() == ItemType::Text) {
                auto *t = static_cast<TextItem *>(it.get());
                if (!seen.contains(t->storyId)) { seen.insert(t->storyId); out << FindStory{d->chainOf(t->id).value(0, t)->id, t->storyId, -1, -1, page, master}; }
            } else if (it->type() == ItemType::Shape) {
                auto *s = static_cast<ShapeItem *>(it.get());
                if (!s->storyId.isEmpty() && !seen.contains(s->storyId)) { seen.insert(s->storyId); out << FindStory{s->id, s->storyId, -1, -1, page, master}; }
            } else if (it->type() == ItemType::Table) {
                auto *tb = static_cast<TableItem *>(it.get());
                for (int r = 0; r < tb->rows; ++r)
                    for (int c = 0; c < tb->cols; ++c) {
                        out << FindStory{tb->id, tb->cell(r, c).storyId, r, c, page, master};
                        if (tb->cell(r, c).divided() && !tb->cell(r, c).storyId2.isEmpty()) out << FindStory{tb->id, tb->cell(r, c).storyId2, r, c, page, master, 1};
                    }
            }
        });
    };
    const int n = int(d->pages.size());
    for (int k = 0; k < n; ++k) {
        const int p = (std::clamp(scope.firstPage, 0, std::max(0, n - 1)) + k) % n;
        visit(d->pages[p]->items, p, QString());
        if (scope.notes && !d->pages[p]->notesStoryId.isEmpty() && !seen.contains(d->pages[p]->notesStoryId)) {
            seen.insert(d->pages[p]->notesStoryId);
            FindStory s;
            s.storyId = d->pages[p]->notesStoryId;
            s.page = p;
            s.notes = true;
            out << s;
        }
    }
    if (scope.masters)
        for (const auto &m : d->masters) visit(m->items, -1, m->id);
    return out;
}

bool findInStories(const Document &doc, const QVector<FindStory> &all, const QString &q, QTextDocument::FindFlags flags, int start, int pos, int *story, QTextCursor *found)
{
    const bool up = flags & QTextDocument::FindBackward;
    for (int k = 0; k <= all.size(); ++k) {
        const int i = up ? (start - k + all.size() * 2) % all.size() : (start + k) % all.size();
        const FindStory &h = all[i];
        QTextDocument *sd = doc.storyDoc(h.storyId);
        if (!sd) continue;
        int from = (k == 0) ? pos : (up ? sd->characterCount() : 0);
        if (from < 0) from = sd->characterCount();
        const QTextCursor c = sd->find(q, from, flags);
        if (c.isNull()) continue;
        *story = i;
        *found = c;
        return true;
    }
    return false;
}

int replaceInStories(const Document &doc, const QVector<FindStory> &stories, const QString &q, QTextDocument::FindFlags flags, const QString &with)
{
    int n = 0;
    for (const FindStory &h : stories) {
        QTextDocument *sd = doc.storyDoc(h.storyId);
        if (!sd) continue;
        QTextCursor c = sd->find(q, 0, flags & ~QTextDocument::FindBackward);
        while (!c.isNull()) {
            c.insertText(with);
            ++n;
            c = sd->find(q, c, flags & ~QTextDocument::FindBackward);
        }
    }
    return n;
}

void showFindMatch(Editor *ed, const FindStory &h, const QTextCursor &found)
{
    if (!h.master.isEmpty()) ed->setMasterView(h.master);
    else if (h.page >= 0) ed->setCurrentPage(h.page);
    if (h.notes) return;
    ed->beginTextEdit(h.itemId, found.selectionStart(), h.row, h.col, h.half);
    QTextCursor c = ed->cursor();
    c.setPosition(found.selectionStart());
    c.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
    ed->setCursor(c);
}

} // namespace jp
