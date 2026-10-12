#include "core/sections.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QUuid>
#include <algorithm>

namespace jp::deck {

namespace {

const QString kKey = QStringLiteral("sections");

// The list as stored, its counts as they are (not yet made to fit the slides).
QVector<Section> stored(const Document &d)
{
    QVector<Section> out;
    for (const QJsonValue &v : d.appData.value(kKey).toArray()) {
        const QJsonObject o = v.toObject();
        out << Section{o.value(QStringLiteral("id")).toString(), o.value(QStringLiteral("name")).toString(), std::max(0, o.value(QStringLiteral("count")).toInt())};
    }
    return out;
}

// The counts made to add up to `slides`: slides they don't reach join the
// last section, counts past the last slide are cut off from the end.
void fit(QVector<Section> &list, int slides)
{
    if (list.isEmpty()) return;
    int total = 0;
    for (const Section &s : std::as_const(list)) total += s.count;
    if (total < slides) list.last().count += slides - total;
    for (int i = int(list.size()) - 1; i >= 0 && total > slides; --i) {
        const int cut = std::min(list[i].count, total - slides);
        list[i].count -= cut;
        total -= cut;
    }
}

// The section of each of `slides` slides, in order.
QVector<int> perSlide(const QVector<Section> &list, int slides)
{
    QVector<int> out;
    out.reserve(slides);
    for (int s = 0; s < list.size(); ++s)
        for (int k = 0; k < list[s].count && out.size() < slides; ++k) out << s;
    while (out.size() < slides) out << std::max(0, int(list.size()) - 1);
    return out;
}

// The index of the section that holds slide `slide`, the last one for a place past the end; -1 with no sections.
int containing(const QVector<Section> &list, int slide)
{
    int start = 0;
    for (int s = 0; s < list.size(); ++s) {
        if (slide < start + list[s].count) return s;
        start += list[s].count;
    }
    return list.isEmpty() ? -1 : int(list.size()) - 1;
}

} // namespace

QVector<Section> sectionsOf(const Document &d)
{
    QVector<Section> list = stored(d);
    fit(list, int(d.pages.size()));
    return list;
}

void setSections(Document &d, const QVector<Section> &sections)
{
    if (sections.isEmpty()) {
        d.appData.remove(kKey);
        return;
    }
    QVector<Section> list = sections;
    fit(list, int(d.pages.size()));
    QJsonArray out;
    for (const Section &s : std::as_const(list))
        out.append(QJsonObject{{"id", s.id}, {"name", s.name}, {"count", s.count}});
    d.appData[kKey] = out;
}

bool hasSections(const Document &d) { return !d.appData.value(kKey).toArray().isEmpty(); }

QString newSectionId() { return QUuid::createUuid().toString(QUuid::WithBraces).toUpper(); }

QString untitledSectionName() { return QCoreApplication::translate("Deck", "Untitled Section"); }
QString defaultSectionName() { return QCoreApplication::translate("Deck", "Default Section"); }

int sectionOfSlide(const Document &d, int slide)
{
    if (slide < 0 || slide >= d.pages.size()) return -1;
    return containing(sectionsOf(d), slide);
}

int firstSlideOf(const Document &d, int section)
{
    const QVector<Section> list = sectionsOf(d);
    int start = 0;
    for (int s = 0; s < list.size() && s < section; ++s) start += list[s].count;
    return start;
}

int slideCountOf(const Document &d, int section)
{
    const QVector<Section> list = sectionsOf(d);
    return section >= 0 && section < list.size() ? list[section].count : 0;
}

int addSection(Document &d, int before, const QString &name)
{
    QVector<Section> list = sectionsOf(d);
    const int n = int(d.pages.size());
    before = std::clamp(before, 0, n);
    Section made{newSectionId(), name.isEmpty() ? untitledSectionName() : name, 0};
    if (list.isEmpty()) {
        if (before > 0) list << Section{newSectionId(), defaultSectionName(), before};
        made.count = n - before;
        list << made;
        setSections(d, list);
        return int(list.size()) - 1;
    }
    int start = 0, at = int(list.size()) - 1;
    bool inside = false;
    for (int s = 0; s < list.size(); ++s) {
        if (before < start + list[s].count) {
            at = s;
            inside = true;
            break;
        }
        start += list[s].count;
    }
    if (inside) {
        made.count = list[at].count - (before - start);
        list[at].count = before - start;
    }
    list.insert(at + 1, made);
    setSections(d, list);
    return at + 1;
}

void renameSection(Document &d, int section, const QString &name)
{
    QVector<Section> list = sectionsOf(d);
    if (section < 0 || section >= list.size()) return;
    list[section].name = name;
    setSections(d, list);
}

void removeSection(Document &d, int section)
{
    QVector<Section> list = sectionsOf(d);
    if (section < 0 || section >= list.size()) return;
    if (list.size() == 1) {
        removeAllSections(d);
        return;
    }
    // The slides go up to the section before, the first section's to the next.
    list[section ? section - 1 : section + 1].count += list[section].count;
    list.remove(section);
    setSections(d, list);
}

void removeAllSections(Document &d) { d.appData.remove(kKey); }

bool removeSectionAndSlides(Document &d, int section)
{
    QVector<Section> list = sectionsOf(d);
    if (section < 0 || section >= list.size()) return false;
    const int first = firstSlideOf(d, section), count = list[section].count;
    if (count >= d.pages.size()) return false;
    d.pages.remove(first, count);
    list.remove(section);
    setSections(d, list);
    return true;
}

int moveSection(Document &d, int section, int by)
{
    QVector<Section> list = sectionsOf(d);
    const int other = section + (by < 0 ? -1 : 1);
    if (section < 0 || section >= list.size() || other < 0 || other >= list.size() || by == 0) return -1;
    const int lower = std::min(section, other);
    const int start = firstSlideOf(d, lower), middle = start + list[lower].count, end = middle + list[lower + 1].count;
    // The later block of slides comes before the earlier one.
    std::rotate(d.pages.begin() + start, d.pages.begin() + middle, d.pages.begin() + end);
    std::swap(list[lower], list[lower + 1]);
    setSections(d, list);
    return other;
}

void slideInserted(Document &d, int at)
{
    QVector<Section> list = stored(d);
    if (list.isEmpty()) return;
    fit(list, int(d.pages.size()) - 1);
    // With the slide before it, or first.
    list[at <= 0 ? 0 : containing(list, at - 1)].count++;
    setSections(d, list);
}

void slideRemoved(Document &d, int at)
{
    QVector<Section> list = stored(d);
    if (list.isEmpty()) return;
    fit(list, int(d.pages.size()) + 1);
    const int s = containing(list, at);
    if (s >= 0 && list[s].count > 0) list[s].count--;
    setSections(d, list);
}

void slidesReordered(Document &d, const QList<int> &order, const QList<int> &moved, int into)
{
    QVector<Section> list = stored(d);
    if (list.isEmpty()) return;
    const int n = int(d.pages.size());
    fit(list, n);
    if (order.size() != n) return;
    const QVector<int> was = perSlide(list, n);
    const QSet<int> away(moved.begin(), moved.end());
    const int last = int(list.size()) - 1;
    QVector<int> now(n);
    for (int k = 0; k < n; ++k) {
        const int old = order[k];
        if (old < 0 || old >= n) return;
        if (!away.contains(old)) {
            now[k] = was[old];
            continue;
        }
        // Between the slides each side of its new place, in the section there is.
        const int low = k ? now[k - 1] : 0;
        int high = last;
        for (int j = k + 1; j < n; ++j)
            if (!away.contains(order[j])) {
                high = was[order[j]];
                break;
            }
        now[k] = into >= 0 ? std::clamp(into, low, std::max(low, high)) : low;
    }
    for (Section &s : list) s.count = 0;
    for (int s : now) list[s].count++;
    setSections(d, list);
}

QList<int> orderAfterMoving(int count, const QList<int> &moved, int before)
{
    QList<int> chosen;
    for (int i : moved)
        if (i >= 0 && i < count && !chosen.contains(i)) chosen << i;
    std::sort(chosen.begin(), chosen.end());
    before = std::clamp(before, 0, count);
    QList<int> order;
    for (int i = 0; i < count; ++i) {
        if (i == before) order << chosen;
        if (!chosen.contains(i)) order << i;
    }
    if (before == count) order << chosen;
    return order;
}

} // namespace jp::deck
