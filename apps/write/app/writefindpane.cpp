#include "app/writefindpane.h"

#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPair>
#include <QPointer>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFragment>
#include <QVBoxLayout>
#include <algorithm>

namespace jp {

// The story's words as a search sees them: its characters without the pieces kept as XML that show nothing (a bookmark between two words does not part them),
// as a document of their own, and where each run of them is in the story. A story with something in it that a document of plain text would not keep as it is
// (so that places could not be told apart) is searched as it is.
class StoryWords : public QObject {
public:
    explicit StoryWords(QTextDocument *story) : m_story(story)
    {
        rebuild();
        // (The words are made again, the next time they are asked for, after the story changes.)
        connect(story, &QTextDocument::contentsChanged, this, [this] { dirty = true; });
    }
    void rebuild()
    {
        dirty = false;
        m_runs.clear();
        QString text;
        auto extend = [&](int at, const QString &piece) {
            if (!m_runs.isEmpty() && m_runs.last().story + m_runs.last().length == at) m_runs.last().length += int(piece.size());
            else m_runs.push_back({int(text.size()), at, int(piece.size())});
            text += piece;
        };
        // (The endnotes the pages set after the text are not searched: they are the last paragraphs, marked as theirs.)
        for (QTextBlock b = m_story->begin(); b.isValid() && !b.blockFormat().hasProperty(write::prop::EndnoteBlock); b = b.next()) {
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                if (!write::isInvisibleAtom(it.fragment().charFormat())) extend(it.fragment().position(), it.fragment().text());
            if (b.next().isValid() && !b.next().blockFormat().hasProperty(write::prop::EndnoteBlock)) extend(b.position() + b.length() - 1, QStringLiteral("\n"));
        }
        m_words.setUndoRedoEnabled(false);
        m_words.setPlainText(text);
        m_identity = m_words.characterCount() != text.size() + 1;
        if (m_identity) m_runs.clear();
    }
    QTextDocument *story() const { return m_story; }
    QTextDocument *words() { return m_identity ? m_story.data() : &m_words; }

    // A place in the story as a place in the words: the first of them at or after it.
    int toWords(int at) const
    {
        if (m_identity || m_runs.isEmpty()) return at;
        const Run &r = runAt(at, false);
        return r.words + std::clamp(at - r.story, 0, r.length);
    }
    // The place of the character that the words have at a place (the place before it), and the place after the character before it (the end of a match).
    int toStory(int at) const
    {
        if (m_identity || m_runs.isEmpty()) return at;
        const Run &r = runAt(at, true);
        return r.story + std::min(at - r.words, r.length);
    }
    int toStoryEnd(int at) const { return at > 0 ? toStory(at - 1) + 1 : toStory(at); }
    // The places of the story that the words [from, to) are: runs of characters, in order, with the pieces kept as XML between them left out.
    QVector<QPair<int, int>> pieces(int from, int to) const
    {
        if (m_identity || m_runs.isEmpty()) return {{from, to}};
        QVector<QPair<int, int>> out;
        for (const Run &r : m_runs) {
            const int a = std::max(from, r.words), b = std::min(to, r.words + r.length);
            if (a < b) out.push_back({r.story + a - r.words, r.story + b - r.words});
        }
        return out;
    }
    bool dirty = false;

private:
    struct Run {
        int words = 0, story = 0, length = 0;
    };
    // The run that holds the place (in the words, or the story): the last that starts at or before it.
    const Run &runAt(int at, bool inWords) const
    {
        int lo = 0, hi = int(m_runs.size()) - 1;
        while (lo < hi) {
            const int mid = (lo + hi + 1) / 2;
            if ((inWords ? m_runs[mid].words : m_runs[mid].story) <= at) lo = mid;
            else hi = mid - 1;
        }
        return m_runs[lo];
    }
    QPointer<QTextDocument> m_story;
    QTextDocument m_words;
    QVector<Run> m_runs;
    bool m_identity = false;
};

WriteFindPane::WriteFindPane(WriteWindow *win, QWidget *parent) : QWidget(parent), m_win(win)
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
    v->addWidget(m_case);
    v->addWidget(m_whole);
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
    m_status->setTextFormat(Qt::PlainText);   // it says what was searched for, which may look like markup
    m_status->setWordWrap(true);
    v->addWidget(m_status);
    v->addStretch(1);
    connect(next, &QPushButton::clicked, this, [this] { findNext(); });
    connect(m_find, &QLineEdit::returnPressed, this, [this] { findNext(); });
    connect(rep, &QPushButton::clicked, this, [this] { replaceOne(); });
    connect(all, &QPushButton::clicked, this, [this] { replaceAll(); });
    setMinimumWidth(240);
}

WriteFindPane::~WriteFindPane() = default;

void WriteFindPane::focusBox(bool replace)
{
    QLineEdit *box = replace ? m_replace : m_find;
    box->setFocus(Qt::OtherFocusReason);
    box->selectAll();
}

QString WriteFindPane::status() const { return m_status->text(); }

QTextDocument::FindFlags WriteFindPane::flags() const
{
    QTextDocument::FindFlags f;
    if (m_case->isChecked()) f |= QTextDocument::FindCaseSensitively;
    if (m_whole->isChecked()) f |= QTextDocument::FindWholeWords;
    return f;
}

