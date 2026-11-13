#include "app/grammarcheck.h"

#include "app/settings.h"
#include "text/textprops.h"

#include <QElapsedTimer>
#include <QCoreApplication>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextFragment>
#include <QThreadPool>
#include <QSet>
#include <algorithm>
#include <cstdlib>
#include <atomic>

namespace jp {

namespace {
// What is kept of answers, in characters of the paragraphs they are for (a book of 600,000 characters is kept whole, several times over, and no more).
constexpr qint64 kKeptChars = 4000000;
// The paragraphs of one story whose look is remembered (a book has a few thousand; past the limit the looks are made afresh, not kept).
constexpr int kMaxLooks = 50000;
// The most jobs waiting in a pool at once, over all the checks that share it (the rest wait in their checks, by paragraph, and only the latest text of each).
constexpr int kMaxQueued = 256;
// What a document may spend on grammar in all while it is open: a book of 600,000 characters is 0.6 million, and checking it again after every edit of every paragraph of it
// for a working day would come to a few million more; 50 million characters (about 40 seconds of the checker) is far above that, and a hostile document cannot get past it.
constexpr qint64 kBudgetChars = 50000000;
constexpr int kBudgetMs = 600000;
// How far each way a sentence is looked for when an issue is ignored in its place.
constexpr int kSentenceReach = 1000;
}

// What the thread and the window share: the budget of the whole document, and whether the checker is gone. The lock is held for a moment at a time, never while checking.
struct GrammarCheck::Shared {
    QMutex lock;
    GrammarBudget budget{kBudgetChars, kBudgetMs};
    std::atomic<bool> gone{false};
    std::atomic<int> queued{0};                 // jobs sent to the pool and not yet answered, over all the checks that share it
    QHash<quint64, GrammarCheck *> registry;    // the checks, by number: the window's thread only (an answer finds its check here, or finds it gone)
    QSet<QString> ignoredAll;                   // "Ignore All" for the checks that share this, those made later too: the window's thread only
    std::atomic<quint64> lastId{0};
    bool exhausted()
    {
        QMutexLocker l(&lock);
        return budget.exhausted();
    }
    void spend(qint64 chars, qint64 ms)
    {
        QMutexLocker l(&lock);
        budget.spend(chars, ms);
    }
};

GrammarCheck::GrammarCheck(QTextDocument *story, QObject *parent) : QObject(parent), m_story(story), m_shared(std::make_shared<Shared>()), m_ownPool(new QThreadPool)
{
    m_pool = m_ownPool.get();
    m_pool->setMaxThreadCount(1);
    m_id = ++m_shared->lastId;
    m_shared->registry.insert(m_id, this);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &GrammarCheck::send);
    connect(&GrammarSettingNotifier::get(), &GrammarSettingNotifier::kindsChanged, this, [this] {   // (another choice of kinds: what was found is kept under its own key, and the paragraphs are looked at again)
        m_looks.clear();
        m_wanted.clear();
        bump();
    });
}

std::shared_ptr<GrammarCheck::Shared> GrammarCheck::newShared() { return std::make_shared<Shared>(); }

GrammarCheck::GrammarCheck(QTextDocument *story, const std::shared_ptr<Shared> &shared, QThreadPool *pool, QObject *parent) : QObject(parent), m_story(story), m_shared(shared), m_pool(pool)
{
    m_id = ++m_shared->lastId;
    m_shared->registry.insert(m_id, this);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &GrammarCheck::send);
    connect(&GrammarSettingNotifier::get(), &GrammarSettingNotifier::kindsChanged, this, [this] {   // (another choice of kinds: what was found is kept under its own key, and the paragraphs are looked at again)
        m_looks.clear();
        m_wanted.clear();
        bump();
    });
}

void GrammarCheck::shutDown(const std::shared_ptr<Shared> &shared) { shared->gone = true; }

GrammarCheck::~GrammarCheck()
{
    m_shared->registry.remove(m_id);   // (an answer that comes later finds no check, and is let go)
    if (!m_ownPool) return;   // (the jobs of a shared pool hold what they need)
    m_shared->gone = true;   // (jobs that have not begun do nothing; the one under way finishes, which the service bounds)
    m_pool->clear();
    m_pool->waitForDone();
}

