#include "core/linkareas.h"
#include "core/items.h"

#include "render/renderer.h"
#include "text/textengine.h"

#include <QTextBlock>
#include <QTextDocument>

namespace jp::deck {

namespace {

// A link the document gives, as the show follows it: one that reads like an object's click area (see triggerLink) is an ordinary link to nowhere, never a click on an object.
QString documentLink(const QString &address)
{
    const QString link = address.trimmed();
    return triggerOfLink(link) ? QStringLiteral("x-") + link : link;
}

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

void collect(const Document &d, LayoutCache &cache, const RenderOptions &opt, int slide, const ItemList &items, QVector<LinkArea> &out, bool withHover, const QSet<int> &triggers)
{
    for (const ItemPtr &ptr : items) {
        const Item &it = *ptr;
        if (it.hidden) continue;
        // Before the object's own link: where both are, the link wins.
        if (it.sourceId > 0 && triggers.contains(it.sourceId) && !it.bounds().isEmpty()) out.push_back({it.bounds(), triggerLink(it.sourceId), &it});
        if (!it.hyperlink.trimmed().isEmpty() && !it.bounds().isEmpty()) out.push_back({it.bounds(), documentLink(it.hyperlink), &it});
        if (withHover && !it.hoverLink.trimmed().isEmpty() && !it.bounds().isEmpty()) out.push_back({it.bounds(), documentLink(it.hoverLink), &it, true});
        if (it.type() == ItemType::Group) {
            collect(d, cache, opt, slide, static_cast<const GroupItem &>(it).children, out, withHover, triggers);
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
                        if (!r.isEmpty()) out.push_back({t.transform().mapRect(r), documentLink(cf.anchorHref()), &it});
                }
        } else if (it.type() == ItemType::Shape) {
            // The words of a shape that are linked: the shape's own box is the area.
            const QTextDocument *td = d.storyDoc(static_cast<const ShapeItem &>(it).storyId);
            if (!it.hyperlink.trimmed().isEmpty() || !td) continue;
            for (QTextBlock b = td->begin(); b.isValid(); b = b.next())
                for (auto f = b.begin(); !f.atEnd(); ++f) {
                    const QTextCharFormat cf = f.fragment().charFormat();
                    if (cf.isAnchor() && !cf.anchorHref().trimmed().isEmpty()) {
                        out.push_back({it.bounds(), documentLink(cf.anchorHref()), &it});
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

QString triggerLink(int shape) { return QStringLiteral("jdtrigger:") + QString::number(shape); }

int triggerOfLink(const QString &link)
{
    const QString head = QStringLiteral("jdtrigger:");
    if (!link.startsWith(head)) return 0;
    bool ok = false;
    const int n = link.mid(head.size()).toInt(&ok);
    return ok && n > 0 ? n : 0;
}

QSet<int> clickableTriggerShapes(const Document &d, LayoutCache &cache, int slide, const QSet<int> &triggers)
{
    QSet<int> out;
    if (triggers.isEmpty()) return out;
    // Where the slide is: an object wholly off it can't be clicked.
    const QRectF page(QPointF(0, 0), d.pageSize());
    for (const LinkArea &a : linkAreas(d, cache, slide, RenderOptions(), false, triggers))
        if (a.item && a.rect.intersects(page) && a.item->hyperlink.trimmed().isEmpty() && a.link == triggerLink(a.item->sourceId)) out.insert(a.item->sourceId);
    return out;
}

QVector<LinkArea> linkAreas(const Document &d, LayoutCache &cache, int slide, const RenderOptions &opt, bool withHover, const QSet<int> &triggers)
{
    ++g_runs;
    QVector<LinkArea> out;
    if (slide < 0 || slide >= d.pages.size()) return out;
    collect(d, cache, opt, slide, d.pages[slide]->items, out, withHover, triggers);
    return out;
}

} // namespace jp::deck
