#include "core/spellcheck.h"

#include "app/spelling.h"
#include "core/contentcontrolops.h"
#include "core/dropcapops.h"
#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QSet>
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
constexpr int kWalkParagraphs = 256;   // the most paragraphs the counting pass goes by (checked or not) in one slice
constexpr int kMaxWanted = 32;      // the most ranges that wait for idle time; past it they are one
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

// What is kept for a story, however many checks look at it: the misspelled words in all the answers, and the answers of the paragraphs an edit changed that are not yet checked again.
// (It is a child of the story, so it is as long-lived.)
struct SpellCheck::Tally : QObject {
    explicit Tally(QTextDocument *story);
    static Tally *of(QTextDocument *story);
    bool active() const;
    void changed();
    void mark(int position, int added);
    int total = 0;
    QSet<Answer *> marked;
    QList<SpellCheck *> checks;
};

// What was found in a paragraph, kept with it (the document drops it with the paragraph).
struct SpellCheck::Answer : QTextBlockUserData {
    Stamp stamp;
    int revision = 0;                  // of the words accepted and the options, as checked
    int checkedTo = 0;                 // how far in the paragraph (an offset) the words have been looked at
    bool done = false;                 // the whole paragraph has
    QVector<QPair<int, int>> words;    // the misspelled ones so far, as [start, end) in the paragraph
    int counted = 0;                   // how many of them the story's total holds
    QPointer<Tally> tally;
    QTextBlock block;                  // its paragraph (the answer goes with the paragraph, so it is valid as long as the answer is)
    ~Answer() override
    {
        if (tally) {
            tally->total -= counted;
            tally->marked.remove(this);
        }
    }
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

SpellCheck::Tally::Tally(QTextDocument *story) : QObject(story)
{
    // (An edit: the paragraphs it changed are marked, when the count is being kept, so the total is not left as it was.)
    connect(story, &QTextDocument::contentsChange, this, [this](int position, int, int added) { mark(position, added); });
}

SpellCheck::Tally *SpellCheck::Tally::of(QTextDocument *story)
{
    for (QObject *child : story->children())
        if (auto *t = dynamic_cast<Tally *>(child)) return t;
    return new Tally(story);
}

bool SpellCheck::Tally::active() const
{
    for (const SpellCheck *c : checks)
        if (c->m_counting && c->m_enabled) return true;
    return false;
}

void SpellCheck::Tally::changed()
{
    for (SpellCheck *c : checks)
        if (c->m_counting) Q_EMIT c->countChanged();
}

// The paragraphs from the one at `position` to the one at the end of what was put in (the paragraph that starts right at the end of it too: a paragraph split, or one pasted
// that ends with a break, has its second part there) are marked: each has an answer that says it is not checked, until it is.
void SpellCheck::Tally::mark(int position, int added)
{
    auto *story = qobject_cast<QTextDocument *>(parent());
    if (!story || !active()) return;
    const int end = position + std::max(added, 1);
    for (QTextBlock b = story->findBlock(std::max(0, position)); b.isValid() && b.position() <= end; b = b.next()) {
        auto *a = dynamic_cast<Answer *>(b.userData());
        if (!a) {
            a = new Answer;
            a->tally = this;
            QTextBlock hold = b;
            hold.setUserData(a);
        }
        a->block = b;
        marked.insert(a);
    }
    for (SpellCheck *c : checks)
        if (c->m_counting && c->m_enabled) c->m_idle.start();
    changed();
}

SpellCheck::SpellCheck(QTextDocument *story, QObject *parent) : QObject(parent), m_story(story)
{
    m_idle.setSingleShot(true);
    m_idle.setInterval(0);
    connect(&m_idle, &QTimer::timeout, this, &SpellCheck::idle);
    // (Words another app added or excluded: the pages are drawn again, and the paragraphs are checked as the words now stand.)
    connect(Speller::get().watch(), &SpellingWatch::changed, this, &SpellCheck::refresh);
    if (story) {
        m_tally = Tally::of(story);
        m_tally->checks << this;
    }
}

SpellCheck::~SpellCheck()
{
    if (m_tally) m_tally->checks.removeAll(this);
}

int SpellCheck::knownMisspellings() const { return m_tally ? m_tally->total : 0; }

bool SpellCheck::pending() const { return !m_wanted.isEmpty() || (m_counting && m_enabled && !m_paused && m_tally && !m_tally->marked.isEmpty()); }

// The counting pass begins again, from the top of the story.
void SpellCheck::restartSweep()
{
    m_swept = false;
    if (!m_sweepCursor.isNull()) m_sweepCursor.setPosition(0);
    if (m_counting && m_enabled) {
        m_idle.start();
        Q_EMIT countChanged();
    }
}

void SpellCheck::refresh()
{
    restartSweep();
    Q_EMIT changed();
}

void SpellCheck::setPaused(bool on)
{
    if (on == m_paused) return;
    m_paused = on;
    if (!on && m_counting) m_idle.start();
}

void SpellCheck::setCounting(bool on)
{
    if (on == m_counting) return;
    m_counting = on;
    m_swept = false;
    if (!m_sweepCursor.isNull()) m_sweepCursor.setPosition(0);
    if (on) m_idle.start();
}

void SpellCheck::setEnabled(bool on)
{
    if (on == m_enabled) return;
    m_enabled = on;
    if (!on) {
        m_wanted.clear();
        m_idle.stop();
    } else if (m_counting) {
        restartSweep();
    }
    Q_EMIT changed();
    Q_EMIT countChanged();
}

void SpellCheck::setDefaultLanguage(const QString &tag)
{
    if (tag == m_language) return;
    m_language = tag;
    if (m_counting) restartSweep();
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
        a->tally = m_tally;
        b.setUserData(a);
    }
    a->block = block;
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
    // The paragraph after a drop cap goes on with the drop cap's word: that word is read whole, the big letters and the rest of it together.
    if (a->checkedTo > 0 && isDropCapFrame(block.previous()) && !block.previous().text().isEmpty()) {
        static const QRegularExpression letters(QStringLiteral("^[\\p{L}\\p{M}]+"));
        const QRegularExpressionMatch lead = letters.match(block.text());
        if (lead.hasMatch() && lead.capturedLength() <= a->checkedTo) {
            for (int i = int(a->words.size()) - 1; i >= 0; --i)
                if (a->words[i].first == 0) a->words.removeAt(i);
            QTextDocument whole;
            whole.setPlainText(block.previous().text() + lead.captured());
            if (!misspellingsIn(whole.begin(), m_language).isEmpty()) a->words.prepend(qMakePair(0, int(lead.capturedLength())));
        }
    }
    if (a->tally && a->counted != int(a->words.size())) {
        a->tally->total += int(a->words.size()) - a->counted;
        a->counted = int(a->words.size());
        a->tally->changed();
    }
    if (a->done && a->tally) a->tally->marked.remove(a);
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
    if (m_wanted.size() >= kMaxWanted) {   // (so many apart: they are one range, which holds them all)
        for (const auto &w : m_wanted) {
            from = std::min(from, w.first);
            to = std::max(to, w.second);
        }
        m_wanted.clear();
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
    const bool counting = m_counting && m_enabled && m_story && !m_paused;
    const bool sweeping = counting && !m_swept;
    const bool marked = counting && m_tally && !m_tally->marked.isEmpty();
    if (m_wanted.isEmpty() && !sweeping && !marked) return;
    const QVector<QPair<int, int>> wanted = std::exchange(m_wanted, {});
    const qint64 before = m_checked, wordsBefore = m_words;
    {
        Pass pass(*this, kIdleBudgetMs);
        for (const auto &w : wanted) misspelledIn(w.first, w.second);   // (what it can't do in time is wanted again)
        if (counting) {
            checkMarked(currentRevision());
            if (sweeping) sweepStep();
        }
    }
    if (counting && (!m_swept || (m_tally && !m_tally->marked.isEmpty()))) m_idle.start();
    if (m_checked != before || m_words != wordsBefore) Q_EMIT changed();
}

// The paragraphs an edit changed, checked again for as long as the pass has time.
void SpellCheck::checkMarked(int revision)
{
    if (!m_tally) return;
    while (!m_tally->marked.isEmpty()) {
        if (outOfTime()) return;
        Answer *a = *m_tally->marked.constBegin();
        const QTextBlock b = a->block;
        if (!b.isValid() || b.userData() != a) {   // (not its paragraph's: the counting pass looks at them all again)
            m_tally->marked.remove(a);
            m_swept = false;
            continue;
        }
        check(b, revision);
        if (!a->done) return;
    }
}

// The counting pass: the paragraphs from where it left off that are not checked, for as long as the pass has time, and not going by more than a number of paragraphs (the ones that
// are checked already take no time, but a long document of them is not gone through in one slice). A change of the words accepted begins it again, and where it left off moves
// with the edits.
void SpellCheck::sweepStep()
{
    const int revision = currentRevision();
    if (m_sweepCursor.isNull()) {
        m_sweepCursor = QTextCursor(m_story);
        m_sweepCursor.setKeepPositionOnInsert(true);
    }
    if (revision != m_sweepRevision) {
        m_sweepRevision = revision;
        m_sweepCursor.setPosition(0);
    }
    QTextBlock b = m_story->findBlock(m_sweepCursor.position());
    int walked = 0;
    for (; b.isValid() && walked < kWalkParagraphs; b = b.next()) {
        ++walked;
        ++m_walked;
        Answer *a = answerFor(b, revision);
        if (!a || !a->done) {
            if (outOfTime()) break;
            a = check(b, revision);
            if (!a->done) break;
        }
    }
    if (b.isValid()) {
        m_sweepCursor.setPosition(b.position());
        return;
    }
    m_swept = true;
    m_sweepCursor.setPosition(0);
    Q_EMIT countChanged();
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

bool canReplaceWord(const QTextDocument *story, int from, int to)
{
    return from < editableEnd(story) && to <= editableEnd(story) && !touchesEndnotes(story, from, to) && contentControlsAllowIn(const_cast<QTextDocument *>(story), from, to, true);   // (a word of the endnotes the pages set is not changed)
}

void replaceWord(QTextDocument *story, int from, int to, const QString &text)
{
    if (!canReplaceWord(story, from, to)) return;
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