void GrammarCheck::bump()
{
    ++m_version;
    Q_EMIT changed();
}

void GrammarCheck::setStory(QTextDocument *story)
{
    if (story == m_story) return;
    m_story = story;
    m_looks.clear();
    m_wanted.clear();
    bump();
}

void GrammarCheck::setEnabled(bool on)
{
    if (on == m_enabled) return;
    m_enabled = on;
    if (!on) {
        m_wanted.clear();
        m_timer.stop();
    }
    bump();
}

void GrammarCheck::setDefaultLanguage(const QString &tag)
{
    if (tag == m_language) return;
    m_language = tag;
    m_looks.clear();
    bump();
}

void GrammarCheck::setBudget(qint64 chars, int ms)
{
    QMutexLocker l(&m_shared->lock);
    m_shared->budget = GrammarBudget(chars, ms);
    m_exhausted = false;
}

// The paragraph as the checker takes it. Only English is checked: the text of a stretch in another language, or marked "do not check", is blanked out (with a character
// that is no word and no space, so no rule joins the words on its two sides), and a paragraph with no English in it is not sent at all. The look is kept with the paragraph's
// revision, so that a repaint that changes nothing asks no question of the text.
const GrammarCheck::Look &GrammarCheck::lookAt(const QTextBlock &block)
{
    if (m_looks.size() > kMaxLooks) m_looks.clear();   // (only a story of more paragraphs than that is ever here)
    BlockLook &cached = m_looks[block.blockNumber()];
    // (the revision, the place, and the length say it is the paragraph the look was made of; the text is compared too, since two paragraphs that were never edited can agree on those)
    size_t marks = 0;
    for (auto it = block.begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        marks = qHashMulti(marks, it.fragment().length(), cf.stringProperty(tp::Language), cf.boolProperty(tp::NoProof));
    }
    if (cached.position == block.position() && cached.revision == block.revision() && cached.length == block.length() && cached.marks == marks && cached.look.text == block.text()) return cached.look;
    ++m_looksMade;
    Look look;
    look.text = block.text();
    look.masked = look.text;
    QString english;
    for (auto it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        const QTextCharFormat cf = f.charFormat();
        QString language = cf.stringProperty(tp::Language);
        if (language.isEmpty()) language = m_language;
        if (grammarSupports(language) && !cf.boolProperty(tp::NoProof)) {
            if (english.isEmpty()) english = language.isEmpty() ? QStringLiteral("en") : language;
            continue;
        }
        const int from = f.position() - block.position(), to = std::min(from + f.length(), int(look.masked.size()));
        for (int p = std::max(0, from); p < to; ++p) look.masked[p] = QChar(0xFFFC);
    }
    look.language = english.isEmpty() ? QStringLiteral("zz") : english;   // ("zz": none, as far as the service knows)
    look.kinds = grammarKinds();
    look.key = QString::number(look.kinds) + QChar(2) + look.language + QChar(1) + look.masked;
    cached.position = block.position();
    cached.revision = block.revision();
    cached.length = block.length();
    cached.marks = marks;
    cached.look = look;
    return cached.look;
}

// What an ignored issue is known by: the message and the words, and (here) the sentence they are in and where in it, so that it is this place and not every like it.
QString GrammarCheck::ignoreKey(const Look &look, int start, int end, bool here) const
{
    QString key = look.text.mid(start, end - start);
    if (!here) return key;
    const QString &t = look.text;
    int from = start, to = end;
    for (int n = 0; from > 0 && n < kSentenceReach; ++n, --from)
        if ((t.at(from - 1) == QLatin1Char('.') || t.at(from - 1) == QLatin1Char('!') || t.at(from - 1) == QLatin1Char('?')) && from < t.size() && t.at(from).isSpace()) break;
    for (int n = 0; to < t.size() && n < kSentenceReach; ++n, ++to)
        if (t.at(to) == QLatin1Char('.') || t.at(to) == QLatin1Char('!') || t.at(to) == QLatin1Char('?')) {
            ++to;
            break;
        }
    while (from < start && t.at(from).isSpace()) ++from;   // (the sentence begins at its first letter)
    return key + QChar(1) + t.mid(from, to - from) + QChar(1) + QString::number(start - from);
}

