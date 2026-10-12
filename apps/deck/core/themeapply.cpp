#include "core/color.h"
#include "core/themes.h"
#include "core/themes_p.h"
#include "office/theme.h"
#include "text/storyio.h"
#include "text/textprops.h"

#include <QHash>
#include <QSet>
#include <algorithm>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <functional>

namespace jp::deck {

namespace detail {

MasterPage *firstSlideMaster(const Document &d)
{
    for (const auto &m : d.masters)
        if (isSlideMaster(*m)) return m.get();
    return nullptr;
}

namespace {
QByteArray decoded(const QJsonValue &v) { return QByteArray::fromBase64(v.toString().toLatin1()); }
QJsonValue encoded(const QByteArray &xml) { return QString::fromLatin1(xml.toBase64()); }
} // namespace

QByteArray masterThemeXml(const Document &d, const MasterPage &m)
{
    return decoded(&m == firstSlideMaster(d) ? d.appData.value(QStringLiteral("themeXml")) : m.props.value(QStringLiteral("themeXml")));
}

DeckTheme masterTheme(const Document &d, const MasterPage &m)
{
    if (&m == firstSlideMaster(d)) return themeOf(d);
    const QByteArray xml = masterThemeXml(d, m);
    office::Theme t;
    if (xml.isEmpty() || !office::readTheme(xml, &t)) return themeOf(d);
    DeckTheme out = defaultTheme();
    if (!t.name.isEmpty()) out.name = t.name;
    for (int i = 0; i < 12; ++i) out.colors[i] = t.colors[i];
    if (!t.majorLatin.isEmpty()) out.headingFont = t.majorLatin;
    if (!t.minorLatin.isEmpty()) out.bodyFont = t.minorLatin;
    return out;
}

void syncSchemeColors(Document &d)
{
    const MasterPage *master = firstSlideMaster(d);
    if (!master) return;
    const DeckTheme t = themeOf(d);
    const bool dark = isDarkBackground(master->background);
    d.colors.name = t.name;
    d.colors.c[Main] = dark ? t.colors[1] : t.colors[0];
    d.colors.c[Accent1] = t.colors[4];
    d.colors.c[Accent2] = t.colors[5];
    d.colors.c[Accent3] = t.colors[6];
    d.colors.c[Accent4] = t.colors[7];
    d.colors.c[Accent5] = t.colors[8];
    d.colors.c[Hyperlink] = t.colors[10];
    d.colors.c[Followed] = t.colors[11];
}

void storeMasterTheme(Document &d, MasterPage &m, const DeckTheme &t, const QByteArray &xml)
{
    if (&m == firstSlideMaster(d)) {
        setTheme(d, t);
        d.fonts = {t.name, t.headingFont, t.bodyFont};
        if (xml.isEmpty()) d.appData.remove(QStringLiteral("themeXml"));
        else d.appData[QStringLiteral("themeXml")] = encoded(xml);
        syncSchemeColors(d);
    } else {
        m.props[QStringLiteral("themeXml")] = encoded(xml.isEmpty() ? themePartXml(t) : xml);
    }
}

void storeMasterTheme(Document &d, MasterPage &m, const DeckTheme &t)
{
    const QByteArray xml = masterThemeXml(d, m);
    storeMasterTheme(d, m, t, xml.isEmpty() ? QByteArray() : patchThemePart(xml, t));
}

// ---- swapping colors and fonts ----

QColor ColorSwap::operator()(const QColor &c) const
{
    const auto it = to.constFind(c.rgb());
    if (it == to.cend()) return c;
    QColor n = it.value();
    n.setAlpha(c.alpha());
    return n;
}

ColorSwap colorSwap(const DeckTheme &from, const DeckTheme &toTheme, bool flip)
{
    ColorSwap s;
    auto add = [&s](const QColor &key, const QColor &value) {
        if (key.isValid() && !s.to.contains(key.rgb())) s.to.insert(key.rgb(), value);
    };
    // Dark 1, light 1, dark 2, light 2 trade places when the background turns dark or light.
    const int target[12] = {flip ? 1 : 0, flip ? 0 : 1, flip ? 3 : 2, flip ? 2 : 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for (int i = 0; i < 12; ++i) add(from.colors[i], toTheme.colors[target[i]]);
    // The grays of a new presentation's footers and section text are its dark 1 toward white (45 and 40 percent): the new dark 1 toward white
    // again, or its light 2 on a dark background.
    for (double toward : {0.45, 0.40}) add(mix(from.colors[0], Qt::white, toward), flip ? toTheme.colors[3] : QColor(mix(toTheme.colors[0], Qt::white, toward).rgb()));
    return s;
}

QString FontSwap::operator()(const QString &family, bool title) const
{
    if (fromHeading != fromBody) {
        if (family == fromHeading) return toHeading;
        if (family == fromBody) return toBody;
        return family;
    }
    if (family == fromHeading) return title ? toHeading : toBody;
    return family;
}

namespace {

// The plain color a run is set in, or invalid (not set, or a color of the presentation's scheme).
QColor runColor(const QTextCharFormat &cf)
{
    const QString ref = cf.stringProperty(tp::ColorRefP);
    if (!ref.isEmpty()) {
        const ColorRef r = ColorRef::fromString(ref);
        return r.kind() == ColorRef::Rgb ? r.rgbValue() : QColor();
    }
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() == Qt::SolidPattern) return cf.foreground().color();
    return QColor();
}

void setRunColor(QTextCharFormat &cf, const QColor &c)
{
    cf.setForeground(QBrush(c));
    cf.setProperty(tp::ColorRefP, ColorRef::rgb(c).toString());
}

// Changes the format of each run of a story (and of each paragraph's end) that `fn` changes.
void editRuns(QTextDocument *sd, const std::function<bool(QTextCharFormat &)> &fn)
{
    struct Run {
        int from, to;
        QTextCharFormat format;
    };
    QVector<Run> runs;
    QVector<QPair<int, QTextCharFormat>> ends;
    for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
        QTextCharFormat end = b.charFormat();
        if (fn(end)) ends << qMakePair(b.position(), end);
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            QTextCharFormat cf = it.fragment().charFormat();
            if (fn(cf)) runs << Run{it.fragment().position(), it.fragment().position() + it.fragment().length(), cf};
        }
    }
    for (const Run &r : runs) {
        QTextCursor c(sd);
        c.setPosition(r.from);
        c.setPosition(r.to, QTextCursor::KeepAnchor);
        c.setCharFormat(r.format);
    }
    for (const auto &e : ends) {
        QTextCursor c(sd);
        c.setPosition(e.first);
        c.setBlockCharFormat(e.second);
    }
}

// Swaps the old scheme's colors and fonts in a run; true if it changed.
bool swapRun(QTextCharFormat &cf, const Swap &swap, bool title, bool colorToo = true, bool fontToo = true)
{
    bool changed = false;
    if (colorToo) {
        const QColor c = runColor(cf), n = c.isValid() ? swap.colors(c) : c;
        if (c.isValid() && n != c) {
            setRunColor(cf, n);
            changed = true;
        }
    }
    if (fontToo && cf.hasProperty(QTextFormat::FontFamilies)) {
        QStringList families = cf.fontFamilies().toStringList();
        if (!families.isEmpty()) {
            const QString n = swap.fonts(families.first(), title);
            if (n != families.first()) {
                families[0] = n;
                cf.setFontFamilies(families);
                changed = true;
            }
        }
    }
    return changed;
}

void swapStory(Document &d, const QString &storyId, const Swap &swap, bool title)
{
    QTextDocument *sd = storyId.isEmpty() ? nullptr : d.storyDoc(storyId);
    if (!sd) return;
    editRuns(sd, [&](QTextCharFormat &cf) { return swapRun(cf, swap, title); });
}

void swapColor(ColorRef &c, const ColorSwap &s)
{
    if (c.kind() != ColorRef::Rgb || c.shownValue().isValid()) return;
    const QColor n = s(c.rgbValue());
    if (n != c.rgbValue()) c = ColorRef::rgb(n);
}

void swapFill(Fill &f, const ColorSwap &s)
{
    swapColor(f.color, s);
    swapColor(f.color2, s);
    for (GradientStop &st : f.stops) swapColor(st.color, s);
}

void swapStroke(Stroke &st, const ColorSwap &s) { swapColor(st.color, s); }

bool isTitleType(const QString &type) { return type == QLatin1String("title") || type == QLatin1String("ctrTitle"); }

} // namespace

