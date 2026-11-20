#include "app/spellpane.h"

#include "app/pageview.h"
#include "app/grammar.h"
#include "app/settings.h"
#include "app/spelling.h"
#include "app/grammarcheck.h"
#include "core/editops.h"
#include "app/shapetextedit.h"
#include "core/pictureops.h"
#include "core/spellcheck.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QAction>
#include <QCoreApplication>
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>
#include <algorithm>
#include <memory>
#include <tuple>
#include "app/plaintext.h"

namespace jp::write {

namespace {

// The language of the word at a position: the text's own, else the document's.
QString languageAt(SpellCheck *spelling, int position)
{
    QTextCursor c(spelling->story());
    c.setPosition(position + 1);
    const QString language = c.charFormat().stringProperty(tp::Language);
    return language.isEmpty() ? spelling->defaultLanguage() : language;
}

constexpr int kSliceMs = 30;   // the time the pane's search takes at a time

} // namespace

bool addSpellingSuggestions(QMenu &menu, PageView *view)
{
    // (Of what is being edited: the text, or the header or footer.)
    SpellCheck *spelling = view ? view->editedSpelling() : nullptr;
    // (a document open for reading only is not changed by a correction: none is offered)
    if (!view || !view->document() || view->readOnly() || !spelling || !spelling->enabled() || !spelling->story()) return false;
    const QPair<int, int> word = spelling->misspellingAt(view->contextPosition());
    if (word.first < 0) return false;
    QTextDocument *story = spelling->story();
    QTextCursor w(story);
    w.setPosition(word.first);
    w.setPosition(word.second, QTextCursor::KeepAnchor);
    const QString text = w.selectedText();
    const QStringList suggestions = spellingSuggestions(text, languageAt(spelling, word.first)).mid(0, 6);
    QPointer<PageView> where = view;
    bool first = true;
    for (const QString &s : suggestions) {
        QAction *a = menu.addAction(s);
        if (first) {
            QFont f = a->font();
            f.setBold(true);
            a->setFont(f);
            first = false;
        }
        QPointer<QTextDocument> whose = story;
        QObject::connect(a, &QAction::triggered, &menu, [where, whose, word, s] {
            if (!where || !whose) return;
            replaceWord(whose, word.first, word.second, s);
            QTextCursor c = where->cursor();
            c.setPosition(word.first + int(s.size()));
            where->setCursor(c);
        });
    }
    if (suggestions.isEmpty()) menu.addAction(QCoreApplication::translate("Proofing", "(No spelling suggestions)"))->setEnabled(false);
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Ignore All")), &QAction::triggered, &menu, [where, text] {
        spellingIgnore(text);
        if (where) where->editedSpelling()->refresh();
    });
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Add to Dictionary")), &QAction::triggered, &menu, [where, text] {
        QString failure;
        if (!spellingAdd(text, &failure)) spellingReportFailure(where, failure);
        if (where) where->editedSpelling()->refresh();
    });
    menu.addSeparator();
    return true;
}

bool addGrammarSuggestions(QMenu &menu, PageView *view)
{
    GrammarCheck *grammar = view ? view->grammar() : nullptr;
    if (!view || !view->document() || view->readOnly() || view->inHeaderFooter() || view->inNote() || !grammar || !grammar->enabled() || !grammar->story()) return false;
    const GrammarCheck::Issue issue = grammar->issueAt(view->contextPosition());
    if (!issue.valid()) return false;
    QTextDocument *story = grammar->story();
    menu.addAction(issue.message)->setEnabled(false);
    QPointer<PageView> where = view;
    QPointer<QTextDocument> whose = story;
    bool first = true;
    for (const QString &s : issue.suggestions) {
        QAction *a = menu.addAction(s.isEmpty() ? QCoreApplication::translate("Proofing", "(Delete)") : s);
        if (first) {
            QFont f = a->font();
            f.setBold(true);
            a->setFont(f);
            first = false;
        }
        QObject::connect(a, &QAction::triggered, &menu, [where, whose, issue, s] {
            if (!where || !whose) return;
            replaceWord(whose, issue.start, issue.end, s);
            QTextCursor c = where->cursor();
            c.setPosition(std::min(issue.start + int(s.size()), whose->characterCount() - 1));
            where->setCursor(c);
        });
    }
    // This one, in its place (the same words elsewhere are still marked), and every one of this kind on these words.
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Ignore This Issue")), &QAction::triggered, &menu, [where, issue] {
        if (where && where->grammar()) where->grammar()->ignoreHere(issue);
    });
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Ignore All")), &QAction::triggered, &menu, [where, issue] {
        if (where && where->grammar()) where->grammar()->ignoreAll(issue);
    });
    menu.addSeparator();
    return true;
}

