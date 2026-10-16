#include "core/masteredit.h"

#include "core/themes_p.h"
#include "text/storyio.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

const QString kPreserve = QStringLiteral("preserve");

QString say(const char *text) { return QCoreApplication::translate("Deck", text); }

bool isFooter(const QString &type) { return type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum"); }
bool isTitle(const QString &type) { return type == QLatin1String("title") || type == QLatin1String("ctrTitle"); }

// The kind of place on a slide master that a placeholder takes its look from, or empty when the slide master has none for it.
QString masterKind(const QString &type)
{
    if (isTitle(type)) return QStringLiteral("title");
    if (type == QLatin1String("body") || type == QLatin1String("obj") || type == QLatin1String("subTitle")) return QStringLiteral("body");
    return isFooter(type) ? type : QString();
}

// The text of a placeholder, with its formatting, is a story of the presentation; a page that is taken away takes its stories.
void forgetStories(Document &d, const ItemList &items)
{
    walkItems(items, [&](const ItemPtr &it) {
        if (it->type() == ItemType::Text) {
            d.removeStory(static_cast<TextItem *>(it.get())->storyId);
        } else if (it->type() == ItemType::Shape) {
            d.removeStory(static_cast<ShapeItem *>(it.get())->storyId);
        } else if (it->type() == ItemType::Table) {
            for (const TableCell &c : static_cast<TableItem *>(it.get())->cells) {
                d.removeStory(c.storyId);
                d.removeStory(c.storyId2);
            }
        }
    });
}

void removePlaceholders(Document &d, MasterPage &page, const std::function<bool(const QString &)> &which)
{
    ItemList kept;
    ItemList gone;
    for (const auto &it : page.items) (which(placeholderType(*it)) && it->type() == ItemType::Text ? gone : kept).push_back(it);
    forgetStories(d, gone);
    page.items = kept;
}

const Item *placeholderOn(const PageBase &page, const std::function<bool(const QString &)> &which)
{
    for (const auto &it : page.items)
        if (it->type() == ItemType::Text && which(placeholderType(*it))) return it.get();
    return nullptr;
}

// A copy of the slide master's placeholder of that type (the first master of the layout's chain that has one), else the standard one.
ItemPtr placeholderLikeMaster(Document &d, const MasterPage &layout, const QString &type)
{
    const MasterPage *master = d.master(layout.baseId);
    if (master && isSlideMaster(*master))
        if (const Item *from = placeholderOn(*master, [&](const QString &t) { return t == type; })) return d.cloneItem(*from);
    return standardPlaceholder(d, type);
}

qint64 numberOf(const Item &it) { return it.role.split(QLatin1Char(':')).value(2).toLongLong(); }

} // namespace

QString placeholderTypeOf(PlaceholderKind kind)
{
    switch (kind) {
    case PlaceholderKind::Content: return QStringLiteral("obj");
    case PlaceholderKind::Text: return QStringLiteral("body");
    case PlaceholderKind::Picture: return QStringLiteral("pic");
    case PlaceholderKind::Chart: return QStringLiteral("chart");
    case PlaceholderKind::Table: return QStringLiteral("tbl");
    case PlaceholderKind::Media: return QStringLiteral("media");
    case PlaceholderKind::OnlineImage: return QStringLiteral("clipArt");
    }
    return QString();
}

QString placeholderLabel(const QString &type)
{
    if (type == QLatin1String("pic")) return say("Picture");
    if (type == QLatin1String("chart")) return say("Chart");
    if (type == QLatin1String("tbl")) return say("Table");
    if (type == QLatin1String("media")) return say("Media");
    if (type == QLatin1String("clipArt")) return say("Online Image");
    return QString();
}

int slidesUsing(const Document &d, const MasterPage &page)
{
    int n = 0;
    for (const auto &p : d.pages) {
        if (p->masterId == page.id) {
            ++n;
        } else if (isSlideMaster(page)) {
            const MasterPage *layout = d.master(p->masterId);
            if (layout && isLayout(*layout) && layout->baseId == page.id) ++n;
        }
    }
    return n;
}