QVector<GrammarCheck::Issue> GrammarCheck::issuesOf(const QTextBlock &block, const Look &look, const Entry &entry) const
{
    QVector<Issue> out;
    for (const GrammarIssue &g : entry.issues) {
        if (g.end > look.text.size()) continue;
        // What was blanked out (another language, "do not check") is not ours to change: an issue on it is let go, and so is a suggestion that has its blanks in it.
        bool touches = false;
        for (int p = g.start; p < g.end && !touches; ++p) touches = look.masked.at(p) != look.text.at(p);
        if (touches) continue;
        if (!m_shared->ignoredAll.isEmpty() && m_shared->ignoredAll.contains(g.message + QChar(1) + ignoreKey(look, g.start, g.end, false))) continue;
        if (!m_ignoredHere.isEmpty() && m_ignoredHere.contains(g.message + QChar(1) + ignoreKey(look, g.start, g.end, true))) continue;
        Issue is;
        is.start = block.position() + g.start;
        is.end = block.position() + g.end;
        is.kind = g.kind;
        is.message = g.message;
        const bool blankInside = look.text.mid(g.start, g.end - g.start).contains(QChar(0xFFFC));
        for (const QString &sg : g.suggestions)
            if (blankInside || !sg.contains(QChar(0xFFFC))) is.suggestions << sg;
        if (!g.suggestions.isEmpty() && is.suggestions.isEmpty()) continue;
        out << is;
    }
    return out;
}

QVector<GrammarCheck::Issue> GrammarCheck::issuesIn(int from, int to)
{
    QVector<Issue> out;
    if (!m_enabled || !m_story) return out;
    for (QTextBlock b = m_story->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (b.length() <= 1) continue;
        const Look &look = lookAt(b);
        const auto it = m_kept.constFind(look.key);
        if (it != m_kept.constEnd()) {
            for (const Issue &is : issuesOf(b, look, *it))
                if (is.end > from && is.start < to) out << is;
        } else if (!m_exhausted && grammarAvailable() && grammarSupports(look.language)) {
            want(b.blockNumber(), look);
        }
    }
    return out;
}

GrammarCheck::Issue GrammarCheck::issueAt(int position)
{
    if (!m_enabled || !m_story) return Issue();
    const QTextBlock b = m_story->findBlock(position);
    if (!b.isValid()) return Issue();
    const Look &look = lookAt(b);
    const auto it = m_kept.constFind(look.key);
    if (it == m_kept.constEnd()) return Issue();
    for (const Issue &is : issuesOf(b, look, *it))
        if (position >= is.start && position <= is.end) return is;
    return Issue();
}

GrammarCheck::Step GrammarCheck::search(int from, int budgetMs, Issue *found, int *next)
{
    if (!m_story || !grammarAvailable()) return Step::None;
    QElapsedTimer timer;
    timer.start();
    bool first = true;
    for (QTextBlock b = m_story->findBlock(from); b.isValid(); b = b.next()) {
        if (b.length() <= 1) continue;
        if (!first && budgetMs >= 0 && timer.elapsed() >= budgetMs) {
            if (next) *next = b.position();
            return Step::More;
        }
        first = false;
        const Look look = lookAt(b);   // (a copy: the table may change under the call to the checker below)
        auto it = m_kept.constFind(look.key);
        if (it == m_kept.constEnd()) {
            if (!grammarSupports(look.language)) continue;
            if (m_shared->exhausted()) {
                stopForBudget();
                return Step::None;
            }
            // (checked here, on the window's thread, without the lock: the thread of the checker is never waited for)
            QElapsedTimer t;
            t.start();
            Entry e;
            e.issues = grammarCheck(look.masked, look.language, nullptr, nullptr, look.kinds);
            m_shared->spend(look.masked.size(), t.elapsed());
            ++m_checked;
            keep(look.key, e);
            it = m_kept.constFind(look.key);
        }
        for (const Issue &is : issuesOf(b, look, *it))
            if (is.start >= from) {
                if (found) *found = is;
                return Step::Found;
            }
    }
    return Step::None;
}

