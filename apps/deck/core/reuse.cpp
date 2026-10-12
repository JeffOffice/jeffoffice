#include "core/reuse.h"

#include "core/deck.h"
#include "core/sections.h"
#include "office/theme.h"
#include "text/storyio.h"
#include "text/textprops.h"

#include <QCryptographicHash>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace jp::deck {

namespace {

// The ways of carrying what an object holds into another presentation: its
// text, pictures, and movies are copied there, once each, under new ids.
class Carry {
public:
    // `keepFonts`: text that follows its presentation's fonts is set in the fonts of `from`.
    Carry(Document &to, const Document &from, bool keepFonts) : m_to(to), m_from(from), m_keepFonts(keepFonts && fontsDiffer(to, from)) {}

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
        auto found = m_stories.constFind(id);
        if (found == m_stories.cend()) {
            const QString made = m_to.createStory();
            if (const QTextDocument *sd = m_from.storyDoc(id)) storyFromJson(m_to.storyDoc(made), storyToJson(sd));
            if (m_keepFonts) namedFontsToFamilies(m_to.storyDoc(made));
            found = m_stories.insert(id, made);
        }
        return found.value();
    }
    void fill(Fill &f) { f.imageId = image(f.imageId); }

    // Copies of the objects, with new ids all through; the shape numbers a file gave them stay, as what an animation aims at. `ids` gets old -> new.
    ItemList items(const ItemList &src, QHash<QString, QString> *ids)
    {
        ItemList out;
        for (const auto &it : src) out.push_back(it->clone());
        std::vector<const Item *> was;
        std::vector<Item *> now;
        walkItems(src, [&](const ItemPtr &it) { was.push_back(it.get()); });
        walkItems(out, [&](const ItemPtr &it) { now.push_back(it.get()); });
        for (size_t i = 0; i < now.size() && i < was.size(); ++i) {
            now[i]->sourceId = was[i]->sourceId;
            ids->insert(was[i]->id, now[i]->id);
            carry(now[i]);
        }
        remapGlue(out, *ids);
        return out;
    }

private:
    static bool fontsDiffer(const Document &a, const Document &b) { return a.fonts.heading != b.fonts.heading || a.fonts.body != b.fonts.body; }

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
            break;
        }
        default: break;
        }
    }

    // Text that names the theme's fonts ("major", "minor") says the fonts of the presentation it is in: here, the other one's.
    void namedFontsToFamilies(QTextDocument *sd)
    {
        struct Run {
            int from, to;
            QTextCharFormat format;
        };
        QVector<Run> runs;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
            for (auto f = b.begin(); !f.atEnd(); ++f) {
                const QTextCharFormat cf = f.fragment().charFormat();
                if (cf.hasProperty(tp::ThemeFont) && !cf.hasProperty(QTextFormat::FontFamilies)) runs << Run{f.fragment().position(), f.fragment().position() + f.fragment().length(), cf};
            }
        for (Run &r : runs) {
            const QString family = r.format.stringProperty(tp::ThemeFont) == QLatin1String("major") ? m_from.fonts.heading : m_from.fonts.body;
            r.format.clearProperty(tp::ThemeFont);
            r.format.setFontFamilies(QStringList{family});
            QTextCursor c(sd);
            c.setPosition(r.from);
            c.setPosition(r.to, QTextCursor::KeepAnchor);
            c.setCharFormat(r.format);
        }
    }

    Document &m_to;
    const Document &m_from;
    bool m_keepFonts;
    QHash<QString, QString> m_images, m_media, m_stories;
};

// ---- telling whether a layout or slide master is one already here ----

