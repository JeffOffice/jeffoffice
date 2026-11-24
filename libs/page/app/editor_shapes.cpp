// Merge Shapes, Convert to Curve, Polygon, or Contour, and Break Apart: the outlines of shapes made into new shapes. Every shape made is unturned and sits on the page where
// the outline was, with the look of the shape it came from.
#include "app/editor.h"

#include "office/connectionsites.h"
#include "render/shapes.h"

#include "core/workthread.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <algorithm>

namespace jp {

namespace {

// The shape's outline on the page: its own points or its preset's drawing, turned and flipped as the shape is.
QPainterPath pageOutline(const ShapeItem &s)
{
    const QPainterPath own = s.customPath.isEmpty() ? shapePath(s.shape, s.rect.size(), s.adj) : s.customPath;
    return s.transform().map(own);
}

// A shape with the outline on the page: the look of `from`, no turn, and its text only when `keepText` is set.
std::shared_ptr<ShapeItem> shapeWith(const ShapeItem &from, const QPainterPath &onPage, bool keepText = false)
{
    auto s = std::static_pointer_cast<ShapeItem>(from.clone());
    if (!keepText) s->storyId.clear();
    s->gluePoints.clear();
    s->adj.clear();
    s->rotation = 0;
    s->flipH = s->flipV = false;
    const QRectF box = onPage.boundingRect();
    s->rect = box;
    s->customPath = QTransform::fromTranslate(-box.left(), -box.top()).map(onPage);
    return s;
}

// Where the ends of connectors attached to these shapes are on the page, noted before the shapes go.
struct Attach {
    LineItem::Glue *glue;
    QPointF at;
};

QVector<Attach> attachedTo(Document &doc, const QStringList &ids)
{
    QVector<Attach> out;
    doc.forEachItem([&](Item *it, int, const QString &) {
        if (it->type() != ItemType::Line) return;
        for (LineItem::Glue *g : {&static_cast<LineItem *>(it)->start, &static_cast<LineItem *>(it)->end}) {
            const Item *o = g->site >= 0 && ids.contains(g->id) ? doc.item(g->id) : nullptr;
            if (o && g->site < connectionSiteCount(*o)) out.push_back(Attach{g, connectionSite(*o, g->site)});
        }
    });
    return out;
}

// Each of those ends goes to the site of a new shape that is where it was (the middle of a side of the new shape's frame); an end with none is let go where it was.
void reattach(const QVector<Attach> &ends, const QVector<std::shared_ptr<ShapeItem>> &shapes)
{
    for (const Attach &e : ends) {
        *e.glue = LineItem::Glue();
        for (const auto &s : shapes) {
            bool found = false;
            for (int j = 0, n = connectionSiteCount(*s); j < n && !found; ++j)
                if (QLineF(connectionSite(*s, j), e.at).length() < 0.5) {
                    e.glue->id = s->id;
                    e.glue->site = j;
                    found = true;
                }
            if (found) break;
        }
    }
}

bool hasText(const Document &doc, const ShapeItem &s)
{
    const QTextDocument *d = s.storyId.isEmpty() ? nullptr : doc.storyDoc(s.storyId);
    return d && !d->isEmpty();
}

} // namespace

// The top-level selected shapes that can be changed (the reason when one cannot); they share one list.
static bool pickShapes(Document &doc, const QStringList &ids, QVector<ShapeItem *> *out, ItemList **list, QString *why, bool needTwo)
{
    auto refuse = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    if (ids.isEmpty() || (needTwo && ids.size() < 2)) return refuse(needTwo ? QCoreApplication::translate("jp::Editor", "Select two or more shapes first.") : QCoreApplication::translate("jp::Editor", "Select a shape first."));
    if (ids.size() > pathops::kMostShapes) return refuse(QCoreApplication::translate("jp::Editor", "Select no more than 64 shapes."));
    *list = nullptr;
    for (const QString &id : ids) {
        const Document::Loc at = doc.find(id);
        auto *s = dynamic_cast<ShapeItem *>(at.item);
        if (!s || s->isArt()) return refuse(QCoreApplication::translate("jp::Editor", "Only shapes can be changed this way. Artwork and other objects stay as they are."));
        if (s->locked) return refuse(QCoreApplication::translate("jp::Editor", "An object that is locked stays as it is."));
        if (at.parent) return refuse(QCoreApplication::translate("jp::Editor", "An object inside a group stays as it is. Select the whole group."));
        if (!*list) *list = at.list;
        else if (*list != at.list) return refuse(QCoreApplication::translate("jp::Editor", "Select shapes that are on the same page."));
        out->push_back(s);
    }
    return true;
}

int Editor::mergeTextCount() const
{
    int n = 0;
    for (const QString &id : topLevelSelection())
        if (const auto *s = dynamic_cast<const ShapeItem *>(m_doc->item(id)); s && hasText(*m_doc, *s)) ++n;
    return n;
}

bool Editor::mergeShapes(pathops::Op op, QString *why, int *made, int *textRemoved)
{
    if (made) *made = 0;
    if (textRemoved) *textRemoved = 0;
    QVector<ShapeItem *> shapes;
    ItemList *list = nullptr;
    if (!pickShapes(*m_doc, topLevelSelection(), &shapes, &list, why, true)) return false;
    QVector<QPainterPath> outlines;
    for (const ShapeItem *s : shapes) outlines << pageOutline(*s);
    int refused = 0;
    QVector<QPainterPath> result;
    if (const qint64 work = pathops::estimateWork(op, outlines); work <= pathops::kQuickWork || work > pathops::kMostWork) {   // (one past the most is refused at once, so it needs no box)
        result = pathops::merge(op, outlines, &refused);
    } else {
        // (the work is counted before it starts and refused past pathops::kMostWork; what is left runs on a thread of its own, and the window stays alive and can cancel it)
        std::atomic<bool> cancel{false};
        QEventLoop loop;
        Q_EMIT busyStarted(tr("Merging shapes…"), &cancel);
        {
            WorkThread worker([&] {
                result = pathops::merge(op, outlines, &refused, [&](int step, int) {
                    if (mergeStepHook) mergeStepHook(step, cancel);
                    return !cancel.load();
                });
                QMetaObject::invokeMethod(&loop, &QEventLoop::quit, Qt::QueuedConnection);
            });
            if (worker.started()) loop.exec();
            else result = pathops::merge(op, outlines, &refused);
        }
        Q_EMIT busyEnded();
    }
    if (result.isEmpty()) {
        if (why)
            *why = refused == 0 ? tr("The shapes leave nothing.") : refused == 2 ? tr("That would cut the shapes into too many pieces.") : refused == 3 ? tr("The shapes are too complex to merge in a reasonable time.")
                : refused == 4 ? tr("Cancelled. The shapes are as they were.") : tr("The shapes are too complex to merge.");
        return false;
    }
    const ShapeItem first = *shapes.first();   // (a copy: the shapes go from the list below)
    QStringList gone;
    int withText = 0;
    for (const ShapeItem *s : shapes) {
        gone << s->id;
        if (hasText(*m_doc, *s)) ++withText;
    }
    const QVector<Attach> ends = attachedTo(*m_doc, gone);
    int at = int(list->size());
    for (int i = 0; i < list->size(); ++i)
        if (gone.contains((*list)[i]->id)) at = std::min(at, i);
    beginChange(op == pathops::Op::Union ? tr("Union") : op == pathops::Op::Combine ? tr("Combine") : op == pathops::Op::Subtract ? tr("Subtract")
                : op == pathops::Op::Intersect ? tr("Intersect") : tr("Fragment"));
    // (what waited for a click on a shape that goes is let go, as for a delete: the first shape stays, with its number)
    if (itemsDeleting) {
        QStringList leaving = gone;
        leaving.removeAll(first.id);
        itemsDeleting(leaving);
    }
    list->erase(std::remove_if(list->begin(), list->end(), [&](const ItemPtr &it) { return gone.contains(it->id); }), list->end());
    at = std::min(at, int(list->size()));
    QStringList ids;
    QVector<std::shared_ptr<ShapeItem>> made2;
    for (const QPainterPath &p : result) {
        const auto s = shapeWith(first, p);
        if (made2.isEmpty()) s->sourceId = first.sourceId;   // (the first piece is the shape selected first: what a file's animations aim at stays aimed at it)
        list->insert(list->begin() + at++, s);
        ids << s->id;
        made2 << s;
    }
    reattach(ends, made2);
    select(ids);
    endChange();
    if (made) *made = int(result.size());
    if (textRemoved) *textRemoved = withText;
    return true;
}

bool Editor::convertOutline(Convert how, QString *why)
{
    QVector<ShapeItem *> shapes;
    ItemList *list = nullptr;
    if (!pickShapes(*m_doc, topLevelSelection(), &shapes, &list, why, false)) return false;
    // What each shape becomes, worked out before anything is changed.
    QVector<QPainterPath> to(shapes.size());
    QVector<bool> change(shapes.size(), false);
    bool any = false;
    for (int i = 0; i < shapes.size(); ++i) {
        const ShapeItem &s = *shapes[i];
        if (how == Convert::Curve) {
            change[i] = s.customPath.isEmpty();
        } else if (how == Convert::Polygon) {
            if (s.customPath.elementCount() > pathops::kMostElements) continue;
            to[i] = pathops::polygon(s.customPath.isEmpty() ? shapePath(s.shape, s.rect.size(), s.adj) : s.customPath);
            change[i] = to[i].elementCount() <= pathops::kMostResultElements && !to[i].isEmpty();
        } else if (!s.stroke.isNone() && s.stroke.width > 0 && s.customPath.elementCount() <= pathops::kMostElements) {
            to[i] = pathops::contour(pageOutline(s), s.stroke.width);
            change[i] = !to[i].isEmpty() && to[i].elementCount() <= pathops::kMostResultElements;
        }
        any = any || change[i];
    }
    if (!any) {
        if (why) *why = how == Convert::Curve ? tr("The shapes are curves already.") : how == Convert::Polygon ? tr("The shapes cannot be made of straight lines.") : tr("Give the shapes an outline first.");
        return false;
    }
    beginChange(how == Convert::Curve ? tr("Convert to Curve") : how == Convert::Polygon ? tr("Convert to Polygon") : tr("Convert to Contour"));
    QStringList ids;
    for (int i = 0; i < shapes.size(); ++i) {
        ShapeItem *s = shapes[i];
        if (!change[i]) {
            ids << s->id;
            continue;
        }
        if (how == Convert::Contour) {
            // The outline of the line is the new shape, filled with the line's color and with no line.
            const auto c = shapeWith(*s, to[i], true);
            c->fill = Fill::solid(s->stroke.color);
            c->stroke = Stroke::none();
            const QVector<Attach> ends = attachedTo(*m_doc, QStringList{s->id});
            for (ItemPtr &slot : *list)
                if (slot.get() == s) {
                    slot = c;
                    break;
                }
            reattach(ends, QVector<std::shared_ptr<ShapeItem>>{c});
            ids << c->id;
        } else {
            // (a shape with its own outline has the four sites of its sides, not its preset's: a connector attached to one keeps its side, and one attached to a glue point its number)
            struct End { LineItem::Glue *glue; int side, glue_index; };
            QVector<End> ends;
            const int before = geometrySiteCount(*s);
            m_doc->forEachItem([&](Item *it, int, const QString &) {
                if (it->type() != ItemType::Line) return;
                for (LineItem::Glue *g : {&static_cast<LineItem *>(it)->start, &static_cast<LineItem *>(it)->end})
                    if (g->id == s->id && g->site >= 0) ends.push_back(End{g, g->site < before ? sideOfConnectionSite(*s, g->site) : -1, g->site - before});
            });
            s->customPath = how == Convert::Curve ? shapePath(s->shape, s->rect.size(), s->adj) : to[i];
            for (const End &e : ends) e.glue->site = e.side >= 0 ? connectionSiteOnSide(*s, e.side) : geometrySiteCount(*s) + e.glue_index;
            ids << s->id;
        }
    }
    select(ids);
    endChange();
    return true;
}

bool Editor::breakApart(QString *why, int *made)
{
    if (made) *made = 0;
    QVector<ShapeItem *> shapes;
    ItemList *list = nullptr;
    if (!pickShapes(*m_doc, topLevelSelection(), &shapes, &list, why, false)) return false;
    QVector<QVector<QPainterPath>> parts(shapes.size());
    int total = 0;
    for (int i = 0; i < shapes.size(); ++i) {
        const ShapeItem &s = *shapes[i];
        if (s.customPath.elementCount() > pathops::kMostElements) continue;
        parts[i] = pathops::subpaths(pageOutline(s));
        if (parts[i].size() < 2) parts[i].clear();
        total += parts[i].size();
    }
    if (total == 0) {
        if (why) *why = tr("Nothing to break apart: each shape is one piece.");
        return false;
    }
    if (total > pathops::kMostPieces) {
        if (why) *why = tr("That would make more than 256 shapes.");
        return false;
    }
    beginChange(tr("Break Apart"));
    QStringList ids;
    int count = 0;
    for (int i = 0; i < shapes.size(); ++i) {
        ShapeItem *s = shapes[i];
        if (parts[i].isEmpty()) {
            ids << s->id;
            continue;
        }
        const ShapeItem look = *s;
        const QVector<Attach> ends = attachedTo(*m_doc, QStringList{s->id});
        QVector<std::shared_ptr<ShapeItem>> pieces;
        int at = 0;
        for (int k = 0; k < list->size(); ++k)
            if ((*list)[k].get() == s) at = k;
        list->erase(list->begin() + at);   // (s is not used after this)
        for (const QPainterPath &p : parts[i]) {
            const auto c = shapeWith(look, p, pieces.isEmpty());   // (the text stays with the first piece)
            list->insert(list->begin() + at++, c);
            pieces << c;
            ids << c->id;
            ++count;
        }
        reattach(ends, pieces);
    }
    select(ids);
    endChange();
    if (made) *made = count;
    return true;
}

bool Editor::rerouteConnectors(QString *why, int *changed)
{
    if (changed) *changed = 0;
    const QStringList selected = topLevelSelection();
    QSet<QString> picked(selected.begin(), selected.end());
    QHash<QString, const Item *> byId;
    QVector<LineItem *> lines;
    m_doc->forEachItem([&](Item *it, int, const QString &) {
        byId.insert(it->id, it);
        if (it->type() != ItemType::Line) return;
        auto *l = static_cast<LineItem *>(it);
        if (l->locked || (l->start.id.isEmpty() && l->end.id.isEmpty())) return;
        if (picked.contains(l->id) || picked.contains(l->start.id) || picked.contains(l->end.id)) lines << l;
    });
    // The site of `object` nearest `from` (the lowest number of those equally near); -1 for an object that is gone.
    auto nearest = [&](const QString &object, const QPointF &from) {
        const Item *o = byId.value(object);
        if (!o || o->type() == ItemType::Line) return -1;
        int best = -1;
        double bestAway = 0;
        for (int i = 0, n = connectionSiteCount(*o); i < n; ++i) {
            const double away = QLineF(connectionSite(*o, i), from).length();
            if (best < 0 || away < bestAway - 1e-9) {
                best = i;
                bestAway = away;
            }
        }
        return best;
    };
    struct Move { LineItem *line; int start, end; };
    QVector<Move> moves;
    for (LineItem *l : lines) {
        int s = l->start.site, e = l->end.site;
        const Item *so = byId.value(l->start.id), *eo = byId.value(l->end.id);
        // (both ends attached: the pair of sites nearest each other, a few passes, each end moving to the site nearest the other end's site until neither moves)
        for (int pass = 0; pass < 4; ++pass) {
            const int ns = so && (s >= 0) ? nearest(l->start.id, eo && e >= 0 ? connectionSite(*eo, e) : l->p2) : s;
            const int ne = eo && (e >= 0) ? nearest(l->end.id, so && ns >= 0 ? connectionSite(*so, ns) : l->p1) : e;
            if (ns == s && ne == e) break;
            s = ns;
            e = ne;
        }
        if (s != l->start.site || e != l->end.site) moves << Move{l, s, e};
    }
    if (moves.isEmpty()) {
        if (why) *why = lines.isEmpty() ? tr("Select a shape that a connector is attached to, or a connector.") : tr("The connectors are on their shortest paths already.");
        return false;
    }
    beginChange(tr("Reroute Connectors"));
    for (const Move &m : moves) {
        m.line->start.site = m.start;
        m.line->end.site = m.end;
    }
    endChange();   // (the connectors follow their sites as a change ends)
    if (changed) *changed = int(moves.size());
    return true;
}

} // namespace jp
