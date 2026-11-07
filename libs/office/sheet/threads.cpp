#include "sheet/threads.h"

#include <QDateTime>
#include <QUuid>

#include <algorithm>
#include <set>
#include <unordered_map>

namespace jp::office::sheet {

bool sameThread(const ThreadPtr &a, const ThreadPtr &b)
{
    if (a == b) return true;
    const size_t na = a ? a->size() : 0, nb = b ? b->size() : 0;
    return na == nb && (na == 0 || *a == *b);
}

QString threadWords(const ThreadList &thread)
{
    QString out;
    for (const ThreadComment &c : thread) {
        if (!out.isEmpty()) out += QStringLiteral("\n\n");
        out += c.author.isEmpty() ? c.text : c.author + QStringLiteral(": ") + c.text;
    }
    return out;
}

QString legacyThreadText(const ThreadList &thread)
{
    QString out = QStringLiteral("[Threaded comment]\n\nThis cell has a thread of comments. A program that cannot show threads shows this note in its place, and changes made to the note are lost when the file is opened in a program that can.");
    for (size_t i = 0; i < thread.size(); ++i) {
        const ThreadComment &c = thread[i];
        out += i == 0 ? QStringLiteral("\n\nComment:\n    ") : QStringLiteral("\nReply:\n    ");
        out += c.text;
    }
    return out;
}

QString legacyThreadAuthor(const ThreadList &thread) { return QStringLiteral("tc=") + (thread.empty() ? QString() : thread.front().id); }

bool threadResolved(const ThreadList &thread) { return !thread.empty() && thread.front().done; }

int threadDepth(const ThreadList &thread, size_t index)
{
    int depth = 0;
    while (index < thread.size() && !thread[index].parentId.isEmpty() && depth < 32) {
        size_t up = thread.size();
        for (size_t i = 0; i < thread.size(); ++i)
            if (thread[i].id == thread[index].parentId) {
                up = i;
                break;
            }
        if (up >= thread.size() || up == index) break;
        index = up;
        ++depth;
    }
    return depth;
}

std::vector<int> threadDepths(const ThreadList &thread)
{
    std::unordered_map<QString, size_t> at;
    at.reserve(thread.size());
    for (size_t i = 0; i < thread.size(); ++i) at.emplace(thread[i].id, i);   // (the first comment of an id is the one that replies name)
    std::vector<int> out(thread.size(), 0);
    for (size_t i = 0; i < thread.size(); ++i) {
        size_t here = i;
        int depth = 0;
        while (!thread[here].parentId.isEmpty() && depth < 32) {
            const auto up = at.find(thread[here].parentId);
            if (up == at.end() || up->second == here) break;
            here = up->second;
            ++depth;
        }
        out[i] = depth;
    }
    return out;
}

QString newThreadId() { return QUuid::createUuid().toString(QUuid::WithBraces).toUpper(); }

QString threadTimeNow()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    return now.toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss")) + QStringLiteral(".00");
}

} // namespace jp::office::sheet