// ---------------- the Spelling pane ----------------

SpellPane::SpellPane(PageView *view, QWidget *parent) : QWidget(parent), m_view(view)
{
    setObjectName(QStringLiteral("spellPane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(8);
    m_status = plainLabel(new QLabel(this));
    m_status->setObjectName(QStringLiteral("spellStatus"));
    m_status->setWordWrap(true);
    v->addWidget(m_status);
    m_result = plainLabel(new QLabel(this));
    m_result->setObjectName(QStringLiteral("spellResult"));
    m_result->setWordWrap(true);
    m_result->hide();
    v->addWidget(m_result);
    m_wordLabel = plainLabel(new QLabel(this));
    m_wordLabel->setObjectName(QStringLiteral("spellWord"));
    QFont f = m_wordLabel->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 1.3);
    m_wordLabel->setFont(f);
    m_wordLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    v->addWidget(m_wordLabel);
    m_context = new QLabel(this);
    m_context->setObjectName(QStringLiteral("spellContext"));
    m_context->setWordWrap(true);
    m_context->setTextFormat(Qt::RichText);
    v->addWidget(m_context);
    v->addWidget(new QLabel(tr("Suggestions"), this));
    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("spellSuggestions"));
    m_list->setAccessibleName(tr("Suggestions"));
    v->addWidget(m_list, 1);
    auto *grid = new QGridLayout();
    auto make = [&](const char *name, const QString &text, int row, int column, int across = 1) {
        auto *b = new QPushButton(text, this);
        b->setObjectName(QString::fromLatin1(name));
        grid->addWidget(b, row, column, 1, across);
        return b;
    };
    m_change = make("spellChange", tr("Change"), 0, 0);
    m_changeAll = make("spellChangeAll", tr("Change All"), 0, 1);
    m_ignore = make("spellIgnore", tr("Ignore"), 1, 0);
    m_ignoreAll = make("spellIgnoreAll", tr("Ignore All"), 1, 1);
    m_add = make("spellAdd", tr("Add to Dictionary"), 2, 0, 2);
    m_change->setDefault(true);
    v->addLayout(grid);
    connect(m_change, &QPushButton::clicked, this, [this] { change(false); });
    connect(m_changeAll, &QPushButton::clicked, this, [this] { change(true); });
    connect(m_ignore, &QPushButton::clicked, this, [this] { ignore(false); });
    connect(m_ignoreAll, &QPushButton::clicked, this, [this] { ignore(true); });
    connect(m_add, &QPushButton::clicked, this, [this] { addToDictionary(); });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] { change(false); });
    m_timer.setSingleShot(true);
    m_timer.setInterval(0);
    connect(&m_timer, &QTimer::timeout, this, [this] { search(); });
    finish();
    m_complete = false;
    m_status->clear();
}