QString whyCantDelete(const Document &d, const MasterPage &page)
{
    const int used = slidesUsing(d, page);
    if (isSlideMaster(page)) {
        const int masters = int(std::count_if(d.masters.begin(), d.masters.end(), [](const auto &m) { return isSlideMaster(*m); }));
        if (masters < 2) return say("This is the only slide master. A presentation needs at least one.");
        if (used == 1) return say("1 slide uses this slide master. Give it a layout of another slide master first.");
        if (used > 1) return QCoreApplication::translate("Deck", "%1 slides use this slide master. Give them layouts of another slide master first.").arg(used);
        return QString();
    }
    if (used == 1) return say("1 slide uses this layout. Give it another layout first.");
    if (used > 1) return QCoreApplication::translate("Deck", "%1 slides use this layout. Give them another layout first.").arg(used);
    if (isLayout(page) && layouts(d, page.baseId).size() < 2) return say("A slide master needs at least one layout.");
    return QString();
}

bool deleteMasterPage(Document &d, const QString &id, QString *why)
{
    MasterPage *page = d.master(id);
    const QString reason = !page || !(isSlideMaster(*page) || isLayout(*page)) ? say("There is nothing to delete.") : whyCantDelete(d, *page);
    if (!reason.isEmpty()) {
        if (why) *why = reason;
        return false;
    }
    // The first slide master holds the presentation's theme: the next one takes it over.
    MasterPage *next = nullptr;
    DeckTheme theme;
    QByteArray themeXml;
    if (isSlideMaster(*page) && page == detail::firstSlideMaster(d)) {
        for (const auto &m : d.masters)
            if (isSlideMaster(*m) && m.get() != page) {
                next = m.get();
                break;
            }
        if (next) {
            theme = detail::masterTheme(d, *next);
            themeXml = detail::masterThemeXml(d, *next);
        }
    }
    if (why) why->clear();
    const QVector<std::shared_ptr<MasterPage>> all = d.masters;
    for (const auto &m : all) {
        if (m.get() != page && !(isSlideMaster(*page) && isLayout(*m) && m->baseId == id)) continue;
        forgetStories(d, m->items);
        d.masters.removeOne(m);
    }
    if (next) {
        next->props.remove(QStringLiteral("themeXml"));
        detail::storeMasterTheme(d, *next, theme, themeXml);
    }
    return true;
}

void renameMasterPage(Document &d, const QString &id, const QString &name)
{
    if (MasterPage *page = d.master(id); page && !name.trimmed().isEmpty()) page->name = name.trimmed();
}

bool isPreserved(const MasterPage &page) { return page.props.value(kPreserve).toBool(true); }

bool isKept(const Document &d, const MasterPage &page)
{
    if (!isPreserved(page)) return false;
    const MasterPage *master = isLayout(page) ? d.master(page.baseId) : &page;
    return !(master && isSlideMaster(*master) && isThemeMade(*master));
}

bool keepMasterOf(Document &d, const QString &id)
{
    const MasterPage *page = d.master(id);
    if (page && isLayout(*page)) page = d.master(page->baseId);
    if (!page || !isSlideMaster(*page)) return false;
    MasterPage *master = d.master(page->id);
    const QJsonObject was = master->props;
    keepSlideMaster(*master);
    return master->props != was;
}

void setPreserved(Document &d, MasterPage &page, bool on)
{
    if (!on) {
        page.props[kPreserve] = false;
        return;
    }
    page.props.remove(kPreserve);
    keepMasterOf(d, page.id);
}

Item *insertPlaceholder(Document &d, MasterPage &layout, PlaceholderKind kind)
{
    if (!isLayout(layout)) return nullptr;
    const QString type = placeholderTypeOf(kind);
    // Its text styles are the slide master's body: the layout's own content or text place, else the master's.
    const Item *from = placeholderOn(layout, [](const QString &t) { return t == QLatin1String("obj") || t == QLatin1String("body"); });
    if (!from)
        for (const MasterPage *m : d.masterChain(&layout))
            if ((from = placeholderOn(*m, [](const QString &t) { return t == QLatin1String("body") || t == QLatin1String("obj"); }))) break;
    ItemPtr made;
    if (from) made = copyPlaceholder(d, *from, kind != PlaceholderKind::Content && kind != PlaceholderKind::Text);
    else made = standardPlaceholder(d, QStringLiteral("body"));
    if (!made || made->type() != ItemType::Text) return nullptr;
    auto *t = static_cast<TextItem *>(made.get());
    if (kind == PlaceholderKind::Text) {
        // One line of sample text, not the five levels.
        if (QTextDocument *sd = d.storyDoc(t->storyId)) {
            QTextCursor c(sd);
            c.movePosition(QTextCursor::EndOfBlock);
            c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            c.removeSelectedText();
        }
    }
    // The smallest number from 1 that no place of the layout has (a file may number a place 4294967295).
    QSet<qint64> used;
    for (const auto &it : layout.items) used.insert(numberOf(*it));
    qint64 number = 1;
    while (used.contains(number)) ++number;
    t->role = placeholderRole(type, number);
    const QString label = placeholderLabel(type);
    t->name = QCoreApplication::translate("Deck", "%1 Placeholder").arg(label.isEmpty() ? (kind == PlaceholderKind::Text ? say("Text") : say("Content")) : label);
    t->prompt.clear();
    // Where the body goes, moved aside from the placeholders that already stand there.
    QRectF rect = t->rect;
    const QSizeF page = d.pageSize();
    for (int tries = 0; tries < 20; ++tries) {
        const bool taken = std::any_of(layout.items.begin(), layout.items.end(), [&](const ItemPtr &o) {
            return o->type() == ItemType::Text && !placeholderType(*o).isEmpty() && std::abs(o->rect.x() - rect.x()) < 1 && std::abs(o->rect.y() - rect.y()) < 1;
        });
        if (!taken) break;
        rect.translate(24, 24);
    }
    rect.setRight(std::min(rect.right(), page.width()));
    rect.setBottom(std::min(rect.bottom(), page.height()));
    t->rect = rect;
    layout.items.push_back(made);
    return made.get();
}

