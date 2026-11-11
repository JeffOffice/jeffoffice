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

// What is painted on a page, bottom first: the objects of its master pages (as they are painted under it, without placeholder prompts), then its own, groups taken apart.
struct Stack {
    std::vector<const Item *> items;
    int firstOfPage = 0;   // where the page's own objects begin
};

void addItems(const ItemList &list, bool skipPlaceholders, std::vector<const Item *> *out)
{
    for (const ItemPtr &it : list) {
        if (it->hidden || (skipPlaceholders && it->role.startsWith(QLatin1String("ph:")))) continue;
        if (it->type() == ItemType::Group) addItems(static_cast<const GroupItem &>(*it).children, false, out);
        else out->push_back(it.get());
    }
}

Stack stackOf(const Document &doc, const Page &pg)
{
    Stack st;
    if (pg.showMasterItems) {
        const QVector<MasterPage *> chain = doc.masterChain(doc.masterFor(pg));
        int lowest = int(chain.size()) - 1;
        while (lowest > 0 && chain[lowest]->showMasterItems) --lowest;
        for (int i = std::max(lowest, 0); i < chain.size(); ++i) addItems(chain[i]->items, true, &st.items);
    }
    st.firstOfPage = int(st.items.size());
    addItems(pg.items, false, &st.items);
    return st;
}

// What is under the object at `index` of the stack at the point `at`: the nearest picture there, or the fills there (a see-through fill is worked over what is below it), down to
// the page. Done by going down once, so the cost is the number of objects, however they stack.
Behind underneath(const Document &doc, const Page &pg, const Stack &st, int index, const QPointF &at)
{
    std::vector<const Item *> seeThrough;   // the fills passed that let what is below show, nearest first
    Behind base;
    bool found = false;
    for (int k = index - 1; k >= 0 && !found; --k) {
        const Item &it = *st.items[size_t(k)];
        if (it.type() == ItemType::Line || !it.bounds().contains(at)) continue;
        if (it.type() == ItemType::Picture) {
            base.picture = true;
            found = true;
            break;
        }
        const Behind own = fillBehind(it.fill, doc, Behind());
        if (own.picture) {
            base.picture = true;
            found = true;
        } else if (!own.colors.isEmpty()) {
            if (it.fill.transparency > 0.05) seeThrough.push_back(&it);
            else {
                base = own;
                found = true;
            }
        }
    }
    if (!found) base = pageBehind(doc, pg);
    for (auto it = seeThrough.rbegin(); it != seeThrough.rend() && !base.picture; ++it) base = fillBehind((*it)->fill, doc, base);
    return base;
}

// The worst contrast of the words of one story over the colors in `behind`: how far under (or over) what the words' size needs, and what that came to.
struct Worst {
    double ratio = -1, margin = 1e9, points = 0;
    bool bold = false;
};

// `scale` is how much smaller the words are drawn than their size says (a box that shrinks its text to fit).
void judgeStory(const Document &doc, const LayoutEnv &env, const QString &storyId, const QVector<QColor> &behind, double scale, Worst *worst)
{
    const QTextDocument *sd = doc.storyDoc(storyId);
    if (!sd || behind.isEmpty()) return;
    for (QTextBlock block = sd->begin(); block.isValid(); block = block.next())
        for (auto frag = block.begin(); !frag.atEnd(); ++frag) {
            if (frag.fragment().text().trimmed().isEmpty()) continue;
            const QTextCharFormat cf = frag.fragment().charFormat();
            if (cf.hasProperty(tp::HiddenText) && cf.boolProperty(tp::HiddenText)) continue;
            const QTextCharFormat r = resolveCharFormat(cf, env, behind.first());
            if (r.foreground().style() != Qt::SolidPattern) continue;
            const QColor text = r.foreground().color();
            // (a run with a highlight is over that)
            QVector<QColor> over = behind;
            if (r.background().style() == Qt::SolidPattern && r.background().color().alpha() > 0) over = {r.background().color()};
            const double points = (cf.fontPointSize() > 0 ? cf.fontPointSize() : 11) * scale;
            const bool bold = cf.fontWeight() >= QFont::Bold || r.fontWeight() >= QFont::Bold;
            const double need = neededContrast(points, bold);
            for (const QColor &o : over) {
                // (see-through text shows what is behind it: its mix with it)
                const QColor shown = text.alpha() < 255 ? mix(o, QColor(text.red(), text.green(), text.blue()), text.alphaF()) : text;
                const double ratio = contrastRatio(shown, o);
                if (ratio - need < worst->margin) {
                    worst->margin = ratio - need;
                    worst->ratio = ratio;
                    worst->points = points;
                    worst->bold = bold;
                }
            }
        }
}

