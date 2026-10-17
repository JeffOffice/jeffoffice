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
#include <QPointer>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>
#include <algorithm>

namespace jp::write {

namespace {

// The language of the word at a position: the text's own, else the document's.
QString languageAt(PageView *view, int position)
{
    QTextCursor c(view->document()->story());
    c.setPosition(position + 1);
    const QString language = c.charFormat().stringProperty(tp::Language);
    return language.isEmpty() ? view->spelling()->defaultLanguage() : language;
}

} // namespace

bool addSpellingSuggestions(QMenu &menu, PageView *view)
{
    if (!view || !view->document() || !view->spelling() || !view->spelling()->enabled()) return false;
    const QPair<int, int> word = view->spelling()->misspellingAt(view->cursor().position());
    if (word.first < 0) return false;
    QTextCursor w(view->document()->story());
    w.setPosition(word.first);
    w.setPosition(word.second, QTextCursor::KeepAnchor);
    const QString text = w.selectedText();
    const QStringList suggestions = spellingSuggestions(text, languageAt(view, word.first)).mid(0, 6);
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
        QObject::connect(a, &QAction::triggered, &menu, [where, word, s] {
            if (!where) return;
            replaceWord(where->document()->story(), word.first, word.second, s);
            QTextCursor c = where->cursor();
            c.setPosition(word.first + int(s.size()));
            where->setCursor(c);
        });
    }
    if (suggestions.isEmpty()) menu.addAction(QCoreApplication::translate("Proofing", "(No spelling suggestions)"))->setEnabled(false);
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Ignore All")), &QAction::triggered, &menu, [where, text] {
        spellingIgnore(text);
        if (where) where->spelling()->refresh();
    });
    QObject::connect(menu.addAction(QCoreApplication::translate("Proofing", "Add to Dictionary")), &QAction::triggered, &menu, [where, text] {
        spellingAdd(text);
        if (where) where->spelling()->refresh();
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
    m_wordLabel = new QLabel(this);
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
    finish();
    m_complete = false;
    m_status->clear();
}

void SpellPane::start()
{
    if (!m_view->document() || !m_view->spelling()) {
        finish();
        return;
    }
    m_start = m_view->cursor().selectionStart();
    m_wrapped = false;
    m_complete = false;
    find(m_start);
}

void SpellPane::find(int from)
{
    SpellCheck *check = m_view->spelling();
    QPair<int, int> w = check->nextMisspelling(from);
    if (w.first >= 0 && m_wrapped && w.first >= m_start) w = qMakePair(-1, -1);   // (back to where it began)
    if (w.first < 0 && !m_wrapped && m_start > 0) {
        m_wrapped = true;
        w = check->nextMisspelling(0);
        if (w.first >= m_start) w = qMakePair(-1, -1);
    }
    if (w.first < 0) finish();
    else show(w);
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
    if (m_complete || !m_list->currentItem() || !stillThere()) return;
    QTextDocument *story = m_view->document()->story();
    const QString chosen = m_list->currentItem()->text(), word = m_word;
    const int first = m_range.first;
    if (!all) {
        replaceWord(story, m_range.first, m_range.second, chosen);
    } else {
        // Every misspelling of the same word, from the last, in one step.
        QVector<QPair<int, int>> same;
        QTextCursor c(story);
        for (QPair<int, int> w = m_view->spelling()->nextMisspelling(0); w.first >= 0; w = m_view->spelling()->nextMisspelling(w.second)) {
            c.setPosition(w.first);
            c.setPosition(w.second, QTextCursor::KeepAnchor);
            if (c.selectedText() == word) same << w;
        }
        QTextCursor block(story);
        block.beginEditBlock();
        for (int i = int(same.size()) - 1; i >= 0; --i) replaceWord(story, same[i].first, same[i].second, chosen);
        block.endEditBlock();
    }
    find(first + int(chosen.size()));
}

void SpellPane::ignore(bool all)
{
    if (m_complete || !stillThere()) return;
    const int to = m_range.second;
    if (all) {
        spellingIgnore(m_word);
        m_view->spelling()->refresh();
    }
    find(to);
}

void SpellPane::addToDictionary()
{
    if (m_complete || !stillThere()) return;
    const int to = m_range.second;
    spellingAdd(m_word);
    m_view->spelling()->refresh();
    find(to);
}

} // namespace jp::write