void swapOnPage(Document &d, PageBase &page, const Swap &swap)
{
    if (swap.isEmpty()) return;
    swapFill(page.background, swap.colors);
    QSet<QString> stories;
    auto story = [&](const QString &id, bool title) {
        if (id.isEmpty() || stories.contains(id)) return;
        stories.insert(id);
        swapStory(d, id, swap, title);
    };
    walkItems(page.items, [&](const ItemPtr &it) {
        swapFill(it->fill, swap.colors);
        swapStroke(it->stroke, swap.colors);
        swapColor(it->fx.shadow.color, swap.colors);
        swapColor(it->fx.glow.color, swap.colors);
        const bool title = isTitleType(placeholderType(*it));
        if (it->type() == ItemType::Text) {
            story(static_cast<TextItem *>(it.get())->storyId, title);
        } else if (it->type() == ItemType::Shape) {
            story(static_cast<ShapeItem *>(it.get())->storyId, title);
        } else if (it->type() == ItemType::Table) {
            for (TableCell &c : static_cast<TableItem *>(it.get())->cells) {
                swapFill(c.fill, swap.colors);
                for (Stroke *b : {&c.border.top, &c.border.bottom, &c.border.left, &c.border.right, &c.diagonalLine}) swapStroke(*b, swap.colors);
                story(c.storyId, false);
                story(c.storyId2, false);
            }
        }
    });
}

