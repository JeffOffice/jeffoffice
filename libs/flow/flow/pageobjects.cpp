// The objects set at places on a page: pictures, rules, boxes, and text boxes anchored to a paragraph, the margin, or the page. (The paginator's functions for
// them are here, apart from its layout of the text.)

#include "flow/paginator.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::flow {

QVector<Paginator::Anchor> Paginator::anchorsIn(const QTextDocument *story, int from, int to, const QVector<AnchorLine> &lines, const QHash<int, QVector<int>> *byBlock) const
{
    QVector<Anchor> out;
    if (m_doc->objects.isEmpty()) return out;
    // The lines by the paragraph they are in (found once for a page, not for each anchor).
    QHash<int, QVector<int>> own;
    if (!byBlock) {
        for (int i = 0; i < lines.size(); ++i) own[lines[i].block] << i;
        byBlock = &own;
    }
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to && out.size() < kMostObjects; b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const int id = frag.charFormat().intProperty(fp::AnchorId);
            if (id <= 0 || id > m_doc->objects.size() || frag.position() >= to || frag.position() + frag.length() <= from) continue;
            const auto held = byBlock->constFind(b.position());
            if (held == byBlock->constEnd()) continue;
            for (int k = std::max(frag.position(), from); k < std::min(frag.position() + frag.length(), to); ++k) {
                Anchor a;
                a.object = id;
                a.pos = k;
                bool paragraphSeen = false, found = false;
                for (const int at : held.value()) {
                    const AnchorLine &l = lines[at];
                    if (!paragraphSeen) {
                        a.paragraphTop = l.rect.top() - l.pushed;
                        a.columnLeft = l.columnLeft;
                        a.lineTop = l.rect.top() - l.pushed;
                        a.x = l.rect.left();
                        paragraphSeen = true;
                    }
                    if (!found && k >= l.pos && k < l.upTo()) {
                        a.lineTop = l.rect.top() - l.pushed;
                        a.x = l.rect.left();
                        a.columnLeft = l.columnLeft;
                        found = true;
                    }
                }
                if (paragraphSeen) out << a;
            }
        }
    return out;
}

Paginator::AnchorLine Paginator::anchorLineOf(const StoryLayout::LineBrief &ln, const QRectF &rect, double columnLeft) const
{
    AnchorLine l{ln.block, ln.pos, ln.end, rect, columnLeft, ln.pushed, ln.end};
    if (!ln.cut && ln.line == ln.lines - 1) {
        const QTextBlock b = m_doc->story()->findBlock(ln.block);
        if (b.isValid()) l.reach = std::max(ln.end, b.position() + b.length() - 1);
    }
    return l;
}

QString Paginator::imageIdOf(Document *d, int image)
{
    if (image < 0 || image >= m_doc->images.size()) return QString();
    const auto found = m_imageIds.constFind(image);
    if (found != m_imageIds.constEnd()) return found.value();
    const FlowImage &img = m_doc->images[image];
    const QString id = d->addImage(img.bytes, img.format);
    m_imageIds.insert(image, id);
    return id;
}

void Paginator::resolveImages(QTextDocument *story, Document *d)
{
    QVector<std::tuple<int, int, QString>> changes;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
            if (json.isEmpty() || !json.contains(kFlowImagePrefix)) continue;
            QJsonObject o = QJsonDocument::fromJson(json.toUtf8()).object();
            const QString ref = o.value(QStringLiteral("image")).toString();
            if (!ref.startsWith(kFlowImagePrefix)) continue;
            o.insert(QStringLiteral("image"), imageIdOf(d, ref.mid(kFlowImagePrefix.size()).toInt()));
            changes.push_back({it.fragment().position(), it.fragment().length(), QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact))});
        }
    for (const auto &c : changes) {
        QTextCursor cur(story);
        cur.setPosition(std::get<0>(c));
        cur.setPosition(std::get<0>(c) + std::get<1>(c), QTextCursor::KeepAnchor);
        QTextCharFormat cf;
        cf.setProperty(tp::InlineObject, std::get<2>(c));
        cur.mergeCharFormat(cf);
    }
}

void Paginator::resolveCellImages(MaterializedPage *page, Document *d)
{
    for (const auto &fragment : page->tables)
        fragment->forTexts([&](const StoryLayout &text) {
            for (const StoryLayout::InlineObject &ob : text.inlineObjects()) {
                if (!ob.json.contains(kFlowImagePrefix) || page->cellImages.size() >= 1000) continue;
                const QString ref = QJsonDocument::fromJson(ob.json.toUtf8()).object().value(QStringLiteral("image")).toString();
                if (!ref.startsWith(kFlowImagePrefix) || page->cellImages.contains(ref)) continue;
                const QString id = imageIdOf(d, ref.mid(kFlowImagePrefix.size()).toInt());
                if (!id.isEmpty()) page->cellImages.insert(ref, id);
            }
        });
}

