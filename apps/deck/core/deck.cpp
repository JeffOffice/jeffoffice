#include "core/deck.h"

#include "core/color.h"
#include "core/sections.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>

namespace jp::deck {

namespace {

const QString kPh = QStringLiteral("ph:");

// How a placeholder's text looks.
struct Look {
    double size = 18;
    bool heading = false;   // the theme's heading font, else its body font
    bool bold = false;
    int color = 0;          // a theme color: 0 dark 1, 2 dark 2...
    int tint = 0;           // percent toward white
    Qt::Alignment align = Qt::AlignLeft;
};

// A placeholder as a layout places it.
struct Place {
    QString type;
    int index = 0;
    QRectF rect;
    Look look;
    VAlign valign = VAlign::Top;
    QString sample;          // what it says on the layout itself
};

QColor tinted(const QColor &c, int tint) { return tint ? mix(c, Qt::white, tint / 100.0) : c; }

void styleStory(Document &d, const QString &storyId, const Look &look, const DeckTheme &theme)
{
    QTextDocument *sd = d.storyDoc(storyId);
    if (!sd) return;
    QTextCharFormat cf;
    cf.setFontFamilies({look.heading ? theme.headingFont : theme.bodyFont});
    cf.setFontPointSize(look.size);
    cf.setFontWeight(look.bold ? QFont::Bold : QFont::Normal);
    const QColor color = tinted(theme.colors[std::clamp(look.color, 0, 11)], look.tint);
    cf.setForeground(color);
    cf.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
    QTextBlockFormat bf;
    bf.setAlignment(look.align);
    QTextCursor c(sd);
    c.select(QTextCursor::Document);
    c.mergeCharFormat(cf);
    c.mergeBlockFormat(bf);
    QTextCursor start(sd);
    start.mergeBlockCharFormat(cf);
}

// How a placeholder sets a paragraph: the paragraph's own format, the characters it starts with, and the list it is in (bullet, font, color,
// size, and level).
struct ParaLook {
    bool valid = false;
    QTextBlockFormat block;
    QTextCharFormat chars;
    bool listed = false;
    QTextListFormat list;
};

// The look of paragraph `b`; its characters are those of its first letter (the block's own when it has none).
ParaLook paraLook(const QTextBlock &b)
{
    ParaLook p;
    if (!b.isValid()) return p;
    p.valid = true;
    p.block = b.blockFormat();
    p.block.clearProperty(QTextFormat::ObjectIndex);   // the list it is in is another story's
    p.chars = b.begin().atEnd() ? b.charFormat() : b.begin().fragment().charFormat();
    if (const QTextList *l = b.textList()) {
        p.listed = true;
        p.list = l->format();
    }
    return p;
}

// The first paragraph of a story takes the look, as what is typed in it will have it.
void givePara(QTextDocument *sd, const ParaLook &look)
{
    if (!look.valid) return;
    QTextBlock first = sd->begin();
    if (QTextList *l = first.textList()) l->remove(first);
    QTextCursor c(sd);
    c.setBlockFormat(look.block);
    c.setBlockCharFormat(look.chars);
    if (look.listed) c.createList(look.list);
}

// The text cleared, its formatting kept for what is typed next: the first paragraph's, with its list.
void clearStory(Document &d, const QString &storyId)
{
    QTextDocument *sd = d.storyDoc(storyId);
    if (!sd || sd->isEmpty()) return;
    const ParaLook first = paraLook(sd->begin());
    sd->clear();
    givePara(sd, first);
}

ItemPtr placeholder(Document &d, const Place &pl, const DeckTheme &theme)
{
    auto t = std::make_shared<TextItem>();
    t->rect = pl.rect;
    t->role = placeholderRole(pl.type, pl.index);
    t->name = QCoreApplication::translate("Deck", "%1 Placeholder").arg(promptFor(pl.type).isEmpty() ? pl.type : pl.type);
    t->valign = pl.valign;
    t->insets = QMarginsF(7.2, 3.6, 7.2, 3.6);
    t->autofit = TextItem::ShrinkOnOverflow;
    t->hyphenate = false;
    t->storyId = d.createStory(pl.sample);
    styleStory(d, t->storyId, pl.look, theme);
    return t;
}

Look titleLook() { Look l; l.size = 44; l.heading = true; return l; }
Look bodyLook(double size = 28) { Look l; l.size = size; return l; }
Look smallLook(Qt::Alignment a) { Look l; l.size = 12; l.color = 0; l.tint = 45; l.align = a; return l; }

const QString kTitleSample = QStringLiteral("Click to edit Master title style");
const QString kBodySample = QStringLiteral("Click to edit Master text styles\nSecond level\nThird level\nFourth level\nFifth level");

QVector<Place> footers()
{
    return {{QStringLiteral("dt"), 0, QRectF(66, 500.5, 216, 28.75), smallLook(Qt::AlignLeft), VAlign::Middle, QString()},
            {QStringLiteral("ftr"), 0, QRectF(318, 500.5, 324, 28.75), smallLook(Qt::AlignHCenter), VAlign::Middle, QString()},
            {QStringLiteral("sldNum"), 0, QRectF(678, 500.5, 216, 28.75), smallLook(Qt::AlignRight), VAlign::Middle, QString()}};
}

struct LayoutDef {
    QString type, name;
    QVector<Place> places;
};

QVector<LayoutDef> layoutDefs()
{
    const QRectF title(66, 28.75, 828, 104.4), body(66, 143.75, 828, 342.6);
    Look big = titleLook();
    big.size = 60;
    Look sub = bodyLook(24);
    sub.align = Qt::AlignHCenter;
    Look centerBig = big;
    centerBig.align = Qt::AlignHCenter;
    Look head = bodyLook(24);
    head.bold = true;
    Look caption = titleLook();
    caption.size = 32;
    Look note = bodyLook(16);
    Look sectionText = bodyLook(24);
    sectionText.color = 0;
    sectionText.tint = 40;
    const QString t = QStringLiteral("title"), b = QStringLiteral("body"), o = QStringLiteral("obj");
    return {
        {QStringLiteral("title"), QStringLiteral("Title Slide"),
         {{QStringLiteral("ctrTitle"), 0, QRectF(120, 88.4, 720, 188), centerBig, VAlign::Bottom, kTitleSample},
          {QStringLiteral("subTitle"), 1, QRectF(120, 283.6, 720, 130.4), sub, VAlign::Top, QStringLiteral("Click to edit Master subtitle style")}}},
        {QStringLiteral("obj"), QStringLiteral("Title and Content"),
         {{t, 0, title, titleLook(), VAlign::Middle, kTitleSample}, {o, 1, body, bodyLook(), VAlign::Top, kBodySample}}},
        {QStringLiteral("secHead"), QStringLiteral("Section Header"),
         {{t, 0, QRectF(65.5, 134.6, 828, 224.6), big, VAlign::Bottom, kTitleSample},
          {b, 1, QRectF(65.5, 361.4, 828, 118.1), sectionText, VAlign::Top, QStringLiteral("Click to edit Master text styles")}}},
        {QStringLiteral("twoObj"), QStringLiteral("Two Content"),
         {{t, 0, title, titleLook(), VAlign::Middle, kTitleSample},
          {o, 1, QRectF(66, 143.75, 408, 342.6), bodyLook(), VAlign::Top, kBodySample},
          {o, 2, QRectF(486, 143.75, 408, 342.6), bodyLook(), VAlign::Top, kBodySample}}},
        {QStringLiteral("twoTxTwoObj"), QStringLiteral("Comparison"),
         {{t, 0, QRectF(66.1, 28.75, 828, 104.4), titleLook(), VAlign::Middle, kTitleSample},
          {b, 1, QRectF(66.1, 132.4, 406.1, 64.9), head, VAlign::Bottom, QStringLiteral("Click to edit Master text styles")},
          {o, 2, QRectF(66.1, 197.3, 406.1, 290.2), bodyLook(24), VAlign::Top, kBodySample},
          {b, 3, QRectF(486, 132.4, 408.1, 64.9), head, VAlign::Bottom, QStringLiteral("Click to edit Master text styles")},
          {o, 4, QRectF(486, 197.3, 408.1, 290.2), bodyLook(24), VAlign::Top, kBodySample}}},
        {QStringLiteral("titleOnly"), QStringLiteral("Title Only"), {{t, 0, title, titleLook(), VAlign::Middle, kTitleSample}}},
        {QStringLiteral("blank"), QStringLiteral("Blank"), {}},
        {QStringLiteral("objTx"), QStringLiteral("Content with Caption"),
         {{t, 0, QRectF(66.1, 36, 309.6, 126), caption, VAlign::Bottom, kTitleSample},
          {o, 1, QRectF(408.1, 77.8, 486, 383.8), bodyLook(32), VAlign::Top, kBodySample},
          {b, 2, QRectF(66.1, 162, 309.6, 300.1), note, VAlign::Top, QStringLiteral("Click to edit Master text styles")}}},
        {QStringLiteral("picTx"), QStringLiteral("Picture with Caption"),
         {{t, 0, QRectF(66.1, 36, 309.6, 126), caption, VAlign::Bottom, kTitleSample},
          {QStringLiteral("pic"), 1, QRectF(408.1, 77.8, 486, 383.8), bodyLook(32), VAlign::Top, QString()},
          {b, 2, QRectF(66.1, 162, 309.6, 300.1), note, VAlign::Top, QStringLiteral("Click to edit Master text styles")}}},
    };
}

// Types that take each other's places when a slide changes layout.
QString family(const QString &type)
{
    if (type == QLatin1String("ctrTitle")) return QStringLiteral("title");
    if (type == QLatin1String("obj") || type == QLatin1String("subTitle")) return QStringLiteral("body");
    return type;
}

// The placeholder's index as written ("0" without one): files use numbers
// past what an int holds (4294967295), so it's compared as text.
QString placeholderIndex(const Item &it)
{
    const QStringList parts = it.role.split(QLatin1Char(':'));
    return parts.size() > 2 && !parts[2].isEmpty() ? parts[2] : QStringLiteral("0");
}

bool isFooterType(const QString &type)
{
    return type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum");
}

// The layout's placeholder for a slide's: the same type and index, else the
// first of the same family not yet taken.
Item *matchIn(const MasterPage &layout, const Item &it, QSet<const Item *> &taken)
{
    const QString type = placeholderType(it), fam = family(type);
    const QString index = placeholderIndex(it);
    for (const auto &l : layout.items)
        if (!taken.contains(l.get()) && placeholderType(*l) == type && placeholderIndex(*l) == index) { taken.insert(l.get()); return l.get(); }
    for (const auto &l : layout.items)
        if (!taken.contains(l.get()) && !placeholderType(*l).isEmpty() && family(placeholderType(*l)) == fam) { taken.insert(l.get()); return l.get(); }
    return nullptr;
}

ItemPtr slidePlaceholder(Document &d, const Item &fromLayout)
{
    ItemPtr copy = d.cloneItem(fromLayout);
    if (copy->type() == ItemType::Text) {
        auto *t = static_cast<TextItem *>(copy.get());
        clearStory(d, t->storyId);
        t->prompt = promptFor(placeholderType(fromLayout));
    }
    return copy;
}

// The layout's own placeholder of that type, else its slide master's.
const Item *layoutPlaceholder(const Document &d, const MasterPage &layout, const QString &type)
{
    const QVector<MasterPage *> chain = d.masterChain(&layout);
    for (int i = int(chain.size()) - 1; i >= 0; --i)
        for (const auto &it : chain[i]->items)
            if (placeholderType(*it) == type) return it.get();
    return nullptr;
}

// A footer placeholder's content: the footer text, or the date or slide
// number as the field that keeps it current.
void fillFooter(Document &d, Item &it, const QString &type, const FooterOptions &o)
{
    if (it.type() != ItemType::Text) return;
    const QString storyId = static_cast<TextItem &>(it).storyId;
    clearStory(d, storyId);
    QTextDocument *sd = d.storyDoc(storyId);
    if (!sd) return;
    QTextCursor c(sd);
    if (type == QLatin1String("ftr")) {
        c.insertText(o.footerText);
        return;
    }
    QTextCharFormat f = c.charFormat();
    f.setProperty(tp::Field, type == QLatin1String("sldNum") ? QStringLiteral("page") : QStringLiteral("date"));
    c.insertText(QString(QChar::ObjectReplacementCharacter), f);
}

// The text of a story at a new size: each run's point size times `k`.
void scaleStoryText(QTextDocument *sd, double k)
{
    struct Run { int from, to; double size; };
    QVector<Run> runs;
    QVector<QPair<int, double>> blocks;
    for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
        if (b.charFormat().hasProperty(QTextFormat::FontPointSize)) blocks << qMakePair(b.position(), b.charFormat().fontPointSize());
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (fr.charFormat().hasProperty(QTextFormat::FontPointSize)) runs << Run{fr.position(), fr.position() + fr.length(), fr.charFormat().fontPointSize()};
        }
    }
    for (const auto &b : blocks) {
        QTextCursor c(sd);
        c.setPosition(b.first);
        QTextCharFormat f;
        f.setFontPointSize(b.second * k);
        c.mergeBlockCharFormat(f);
    }
    for (const Run &r : runs) {
        QTextCursor c(sd);
        c.setPosition(r.from);
        c.setPosition(r.to, QTextCursor::KeepAnchor);
        QTextCharFormat f;
        f.setFontPointSize(r.size * k);
        c.mergeCharFormat(f);
    }
}