// The page as JSON with the ids that name its parts left out and the parts
// themselves (a story's text, a picture's bytes) put in their place.
QJsonValue plain(const Document &d, const QJsonValue &v)
{
    if (v.isArray()) {
        QJsonArray out;
        for (const QJsonValue &e : v.toArray()) out.append(plain(d, e));
        return out;
    }
    if (!v.isObject()) return v;
    QJsonObject out;
    const QJsonObject o = v.toObject();
    for (auto it = o.begin(); it != o.end(); ++it) {
        const QString key = it.key();
        if (key == QLatin1String("id") || key == QLatin1String("next")) continue;
        if (key == QLatin1String("story") || key == QLatin1String("story2")) {
            const QTextDocument *sd = d.storyDoc(it.value().toString());
            out.insert(key, sd ? QJsonValue(storyToJson(sd)) : QJsonValue());
        } else if (key == QLatin1String("image")) {
            out.insert(key, QString::fromLatin1(QCryptographicHash::hash(d.images.value(it.value().toString()).bytes, QCryptographicHash::Sha1).toHex()));
        } else if (key == QLatin1String("media")) {
            out.insert(key, QString::fromLatin1(QCryptographicHash::hash(d.media.value(it.value().toString()).bytes, QCryptographicHash::Sha1).toHex()));
        } else {
            out.insert(key, plain(d, it.value()));
        }
    }
    return out;
}

QString lookOf(const Document &d, const MasterPage &page)
{
    QJsonObject props = page.props;
    for (const char *key : {"themeXml", "charts", "reuseKey"}) props.remove(QLatin1String(key));
    QJsonArray items;
    for (const auto &it : page.items) items.append(plain(d, it->toJson()));
    const QJsonObject o{{"name", page.name}, {"props", props}, {"background", plain(d, page.background.toJson())}, {"showMasterItems", page.showMasterItems}, {"items", items}};
    return QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(o).toJson(QJsonDocument::Compact), QCryptographicHash::Sha1).toHex());
}

// What a slide master is, and what a layout is on its slide master.
QString masterKey(const Document &d, const MasterPage &master) { return lookOf(d, master); }
QString layoutKey(const Document &d, const MasterPage &layout, const MasterPage *master) { return (master ? masterKey(d, *master) : QString()) + lookOf(d, layout); }

// Whether the page is the one the key names: it was brought from there, or looks the same.
bool isKnownAs(const MasterPage &page, const QString &key, const QString &lookHere) { return page.props.value(QStringLiteral("reuseKey")).toString() == key || lookHere == key; }

QString uniqueMasterId(const Document &d, const QString &prefix)
{
    for (int n = 1;; ++n)
        if (!d.master(prefix + QString::number(n))) return prefix + QString::number(n);
}

// The theme part of a slide master of `d`: the one kept from its file, else made from the presentation's colors and fonts.
QByteArray themeXmlOf(const Document &d, const MasterPage &master)
{
    QByteArray kept = QByteArray::fromBase64(master.props.value(QStringLiteral("themeXml")).toString().toLatin1());
    const MasterPage *first = nullptr;
    for (const auto &m : d.masters)
        if (isSlideMaster(*m)) {
            first = m.get();
            break;
        }
    if (kept.isEmpty() && first == &master) kept = QByteArray::fromBase64(d.appData.value(QStringLiteral("themeXml")).toString().toLatin1());
    if (!kept.isEmpty()) return kept;
    const DeckTheme deckTheme = themeOf(d);
    office::Theme theme;
    theme.name = theme.colorSchemeName = theme.fontSchemeName = deckTheme.name;
    for (int i = 0; i < office::Theme::ColorCount; ++i) theme.colors[i] = deckTheme.colors[i];
    theme.majorLatin = deckTheme.headingFont;
    theme.minorLatin = deckTheme.bodyFont;
    return office::writeTheme(theme);
}

// The charts a page keeps name the objects they are the picture of: the copies.
void remapCharts(QJsonObject *props, const QHash<QString, QString> &ids)
{
    QJsonArray charts = props->value(QStringLiteral("charts")).toArray();
    for (int i = 0; i < charts.size(); ++i) {
        QJsonObject c = charts[i].toObject();
        c[QStringLiteral("item")] = ids.value(c.value(QStringLiteral("item")).toString());
        charts[i] = c;
    }
    if (!charts.isEmpty()) props->insert(QStringLiteral("charts"), charts);
}