QHash<QString, Family> familiesOf(Document &d)
{
    QHash<QString, MasterPage *> byId;
    for (auto &m : d.masters) byId.insert(m->id, m.get());
    QHash<QString, Family> out;
    for (auto &m : d.masters)
        if (isSlideMaster(*m)) out[m->id].master = m.get();
    for (auto &m : d.masters)
        if (isLayout(*m) && out.contains(m->baseId)) out[m->baseId].layouts << m.get();
    for (auto &p : d.pages) {
        const MasterPage *under = byId.value(p->masterId);
        if (!under) continue;
        const QString owner = isSlideMaster(*under) ? under->id : under->baseId;
        if (out.contains(owner)) out[owner].slides << p.get();
    }
    return out;
}

} // namespace detail

namespace {

using namespace detail;

void swapFamily(Document &d, const Family &f, const Swap &swap)
{
    swapOnPage(d, *f.master, swap);
    for (MasterPage *l : f.layouts) swapOnPage(d, *l, swap);
    for (Page *p : f.slides) swapOnPage(d, *p, swap);
}

QVector<MasterPage *> slideMasters(const Document &d)
{
    QVector<MasterPage *> out;
    for (const auto &m : d.masters)
        if (isSlideMaster(*m)) out << m.get();
    return out;
}

// ---- bringing a theme in ----

// What marks a slide master that a change of theme made for some slides: one that no slide stands on any more goes the next time.
const QString kMadeByTheme = QStringLiteral("madeByTheme");

// Copies of a slide master and its layouts of another presentation, and of what they hold: the text, pictures, and movies they use are
// copied once each.
class Copier {
public:
    Copier(Document &to, const Document &from) : m_to(to), m_from(from) {}

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
            found = m_stories.insert(id, made);
        }
        return found.value();
    }
    void fill(Fill &f) { f.imageId = image(f.imageId); }

    ItemList items(const ItemList &src)
    {
        ItemList out;
        for (const auto &it : src) out.push_back(it->clone());
        std::vector<const Item *> was;
        std::vector<Item *> now;
        walkItems(src, [&](const ItemPtr &it) { was.push_back(it.get()); });
        walkItems(out, [&](const ItemPtr &it) { now.push_back(it.get()); });
        QHash<QString, QString> ids;
        for (size_t i = 0; i < now.size() && i < was.size(); ++i) {
            now[i]->sourceId = was[i]->sourceId;
            ids.insert(was[i]->id, now[i]->id);
            carry(now[i]);
        }
        remapGlue(out, ids);
        return out;
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
            break;
        }
        default: break;
        }
    }

    Document &m_to;
    const Document &m_from;
    QHash<QString, QString> m_images, m_media, m_stories;
};