// Everything on one master or slide at a new size: stretched to the new
// shape (`whole`), or, on a slide, objects that aren't placeholders kept in
// shape, scaled by `k` and centered (`fitted`). Text sizes and outline
// weights go by `k`.
void scaleItems(Document &d, ItemList &items, const QRectF &from, const QRectF &whole, const QRectF &fitted, double k, bool onSlide, QSet<QString> &stories)
{
    for (const auto &it : items) it->scaleInto(from, onSlide && placeholderType(*it).isEmpty() ? fitted : whole);
    walkItems(items, [&](const ItemPtr &it) {
        if (!it->stroke.isNone()) it->stroke.width *= k;
        QStringList ids;
        if (it->type() == ItemType::Text) {
            auto *t = static_cast<TextItem *>(it.get());
            t->insets = QMarginsF(t->insets.left() * k, t->insets.top() * k, t->insets.right() * k, t->insets.bottom() * k);
            ids << t->storyId;
        } else if (it->type() == ItemType::Shape) {
            ids << static_cast<ShapeItem *>(it.get())->storyId;
        } else if (it->type() == ItemType::Table) {
            for (const TableCell &c : static_cast<TableItem *>(it.get())->cells) ids << c.storyId << c.storyId2;
        }
        for (const QString &id : ids)
            if (!id.isEmpty() && !stories.contains(id)) {
                stories.insert(id);
                if (QTextDocument *sd = d.storyDoc(id)) scaleStoryText(sd, k);
            }
    });
}

