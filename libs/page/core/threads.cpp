#include "core/threads.h"

#include <QCoreApplication>
#include <atomic>

namespace jp::threads {

namespace {
std::atomic<int> g_threads{0}, g_replies{0};
}

int maxThreads() { return g_threads ? int(g_threads) : kMaxThreads; }
int maxReplies() { return g_replies ? int(g_replies) : kMaxReplies; }
void setCapsForTest(int threads, int replies)
{
    g_threads = threads;
    g_replies = replies;
}

Refusal checkText(const QString &text, bool needWords)
{
    if (needWords && text.trimmed().isEmpty()) return Refusal::Empty;
    if (text.size() > kMaxChars) return Refusal::TooLong;
    return Refusal::None;
}

Refusal checkNewThread(const QString &text, int threadsNow, bool needWords)
{
    const Refusal r = checkText(text, needWords);
    if (r != Refusal::None) return r;
    return threadsNow >= maxThreads() ? Refusal::TooManyThreads : Refusal::None;
}

Refusal checkReply(const QString &text, int repliesNow, bool needWords)
{
    const Refusal r = checkText(text, needWords);
    if (r != Refusal::None) return r;
    return repliesNow >= maxReplies() ? Refusal::TooManyReplies : Refusal::None;
}

QString refusalText(Refusal r)
{
    switch (r) {
    case Refusal::None: return QString();
    case Refusal::Empty: return QCoreApplication::translate("jp::threads", "A comment needs some words.");
    case Refusal::TooLong: return QCoreApplication::translate("jp::threads", "A comment can have up to %1 characters. Shorten it, and try again.").arg(kMaxChars);
    case Refusal::TooManyThreads: return QCoreApplication::translate("jp::threads", "A document can have up to %1 comment threads.").arg(maxThreads());
    case Refusal::TooManyReplies: return QCoreApplication::translate("jp::threads", "A thread can have up to %1 replies.").arg(maxReplies());
    }
    return QString();
}

QString tidyText(const QString &text)
{
    QString t = text;
    t.replace(QLatin1String("\r\n"), QLatin1String("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));
    t.replace(QChar(QChar::LineSeparator), QLatin1Char('\n')).replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
    return t;
}

int stepIndex(const QVector<qint64> &keys, qint64 from, bool forward)
{
    int best = -1;
    for (int i = 0; i < keys.size(); ++i) {
        if (forward ? keys[i] <= from : keys[i] >= from) continue;
        if (best < 0 || (forward ? keys[i] < keys[best] : keys[i] > keys[best])) best = i;
    }
    return best;
}

QString threadRoot(const QString &id, const std::function<QString(const QString &)> &parentOf, int most)
{
    QString at = id;
    for (int i = 0; i < most; ++i) {
        const QString up = parentOf(at);
        if (up.isEmpty() || up == at) return at;
        at = up;
    }
    return at;
}

} // namespace jp::threads