bool layoutHasTitle(const MasterPage &layout) { return placeholderOn(layout, isTitle) != nullptr; }
bool layoutHasFooters(const MasterPage &layout) { return placeholderOn(layout, isFooter) != nullptr; }

void setLayoutTitle(Document &d, MasterPage &layout, bool on)
{
    if (!isLayout(layout) || layoutHasTitle(layout) == on) return;
    if (!on) {
        removePlaceholders(d, layout, isTitle);
        return;
    }
    if (ItemPtr made = placeholderLikeMaster(d, layout, QStringLiteral("title"))) layout.items.insert(layout.items.begin(), made);
}

void setLayoutFooters(Document &d, MasterPage &layout, bool on)
{
    if (!isLayout(layout)) return;
    if (!on) {
        removePlaceholders(d, layout, isFooter);
        return;
    }
    for (const QString &type : {QStringLiteral("dt"), QStringLiteral("ftr"), QStringLiteral("sldNum")})
        if (!placeholderOn(layout, [&](const QString &t) { return t == type; }))
            if (ItemPtr made = placeholderLikeMaster(d, layout, type)) layout.items.push_back(made);
}

MasterParts masterParts(const MasterPage &master)
{
    auto has = [&](const char *type) { return placeholderOn(master, [&](const QString &t) { return t == QLatin1String(type); }) != nullptr; };
    MasterParts p;
    p.title = has("title");
    p.text = has("body");
    p.date = has("dt");
    p.number = has("sldNum");
    p.footer = has("ftr");
    return p;
}

namespace {

const QString kHeld = QStringLiteral("heldParts");

// A place taken off the slide master is kept in its properties (its object and its text), so that it comes back as it was.
void holdPart(Document &d, MasterPage &master, const QString &type)
{
    const Item *it = placeholderOn(master, [&](const QString &t) { return t == type; });
    if (!it) return;
    QJsonObject kept = master.props.value(kHeld).toObject();
    QJsonObject entry{{"item", it->toJson()}};
    if (const QTextDocument *sd = d.storyDoc(static_cast<const TextItem *>(it)->storyId)) entry.insert(QStringLiteral("story"), storyToJson(sd));
    kept.insert(type, entry);
    master.props[kHeld] = kept;
}

// The place as it was held (a new object with a story of its own), else null.
ItemPtr heldPart(Document &d, MasterPage &master, const QString &type)
{
    QJsonObject kept = master.props.value(kHeld).toObject();
    const QJsonObject entry = kept.value(type).toObject();
    if (entry.isEmpty()) return nullptr;
    kept.remove(type);
    if (kept.isEmpty()) master.props.remove(kHeld);
    else master.props[kHeld] = kept;
    ItemPtr made = Item::fromJsonAny(entry.value(QStringLiteral("item")).toObject());
    if (!made || made->type() != ItemType::Text) return nullptr;
    made = made->clone();   // with an id of its own
    auto *t = static_cast<TextItem *>(made.get());
    t->storyId = d.createStory();
    if (QTextDocument *sd = d.storyDoc(t->storyId)) storyFromJson(sd, entry.value(QStringLiteral("story")).toObject());
    return made;
}

} // namespace