void scaleGuides(RulerGuides &g, double sx, double sy)
{
    for (double &y : g.h) y *= sy;
    for (double &x : g.v) x *= sx;
}

} // namespace

QJsonObject DeckTheme::toJson() const
{
    QJsonArray cs;
    for (const QColor &c : colors) cs.append(colorToString(c));
    return {{"name", name}, {"colors", cs}, {"heading", headingFont}, {"body", bodyFont}};
}

DeckTheme DeckTheme::fromJson(const QJsonObject &o)
{
    DeckTheme t = defaultTheme();
    if (o.isEmpty()) return t;
    t.name = o["name"].toString(t.name);
    const QJsonArray cs = o["colors"].toArray();
    for (int i = 0; i < 12 && i < cs.size(); ++i) {
        const QColor c = colorFromString(cs[i].toString());
        if (c.isValid()) t.colors[i] = c;
    }
    t.headingFont = o["heading"].toString(t.headingFont);
    t.bodyFont = o["body"].toString(t.bodyFont);
    return t;
}

DeckTheme defaultTheme()
{
    DeckTheme t;
    t.name = QStringLiteral("JeffOffice");
    const char *hex[12] = {"#1F2937", "#FFFFFF", "#24364F", "#E8ECF2", "#1D55C4", "#B6420F",
                           "#13773A", "#9E1F63", "#0A6C8C", "#5E2CA5", "#1D55C4", "#5E2CA5"};
    for (int i = 0; i < 12; ++i) t.colors[i] = QColor(QLatin1String(hex[i]));
    t.headingFont = t.bodyFont = QStringLiteral("Carlito");
    return t;
}

