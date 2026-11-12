#include "core/grammarcheck.h"

#include "text/textprops.h"

#include <QElapsedTimer>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextFragment>
#include <QThreadPool>
#include <algorithm>
#include <atomic>

namespace jp::write {

namespace {
// What is kept of answers, in characters of the paragraphs they are for (a book of 600,000 characters is kept whole, several times over, and no more).
constexpr qint64 kKeptChars = 4000000;
// The paragraphs whose look is remembered (a book has a few thousand).
constexpr int kMaxLooks = 6000;
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

GrammarCheck::GrammarCheck(QTextDocument *story, QObject *parent) : QObject(parent), m_story(story), m_shared(std::make_shared<Shared>()), m_pool(new QThreadPool)
{
    m_pool->setMaxThreadCount(1);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &GrammarCheck::send);
}

GrammarCheck::~GrammarCheck()
{
    m_shared->gone = true;   // (jobs that have not begun do nothing; the one under way finishes, which the service bounds)
    m_pool->clear();
    m_pool->waitForDone();
}

void GrammarCheck::setEnabled(bool on)
{
    if (on == m_enabled) return;
    m_enabled = on;
    if (!on) {
        m_wanted.clear();
        m_timer.stop();
    }
    Q_EMIT changed();
}

void GrammarCheck::setDefaultLanguage(const QString &tag)
{
    if (tag == m_language) return;
    m_language = tag;
    m_looks.clear();
    Q_EMIT changed();
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
    if (m_looks.size() > kMaxLooks) m_looks.clear();
    BlockLook &cached = m_looks[block.blockNumber()];
    // (the revision, the place, and the length say it is the paragraph the look was made of; the text is compared too, since two paragraphs that were never edited can agree on those)
    if (cached.position == block.position() && cached.revision == block.revision() && cached.length == block.length() && cached.look.text == block.text()) return cached.look;
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
    look.key = look.language + QChar(1) + look.masked;
    cached.position = block.position();
    cached.revision = block.revision();
    cached.length = block.length();
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
        if (!m_ignoredAll.isEmpty() && m_ignoredAll.contains(g.message + QChar(1) + ignoreKey(look, g.start, g.end, false))) continue;
        if (!m_ignoredHere.isEmpty() && m_ignoredHere.contains(g.message + QChar(1) + ignoreKey(look, g.start, g.end, true))) continue;
        Issue is;
        is.start = block.position() + g.start;
        is.end = block.position() + g.end;
        is.kind = g.kind;
        is.message = g.message;
        is.suggestions = g.suggestions;
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
            e.issues = grammarCheck(look.masked, look.language);
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
    Q_EMIT changed();
}

void GrammarCheck::ignoreAll(const Issue &issue)
{
    if (!m_story || !issue.valid()) return;
    const QTextBlock b = m_story->findBlock(issue.start);
    const Look &look = lookAt(b);
    m_ignoredAll.insert(issue.message + QChar(1) + ignoreKey(look, issue.start - b.position(), issue.end - b.position(), false));
    Q_EMIT changed();
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

void GrammarCheck::send()
{
    const QHash<int, Look> wanted = m_wanted;
    m_wanted.clear();
    QList<int> slots = wanted.keys();
    std::sort(slots.begin(), slots.end());   // (in the order of the text: what the budget has left goes to the first paragraphs)
    for (int slot : slots) {
        const Look &look = wanted[slot];
        if (m_exhausted) return;
        if (m_sent.contains(look.key) || m_kept.contains(look.key)) continue;
        m_sent.insert(look.key);
        ++m_inFlight;
        ++m_checked;
        const std::shared_ptr<Shared> shared = m_shared;
        const QString key = look.key, text = look.masked, language = look.language;
        QPointer<GrammarCheck> self(this);
        m_pool->start(QRunnable::create([shared, self, key, text, language] {
            if (shared->gone) return;
            QVector<GrammarIssue> issues;
            const bool exhausted = shared->exhausted();
            if (!exhausted) {
                QElapsedTimer t;
                t.start();
                issues = grammarCheck(text, language);
                shared->spend(text.size(), t.elapsed());
            }
            if (shared->gone || !self) return;
            QMetaObject::invokeMethod(self.data(), [self, key, issues, exhausted] { if (self) self->done(key, issues, exhausted); }, Qt::QueuedConnection);
        }));
    }
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
    if (m_enabled) Q_EMIT changed();
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

} // namespace jp::write
