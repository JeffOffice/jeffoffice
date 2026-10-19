#include "core/spellcheck.h"

#include "app/spelling.h"
#include "core/editops.h"
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
constexpr int kPieceWords = 400;    // the most words of a paragraph that are looked at before the time is asked after
}

namespace {

// The paragraph as checked, as a stamp: its length, and two hashes of its text and of the languages and "do not check" marks in it.
struct Stamp {
    qsizetype length = -1;
    quint64 a = 0, b = 0;
    bool operator==(const Stamp &o) const { return length == o.length && a == o.a && b == o.b; }
    bool operator!=(const Stamp &o) const { return !(*this == o); }
};

} // namespace

// What was found in a paragraph, kept with it (the document drops it with the paragraph).
struct SpellCheck::Answer : QTextBlockUserData {
    Stamp stamp;
    int revision = 0;                  // of the words accepted and the options, as checked
    int checkedTo = 0;                 // how far in the paragraph (an offset) the words have been looked at
    bool done = false;                 // the whole paragraph has
    QVector<QPair<int, int>> words;    // the misspelled ones so far, as [start, end) in the paragraph
};

namespace {

// What an answer depends on in a paragraph besides the words accepted: the text, and the language and "do not check" of each stretch that states one.
Stamp stampOf(const QTextBlock &block, const QString &fallback)
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
    Stamp stamp;
    stamp.length = key.size();
    stamp.a = quint64(qHash(QStringView(key), size_t(0x9E3779B97F4A7C15ull)));
    stamp.b = quint64(qHash(QStringView(key), size_t(0xC2B2AE3D27D4EB4Full)));
    return stamp;
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

// The answer kept with the paragraph if it is still its answer (text, languages, and words accepted as they were), done or not.
SpellCheck::Answer *SpellCheck::answerFor(const QTextBlock &block, int revision) const
{
    auto *a = dynamic_cast<Answer *>(block.userData());
    if (!a || a->revision != revision || a->stamp != stampOf(block, m_language)) return nullptr;
    return a;
}

SpellCheck::Answer *SpellCheck::check(const QTextBlock &block, int revision, int upTo, bool ignoreTime)
{
    QTextBlock b = block;
    auto *a = dynamic_cast<Answer *>(b.userData());
    if (!a) {
        a = new Answer;
        b.setUserData(a);
    }
    const Stamp stamp = stampOf(block, m_language);
    if (a->stamp != stamp || a->revision != revision) {
        // Not what was checked (or never): from the start.
        a->stamp = stamp;
        a->revision = revision;
        a->checkedTo = 0;
        a->done = false;
        a->words.clear();
        ++m_checked;
    }
    const int length = block.length() - 1;
    // (A paragraph that stands for an element kept as XML has no text of the person's.)
    // (Nor have the endnotes the pages set after the text, which nothing edits.)
    if (block.blockFormat().hasProperty(prop::RawBlock) || block.blockFormat().hasProperty(prop::EndnoteBlock)) a->done = true;
    while (!a->done) {
        int stopped = 0, looked = 0;
        const auto part = misspellingsIn(block, m_language, a->checkedTo, kPieceWords, &stopped, &looked);
        for (const auto &r : part) a->words << qMakePair(r.first - block.position(), r.second - block.position());
        a->checkedTo = std::max(stopped, a->checkedTo + (looked == 0 ? 1 : 0));
        m_words += looked;
        ++m_doneInPass;
        if (a->checkedTo >= length) a->done = true;
        if (a->done || (upTo >= 0 && a->checkedTo > upTo)) break;
        if (!ignoreTime && outOfTime()) break;
    }
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
        Answer *a = answerFor(b, revision);
        if (!a || !a->done) {
            if (outOfTime()) {
                left = true;
                if (!a) continue;   // (a piece of it is there, as far as it went)
            } else {
                a = check(b, revision);
                if (!a->done) left = true;
            }
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
    const qint64 before = m_checked, wordsBefore = m_words;
    {
        Pass pass(*this, kIdleBudgetMs);
        for (const auto &w : wanted) misspelledIn(w.first, w.second);   // (what it can't do in time is wanted again)
    }
    if (m_checked != before || m_words != wordsBefore) Q_EMIT changed();
}

QPair<int, int> SpellCheck::misspellingAt(int position)
{
    if (!m_story || position < 0 || position >= m_story->characterCount()) return qMakePair(-1, -1);
    const QTextBlock b = m_story->findBlock(position);
    const int revision = currentRevision();
    Answer *a = answerFor(b, revision);
    // As far as the position, not the whole of a long paragraph.
    if (!a || (!a->done && a->checkedTo <= position - b.position())) a = check(b, revision, position - b.position(), true);
    for (const auto &w : a->words)
        if (position >= b.position() + w.first && position <= b.position() + w.second) return qMakePair(b.position() + w.first, b.position() + w.second);
    return qMakePair(-1, -1);
}

SpellCheck::Step SpellCheck::search(int from, int budgetMs, QPair<int, int> *found, int *next)
{
    *found = qMakePair(-1, -1);
    if (next) *next = from;
    if (!m_story || from >= m_story->characterCount()) return Step::None;
    Pass pass(*this, budgetMs);
    const int revision = currentRevision();
    for (QTextBlock b = m_story->findBlock(std::max(0, from)); b.isValid(); b = b.next()) {
        Answer *a = answerFor(b, revision);
        if (!a || !a->done) {
            if (outOfTime()) {
                if (next) *next = std::max(from, b.position());
                return Step::More;
            }
            a = check(b, revision);
        }
        // (The words of a paragraph that is only partly checked are its first: one of them is the first in the story.)
        for (const auto &w : a->words)
            if (b.position() + w.first >= from) {
                *found = qMakePair(b.position() + w.first, b.position() + w.second);
                return Step::Found;
            }
        if (!a->done) {
            if (next) *next = std::max(from, b.position());
            return Step::More;
        }
    }
    return Step::None;
}

QPair<int, int> SpellCheck::nextMisspelling(int from)
{
    QPair<int, int> found;
    int next = from;
    while (search(from, -1, &found, &next) == Step::More) from = next;
    return found;
}

qint64 SpellCheck::keptBytes() const
{
    qint64 bytes = 0;
    if (!m_story) return bytes;
    for (QTextBlock b = m_story->begin(); b.isValid(); b = b.next())
        if (auto *a = dynamic_cast<Answer *>(b.userData())) bytes += qint64(sizeof(Answer)) + qint64(a->words.capacity()) * qint64(sizeof(QPair<int, int>));
    return bytes;
}

void replaceWord(QTextDocument *story, int from, int to, const QString &text)
{
    if (from >= editableEnd(story) || to > editableEnd(story)) return;   // (a word of the endnotes the pages set after the text is not changed)
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
