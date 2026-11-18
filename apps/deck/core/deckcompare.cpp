#include "core/deckcompare.h"

#include "core/items.h"
#include "core/comments.h"
#include "core/reuse.h"
#include "core/sections.h"
#include "text/storyio.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonDocument>
#include <QSet>
#include <QTextDocument>
#include <algorithm>
#include <cmath>
#include <vector>

namespace jp::deck {

using review::Change;
using review::Kind;
using review::State;

namespace {

constexpr int kMaxDepth = 16;             // groups inside groups that are read
constexpr int kMaxTokens = 200;           // words of a slide that its likeness to another is judged by
constexpr int kMaxPosting = 64;           // a word on more slides than this tells nothing, and is not followed
constexpr qsizetype kMaxItemText = 100000;
constexpr qsizetype kMaxSlideText = 20000;

// ---- reading a presentation ----

struct Rec {
    const Item *item = nullptr;
    int parent = -1;     // the group it is in (index into recs), -1 on the page
    int index = 0;       // its place in its list
    int ord = 0;         // which of the objects of the slide with its id it is, in walk order
    QString text;        // the text of all its stories, one separator between stories
    QString path;        // its place from the top of the slide: the indices of the lists, dotted
    QVector<int> kids;
};

struct Flat {
    QVector<Rec> recs;
    QVector<int> top;
    bool complete = true;    // every object of the slide was read
};

QString textOf(const Document &d, const Item &it)
{
    auto story = [&](const QString &id) {
        const QTextDocument *sd = id.isEmpty() ? nullptr : d.storyDoc(id);
        return sd ? sd->toPlainText().left(kMaxItemText) : QString();
    };
    switch (it.type()) {
    case ItemType::Text: return story(static_cast<const TextItem &>(it).storyId);
    case ItemType::Shape: return story(static_cast<const ShapeItem &>(it).storyId);
    case ItemType::Table: {
        QString out;
        for (const TableCell &c : static_cast<const TableItem &>(it).cells) {
            if (out.size() >= kMaxItemText) break;
            out += story(c.storyId) + QChar(0x1f) + story(c.storyId2) + QChar(0x1f);
        }
        return out;
    }
    default: return QString();
    }
}

QStringList storyIdsOf(const Item &it)
{
    QStringList out;
    switch (it.type()) {
    case ItemType::Text: out << static_cast<const TextItem &>(it).storyId; break;
    case ItemType::Shape: out << static_cast<const ShapeItem &>(it).storyId; break;
    case ItemType::Table:
        for (const TableCell &c : static_cast<const TableItem &>(it).cells) out << c.storyId << c.storyId2;
        break;
    default: break;
    }
    return out;
}

// The text with its formatting, for telling a change of look from none.
QString storySignature(const Document &d, const Item &it)
{
    QString out;
    for (const QString &id : storyIdsOf(it)) {
        const QTextDocument *sd = id.isEmpty() ? nullptr : d.storyDoc(id);
        if (sd) out += QString::fromUtf8(QJsonDocument(storyToJson(sd)).toJson(QJsonDocument::Compact));
        out += QChar(0x1f);
    }
    return out;
}

void flatten(const Document &d, const ItemList &list, int parent, int depth, const QString &prefix, Flat &flat, QHash<QString, int> &seen, int &budget)
{
    for (int i = 0; i < int(list.size()); ++i) {
        if (budget <= 0) {
            flat.complete = false;
            return;
        }
        --budget;
        const Item *it = list[i].get();
        Rec r;
        r.item = it;
        r.parent = parent;
        r.index = i;
        r.ord = seen[it->id]++;
        r.text = textOf(d, *it);
        r.path = prefix.isEmpty() ? QString::number(i) : prefix + QLatin1Char('.') + QString::number(i);
        const int at = int(flat.recs.size());
        flat.recs << r;
        if (parent < 0) flat.top << at;
        else flat.recs[parent].kids << at;
        if (it->type() == ItemType::Group && depth < kMaxDepth) flatten(d, static_cast<const GroupItem *>(it)->children, at, depth + 1, r.path, flat, seen, budget);
    }
}

struct SideInfo {
    const Document *doc = nullptr;
    int slides = 0;
    int objects = 0;
    QVector<Flat> flats;
    QVector<QString> titles, keys;
    QVector<QVector<uint>> tokens;   // sorted, distinct
};

QString simplified(const QString &s) { return s.simplified().toLower(); }

QVector<uint> tokensOf(const QString &text)
{
    QVector<uint> out;
    QSet<uint> seen;
    qsizetype from = -1;
    auto close = [&](qsizetype to) {
        if (from < 0) return;
        const uint h = qHash(QStringView(text).mid(from, to - from).toString().toLower());
        from = -1;
        if (out.size() < kMaxTokens && !seen.contains(h)) { seen.insert(h); out << h; }
    };
    for (qsizetype i = 0; i < text.size() && out.size() < kMaxTokens; ++i) {
        if (text[i].isLetterOrNumber()) { if (from < 0) from = i; }
        else close(i);
    }
    close(text.size());
    std::sort(out.begin(), out.end());
    return out;
}

SideInfo readSide(const Document &d, int *objectBudgetUsed)
{
    SideInfo s;
    s.doc = &d;
    s.slides = std::min(int(d.pages.size()), kMaxCompareSlides);
    int budget = kMaxCompareObjects;
    s.flats.resize(s.slides);
    s.titles.resize(s.slides);
    s.keys.resize(s.slides);
    s.tokens.resize(s.slides);
    for (int i = 0; i < s.slides; ++i) {
        Flat &f = s.flats[i];
        QHash<QString, int> seen;
        flatten(d, d.pages[i]->items, -1, 0, QString(), f, seen, budget);
        QString all;
        for (const Rec &r : std::as_const(f.recs)) {
            if (all.size() >= kMaxSlideText) break;
            if (!r.text.isEmpty()) all += r.text + QLatin1Char('\n');
        }
        all = all.left(kMaxSlideText);
        const QString title = simplified(slideTitle(d, i).left(kMaxItemText));
        s.titles[i] = title;
        s.keys[i] = title + QChar(0x1e) + simplified(all);
        s.tokens[i] = tokensOf(title + QLatin1Char(' ') + all);
    }
    *objectBudgetUsed = kMaxCompareObjects - budget;
    s.objects = *objectBudgetUsed;
    return s;
}

// ---- matching ----

struct Queue {
    QVector<int> v;
    int at = 0;
};

// One stage of matching: each of the `theirs` not yet matched takes the first unused one of `mine` that has the same key (an empty key matches nothing).
// Hash lookups and no pair is compared, so the work is a few steps for each.
template <class KM, class KT>
void stage(const QVector<int> &mine, const QVector<int> &theirs, KM keyM, KT keyT, std::vector<int> &mineOf, std::vector<char> &used, qint64 &work)
{
    QHash<QString, Queue> table;
    for (int m : mine) {
        if (used[m]) continue;
        const QString k = keyM(m);
        ++work;
        if (!k.isEmpty()) table[k].v << m;
    }
    for (int t : theirs) {
        if (mineOf[t] >= 0) continue;
        const QString k = keyT(t);
        ++work;
        if (k.isEmpty()) continue;
        const auto found = table.find(k);
        if (found == table.end()) continue;
        Queue &q = found.value();
        while (q.at < q.v.size() && used[q.v[q.at]]) ++q.at;
        if (q.at < q.v.size()) {
            const int m = q.v[q.at++];
            mineOf[t] = m;
            used[m] = 1;
        }
    }
}

// Slides: by id, the same title and text, the words they share, then the same title.
std::vector<int> matchSlides(const SideInfo &mine, const SideInfo &theirs, qint64 &work)
{
    const int nm = mine.slides, nt = theirs.slides;
    std::vector<int> mineOf(nt, -1);
    std::vector<char> used(nm, 0);
    QVector<int> allM(nm), allT(nt);
    for (int i = 0; i < nm; ++i) allM[i] = i;
    for (int i = 0; i < nt; ++i) allT[i] = i;
    stage(allM, allT, [&](int i) { return mine.doc->pages[i]->id; }, [&](int i) { return theirs.doc->pages[i]->id; }, mineOf, used, work);
    stage(allM, allT, [&](int i) { return mine.keys[i]; }, [&](int i) { return theirs.keys[i]; }, mineOf, used, work);
    // The words they share: for each slide of theirs, the slides of mine that share a word that is on few slides, and the one most like it (at least half the words alike).
    QHash<uint, QVector<int>> postings;
    for (int m = 0; m < nm; ++m) {
        if (used[m]) continue;
        for (uint h : mine.tokens[m]) { postings[h] << m; ++work; }
    }
    QHash<int, int> shared;
    for (int t = 0; t < nt; ++t) {
        if (mineOf[t] >= 0 || theirs.tokens[t].isEmpty()) continue;
        shared.clear();
        for (uint h : theirs.tokens[t]) {
            const auto found = postings.constFind(h);
            ++work;
            if (found == postings.cend() || found->size() > kMaxPosting) continue;
            for (int m : *found) {
                if (used[m]) continue;
                ++shared[m];
                ++work;
            }
        }
        int best = -1;
        double bestScore = 0;
        for (auto it = shared.cbegin(); it != shared.cend(); ++it) {
            const int m = it.key();
            const double score = double(it.value()) / double(theirs.tokens[t].size() + mine.tokens[m].size() - it.value());
            if (score > bestScore || (score == bestScore && best >= 0 && std::abs(m - t) < std::abs(best - t)) || (score == bestScore && best >= 0 && std::abs(m - t) == std::abs(best - t) && m < best)) {
                best = m;
                bestScore = score;
            }
        }
        if (best >= 0 && bestScore >= 0.5) {
            mineOf[t] = best;
            used[best] = 1;
        }
    }
    stage(allM, allT, [&](int i) { return mine.titles[i]; }, [&](int i) { return theirs.titles[i]; }, mineOf, used, work);
    return mineOf;
}

QString geometryKey(const Item &it, int grid)
{
    return QString::number(qRound(it.rect.x() / grid)) + QLatin1Char(',') + QString::number(qRound(it.rect.y() / grid)) + QLatin1Char(',') + QString::number(qRound(it.rect.width() / grid)) + QLatin1Char(',')
           + QString::number(qRound(it.rect.height() / grid));
}

// The objects of one list (a slide's, or a group's) matched with the objects of the other: by id, then the same kind with the same text and place, the same text, the
// same role and place, and the same role. Groups that match have their children matched the same way.
void matchItems(const Flat &fm, const Flat &ft, const QVector<int> &mine, const QVector<int> &theirs, std::vector<int> &mineOf, std::vector<char> &used, qint64 &work)
{
    auto kind = [](const Rec &r) { return QString::number(int(r.item->type())) + QLatin1Char('|'); };
    auto keyed = [&](const Flat &f, int i, int which) -> QString {
        const Rec &r = f.recs[i];
        const Item &it = *r.item;
        switch (which) {
        case 0: return it.id.isEmpty() ? QString() : kind(r) + it.id;
        case 1: return kind(r) + it.role + QLatin1Char('|') + r.text + QLatin1Char('|') + geometryKey(it, 1);
        case 2: return r.text.isEmpty() ? QString() : kind(r) + it.role + QLatin1Char('|') + r.text;
        case 3: return kind(r) + it.role + QLatin1Char('|') + geometryKey(it, 10);
        default: return it.role.isEmpty() ? QString() : kind(r) + it.role;
        }
    };
    for (int which = 0; which < 5; ++which)
        stage(mine, theirs, [&](int i) { return keyed(fm, i, which); }, [&](int i) { return keyed(ft, i, which); }, mineOf, used, work);
    for (int t : theirs) {
        const int m = mineOf[t];
        if (m < 0) continue;
        if (ft.recs[t].item->type() == ItemType::Group && fm.recs[m].item->type() == ItemType::Group) matchItems(fm, ft, fm.recs[m].kids, ft.recs[t].kids, mineOf, used, work);
    }
}

// The slides of mine and theirs, matched in pairs, that keep their order: a longest run of rising indices (patience sorting).
std::vector<char> keepsOrder(const std::vector<int> &mineOf)
{
    std::vector<int> tails, prev(mineOf.size(), -1);
    std::vector<char> keeps(mineOf.size(), 0);
    for (int t = 0; t < int(mineOf.size()); ++t) {
        if (mineOf[t] < 0) continue;
        const int m = mineOf[t];
        const int at = int(std::lower_bound(tails.begin(), tails.end(), m, [&](int slot, int value) { return mineOf[slot] < value; }) - tails.begin());
        if (at > 0) prev[t] = tails[at - 1];
        if (at == int(tails.size())) tails.push_back(t);
        else tails[at] = t;
    }
    if (!tails.empty())
        for (int t = tails.back(); t >= 0; t = prev[t]) keeps[t] = 1;
    return keeps;
}

// ---- describing ----

QString shortText(const QString &s)
{
    const QString one = s.simplified();
    return one.size() > 40 ? one.left(40) + QChar(0x2026) : one;
}

QString describe(const Item &it, const QString &text)
{
    if (!it.name.isEmpty()) return shortText(it.name);
    const QString t = shortText(text);
    return t.isEmpty() ? itemTypeName(it.type()) : t;
}

bool sameGeometry(const Item &a, const Item &b)
{
    return std::abs(a.rect.x() - b.rect.x()) < 0.01 && std::abs(a.rect.y() - b.rect.y()) < 0.01 && std::abs(a.rect.width() - b.rect.width()) < 0.01 && std::abs(a.rect.height() - b.rect.height()) < 0.01
           && std::abs(a.rotation - b.rotation) < 0.01 && a.flipH == b.flipH && a.flipV == b.flipV;
}

QString commentKey(const comments::Comment &c) { return c.author + QLatin1Char('|') + c.date.toUTC().toString(Qt::ISODate) + QLatin1Char('|') + c.text; }

qint64 positionOf(int slideKey, int sub) { return qint64(slideKey) * 100000 + sub; }

// ---- applying ----

CompareRef refOf(const Document &d, int index)
{
    CompareRef r;
    if (index < 0 || index >= d.pages.size()) return r;
    r.id = d.pages[index]->id;
    r.ord = 0;
    for (int i = 0; i < index; ++i) r.ord += d.pages[i]->id == r.id;
    return r;
}

int findSlide(const Document &d, const CompareRef &r)
{
    if (!r.valid()) return -1;
    int n = 0;
    for (int i = 0; i < d.pages.size(); ++i)
        if (d.pages[i]->id == r.id && n++ == r.ord) return i;
    return -1;
}

// Just after the slide before slide `src` of a side that is in the working presentation (the nearest such one), else the first place.
int placeAfter(const Document &w, const QVector<CompareRef> &refs, int src)
{
    for (int j = std::min(src, int(refs.size())) - 1; j >= 0; --j) {
        const int k = findSlide(w, refs[j]);
        if (k >= 0) return k + 1;
    }
    return 0;
}

struct Spot {
    ItemList *list = nullptr;
    int index = -1;
    Item *item = nullptr;
};

bool scanFor(ItemList &list, const QString &id, int *ord, Spot *out)
{
    for (int i = 0; i < int(list.size()); ++i) {
        if (list[i]->id == id && (*ord)-- == 0) {
            *out = Spot{&list, i, list[i].get()};
            return true;
        }
        if (list[i]->type() == ItemType::Group && scanFor(static_cast<GroupItem *>(list[i].get())->children, id, ord, out)) return true;
    }
    return false;
}

Spot findItem(Document &d, int slide, const QString &id, int ord)
{
    if (slide < 0 || slide >= d.pages.size() || ord < 0) return {};
    if (ord == 0) {
        const Document::Loc loc = d.find(id);
        if (loc.item && loc.item->id == id && loc.page == slide && loc.list && !loc.scratch) return Spot{loc.list, loc.index, loc.item};
    }
    Spot out;
    int n = ord;
    scanFor(d.pages[slide]->items, id, &n, &out);
    return out;
}

Item *itemAtPath(const ItemList &top, const QString &path)
{
    if (path.isEmpty()) return nullptr;
    const ItemList *list = &top;
    Item *at = nullptr;
    const QStringList parts = path.split(QLatin1Char('.'));
    for (const QString &p : parts) {
        bool ok = false;
        const int i = p.toInt(&ok);
        if (!list || !ok || i < 0 || i >= int(list->size())) return nullptr;
        at = (*list)[i].get();
        list = at->type() == ItemType::Group ? &static_cast<GroupItem *>(at)->children : nullptr;
    }
    return at;
}

// What an object carries into another presentation: its text, pictures, and movies are copied there, once each.
class Carry {
public:
    Carry(Document &to, const Document &from) : m_to(to), m_from(from) {}

