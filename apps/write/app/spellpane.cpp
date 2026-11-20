#include "app/spellpane.h"

#include "app/pageview.h"
#include "app/grammar.h"
#include "app/settings.h"
#include "app/spelling.h"
#include "app/grammarcheck.h"
#include "core/editops.h"
#include "core/spellcheck.h"
#include "core/writedoc.h"
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

// The stories the pane goes through: the text, then the footnotes, then the headers and footers (the stories that the review code, WriteDoc::storiesWithChanges, goes through).
void SpellPane::collectStories()
{
    m_order.clear();
    m_checks.clear();
    WriteDoc *doc = m_view->document();
    QTextDocument *body = doc->story();
    m_order.push_back(body);
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
    for (const Key &k : keys)
        if (!m_order.contains(k.story)) m_order.push_back(k.story);
}

// The check of a story: the text's is the view's own, the others are the pane's (made when first asked for).
SpellCheck *SpellPane::checkFor(QTextDocument *story)
{
    if (story == m_view->document()->story()) return m_view->spelling();
    auto &check = m_checks[story];
    if (!check) {
        check = std::make_unique<SpellCheck>(story);
        check->setDefaultLanguage(m_view->document()->proofingLanguage());
    }
    return check.get();
}

// Shows a story for the pane's word: the text, or the header, footer, or note edited in place. False where no page shows it (it is passed over).
bool SpellPane::reveal(QTextDocument *story)
{
    if (story == m_view->document()->story()) {
        if (m_view->inHeaderFooter()) m_view->leaveHeaderFooter();
        if (m_view->inNote()) m_view->leaveNote();
        return true;
    }
    if (m_view->editedSideStory() == story) return true;
    return m_view->enterSideStory(story);
}

// On to the next story of the ring (the text again, from the top to where the check began, when it comes round); false when the check has been all the way round.
bool SpellPane::nextStory()
{
    if (m_wrapped) return false;
    m_idx = (m_idx + 1) % int(m_order.size());
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
    if (!m_view->document() || !m_view->spelling()) {
        finish();
        return;
    }
    collectStories();
    // (From the caret: in the text, or in the header, footer, or note that is being edited.)
    const QTextDocument *edited = m_view->editedSideStory();
    m_startIdx = edited ? int(m_order.indexOf(const_cast<QTextDocument *>(edited))) : 0;
    if (m_startIdx < 0) {
        m_view->leaveHeaderFooter();
        m_view->leaveNote();
        m_startIdx = 0;
    }
    m_idx = m_startIdx;
    m_story = m_order[m_idx];
    m_start = m_view->cursor().selectionStart();
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
        QTextDocument *story = m_order.value(m_idx);
        if (!story) {
            spellingDone();
            return;
        }
        QPair<int, int> found;
        int next = m_pos;
        bool onward = false;
        switch (checkFor(story)->search(m_pos, kSliceMs, &found, &next)) {
        case SpellCheck::Step::Found:
            if (m_wrapped && m_idx == m_startIdx && found.first >= m_start) {   // (back to where it began)
                spellingDone();
                return;
            }
            if (reveal(story)) {
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
    m_view->setCursor(c);
    int section = 0, slot = 0;
    bool footer = false;
    if (story == m_view->document()->story()) m_status->setText(tr("Not in dictionary"));
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
    m_list->addItems(spellingSuggestions(m_word, language.isEmpty() ? checkFor(story)->defaultLanguage() : language).mid(0, 12));
    if (m_list->count() > 0) m_list->setCurrentRow(0);
    for (QPushButton *b : {m_ignore, m_ignoreAll, m_add}) b->setEnabled(true);
    // (A word of the endnotes the pages set is not changed anywhere: it can be ignored or added, not changed.)
    for (QPushButton *b : {m_change, m_changeAll}) b->setEnabled(m_list->count() > 0 && !touchesEndnotes(story, range.first, range.second));
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
    if (m_range.first >= 0 && m_view->document() && m_story) {
        QTextCursor c(m_story);
        if (m_range.second <= m_story->characterCount() - 1) {
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
    QTextDocument *story = m_story;
    if (!m_grammarPhase && touchesEndnotes(story, m_range.first, m_range.second)) return;
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
        if (m_wrapped && m_idx == m_startIdx) m_start += delta;   // (the word is before where the check began)
    } else {
        // Every misspelling of the same word, in every story the pane goes through, from the last, in one step that undoes: the places the word is in the text, those the check
        // calls misspelled whole.
        struct Same {
            QTextDocument *story;
            QVector<QPair<int, int>> places;
        };
        QVector<Same> found;
        int before = 0, beforeStart = 0;
        for (int k = 0; k < m_order.size(); ++k) {
            QTextDocument *where = m_order[k];
            if (!where) continue;
            Same one{where, {}};
            SpellCheck *check = checkFor(where);
            for (QTextBlock b = where->begin(); b.isValid(); b = b.next()) {
                const QString text = b.text();
                for (int at = int(text.indexOf(word)); at >= 0; at = int(text.indexOf(word, at + 1))) {
                    const int pos = b.position() + at;
                    const QPair<int, int> w = check->misspellingAt(pos);
                    if (w.first == pos && w.second == pos + int(word.size())) one.places << w;
                }
            }
            // What changed length before the word, and before where the check began, moved them.
            for (const auto &w : std::as_const(one.places)) {
                if (where == story && w.first < first) ++before;
                if (k == m_startIdx && w.first < m_start) ++beforeStart;
            }
            if (!one.places.isEmpty()) found << one;
        }
        const WriteDoc::StepGroup together(*m_view->document());
        for (const Same &one : std::as_const(found)) {
            QTextCursor block(one.story);
            block.beginEditBlock();
            for (int i = int(one.places.size()) - 1; i >= 0; --i) replaceWord(one.story, one.places[i].first, one.places[i].second, chosen);
            block.endEditBlock();
        }
        resume += before * delta;
        m_start += beforeStart * delta;
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
    const int to = m_range.second;
    QString failure;
    if (!spellingAdd(m_word, &failure)) spellingReportFailure(this, failure);
    refreshChecks();
    find(to);
}

} // namespace jp::write