StoryWords *WriteFindPane::storyWords()
{
    QTextDocument *story = m_win->document() ? m_win->document()->story() : nullptr;
    if (!story) return nullptr;
    if (!m_text || m_text->story() != story) m_text = std::make_unique<StoryWords>(story);
    else if (m_text->dirty) m_text->rebuild();
    return m_text.get();
}

// The document's one story, as libs/page's search is given it: its words, and the places of the story that a swap puts new words in.
FindWords WriteFindPane::words()
{
    FindWords w;
    StoryWords *text = storyWords();
    QTextDocument *story = text ? text->story() : nullptr;
    w.story = [text](const FindStory &) { return text ? text->words() : nullptr; };
    // Hidden text is not found (nor replaced): it is not shown.
    w.skip = [text, story](const QTextCursor &match) {
        if (text->toStory(match.selectionStart()) >= write::editableEnd(story)) return true;   // (nor a match in the endnotes the pages set after the text)
        QTextCursor shown(story);
        shown.setPosition(text->toStory(match.selectionStart()));
        shown.setPosition(text->toStoryEnd(match.selectionEnd()), QTextCursor::KeepAnchor);
        return !write::allSelected(shown, [](const QTextCharFormat &f) { return !f.boolProperty(tp::HiddenText); });
    };
    // The new words take the place of the first of the characters of the match, and the look of the first of them; the characters of it between pieces kept as XML
    // are taken away, and the pieces stay. (The matches come from the last to the first, so that no swap moves another.)
    w.put = [text, story](QTextCursor &match, const QString &with) {
        const QVector<QPair<int, int>> pieces = text->pieces(match.selectionStart(), match.selectionEnd());
        if (pieces.isEmpty()) return;
        QTextCursor first(story);
        first.setPosition(pieces.first().first + 1);
        const QTextCharFormat look = first.charFormat();
        QTextCursor edit(story);
        edit.beginEditBlock();
        for (int i = int(pieces.size()) - 1; i >= 1; --i) {
            edit.setPosition(pieces[i].first);
            edit.setPosition(pieces[i].second, QTextCursor::KeepAnchor);
            edit.removeSelectedText();
        }
        edit.setPosition(pieces.first().first);
        edit.setPosition(pieces.first().second, QTextCursor::KeepAnchor);
        edit.insertText(with, look);
        edit.endEditBlock();
    };
    return w;
}

QVector<FindStory> WriteFindPane::stories() const
{
    QVector<FindStory> all;
    all << FindStory{QString(), QStringLiteral("story")};
    return all;
}

// Whether what is selected is a match, as the search sees it (so that Replace does not change words the search would not have shown).
bool WriteFindPane::selectionIsMatch()
{
    const QTextCursor &sel = m_win->view()->cursor();
    StoryWords *text = storyWords();
    if (!text || !sel.hasSelection() || m_find->text().isEmpty()) return false;
    const int from = text->toWords(sel.selectionStart()), to = text->toWords(sel.selectionEnd());
    const QTextCursor m = text->words()->find(m_find->text(), from, flags());
    return !m.isNull() && m.selectionStart() == from && m.selectionEnd() == to && !words().skip(m);
}

bool WriteFindPane::findNext()
{
    const QString q = m_find->text();
    StoryWords *text = storyWords();
    if (!text || q.isEmpty()) return false;
    int story = 0;
    QTextCursor found;
    if (findInStories(words(), stories(), q, flags(), 0, text->toWords(m_win->view()->cursor().selectionEnd()), &story, &found)) {
        QTextCursor shown(text->story());
        shown.setPosition(text->toStory(found.selectionStart()));
        shown.setPosition(text->toStoryEnd(found.selectionEnd()), QTextCursor::KeepAnchor);
        m_win->view()->showCursor(shown);
        m_status->clear();
        return true;
    }
    m_status->setText(tr("JeffWrite finished searching the document. \"%1\" was not found.").arg(findTextForMessage(q)));
    return false;
}

void WriteFindPane::replaceOne()
{
    if (!m_win->document() || m_find->text().isEmpty()) return;
    if (selectionIsMatch()) {
        StoryWords *text = storyWords();
        const QTextCursor &sel = m_win->view()->cursor();
        QTextCursor match(text->words());
        match.setPosition(text->toWords(sel.selectionStart()));
        match.setPosition(text->toWords(sel.selectionEnd()), QTextCursor::KeepAnchor);
        const int at = text->pieces(match.selectionStart(), match.selectionEnd()).first().first;
        words().put(match, m_replace->text());
        // The caret is after the new words, where the next search goes on from.
        QTextCursor after(text->story());
        after.setPosition(at + int(m_replace->text().size()));
        m_win->view()->setCursor(after);
    }
    findNext();
}

void WriteFindPane::replaceAll()
{
    const QString q = m_find->text();
    if (!m_win->document() || q.isEmpty()) return;
    // Each swap is a step of its own; the edit block makes them one.
    QTextCursor block(m_win->document()->story());
    block.beginEditBlock();
    const int n = replaceInStories(words(), stories(), q, flags(), m_replace->text());
    block.endEditBlock();
    m_status->setText(n == 0   ? tr("JeffWrite made no replacements. \"%1\" was not found.").arg(findTextForMessage(q))
                      : n == 1 ? tr("JeffWrite made 1 replacement.")
                               : tr("JeffWrite made %1 replacements.").arg(n));
}

} // namespace jp