    QString image(const QString &id)
    {
        if (id.isEmpty() || !m_from.images.contains(id)) return id;
        auto found = m_images.constFind(id);
        if (found == m_images.cend()) {
            const ImageData &data = m_from.images[id];
            found = m_images.insert(id, m_to.addImage(data.bytes, data.format));
        }
        return found.value();
    }
    QString media(const QString &id)
    {
        if (id.isEmpty() || !m_from.media.contains(id)) return id;
        auto found = m_media.constFind(id);
        if (found == m_media.cend()) {
            const MediaData &data = m_from.media[id];
            found = m_media.insert(id, m_to.addMedia(data.bytes, data.contentType, data.extension));
        }
        return found.value();
    }
    QString story(const QString &id)
    {
        if (id.isEmpty()) return id;
        const QString made = m_to.createStory();
        if (const QTextDocument *sd = m_from.storyDoc(id)) storyFromJson(m_to.storyDoc(made), storyToJson(sd));
        return made;
    }
    void fill(Fill &f) { f.imageId = image(f.imageId); }
    // A copy of `src` (a new id for it and its children) that stands in the working presentation.
    ItemPtr copy(const Item &src)
    {
        ItemPtr c = src.clone();
        ItemList one{c};
        walkItems(one, [&](const ItemPtr &it) { carry(it.get()); });
        return c;
    }

private:
    void carry(Item *it)
    {
        fill(it->fill);
        switch (it->type()) {
        case ItemType::Text: {
            auto *t = static_cast<TextItem *>(it);
            t->storyId = story(t->storyId);
            t->nextId.clear();
            break;
        }
        case ItemType::Shape: {
            auto *s = static_cast<ShapeItem *>(it);
            s->storyId = story(s->storyId);
            break;
        }
        case ItemType::Table:
            for (TableCell &cell : static_cast<TableItem *>(it)->cells) {
                cell.storyId = story(cell.storyId);
                cell.storyId2 = story(cell.storyId2);
                fill(cell.fill);
            }
            break;
        case ItemType::Picture: {
            auto *p = static_cast<PictureItem *>(it);
            p->imageId = image(p->imageId);
            p->mediaId = media(p->mediaId);
            for (PictureItem::Caption &c : p->mediaCaptions) c.mediaId = media(c.mediaId);
            break;
        }
        default: break;
        }
    }

