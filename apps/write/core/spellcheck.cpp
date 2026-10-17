#include "core/spellcheck.h"

#include "app/spelling.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextBlockUserData>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <algorithm>
#include <utility>

namespace jp::write {

namespace {
constexpr int kIdleBudgetMs = 10;   // the time an idle step takes at most
}

// What was found in a paragraph, kept with it (the document drops it with the paragraph).
struct SpellCheck::Answer : QTextBlockUserData {
    QString key;                       // the paragraph as checked: its text, and the languages and "do not check" marks in it
    int revision = 0;                  // of the words accepted and the options, as checked
    QVector<QPair<int, int>> words;    // the misspelled ones, as [start, end) in the paragraph
};

namespace {

// What an answer depends on in a paragraph besides the words accepted: the text, and the language and "do not check" of each stretch that states one.
QString keyOf(const QTextBlock &block, const QString &fallback)
{
    QString key = block.text();
    for (auto it = block.begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        const QString language = cf.stringProperty(tp::Language);
        const bool noProof = cf.boolProperty(tp::NoProof);
        if (language.isEmpty() && !noProof) continue;
        key += QChar(1);
        key += QString::number(it.fragment().position() - block.position());
        key += noProof ? QLatin1Char('!') : QLatin1Char(':');
        key += language;
    }
    key += QChar(2);
    key += fallback;
    return key;
}

} // namespace

SpellCheck::SpellCheck(QTextDocument *story, QObject *parent) : QObject(parent), m_story(story)
{
    m_idle.setSingleShot(true);
    m_idle.setInterval(0);
    connect(&m_idle, &QTimer::timeout, this, &SpellCheck::idle);
}

void SpellCheck::setEnabled(bool on)
{
    if (on == m_enabled) return;
    m_enabled = on;
    if (!on) {
        m_wanted.clear();
        m_idle.stop();
    }
    Q_EMIT changed();
}

void SpellCheck::setDefaultLanguage(const QString &tag)
{
    if (tag == m_language) return;
    m_language = tag;
    Q_EMIT changed();
}

SpellCheck::Pass::Pass(SpellCheck &check, int budgetMs) : m_check(check)
{
    m_check.m_budget = budgetMs;
    m_check.m_doneInPass = 0;
    m_check.m_pass.start();
}

SpellCheck::Pass::~Pass() { m_check.m_budget = -1; }

int SpellCheck::currentRevision() const
{
    const Speller::Rules rules = Speller::rules();
    return spellingRevision() * 4 + (rules.ignoreUpper ? 2 : 0) + (rules.ignoreNumbers ? 1 : 0);
}

bool SpellCheck::outOfTime() const
{
    return m_budget >= 0 && m_doneInPass > 0 && m_pass.nsecsElapsed() >= qint64(m_budget) * 1000000;
}

SpellCheck::Answer *SpellCheck::answerFor(const QTextBlock &block, int revision) const
{
    auto *a = dynamic_cast<Answer *>(block.userData());
    if (!a || a->revision != revision || a->key != keyOf(block, m_language)) return nullptr;
    return a;
}

SpellCheck::Answer *SpellCheck::check(const QTextBlock &block, int revision)
{
    QTextBlock b = block;
    auto *a = dynamic_cast<Answer *>(b.userData());
    if (!a) {
        a = new Answer;
        b.setUserData(a);
    }
    a->key = keyOf(block, m_language);
    a->revision = revision;
    a->words.clear();
    // (A paragraph that stands for an element kept as XML has no text of the person's.)
    if (!block.blockFormat().hasProperty(prop::RawBlock))
        for (const auto &r : misspellingsIn(block, m_language)) a->words << qMakePair(r.first - block.position(), r.second - block.position());
    ++m_checked;
    ++m_doneInPass;
    return a;
}

void SpellCheck::want(int from, int to)
{
    for (auto &w : m_wanted)
        if (from <= w.second && to >= w.first) {
            w.first = std::min(w.first, from);
            w.second = std::max(w.second, to);
            return;
        }
    m_wanted << qMakePair(from, to);
    m_idle.start();
}

QVector<QPair<int, int>> SpellCheck::misspelledIn(int from, int to)
{
    QVector<QPair<int, int>> out;
    if (!m_enabled || !m_story || to <= from) return out;
    from = std::max(0, from);
    if (from >= m_story->characterCount()) return out;
    const int revision = currentRevision();
    bool left = false;
    for (QTextBlock b = m_story->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        const Answer *a = answerFor(b, revision);
        if (!a) {
            if (outOfTime()) {
                left = true;
                continue;
            }
            a = check(b, revision);
        }
        for (const auto &w : a->words) {
            const int s = b.position() + w.first, e = b.position() + w.second;
            if (e > from && s < to) out << qMakePair(s, e);
        }
    }
    if (left) want(from, to);
    return out;
}

void SpellCheck::idle()
{
    if (m_wanted.isEmpty()) return;
    const QVector<QPair<int, int>> wanted = std::exchange(m_wanted, {});
    const qint64 before = m_checked;
    {
        Pass pass(*this, kIdleBudgetMs);
        for (const auto &w : wanted) misspelledIn(w.first, w.second);   // (what it can't do in time is wanted again)
    }
    if (m_checked != before) Q_EMIT changed();
}

QPair<int, int> SpellCheck::misspellingAt(int position)
{
    if (!m_enabled || !m_story || position < 0 || position >= m_story->characterCount()) return qMakePair(-1, -1);
    const QTextBlock b = m_story->findBlock(position);
    const int revision = currentRevision();
    const Answer *a = answerFor(b, revision);
    if (!a) a = check(b, revision);
    for (const auto &w : a->words)
        if (position >= b.position() + w.first && position <= b.position() + w.second) return qMakePair(b.position() + w.first, b.position() + w.second);
    return qMakePair(-1, -1);
}

QPair<int, int> SpellCheck::nextMisspelling(int from)
{
    if (!m_enabled || !m_story || from >= m_story->characterCount()) return qMakePair(-1, -1);
    const int revision = currentRevision();
    for (QTextBlock b = m_story->findBlock(std::max(0, from)); b.isValid(); b = b.next()) {
        const Answer *a = answerFor(b, revision);
        if (!a) a = check(b, revision);
        for (const auto &w : a->words)
            if (b.position() + w.first >= from) return qMakePair(b.position() + w.first, b.position() + w.second);
    }
    return qMakePair(-1, -1);
}

void replaceWord(QTextDocument *story, int from, int to, const QString &text)
{
    QTextCursor look(story);
    look.setPosition(from + 1);
    const QTextCharFormat format = look.charFormat();
    QTextCursor c(story);
    c.setPosition(from);
    c.setPosition(to, QTextCursor::KeepAnchor);
    c.beginEditBlock();
    c.removeSelectedText();
    c.insertText(text, format);
    c.endEditBlock();
}

} // namespace jp::write