DeckTheme themeOf(const Document &d) { return DeckTheme::fromJson(d.appData.value(QStringLiteral("theme")).toObject()); }

void setTheme(Document &d, const DeckTheme &t) { d.appData[QStringLiteral("theme")] = t.toJson(); }

QString placeholderType(const Item &it)
{
    return it.role.startsWith(kPh) ? it.role.mid(kPh.size()).section(QLatin1Char(':'), 0, 0) : QString();
}

QString placeholderRole(const QString &type, int index)
{
    return index ? kPh + type + QLatin1Char(':') + QString::number(index) : kPh + type;
}

QString promptFor(const QString &type)
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return QCoreApplication::translate("Deck", "Click to add title");
    if (type == QLatin1String("subTitle")) return QCoreApplication::translate("Deck", "Click to add subtitle");
    if (type == QLatin1String("body") || type == QLatin1String("obj")) return QCoreApplication::translate("Deck", "Click to add text");
    if (type == QLatin1String("pic")) return QCoreApplication::translate("Deck", "Click to add a picture");
    return QString();
}

bool isSlideMaster(const MasterPage &m) { return m.props.value(QStringLiteral("kind")).toString() == QLatin1String("master"); }
bool isLayout(const MasterPage &m) { return m.props.value(QStringLiteral("kind")).toString() == QLatin1String("layout"); }
QString layoutType(const MasterPage &m) { return m.props.value(QStringLiteral("type")).toString(); }

QVector<MasterPage *> layouts(const Document &d, const QString &masterId)
{
    QVector<MasterPage *> out;
    for (const auto &m : d.masters)
        if (isLayout(*m) && (masterId.isEmpty() || m->baseId == masterId)) out << m.get();
    return out;
}

QString slideMasterOf(const Document &d, int slide)
{
    if (slide < 0 || slide >= d.pages.size()) return QString();
    const MasterPage *layout = d.masterFor(*d.pages[slide]);
    return layout ? (isSlideMaster(*layout) ? layout->id : layout->baseId) : QString();
}

MasterPage *layoutOfType(const Document &d, const QString &type, const QString &masterId)
{
    for (MasterPage *m : layouts(d, masterId))
        if (layoutType(*m) == type) return m;
    return nullptr;
}

std::unique_ptr<Document> newPresentation()
{
    auto d = std::make_unique<Document>();
    d->slideText = true;
    d->masters.clear();
    d->setup.size = d->setup.sheet = QSizeF(kSlideWidth, kSlideHeight);
    d->setup.sizeName = QStringLiteral("Widescreen");
    d->setup.margins = QMarginsF(0, 0, 0, 0);
    const DeckTheme theme = defaultTheme();
    setTheme(*d, theme);
    d->fonts = {theme.name, theme.headingFont, theme.bodyFont};

    auto master = std::make_shared<MasterPage>();
    master->id = QStringLiteral("SM1");
    master->name = QStringLiteral("Slide Master");
    master->abbr = QStringLiteral("SM");
    master->props[QStringLiteral("kind")] = QStringLiteral("master");
    master->background = Fill::solid(ColorRef::rgb(theme.colors[1]));
    master->items.push_back(placeholder(*d, {QStringLiteral("title"), 0, QRectF(66, 28.75, 828, 104.4), titleLook(), VAlign::Middle, kTitleSample}, theme));
    master->items.push_back(placeholder(*d, {QStringLiteral("body"), 1, QRectF(66, 143.75, 828, 342.6), bodyLook(), VAlign::Top, kBodySample}, theme));
    for (const Place &f : footers()) master->items.push_back(placeholder(*d, f, theme));
    d->masters << master;

    int n = 0;
    for (const LayoutDef &def : layoutDefs()) {
        auto layout = std::make_shared<MasterPage>();
        layout->id = QStringLiteral("L%1").arg(++n);
        layout->name = def.name;
        layout->abbr = QString::number(n);
        layout->baseId = master->id;
        layout->props[QStringLiteral("kind")] = QStringLiteral("layout");
        layout->props[QStringLiteral("type")] = def.type;
        for (const Place &pl : def.places) layout->items.push_back(placeholder(*d, pl, theme));
        for (const Place &f : footers()) layout->items.push_back(placeholder(*d, f, theme));
        d->masters << layout;
    }
    addSlide(*d, *layoutOfType(*d, QStringLiteral("title")));
    return d;
}

bool ensureASlide(Document &d)
{
    if (!d.pages.isEmpty()) return true;
    const MasterPage *layout = layoutOfType(d, QStringLiteral("title"));
    if (!layout) layout = layouts(d).value(0);
    if (!layout && !d.masters.isEmpty()) layout = d.masters.first().get();
    if (!layout) return false;
    addSlide(d, *layout);
    return true;
}

int addSlide(Document &d, const MasterPage &layout, int at)
{
    auto slide = std::make_shared<Page>();
    slide->masterId = layout.id;
    const FooterOptions footers = footerOptions(d);
    const bool chosen = d.appData.contains(QStringLiteral("footerOptions"));   // else the footers are only empty places
    for (const auto &it : layout.items) {
        const QString type = placeholderType(*it);
        if (type.isEmpty() || (isFooterType(type) && !footers.shows(type, layoutType(layout)))) continue;
        slide->items.push_back(slidePlaceholder(d, *it));
        if (chosen && isFooterType(type)) fillFooter(d, *slide->items.back(), type, footers);
    }
    slide->notesStoryId = d.createStory();
    const int index = at < 0 || at > d.pages.size() ? int(d.pages.size()) : at;
    d.pages.insert(index, slide);
    slideInserted(d, index);
    return index;
}