    Document &m_to;
    const Document &m_from;
    QHash<QString, QString> m_images, m_media;
};

// The text of one story put into another (no story: nothing).
void copyStory(Document &to, const QString &toId, const Document &from, const QString &fromId)
{
    QTextDocument *dest = toId.isEmpty() ? nullptr : to.storyDoc(toId);
    if (!dest) return;
    const QTextDocument *src = fromId.isEmpty() ? nullptr : from.storyDoc(fromId);
    if (src) storyFromJson(dest, storyToJson(src));
    else setStoryText(dest, QString());
}

// Gives the working object `w` the property `path` of `s`, an object of `src`. False when it can't (the text of boxes that don't have the same stories).
bool takeProperty(Document &work, Item &w, const Document &src, const Item &s, const QString &path)
{
    if (path == QLatin1String("rect")) {
        w.rect = s.rect;
        w.rotation = s.rotation;
        w.flipH = s.flipH;
        w.flipV = s.flipV;
        return true;
    }
    if (path == QLatin1String("fill")) {
        w.fill = s.fill;
        Carry(work, src).fill(w.fill);
        return true;
    }
    if (path == QLatin1String("line")) {
        w.stroke = s.stroke;
        return true;
    }
    if (path == QLatin1String("text") || path == QLatin1String("font")) {
        const QStringList to = storyIdsOf(w), from = storyIdsOf(s);
        if (to.size() != from.size()) return false;
        for (int i = 0; i < to.size(); ++i) copyStory(work, to[i], src, from[i]);
        return true;
    }
    return false;
}

void removeSlideAt(Document &w, int index)
{
    w.pages.removeAt(index);
    slideRemoved(w, index);
}

}   // namespace

// ---- compareDecks ----

DeckComparison compareDecks(const Document &mine, const Document &theirs)
{
    DeckComparison out;
    out.theirs = &theirs;
    out.mine = Document::fromSnapshot(mine.snapshot());
    int usedM = 0, usedT = 0;
    const SideInfo sm = readSide(mine, &usedM);
    const SideInfo st = readSide(theirs, &usedT);
    out.slidesMine = sm.slides;
    out.slidesTheirs = st.slides;
    out.objectsMine = usedM;
    out.objectsTheirs = usedT;
    const bool slidesCapped = mine.pages.size() > kMaxCompareSlides || theirs.pages.size() > kMaxCompareSlides;
    bool cut = slidesCapped;
    for (const Flat &f : sm.flats) cut |= !f.complete;
    for (const Flat &f : st.flats) cut |= !f.complete;
    if (cut) out.changes.tooMany = true;
    const int nm = sm.slides, nt = st.slides;

    std::vector<int> mineOf = matchSlides(sm, st, out.work);
    std::vector<int> theirsOf(nm, -1);
    for (int t = 0; t < nt; ++t)
        if (mineOf[t] >= 0) theirsOf[mineOf[t]] = t;
    const std::vector<char> keeps = keepsOrder(mineOf);

    out.mineOfTheirs.resize(theirs.pages.size());
    out.theirsOfMine.resize(mine.pages.size());
    out.mineOfTheirs.fill(-1);
    out.theirsOfMine.fill(-1);
    out.workOfMine.resize(mine.pages.size());
    out.workOfTheirs.resize(theirs.pages.size());
    for (int m = 0; m < mine.pages.size(); ++m) out.workOfMine[m] = refOf(mine, m);
    for (int t = 0; t < nt; ++t) {
        out.mineOfTheirs[t] = mineOf[t];
        if (mineOf[t] >= 0) {
            out.theirsOfMine[mineOf[t]] = t;
            out.workOfTheirs[t] = out.workOfMine[mineOf[t]];
        }
    }

    const QString author = !theirs.props.lastModifiedBy.isEmpty() ? theirs.props.lastModifiedBy : theirs.props.author;
    const QDateTime when = theirs.props.modified.toUTC();
    bool full = false;
    auto emit = [&](Kind kind, const QString &container, const QString &object, const QString &path, const QString &summary, qint64 position, QJsonObject data, const QString &who = QString(),
                    const QDateTime &at = QDateTime()) {
        if (out.changes.changes.size() >= review::kMaxChanges) {
            out.changes.tooMany = true;
            full = true;
            return;
        }
        Change c;
        c.kind = kind;
        c.author = who.isEmpty() ? author : who;
        c.when = at.isValid() ? at : when;
        c.target = {container, object, path};
        c.summary = summary;
        c.position = position;
        c.data = data;
        out.changes.changes << c;
    };
    auto slideName = [&](const Document &d, int i) {
        const QString t = shortText(slideTitle(d, i));
        return t.isEmpty() ? QCoreApplication::translate("DeckCompare", "Slide %1").arg(i + 1) : t;
    };

    // Slides added and moved, in the order of theirs.
    for (int t = 0; t < nt && !full; ++t) {
        const int key = 2 * t + 2;
        const QString tid = theirs.pages[t]->id;
        if (mineOf[t] < 0) {
            QJsonObject data;
            data[QStringLiteral("ts")] = t;
            data[QStringLiteral("ms")] = -1;
            emit(Kind::Added, tid, QString(), QStringLiteral("slide"), QCoreApplication::translate("DeckCompare", "Slide added: %1").arg(slideName(theirs, t)), positionOf(key, 0), data);
        } else if (!keeps[t]) {
            QJsonObject data;
            data[QStringLiteral("ts")] = t;
            data[QStringLiteral("ms")] = mineOf[t];
            emit(Kind::Moved, mine.pages[mineOf[t]]->id, QString(), QStringLiteral("order"),
                 QCoreApplication::translate("DeckCompare", "Slide moved from %1 to %2: %3").arg(mineOf[t] + 1).arg(t + 1).arg(slideName(theirs, t)), positionOf(key, 0), data);
        }
    }
    // Slides deleted: mine that no slide of theirs matched, listed after the slide before them.
    int lastT = -1;
    for (int m = 0; m < nm && !full; ++m) {
        if (theirsOf[m] >= 0) {
            lastT = theirsOf[m];
            continue;
        }
        QJsonObject data;
        data[QStringLiteral("ts")] = -1;
        data[QStringLiteral("ms")] = m;
        emit(Kind::Deleted, mine.pages[m]->id, QString(), QStringLiteral("slide"), QCoreApplication::translate("DeckCompare", "Slide deleted: %1").arg(slideName(mine, m)), positionOf(2 * lastT + 3, 0), data);
    }

    // The matched slides: notes, comments, and objects.
    for (int t = 0; t < nt && !full; ++t) {
        const int m = mineOf[t];
        if (m < 0) continue;
        const Page &pm = *mine.pages[m];
        const Page &pt = *theirs.pages[t];
        const QString container = pm.id;
        const int key = 2 * t + 2;
        QJsonObject slideData;
        slideData[QStringLiteral("ts")] = t;
        slideData[QStringLiteral("ms")] = m;

        auto notes = [](const Document &d, const Page &p) {
            const QTextDocument *sd = p.notesStoryId.isEmpty() ? nullptr : d.storyDoc(p.notesStoryId);
            return sd ? sd->toPlainText() : QString();
        };
        const QString notesM = notes(mine, pm), notesT = notes(theirs, pt);
        if (notesM != notesT)
            emit(Kind::Edited, container, QString(), QStringLiteral("notes"), QCoreApplication::translate("DeckCompare", "Notes changed on %1").arg(slideName(theirs, t)), positionOf(key, 1), slideData);

        const QVector<comments::Comment> cm = comments::commentsOf(pm), ct = comments::commentsOf(pt);
        if (!ct.isEmpty()) {
            QSet<QString> have;
            for (const comments::Comment &c : cm) have.insert(commentKey(c));
            for (const comments::Comment &c : ct) {
                if (have.contains(commentKey(c))) continue;
                QJsonObject data = slideData;
                data[QStringLiteral("ckey")] = commentKey(c);
                data[QStringLiteral("cid")] = c.id;
                emit(Kind::Commented, container, c.id, QStringLiteral("comment"), QCoreApplication::translate("DeckCompare", "Comment by %1: %2").arg(c.author, shortText(c.text)), positionOf(key, 2), data, c.author, c.date);
            }
        }

        const Flat &fm = sm.flats[m];
        const Flat &ft = st.flats[t];
        if (!fm.complete || !ft.complete) continue;
        std::vector<int> itemMine(ft.recs.size(), -1);
        std::vector<char> usedM2(fm.recs.size(), 0);
        matchItems(fm, ft, fm.top, ft.top, itemMine, usedM2, out.work);
        // Objects of theirs: added when unmatched, else compared.
        for (int i = 0; i < int(ft.recs.size()) && !full; ++i) {
            const Rec &rt = ft.recs[i];
            // An object inside an object that was added comes with it.
            if (rt.parent >= 0 && itemMine[rt.parent] < 0) continue;
            const qint64 pos = positionOf(key, 3 + i);
            QJsonObject data = slideData;
            data[QStringLiteral("ti")] = rt.path;
            if (itemMine[i] < 0) {
                // The group it is in, in the working presentation: the matched object of mine.
                if (rt.parent >= 0) {
                    const Rec &pr = fm.recs[itemMine[rt.parent]];
                    data[QStringLiteral("pid")] = pr.item->id;
                    data[QStringLiteral("pord")] = pr.ord;
                }
                data[QStringLiteral("idx")] = rt.index;
                emit(Kind::Added, container, rt.item->id, QStringLiteral("object"), QCoreApplication::translate("DeckCompare", "Object added: %1").arg(describe(*rt.item, rt.text)), pos, data);
                continue;
            }
            const Rec &rm = fm.recs[itemMine[i]];
            const Item &a = *rm.item, &b = *rt.item;
            data[QStringLiteral("mi")] = rm.path;
            data[QStringLiteral("mid")] = a.id;
            data[QStringLiteral("mord")] = rm.ord;
            const QString what = describe(b, rt.text);
            if (!sameGeometry(a, b)) {
                const bool sized = std::abs(a.rect.width() - b.rect.width()) >= 0.01 || std::abs(a.rect.height() - b.rect.height()) >= 0.01;
                const bool moved = std::abs(a.rect.x() - b.rect.x()) >= 0.01 || std::abs(a.rect.y() - b.rect.y()) >= 0.01 || std::abs(a.rotation - b.rotation) >= 0.01 || a.flipH != b.flipH || a.flipV != b.flipV;
                const QString text = sized && moved ? QCoreApplication::translate("DeckCompare", "Moved and resized: %1").arg(what)
                                     : sized        ? QCoreApplication::translate("DeckCompare", "Resized: %1").arg(what)
                                                    : QCoreApplication::translate("DeckCompare", "Moved: %1").arg(what);
                emit(Kind::Moved, container, a.id, QStringLiteral("rect"), text, pos, data);
            }
            if (full) break;
            if (rm.text != rt.text)
                emit(Kind::Edited, container, a.id, QStringLiteral("text"), QCoreApplication::translate("DeckCompare", "Text changed from \"%1\" to \"%2\"").arg(shortText(rm.text), shortText(rt.text)), pos, data);
            else if (!storyIdsOf(b).isEmpty() && storySignature(mine, a) != storySignature(theirs, b))
                emit(Kind::Reformatted, container, a.id, QStringLiteral("font"), QCoreApplication::translate("DeckCompare", "Text formatting changed: %1").arg(what), pos, data);
            if (full) break;
            if (!(a.fill == b.fill)) emit(Kind::Reformatted, container, a.id, QStringLiteral("fill"), QCoreApplication::translate("DeckCompare", "Fill changed: %1").arg(what), pos, data);
            if (full) break;
            if (!(a.stroke == b.stroke)) emit(Kind::Reformatted, container, a.id, QStringLiteral("line"), QCoreApplication::translate("DeckCompare", "Line changed: %1").arg(what), pos, data);
        }
        // Objects of mine that no object of theirs matched.
        for (int i = 0; i < int(fm.recs.size()) && !full; ++i) {
            if (usedM2[i]) continue;
            const Rec &rm = fm.recs[i];
            if (rm.parent >= 0 && !usedM2[rm.parent]) continue;
            QJsonObject data = slideData;
            data[QStringLiteral("mi")] = rm.path;
            data[QStringLiteral("mid")] = rm.item->id;
            data[QStringLiteral("mord")] = rm.ord;
            if (rm.parent >= 0) {
                data[QStringLiteral("pid")] = fm.recs[rm.parent].item->id;
                data[QStringLiteral("pord")] = fm.recs[rm.parent].ord;
            }
            data[QStringLiteral("idx")] = rm.index;
            emit(Kind::Deleted, container, rm.item->id, QStringLiteral("object"), QCoreApplication::translate("DeckCompare", "Object deleted: %1").arg(describe(*rm.item, rm.text)), positionOf(key, 3 + i), data);
        }
    }

    std::stable_sort(out.changes.changes.begin(), out.changes.changes.end(), [](const Change &a, const Change &b) { return a.position < b.position; });
    for (int i = 0; i < out.changes.changes.size(); ++i) out.changes.changes[i].id = QStringLiteral("c%1").arg(i + 1);
    return out;
}

// ---- accepting and rejecting ----

namespace {

int dataInt(const Change &c, const char *key, int fallback = -1)
{
    const auto v = c.data.value(QLatin1String(key));
    return v.isDouble() ? v.toInt() : fallback;
}
QString dataString(const Change &c, const char *key) { return c.data.value(QLatin1String(key)).toString(); }

// Takes one side's version of change `ch` into `w`. `accept`: theirs, else mine. Safe to repeat.
bool take(Document &w, DeckComparison &cmp, Change &ch, bool accept)
{
    const Document &src = accept ? *cmp.theirs : *cmp.mine;
    QVector<CompareRef> &refs = accept ? cmp.workOfTheirs : cmp.workOfMine;
    const int ms = dataInt(ch, "ms"), ts = dataInt(ch, "ts");
    const int srcSlide = accept ? ts : ms;
    const QString path = ch.target.path;

    if (path == QLatin1String("slide")) {
        // An added slide (only in theirs) or a deleted one (only in mine): the side that has it is the one that keeps it.
        const bool inTheirs = ts >= 0;
        QVector<CompareRef> &mine = inTheirs ? cmp.workOfTheirs : cmp.workOfMine;
        const int own = inTheirs ? ts : ms;
        if (own < 0 || own >= mine.size()) return false;
        const bool wanted = inTheirs == accept;   // the slide should be in the working presentation
        const int at = findSlide(w, mine[own]);
        if (wanted) {
            if (at >= 0) return true;
            const Document &from = inTheirs ? *cmp.theirs : *cmp.mine;
            const int index = insertSlideFrom(w, placeAfter(w, mine, own), from, own, true);
            if (index < 0) return false;
            // The slide comes back under the id it had, so that what refers to it by id (and an undo) still finds it.
            const QString want = from.pages[own]->id;
            if (!want.isEmpty() && findSlide(w, CompareRef{want, 0}) < 0) w.pages[index]->id = want;
            mine[own] = refOf(w, index);
            return true;
        }
        if (at < 0) return true;
        if (w.pages.size() < 2) return false;
        removeSlideAt(w, at);
        return true;
    }
    if (path == QLatin1String("order")) {
        if (ms < 0 || ts < 0 || ms >= cmp.workOfMine.size()) return false;
        const int idx = findSlide(w, cmp.workOfMine[ms]);
        if (idx < 0) return false;
        const int k = placeAfter(w, refs, srcSlide) - 1;   // the slide it follows, -1 for the first place
        if (k + 1 == idx) return true;
        const std::shared_ptr<Page> page = w.pages[idx];
        const int at = k < idx ? k + 1 : k;
        removeSlideAt(w, idx);
        w.pages.insert(at, page);
        slideInserted(w, at);
        return true;
    }

    // Everything else is on a slide that both have.
    if (ms < 0 || ms >= cmp.workOfMine.size()) return false;
    const int sl = findSlide(w, cmp.workOfMine[ms]);
    if (sl < 0) return false;
    Page &page = *w.pages[sl];
    const Page &from = *src.pages[srcSlide];

    if (path == QLatin1String("notes")) {
        if (page.notesStoryId.isEmpty()) page.notesStoryId = w.createStory();
        copyStory(w, page.notesStoryId, src, from.notesStoryId);
        return true;
    }
    if (path == QLatin1String("comment")) {
        const QString key = dataString(ch, "ckey");
        QVector<comments::Comment> list = comments::commentsOf(page);
        int at = -1;
        for (int i = 0; i < list.size(); ++i)
            if (commentKey(list[i]) == key) { at = i; break; }
        if (accept) {
            if (at >= 0) return true;
            for (const comments::Comment &c : comments::commentsOf(*cmp.theirs->pages[ts])) {
                if (commentKey(c) != key) continue;
                comments::Comment copy = c;
                for (const comments::Comment &have : list)
                    if (have.id == copy.id) { copy.id = newId("c"); break; }
                list << copy;
                comments::setComments(page, list);
                return true;
            }
            return false;
        }
        if (at < 0) return true;
        list.removeAt(at);
        comments::setComments(page, list);
        return true;
    }

    // Objects.
    const QString mid = dataString(ch, "mid");
    const int mord = dataInt(ch, "mord", 0);
    const QString wid = dataString(ch, "wid");
    auto parentList = [&]() -> ItemList * {
        const QString pid = dataString(ch, "pid");
        if (pid.isEmpty()) return &page.items;
        const Spot parent = findItem(w, sl, pid, dataInt(ch, "pord", 0));
        if (!parent.item || parent.item->type() != ItemType::Group) return nullptr;
        return &static_cast<GroupItem *>(parent.item)->children;
    };
    if (path == QLatin1String("object")) {
        const bool added = ch.kind == Kind::Added;
        const bool wanted = added == accept;   // the object should be in the working presentation
        Spot here;
        if (!wid.isEmpty()) here = findItem(w, sl, wid, 0);
        if (!here.item && !added) here = findItem(w, sl, mid, mord);
        if (wanted) {
            if (here.item) return true;
            ItemList *list = parentList();
            if (!list) return false;
            const QString ipath = dataString(ch, added ? "ti" : "mi");
            const Item *srcItem = itemAtPath(from.items, ipath);
            if (!srcItem) return false;
            ItemPtr made = Carry(w, src).copy(*srcItem);
            if (!srcItem->id.isEmpty() && !w.find(srcItem->id).item) made->id = srcItem->id;   // the id it had, if free
            const int at = std::clamp(dataInt(ch, "idx", int(list->size())), 0, int(list->size()));
            list->insert(list->begin() + at, made);
            ch.data[QStringLiteral("wid")] = made->id;
            return true;
        }
        if (!here.item) return true;
        here.list->erase(here.list->begin() + here.index);
        return true;
    }
    // A property of an object both have.
    const Spot spot = findItem(w, sl, mid, mord);
    const QString ipath = dataString(ch, accept ? "ti" : "mi");
    const Item *srcItem = itemAtPath(from.items, ipath);
    if (!spot.item || !srcItem) return false;
    return takeProperty(w, *spot.item, src, *srcItem, path);
}

bool phaseOfSlide(const Change &c) { return c.target.path == QLatin1String("slide") || c.target.path == QLatin1String("order"); }

}   // namespace

bool applyChange(Document &working, DeckComparison &cmp, int index)
{
    if (index < 0 || index >= cmp.changes.changes.size()) return false;
    Change &ch = cmp.changes.changes[index];
    if (!take(working, cmp, ch, true)) return false;
    ch.state = State::Accepted;
    return true;
}

bool rejectChange(Document &working, DeckComparison &cmp, int index)
{
    if (index < 0 || index >= cmp.changes.changes.size()) return false;
    Change &ch = cmp.changes.changes[index];
    if (ch.state != State::Pending && !take(working, cmp, ch, false)) return false;
    ch.state = State::Rejected;
    return true;
}

int acceptAll(Document &working, DeckComparison &cmp)
{
    // Slides added and moved in the order of theirs, the others, then slides deleted (the presentation never has to be empty on the way).
    std::vector<int> order;
    for (int i = 0; i < cmp.changes.changes.size(); ++i)
        if (cmp.changes.changes[i].state == State::Pending) order.push_back(i);
    auto rank = [&](int i) {
        const Change &c = cmp.changes.changes[i];
        if (phaseOfSlide(c)) return c.kind == Kind::Deleted ? 2 : 0;
        return 1;
    };
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        const int ra = rank(a), rb = rank(b);
        if (ra != rb) return ra < rb;
        return ra == 0 && dataInt(cmp.changes.changes[a], "ts") < dataInt(cmp.changes.changes[b], "ts");
    });
    int n = 0;
    for (int i : order) n += applyChange(working, cmp, i);
    return n;
}

int rejectAll(Document &working, DeckComparison &cmp, bool alsoAccepted)
{
    // Slides added go first, then slides deleted and moved in the order of mine, then the others.
    std::vector<int> order;
    for (int i = 0; i < cmp.changes.changes.size(); ++i) {
        const State s = cmp.changes.changes[i].state;
        if (s == State::Pending || (alsoAccepted && s == State::Accepted)) order.push_back(i);
    }
    auto rank = [&](int i) {
        const Change &c = cmp.changes.changes[i];
        if (phaseOfSlide(c)) return c.kind == Kind::Added ? 0 : 1;
        return 2;
    };
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        const int ra = rank(a), rb = rank(b);
        if (ra != rb) return ra < rb;
        return ra == 1 && dataInt(cmp.changes.changes[a], "ms") < dataInt(cmp.changes.changes[b], "ms");
    });
    int n = 0;
    for (int i : order) n += rejectChange(working, cmp, i);
    return n;
}

}   // namespace jp::deck
