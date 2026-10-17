#include "app/findreplace.h"

#include "app/editor.h"
#include "core/document.h"

#include <QHash>
#include <QSet>
#include <algorithm>

namespace jp {

// The words of a story to search: the story's, or the copy of a Text Art's.
static QTextDocument *wordsOf(const Document &doc, const FindStory &h) { return h.art ? h.art.get() : doc.storyDoc(h.storyId); }

QVector<FindStory> findStories(const Document &doc, const FindScope &scope)
{
    // Every story once, in page order, using the first frame of each chain.
    QVector<FindStory> out;
    QSet<QString> seen;
    const Document *d = &doc;
    // The frame before each frame of a chain, found once: asking the document for the frame before every text box would take the
    // whole document's time again for each of them.
    QHash<QString, const TextItem *> before;
    d->forEachItem([&](Item *it, int, const QString &) {
        if (it->type() == ItemType::Text && !static_cast<TextItem *>(it)->nextId.isEmpty()) before.insert(static_cast<TextItem *>(it)->nextId, static_cast<TextItem *>(it));
    });
    auto firstFrame = [&](const TextItem *t) {
        const TextItem *head = t;
        QSet<QString> walked{head->id};
        while (const TextItem *p = before.value(head->id)) {
            if (walked.contains(p->id)) break;
            walked.insert(p->id);
            head = p;
        }
        return head;
    };
    auto visit = [&](const ItemList &items, int page, const QString &master) {
        walkItems(items, [&](const ItemPtr &it) {
            if (it->type() == ItemType::Text) {
                auto *t = static_cast<TextItem *>(it.get());
                if (!seen.contains(t->storyId)) { seen.insert(t->storyId); out << FindStory{firstFrame(t)->id, t->storyId, -1, -1, page, master}; }
            } else if (it->type() == ItemType::Shape) {
                auto *s = static_cast<ShapeItem *>(it.get());
                if (!s->storyId.isEmpty() && !seen.contains(s->storyId)) { seen.insert(s->storyId); out << FindStory{s->id, s->storyId, -1, -1, page, master}; }
            } else if (it->type() == ItemType::TextArt) {
                auto *a = static_cast<TextArtItem *>(it.get());
                if (scope.textArt && !a->text.isEmpty()) {
                    FindStory s;
                    s.itemId = a->id;
                    s.page = page;
                    s.master = master;
                    s.art = std::make_shared<QTextDocument>();
                    s.art->setPlainText(a->text);
                    out << s;
                }
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

// The words of every story of a Document of placed objects.
static FindWords wordsOfDocument(const Document &doc)
{
    FindWords w;
    w.story = [&doc](const FindStory &h) { return wordsOf(doc, h); };
    return w;
}

bool findInStories(const Document &doc, const QVector<FindStory> &all, const QString &q, QTextDocument::FindFlags flags, int start, int pos, int *story, QTextCursor *found)
{
    return findInStories(wordsOfDocument(doc), all, q, flags, start, pos, story, found);
}

bool findInStories(const FindWords &words, const QVector<FindStory> &all, const QString &q, QTextDocument::FindFlags flags, int start, int pos, int *story, QTextCursor *found)
{
    const bool up = flags & QTextDocument::FindBackward;
    for (int k = 0; k <= all.size(); ++k) {
        const int i = up ? (start - k + all.size() * 2) % all.size() : (start + k) % all.size();
        const FindStory &h = all[i];
        QTextDocument *sd = words.story(h);
        if (!sd) continue;
        int from = (k == 0) ? pos : (up ? sd->characterCount() : 0);
        if (from < 0) from = sd->characterCount();
        QTextCursor c = sd->find(q, from, flags);
        // (Words that are not shown are not found.)
        while (!c.isNull() && words.skip && words.skip(c)) c = sd->find(q, up ? c.selectionStart() : c.selectionEnd(), flags);
        if (c.isNull()) continue;
        *story = i;
        *found = c;
        return true;
    }
    return false;
}

int replaceInStories(const Document &doc, const QVector<FindStory> &stories, const QString &q, QTextDocument::FindFlags flags, const QString &with, QVector<FindStory> *changed)
{
    QVector<FindStory> mine;
    const int n = replaceInStories(wordsOfDocument(doc), stories, q, flags, with, &mine);
    for (const FindStory &h : mine)
        if (h.art)
            if (auto *a = dynamic_cast<TextArtItem *>(doc.item(h.itemId))) a->text = h.art->toPlainText();
    if (changed) *changed << mine;
    return n;
}

int replaceInStories(const FindWords &words, const QVector<FindStory> &stories, const QString &q, QTextDocument::FindFlags flags, const QString &with, QVector<FindStory> *changed)
{
    int n = 0;
    for (const FindStory &h : stories) {
        QTextDocument *sd = words.story(h);
        if (!sd) continue;
        const int was = n;
        QTextCursor c = sd->find(q, 0, flags & ~QTextDocument::FindBackward);
        while (!c.isNull()) {
            if (!(words.skip && words.skip(c))) {
                if (words.put) words.put(c, with);
                else c.insertText(with);
                ++n;
            }
            c = sd->find(q, c, flags & ~QTextDocument::FindBackward);
        }
        if (n == was) continue;
        if (changed) *changed << h;
    }
    return n;
}

void refitFoundStories(Editor *ed, const QVector<FindStory> &changed)
{
    QSet<QString> done;
    for (const FindStory &h : changed) {
        if (h.notes || h.art || h.itemId.isEmpty() || done.contains(h.itemId)) continue;
        done.insert(h.itemId);
        Item *it = ed->doc()->item(h.itemId);
        if (!it) continue;
        if (it->type() == ItemType::Text) {
            for (TextItem *f : ed->doc()->chainOf(it->id))
                if (f->autofit == TextItem::GrowBox) ed->autoGrowText(f);
        } else if (it->type() == ItemType::Table) {
            ed->fitTableRows(static_cast<TableItem *>(it));
        }
    }
}

bool replaceArtWords(const Document &doc, const QString &itemId, int start, int end, const QString &expected, Qt::CaseSensitivity cs, const QString &with)
{
    auto *a = dynamic_cast<TextArtItem *>(doc.item(itemId));
    if (!a) return false;
    QTextDocument words;
    words.setPlainText(a->text);
    if (start < 0 || end <= start || end >= words.characterCount()) return false;
    QTextCursor c(&words);
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    if (c.selectedText().compare(expected, cs) != 0) return false;
    c.insertText(with);
    a->text = words.toPlainText();
    return true;
}

QString findTextForMessage(const QString &text)
{
    QString out = text.left(40);
    if (out.size() < text.size()) {
        if (out.back().isHighSurrogate()) out.chop(1);
        out += QChar(0x2026);
    }
    // A zero width space after each 12 characters in a row without a space: somewhere for a long word to break.
    QString broken;
    int run = 0;
    for (int i = 0; i < out.size(); ++i) {
        broken += out[i];
        run = out[i].isSpace() ? 0 : run + 1;
        if (run >= 12 && i + 1 < out.size() && !out[i].isHighSurrogate() && !out[i + 1].isSpace()) {
            broken += QChar(0x200B);
            run = 0;
        }
    }
    return broken;
}

void showFindMatch(Editor *ed, const FindStory &h, const QTextCursor &found)
{
    if (!h.master.isEmpty()) ed->setMasterView(h.master);
    else if (h.page >= 0) ed->setCurrentPage(h.page);
    if (h.notes) return;
    if (h.art) {
        ed->endTextEdit();
        ed->select(h.itemId);
        return;
    }
    ed->beginTextEdit(h.itemId, found.selectionStart(), h.row, h.col, h.half);
    QTextCursor c = ed->cursor();
    c.setPosition(found.selectionStart());
    c.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);
    ed->setCursor(c);
}

} // namespace jp