// The places the pane goes through: the text, then the text boxes of the text (in the order of the text), then the footnotes, then the headers and footers (the stories that the review
// code, WriteDoc::storiesWithChanges, goes through).
void SpellPane::collectStories()
{
    m_parts.clear();
    WriteDoc *doc = m_view->document();
    QTextDocument *body = doc->story();
    Part text;
    text.story = body;
    m_parts.push_back(std::move(text));
    for (QTextBlock b = body->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (!f.charFormat().hasProperty(prop::Atom)) continue;
            for (int i = 0; i < int(f.length()); ++i) {
                const int at = f.position() + i;
                if (!isDrawnObjectAt(body, at)) continue;
                const PictureInfo info = pictureAt(*doc, body, at);
                if (!info.valid || !info.editable || (info.kind != DrawnKind::Shape && info.kind != DrawnKind::TextBox)) continue;
                std::unique_ptr<QTextDocument> words = textBoxStory(*doc, at);
                if (!words || words->toPlainText().trimmed().isEmpty()) continue;
                Part box;
                box.owned = std::move(words);
                box.story = box.owned.get();
                box.isBox = true;
                box.box = QTextCursor(body);
                box.box.setPosition(at);
                m_parts.push_back(std::move(box));
            }
        }
    struct Key {
        int part, a, b, c;
        QTextDocument *story;
    };
    QVector<Key> keys;
    for (QTextDocument *d : doc->storiesWithChanges(true)) {
        if (!d || d == body) continue;
        int section = 0, slot = 0, id = 0;
        bool footer = false;
        if (doc->footnoteIdOf(d, &id)) keys.push_back({0, id, 0, 0, d});
        else if (doc->headerFooterSlotOf(d, &section, &footer, &slot)) keys.push_back({1, section, footer ? 1 : 0, slot, d});
    }
    std::stable_sort(keys.begin(), keys.end(), [](const Key &x, const Key &y) { return std::tie(x.part, x.a, x.b, x.c) < std::tie(y.part, y.a, y.b, y.c); });
    for (const Key &k : keys) {
        const bool seen = std::any_of(m_parts.begin(), m_parts.end(), [&](const Part &p) { return p.story == k.story; });
        if (seen) continue;
        Part side;
        side.story = k.story;
        m_parts.push_back(std::move(side));
    }
}

// The check of a place: the text's is the view's own, the others are the pane's (made when first asked for).
SpellCheck *SpellPane::checkFor(Part &part)
{
    if (!part.isBox && part.story == m_view->document()->story()) return m_view->spelling();
    if (!part.check && part.story) {
        part.check = std::make_unique<SpellCheck>(part.story);
        part.check->setDefaultLanguage(m_view->document()->proofingLanguage());
    }
    return part.check.get();
}

// Whether the pane goes to this place: the text and the text boxes, and a header, footer, or note that a page shows. (Change All changes the same places.)
bool SpellPane::visits(Part &part)
{
    if (!part.story) return false;
    if (part.isBox) return isDrawnObjectAt(m_view->document()->story(), part.box.position());
    if (part.story == m_view->document()->story()) return true;
    return m_view->showsSideStory(part.story);
}

// The words of a text box as they stand: the editor's while the box is open for typing, else the pane's copy. For any other place, its story.
QTextDocument *SpellPane::liveStory()
{
    if (m_idx >= 0 && m_idx < int(m_parts.size()) && m_parts[m_idx].isBox && m_view->editingShapeText() && m_view->shapeEditorAt() == m_parts[m_idx].box.position() && m_view->shapeEditor())
        return m_view->shapeEditor()->story();
    return m_story;
}

// The pane's copy of a text box's words, made again from the document (after the words were put in it).
void SpellPane::refreshBox(Part &part)
{
    const int at = part.box.position();
    part.check.reset();
    part.owned = at >= 0 && isDrawnObjectAt(m_view->document()->story(), at) ? textBoxStory(*m_view->document(), at) : nullptr;
    part.story = part.owned.get();
}

// The words typed in a text box that is open are put in the document (one step that undoes), and the copy is made again from them.
void SpellPane::commitBox(Part &part)
{
    m_view->commitShapeText(false);
    refreshBox(part);
}

// Shows a place for the pane's word: the text, a text box opened for typing, or the header, footer, or note edited in place. False where nothing can show it (it is passed over).
bool SpellPane::reveal(Part &part)
{
    QTextDocument *story = part.story;
    if (part.isBox) {
        const int at = part.box.position();
        if (at < 0 || !isDrawnObjectAt(m_view->document()->story(), at)) return false;
        if (m_view->inHeaderFooter()) m_view->leaveHeaderFooter();
        if (m_view->inNote()) m_view->leaveNote();
        if (m_view->editingShapeText() && m_view->shapeEditorAt() == at) return true;
        if (m_view->editingShapeText() && !m_view->endShapeTextEdit(true)) return false;
        m_view->selectPicture(at);
        return m_view->beginShapeTextEdit();
    }
    if (m_view->editingShapeText()) m_view->endShapeTextEdit(true);
    if (story == m_view->document()->story()) {
        if (m_view->inHeaderFooter()) m_view->leaveHeaderFooter();
        if (m_view->inNote()) m_view->leaveNote();
        return true;
    }
    if (m_view->editedSideStory() == story) return true;
    return m_view->enterSideStory(story);
}

