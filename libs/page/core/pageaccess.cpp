#include "core/pageaccess.h"

#include "core/document.h"
#include "render/renderer.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextDocument>

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

double linear(int v)
{
    const double c = v / 255.0;
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

double luminance(const QColor &c) { return 0.2126 * linear(c.red()) + 0.7152 * linear(c.green()) + 0.0722 * linear(c.blue()); }

// What is behind a piece of text: the colors it may be over (a gradient has several), or a picture, which can't be told from here.
struct Behind {
    QVector<QColor> colors;
    bool picture = false;
};

// The colors of a fill; a picture, a texture, or a pattern is a picture. A fill with no color of its own (none) gives no colors.
Behind fillBehind(const Fill &f, const Document &doc, const Behind &under)
{
    Behind b;
    const ColorScheme &cs = doc.colors;
    auto over = [&](const QColor &c) {
        // (a see-through fill lets what is under it show: its mix with it)
        if (f.transparency <= 0.001 || under.colors.isEmpty()) return c;
        return mix(c, under.colors.first(), std::clamp(f.transparency, 0.0, 1.0));
    };
    switch (f.type) {
    case Fill::Solid:
        if (!f.color.isNone()) b.colors << over(f.color.resolve(cs));
        break;
    case Fill::Gradient:
        if (f.stops.isEmpty()) {
            b.colors << over(f.color.resolve(cs)) << over(f.color2.resolve(cs));
        } else {
            for (const GradientStop &st : f.stops) b.colors << over(st.color.resolve(cs));
        }
        break;
    case Fill::Picture:
    case Fill::Texture:
    case Fill::Pattern: b.picture = true; break;
    case Fill::NoFill: break;
    }
    return b;
}

// The background of the page: its own fill, else its master's, else that one's base's; white with none.
Behind pageBehind(const Document &doc, const Page &pg)
{
    Fill bg = pg.background;
    if (bg.type == Fill::NoFill) {
        const QVector<MasterPage *> chain = doc.masterChain(doc.masterFor(pg));
        for (int i = int(chain.size()) - 1; i >= 0; --i)
            if (chain[i]->background.type != Fill::NoFill) {
                bg = chain[i]->background;
                break;
            }
    }
    Behind white;
    white.colors << QColor(Qt::white);
    Behind b = fillBehind(bg, doc, white);
    if (b.colors.isEmpty() && !b.picture) b = white;
    return b;
}

// What is under the object at `index` of the page's objects where the text is (`where`, in page coordinates): a picture that overlaps it; else the nearest fill below it that holds
// all of it; else the page.
Behind underneath(const Document &doc, const Page &pg, int index, const QRectF &where)
{
    for (int k = index - 1; k >= 0; --k) {
        const Item &it = *pg.items[size_t(k)];
        if (it.hidden || !it.bounds().intersects(where)) continue;
        if (it.type() == ItemType::Picture) {
            Behind b;
            b.picture = true;
            return b;
        }
        if (it.type() == ItemType::Line || !it.bounds().contains(where)) continue;
        Behind below = underneath(doc, pg, k, where);
        Behind own = fillBehind(it.fill, doc, below);
        if (own.picture || !own.colors.isEmpty()) return own;
    }
    return pageBehind(doc, pg);
}

// The worst contrast of the words of one story over `behind` (and the size and boldness of the words that came to it); -1 for none to judge.
struct Worst {
    double ratio = -1, margin = 1e9, points = 0;
    bool bold = false;
};

void judgeStory(const Document &doc, const LayoutEnv &env, const QString &storyId, const Behind &behind, Worst *worst)
{
    const QTextDocument *sd = doc.storyDoc(storyId);
    if (!sd || behind.colors.isEmpty()) return;
    for (QTextBlock block = sd->begin(); block.isValid(); block = block.next())
        for (auto frag = block.begin(); !frag.atEnd(); ++frag) {
            if (frag.fragment().text().trimmed().isEmpty()) continue;
            const QTextCharFormat cf = frag.fragment().charFormat();
            if (cf.hasProperty(tp::HiddenText) && cf.boolProperty(tp::HiddenText)) continue;
            QColor back = behind.colors.first();
            const QTextCharFormat r = resolveCharFormat(cf, env, back);
            if (r.foreground().style() != Qt::SolidPattern) continue;
            const QColor text = r.foreground().color();
            // (a run with a highlight is over that)
            QVector<QColor> over = behind.colors;
            if (r.background().style() == Qt::SolidPattern && r.background().color().alpha() > 0) over = {r.background().color()};
            const double points = cf.fontPointSize() > 0 ? cf.fontPointSize() : 11;
            const bool bold = cf.fontWeight() >= QFont::Bold || r.fontWeight() >= QFont::Bold;
            const double need = neededContrast(points, bold);
            for (const QColor &o : over) {
                const double ratio = contrastRatio(text, o);
                if (ratio - need < worst->margin) {
                    worst->margin = ratio - need;
                    worst->ratio = ratio;
                    worst->points = points;
                    worst->bold = bold;
                }
            }
        }
}

} // namespace

