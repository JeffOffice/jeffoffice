#include "app/spellpane.h"

#include "app/pageview.h"
#include "app/spelling.h"
#include "core/spellcheck.h"
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
    if (!view || !view->document() || !spelling || !spelling->enabled() || !spelling->story()) return false;
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

// ---------------- the Spelling pane ----------------

SpellPane::SpellPane(PageView *view, QWidget *parent) : QWidget(parent), m_view(view)
{
    setObjectName(QStringLiteral("spellPane"));
    auto *v = new QVBoxLayout(this);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(8);
    m_status = new QLabel(this);
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
    v->addWidget(new QLabel(tr("Suggestions:"), this));
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

void SpellPane::start()
{
    m_timer.stop();
    m_searching = false;
    if (!m_view->document() || !m_view->spelling()) {
        finish();
        return;
    }
    m_view->leaveHeaderFooter();   // (the check is of the text: its caret is the text's)
    m_view->leaveNote();
    m_start = m_view->cursor().selectionStart();
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
    SpellCheck *check = m_view->spelling();
    // (From the caret to the end, then the top to where it began: two slices at most before the pane says anything.)
    for (int turn = 0; turn < 2; ++turn) {
        QPair<int, int> found;
        int next = m_pos;
        switch (check->search(m_pos, kSliceMs, &found, &next)) {
        case SpellCheck::Step::Found:
            if (m_wrapped && found.first >= m_start) finish();   // (back to where it began)
            else show(found);
            return;
        case SpellCheck::Step::More:
            m_pos = next;
            if (m_wrapped && m_pos >= m_start) {
                finish();
                return;
            }
            checking();
            m_timer.start();
            return;
        case SpellCheck::Step::None:
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

// The pane while the search goes on: it says so, and has nothing to offer until the next word is found.
void SpellPane::checking()
{
    m_searching = true;
    m_complete = false;
    m_range = qMakePair(-1, -1);
    m_word.clear();
    m_wordLabel->clear();
    m_context->clear();
    m_list->clear();
    m_status->setText(tr("Checking spelling…"));
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

void SpellPane::show(const QPair<int, int> &range)
{
    QTextDocument *story = m_view->document()->story();
    QTextCursor c(story);
    c.setPosition(range.first);
    c.setPosition(range.second, QTextCursor::KeepAnchor);
    m_range = range;
    m_word = c.selectedText();
    m_complete = false;
    m_searching = false;
    m_view->setCursor(c);
    m_status->setText(tr("Not in dictionary:"));
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
    m_list->addItems(spellingSuggestions(m_word, language.isEmpty() ? m_view->spelling()->defaultLanguage() : language).mid(0, 12));
    if (m_list->count() > 0) m_list->setCurrentRow(0);
    for (QPushButton *b : {m_ignore, m_ignoreAll, m_add}) b->setEnabled(true);
    for (QPushButton *b : {m_change, m_changeAll}) b->setEnabled(m_list->count() > 0);
}

void SpellPane::finish()
{
    m_timer.stop();
    m_searching = false;
    m_complete = true;
    m_range = qMakePair(-1, -1);
    m_word.clear();
    m_wordLabel->clear();
    m_context->clear();
    m_list->clear();
    m_status->setText(tr("The spelling check is complete."));
    for (QPushButton *b : {m_change, m_changeAll, m_ignore, m_ignoreAll, m_add}) b->setEnabled(false);
}

// Whether the word shown is still where it was (the text may have been edited while the pane was open); if not, the check goes on from the caret.
bool SpellPane::stillThere()
{
    if (m_range.first >= 0 && m_view->document()) {
        QTextCursor c(m_view->document()->story());
        if (m_range.second <= m_view->document()->story()->characterCount() - 1) {
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
    if (m_complete || m_searching || !m_list->currentItem() || !stillThere()) return;
    QTextDocument *story = m_view->document()->story();
    const QString chosen = m_list->currentItem()->text(), word = m_word;
    const int first = m_range.first, delta = int(chosen.size() - word.size());
    int resume = first + int(chosen.size());
    if (!all) {
        replaceWord(story, m_range.first, m_range.second, chosen);
        if (m_wrapped) m_start += delta;   // (the word is before where the check began)
    } else {
        // Every misspelling of the same word, from the last, in one step: the places the word is in the text, those the check calls misspelled whole. (The rest of the
        // document is not checked for it.)
        QVector<QPair<int, int>> same;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            const QString text = b.text();
            for (int at = int(text.indexOf(word)); at >= 0; at = int(text.indexOf(word, at + 1))) {
                const int pos = b.position() + at;
                const QPair<int, int> w = m_view->spelling()->misspellingAt(pos);
                if (w.first == pos && w.second == pos + int(word.size())) same << w;
            }
        }
        QTextCursor block(story);
        block.beginEditBlock();
        for (int i = int(same.size()) - 1; i >= 0; --i) replaceWord(story, same[i].first, same[i].second, chosen);
        block.endEditBlock();
        // What changed length before the word, and before where the check began, moved them.
        int before = 0, beforeStart = 0;
        for (const auto &w : same) {
            if (w.first < first) ++before;
            if (w.first < m_start) ++beforeStart;
        }
        resume += before * delta;
        m_start += beforeStart * delta;
    }
    find(resume);
}

void SpellPane::ignore(bool all)
{
    if (m_complete || m_searching || !stillThere()) return;
    const int to = m_range.second;
    if (all) {
        spellingIgnore(m_word);
        m_view->spelling()->refresh();
    }
    find(to);
}

void SpellPane::addToDictionary()
{
    if (m_complete || m_searching || !stillThere()) return;
    const int to = m_range.second;
    QString failure;
    if (!spellingAdd(m_word, &failure)) spellingReportFailure(this, failure);
    m_view->spelling()->refresh();
    find(to);
}

} // namespace jp::write