struct Copy {
    MasterPage *master = nullptr;
    QVector<MasterPage *> layouts;
};

QString freeId(const Document &d, const QString &prefix)
{
    for (int n = 1;; ++n)
        if (!d.master(prefix + QString::number(n))) return prefix + QString::number(n);
}

// A copy of slide master `src` of `from` and its layouts at the end of `to`'s masters, fitted to the slide size.
Copy copyMaster(Document &to, const Document &from, const MasterPage &src)
{
    Copier copy(to, from);
    auto page = [&](const MasterPage &s, const QString &id) {
        auto p = std::make_shared<MasterPage>();
        p->id = id;
        p->name = s.name;
        p->abbr = s.abbr;
        p->twoPage = s.twoPage;
        p->grid = s.grid;
        p->guides = s.guides;
        p->showMasterItems = s.showMasterItems;
        p->props = s.props;
        p->props.remove(QStringLiteral("themeXml"));
        p->props.remove(QStringLiteral("reuseKey"));
        p->props.remove(kMadeByTheme);
        p->background = s.background;
        copy.fill(p->background);
        p->items = copy.items(s.items);
        fitPageToSize(to, *p, from.pageSize(), to.pageSize(), false);
        return p;
    };
    Copy out;
    auto master = page(src, freeId(to, QStringLiteral("SM")));
    to.masters << master;
    out.master = master.get();
    int n = 0;
    for (const MasterPage *l : layouts(from, src.id)) {
        auto layout = page(*l, freeId(to, QStringLiteral("L")));
        layout->baseId = master->id;
        layout->abbr = QString::number(++n);
        to.masters << layout;
        out.layouts << layout.get();
    }
    return out;
}

// Takes a page's stories away with it.
void dropMaster(Document &d, const std::shared_ptr<MasterPage> &m)
{
    walkItems(m->items, [&](const ItemPtr &it) {
        if (it->type() == ItemType::Text) {
            d.removeStory(static_cast<TextItem *>(it.get())->storyId);
        } else if (it->type() == ItemType::Shape && !static_cast<ShapeItem *>(it.get())->storyId.isEmpty()) {
            d.removeStory(static_cast<ShapeItem *>(it.get())->storyId);
        } else if (it->type() == ItemType::Table) {
            for (const TableCell &c : static_cast<TableItem *>(it.get())->cells) {
                d.removeStory(c.storyId);
                d.removeStory(c.storyId2);
            }
        }
    });
    d.masters.removeOne(m);
}

// Takes away the slide masters (and their layouts) that earlier changes of theme made for some slides and no slide stands on now,
// so that trying one theme after another on the same slides doesn't leave a master behind for each; `keep` stays.
void dropUnusedThemeMasters(Document &d, const MasterPage *keep)
{
    QHash<QString, const MasterPage *> byId;
    for (const auto &m : d.masters) byId.insert(m->id, m.get());
    QSet<QString> used;
    for (const auto &p : d.pages)
        if (const MasterPage *under = byId.value(p->masterId)) used.insert(isSlideMaster(*under) ? under->id : under->baseId);
    const MasterPage *first = firstSlideMaster(d);
    const QVector<std::shared_ptr<MasterPage>> all = d.masters;
    for (const auto &m : all) {
        if (!isSlideMaster(*m) || m.get() == keep || m.get() == first || used.contains(m->id) || !m->props.value(kMadeByTheme).toBool()) continue;
        for (const auto &l : all)
            if (isLayout(*l) && l->baseId == m->id) dropMaster(d, l);
        dropMaster(d, m);
    }
}

