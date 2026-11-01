#include "core/rulerops.h"

#include "core/editops.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <algorithm>
#include <cmath>

namespace jp::write {

QVector<TabStopEntry> tabStopsOf(const QTextBlockFormat &format)
{
    QVector<TabStopEntry> out;
    const QString leaders = format.stringProperty(tp::TabLeaders);
    const QList<QTextOption::Tab> tabs = format.tabPositions();
    for (int i = 0; i < tabs.size() && out.size() < kMostTabStops; ++i) {
        TabStopEntry e;
        e.position = tabs[i].position;
        e.type = tabs[i].type;
        e.leader = i < leaders.size() ? leaders[i] : QLatin1Char(' ');
        out.push_back(e);
    }
    std::stable_sort(out.begin(), out.end(), [](const TabStopEntry &a, const TabStopEntry &b) { return a.position < b.position; });
    return out;
}

void setTabStopsOf(QTextBlockFormat &format, QVector<TabStopEntry> stops)
{
    for (TabStopEntry &s : stops) s.position = std::isfinite(s.position) ? std::clamp(s.position, 0.0, kMostTabPosition) : 0.0;
    std::stable_sort(stops.begin(), stops.end(), [](const TabStopEntry &a, const TabStopEntry &b) { return a.position < b.position; });
    QVector<TabStopEntry> kept;
    for (const TabStopEntry &s : std::as_const(stops)) {
        if (!kept.isEmpty() && std::abs(kept.last().position - s.position) < 0.05) kept.last() = s;
        else kept.push_back(s);
    }
    if (kept.size() > kMostTabStops) kept.resize(kMostTabStops);
    QList<QTextOption::Tab> tabs;
    QString leaders;
    bool anyLeader = false;
    for (const TabStopEntry &s : std::as_const(kept)) {
        QTextOption::Tab t;
        t.position = s.position;
        t.type = s.type;
        if (s.type == QTextOption::DelimiterTab) t.delimiter = QLatin1Char('.');
        tabs << t;
        leaders += s.leader;
        anyLeader = anyLeader || s.leader != QLatin1Char(' ');
    }
    format.setTabPositions(tabs);
    if (anyLeader) format.setProperty(tp::TabLeaders, leaders);
    else format.clearProperty(tp::TabLeaders);
}

void setTabStops(WriteDoc &doc, QTextCursor &c, const QVector<TabStopEntry> &stops)
{
    changeBlockFormat(doc, c, [&](QTextBlockFormat &bf) { setTabStopsOf(bf, stops); });
}

namespace {

int indexNear(const QVector<TabStopEntry> &stops, double at)
{
    int best = -1;
    double distance = 0.5;
    for (int i = 0; i < stops.size(); ++i)
        if (std::abs(stops[i].position - at) <= distance) {
            best = i;
            distance = std::abs(stops[i].position - at);
        }
    return best;
}

// What `change` does to the first paragraph's stops is done to every paragraph's own (so that paragraphs with different stops each keep theirs).
bool changeStops(WriteDoc &doc, QTextCursor &c, const std::function<bool(QVector<TabStopEntry> *)> &change)
{
    bool any = false;
    changeBlockFormat(doc, c, [&](QTextBlockFormat &bf) {
        QVector<TabStopEntry> stops = tabStopsOf(bf);
        if (!change(&stops)) return;
        setTabStopsOf(bf, stops);
        any = true;
    });
    return any;
}

} // namespace

bool addTabStop(WriteDoc &doc, QTextCursor &c, const TabStopEntry &stop)
{
    return changeStops(doc, c, [&](QVector<TabStopEntry> *stops) {
        const int at = indexNear(*stops, stop.position);
        if (at >= 0 && (*stops)[at] == stop) return false;
        if (at < 0 && stops->size() >= kMostTabStops) return false;
        if (at >= 0) (*stops)[at] = stop;
        else stops->push_back(stop);
        return true;
    });
}

bool moveTabStop(WriteDoc &doc, QTextCursor &c, double from, double to)
{
    return changeStops(doc, c, [&](QVector<TabStopEntry> *stops) {
        const int at = indexNear(*stops, from);
        if (at < 0 || std::abs((*stops)[at].position - to) < 0.05) return false;
        (*stops)[at].position = to;
        return true;
    });
}

bool removeTabStop(WriteDoc &doc, QTextCursor &c, double at)
{
    return changeStops(doc, c, [&](QVector<TabStopEntry> *stops) {
        const int i = indexNear(*stops, at);
        if (i < 0) return false;
        stops->removeAt(i);
        return true;
    });
}

} // namespace jp::write