// A copy of a layout or slide master of `from` in `to` (its theme too, for a slide master), the pages it stands on not yet looked at.
std::shared_ptr<MasterPage> copyMasterPage(Document &to, const Document &from, const MasterPage &src, const QString &id, const QString &key)
{
    Carry carry(to, from, true);
    auto page = std::make_shared<MasterPage>();
    page->id = id;
    page->name = src.name;
    page->abbr = src.abbr;
    page->twoPage = src.twoPage;
    page->grid = src.grid;
    page->guides = src.guides;
    page->showMasterItems = src.showMasterItems;
    page->props = src.props;
    page->background = src.background;
    carry.fill(page->background);
    QHash<QString, QString> ids;
    page->items = carry.items(src.items, &ids);
    remapCharts(&page->props, ids);
    page->props[QStringLiteral("reuseKey")] = key;
    if (isSlideMaster(src)) page->props[QStringLiteral("themeXml")] = QString::fromLatin1(themeXmlOf(from, src).toBase64());
    return page;
}

// The layout of `to` that stands for slide layout `layout` of `from` (and its slide master), brought in when there is none.
const MasterPage *layoutFor(Document &to, const Document &from, const MasterPage &layout)
{
    const MasterPage *master = isLayout(layout) ? from.master(layout.baseId) : &layout;
    if (master && !isSlideMaster(*master)) master = nullptr;
    const QString mKey = master ? masterKey(from, *master) : QString();
    const MasterPage *base = nullptr;
    if (master) {
        for (const auto &m : to.masters)
            if (isSlideMaster(*m) && isKnownAs(*m, mKey, masterKey(to, *m))) {
                base = m.get();
                break;
            }
        if (!base) {
            auto made = copyMasterPage(to, from, *master, uniqueMasterId(to, QStringLiteral("SM")), mKey);
            to.masters << made;
            base = made.get();
        }
    }
    if (!isLayout(layout)) return base;   // a slide that stands on a slide master directly
    const QString lKey = layoutKey(from, layout, master);
    int last = -1;
    for (int i = 0; i < to.masters.size(); ++i) {
        const MasterPage &m = *to.masters[i];
        if (!base || (m.id != base->id && m.baseId != base->id)) continue;
        last = i;
        if (isLayout(m) && isKnownAs(m, lKey, layoutKey(to, m, base))) return &m;
    }
    auto made = copyMasterPage(to, from, layout, uniqueMasterId(to, QStringLiteral("L")), lKey);
    made->baseId = base ? base->id : QString();
    int count = 0;
    for (const auto &m : to.masters) count += isLayout(*m);
    made->abbr = QString::number(count + 1);
    to.masters.insert(last < 0 ? int(to.masters.size()) : last + 1, made);
    return made.get();
}

// ---- taking this presentation's look ----

// The layout of the kind `type`: on the slide master `master`, else on any.
const MasterPage *layoutLike(const Document &to, const QString &type, const QString &master)
{
    const MasterPage *found = layoutOfType(to, type, master);
    if (!found) found = layoutOfType(to, type);
    if (!found) found = layoutOfType(to, QStringLiteral("obj"), master);
    if (!found) found = layoutOfType(to, QStringLiteral("obj"));
    if (!found) {
        const QVector<MasterPage *> all = layouts(to, master);
        found = all.isEmpty() ? (layouts(to).isEmpty() ? nullptr : layouts(to).first()) : all.first();
    }
    return found;
}

// How text looks in the first run of a story.
QTextCharFormat firstFormat(const QTextDocument *sd)
{
    if (!sd) return QTextCharFormat();
    for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
        if (!b.begin().atEnd()) return b.begin().fragment().charFormat();
    return sd->begin().charFormat();
}