// The layout kind a set of placeholders asks for.
QString kindOf(const ItemList &items)
{
    int title = 0, content = 0, text = 0, picture = 0;
    for (const auto &it : items) {
        const QString type = placeholderType(*it);
        if (type == QLatin1String("ctrTitle") || type == QLatin1String("subTitle")) return QStringLiteral("title");
        if (type == QLatin1String("title")) ++title;
        else if (type == QLatin1String("obj")) ++content;
        else if (type == QLatin1String("body")) ++text;
        else if (type == QLatin1String("pic")) ++picture;
    }
    const int boxes = content + text;
    if (title && picture) return QStringLiteral("picTx");
    if (title && boxes >= 4) return QStringLiteral("twoTxTwoObj");
    if (title && boxes >= 2) return QStringLiteral("twoObj");
    if (title && boxes == 1) return QStringLiteral("obj");
    if (title) return QStringLiteral("titleOnly");
    return boxes ? QStringLiteral("obj") : QStringLiteral("blank");
}

// The layout of slide master `master` that a slide on `old` goes to: the one of its kind, else the kind its placeholders ask for.
const MasterPage *targetLayout(const Document &d, const MasterPage *old, const Page &slide, const QString &master)
{
    const QString type = old && isLayout(*old) ? layoutType(*old) : QString();
    if (!type.isEmpty() && type != QLatin1String("cust"))
        if (const MasterPage *l = layoutOfType(d, type, master)) return l;
    const MasterPage *l = layoutOfType(d, kindOf(old && isLayout(*old) && !old->items.empty() ? old->items : slide.items), master);
    if (!l) l = layoutOfType(d, QStringLiteral("obj"), master);
    if (!l) {
        const QVector<MasterPage *> all = layouts(d, master);
        if (!all.isEmpty()) l = all.first();
    }
    return l;
}

// How the first run of a placeholder's text looks where a layout says.
struct Look {
    bool valid = false;
    QTextCharFormat run;
    Qt::Alignment align = Qt::AlignLeft;
};

Look lookOf(const Document &d, const Item *place)
{
    Look l;
    if (!place || place->type() != ItemType::Text) return l;
    const QTextDocument *sd = d.storyDoc(static_cast<const TextItem *>(place)->storyId);
    if (!sd) return l;
    l.valid = true;
    l.run = sd->begin().charFormat();
    l.align = sd->begin().blockFormat().alignment();
    for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
        if (!b.begin().atEnd()) {
            l.run = b.begin().fragment().charFormat();
            l.align = b.blockFormat().alignment();
            break;
        }
    return l;
}

// Placeholders found by role through a layout and its slide master (the layout's own first), the roles of each layout read once.
class Places {
public:
    explicit Places(const Document &d) : m_d(d) {}
    const Item *of(const MasterPage &layout, const QString &role)
    {
        auto it = m_roles.find(&layout);
        if (it == m_roles.end()) {
            QHash<QString, const Item *> found;
            for (const MasterPage *m : m_d.masterChain(&layout))   // the slide master, then the layout over it
                for (const auto &item : m->items)
                    if (!item->role.isEmpty()) found.insert(item->role, item.get());
            it = m_roles.insert(&layout, found);
        }
        return it->value(role);
    }

private:
    const Document &m_d;
    QHash<const MasterPage *, QHash<QString, const Item *>> m_roles;
};

// A slide's placeholders are matched to a layout's by looking through the layout for each: real slides and layouts have about ten of each
// (a hundred matches), so 90,000 (300 of each) is a thousand times that. Past it the slide is put on the layout and its placeholders stay
// where they are, so that a file can't make a change of theme take minutes.
constexpr qint64 kMaxMatches = 90000;