void setMasterParts(Document &d, MasterPage &master, const MasterParts &parts)
{
    if (!isSlideMaster(master)) return;
    struct Part { const char *type; bool wanted; };
    for (const Part &part : {Part{"title", parts.title}, Part{"body", parts.text}, Part{"dt", parts.date}, Part{"sldNum", parts.number}, Part{"ftr", parts.footer}}) {
        const QString type = QString::fromLatin1(part.type);
        const bool has = placeholderOn(master, [&](const QString &t) { return t == type; }) != nullptr;
        if (has == part.wanted) continue;
        if (!part.wanted) {
            holdPart(d, master, type);
            removePlaceholders(d, master, [&](const QString &t) { return t == type; });
        } else {
            ItemPtr made = heldPart(d, master, type);
            if (!made) made = standardPlaceholder(d, type);
            if (!made) continue;
            if (type == QLatin1String("title")) master.items.insert(master.items.begin(), made);
            else master.items.push_back(made);
        }
    }
}

// ---- carrying an edit to what stands on it

namespace {

QByteArray compact(const QJsonObject &o) { return QJsonDocument(o).toJson(QJsonDocument::Compact); }

// The stories of an object.
QStringList storiesOf(const Item &it)
{
    QStringList stories;
    if (it.type() == ItemType::Text) stories << static_cast<const TextItem &>(it).storyId;
    else if (it.type() == ItemType::Shape) stories << static_cast<const ShapeItem &>(it).storyId;
    return stories;
}

// A story as JSON without the numbers Qt gives the lists in it (each list object has one, which differs between a text and its copy read
// from JSON), so that the same text compares equal.
QJsonObject storyLook(const QTextDocument *story)
{
    const QString key = QString::number(int(QTextFormat::ObjectIndex));
    QJsonObject o = storyToJson(story);
    QJsonArray blocks = o.value(QStringLiteral("blocks")).toArray();
    for (int i = 0; i < blocks.size(); ++i) {
        QJsonObject b = blocks[i].toObject();
        QJsonObject f = b.value(QStringLiteral("f")).toObject();
        if (f.contains(key)) {
            f.remove(key);
            b.insert(QStringLiteral("f"), f);
            blocks[i] = b;
        }
    }
    o.insert(QStringLiteral("blocks"), blocks);
    QJsonArray lists = o.value(QStringLiteral("lists")).toArray();
    for (int i = 0; i < lists.size(); ++i) {
        QJsonObject f = lists[i].toObject();
        if (f.contains(key)) {
            f.remove(key);
            lists[i] = f;
        }
    }
    if (!lists.isEmpty()) o.insert(QStringLiteral("lists"), lists);
    return o;
}

// What an object looks like, as bytes that are equal when it looks the same: its own properties (not which story it has), and its text.
QByteArray lookOfItem(const Document &d, const Item &it)
{
    QJsonObject o = it.toJson();
    o.remove(QStringLiteral("story"));
    QByteArray out = compact(o);
    for (const QString &id : storiesOf(it))
        if (const QTextDocument *story = id.isEmpty() ? nullptr : d.storyDoc(id)) out += compact(storyLook(story));
    return out;
}

// What a page looks like: its background, whether the pages under it show, and its objects.
QByteArray lookOf(const Document &d, const MasterPage &page)
{
    QByteArray out = compact(page.background.toJson()) + (page.showMasterItems ? "m1" : "m0");
    walkItems(page.items, [&](const ItemPtr &it) { out += lookOfItem(d, *it); });
    return out;
}

// What the placeholders of a page look like (the places a slide takes its look from).
QByteArray holdersOf(const Document &d, const MasterPage &page)
{
    QByteArray out;
    for (const auto &it : page.items)
        if (it->type() == ItemType::Text && !placeholderType(*it).isEmpty()) out += it->id.toUtf8() + lookOfItem(d, *it);
    return out;
}

bool near(const QRectF &a, const QRectF &b)
{
    return std::abs(a.x() - b.x()) < 0.01 && std::abs(a.y() - b.y()) < 0.01 && std::abs(a.width() - b.width()) < 0.01 && std::abs(a.height() - b.height()) < 0.01;
}

const Item *itemWithId(const PageBase &page, const QString &id)
{
    for (const auto &it : page.items)
        if (it->id == id) return it.get();
    return nullptr;
}

// The placeholder of a slide master that a layout's placeholder takes its look from.
const Item *masterPlaceFor(const PageBase &master, const Item &holder)
{
    const QString kind = masterKind(placeholderType(holder));
    if (kind.isEmpty()) return nullptr;
    for (const auto &it : master.items)
        if (it->type() == ItemType::Text && masterKind(placeholderType(*it)) == kind) return it.get();
    return nullptr;
}

// A placeholder of a page, or a layout's of the slide master, that stood as `from` and now stands as `to` follows: where it had the old
// place and look it takes the new, and the text that looked as the old placeholder's text did looks as the new one's does.
void follow(Document &d, TextItem &t, const Item &from, const Item &to)
{
    if (near(t.rect, from.rect)) t.rect = to.rect;
    if (t.rotation == from.rotation) t.rotation = to.rotation;
    if (to.type() == ItemType::Text && from.type() == ItemType::Text) {
        const auto &f = static_cast<const TextItem &>(from);
        const auto &n = static_cast<const TextItem &>(to);
        if (t.valign == f.valign) t.valign = n.valign;
        if (t.insets == f.insets) t.insets = n.insets;
    }
    if (t.fill.toJson() == from.fill.toJson()) t.fill = to.fill;
    if (t.stroke == from.stroke) t.stroke = to.stroke;
    followLayout(d, t, &from, &to, std::function<bool(QTextCharFormat &, const Following &)>());
}

} // namespace