double contrastRatio(const QColor &a, const QColor &b)
{
    const double la = luminance(a), lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

double neededContrast(double points, bool bold) { return points >= 18 || (bold && points >= 14) ? 3.0 : 4.5; }

QVector<int> outOfReadingOrder(const ItemList &items)
{
    constexpr double kSameRow = 6;   // points
    QVector<int> read;                // the objects that are read, by number in the list
    for (int i = 0; i < int(items.size()); ++i) {
        const Item &it = *items[size_t(i)];
        if (it.hidden || it.decorative || it.type() == ItemType::Line || (it.type() == ItemType::Shape && !it.hasText() && it.altText.trimmed().isEmpty())) continue;
        read << i;
    }
    if (read.size() < 2 || read.size() > 2000) return {};
    // Their order by position: rows from the top, each row from the left.
    QVector<int> byPlace = read;
    std::stable_sort(byPlace.begin(), byPlace.end(), [&](int a, int b) { return items[size_t(a)]->bounds().top() < items[size_t(b)]->bounds().top(); });
    QVector<int> rank(int(items.size()), -1);
    for (int start = 0, n = int(byPlace.size()); start < n;) {
        int end = start + 1;
        const double top = items[size_t(byPlace[start])]->bounds().top();
        while (end < n && items[size_t(byPlace[end])]->bounds().top() - top <= kSameRow) ++end;
        std::stable_sort(byPlace.begin() + start, byPlace.begin() + end, [&](int a, int b) { return items[size_t(a)]->bounds().left() < items[size_t(b)]->bounds().left(); });
        start = end;
    }
    for (int r = 0; r < byPlace.size(); ++r) rank[byPlace[r]] = r;
    // The longest run of the list's objects whose ranks go up stays; the others are out of place.
    const int n = int(read.size());
    QVector<int> len(n, 1), prev(n, -1);
    int best = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j)
            if (rank[read[j]] < rank[read[i]] && len[j] + 1 > len[i]) {
                len[i] = len[j] + 1;
                prev[i] = j;
            }
        if (len[i] > len[best]) best = i;
    }
    QVector<bool> keep(n, false);
    for (int i = best; i >= 0; i = prev[i]) keep[i] = true;
    QVector<int> out;
    for (int i = 0; i < n; ++i)
        if (!keep[i]) out << read[i];
    return out;
}