// Text that looked as the old layout said looks as the new layout says; whatever else was set stays, except that the old scheme's
// colors and fonts become the new scheme's.
void restyleStory(Document &d, const QString &storyId, const Look &old, const Look &now, const Swap &swap, bool title)
{
    QTextDocument *sd = storyId.isEmpty() ? nullptr : d.storyDoc(storyId);
    if (!sd) return;
    static const int kProps[] = {QTextFormat::FontFamilies, QTextFormat::FontPointSize, QTextFormat::FontWeight, QTextFormat::FontItalic,
                                 QTextFormat::FontCapitalization, tp::ThemeFont};
    editRuns(sd, [&](QTextCharFormat &cf) {
        const QTextCharFormat was = cf;
        bool colorDone = false, fontDone = false;
        if (old.valid && now.valid) {
            for (int p : kProps) {
                const bool has = was.hasProperty(p);
                if (has != old.run.hasProperty(p) || (has && was.property(p) != old.run.property(p))) continue;
                if (now.run.hasProperty(p)) cf.setProperty(p, now.run.property(p));
                else cf.clearProperty(p);
                if (p == QTextFormat::FontFamilies) fontDone = true;
            }
            if (runColor(was) == runColor(old.run)) {
                const QColor n = runColor(now.run);
                if (n.isValid()) {
                    setRunColor(cf, n);
                } else {
                    cf.clearProperty(tp::ColorRefP);
                    cf.clearProperty(QTextFormat::ForegroundBrush);
                }
                colorDone = true;
            }
        }
        // What didn't follow the old layout: the old scheme's colors and fonts, where it used them.
        QTextCharFormat rest = was;
        if (swapRun(rest, swap, title, !colorDone, !fontDone)) {
            if (!colorDone && rest.stringProperty(tp::ColorRefP) != was.stringProperty(tp::ColorRefP)) {
                cf.setForeground(rest.foreground());
                cf.setProperty(tp::ColorRefP, rest.stringProperty(tp::ColorRefP));
            }
            if (!fontDone && rest.fontFamilies() != was.fontFamilies()) cf.setFontFamilies(rest.fontFamilies().toStringList());
        }
        return cf != was;
    });
    if (old.valid && now.valid && old.align != now.align) {
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
            if (b.blockFormat().alignment() != old.align) continue;
            QTextBlockFormat bf;
            bf.setAlignment(now.align);
            QTextCursor(b).mergeBlockFormat(bf);
        }
    }
}

// Puts a slide on `layout`, as applyLayout does, and sets its text in the new look.
void moveSlide(Document &d, Page &slide, const MasterPage *old, const MasterPage &layout, const Swap &swap, Places &places)
{
    qint64 placeholders = 0;
    for (const auto &it : slide.items) placeholders += !placeholderType(*it).isEmpty();
    if (placeholders * qint64(layout.items.size()) > kMaxMatches) {
        slide.masterId = layout.id;
        swapOnPage(d, slide, swap);
        return;
    }
    QHash<const Item *, Look> was;
    QSet<const Item *> before, empty;
    walkItems(slide.items, [&](const ItemPtr &it) {
        before.insert(it.get());
        if (old && !placeholderType(*it).isEmpty()) was.insert(it.get(), lookOf(d, places.of(*old, it->role)));
        if (it->type() == ItemType::Text && !placeholderType(*it).isEmpty())
            if (const QTextDocument *sd = d.storyDoc(static_cast<const TextItem *>(it.get())->storyId); sd && sd->isEmpty()) empty.insert(it.get());
    });
    applyLayout(d, slide, layout);
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (!before.contains(it.get())) return;   // made from the new layout: already in its look
        // An empty placeholder that has a place in the new layout was given its look (the first paragraph's, with its list) by applyLayout.
        if (empty.contains(it.get()) && places.of(layout, it->role)) return;
        const QString type = placeholderType(*it);
        const bool title = isTitleType(type);
        Look now;
        if (!type.isEmpty()) now = lookOf(d, places.of(layout, it->role));
        const Look &then = was.contains(it.get()) ? was[it.get()] : Look();
        if (it->type() == ItemType::Text) {
            restyleStory(d, static_cast<TextItem *>(it.get())->storyId, then, now, swap, title);
        } else if (it->type() == ItemType::Shape) {
            swapStory(d, static_cast<ShapeItem *>(it.get())->storyId, swap, false);
        } else if (it->type() == ItemType::Table) {
            for (const TableCell &c : static_cast<TableItem *>(it.get())->cells) {
                swapStory(d, c.storyId, swap, false);
                swapStory(d, c.storyId2, swap, false);
            }
        }
        swapFill(it->fill, swap.colors);
        swapStroke(it->stroke, swap.colors);
    });
}

// The slide master a slide stands on, and its theme.
struct Standing {
    const MasterPage *layout = nullptr;
    DeckTheme theme;
    bool dark = false;
};