bool carryMasterEdits(Document &now, const Document &before)
{
    // The pages edited: those of the same id whose look differs.
    QSet<QString> edited;
    for (const auto &m : now.masters)
        if (const MasterPage *was = before.master(m->id); was && lookOf(now, *m) != lookOf(before, *was)) edited.insert(m->id);
    if (edited.isEmpty()) return false;
    bool changed = false;
    for (const auto &m : now.masters)
        if (edited.contains(m->id)) changed = keepMasterOf(now, m->id) || changed;

    // What the old placeholder looked like, as a placeholder of `now` (its text copied into a story of its own, taken away at the end).
    QStringList borrowed;
    QHash<const Item *, ItemPtr> lent;   // each once, however many slides stand on it
    auto lend = [&](const Item &old) {
        if (const auto found = lent.constFind(&old); found != lent.cend()) return found.value();
        ItemPtr copy = old.clone();
        if (copy->type() == ItemType::Text) {
            auto *t = static_cast<TextItem *>(copy.get());
            const QString story = now.createStory();
            if (const QTextDocument *src = before.storyDoc(t->storyId)) storyFromJson(now.storyDoc(story), storyToJson(src));
            t->storyId = story;
            borrowed << story;
        }
        lent.insert(&old, copy);
        return copy;
    };
    // Only a placeholder that changed is followed: setting text that did not change would set it again (its lists regroup).
    auto differs = [&](const Item &to, const Item &from) { return lookOfItem(now, to) != lookOfItem(before, from); };

    // A slide master's change reaches its layouts' placeholders.
    for (const auto &m : now.masters) {
        const MasterPage *was = isSlideMaster(*m) && edited.contains(m->id) ? before.master(m->id) : nullptr;
        if (!was || holdersOf(now, *m) == holdersOf(before, *was)) continue;
        for (const auto &l : now.masters) {
            if (!isLayout(*l) || l->baseId != m->id) continue;
            for (const auto &p : l->items) {
                if (p->type() != ItemType::Text || placeholderType(*p).isEmpty()) continue;
                const Item *to = masterPlaceFor(*m, *p), *from = masterPlaceFor(*was, *p);
                if (!to || !from || !differs(*to, *from)) continue;
                const ItemPtr old = lend(*from);
                follow(now, static_cast<TextItem &>(*p), *old, *to);
                changed = true;
            }
        }
    }

    // A layout's change (or its slide master's, as the layout now has it) reaches the slides that stand on it: those whose placeholders it
    // changed, each placeholder in the place the layout has for it (a "text" place stands over a "content" place).
    for (const auto &l : now.masters) {
        const MasterPage *was = before.master(l->id);
        if (!was || (!isLayout(*l) && !isSlideMaster(*l)) || holdersOf(now, *l) == holdersOf(before, *was)) continue;
        for (const auto &slide : now.pages) {
            if (slide->masterId != l->id) continue;
            QSet<const Item *> taken;
            for (const auto &p : slide->items) {
                if (p->type() != ItemType::Text || placeholderType(*p).isEmpty()) continue;
                const Item *to = matchPlaceholder(*l, *p, taken);
                const Item *from = to ? itemWithId(*was, to->id) : nullptr;
                if (!to || !from || !differs(*to, *from)) continue;
                const ItemPtr old = lend(*from);
                follow(now, static_cast<TextItem &>(*p), *old, *to);
                changed = true;
            }
        }
    }
    for (const QString &id : borrowed) now.removeStory(id);
    return changed;
}

} // namespace jp::deck
