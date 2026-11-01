#include "core/outline.h"

#include <algorithm>
#include <map>

namespace jp::crunch {

Outline::Outline(std::vector<OutlineLine> lines, quint32 count, bool summaryAfter) : m_lines(std::move(lines)), m_count(count), m_after(summaryAfter)
{
    for (const OutlineLine &l : m_lines) m_deepest = std::max<int>(m_deepest, l.level);
}

const OutlineLine *Outline::find(quint32 index) const
{
    const auto it = std::lower_bound(m_lines.begin(), m_lines.end(), index, [](const OutlineLine &l, quint32 i) { return l.index < i; });
    return it != m_lines.end() && it->index == index ? &*it : nullptr;
}

quint8 Outline::levelOf(quint32 index) const
{
    const OutlineLine *l = find(index);
    return l ? l->level : 0;
}

bool Outline::hiddenOf(quint32 index) const
{
    const OutlineLine *l = find(index);
    return l && l->hidden;
}

bool Outline::collapsedOf(quint32 index) const
{
    const OutlineLine *l = find(index);
    return l && l->collapsed;
}

std::vector<Outline::Group> Outline::groups(quint32 from, quint32 to) const
{
    std::vector<Group> out;
    for (int level = 1; level <= m_deepest; ++level) {
        // the runs of lines of this level or deeper, as the lines listed are (a gap in the indexes ends a run)
        size_t i = 0;
        while (i < m_lines.size()) {
            if (m_lines[i].level < level) {
                ++i;
                continue;
            }
            size_t j = i;
            while (j + 1 < m_lines.size() && m_lines[j + 1].index == m_lines[j].index + 1 && m_lines[j + 1].level >= level) ++j;
            Group g;
            g.first = m_lines[i].index;
            g.last = m_lines[j].index;
            g.level = level;
            const qint64 s = m_after ? qint64(g.last) + 1 : qint64(g.first) - 1;
            g.summary = s >= 0 && s < qint64(m_count) ? s : -1;
            g.collapsed = g.summary >= 0 && collapsedOf(quint32(g.summary));
            if (g.last >= from && g.first <= to) out.push_back(g);
            i = j + 1;
        }
    }
    return out;
}

std::vector<OutlineChange> Outline::collapse(const Group &g) const
{
    std::vector<OutlineChange> out;
    for (quint32 i = g.first; i <= g.last; ++i) {
        if (!hiddenOf(i)) out.push_back({i, -1, 1, -1});
        if (i == 0xFFFFFFFFu) break;
    }
    if (g.summary >= 0 && !collapsedOf(quint32(g.summary))) out.push_back({quint32(g.summary), -1, -1, 1});
    return out;
}

std::vector<OutlineChange> Outline::expand(const Group &g) const
{
    std::vector<OutlineChange> out;
    if (g.summary >= 0 && collapsedOf(quint32(g.summary))) out.push_back({quint32(g.summary), -1, -1, 0});
    // the lines of the groups inside this one that are collapsed stay hidden: those groups, as intervals in order of place, joined where they touch, and one pass over the lines of this group
    std::vector<std::pair<quint32, quint32>> held;
    for (const Group &h : groups(g.first, g.last)) {
        ++m_steps;
        if (h.level > g.level && h.collapsed) held.emplace_back(h.first, h.last);
    }
    std::sort(held.begin(), held.end());
    size_t at = 0;
    auto line = std::lower_bound(m_lines.begin(), m_lines.end(), g.first, [](const OutlineLine &l, quint32 i) { return l.index < i; });
    for (; line != m_lines.end() && line->index <= g.last; ++line) {
        ++m_steps;
        while (at < held.size() && held[at].second < line->index) ++at;
        const bool hide = at < held.size() && held[at].first <= line->index;
        if (line->hidden != hide) out.push_back({line->index, -1, hide ? 1 : 0, -1});
    }
    return out;
}

std::vector<OutlineChange> Outline::showButton(int button) const
{
    std::vector<OutlineChange> out;
    std::map<quint32, OutlineChange> by;
    auto at = [&](quint32 i) -> OutlineChange & {
        auto it = by.find(i);
        if (it == by.end()) it = by.emplace(i, OutlineChange{i, -1, -1, -1}).first;
        return it->second;
    };
    // a line is shown when its level is less than the button; a group of the button's level or deeper is collapsed, the rest are not
    for (const OutlineLine &l : m_lines) {
        if (l.level == 0) continue;   // (a line in no group that is hidden was hidden by hand: the buttons leave it)
        const bool hide = l.level >= button;
        if (l.hidden != hide) at(l.index).hidden = hide ? 1 : 0;
    }
    for (const Group &g : groups(0, 0xFFFFFFFFu)) {
        if (g.summary < 0) continue;
        const bool collapse = g.level >= button;
        if (collapsedOf(quint32(g.summary)) != collapse) at(quint32(g.summary)).collapsed = collapse ? 1 : 0;
    }
    for (const auto &kv : by) out.push_back(kv.second);
    return out;
}

namespace {

// The changes of levels of lines first..last by `delta`.
std::vector<OutlineChange> shifted(const Outline &o, quint32 first, quint32 last, int delta)
{
    std::vector<OutlineChange> out;
    for (quint32 i = first; i <= last; ++i) {
        const int now = o.levelOf(i), then = std::clamp(now + delta, 0, kMaxOutlineLevel);
        if (then != now) {
            OutlineChange c{i, then, -1, -1};
            if (then == 0 && o.hiddenOf(i)) c.hidden = 0;   // (a line that is no longer in a group is shown)
            out.push_back(c);
        }
        if (i == 0xFFFFFFFFu) break;
    }
    return out;
}

} // namespace

std::vector<OutlineChange> Outline::group(quint32 first, quint32 last) const { return shifted(*this, first, last, 1); }

std::vector<OutlineChange> Outline::ungroup(quint32 first, quint32 last) const
{
    std::vector<OutlineChange> out = shifted(*this, first, last, -1);
    // the summaries of groups that are gone no longer say they are collapsed: a summary line that is beside no group once these lines are one level less
    std::vector<OutlineLine> after = m_lines;
    for (const OutlineChange &c : out) {
        const auto it = std::lower_bound(after.begin(), after.end(), c.index, [](const OutlineLine &l, quint32 i) { return l.index < i; });
        if (it != after.end() && it->index == c.index) it->level = quint8(c.level);
        if (c.hidden == 0 && it != after.end() && it->index == c.index) it->hidden = false;
    }
    const Outline now(after, m_count, m_after);
    for (const Group &g : groups(0, 0xFFFFFFFFu)) {
        if (g.summary < 0 || !g.collapsed) continue;
        bool still = false;
        for (const Group &h : now.groups(0, 0xFFFFFFFFu))
            if (h.first == g.first && h.last == g.last && h.level == g.level) still = true;
        if (!still) {
            auto it = std::find_if(out.begin(), out.end(), [&](const OutlineChange &c) { return c.index == quint32(g.summary); });
            if (it == out.end()) out.push_back({quint32(g.summary), -1, -1, 0});
            else it->collapsed = 0;
        }
    }
    // (and the lines of a group that is gone while it was collapsed are shown, which the level of 0 above does for those that left every group)
    std::sort(out.begin(), out.end(), [](const OutlineChange &a, const OutlineChange &b) { return a.index < b.index; });
    return out;
}

} // namespace jp::crunch