int duplicateSlide(Document &d, int index)
{
    if (index < 0 || index >= d.pages.size()) return -1;
    const Page &from = *d.pages[index];
    auto slide = std::make_shared<Page>();
    slide->masterId = from.masterId;
    slide->background = from.background;
    slide->showMasterItems = from.showMasterItems;
    slide->props = from.props;
    slide->hidden = from.hidden;
    slide->title = from.title;
    slide->items = d.cloneItems(from.items);
    // The copies stand where the originals do, on a slide of their own: they
    // keep the shape numbers the slide's animations aim at (numbers count
    // within a slide), and the charts the slide keeps are the copies' now.
    std::vector<const Item *> originals;
    std::vector<Item *> copies;
    walkItems(from.items, [&](const ItemPtr &it) { originals.push_back(it.get()); });
    walkItems(slide->items, [&](const ItemPtr &it) { copies.push_back(it.get()); });
    if (originals.size() == copies.size()) {
        QHash<QString, QString> ids;
        for (size_t i = 0; i < copies.size(); ++i) {
            copies[i]->sourceId = originals[i]->sourceId;
            ids.insert(originals[i]->id, copies[i]->id);
        }
        QJsonArray charts = slide->props.value(QStringLiteral("charts")).toArray();
        for (int i = 0; i < charts.size(); ++i) {
            QJsonObject c = charts[i].toObject();
            c[QStringLiteral("item")] = ids.value(c.value(QStringLiteral("item")).toString());
            charts[i] = c;
        }
        if (!charts.isEmpty()) slide->props[QStringLiteral("charts")] = charts;
    }
    slide->notesStoryId = from.notesStoryId.isEmpty() ? d.createStory() : d.copyStory(from.notesStoryId);
    d.pages.insert(index + 1, slide);
    slideInserted(d, index + 1);
    return index + 1;
}

void applyLayout(Document &d, Page &slide, const MasterPage &layout)
{
    slide.masterId = layout.id;
    QSet<const Item *> taken;
    for (const auto &it : slide.items) {
        if (placeholderType(*it).isEmpty()) continue;
        if (Item *place = matchIn(layout, *it, taken)) {
            it->rect = place->rect;
            it->rotation = place->rotation;
            it->role = place->role;
            if (it->type() == ItemType::Text && place->type() == ItemType::Text) {
                auto *t = static_cast<TextItem *>(it.get());
                const auto *p = static_cast<const TextItem *>(place);
                t->valign = p->valign;
                t->insets = p->insets;
                t->prompt = promptFor(placeholderType(*place));
                // With nothing in it, what is typed next has the new layout's look: its bullet, indent, and text.
                QTextDocument *sd = d.storyDoc(t->storyId);
                if (const QTextDocument *ls = d.storyDoc(p->storyId); sd && sd->isEmpty() && ls) givePara(sd, paraLook(ls->begin()));
            }
        }
    }
    // The new layout's places that nothing filled get empty placeholders.
    const FooterOptions footers = footerOptions(d);
    const bool chosen = d.appData.contains(QStringLiteral("footerOptions"));
    for (const auto &l : layout.items) {
        const QString type = placeholderType(*l);
        if (type.isEmpty() || taken.contains(l.get()) || (isFooterType(type) && !footers.shows(type, layoutType(layout)))) continue;
        slide.items.push_back(slidePlaceholder(d, *l));
        if (chosen && isFooterType(type)) fillFooter(d, *slide.items.back(), type, footers);
    }
}

namespace {

// The plain color a run is set in, or invalid (not set, or a color of the presentation's scheme).
QColor runColorOf(const QTextCharFormat &cf)
{
    const QString ref = cf.stringProperty(tp::ColorRefP);
    if (!ref.isEmpty()) {
        const ColorRef r = ColorRef::fromString(ref);
        return r.kind() == ColorRef::Rgb ? r.rgbValue() : QColor();
    }
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() == Qt::SolidPattern) return cf.foreground().color();
    return QColor();
}

// The placeholder with that role on a layout (its own, else its slide master's); null for none.
const Item *placeByRole(const Document &d, const MasterPage &layout, const QString &role)
{
    if (role.isEmpty()) return nullptr;
    const QVector<MasterPage *> chain = d.masterChain(&layout);
    for (int i = int(chain.size()) - 1; i >= 0; --i)
        for (const auto &it : chain[i]->items)
            if (it->role == role && it->type() == ItemType::Text) return it.get();
    return nullptr;
}

// The layout's placeholder that a placeholder of a slide stands for, found by its role; null for none.
const Item *layoutPlaceFor(const Document &d, const Item &placeholder)
{
    if (placeholder.type() != ItemType::Text) return nullptr;
    const Document::Loc where = d.find(placeholder.id);
    if (where.page < 0 || where.page >= d.pages.size()) return nullptr;
    const MasterPage *layout = d.masterFor(*d.pages[where.page]);
    return layout ? placeByRole(d, *layout, placeholder.role) : nullptr;
}

// The level of a paragraph of a slide's text: 0 is the first.
int levelOfBlock(const QTextBlock &b)
{
    const QTextBlockFormat bf = b.blockFormat();
    int level = bf.intProperty(tp::ListLevel);
    if (!bf.hasProperty(tp::ListLevel))
        if (const QTextList *list = b.textList()) level = list->format().intProperty(tp::ListLevel);
    return std::clamp(level, 0, 8);
}