void GrammarCheck::ignoreHere(const Issue &issue)
{
    if (!m_story || !issue.valid()) return;
    const QTextBlock b = m_story->findBlock(issue.start);
    const Look &look = lookAt(b);
    m_ignoredHere.insert(issue.message + QChar(1) + ignoreKey(look, issue.start - b.position(), issue.end - b.position(), true));
    bump();
}

void GrammarCheck::ignoreAll(const Issue &issue)
{
    if (!m_story || !issue.valid()) return;
    const QTextBlock b = m_story->findBlock(issue.start);
    const Look &look = lookAt(b);
    m_shared->ignoredAll.insert(issue.message + QChar(1) + ignoreKey(look, issue.start - b.position(), issue.end - b.position(), false));
    bump();
}

void GrammarCheck::ignoreAllOf(const QString &message, const QString &words)
{
    m_shared->ignoredAll.insert(message + QChar(1) + words);
    bump();
}

// A paragraph wants checking: only its latest text, though it changes with every key. The wait begins again with each change.
void GrammarCheck::want(int slot, const Look &look)
{
    if (m_sent.contains(look.key)) return;
    const auto it = m_wanted.constFind(slot);
    if (it != m_wanted.constEnd() && it->key == look.key) return;
    m_wanted.insert(slot, look);
    m_timer.start(m_delay);
}

// Sends what is wanted to the thread: the paragraph being edited first, then the nearest, and no more than the pool has room for (the rest wait here, by paragraph, and a
// newer text of a paragraph takes the place of its old one); the answers come back to the window's thread through the shared registry, never through a pointer to this check.
void GrammarCheck::send()
{
    QList<int> slots = m_wanted.keys();
    const int near = m_priority;
    std::sort(slots.begin(), slots.end(), [near](int a, int b) {
        if (near >= 0 && std::abs(a - near) != std::abs(b - near)) return std::abs(a - near) < std::abs(b - near);
        return a < b;   // (in the order of the text: what the budget has left goes to the first paragraphs)
    });
    for (int slot : slots) {
        if (m_exhausted) return;
        if (m_shared->queued >= kMaxQueued) break;
        const Look look = m_wanted.take(slot);
        if (m_sent.contains(look.key) || m_kept.contains(look.key)) continue;
        m_sent.insert(look.key);
        ++m_inFlight;
        ++m_checked;
        ++m_shared->queued;
        const std::shared_ptr<Shared> shared = m_shared;
        const quint64 id = m_id;
        const QString key = look.key, text = look.masked, language = look.language;
        const quint32 kinds = look.kinds;
        m_pool->start(QRunnable::create([shared, id, key, text, language, kinds] {
            if (shared->gone) return;
            QVector<GrammarIssue> issues;
            const bool exhausted = shared->exhausted();
            if (!exhausted) {
                QElapsedTimer t;
                t.start();
                issues = grammarCheck(text, language, nullptr, nullptr, kinds);
                shared->spend(text.size(), t.elapsed());
            }
            if (shared->gone || !QCoreApplication::instance()) return;
            QMetaObject::invokeMethod(QCoreApplication::instance(), [shared, id, key, issues, exhausted] {
                --shared->queued;
                if (GrammarCheck *check = shared->registry.value(id)) check->done(key, issues, exhausted);
            }, Qt::QueuedConnection);
        }));
    }
    if (!m_wanted.isEmpty()) m_timer.start(std::max(m_delay, 20));   // (the pool is full: again when it has had time)
}

void GrammarCheck::done(const QString &key, const QVector<GrammarIssue> &issues, bool exhausted)
{
    --m_inFlight;
    m_sent.remove(key);
    if (exhausted) {   // (no answer: what the checker would have found is not "nothing", and is not kept as that)
        stopForBudget();
        return;
    }
    Entry e;
    e.issues = issues;
    keep(key, e);
    if (m_enabled) bump();
}

void GrammarCheck::stopForBudget()
{
    if (m_exhausted) return;
    m_exhausted = true;
    m_wanted.clear();
    m_timer.stop();
    Q_EMIT budgetExhausted();
}

void GrammarCheck::keep(const QString &key, const Entry &entry)
{
    if (m_keptChars > kKeptChars) {
        m_kept.clear();
        m_keptChars = 0;
    }
    if (!m_kept.contains(key)) m_keptChars += key.size();
    m_kept.insert(key, entry);
}

} // namespace jp