// The places a box's words may be: a few points across the part of the box they fill (about a line for each paragraph, from the top, the middle, or the bottom as the box sets them).
QVector<QPointF> samplePoints(const Document &doc, const QString &storyId, const QRectF &box, const QMarginsF &insets, VAlign valign, double scale)
{
    QRectF text = box.marginsRemoved(insets);
    if (text.width() <= 0 || text.height() <= 0) text = box;
    if (const QTextDocument *sd = doc.storyDoc(storyId)) {
        double size = 11;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next())
            for (auto f = b.begin(); !f.atEnd(); ++f) size = std::max(size, f.fragment().charFormat().fontPointSize());
        const double high = std::min(text.height(), sd->blockCount() * size * scale * 1.2);
        if (valign == VAlign::Top) text.setHeight(high);
        else if (valign == VAlign::Bottom) text.setTop(text.bottom() - high);
        else text = QRectF(text.left(), text.center().y() - high / 2, text.width(), high);
    }
    QVector<QPointF> out;
    for (double fy : {0.05, 0.5, 0.95})
        for (double fx : {0.05, 0.5, 0.95}) out << QPointF(text.left() + text.width() * fx, text.top() + text.height() * fy);
    return out;
}

} // namespace

double contrastRatio(const QColor &a, const QColor &b)
{
    const double la = luminance(a), lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

double neededContrast(double points, bool bold) { return points >= 18 || (bold && points >= 14) ? 3.0 : 4.5; }

QVector<int> outOfReadingOrder(const ItemList &items, const QSizeF &pageSize)
{
    const double pageArea = pageSize.isEmpty() ? 0 : pageSize.width() * pageSize.height();
    QVector<int> read;                // the objects that are read, by number in the list
    for (int i = 0; i < int(items.size()); ++i) {
        const Item &it = *items[size_t(i)];
        if (it.hidden || it.decorative || it.type() == ItemType::Line || (it.type() == ItemType::Shape && !it.hasText() && it.altText.trimmed().isEmpty())) continue;
        // a picture over most of the page is a backdrop: the words on it are read as if it were not there
        if (it.type() == ItemType::Picture && pageArea > 0 && it.bounds().width() * it.bounds().height() >= 0.5 * pageArea) continue;
        read << i;
    }
    if (read.size() < 2 || read.size() > 2000) return {};
    auto top = [&](int i) { return items[size_t(i)]->bounds().top(); };
    auto bottom = [&](int i) { return items[size_t(i)]->bounds().bottom(); };
    // Their order by place: rows from the top, each row from the left. An object is in the row of the first one in it if their vertical spans overlap by half of the shorter.
    QVector<int> byPlace = read;
    std::stable_sort(byPlace.begin(), byPlace.end(), [&](int a, int b) { return top(a) < top(b); });
    for (int start = 0, n = int(byPlace.size()); start < n;) {
        int end = start + 1;
        const int lead = byPlace[start];
        while (end < n) {
            const int o = byPlace[end];
            const double overlap = std::min(bottom(lead), bottom(o)) - std::max(top(lead), top(o));
            const double shorter = std::min(bottom(lead) - top(lead), bottom(o) - top(o));
            if (overlap < 0.5 * shorter || shorter <= 0) break;
            ++end;
        }
        std::stable_sort(byPlace.begin() + start, byPlace.begin() + end, [&](int a, int b) { return items[size_t(a)]->bounds().left() < items[size_t(b)]->bounds().left(); });
        start = end;
    }
    QVector<int> rank(int(items.size()), -1);
    for (int r = 0; r < byPlace.size(); ++r) rank[byPlace[r]] = r;
    // The longest run of the list's objects whose ranks go up stays; the others are out of place (of two runs as long, the one that keeps the object placed earlier).
    const int n = int(read.size());
    QVector<int> len(n, 1), prev(n, -1);
    int best = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j)
            if (rank[read[j]] < rank[read[i]] && (len[j] + 1 > len[i] || (len[j] + 1 == len[i] && prev[i] >= 0 && rank[read[j]] < rank[read[prev[i]]]))) {
                len[i] = len[j] + 1;
                prev[i] = j;
            }
        if (len[i] > len[best] || (len[i] == len[best] && rank[read[i]] < rank[read[best]])) best = i;
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
        // A table whose first row is not a header row, and a video (nothing here carries captions for one).
        walkItems(doc.pages[p]->items, [&](const ItemPtr &it) {
            if (out.size() >= most || it->decorative) return;
            if (it->type() == ItemType::Table && static_cast<const TableItem &>(*it).rows > 0 && !static_cast<const TableItem &>(*it).header)
                out.push_back({PageAccessIssue::NoTableHeader, PageAccessIssue::Warning, p, it->id,
                               (slides ? QCoreApplication::translate("jp::PageAccess", "A table on slide %1 has no header row. A header row names the columns for people who use a screen reader.")
                                       : QCoreApplication::translate("jp::PageAccess", "A table on page %1 has no header row. A header row names the columns for people who use a screen reader."))
                                   .arg(p + 1)});
            else if (it->type() == ItemType::Picture && static_cast<const PictureItem &>(*it).mediaKind == QLatin1String("video"))
                out.push_back({PageAccessIssue::NoCaptions, PageAccessIssue::Tip, p, it->id,
                               (slides ? QCoreApplication::translate("jp::PageAccess", "Videos need captions for people who can't hear them. Check the video on slide %1.")
                                       : QCoreApplication::translate("jp::PageAccess", "Videos need captions for people who can't hear them. Check the video on page %1."))
                                   .arg(p + 1)});
        });
        // Text that is hard to read against what is behind it, in each object that has some (those in groups too), where the text is.
        const Page &pg = *doc.pages[p];
        const Stack st = stackOf(doc, pg);
        const QString where = slides ? QCoreApplication::translate("jp::PageAccess", "slide %1").arg(p + 1) : QCoreApplication::translate("jp::PageAccess", "page %1").arg(p + 1);
        for (int k = st.firstOfPage; k < int(st.items.size()) && out.size() < most; ++k) {
            const Item &it = *st.items[size_t(k)];
            if (it.type() != ItemType::Text && it.type() != ItemType::Shape && it.type() != ItemType::Table) continue;
            Worst worst;
            bool overPicture = false;
            // the colors behind a word at each place, from the object's own fill, else from what is below it
            auto judge = [&](const QString &story, const QRectF &box, const QMarginsF &insets, VAlign valign, double scale, const Fill &fill, const Fill &fallback) {
                if (story.isEmpty()) return;
                QVector<QColor> colors;
                for (const QPointF &pt : samplePoints(doc, story, box, insets, valign, scale)) {
                    const Behind below = underneath(doc, pg, st, k, pt);
                    Behind own = fillBehind(fill, doc, below);
                    if (own.colors.isEmpty() && !own.picture) own = fillBehind(fallback, doc, below);
                    if (own.colors.isEmpty() && !own.picture) own = below;
                    overPicture = overPicture || own.picture;
                    for (const QColor &c : std::as_const(own.colors))
                        if (!colors.contains(c)) colors << c;
                }
                judgeStory(doc, env, story, colors, scale, &worst);
            };
            if (it.type() == ItemType::Table) {
                const auto &t = static_cast<const TableItem &>(it);
                for (int row = 0; row < t.rows; ++row)
                    for (int col = 0; col < t.cols; ++col) {
                        const TableCell &c = t.cell(row, col);
                        if (c.covered) continue;
                        const QRectF cr = t.cellRect(row, col).translated(t.rect.topLeft());
                        for (const QString &story : {c.storyId, c.storyId2}) judge(story, cr, c.margins, c.valign, 1.0, c.fill, t.fill);
                    }
            } else if (it.type() == ItemType::Text) {
                const auto &t = static_cast<const TextItem &>(it);
                judge(t.storyId, t.rect, t.insets, t.valign, t.autofit != TextItem::NoAutofit ? std::clamp(t.fitScale, 0.01, 1.0) : 1.0, t.fill, Fill());
            } else {
                const auto &sh = static_cast<const ShapeItem &>(it);
                judge(sh.storyId, sh.rect, sh.insets, sh.valign, sh.shrinks ? std::clamp(sh.fitScale, 0.01, 1.0) : 1.0, sh.fill, Fill());
            }
            // (the contrast is shown cut, not rounded, so that "4.50" is never shown for a text that fails)
            auto cut = [](double v) { return QString::number(std::floor(v * 100 + 1e-9) / 100, 'f', 2); };
            if (worst.ratio >= 0 && worst.margin < 0)
                out.push_back({PageAccessIssue::LowContrast, PageAccessIssue::Warning, p, it.id,
                               QCoreApplication::translate("jp::PageAccess", "Text on %1 is hard to read: its color and its background are too close (a contrast of %2 to 1, where %3 to 1 is needed for text this size). "
                                                                             "Choose a darker text color or a lighter background, or the reverse, so that people with low vision can read it.")
                                   .arg(where, cut(worst.ratio), QString::number(neededContrast(worst.points, worst.bold), 'g', 2))});
            else if (overPicture)
                out.push_back({PageAccessIssue::LowContrast, PageAccessIssue::Tip, p, it.id,
                               QCoreApplication::translate("jp::PageAccess", "Text on %1 is over a picture. Check contrast by eye: the text must stand out from the picture wherever it crosses it.").arg(where)});
        }
        // Objects read in another order than their places suggest.
        const ItemList &items = pg.items;
        const QVector<int> moved = outOfReadingOrder(items, doc.pageSize());
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
