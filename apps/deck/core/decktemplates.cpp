#include "core/decktemplates.h"
#include "core/themes.h"

#include "core/items.h"
#include "core/masteredit.h"
#include "core/themes_p.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <algorithm>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace jp::deck {

namespace {

// (the texts are marked where they stand with QT_TRANSLATE_NOOP, and translated here)
QString t(const char *text) { return QCoreApplication::translate("DeckTemplates", text); }

// What a template is made of: its colors (dark 1, light 1, dark 2, light 2, accents 1 to 6, the link colors), its fonts, and the style of its background (see backgroundStyle). Text is
// always dark 1 on light 1 (or the other way round), at 4.5 to 1 or better; the accents only color shapes.
struct Spec {
    const char *id;
    const char *colors[12];
    const char *heading, *body;
    int style;         // the background of the slides
    int titleStyle;    // the background of the Title Slide
};

const Spec kSpecs[] = {
    {"pitch", {"#14213D", "#FFFFFF", "#1D2B53", "#F1F4FA", "#E4572E", "#2A9D8F", "#F4A261", "#6C63FF", "#E9C46A", "#264653", "#1A56B8", "#6B3FA0"}, "Montserrat", "Open Sans", 0, 9},
    {"status", {"#1F2933", "#FFFFFF", "#243B53", "#EEF2F6", "#2F6FDB", "#2E9E5B", "#E8A317", "#D64545", "#7A5AF8", "#0E8F9C", "#1D55C4", "#5E2CA5"}, "Raleway", "Lato", 0, 0},
    {"lesson", {"#1E2D24", "#FFFDF7", "#2E5339", "#F5EFDD", "#3F8F4F", "#E0603A", "#F2B134", "#2F8FBF", "#7A5C99", "#C9477A", "#1E6FB8", "#7A4F9C"}, "Quicksand", "Nunito", 3, 6},
    {"album", {"#2B2622", "#FFFFFF", "#3A322B", "#F4EEE6", "#B5651D", "#4F7A5A", "#6F8FAF", "#C9A227", "#8C3B2E", "#5E6B73", "#1F5C99", "#6B4E7A"}, "Playfair Display", "Lato", 3, 6},
    {"minimallight", {"#111111", "#FFFFFF", "#2B2B2B", "#F2F2F2", "#2563EB", "#6B7280", "#0F766E", "#B45309", "#7C3AED", "#BE123C", "#1D4ED8", "#6D28D9"}, "Jost", "Work Sans", 0, 0},
    {"minimaldark", {"#101114", "#FFFFFF", "#1C1E24", "#E5E7EB", "#7DD3FC", "#A7F3D0", "#FDE68A", "#FBCFE8", "#C4B5FD", "#FDBA74", "#93C5FD", "#D8B4FE"}, "Jost", "Work Sans", 9, 9},
};

const Spec &specOf(const QString &id)
{
    for (const Spec &s : kSpecs)
        if (id == QLatin1String(s.id)) return s;
    return kSpecs[0];
}

ColorSet colorSetOf(const Spec &s, const QString &name)
{
    ColorSet c;
    c.name = name;
    for (int i = 0; i < 12; ++i) c.colors[i] = QColor(QLatin1String(s.colors[i]));
    return c;
}

ItemPtr decoration(const char *kind, const QRectF &r, const QColor &color, double transparency = 0)
{
    auto it = std::make_shared<ShapeItem>();
    it->shape = QString::fromLatin1(kind);
    it->rect = r;
    it->name = QCoreApplication::translate("Deck", "Decoration");
    it->fill = Fill::solid(ColorRef::rgb(color), transparency);
    it->stroke = Stroke::none();
    it->decorative = true;   // (a screen reader leaves it out)
    return it;
}

// Draws shapes first on a page, under its placeholders.
void drawFirst(MasterPage &page, const ItemList &shapes) { page.items.insert(page.items.begin(), shapes.begin(), shapes.end()); }

// Gives the text of a placeholder one color.
void colorText(Document &d, TextItem &item, const QColor &color)
{
    QTextDocument *sd = d.storyDoc(item.storyId);
    if (!sd) return;
    QTextCharFormat cf;
    cf.setForeground(color);
    cf.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
    QTextCursor c(sd);
    c.select(QTextCursor::Document);
    c.mergeCharFormat(cf);
    QTextCursor start(sd);
    start.mergeBlockCharFormat(cf);
}

// Sets where the text of a placeholder stands.
void alignText(Document &d, TextItem &item, Qt::Alignment align)
{
    QTextDocument *sd = d.storyDoc(item.storyId);
    if (!sd) return;
    QTextBlockFormat bf;
    bf.setAlignment(align);
    QTextCursor c(sd);
    c.select(QTextCursor::Document);
    c.mergeBlockFormat(bf);
}

// The names of the photo album's layouts (one picture, two pictures, three pictures).
QString photoLayoutName(int pictures)
{
    return pictures == 1 ? t(QT_TRANSLATE_NOOP("DeckTemplates", "One Picture")) : pictures == 2 ? t(QT_TRANSLATE_NOOP("DeckTemplates", "Two Pictures")) : t(QT_TRANSLATE_NOOP("DeckTemplates", "Three Pictures"));
}

// The slide master's and layouts' look: the accent marks, and a Title Slide on a background of its own.
void dress(Document &d, const Spec &s, const QString &id)
{
    const DeckTheme theme = themeOf(d);
    const QColor accent = theme.colors[4], second = theme.colors[5], third = theme.colors[6];
    // The bars along the slide's edges, and the rule under the title (just below the bottom edge of the title's place, so that a title of three lines does not run into it).
    auto bars = [&]() -> ItemList {
        if (id == QLatin1String("pitch")) return {decoration("rect", QRectF(0, 0, 14, 540), accent)};
        if (id == QLatin1String("status")) return {decoration("rect", QRectF(0, 0, 960, 10), accent)};
        if (id == QLatin1String("lesson")) return {decoration("rect", QRectF(0, 0, 960, 14), accent), decoration("rect", QRectF(0, 14, 960, 5), third)};
        return {};
    };
    auto underTitle = [&](double y) -> ItemList {
        if (id == QLatin1String("pitch")) return {decoration("rect", QRectF(66, y, 96, 5), accent)};
        if (id == QLatin1String("status")) return {decoration("rect", QRectF(66, y, 96, 4), second)};
        if (id == QLatin1String("album")) return {decoration("rect", QRectF(66, y, 828, 1.5), accent)};
        if (id == QLatin1String("minimallight") || id == QLatin1String("minimaldark")) return {decoration("rect", QRectF(66, y, 40, 3), accent)};
        return {};
    };
    auto titleOf = [](const MasterPage &page) -> const Item * {
        for (const auto &it : page.items)
            if (it->type() == ItemType::Text && placeholderType(*it) == QLatin1String("title")) return it.get();
        return nullptr;
    };
    double titleBottom = 133;
    for (const auto &m : d.masters)
        if (isSlideMaster(*m))
            if (const Item *title = titleOf(*m)) titleBottom = title->rect.bottom();
    for (auto &m : d.masters) {
        const bool master = isSlideMaster(*m);
        // (a list of five levels, with bullets in the first accent, in the places that hold the slides' lists)
        for (const auto &it : m->items)
            if (it->type() == ItemType::Text && (placeholderType(*it) == QLatin1String("obj") || (master && placeholderType(*it) == QLatin1String("body")))) detail::bulletizeBody(d, *static_cast<TextItem *>(it.get()), 26, theme, 4);
        const QString kind = master ? QStringLiteral("master") : layoutType(*m);
        if (master) {
            ItemList shapes = bars();
            const ItemList rule = underTitle(titleBottom + 4);
            shapes.insert(shapes.end(), rule.begin(), rule.end());
            drawFirst(*m, shapes);
        } else if (kind != QLatin1String("title")) {
            // A layout whose title stands somewhere else (a section header's, a caption's), or that has a place where the rule goes (a comparison's headings), has the bars but not the rule, which would cross its words.
            const Item *title = titleOf(*m);
            bool crossed = title && std::abs(title->rect.bottom() - titleBottom) > 0.5;
            for (const auto &it : m->items) {
                const QString type = placeholderType(*it);
                if (it->type() != ItemType::Text || type.isEmpty() || type == QLatin1String("title") || type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum")) continue;
                for (const auto &line : underTitle(titleBottom + 4)) crossed = crossed || line->rect.intersects(it->rect);
            }
            if (crossed) {
                m->showMasterItems = false;
                drawFirst(*m, bars());
            }
        }
        if (kind == QLatin1String("title")) {
            m->showMasterItems = false;
            m->background = backgroundStyle(theme, s.titleStyle);
            ItemList shapes;
            if (id == QLatin1String("pitch")) shapes = {decoration("ellipse", QRectF(620, -60, 420, 420), accent), decoration("ellipse", QRectF(760, 280, 260, 260), second), decoration("rect", QRectF(66, 436, 96, 5), third)};
            else if (id == QLatin1String("lesson")) shapes = {decoration("rect", QRectF(0, 0, 960, 20), accent), decoration("ellipse", QRectF(780, 300, 300, 300), third, 0.1), decoration("ellipse", QRectF(700, 380, 160, 160), second)};
            else if (id == QLatin1String("album")) shapes = {decoration("rect", QRectF(380, 440, 200, 2), accent)};
            else if (id == QLatin1String("minimallight")) shapes = {decoration("rect", QRectF(66, 436, 40, 3), accent)};
            else if (id == QLatin1String("minimaldark")) shapes = {decoration("rect", QRectF(66, 436, 40, 3), accent)};
            else if (id == QLatin1String("status")) shapes = {decoration("rect", QRectF(0, 0, 960, 10), accent), decoration("rect", QRectF(66, 436, 96, 4), second)};
            drawFirst(*m, shapes);
            // the Title Slide's words are light on its dark background, and dark on a light one
            const bool darkTitle = backgroundStyleIsDark(s.titleStyle);
            // (the words stand at the left, clear of the drawing at the right, except in the album, whose title is in the middle)
            const bool leftSide = id != QLatin1String("album");
            const double room = id == QLatin1String("pitch") || id == QLatin1String("lesson") ? 540 : 828;
            for (const auto &it : m->items) {
                if (it->type() != ItemType::Text) continue;
                const QString type = placeholderType(*it);
                if (type == QLatin1String("ctrTitle") && leftSide) it->rect = QRectF(66, 110, room, 190);
                if (type == QLatin1String("subTitle") && leftSide) it->rect = QRectF(66, 312, room, 100);
                if (type.isEmpty() || type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum")) continue;
                auto *text = static_cast<TextItem *>(it.get());
                colorText(d, *text, theme.colors[darkTitle ? 1 : 0]);
                if (leftSide) alignText(d, *text, Qt::AlignLeft);
            }
        } else if (kind == QLatin1String("secHead")) {
            // (the Section Header's gray words are dark enough to read, 4.5 to 1)
            for (const auto &it : m->items)
                if (it->type() == ItemType::Text && !placeholderType(*it).isEmpty() && placeholderType(*it) != QLatin1String("dt") && placeholderType(*it) != QLatin1String("ftr") && placeholderType(*it) != QLatin1String("sldNum"))
                    colorText(d, *static_cast<TextItem *>(it.get()), theme.colors[backgroundStyleIsDark(s.style) ? 1 : 0]);
        }
    }
    // The photo album's own layouts: a caption, and one, two, or three places for pictures.
    if (id == QLatin1String("album"))
        for (const auto &m : d.masters) {
            if (!isSlideMaster(*m)) continue;
            const QString master = m->id;
            const QVector<QVector<QRectF>> pictures = {{QRectF(66, 144, 828, 340)}, {QRectF(66, 144, 400, 340), QRectF(494, 144, 400, 340)}, {QRectF(66, 144, 262, 340), QRectF(349, 144, 262, 340), QRectF(632, 144, 262, 340)}};
            for (int n = 0; n < pictures.size(); ++n) {
                MasterPage *layout = addLayout(d, master);
                if (!layout) break;
                renameMasterPage(d, layout->id, photoLayoutName(n + 1));
                for (const QRectF &r : pictures[n])
                    if (Item *place = insertPlaceholder(d, *layout, PlaceholderKind::Picture)) place->rect = r;
            }
            break;
        }
}

// Says `lines` in the `nth` placeholder of type `type` of a slide, one paragraph each.
void say(Document &d, Page &slide, const char *type, int nth, const QStringList &lines)
{
    int seen = 0;
    for (const auto &it : slide.items) {
        if (it->type() != ItemType::Text || placeholderType(*it) != QLatin1String(type) || seen++ != nth) continue;
        QTextDocument *sd = d.storyDoc(static_cast<TextItem *>(it.get())->storyId);
        if (!sd) return;
        QTextCursor c(sd);
        c.movePosition(QTextCursor::End);
        for (int i = 0; i < lines.size(); ++i) {
            if (i) c.insertBlock();
            c.insertText(lines[i]);
        }
        return;
    }
}

Page &newSlide(Document &d, const char *layout)
{
    const MasterPage *l = layoutOfType(d, QLatin1String(layout));
    if (!l) l = layoutOfType(d, QStringLiteral("obj"));
    const int at = addSlide(d, *l);
    return *d.pages[at];
}

// A slide with the album's layout that has that many places for pictures.
Page &newPhotoSlide(Document &d, int pictures)
{
    const QString name = photoLayoutName(pictures);
    for (const auto &m : d.masters)
        if (isLayout(*m) && m->name == name) {
            addSlide(d, *m);
            return *d.pages.back();
        }
    return newSlide(d, "titleOnly");
}

void sampleSlides(Document &d, const QString &id)
{
    Page &title = *d.pages[0];
    if (id == QLatin1String("pitch")) {
        say(d, title, "ctrTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Company name]"))});
        say(d, title, "subTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[One sentence that says what you do and for whom]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "The problem"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Who has the problem, and what it costs them]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[How they cope today]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[Why now is the time to fix it]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Our solution"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[What you built, in one line]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[How it works, in three steps]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[What makes it better than the options]"))});
        say(d, newSlide(d, "twoObj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Where we are"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Customers or users]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[Revenue or growth]"))});
        say(d, *d.pages.back(), "obj", 1, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Team: who you are]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[Milestones so far]"))});
        say(d, newSlide(d, "secHead"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "The ask"))});
        say(d, *d.pages.back(), "body", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[What you need, how much, and what it will do]"))});
    } else if (id == QLatin1String("status")) {
        say(d, title, "ctrTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Project name] status update"))});
        say(d, title, "subTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Date]  |  [Your name]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Where we are"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Status: [on track, at risk, or late]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[One sentence on how the work is going]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[The date the work will be done]"))});
        say(d, newSlide(d, "twoObj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Done and next"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Done: [finished item]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Done: [finished item]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Done: [finished item]"))});
        say(d, *d.pages.back(), "obj", 1, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Next: [planned item]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Next: [planned item]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Next: [planned item]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Risks and help needed"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[A risk, and what you are doing about it]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A decision you need, and from whom]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[Help you are asking for]"))});
    } else if (id == QLatin1String("lesson")) {
        say(d, title, "ctrTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Lesson title]"))});
        say(d, title, "subTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Course or grade]  |  [Teacher]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Today's goals"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[What students will be able to do by the end]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A second goal]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A third goal]"))});
        say(d, newSlide(d, "twoObj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Key ideas"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[First idea]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[An example that shows it]"))});
        say(d, *d.pages.back(), "obj", 1, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Second idea]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[An example that shows it]"))});
        say(d, newSlide(d, "secHead"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Try it"))});
        say(d, *d.pages.back(), "body", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Describe the activity, and how long students have]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "Check your understanding"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[A question for students to answer]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A second question]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A question that asks them to explain why]"))});
    } else if (id == QLatin1String("album")) {
        say(d, title, "ctrTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Album title]"))});
        say(d, title, "subTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Place or occasion]  |  [Dates]"))});
        say(d, newPhotoSlide(d, 1), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Caption for this photo]"))});
        say(d, newPhotoSlide(d, 2), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Caption for these photos]"))});
        say(d, newPhotoSlide(d, 3), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Caption for this moment]"))});
    } else {   // the two minimal looks
        say(d, title, "ctrTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Presentation title]"))});
        say(d, title, "subTitle", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Your name]  |  [Date]"))});
        say(d, newSlide(d, "obj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[A heading that says the point]"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[A first point, in a short line]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A second point]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[A third point]"))});
        say(d, newSlide(d, "twoObj"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Two things side by side]"))});
        say(d, *d.pages.back(), "obj", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Left: a point]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[Left: another point]"))});
        say(d, *d.pages.back(), "obj", 1, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Right: a point]")), t(QT_TRANSLATE_NOOP("DeckTemplates", "[Right: another point]"))});
        say(d, newSlide(d, "secHead"), "title", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Thank you]"))});
        say(d, *d.pages.back(), "body", 0, {t(QT_TRANSLATE_NOOP("DeckTemplates", "[Email address or website]"))});
    }
}

} // namespace