// Where each slide stands, the theme of each slide master read once.
class Standings {
public:
    explicit Standings(const Document &d) : m_d(d) {}
    Standing of(const Page &slide)
    {
        Standing s;
        s.layout = m_d.masterFor(slide);
        const MasterPage *master = s.layout ? (isSlideMaster(*s.layout) ? s.layout : m_d.master(s.layout->baseId)) : firstSlideMaster(m_d);
        auto it = m_themes.find(master);
        if (it == m_themes.end()) it = m_themes.insert(master, qMakePair(master ? masterTheme(m_d, *master) : themeOf(m_d), master && isDarkBackground(master->background)));
        s.theme = it->first;
        s.dark = it->second;
        return s;
    }

private:
    const Document &m_d;
    QHash<const MasterPage *, QPair<DeckTheme, bool>> m_themes;
};

Swap swapFor(const Standing &was, const DeckTheme &to, bool toDark)
{
    Swap swap;
    swap.colors = colorSwap(was.theme, to, was.dark != toDark);
    swap.fonts = FontSwap{was.theme.headingFont, was.theme.bodyFont, to.headingFont, to.bodyFont};
    return swap;
}

} // namespace

bool applyThemeToAll(Document &d, const Document &source)
{
    const MasterPage *src = firstSlideMaster(source);
    if (!src) return false;
    DeckTheme target = masterTheme(source, *src);
    const QByteArray targetXml = masterThemeXml(source, *src);
    QVector<Standing> was;
    Standings standings(d);
    for (const auto &p : d.pages) was << standings.of(*p);
    const QVector<std::shared_ptr<MasterPage>> olds = d.masters;
    const Copy made = copyMaster(d, source, *src);
    keepLinksReadable(target, made.master->background);
    const bool dark = isDarkBackground(made.master->background);
    Places places(d);
    for (int i = 0; i < d.pages.size(); ++i) {
        Page &slide = *d.pages[i];
        const MasterPage *layout = targetLayout(d, was[i].layout, slide, made.master->id);
        if (layout) moveSlide(d, slide, was[i].layout, *layout, swapFor(was[i], target, dark), places);
    }
    for (const auto &m : olds) dropMaster(d, m);
    storeMasterTheme(d, *made.master, target, targetXml.isEmpty() ? QByteArray() : patchThemePart(targetXml, target));
    return true;
}

bool applyThemeToSlides(Document &d, const Document &source, const QList<int> &slides)
{
    const MasterPage *src = firstSlideMaster(source);
    if (!src) return false;
    QSet<int> chosen;
    for (int i : slides)
        if (i >= 0 && i < d.pages.size()) chosen.insert(i);
    if (chosen.isEmpty()) return true;
    if (chosen.size() == d.pages.size()) return applyThemeToAll(d, source);
    DeckTheme target = masterTheme(source, *src);
    const QByteArray targetXml = masterThemeXml(source, *src);
    QVector<Standing> was;
    Standings standings(d);
    for (const auto &p : d.pages) was << standings.of(*p);
    const Copy made = copyMaster(d, source, *src);
    made.master->props[kMadeByTheme] = true;
    keepLinksReadable(target, made.master->background);
    const bool dark = isDarkBackground(made.master->background);
    Places places(d);
    for (int i = 0; i < d.pages.size(); ++i) {
        if (!chosen.contains(i)) continue;
        Page &slide = *d.pages[i];
        const MasterPage *layout = targetLayout(d, was[i].layout, slide, made.master->id);
        if (layout) moveSlide(d, slide, was[i].layout, *layout, swapFor(was[i], target, dark), places);
    }
    storeMasterTheme(d, *made.master, target, targetXml.isEmpty() ? QByteArray() : patchThemePart(targetXml, target));
    dropUnusedThemeMasters(d, made.master);
    return true;
}

void applyColors(Document &d, const ColorSet &colors)
{
    QHash<QString, Family> families = familiesOf(d);
    for (MasterPage *m : slideMasters(d)) {
        const DeckTheme from = masterTheme(d, *m);
        DeckTheme to = from;
        for (int i = 0; i < 12; ++i) to.colors[i] = colors.colors[i];
        // A background in one of the twelve styles follows the new colors; any other is swapped like the rest.
        const int style = backgroundStyleOf(m->background, from);
        Fill background = m->background;
        if (style >= 0) {
            background = backgroundStyle(to, style);
        } else {
            swapFill(background, colorSwap(from, to, false));
        }
        keepLinksReadable(to, background);
        Swap swap;
        swap.colors = colorSwap(from, to, false);
        swapFamily(d, families.value(m->id), swap);
        m->background = background;
        storeMasterTheme(d, *m, to);
    }
}