// The paragraph of a layout's placeholder that says how `level` looks: the one of that level, else the deepest above it, which is `past` levels
// short of it.
ParaLook levelLook(const QTextDocument *story, int level, int *past)
{
    QTextBlock best;
    int bestLevel = -1;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const int l = levelOfBlock(b);
        if (l <= level && l > bestLevel) {
            best = b;
            bestLevel = l;
        }
    }
    if (!best.isValid()) best = story->begin();
    *past = bestLevel >= 0 ? level - bestLevel : 0;
    return paraLook(best);
}

constexpr double kLevelPoints = 36;   // a level the layout has no paragraph for is indented this much further

// Whether a paragraph is set as `was` says (a layout's paragraph): the same list, indent, and spacing.
bool sameList(const QTextListFormat &a, const QTextListFormat &b)
{
    return a.style() == b.style() && a.indent() == b.indent() && a.numberPrefix() == b.numberPrefix() && a.numberSuffix() == b.numberSuffix() && a.start() == b.start()
           && a.intProperty(tp::ListLevel) == b.intProperty(tp::ListLevel) && a.stringProperty(tp::BulletChar) == b.stringProperty(tp::BulletChar)
           && a.stringProperty(tp::BulletFont) == b.stringProperty(tp::BulletFont) && a.stringProperty(tp::BulletColor) == b.stringProperty(tp::BulletColor)
           && a.property(tp::BulletSize).toDouble() == b.property(tp::BulletSize).toDouble() && a.intProperty(tp::NumberFormat) == b.intProperty(tp::NumberFormat);
}

bool blockFollows(const QTextBlock &b, const ParaLook &was)
{
    const QTextBlockFormat bf = b.blockFormat();
    for (int prop : {int(QTextFormat::BlockLeftMargin), int(QTextFormat::TextIndent), int(QTextFormat::BlockIndent), int(QTextFormat::BlockTopMargin),
                     int(QTextFormat::BlockBottomMargin), int(QTextFormat::LineHeight), int(QTextFormat::LineHeightType), int(tp::ListLevel)}) {
        const bool has = bf.hasProperty(prop);
        if (has != was.block.hasProperty(prop) || (has && bf.property(prop) != was.block.property(prop))) return false;
    }
    const QTextList *list = b.textList();
    if ((list != nullptr) != was.listed) return false;
    return !list || sameList(list->format(), was.list);
}

using Rest = std::function<bool(QTextCharFormat &, const Following &)>;

// Paragraph `number` of a story takes the look of the layout's paragraph `now` for `level` (`past` levels short of it) where it had `was`'s: its
// text (with `rest` called on what didn't follow), and, when `take` says so (it is set as `was` says, or it has moved to the level), its list, indent,
// and spacing; with `alignment`, its alignment where that was `was`'s. The indent is the level's, the spacing the layout's where it was.
void restyleParagraph(QTextDocument *sd, int number, const ParaLook &was, const ParaLook &now, int level, int past, bool take, bool alignment, const Rest &rest)
{
    const QTextBlock b = sd->findBlockByNumber(number);
    const bool both = was.valid && now.valid;
    struct Run {
        int from, to;
        QTextCharFormat format;
    };
    QVector<Run> runs;
    auto restyle = [&](QTextCharFormat &cf) {
        const QTextCharFormat before = cf;
        const Following followed = both ? followLook(cf, was.chars, now.chars) : Following();
        if (rest) rest(cf, followed);
        return cf != before;
    };
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        QTextCharFormat cf = it.fragment().charFormat();
        if (restyle(cf)) runs << Run{it.fragment().position(), it.fragment().position() + it.fragment().length(), cf};
    }
    QTextCursor c(b);
    QTextCharFormat chars = c.blockCharFormat();
    const bool charsChanged = restyle(chars);
    if (take && both) {
        if (QTextList *list = b.textList()) list->remove(b);
        QTextBlockFormat bf = c.blockFormat();
        bf.clearProperty(QTextFormat::ObjectIndex);
        for (int prop : {int(QTextFormat::BlockLeftMargin), int(QTextFormat::TextIndent), int(QTextFormat::BlockIndent)}) {
            if (now.block.hasProperty(prop)) bf.setProperty(prop, now.block.property(prop));
            else bf.clearProperty(prop);
        }
        if (past > 0) bf.setLeftMargin(bf.leftMargin() + past * kLevelPoints);
        for (int prop : {int(QTextFormat::BlockTopMargin), int(QTextFormat::BlockBottomMargin), int(QTextFormat::LineHeight), int(QTextFormat::LineHeightType)}) {
            const bool has = bf.hasProperty(prop);
            if (has != was.block.hasProperty(prop) || (has && bf.property(prop) != was.block.property(prop))) continue;
            if (now.block.hasProperty(prop)) bf.setProperty(prop, now.block.property(prop));
            else bf.clearProperty(prop);
        }
        if (level > 0) bf.setProperty(tp::ListLevel, level);
        else bf.clearProperty(tp::ListLevel);
        c.setBlockFormat(bf);
        if (now.listed) {
            QTextListFormat lf = now.list;
            if (past > 0) lf.setProperty(tp::ListLevel, level);
            // Numbers go on in the list of that level just above, if there is one.
            QTextList *join = nullptr;
            if (!isBulletList(lf.style()) && lf.style() != QTextListFormat::ListStyleUndefined) {
                for (QTextBlock p = b.previous(); p.isValid(); p = p.previous()) {
                    QTextList *above = p.textList();
                    if (!above) break;
                    const int aboveLevel = above->format().intProperty(tp::ListLevel);
                    if (aboveLevel == level && above->format().style() == lf.style()) {
                        join = above;
                        break;
                    }
                    if (aboveLevel < level) break;
                }
            }
            if (join) join->add(b);
            else c.createList(lf);
        }
    }
    if (alignment && both) {
        QTextBlockFormat bf = c.blockFormat();
        if (bf.alignment() == was.block.alignment() && now.block.alignment() != was.block.alignment()) {
            bf.setAlignment(now.block.alignment());
            c.setBlockFormat(bf);
        }
    }
    if (charsChanged) c.setBlockCharFormat(chars);
    for (const Run &r : runs) {
        QTextCursor run(sd);
        run.setPosition(r.from);
        run.setPosition(r.to, QTextCursor::KeepAnchor);
        run.setCharFormat(r.format);
    }
}