QVector<BuiltInTemplate> builtInTemplates()
{
    return {
        {QStringLiteral("pitch"), t(QT_TRANSLATE_NOOP("DeckTemplates", "Pitch")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Five slides that tell investors or partners what you do and what you need."))},
        {QStringLiteral("status"), t(QT_TRANSLATE_NOOP("DeckTemplates", "Status Update")), t(QT_TRANSLATE_NOOP("DeckTemplates", "A short report on where a project stands, what is next, and what could go wrong."))},
        {QStringLiteral("lesson"), t(QT_TRANSLATE_NOOP("DeckTemplates", "Lesson")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Goals, key ideas, an activity, and questions for a class."))},
        {QStringLiteral("album"), t(QT_TRANSLATE_NOOP("DeckTemplates", "Photo Album")), t(QT_TRANSLATE_NOOP("DeckTemplates", "Pages with room for one, two, or three pictures, and a caption for each."))},
        {QStringLiteral("minimallight"), t(QT_TRANSLATE_NOOP("DeckTemplates", "Minimal Light")), t(QT_TRANSLATE_NOOP("DeckTemplates", "A plain white design with lots of room for your words."))},
        {QStringLiteral("minimaldark"), t(QT_TRANSLATE_NOOP("DeckTemplates", "Minimal Dark")), t(QT_TRANSLATE_NOOP("DeckTemplates", "A plain dark design that is easy on the eyes in a dim room."))},
    };
}

std::unique_ptr<Document> builtInTemplateDocument(const QString &id)
{
    bool known = false;
    for (const BuiltInTemplate &b : builtInTemplates()) known = known || b.id == id;
    if (!known) return nullptr;
    const Spec &s = specOf(id);
    auto d = themePresentation(0, 0, false);
    const QString name = [&] {
        for (const BuiltInTemplate &b : builtInTemplates())
            if (b.id == id) return b.name;
        return QString();
    }();
    applyColors(*d, colorSetOf(s, name));
    applyFonts(*d, FontSet{name, QString::fromLatin1(s.heading), QString::fromLatin1(s.body)});
    applyBackgroundStyle(*d, s.style);
    dress(*d, s, id);
    // The Title Slide made again from the dressed layout, then the other sample slides.
    if (!d->pages.isEmpty()) {
        const Page &old = *d->pages[0];
        for (const auto &it : old.items)
            if (it->type() == ItemType::Text) d->removeStory(static_cast<TextItem *>(it.get())->storyId);
        d->removeStory(old.notesStoryId);
        d->pages.clear();
    }
    if (const MasterPage *title = layoutOfType(*d, QStringLiteral("title"))) addSlide(*d, *title);
    sampleSlides(*d, id);
    keepTextReadable(*d);
    detail::fitLinkColors(*d);
    return d;
}

} // namespace jp::deck