// On to the next place of the ring (the text again, from the top to where the check began, when it comes round); false when the check has been all the way round.
bool SpellPane::nextStory()
{
    if (m_wrapped) return false;
    m_idx = (m_idx + 1) % int(m_parts.size());
    m_pos = 0;
    if (m_idx == m_startIdx) {
        if (m_start <= 0) return false;
        m_wrapped = true;
    }
    return true;
}

void SpellPane::start()
{
    m_timer.stop();
    m_searching = false;
    m_result->clear();
    m_result->hide();
    if (!m_view->document() || !m_view->spelling()) {
        finish();
        return;
    }
    collectStories();
    // (From the caret: in the text, or in the header, footer, note, or text box that is being edited.)
    const QTextDocument *edited = m_view->editedSideStory();
    m_startIdx = 0;
    int boxStart = -1;
    if (edited) {
        m_startIdx = -1;
        for (int k = 0; k < int(m_parts.size()); ++k)
            if (m_parts[k].story == edited) m_startIdx = k;
    } else if (m_view->editingShapeText() && m_view->shapeEditor()) {
        for (int k = 0; k < int(m_parts.size()); ++k)
            if (m_parts[k].isBox && m_parts[k].box.position() == m_view->shapeEditorAt()) {
                m_startIdx = k;
                boxStart = m_view->shapeEditor()->textCursor().selectionStart();
            }
    }
    if (m_startIdx < 0) {
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        m_startIdx = 0;
    }
    m_idx = m_startIdx;
    m_story = m_parts[m_idx].story;
    m_start = boxStart >= 0 ? boxStart : m_view->cursor().selectionStart();
    m_grammarPhase = false;
    m_grammarRan = false;
    m_issue = GrammarCheck::Issue();
    m_wrapped = false;
    m_complete = false;
    find(m_start);
}

// Looks for the next misspelled word from a place: a slice now, and more in the event loop's free time if the document is long.
void SpellPane::find(int from)
{
    m_pos = from;
    search();
}

void SpellPane::search()
{
    if (m_grammarPhase) {
        searchGrammar();
        return;
    }
    // (From the caret to the end of its story, then the other stories, then the top to where it began: a few slices at most before the pane says anything.)
    for (int turn = 0; turn < 8; ++turn) {
        Part &part = m_parts[m_idx];
        QTextDocument *story = part.story;
        QPair<int, int> found;
        int next = m_pos;
        bool onward = false;
        // (A place that is gone, a note that was taken out while the pane was open, is passed over.)
        switch (story ? checkFor(part)->search(m_pos, kSliceMs, &found, &next) : SpellCheck::Step::None) {
        case SpellCheck::Step::Found:
            if (m_wrapped && m_idx == m_startIdx && found.first >= m_start) {   // (back to where it began)
                spellingDone();
                return;
            }
            if (reveal(part)) {
                show(story, found);
                return;
            }
            onward = true;   // (no page shows this one)
            break;
        case SpellCheck::Step::More:
            m_pos = next;
            if (m_wrapped && m_idx == m_startIdx && m_pos >= m_start) {
                spellingDone();
                return;
            }
            checking();
            m_timer.start();
            return;
        case SpellCheck::Step::None:
            onward = true;
            break;
        }
        if (onward && !nextStory()) {
            spellingDone();
            return;
        }
    }
    checking();
    m_timer.start();
}

// The spelling is done: on to the grammar and style issues, from where the check began, when they are checked; else the pane is complete.
void SpellPane::spellingDone()
{
    GrammarCheck *grammar = m_view->grammar();
    if (!grammar || !grammar->enabled() || !grammarAvailable() || !m_view->grammarEnabled() || !grammarAsYouType()) {
        finish();
        return;
    }
    // (The grammar is of the text: its caret is the text's, and the check begins at the text's caret.)
    if (m_view->editingShapeText()) m_view->endShapeTextEdit(true);
    m_view->leaveHeaderFooter();
    m_view->leaveNote();
    m_story = m_view->document()->story();
    m_startIdx = 0;
    m_idx = 0;
    m_start = m_view->cursor().selectionStart();
    m_grammarPhase = true;
    m_grammarRan = true;
    m_wrapped = false;
    m_pos = m_start;
    searchGrammar();
}