QVector<PageAccessIssue> checkPageAccessibility(const Document &doc, bool slides, const std::function<QString(int)> &titleOf, int most)
{
    QVector<PageAccessIssue> out;
    const LayoutEnv env = Renderer::layoutEnv(doc);
    for (int p = 0; p < doc.pages.size() && out.size() < most; ++p) {
        if (titleOf && titleOf(p).trimmed().isEmpty())
            out.push_back({PageAccessIssue::NoPageTitle, PageAccessIssue::Warning, p, QString(),
                           QCoreApplication::translate("jp::PageAccess", "Slide %1 has no title. People who use a screen reader find a slide by its title.").arg(p + 1)});
        const QString picture = slides ? QCoreApplication::translate("jp::PageAccess", "A picture on slide %1").arg(p + 1) : QCoreApplication::translate("jp::PageAccess", "A picture on page %1").arg(p + 1);
        const QString shape = slides ? QCoreApplication::translate("jp::PageAccess", "A shape on slide %1").arg(p + 1) : QCoreApplication::translate("jp::PageAccess", "A shape on page %1").arg(p + 1);
        walkItems(doc.pages[p]->items, [&](const ItemPtr &it) {
            if (out.size() >= most || it->decorative || !it->altText.trimmed().isEmpty()) return;
            if (it->type() == ItemType::Picture)
                out.push_back({PageAccessIssue::MissingAltText, PageAccessIssue::Error, p, it->id,
                               QCoreApplication::translate("jp::PageAccess", "%1 has no alt text. Alt text tells a person who cannot see it what the picture shows.").arg(picture)});
            else if (it->type() == ItemType::Shape && !it->hasText())
                out.push_back({PageAccessIssue::MissingAltText, PageAccessIssue::Error, p, it->id,
                               QCoreApplication::translate("jp::PageAccess", "%1 has no alt text. Give it a description, or mark it as decoration.").arg(shape)});
        });
        const Page &pg = *doc.pages[p];
        // Text that is hard to read against what is behind it.
        for (int k = 0; k < int(pg.items.size()) && out.size() < most; ++k) {
            const Item &it = *pg.items[size_t(k)];
            if (it.hidden || (it.type() != ItemType::Text && it.type() != ItemType::Shape && it.type() != ItemType::Table) || !it.hasText()) continue;
            const Behind under = underneath(doc, pg, k, it.bounds());
            Worst worst;
            bool overPicture = false;
            if (it.type() == ItemType::Table) {
                const auto &t = static_cast<const TableItem &>(it);
                const Behind own = fillBehind(t.fill, doc, under);
                const Behind tableBack = own.colors.isEmpty() && !own.picture ? under : own;
                for (const TableCell &c : t.cells) {
                    if (c.covered) continue;
                    const Behind cell = fillBehind(c.fill, doc, tableBack);
                    const Behind &back = cell.colors.isEmpty() && !cell.picture ? tableBack : cell;
                    overPicture = overPicture || back.picture;
                    for (const QString &story : {c.storyId, c.storyId2})
                        if (!story.isEmpty()) judgeStory(doc, env, story, back, &worst);
                }
            } else {
                const QString story = it.type() == ItemType::Text ? static_cast<const TextItem &>(it).storyId : static_cast<const ShapeItem &>(it).storyId;
                const Behind own = fillBehind(it.fill, doc, under);
                const Behind &back = own.colors.isEmpty() && !own.picture ? under : own;
                overPicture = back.picture;
                judgeStory(doc, env, story, back, &worst);
            }
            const QString where = slides ? QCoreApplication::translate("jp::PageAccess", "slide %1").arg(p + 1) : QCoreApplication::translate("jp::PageAccess", "page %1").arg(p + 1);
            if (worst.ratio >= 0 && worst.margin < 0)
                out.push_back({PageAccessIssue::LowContrast, PageAccessIssue::Warning, p, it.id,
                               QCoreApplication::translate("jp::PageAccess", "Text on %1 is hard to read: its color and its background are too close (a contrast of %2 to 1, where %3 to 1 is needed for text this size). "
                                                                             "Choose a darker text color or a lighter background, or the reverse, so that people with low vision can read it.")
                                   .arg(where, QString::number(worst.ratio, 'f', 2), QString::number(neededContrast(worst.points, worst.bold), 'g', 2))});
            else if (overPicture)
                out.push_back({PageAccessIssue::LowContrast, PageAccessIssue::Tip, p, it.id,
                               QCoreApplication::translate("jp::PageAccess", "Text on %1 is over a picture. Check contrast by eye: the text must stand out from the picture wherever it crosses it.").arg(where)});
        }
        // Objects read in another order than their places suggest.
        const ItemList &items = pg.items;
        const QVector<int> moved = outOfReadingOrder(items);
        for (int k : moved) {
            if (out.size() >= most) break;
            const QString name = items[size_t(k)]->name.isEmpty() ? itemTypeName(items[size_t(k)]->type()) : items[size_t(k)]->name;
            out.push_back({PageAccessIssue::ReadingOrder, PageAccessIssue::Tip, p, items[size_t(k)]->id,
                           (slides ? QCoreApplication::translate("jp::PageAccess", "\"%1\" on slide %2 is read in a different order than its place on the slide suggests (which is top to bottom, then left to right).")
                                   : QCoreApplication::translate("jp::PageAccess", "\"%1\" on page %2 is read in a different order than its place on the page suggests (which is top to bottom, then left to right)."))
                               .arg(name)
                               .arg(p + 1) + QLatin1Char(' ') + QCoreApplication::translate("jp::PageAccess", "Open Reading Order to change it.")});
        }
    }
    return out;
}

} // namespace jp