// Sets the text in the look `base` has (font, size, color), leaving what the writer emphasized: bold, italic, underline, a link. Weight counts as
// emphasis only where some of the text has it and some doesn't.
void restyle(QTextDocument *sd, const QTextCharFormat &base)
{
    QList<int> look{QTextFormat::FontFamilies, QTextFormat::FontFamily, QTextFormat::FontPointSize, QTextFormat::FontPixelSize, QTextFormat::ForegroundBrush, tp::ColorRefP, tp::ThemeFont};
    struct Run {
        int from, to;
        QTextCharFormat format;
    };
    QVector<Run> runs;
    QSet<int> weights;
    for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
        for (auto f = b.begin(); !f.atEnd(); ++f) {
            runs << Run{f.fragment().position(), f.fragment().position() + f.fragment().length(), f.fragment().charFormat()};
            weights.insert(runs.last().format.hasProperty(QTextFormat::FontWeight) ? runs.last().format.fontWeight() : int(QFont::Normal));
        }
    if (weights.size() < 2) look << int(QTextFormat::FontWeight);
    for (Run &r : runs) {
        for (int key : look) {
            r.format.clearProperty(key);
            if (base.hasProperty(key)) r.format.setProperty(key, base.property(key));
        }
        QTextCursor c(sd);
        c.setPosition(r.from);
        c.setPosition(r.to, QTextCursor::KeepAnchor);
        c.setCharFormat(r.format);
    }
}

// Gives the slide the look of the layout it stands on: the text of each placeholder in that layout's place in its look.
void takeLayoutLook(Document &to, Page &slide, const MasterPage &layout)
{
    for (const auto &it : slide.items) {
        if (it->type() != ItemType::Text || placeholderType(*it).isEmpty()) continue;
        const Item *place = nullptr;
        for (const auto &l : layout.items)
            if (l->role == it->role && l->type() == ItemType::Text) place = l.get();
        QTextDocument *sd = to.storyDoc(static_cast<TextItem *>(it.get())->storyId);
        if (place && sd) restyle(sd, firstFormat(to.storyDoc(static_cast<const TextItem *>(place)->storyId)));
    }
}

} // namespace

int insertSlideFrom(Document &to, int at, const Document &from, int slide, bool keepFormatting)
{
    if (slide < 0 || slide >= from.pages.size()) return -1;
    const Page &src = *from.pages[slide];
    const int index = std::clamp(at, 0, int(to.pages.size()));
    // The layout it stands on, found before the slide is in.
    const MasterPage *sourceLayout = from.masterFor(src);
    const MasterPage *layout = nullptr;
    if (keepFormatting) {
        if (sourceLayout) layout = layoutFor(to, from, *sourceLayout);
    } else {
        const QString master = to.pages.isEmpty() ? QString() : slideMasterOf(to, std::clamp(index - 1, 0, int(to.pages.size()) - 1));
        layout = layoutLike(to, sourceLayout && isLayout(*sourceLayout) ? layoutType(*sourceLayout) : QStringLiteral("obj"), master);
    }

    Carry carry(to, from, keepFormatting);
    auto page = std::make_shared<Page>();
    page->masterId = layout ? layout->id : (to.masters.isEmpty() ? QString() : to.masters.first()->id);
    page->title = src.title;
    page->hidden = src.hidden;
    page->showMasterItems = src.showMasterItems;
    page->guides = src.guides;
    page->props = src.props;
    if (keepFormatting) {
        page->background = src.background;
        carry.fill(page->background);
    }
    QHash<QString, QString> ids;
    page->items = carry.items(src.items, &ids);
    remapCharts(&page->props, ids);
    page->notesStoryId = src.notesStoryId.isEmpty() ? to.createStory() : carry.story(src.notesStoryId);
    to.pages.insert(index, page);
    slideInserted(to, index);
    if (!keepFormatting && layout) {
        applyLayout(to, *page, *layout);
        takeLayoutLook(to, *page, *layout);
        applyFooters(to, footerOptions(to), page.get());
    }
    return index;
}

QString slideTitle(const Document &d, int slide)
{
    if (slide < 0 || slide >= d.pages.size()) return QString();
    const Page &page = *d.pages[slide];
    for (const auto &it : page.items) {
        const QString type = placeholderType(*it);
        if ((type != QLatin1String("title") && type != QLatin1String("ctrTitle")) || it->type() != ItemType::Text) continue;
        const QTextDocument *sd = d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
        const QString line = sd ? sd->toPlainText().section(QChar(QChar::LineSeparator), 0, 0).section(QLatin1Char('\n'), 0, 0).trimmed() : QString();
        if (!line.isEmpty()) return line;
    }
    return page.title;
}

} // namespace jp::deck
