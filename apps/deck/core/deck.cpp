#include "core/deck.h"

#include "core/color.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

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

// The text cleared, its formatting kept for what is typed next.
void clearStory(Document &d, const QString &storyId)
{
    QTextDocument *sd = d.storyDoc(storyId);
    if (!sd || sd->isEmpty()) return;
    QTextCursor first(sd);
    const QTextCharFormat cf = first.charFormat();
    const QTextBlockFormat bf = first.blockFormat();
    sd->clear();
    QTextCursor c(sd);
    c.setBlockFormat(bf);
    c.setBlockCharFormat(cf);
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
    d->textRunsOver = true;
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
