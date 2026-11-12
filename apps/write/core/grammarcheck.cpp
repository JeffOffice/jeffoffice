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
}

// What the thread and the window share: the budget of the whole document (spent by the thread), and whether the checker is gone.
struct GrammarCheck::Shared {
    QMutex lock;
    GrammarBudget budget;
    std::atomic<bool> gone{false};
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
    Q_EMIT changed();
}

// The paragraph as the checker takes it: its text, its language (the first stretch that states one, else the story's), and the stretches marked "do not check".
GrammarCheck::Look GrammarCheck::lookAt(const QTextBlock &block) const
{
    Look look;
    look.text = block.text();
    for (auto it = block.begin(); !it.atEnd(); ++it) {
        const QTextCharFormat cf = it.fragment().charFormat();
        if (look.language.isEmpty()) look.language = cf.stringProperty(tp::Language);
        if (cf.boolProperty(tp::NoProof)) look.noProof << qMakePair(it.fragment().position() - block.position(), it.fragment().position() - block.position() + it.fragment().length());
    }
    if (look.language.isEmpty()) look.language = m_language;
    look.key = look.language + QChar(1) + look.text;
    return look;
}

QVector<GrammarCheck::Issue> GrammarCheck::issuesOf(const QTextBlock &block, const Look &look, const Entry &entry) const
{
    QVector<Issue> out;
    for (const GrammarIssue &g : entry.issues) {
        bool skip = false;
        for (const auto &n : look.noProof)
            if (g.start < n.second && g.end > n.first) skip = true;
        if (g.end > look.text.size()) skip = true;
        if (!skip && m_ignored.contains(g.message + QChar(1) + look.text.mid(g.start, g.end - g.start))) skip = true;
        if (skip) continue;
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
        const Look look = lookAt(b);
        const auto it = m_kept.constFind(look.key);
        if (it != m_kept.constEnd()) {
            for (const Issue &is : issuesOf(b, look, *it))
                if (is.end > from && is.start < to) out << is;
        } else if (grammarAvailable() && grammarSupports(look.language)) {
            want(look);
        }
    }
    return out;
}

GrammarCheck::Issue GrammarCheck::issueAt(int position)
{
    if (!m_enabled || !m_story) return Issue();
    const QTextBlock b = m_story->findBlock(position);
    if (!b.isValid()) return Issue();
    const Look look = lookAt(b);
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
        const Look look = lookAt(b);
        auto it = m_kept.constFind(look.key);
        if (it == m_kept.constEnd()) {
            if (!grammarSupports(look.language)) continue;
            bool truncated = false;
            QMutexLocker locker(&m_shared->lock);
            Entry e;
            e.issues = grammarCheck(look.text, look.language, &truncated, &m_shared->budget);
            locker.unlock();
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

void GrammarCheck::ignore(const Issue &issue)
{
    if (!m_story || !issue.valid()) return;
    const QTextBlock b = m_story->findBlock(issue.start);
    m_ignored.insert(issue.message + QChar(1) + b.text().mid(issue.start - b.position(), issue.end - issue.start));
    Q_EMIT changed();
}

void GrammarCheck::want(const Look &look)
{
    if (m_sent.contains(look.key) || m_wanted.contains(look.key)) return;
    m_wanted.insert(look.key, look);
    m_timer.start(m_delay);
}

void GrammarCheck::send()
{
    const QHash<QString, Look> wanted = m_wanted;
    m_wanted.clear();
    for (const Look &look : wanted) {
        if (m_sent.contains(look.key) || m_kept.contains(look.key)) continue;
        m_sent.insert(look.key);
        ++m_inFlight;
        ++m_checked;
        const std::shared_ptr<Shared> shared = m_shared;
        const QString key = look.key, text = look.text, language = look.language;
        QPointer<GrammarCheck> self(this);
        m_pool->start(QRunnable::create([shared, self, key, text, language] {
            if (shared->gone) return;
            QVector<GrammarIssue> issues;
            {
                QMutexLocker locker(&shared->lock);
                issues = grammarCheck(text, language, nullptr, &shared->budget);
            }
            if (shared->gone || !self) return;
            QMetaObject::invokeMethod(self.data(), [self, key, issues] { if (self) self->done(key, issues); }, Qt::QueuedConnection);
        }));
    }
}

void GrammarCheck::done(const QString &key, const QVector<GrammarIssue> &issues)
{
    --m_inFlight;
    m_sent.remove(key);
    Entry e;
    e.issues = issues;
    keep(key, e);
    if (m_enabled) Q_EMIT changed();
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
