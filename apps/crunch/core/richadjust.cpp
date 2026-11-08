#include "core/richadjust.h"

namespace jp::crunch {

void adjustRuns(jp::office::sheet::RichText *rich, const QString &now)
{
    using jp::office::sheet::RichRun;
    const QString old = rich->text;
    rich->text = now;
    if (rich->runs.isEmpty() || old == now) return;
    qsizetype p = 0;
    while (p < old.size() && p < now.size() && old[p] == now[p]) ++p;
    qsizetype s = 0;
    while (s < old.size() - p && s < now.size() - p && old[old.size() - 1 - s] == now[now.size() - 1 - s]) ++s;
    const qsizetype removed = old.size() - p - s, added = now.size() - p - s;
    for (RichRun &run : rich->runs) {
        if (qsizetype(run.start) >= p + removed && run.start > 0) run.start = quint32(qsizetype(run.start) - removed + added);
        else if (qsizetype(run.start) > p) run.start = quint32(p);
    }
    // (runs with nothing in them go, and the first begins at the first character)
    QVector<RichRun> kept;
    for (int i = 0; i < rich->runs.size(); ++i) {
        const qsizetype end = i + 1 < rich->runs.size() ? qsizetype(rich->runs[i + 1].start) : now.size();
        if (end > qsizetype(rich->runs[i].start) || (kept.isEmpty() && i + 1 == rich->runs.size())) kept.append(rich->runs[i]);
    }
    if (!kept.isEmpty()) kept[0].start = 0;
    rich->runs = kept;
    if (now.isEmpty()) rich->runs.clear();
}

} // namespace jp::crunch
