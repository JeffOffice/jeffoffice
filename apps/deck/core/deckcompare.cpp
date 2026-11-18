#include "core/deckcompare.h"

#include "core/items.h"
#include "core/comments.h"
#include "core/reuse.h"
#include "core/sections.h"
#include "text/storyio.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QHash>
#include <QJsonDocument>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextList>
#include <algorithm>
#include <cmath>
#include <functional>
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
    QString text;        // the text of all its stories, one separator between stories
    QString path;        // its place from the top of the slide: the indices of the lists, dotted
    QVector<int> kids;
};

struct Flat {
    QVector<Rec> recs;
    QVector<int> top;
    bool complete = true;    // every object of the slide was read
};

// What a side may still read: objects (a table's cells count as objects) and characters of text, in all.
struct Budget {
    int objects = kMaxCompareObjects;
    qint64 text = kMaxTotalText;
};

// The first characters of a story as plain text, at most kMaxItemText and no more than what is left of the budget (then `cut` is set and nothing is read).
// A selection of that many characters is made, not the whole text and then cut.
QString storyText(const Document &d, const QString &id, Budget &b, bool &cut)
{
    const QTextDocument *sd = id.isEmpty() ? nullptr : d.storyDoc(id);
    if (!sd) return QString();
    const qint64 n = std::min<qint64>(sd->characterCount() - 1, kMaxItemText);
    if (n <= 0) return QString();
    if (n > b.text) {
        cut = true;
        return QString();
    }
    b.text -= n;
    QTextCursor c(const_cast<QTextDocument *>(sd));
    c.setPosition(0);
    c.setPosition(int(n), QTextCursor::KeepAnchor);
    return c.selection().toPlainText();
}

