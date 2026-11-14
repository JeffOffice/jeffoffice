#include "core/linkareas.h"

#include "render/renderer.h"
#include "text/textengine.h"

#include <QTextBlock>
#include <QTextDocument>

namespace jp::deck {

namespace {

bool hasAnchor(const QTextDocument *td)
{
    if (!td) return false;
    for (QTextBlock b = td->begin(); b.isValid(); b = b.next())
        for (auto f = b.begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            if (cf.isAnchor() && !cf.anchorHref().trimmed().isEmpty()) return true;
        }
    return false;
}

void collect(const Document &d, LayoutCache &cache, const RenderOptions &opt, int slide, const ItemList &items, QVector<LinkArea> &out)
{
    for (const ItemPtr &ptr : items) {
        const Item &it = *ptr;
        if (it.hidden) continue;
        if (!it.hyperlink.trimmed().isEmpty() && !it.bounds().isEmpty()) out.push_back({it.bounds(), it.hyperlink.trimmed(), &it});
        if (it.type() == ItemType::Group) {
            collect(d, cache, opt, slide, static_cast<const GroupItem &>(it).children, out);
        } else if (it.type() == ItemType::Text) {
            const auto &t = static_cast<const TextItem &>(it);
            const QTextDocument *td = d.storyDoc(t.storyId);
            if (!hasAnchor(td)) continue;
            const auto fl = cache.textFrame(d, t, slide + 1, opt);
            if (!fl.layout) continue;
            for (QTextBlock b = td->begin(); b.isValid(); b = b.next())
                for (auto f = b.begin(); !f.atEnd(); ++f) {
                    const QTextCharFormat cf = f.fragment().charFormat();
                    if (!cf.isAnchor() || cf.anchorHref().trimmed().isEmpty()) continue;
                    for (const QRectF &r : fl.layout->rangeRects(fl.frame, f.fragment().position(), f.fragment().position() + f.fragment().length()))
                        if (!r.isEmpty()) out.push_back({t.transform().mapRect(r), cf.anchorHref().trimmed(), &it});
                }
        } else if (it.type() == ItemType::Shape) {
            // The words of a shape that are linked: the shape's own box is the area.
            const QTextDocument *td = d.storyDoc(static_cast<const ShapeItem &>(it).storyId);
            if (!it.hyperlink.trimmed().isEmpty() || !td) continue;
            for (QTextBlock b = td->begin(); b.isValid(); b = b.next())
                for (auto f = b.begin(); !f.atEnd(); ++f) {
                    const QTextCharFormat cf = f.fragment().charFormat();
                    if (cf.isAnchor() && !cf.anchorHref().trimmed().isEmpty()) {
                        out.push_back({it.bounds(), cf.anchorHref().trimmed(), &it});
                        goto next;
                    }
                }
        }
    next:;
    }
}

} // namespace

namespace {
thread_local qint64 g_runs = 0;
}

qint64 linkAreaRunsForTest() { return g_runs; }

QVector<LinkArea> linkAreas(const Document &d, LayoutCache &cache, int slide, const RenderOptions &opt)
{
    ++g_runs;
    QVector<LinkArea> out;
    if (slide < 0 || slide >= d.pages.size()) return out;
    collect(d, cache, opt, slide, d.pages[slide]->items, out);
    return out;
}

} // namespace jp::deck