void SpellPane::searchGrammar()
{
    GrammarCheck *grammar = m_view->grammar();
    for (int turn = 0; turn < 2; ++turn) {
        GrammarCheck::Issue found;
        int next = m_pos;
        switch (grammar->search(m_pos, kSliceMs, &found, &next)) {
        case GrammarCheck::Step::Found:
            if (m_wrapped && found.start >= m_start) finish();
            else showIssue(found);
            return;
        case GrammarCheck::Step::More:
            m_pos = next;
            if (m_wrapped && m_pos >= m_start) {
                finish();
                return;
            }
            checking();
            m_timer.start();
            return;
        case GrammarCheck::Step::None:
            if (m_wrapped || m_start <= 0) {
                finish();
                return;
            }
            m_wrapped = true;
            m_pos = 0;
            break;
        }
    }
    checking();
    m_timer.start();
}

void SpellPane::showIssue(const GrammarCheck::Issue &issue)
{
    QTextDocument *story = m_view->document()->story();
    m_story = story;
    QTextCursor c(story);
    c.setPosition(issue.start);
    c.setPosition(issue.end, QTextCursor::KeepAnchor);
    m_range = qMakePair(issue.start, issue.end);
    m_issue = issue;
    m_word = c.selectedText();
    m_complete = false;
    m_searching = false;
    m_view->setCursor(c);
    m_status->setText(tr("Grammar: %1").arg(issue.message));
    m_wordLabel->setText(m_word);
    const QTextBlock block = story->findBlock(issue.start);
    const QString text = block.text();
    const int at = issue.start - block.position(), length = issue.end - issue.start;
    const int from = std::max(0, at - 50), to = std::min(int(text.size()), at + length + 50);
    m_context->setText((from > 0 ? QStringLiteral("…") : QString()) + text.mid(from, at - from).toHtmlEscaped() + QStringLiteral("<b>") + text.mid(at, length).toHtmlEscaped() +
                       QStringLiteral("</b>") + text.mid(at + length, to - at - length).toHtmlEscaped() + (to < text.size() ? QStringLiteral("…") : QString()));
    m_list->clear();
    for (const QString &s : issue.suggestions) {
        auto *item = new QListWidgetItem(s.isEmpty() ? tr("(Delete)") : s, m_list);
        item->setData(Qt::UserRole, s);
    }
    if (m_list->count() > 0) m_list->setCurrentRow(0);
    for (QPushButton *b : {m_ignore, m_ignoreAll}) b->setEnabled(true);
    for (QPushButton *b : {m_changeAll, m_add}) b->setEnabled(false);
    m_change->setEnabled(m_list->count() > 0);
}

// The pane while the search goes on: it says so, and has nothing to offer until the next word is found.
void SpellPane::checking()
{
    m_searching = true;
    m_complete = false;
    m_issue = GrammarCheck::Issue();
    m_range = qMakePair(-1, -1);
    m_word.clear();
    m_wordLabel->clear();
    m_context->clear();
    m_list->clear();
    m_status->setText(m_grammarPhase ? tr("Checking grammar…") : tr("Checking spelling…"));
    for (QPushButton *b : {m_change, m_changeAll, m_ignore, m_ignoreAll, m_add}) b->setEnabled(false);
}

// A pane that is not shown does not search; shown again, it goes on.
void SpellPane::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    m_timer.stop();
}

void SpellPane::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (m_searching) m_timer.start();
}