void applyFonts(Document &d, const FontSet &fonts)
{
    QHash<QString, Family> families = familiesOf(d);
    for (MasterPage *m : slideMasters(d)) {
        const DeckTheme from = masterTheme(d, *m);
        DeckTheme to = from;
        to.headingFont = fonts.heading;
        to.bodyFont = fonts.body;
        Swap swap;
        swap.fonts = FontSwap{from.headingFont, from.bodyFont, to.headingFont, to.bodyFont};
        swapFamily(d, families.value(m->id), swap);
        storeMasterTheme(d, *m, to);
    }
}

void applyBackgroundStyle(Document &d, int style)
{
    style = std::clamp(style, 0, kBackgroundStyles - 1);
    QHash<QString, Family> families = familiesOf(d);
    for (MasterPage *m : slideMasters(d)) {
        const DeckTheme from = masterTheme(d, *m);
        const Fill background = backgroundStyle(from, style);
        DeckTheme to = from;
        keepLinksReadable(to, background);
        Swap swap;
        swap.colors = colorSwap(from, to, isDarkBackground(m->background) != backgroundStyleIsDark(style));
        const Family family = families.value(m->id);
        swapFamily(d, family, swap);
        m->background = background;
        for (MasterPage *l : family.layouts) l->background = Fill();
        for (Page *p : family.slides) p->background = Fill();
        storeMasterTheme(d, *m, to);
    }
}

void keepTextReadable(Document &d)
{
    const MasterPage *master = firstSlideMaster(d);
    if (master && isDarkBackground(master->background)) d.colors.c[Main] = themeOf(d).colors[1];
}

void applyVariant(Document &d, const Variant &v)
{
    applyColors(d, v.colors);
    applyBackgroundStyle(d, v.background);
}

std::unique_ptr<Document> themeSample(const Document &d)
{
    auto s = newPresentation();
    applyThemeToAll(*s, d);
    sayOnTitleSlide(*s, themeOf(d).name);
    return s;
}

std::unique_ptr<Document> variantSample(const Document &d, int variant)
{
    const QVector<Variant> variants = variantsOf(d);
    if (variant < 0 || variant >= variants.size()) return themeSample(d);
    if (const int theme = builtinThemeIndex(themeOf(d).name); theme >= 0) return themePresentation(theme, variant);
    auto s = themeSample(d);
    applyVariant(*s, variants[variant]);
    return s;
}

ColorSet colorSetOf(const Document &d)
{
    const DeckTheme t = themeOf(d);
    ColorSet c;
    c.name = t.name;
    for (int i = 0; i < 12; ++i) c.colors[i] = t.colors[i];
    return c;
}

FontSet fontSetOf(const Document &d)
{
    const DeckTheme t = themeOf(d);
    return {t.name, t.headingFont, t.bodyFont};
}

QVector<Variant> variantsOf(const Document &d)
{
    const int theme = builtinThemeIndex(themeOf(d).name);
    if (theme >= 0) return builtinThemeVariants(theme);
    // Its colors on four backgrounds: Light 1, a graded Light 2, Dark 2, and a strongly graded Dark 1.
    QVector<Variant> out;
    for (int style : {0, 4, 6, 11}) out << Variant{colorSetOf(d), style};
    return out;
}

int currentVariant(const Document &d)
{
    const MasterPage *master = firstSlideMaster(d);
    if (!master) return -1;
    const DeckTheme theme = themeOf(d);
    const int style = backgroundStyleOf(master->background, theme);
    const QVector<Variant> variants = variantsOf(d);
    for (int v = 0; v < variants.size(); ++v) {
        bool same = variants[v].background == style;
        for (int c = 0; c < 10 && same; ++c) same = variants[v].colors.colors[c] == theme.colors[c];
        if (same) return v;
    }
    return -1;
}

} // namespace jp::deck