QRectF Paginator::placeOf(const FloatingObject &o, const Anchor &a, const Section &sec) const
{
    using Rel = FloatingObject::Rel;
    using Align = FloatingObject::Align;
    const QSizeF pageSize = sec.pageSize();
    const QRectF page(0, 0, pageSize.width(), pageSize.height());
    const QRectF margin(sec.margins.left(), sec.margins.top(), std::max(1.0, pageSize.width() - sec.margins.left() - sec.margins.right()),
                        std::max(1.0, pageSize.height() - sec.margins.top() - sec.margins.bottom()));
    const QSizeF size = o.item->rect.size();
    const QRectF column(a.columnLeft, margin.top(), std::max(1.0, sec.columnWidth()), margin.height());
    // Across: from the page's edge, the margin's, a strip of the page beside the margin (the strips are as wide as the margins), the column's, or the anchor's place; or
    // put at the start, the middle, or the end of one of them.
    const QRectF leftStrip(0, 0, sec.margins.left(), pageSize.height()), rightStrip(pageSize.width() - sec.margins.right(), 0, sec.margins.right(), pageSize.height());
    const QRectF across = o.hRel == Rel::Page ? page : o.hRel == Rel::Margin ? margin : o.hRel == Rel::LeftMargin || o.hRel == Rel::InsideMargin ? leftStrip
                        : o.hRel == Rel::RightMargin || o.hRel == Rel::OutsideMargin ? rightStrip : column;
    double x = o.hRel == Rel::Character ? a.x + o.dx : across.left() + o.dx;
    if (o.hAlign == Align::Start) x = across.left();
    else if (o.hAlign == Align::Center) x = across.left() + (across.width() - size.width()) / 2;
    else if (o.hAlign == Align::End) x = across.right() - size.width();
    // Down: the same, from the paragraph's top or the line's (the places the text would set them without the objects it goes round).
    const QRectF topStrip(0, 0, pageSize.width(), sec.margins.top()), footStrip(0, pageSize.height() - sec.margins.bottom(), pageSize.width(), sec.margins.bottom());
    const bool fromArea = o.vRel == Rel::Page || o.vRel == Rel::Margin || o.vRel == Rel::TopMargin || o.vRel == Rel::BottomMargin || o.vRel == Rel::InsideMargin || o.vRel == Rel::OutsideMargin;
    const QRectF down = o.vRel == Rel::Page ? page : o.vRel == Rel::Margin ? margin : o.vRel == Rel::TopMargin || o.vRel == Rel::InsideMargin ? topStrip : footStrip;
    double y = fromArea ? down.top() + o.dy : o.vRel == Rel::Line ? a.lineTop + o.dy : a.paragraphTop + o.dy;
    if (o.vAlign != Align::None && fromArea) {
        y = o.vAlign == Align::Start ? down.top() : o.vAlign == Align::Center ? down.top() + (down.height() - size.height()) / 2 : down.bottom() - size.height();
    } else if (o.vAlign == Align::End) {
        y = (o.vRel == Rel::Line ? a.lineTop : a.paragraphTop) - size.height();
    }
    // An object set from the text stays on the page: moved up from the foot where it would go over it (and down from the top).
    if (!fromArea) y = std::max(0.0, std::min(y, pageSize.height() - size.height()));
    return QRectF(QPointF(x, y), size);
}

QVector<QPolygonF> Paginator::obstaclesFor(const QVector<Wrapped> &wrapped, const QRectF &column) const
{
    QVector<QPolygonF> out;
    for (const Wrapped &w : wrapped) {
        const FloatingObject &o = m_doc->objects[w.object - 1];
        // A text box that does not reach into the column's text pushes none of it (by more than a hair: boxes are set edge to edge).
        if (o.item->type() == ItemType::Text && !w.rect.adjusted(0.55, 0.55, -0.55, -0.55).intersects(column)) continue;
        QRectF b = w.rect.adjusted(-o.gap.left(), -o.gap.top(), o.gap.right(), o.gap.bottom());
        if (!b.intersects(column)) continue;
        if (o.wrap == FloatingObject::Wrap::TopBottom) {
            b.setLeft(-1e5);
            b.setRight(1e5);
        } else if (o.side != FloatingObject::Side::Both) {
            // The side asked for is the room beside it: the other side is a wall as well.
            const bool leftOnly = o.side == FloatingObject::Side::Left || (o.side == FloatingObject::Side::Largest && b.left() - column.left() >= column.right() - b.right());
            if (leftOnly) b.setRight(b.left() + 1e5);
            else b.setLeft(b.right() - 1e5);
        }
        out << QPolygonF(b.translated(-column.left(), -column.top()));
    }
    return out;
}