void SpellPane::show(QTextDocument *story, const QPair<int, int> &range)
{
    m_story = story;
    QTextCursor c(story);
    c.setPosition(range.first);
    c.setPosition(range.second, QTextCursor::KeepAnchor);
    m_range = range;
    m_word = c.selectedText();
    m_complete = false;
    m_searching = false;
    Part &part = m_parts[m_idx];
    if (part.isBox && m_view->shapeEditor()) {
        // (The word is selected in the words of the text box, which are open for typing.)
        QTextCursor inBox(m_view->shapeEditor()->story());
        inBox.setPosition(range.first);
        inBox.setPosition(range.second, QTextCursor::KeepAnchor);
        m_view->shapeEditor()->setTextCursor(inBox);
    } else {
        m_view->setCursor(c);
    }
    int section = 0, slot = 0;
    bool footer = false;
    if (part.isBox) m_status->setText(tr("Not in dictionary, in a text box"));
    else if (story == m_view->document()->story()) m_status->setText(tr("Not in dictionary"));
    else if (m_view->document()->headerFooterSlotOf(story, &section, &footer, &slot)) m_status->setText(footer ? tr("Not in dictionary, in a footer") : tr("Not in dictionary, in a header"));
    else m_status->setText(tr("Not in dictionary, in a note"));
    m_wordLabel->setText(m_word);
    // The words around it, with it in bold.
    const QTextBlock block = story->findBlock(range.first);
    const QString text = block.text();
    const int at = range.first - block.position(), length = range.second - range.first;
    const int from = std::max(0, at - 50), to = std::min(int(text.size()), at + length + 50);
    m_context->setText((from > 0 ? QStringLiteral("…") : QString()) + text.mid(from, at - from).toHtmlEscaped() + QStringLiteral("<b>") + text.mid(at, length).toHtmlEscaped() +
                       QStringLiteral("</b>") + text.mid(at + length, to - at - length).toHtmlEscaped() + (to < text.size() ? QStringLiteral("…") : QString()));
    m_list->clear();
    QTextCursor look(story);
    look.setPosition(range.first + 1);
    const QString language = look.charFormat().stringProperty(tp::Language);
    m_list->addItems(spellingSuggestions(m_word, language.isEmpty() ? checkFor(part)->defaultLanguage() : language).mid(0, 12));
    if (m_list->count() > 0) m_list->setCurrentRow(0);
    for (QPushButton *b : {m_ignore, m_ignoreAll, m_add}) b->setEnabled(true);
    // (A word of the endnotes the pages set is not changed anywhere: it can be ignored or added, not changed.)
    for (QPushButton *b : {m_change, m_changeAll}) b->setEnabled(m_list->count() > 0 && canReplaceWord(story, range.first, range.second));
}

void SpellPane::finish()
{
    m_timer.stop();
    m_searching = false;
    m_complete = true;
    m_issue = GrammarCheck::Issue();
    m_range = qMakePair(-1, -1);
    m_word.clear();
    m_wordLabel->clear();
    m_context->clear();
    m_list->clear();
    m_status->setText(m_grammarRan ? tr("The spelling and grammar check is complete.") : tr("The spelling check is complete."));
    for (QPushButton *b : {m_change, m_changeAll, m_ignore, m_ignoreAll, m_add}) b->setEnabled(false);
}

// Whether the word shown is still where it was (the text may have been edited while the pane was open); if not, the check goes on from the caret.
bool SpellPane::stillThere()
{
    QTextDocument *live = liveStory();
    if (m_range.first >= 0 && m_view->document() && live) {
        QTextCursor c(live);
        if (m_range.second <= live->characterCount() - 1) {
            c.setPosition(m_range.first);
            c.setPosition(m_range.second, QTextCursor::KeepAnchor);
            if (c.selectedText() == m_word) return true;
        }
    }
    start();
    return false;
}