QString textOf(const Document &d, const Item &it, Budget &b, bool &cut)
{
    switch (it.type()) {
    case ItemType::Text: return storyText(d, static_cast<const TextItem &>(it).storyId, b, cut);
    case ItemType::Shape: return storyText(d, static_cast<const ShapeItem &>(it).storyId, b, cut);
    case ItemType::Table: {
        QString out;
        for (const TableCell &c : static_cast<const TableItem &>(it).cells) {
            if (out.size() >= kMaxItemText || cut) break;
            out += storyText(d, c.storyId, b, cut) + QChar(0x1f) + storyText(d, c.storyId2, b, cut) + QChar(0x1f);
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

// A hash of the text of an object with its formatting, for telling a change of look from none. It looks at the first kMaxSigRuns paragraphs and runs and
// the first kMaxItemText characters of the object, and what it looked at is added to `runs`; false when `runs` would pass `limit` (nothing is hashed then).
// A format is turned into JSON once for each distinct one of the story, not for each run.
bool storySignature(const Document &d, const Item &it, qint64 &runs, qint64 limit, QByteArray *out)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    qint64 mine = 0;
    qsizetype chars = 0;
    auto json = [](const QTextFormat &f) { return QJsonDocument(formatToJson(f)).toJson(QJsonDocument::Compact); };
    for (const QString &id : storyIdsOf(it)) {
        hash.addData(QByteArrayView("\x1f"));
        const QTextDocument *sd = id.isEmpty() ? nullptr : d.storyDoc(id);
        if (!sd) continue;
        QHash<int, QByteArray> formats;   // by the format's place in the document's collection
        QHash<const QTextList *, QByteArray> lists;
        auto cached = [&](int index, const QTextFormat &f) -> const QByteArray & {
            auto at = formats.find(index);
            if (at == formats.end()) at = formats.insert(index, json(f));
            return at.value();
        };
        for (QTextBlock b = sd->begin(); b.isValid() && mine < kMaxSigRuns && chars < kMaxItemText; b = b.next()) {
            ++mine;
            hash.addData(QByteArrayView("\x1e"));
            hash.addData(cached(b.blockFormatIndex(), b.blockFormat()));
            hash.addData(cached(b.charFormatIndex(), b.charFormat()));
            if (QTextList *l = b.textList()) {
                auto at = lists.find(l);
                if (at == lists.end()) at = lists.insert(l, json(l->format()));
                hash.addData(at.value());
            }
            for (auto f = b.begin(); !f.atEnd() && mine < kMaxSigRuns && chars < kMaxItemText; ++f) {
                const QTextFragment frag = f.fragment();
                if (!frag.isValid()) continue;
                ++mine;
                const QString t = frag.text().left(kMaxItemText - chars);
                chars += t.size();
                hash.addData(QByteArrayView(reinterpret_cast<const char *>(t.constData()), t.size() * 2));
                hash.addData(cached(frag.charFormatIndex(), frag.charFormat()));
            }
        }
    }
    if (runs + mine > limit) return false;
    runs += mine;
    *out = hash.result();
    return true;
}

void flatten(const Document &d, const ItemList &list, int parent, int depth, const QString &prefix, Flat &flat, Budget &budget)
{
    for (int i = 0; i < int(list.size()); ++i) {
        if (budget.objects <= 0) {
            flat.complete = false;
            return;
        }
        --budget.objects;
        const Item *it = list[i].get();
        // A table's cells are objects too: counted before any of them is read.
        if (it->type() == ItemType::Table) {
            const int cells = int(static_cast<const TableItem *>(it)->cells.size());
            if (cells > budget.objects) {
                budget.objects = 0;
                flat.complete = false;
                return;
            }
            budget.objects -= cells;
        }
        Rec r;
        r.item = it;
        r.parent = parent;
        r.index = i;
        bool cut = false;
        r.text = textOf(d, *it, budget, cut);
        if (cut) {
            flat.complete = false;
            return;
        }
        r.path = prefix.isEmpty() ? QString::number(i) : prefix + QLatin1Char('.') + QString::number(i);
        const int at = int(flat.recs.size());
        flat.recs << r;
        if (parent < 0) flat.top << at;
        else flat.recs[parent].kids << at;
        if (it->type() == ItemType::Group) {
            const ItemList &kids = static_cast<const GroupItem *>(it)->children;
            if (kids.empty()) continue;
            if (depth < kMaxDepth) flatten(d, kids, at, depth + 1, r.path, flat, budget);
            else flat.complete = false;   // a group deeper than that is not read
        }
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

SideInfo readSide(const Document &d, int *objectsUsed, qint64 *textUsed)
{
    SideInfo s;
    s.doc = &d;
    s.slides = std::min(int(d.pages.size()), kMaxCompareSlides);
    Budget budget;
    s.flats.resize(s.slides);
    s.titles.resize(s.slides);
    s.keys.resize(s.slides);
    s.tokens.resize(s.slides);
    for (int i = 0; i < s.slides; ++i) {
        Flat &f = s.flats[i];
        flatten(d, d.pages[i]->items, -1, 0, QString(), f, budget);
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
    *objectsUsed = kMaxCompareObjects - budget.objects;
    *textUsed = kMaxTotalText - budget.text;
    s.objects = *objectsUsed;
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

// What a picture is, for telling two loads of one file apart from two pictures: a hash of its bytes, made once for each id of a presentation.
struct ImageHashes {
    const Document *doc = nullptr;
    QHash<QString, QString> cache;
    const QString &of(const QString &id)
    {
        auto at = cache.find(id);
        if (at == cache.end()) {
            QString h;
            if (!id.isEmpty()) {
                const auto image = doc->images.constFind(id);
                h = image == doc->images.cend() ? QStringLiteral("none") : QString::fromLatin1(QCryptographicHash::hash(image->bytes, QCryptographicHash::Sha1).toHex());
            }
            at = cache.insert(id, h);
        }
        return at.value();
    }
};

bool sameFill(const Fill &a, ImageHashes &ha, const Fill &b, ImageHashes &hb)
{
    if (a.imageId.isEmpty() && b.imageId.isEmpty()) return a == b;
    Fill x = a, y = b;
    x.imageId = ha.of(a.imageId);
    y.imageId = hb.of(b.imageId);
    return x == y;
}

// Two tables that do not have the same rows, columns, and cells: a text change can't be taken from one into the other, so one replaces the other.
bool tableShapeDiffers(const Item &a, const Item &b)
{
    if (a.type() != ItemType::Table || b.type() != ItemType::Table) return false;
    const auto &x = static_cast<const TableItem &>(a), &y = static_cast<const TableItem &>(b);
    return x.rows != y.rows || x.cols != y.cols || x.cells.size() != y.cells.size();
}

// ---- applying ----

QString parentPathOf(const QString &path)
{
    const qsizetype i = path.lastIndexOf(QLatin1Char('.'));
    return i < 0 ? QString() : path.left(i);
}
int lastIndexOf(const QString &path)
{
    const qsizetype i = path.lastIndexOf(QLatin1Char('.'));
    return path.mid(i + 1).toInt();
}
QString childPath(const QString &parent, int k) { return parent.isEmpty() ? QString::number(k) : parent + QLatin1Char('.') + QString::number(k); }

// Where the slide with this id is in the working presentation, or -1. The places are a hash made when needed (and again when a lookup of a slide that
// should be there fails, as after slides came or went), so a lookup is a step and not a walk.
int findSlide(const Document &w, DeckComparison &cmp, const CompareRef &r)
{
    if (!r.valid()) return -1;
    CompareWork &st = cmp.state;
    auto hit = [&]() {
        const auto at = st.slideAt.constFind(r.id);
        return at != st.slideAt.cend() && *at < w.pages.size() && w.pages[*at]->id == r.id ? *at : -1;
    };
    const int i = hit();
    if (i >= 0 || st.slideFresh) return i;
    st.slideAt.clear();
    st.slideAt.reserve(w.pages.size());
    for (int k = 0; k < w.pages.size(); ++k)
        if (!st.slideAt.contains(w.pages[k]->id)) st.slideAt.insert(w.pages[k]->id, k);
    cmp.work += w.pages.size();
    st.slideFresh = true;
    return hit();
}

// Just after the slide before slide `src` of a side that is in the working presentation (the nearest such one), else the first place.
int placeAfter(const Document &w, DeckComparison &cmp, const QVector<CompareRef> &refs, int src)
{
    for (int j = std::min(src, int(refs.size())) - 1; j >= 0; --j) {
        ++cmp.work;
        const int k = findSlide(w, cmp, refs[j]);
        if (k >= 0) return k + 1;
    }
    return 0;
}

struct Spot {
    ItemList *list = nullptr;
    int index = -1;
    Item *item = nullptr;
};

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


// Gives each object of the working presentation that shares its id (or has none) the id its place has in the copy of mine, which has none repeated (or
// a new one when mine has nothing at that place), and
// each slide that shares an id one of its own. The working presentation is taken to be mine as it was compared: slide m is slide m, and an object is where
// its path says. After that an id is an address. Done by the first accept or reject, and again if an undo took the ids away.
void stampItems(ItemList &wl, const ItemList &sl, QSet<QString> &seen, int page, DeckComparison &cmp)
{
    static const ItemList none;
    CompareWork &st = cmp.state;
    for (int i = 0; i < int(wl.size()); ++i) {
        ++cmp.work;
        Item &wi = *wl[i];
        const Item *si = i < int(sl.size()) && sl[i]->type() == wi.type() ? sl[i].get() : nullptr;
        if (si && (wi.id.isEmpty() || seen.contains(wi.id)) && !si->id.isEmpty() && si->id != wi.id) {
            wi.id = si->id;
            if (st.sampleSlide < 0) {
                st.sampleSlide = page;
                st.sampleId = wi.id;
            }
        }
        // One with no counterpart in mine (the presentation was edited since) is made its own too: every object of the working presentation is an address.
        if (wi.id.isEmpty() || seen.contains(wi.id)) wi.id = newId();
        seen.insert(wi.id);
        if (wi.type() == ItemType::Group) stampItems(static_cast<GroupItem &>(wi).children, si ? static_cast<const GroupItem *>(si)->children : none, seen, page, cmp);
    }
}

void bindWorking(Document &w, DeckComparison &cmp)
{
    CompareWork &st = cmp.state;
    if (st.bound) {
        // (the stamps are still there if the slide and the object that was stamped first are: an undo takes them all away at once)
        bool ok = true;
        if (st.samplePage >= 0) ok = findSlide(w, cmp, CompareRef{st.samplePageId}) >= 0;
        if (ok && st.sampleSlide >= 0) {
            const Document::Loc loc = w.find(st.sampleId);
            ok = loc.item && loc.item->id == st.sampleId;
        }
        if (ok) return;
    }
    st.bound = true;
    st.samplePage = st.sampleSlide = -1;
    const int nm = int(cmp.workOfMine.size());
    if (st.pageStamp.size() < nm) st.pageStamp.resize(nm);
    QSet<QString> seenPages, seenItems;
    for (int m = 0; m < w.pages.size(); ++m) {
        Page &p = *w.pages[m];
        const bool compared = m < nm && m < cmp.mine->pages.size();
        if (compared && (p.id.isEmpty() || seenPages.contains(p.id))) {
            if (st.pageStamp[m].isEmpty()) st.pageStamp[m] = newId("p");
            p.id = st.pageStamp[m];
            cmp.workOfMine[m] = CompareRef{p.id};
            const int t = m < cmp.theirsOfMine.size() ? cmp.theirsOfMine[m] : -1;
            if (t >= 0 && t < cmp.workOfTheirs.size()) cmp.workOfTheirs[t] = cmp.workOfMine[m];
            if (st.samplePage < 0) {
                st.samplePage = m;
                st.samplePageId = p.id;
            }
        }
        seenPages.insert(p.id);
        if (compared) {
            stampItems(p.items, cmp.mine->pages[m]->items, seenItems, m, cmp);
        } else {
            walkItems(p.items, [&](const ItemPtr &it) { seenItems.insert(it->id); ++cmp.work; });
        }
    }
    st.ids = seenItems;
    st.slideFresh = false;
}

// Where the object with this id is in `list`: its last known place if it is still there, else a look along the list. -1 when it is not in it.
int indexInList(ItemList &list, const QString &id, DeckComparison &cmp)
{
    auto &hint = cmp.state.hint;
    const auto at = hint.constFind(id);
    if (at != hint.cend() && *at >= 0 && *at < int(list.size()) && list[*at]->id == id) return *at;
    for (int i = 0; i < int(list.size()); ++i) {
        ++cmp.work;
        if (list[i]->id == id) {
            hint[id] = i;
            return i;
        }
    }
    return -1;
}

// Where an object goes in `list` (its side's list stood at `path` on slide `slide` of that side): just after the nearest object before it that is in the
// working list, else first. From theirs the objects before it are known by the table of what is who; from mine, by the copy of mine.
int placeFor(ItemList &list, const QString &path, bool fromTheirs, int slide, DeckComparison &cmp)
{
    const QString parent = parentPathOf(path);
    for (int k = lastIndexOf(path) - 1; k >= 0; --k) {
        ++cmp.work;
        const QString sibling = childPath(parent, k);
        QString id;
        if (fromTheirs) {
            id = cmp.state.theirsWork.value(QString::number(slide) + QLatin1Char('/') + sibling);
        } else if (slide >= 0 && slide < cmp.mine->pages.size()) {
            if (const Item *it = itemAtPath(cmp.mine->pages[slide]->items, sibling)) id = it->id;
        }
        if (id.isEmpty() || !cmp.state.ids.contains(id)) continue;
        const int at = indexInList(list, id, cmp);
        if (at >= 0) return at + 1;
    }
    return 0;
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
    out.theirs = Document::fromSnapshot(theirs.snapshot());
    out.mine = Document::fromSnapshot(mine.snapshot());
    // The copy of mine has no id repeated (the later ones are given new ones); an object with none gets one. The objects of the working presentation are known by these.
    for (auto &p : out.mine->pages)
        walkItems(p->items, [](const ItemPtr &it) {
            if (it->id.isEmpty()) it->id = newId();
        });
    int usedM = 0, usedT = 0;
    qint64 textM = 0, textT = 0;
    const SideInfo sm = readSide(mine, &usedM, &textM);
    const SideInfo st = readSide(theirs, &usedT, &textT);
    out.slidesMine = sm.slides;
    out.slidesTheirs = st.slides;
    out.objectsMine = usedM;
    out.objectsTheirs = usedT;
    out.textMine = textM;
    out.textTheirs = textT;
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

    // Only the slides that were read (at most kMaxCompareSlides of each) have places in these.
    out.mineOfTheirs = QVector<int>(nt, -1);
    out.theirsOfMine = QVector<int>(nm, -1);
    out.workOfMine.resize(nm);
    out.workOfTheirs.resize(nt);
    for (int m = 0; m < nm; ++m) {
        out.workOfMine[m] = CompareRef{mine.pages[m]->id};
        ++out.work;
    }
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
    // What the changed content is, for changeKey: a short hash, never words of the interface.
    auto ck = [](const QByteArray &content) { return QString::fromLatin1(QCryptographicHash::hash(content, QCryptographicHash::Sha1).toHex().left(16)); };
    auto slideName = [&](const Document &d, int i) {
        const QString t = shortText(slideTitle(d, i));
        return t.isEmpty() ? QCoreApplication::translate("DeckCompare", "Slide %1").arg(i + 1) : t;
    };
    // The id an object of mine has in the working presentation (the copy of mine has no id repeated).
    auto snapId = [&](int m, const QString &path) {
        const Item *it = itemAtPath(out.mine->pages[m]->items, path);
        return it ? it->id : QString();
    };
    ImageHashes imagesMine{&mine, {}}, imagesTheirs{&theirs, {}};
    bool sigCut = false;
    // Whether the text of two objects (that is the same) is formatted differently; the runs looked at are counted, at most kMaxTotalSigRuns of each side in all.
    auto formatDiffers = [&](const Item &a, const Item &b) {
        if (sigCut) return false;
        qint64 runsM = out.sigRunsMine, runsT = out.sigRunsTheirs;
        QByteArray ha, hb;
        if (!storySignature(mine, a, runsM, kMaxTotalSigRuns, &ha) || !storySignature(theirs, b, runsT, kMaxTotalSigRuns, &hb)) {
            sigCut = true;
            out.changes.tooMany = true;
            return false;
        }
        out.sigRunsMine = runsM;
        out.sigRunsTheirs = runsT;
        return ha != hb;
    };

    // Slides added and moved, in the order of theirs.
    for (int t = 0; t < nt && !full; ++t) {
        const int key = 2 * t + 2;
        const QString tid = theirs.pages[t]->id;
        if (mineOf[t] < 0) {
            QJsonObject data;
            data[QStringLiteral("ts")] = t;
            data[QStringLiteral("ms")] = -1;
            data[QStringLiteral("ck")] = ck(slideTitle(theirs, t).toUtf8());
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
        data[QStringLiteral("ck")] = ck(slideTitle(mine, m).toUtf8() + '|' + QByteArray::number(int(mine.pages[m]->items.size())));
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
        if (notesM != notesT) {
            QJsonObject nd = slideData;
            nd[QStringLiteral("ck")] = ck(notesT.toUtf8());
            emit(Kind::Edited, container, QString(), QStringLiteral("notes"), QCoreApplication::translate("DeckCompare", "Notes changed on %1").arg(slideName(theirs, t)), positionOf(key, 1), nd);
        }

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
        // A table that does not have the shape of the one it matched is replaced: the old one is deleted and the new one added.
        for (int i = 0; i < int(ft.recs.size()); ++i) {
            const int j = itemMine[i];
            if (j >= 0 && tableShapeDiffers(*fm.recs[j].item, *ft.recs[i].item)) {
                itemMine[i] = -1;
                usedM2[j] = 0;
            }
        }
        // Objects of theirs: added when unmatched, else compared.
        for (int i = 0; i < int(ft.recs.size()) && !full; ++i) {
            const Rec &rt = ft.recs[i];
            // An object inside an object that was added comes with it.
            if (rt.parent >= 0 && itemMine[rt.parent] < 0) continue;
            const qint64 pos = positionOf(key, 3 + i);
            QJsonObject data = slideData;
            data[QStringLiteral("ti")] = rt.path;
            if (itemMine[i] < 0) {
                data[QStringLiteral("ck")] = ck(rt.text.toUtf8());
                // The group it is in, in the working presentation: the matched object of mine.
                if (rt.parent >= 0) data[QStringLiteral("pid")] = snapId(m, fm.recs[itemMine[rt.parent]].path);
                emit(Kind::Added, container, rt.item->id, QStringLiteral("object"), QCoreApplication::translate("DeckCompare", "Object added: %1").arg(describe(*rt.item, rt.text)), pos, data);
                continue;
            }
            const Rec &rm = fm.recs[itemMine[i]];
            const Item &a = *rm.item, &b = *rt.item;
            const QString mid = snapId(m, rm.path);
            data[QStringLiteral("mi")] = rm.path;
            data[QStringLiteral("mid")] = mid;
            // Who is who, for placing the objects that come in beside them (see placeFor).
            out.state.theirsWork.insert(QString::number(t) + QLatin1Char('/') + rt.path, mid);
            out.state.hint.insert(mid, rm.index);
            const QString what = describe(b, rt.text);
            auto withCk = [&](const QByteArray &content) {
                QJsonObject d = data;
                d[QStringLiteral("ck")] = ck(content);
                return d;
            };
            if (!sameGeometry(a, b)) {
                const bool sized = std::abs(a.rect.width() - b.rect.width()) >= 0.01 || std::abs(a.rect.height() - b.rect.height()) >= 0.01;
                const bool moved = std::abs(a.rect.x() - b.rect.x()) >= 0.01 || std::abs(a.rect.y() - b.rect.y()) >= 0.01 || std::abs(a.rotation - b.rotation) >= 0.01 || a.flipH != b.flipH || a.flipV != b.flipV;
                const QString text = sized && moved ? QCoreApplication::translate("DeckCompare", "Moved and resized: %1").arg(what)
                                     : sized        ? QCoreApplication::translate("DeckCompare", "Resized: %1").arg(what)
                                                    : QCoreApplication::translate("DeckCompare", "Moved: %1").arg(what);
                emit(Kind::Moved, container, a.id, QStringLiteral("rect"), text, pos,
                     withCk(QByteArray::number(b.rect.x(), 'f', 2) + ',' + QByteArray::number(b.rect.y(), 'f', 2) + ',' + QByteArray::number(b.rect.width(), 'f', 2) + ',' +
                            QByteArray::number(b.rect.height(), 'f', 2) + ',' + QByteArray::number(b.rotation, 'f', 2) + (b.flipH ? 'h' : '-') + (b.flipV ? 'v' : '-')));
            }
            if (full) break;
            if (rm.text != rt.text)
                emit(Kind::Edited, container, a.id, QStringLiteral("text"), QCoreApplication::translate("DeckCompare", "Text changed from \"%1\" to \"%2\"").arg(shortText(rm.text), shortText(rt.text)), pos, withCk(rt.text.toUtf8()));
            else if (!storyIdsOf(b).isEmpty() && formatDiffers(a, b))
                emit(Kind::Reformatted, container, a.id, QStringLiteral("font"), QCoreApplication::translate("DeckCompare", "Text formatting changed: %1").arg(what), pos, withCk(rt.text.toUtf8()));
            if (full) break;
            if (!sameFill(a.fill, imagesMine, b.fill, imagesTheirs)) emit(Kind::Reformatted, container, a.id, QStringLiteral("fill"), QCoreApplication::translate("DeckCompare", "Fill changed: %1").arg(what), pos, withCk([&] {
                    Fill f = b.fill;
                    f.imageId = imagesTheirs.of(b.fill.imageId);
                    return QJsonDocument(f.toJson()).toJson(QJsonDocument::Compact);
                }()));
            if (full) break;
            if (!(a.stroke == b.stroke)) emit(Kind::Reformatted, container, a.id, QStringLiteral("line"), QCoreApplication::translate("DeckCompare", "Line changed: %1").arg(what), pos, withCk(QJsonDocument(b.stroke.toJson()).toJson(QJsonDocument::Compact)));
        }
        // Objects of mine that no object of theirs matched.
        for (int i = 0; i < int(fm.recs.size()) && !full; ++i) {
            if (usedM2[i]) continue;
            const Rec &rm = fm.recs[i];
            if (rm.parent >= 0 && !usedM2[rm.parent]) continue;
            QJsonObject data = slideData;
            const QString mid = snapId(m, rm.path);
            data[QStringLiteral("mi")] = rm.path;
            data[QStringLiteral("mid")] = mid;
            if (rm.parent >= 0) data[QStringLiteral("pid")] = snapId(m, fm.recs[rm.parent].path);
            data[QStringLiteral("ck")] = ck(rm.text.toUtf8() + '|' + QByteArray::number(int(rm.item->type())) + '|' + QByteArray::number(rm.item->rect.x(), 'f', 2) + ',' +
                                            QByteArray::number(rm.item->rect.y(), 'f', 2));
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
    CompareWork &st = cmp.state;
    st.slideFresh = false;   // (the slides may have changed since the last one)
    bindWorking(w, cmp);
    const Document &src = accept ? *cmp.theirs : *cmp.mine;
    QVector<CompareRef> &refs = accept ? cmp.workOfTheirs : cmp.workOfMine;
    const int ms = dataInt(ch, "ms"), ts = dataInt(ch, "ts");
    const int srcSlide = accept ? ts : ms;
    const QString path = ch.target.path;

    if (path == QLatin1String("slide")) {
        // An added slide (only in theirs) or a deleted one (only in mine): the side that has it is the one that keeps it.
        const bool inTheirs = ts >= 0;
        QVector<CompareRef> &own = inTheirs ? cmp.workOfTheirs : cmp.workOfMine;
        const int n = inTheirs ? ts : ms;
        if (n < 0 || n >= own.size()) return false;
        const bool wanted = inTheirs == accept;   // the slide should be in the working presentation
        const int at = findSlide(w, cmp, own[n]);
        if (wanted) {
            if (at >= 0) return true;
            const Document &from = inTheirs ? *cmp.theirs : *cmp.mine;
            const int index = insertSlideFrom(w, placeAfter(w, cmp, own, n), from, n, true);
            if (index < 0) return false;
            st.slideFresh = false;
            // The slide comes back under the id it had, so that what refers to it by id (and an undo) still finds it.
            const QString want = from.pages[n]->id;
            if (!want.isEmpty() && findSlide(w, cmp, CompareRef{want}) < 0) w.pages[index]->id = want;
            if (w.pages[index]->id.isEmpty()) w.pages[index]->id = newId("p");
            own[n] = CompareRef{w.pages[index]->id};
            st.slideFresh = false;
            return true;
        }
        if (at < 0) return true;
        if (w.pages.size() < 2) return false;
        if (w.pages[at]->id == st.samplePageId) st.samplePage = -1;
        removeSlideAt(w, at);
        st.slideFresh = false;
        return true;
    }
    if (path == QLatin1String("order")) {
        if (ms < 0 || ts < 0 || ms >= cmp.workOfMine.size()) return false;
        const int idx = findSlide(w, cmp, cmp.workOfMine[ms]);
        if (idx < 0) return false;
        const int k = placeAfter(w, cmp, refs, srcSlide) - 1;   // the slide it follows, -1 for the first place
        if (k + 1 == idx) return true;
        const std::shared_ptr<Page> page = w.pages[idx];
        const int at = k < idx ? k + 1 : k;
        removeSlideAt(w, idx);
        w.pages.insert(at, page);
        slideInserted(w, at);
        st.slideFresh = false;
        return true;
    }

    // Everything else is on a slide that both have.
    if (ms < 0 || ms >= cmp.workOfMine.size()) return false;
    const int sl = findSlide(w, cmp, cmp.workOfMine[ms]);
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

    // Objects. Each is known by the id it has in the working presentation (made one of its own when the first accept or reject began).
    const QString mid = dataString(ch, "mid");
    auto locate = [&](const QString &id) -> Spot {
        if (id.isEmpty()) return {};
        // An id the comparison has not seen in the working presentation, or one it put there itself (the index of Document::find does not know it
        // yet), is looked for on this slide alone: find looks through every slide for an id it does not know, and that is most of what accepting
        // objects added or putting back objects deleted asks for.
        if (!st.ids.contains(id) || st.made.contains(id)) {
            Spot found;
            const auto h = st.hint.constFind(id);
            if (h != st.hint.cend() && *h >= 0 && *h < int(page.items.size()) && page.items[*h]->id == id) {
                found = Spot{&page.items, *h, page.items[*h].get()};
            } else {
                std::function<bool(ItemList &)> look = [&](ItemList &l) {
                    for (int i = 0; i < int(l.size()); ++i) {
                        ++cmp.work;
                        if (l[i]->id == id) { found = Spot{&l, i, l[i].get()}; return true; }
                        if (l[i]->type() == ItemType::Group && look(static_cast<GroupItem *>(l[i].get())->children)) return true;
                    }
                    return false;
                };
                look(page.items);
            }
            if (found.item) st.ids.insert(id);
            return found;
        }
        const Document::Loc loc = w.find(id);
        if (loc.item && loc.item->id == id && loc.page == sl && !loc.scratch) return Spot{loc.list, loc.index, loc.item};
        return {};
    };
    if (path == QLatin1String("object")) {
        const bool added = ch.kind == Kind::Added;
        const bool wanted = added == accept;   // the object should be in the working presentation
        const QString tkey = QString::number(ts) + QLatin1Char('/') + dataString(ch, "ti");
        const Spot here = locate(added ? dataString(ch, "wid") : mid);
        if (wanted) {
            if (here.item) return true;
            ItemList *list = &page.items;
            if (const QString pid = dataString(ch, "pid"); !pid.isEmpty()) {
                const Spot parent = locate(pid);
                if (!parent.item || parent.item->type() != ItemType::Group) return false;
                list = &static_cast<GroupItem *>(parent.item)->children;
            }
            const QString ipath = dataString(ch, added ? "ti" : "mi");
            const Item *srcItem = itemAtPath(from.items, ipath);
            if (!srcItem) return false;
            ItemPtr made = Carry(w, src).copy(*srcItem);
            if (!srcItem->id.isEmpty() && !st.ids.contains(srcItem->id)) made->id = srcItem->id;   // the id it had, if free
            ++cmp.work;
            const int at = placeFor(*list, ipath, added, added ? ts : ms, cmp);
            list->insert(list->begin() + at, made);
            ItemList one{made};
            walkItems(one, [&](const ItemPtr &it) { st.ids.insert(it->id); st.made.insert(it->id); });
            st.hint[made->id] = at;
            ch.data[QStringLiteral("wid")] = made->id;
            if (added) st.theirsWork[tkey] = made->id;
            return true;
        }
        if (!here.item) return true;
        const ItemPtr gone = (*here.list)[here.index];
        here.list->erase(here.list->begin() + here.index);
        ItemList one{gone};
        walkItems(one, [&](const ItemPtr &it) {
            st.ids.remove(it->id);
            if (it->id == st.sampleId) st.sampleSlide = -1;
        });
        st.hint.remove(gone->id);
        if (added) st.theirsWork.remove(tkey);
        return true;
    }
    // A property of an object both have.
    const Spot spot = locate(mid);
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

ChangeMark markOf(const Document &working, DeckComparison &cmp, int index)
{
    ChangeMark out;
    if (index < 0 || index >= cmp.changes.changes.size()) return out;
    const Change &ch = cmp.changes.changes[index];
    const int ms = dataInt(ch, "ms"), ts = dataInt(ch, "ts");
    const QString mid = dataString(ch, "mid");
    if (!mid.isEmpty()) {
        const Document::Loc loc = working.find(mid);
        if (loc.item && loc.page >= 0 && loc.item->id == mid) {
            out.slide = loc.page;
            out.rect = loc.item->rect;
            out.hasRect = true;
            return out;
        }
    }
    if (ms >= 0 && ms < cmp.workOfMine.size()) out.slide = findSlide(working, cmp, cmp.workOfMine[ms]);
    else if (ts >= 0 && ts < cmp.workOfTheirs.size()) out.slide = findSlide(working, cmp, cmp.workOfTheirs[ts]);
    if (ch.target.path == QLatin1String("object") && ch.kind == Kind::Added && ts >= 0 && ts < cmp.theirs->pages.size()) {
        if (const Item *it = itemAtPath(cmp.theirs->pages[ts]->items, dataString(ch, "ti"))) {
            out.rect = it->rect;
            out.hasRect = true;
        }
    }
    return out;
}

bool textChangeOf(const DeckComparison &cmp, int index, QString *oldText, QString *newText)
{
    if (index < 0 || index >= cmp.changes.changes.size()) return false;
    const Change &ch = cmp.changes.changes[index];
    if (ch.target.path != QLatin1String("text") || !cmp.mine || !cmp.theirs) return false;
    const int ms = dataInt(ch, "ms"), ts = dataInt(ch, "ts");
    if (ms < 0 || ts < 0 || ms >= cmp.mine->pages.size() || ts >= cmp.theirs->pages.size()) return false;
    const Item *a = itemAtPath(cmp.mine->pages[ms]->items, dataString(ch, "mi"));
    const Item *b = itemAtPath(cmp.theirs->pages[ts]->items, dataString(ch, "ti"));
    if (!a || !b) return false;
    Budget budget;
    bool cut = false;
    const QString before = textOf(*cmp.mine, *a, budget, cut), after = textOf(*cmp.theirs, *b, budget, cut);
    if (oldText) *oldText = before;
    if (newText) *newText = after;
    return true;
}

QString changeKey(const Change &c)
{
    // The ids of a presentation read from a file are made again with each reading, so a change is told by where it is in the other presentation (that is kept
    // whole, and read the same way each time: its slide's and object's places) and by a hash of what changed. A slide taken out has no place there: its hash tells it.
    const QChar bar = QLatin1Char('|');
    return kindKey(c.kind) + bar + c.target.path + bar + QString::number(dataInt(c, "ts")) + bar + dataString(c, "ti") + bar + c.data.value(QStringLiteral("ck")).toString() + bar +
           QString::fromLatin1(QCryptographicHash::hash(c.data.value(QStringLiteral("ckey")).toString().toUtf8(), QCryptographicHash::Sha1).toHex().left(16));
}

}   // namespace jp::deck