void Paginator::addObjects(Document *d, const QVector<Anchor> &anchors, const Section &sec, int number, const QVector<Wrapped> &wrapped, const QVector<QRectF> &columns)
{
    int made = 0;
    for (const Anchor &a : anchors) {
        for (int id = a.object; id > 0 && id <= m_doc->objects.size() && made < kMostObjects; id = m_doc->objects[id - 1].next, ++made) {
            const FloatingObject &o = m_doc->objects[id - 1];
            if (!o.item) continue;
            using Rel = FloatingObject::Rel;
            // The object the text of the page was laid out round is where it was laid out round it (and where it would be set from the lines as they are: the same,
            // where the page was laid out until they agreed).
            const Wrapped *wall = nullptr;
            for (const Wrapped &w : wrapped)
                if (w.object == id && w.at == a.pos) wall = &w;
            const QSizeF size = o.item->rect.size();
            QRectF placed = placeOf(o, a, sec);
            if (wall) placed = o.item->rotation != 0 ? QRectF(wall->rect.center() - QPointF(size.width() / 2, size.height() / 2), size) : wall->rect;
            ItemPtr item = o.item->clone();
            item->id = QStringLiteral("flow-object-%1-%2-%3").arg(d->pages[0]->id).arg(id).arg(made);
            if (item->type() == ItemType::Line) {
                // A line's ends are from the place it is measured from.
                const QRectF page(0, 0, sec.pageSize().width(), sec.pageSize().height());
                const QRectF margin(sec.margins.left(), sec.margins.top(), std::max(1.0, page.width() - sec.margins.left() - sec.margins.right()),
                                    std::max(1.0, page.height() - sec.margins.top() - sec.margins.bottom()));
                const QRectF column(a.columnLeft, margin.top(), std::max(1.0, sec.columnWidth()), margin.height());
                const QRectF across = o.hRel == Rel::Page ? page : o.hRel == Rel::Margin ? margin : column;
                auto *line = static_cast<LineItem *>(item.get());
                const double fromX = (o.hRel == Rel::Character ? a.x : across.left()) + o.dx;
                const double fromY = o.vRel == Rel::Page ? o.dy : o.vRel == Rel::Margin ? margin.top() + o.dy : o.vRel == Rel::Line ? a.lineTop + o.dy : a.paragraphTop + o.dy;
                line->p1 += QPointF(fromX, fromY);
                line->p2 += QPointF(fromX, fromY);
                line->syncRect();
            } else {
                item->rect = placed;
            }
            if (item->type() == ItemType::Picture) {
                auto *pic = static_cast<PictureItem *>(item.get());
                pic->imageId = imageIdOf(d, o.image);
                pic->imgRect = QRectF(QPointF(0, 0), size);
                if (pic->imageId.isEmpty()) continue;
            }
            if (item->type() == ItemType::Text && o.story) {
                auto *box = static_cast<TextItem *>(item.get());
                auto s = std::make_shared<Story>();
                s->id = QStringLiteral("flow-boxtext-%1-%2-%3").arg(d->pages[0]->id).arg(id).arg(made);
                s->doc = cloneStory(o.story.get());
                resolveImages(s->doc.get(), d);
                d->stories.insert(s->id, s);
                box->storyId = s->id;
                box->insets = o.insets;
                box->columns = 1;
                box->hyphenate = false;
            }
            // The text goes round the objects the page was laid out with as walls (and only those: the pages' lines, and so the page layer's, are of them).
            item->wrap.mode = wall ? Wrap::Square : Wrap::None;
            item->wrap.side = o.side == FloatingObject::Side::Left ? Wrap::LeftOnly : o.side == FloatingObject::Side::Right ? Wrap::RightOnly : o.side == FloatingObject::Side::Largest ? Wrap::Largest : Wrap::Both;
            item->wrap.left = o.gap.left();
            item->wrap.top = o.gap.top();
            item->wrap.right = o.gap.right();
            item->wrap.bottom = o.gap.bottom();
            if (wall && o.wrap == FloatingObject::Wrap::TopBottom) {
                // No text beside it: the room round it reaches across the columns it is in (a square wall that wide, for the page layer's wall of this kind is as wide as the page).
                const QRectF room = wall->rect.adjusted(-o.gap.left(), -o.gap.top(), o.gap.right(), o.gap.bottom());
                double lo = room.left(), hi = room.right();
                for (const QRectF &col : columns)
                    if (room.intersects(col)) {
                        lo = std::min(lo, col.left());
                        hi = std::max(hi, col.right());
                    }
                item->wrap.side = Wrap::Both;
                item->wrap.left = wall->rect.left() - lo + 0.5;
                item->wrap.right = hi - wall->rect.right() + 0.5;
            }
            if (o.behind) d->pages[0]->items.insert(d->pages[0]->items.begin(), item);
            else d->pages[0]->items.push_back(item);
        }
    }
    Q_UNUSED(number);
}

} // namespace jp::flow