void SpellPane::change(bool all)
{
    if (m_complete || m_searching || !m_list->currentItem() || m_view->readOnly() || !stillThere()) return;   // (a document open for reading only is not changed by a correction)
    m_result->clear();
    m_result->hide();
    QTextDocument *story = liveStory();
    if (!m_grammarPhase && !canReplaceWord(story, m_range.first, m_range.second)) return;
    const QString chosen = m_grammarPhase ? m_list->currentItem()->data(Qt::UserRole).toString() : m_list->currentItem()->text(), word = m_word;
    if (m_grammarPhase) {
        if (all) return;
        const int first = m_range.first, delta = int(chosen.size() - word.size());
        replaceWord(story, m_range.first, m_range.second, chosen);
        if (m_wrapped) m_start += delta;
        find(first + int(chosen.size()));
        return;
    }
    const int first = m_range.first, delta = int(chosen.size() - word.size());
    int resume = first + int(chosen.size());
    if (!all) {
        replaceWord(story, m_range.first, m_range.second, chosen);
        if (m_parts[m_idx].isBox) commitBox(m_parts[m_idx]);
        if (m_wrapped && m_idx == m_startIdx) m_start += delta;   // (the word is before where the check began)
    } else {
        // Every misspelling of the same word in the places the pane goes through (the text, its text boxes, and the notes, headers, and footers that a page shows), from the last, in one
        // step that undoes: the places the word is in the text, those the check calls misspelled whole, and that can be changed (not a word of the endnotes the pages set).
        // (A text box that is open for typing is put in the document first, so that its words are the document's.)
        if (m_view->editingShapeText() && !m_view->endShapeTextEdit(true)) return;
        struct Same {
            int part;
            QTextDocument *story;
            QVector<QPair<int, int>> places;
        };
        QVector<Same> found;
        int before = 0, beforeStart = 0;
        for (int k = 0; k < int(m_parts.size()); ++k) {
            Part &p = m_parts[k];
            if (p.isBox) refreshBox(p);   // (as the document has the words now)
            QTextDocument *where = p.story;
            if (!where || !visits(p)) continue;
            Same one{k, where, {}};
            SpellCheck *check = checkFor(p);
            for (QTextBlock b = where->begin(); b.isValid(); b = b.next()) {
                const QString text = b.text();
                for (int at = int(text.indexOf(word)); at >= 0; at = int(text.indexOf(word, at + 1))) {
                    const int pos = b.position() + at;
                    const QPair<int, int> w = check->misspellingAt(pos);
                    if (w.first == pos && w.second == pos + int(word.size()) && canReplaceWord(where, w.first, w.second)) one.places << w;
                }
            }
            // What changed length before the word, and before where the check began, moved them.
            for (const auto &w : std::as_const(one.places)) {
                if (k == m_idx && w.first < first) ++before;
                if (k == m_startIdx && w.first < m_start) ++beforeStart;
            }
            if (!one.places.isEmpty()) found << one;
        }
        int words = 0, places = 0;
        {
            const WriteDoc::StepGroup together(*m_view->document());
            for (const Same &one : std::as_const(found)) {
                QTextCursor block(one.story);
                block.beginEditBlock();
                for (int i = int(one.places.size()) - 1; i >= 0; --i) replaceWord(one.story, one.places[i].first, one.places[i].second, chosen);
                block.endEditBlock();
                Part &p = m_parts[one.part];
                // (The words of a text box go into its drawing, and are read again from it.)
                if (p.isBox && !setTextBoxStory(*m_view->document(), p.box.position(), one.story)) continue;
                words += int(one.places.size());
                ++places;
            }
        }
        for (Part &p : m_parts)
            if (p.isBox) refreshBox(p);
        resume += before * delta;
        m_start += beforeStart * delta;
        m_result->setText(tr("%n word changed.", nullptr, words) + QLatin1Char(' ') + tr("Found in %n place.", nullptr, places));
        m_result->show();
    }
    find(resume);
}

// The words accepted have changed: the pages are drawn again (the text's, and the header, footer, or note being edited).
void SpellPane::refreshChecks()
{
    m_view->spelling()->refresh();
    if (m_view->editedSpelling() != m_view->spelling()) m_view->editedSpelling()->refresh();
}

void SpellPane::ignore(bool all)
{
    if (m_complete || m_searching || !stillThere()) return;
    m_result->hide();
    const int to = m_range.second;
    if (all && m_grammarPhase) {
        m_view->grammar()->ignoreAll(m_issue);
    } else if (all) {
        spellingIgnore(m_word);
        refreshChecks();
    }
    find(to);
}

void SpellPane::addToDictionary()
{
    if (m_complete || m_searching || m_grammarPhase || !stillThere()) return;
    m_result->hide();
    const int to = m_range.second;
    QString failure;
    if (!spellingAdd(m_word, &failure)) spellingReportFailure(this, failure);
    refreshChecks();
    find(to);
}

} // namespace jp::write