void putOnLevel(QTextDocument *sd, int number, const ParaLook &was, const ParaLook &now, int level, int past)
{
    restyleParagraph(sd, number, was, now, level, past, true, false, Rest());
}

} // namespace

Following followLook(QTextCharFormat &cf, const QTextCharFormat &was, const QTextCharFormat &now)
{
    static const int props[] = {QTextFormat::FontFamilies, QTextFormat::FontPointSize, QTextFormat::FontWeight, QTextFormat::FontItalic,
                                QTextFormat::FontCapitalization, tp::ThemeFont};
    Following done;
    const QTextCharFormat before = cf;
    for (int p : props) {
        const bool has = before.hasProperty(p);
        if (has != was.hasProperty(p) || (has && before.property(p) != was.property(p))) continue;
        if (now.hasProperty(p)) cf.setProperty(p, now.property(p));
        else cf.clearProperty(p);
        if (p == QTextFormat::FontFamilies) done.family = true;
    }
    if (runColorOf(before) == runColorOf(was)) {
        const QColor c = runColorOf(now);
        if (c.isValid()) {
            cf.setForeground(QBrush(c));
            cf.setProperty(tp::ColorRefP, ColorRef::rgb(c).toString());
        } else {
            cf.clearProperty(tp::ColorRefP);
            cf.clearProperty(QTextFormat::ForegroundBrush);
        }
        done.color = true;
    }
    return done;
}

void inheritLayoutLists(Document &d)
{
    for (const auto &page : d.pages) {
        const MasterPage *layout = d.masterFor(*page);
        if (!layout) continue;
        for (const auto &it : page->items) {
            if (it->type() != ItemType::Text || placeholderType(*it).isEmpty()) continue;
            QTextDocument *sd = d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
            const Item *place = placeByRole(d, *layout, it->role);
            const QTextDocument *ls = place ? d.storyDoc(static_cast<const TextItem *>(place)->storyId) : nullptr;
            if (!sd || !ls || !sd->isEmpty() || sd->begin().textList()) continue;
            int past = 0;
            const ParaLook look = levelLook(ls, levelOfBlock(sd->begin()), &past);
            if (!look.listed) continue;
            QTextListFormat lf = look.list;
            if (past > 0) lf.setProperty(tp::ListLevel, levelOfBlock(sd->begin()));
            QTextCursor(sd).createList(lf);
        }
    }
}

void followLayout(Document &d, TextItem &t, const Item *from, const Item *to, const std::function<bool(QTextCharFormat &, const Following &)> &rest)
{
    QTextDocument *sd = d.storyDoc(t.storyId);
    if (!sd) return;
    const QTextDocument *fs = from && from->type() == ItemType::Text ? d.storyDoc(static_cast<const TextItem *>(from)->storyId) : nullptr;
    const QTextDocument *ts = to && to->type() == ItemType::Text ? d.storyDoc(static_cast<const TextItem *>(to)->storyId) : nullptr;
    for (int n = 0; n < sd->blockCount(); ++n) {
        const int level = levelOfBlock(sd->findBlockByNumber(n));
        int wasPast = 0, past = 0;
        const ParaLook was = fs ? levelLook(fs, level, &wasPast) : ParaLook(), now = ts ? levelLook(ts, level, &past) : ParaLook();
        restyleParagraph(sd, n, was, now, level, past, was.valid && now.valid && blockFollows(sd->findBlockByNumber(n), was), true, rest);
    }
}

bool layoutHasList(const Document &d, const Item &placeholder)
{
    const Item *place = layoutPlaceFor(d, placeholder);
    const QTextDocument *ls = place ? d.storyDoc(static_cast<const TextItem *>(place)->storyId) : nullptr;
    if (!ls) return false;
    for (QTextBlock b = ls->begin(); b.isValid(); b = b.next())
        if (b.textList()) return true;
    return false;
}

bool changeListLevel(Document &d, const Item &placeholder, int first, int last, int dir)
{
    if (!layoutHasList(d, placeholder)) return false;
    const QTextDocument *ls = d.storyDoc(static_cast<const TextItem *>(layoutPlaceFor(d, placeholder))->storyId);
    QTextDocument *sd = d.storyDoc(static_cast<const TextItem &>(placeholder).storyId);
    if (!sd) return false;
    for (int n = std::max(0, first); n <= last && n < sd->blockCount(); ++n) {
        const int level = levelOfBlock(sd->findBlockByNumber(n)), to = std::clamp(level + dir, 0, 8);
        if (to == level) continue;
        int past = 0, wasPast = 0;
        const ParaLook was = levelLook(ls, level, &wasPast), now = levelLook(ls, to, &past);
        putOnLevel(sd, n, was, now, to, past);
    }
    return true;
}

