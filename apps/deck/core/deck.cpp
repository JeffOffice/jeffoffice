#include "core/deck.h"

#include "core/color.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QJsonArray>
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

int placeholderIndex(const Item &it)
{
    const QStringList parts = it.role.split(QLatin1Char(':'));
    return parts.size() > 2 ? parts[2].toInt() : 0;
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
    const int index = placeholderIndex(it);
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

QVector<MasterPage *> layouts(const Document &d)
{
    QVector<MasterPage *> out;
    for (const auto &m : d.masters)
        if (isLayout(*m)) out << m.get();
    return out;
}

MasterPage *layoutOfType(const Document &d, const QString &type)
{
    for (MasterPage *m : layouts(d))
        if (layoutType(*m) == type) return m;
    return nullptr;
}

std::unique_ptr<Document> newPresentation()
{
    auto d = std::make_unique<Document>();
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
    const bool footers = d.appData.value(QStringLiteral("footers")).toBool();
    for (const auto &it : layout.items) {
        const QString type = placeholderType(*it);
        if (type.isEmpty() || (isFooterType(type) && !footers)) continue;
        slide->items.push_back(slidePlaceholder(d, *it));
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
    const bool footers = d.appData.value(QStringLiteral("footers")).toBool();
    for (const auto &l : layout.items) {
        const QString type = placeholderType(*l);
        if (type.isEmpty() || taken.contains(l.get()) || (isFooterType(type) && !footers)) continue;
        slide.items.push_back(slidePlaceholder(d, *l));
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

} // namespace jp::deck