void resetSlide(Document &d, Page &slide)
{
    if (const MasterPage *layout = d.masterFor(slide)) {
        QSet<const Item *> taken;
        for (const auto &it : slide.items) {
            if (placeholderType(*it).isEmpty()) continue;
            if (Item *place = matchIn(*layout, *it, taken)) {
                it->rect = place->rect;
                it->rotation = place->rotation;
                it->flipH = place->flipH;
                it->flipV = place->flipV;
            }
        }
    }
    slide.background = Fill();
    slide.showMasterItems = true;
}

QString notesText(const Document &d, const Page &slide)
{
    const QTextDocument *sd = d.storyDoc(slide.notesStoryId);
    return sd ? sd->toPlainText() : QString();
}

QString slideSizeName(const QSizeF &size)
{
    if (size == QSizeF(kStandardWidth, kStandardHeight)) return QStringLiteral("Standard");
    if (size == QSizeF(kSlideWidth, kSlideHeight)) return QStringLiteral("Widescreen");
    return QStringLiteral("Custom");
}

void setSlideSize(Document &d, const QSizeF &size)
{
    const QSizeF old = d.pageSize();
    if (size.isEmpty() || old.isEmpty() || size == old) return;
    const double sx = size.width() / old.width(), sy = size.height() / old.height(), k = std::min(sx, sy);
    const QRectF from(QPointF(0, 0), old), whole(QPointF(0, 0), size);
    const QRectF fitted((size.width() - old.width() * k) / 2, (size.height() - old.height() * k) / 2, old.width() * k, old.height() * k);
    QSet<QString> stories;
    for (auto &m : d.masters) {
        scaleItems(d, m->items, from, whole, fitted, k, false, stories);
        scaleGuides(m->guides, sx, sy);
    }
    for (auto &p : d.pages) {
        scaleItems(d, p->items, from, whole, fitted, k, true, stories);
        scaleGuides(p->guides, sx, sy);
    }
    d.setup.size = d.setup.sheet = size;
    d.setup.sizeName = slideSizeName(size);
}

void fitPageToSize(Document &d, PageBase &page, const QSizeF &from, const QSizeF &to, bool slide)
{
    if (from.isEmpty() || to.isEmpty() || from == to) return;
    const double sx = to.width() / from.width(), sy = to.height() / from.height(), k = std::min(sx, sy);
    const QRectF was(QPointF(0, 0), from), whole(QPointF(0, 0), to);
    const QRectF fitted((to.width() - from.width() * k) / 2, (to.height() - from.height() * k) / 2, from.width() * k, from.height() * k);
    QSet<QString> stories;
    scaleItems(d, page.items, was, whole, fitted, k, slide, stories);
    scaleGuides(page.guides, sx, sy);
}

bool FooterOptions::shows(const QString &placeholderType, const QString &layoutType) const
{
    if (skipTitleSlide && layoutType == QLatin1String("title")) return false;
    if (placeholderType == QLatin1String("dt")) return date;
    if (placeholderType == QLatin1String("ftr")) return footer;
    if (placeholderType == QLatin1String("sldNum")) return slideNumber;
    return false;
}

QJsonObject FooterOptions::toJson() const
{
    return {{"date", date}, {"slideNumber", slideNumber}, {"footer", footer}, {"text", footerText}, {"skipTitle", skipTitleSlide}};
}

FooterOptions FooterOptions::fromJson(const QJsonObject &o)
{
    FooterOptions f;
    f.date = o["date"].toBool();
    f.slideNumber = o["slideNumber"].toBool();
    f.footer = o["footer"].toBool();
    f.footerText = o["text"].toString();
    f.skipTitleSlide = o["skipTitle"].toBool();
    return f;
}

FooterOptions footerOptions(const Document &d)
{
    const QJsonObject o = d.appData.value(QStringLiteral("footerOptions")).toObject();
    if (!o.isEmpty()) return FooterOptions::fromJson(o);
    FooterOptions all;
    all.date = all.slideNumber = all.footer = d.appData.value(QStringLiteral("footers")).toBool();
    return all;
}

void applyFooters(Document &d, const FooterOptions &o, Page *only)
{
    if (!only) {
        d.appData[QStringLiteral("footerOptions")] = o.toJson();
        d.appData[QStringLiteral("footers")] = o.any();
    }
    for (auto &p : d.pages) {
        if (only && p.get() != only) continue;
        const MasterPage *layout = d.masterFor(*p);
        const QString ltype = layout ? layoutType(*layout) : QString();
        for (const QString &type : {QStringLiteral("dt"), QStringLiteral("ftr"), QStringLiteral("sldNum")}) {
            auto at = std::find_if(p->items.begin(), p->items.end(), [&](const ItemPtr &it) { return placeholderType(*it) == type; });
            if (!o.shows(type, ltype)) {
                if (at == p->items.end()) continue;
                if ((*at)->type() == ItemType::Text) d.removeStory(static_cast<TextItem *>(at->get())->storyId);
                p->items.erase(at);
                continue;
            }
            if (at == p->items.end()) {
                const Item *from = layout ? layoutPlaceholder(d, *layout, type) : nullptr;
                if (!from) continue;
                p->items.push_back(slidePlaceholder(d, *from));
                at = std::prev(p->items.end());
            }
            fillFooter(d, **at, type, o);
        }
    }
}

void setSlideBackground(Page &slide, const QColor &color) { slide.background = Fill::solid(ColorRef::rgb(color)); }

void setAllBackgrounds(Document &d, const QColor &color)
{
    for (auto &m : d.masters) {
        if (isSlideMaster(*m)) m->background = Fill::solid(ColorRef::rgb(color));
        else if (isLayout(*m)) m->background = Fill();
    }
    for (auto &p : d.pages) p->background = Fill();
}

} // namespace jp::deck
