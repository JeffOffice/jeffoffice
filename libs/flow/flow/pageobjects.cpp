// The objects set at places on a page: pictures, rules, boxes, and text boxes anchored to a paragraph, the margin, or the page. (The paginator's functions for
// them are here, apart from its layout of the text.)

#include "flow/paginator.h"

#include <QJsonDocument>
#include <QPainterPath>
#include <QJsonObject>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>
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
    AnchorLine l{ln.block, ln.pos, ln.end, rect, columnLeft, ln.pushed, ln.line == 0, ln.end};
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
        const FloatingObject &o = w.frame ? *w.frame : m_doc->objects[w.object - 1];
        // A text box that does not reach into the column's text pushes none of it (by more than a hair: boxes are set edge to edge).
        if (o.item->type() == ItemType::Text && !w.rect.adjusted(0.55, 0.55, -0.55, -0.55).intersects(column)) continue;
        QRectF b = w.rect.adjusted(-o.gap.left(), -o.gap.top(), o.gap.right(), o.gap.bottom());
        if (!b.intersects(column)) continue;
        if (o.wrap == FloatingObject::Wrap::TopBottom) {
            b.setLeft(-1e5);
            b.setRight(1e5);
        } else if (o.side != FloatingObject::Side::Both) {
            // The side asked for is the room beside it: the other side is a wall as well. (The page layer makes the wall of a path and joins the two, as here, so that the
            // edges of the polygons are the same numbers: a line that touches one has the page layer put it where the paginator did.)
            const bool leftOnly = o.side == FloatingObject::Side::Left || (o.side == FloatingObject::Side::Largest && b.left() - column.left() >= column.right() - b.right());
            QPainterPath path;
            path.addRect(b);
            QPainterPath beyond;
            if (leftOnly) beyond.addRect(QRectF(b.left(), b.top(), 1e5, b.height()));
            else beyond.addRect(QRectF(-1e5, b.top(), b.right() + 1e5, b.height()));
            path = path.united(beyond);
            out << path.toFillPolygon().translated(-column.left(), -column.top());
            continue;
        }
        out << QPolygonF(b.translated(-column.left(), -column.top()));
    }
    return out;
}

void Paginator::addObjects(Document *d, const QVector<Anchor> &anchors, const Section &sec, int number, const QVector<Wrapped> &wrapped, const QVector<QRectF> &columns, MaterializedPage *page)
{
    // The objects are drawn in the order of their heights (the file's: z-index, relative height), and in the order of their anchors where those are the same: those behind the
    // text before the text, the rest after it.
    struct Made {
        int z = 0;
        ItemPtr item;
        bool behind = false;
    };
    QVector<Made> made_;
    int made = 0;
    for (const Anchor &a : anchors) {
        for (int id = a.object; (a.frame || (id > 0 && id <= m_doc->objects.size())) && id > 0 && made < kMostObjects; id = objectAt(a, id).next, ++made) {
            const FloatingObject &o = objectAt(a, id);
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
            if (wall && item->type() == ItemType::Text) {
                // A text box does not push another box's text aside unless its own frame reaches into that text (the page layer's rule, for boxes that are set edge to edge), and the
                // room round it does not count: a wall that is a text box's room alone would be one for the paginator and none for the page layer. So the box wraps nothing, and an
                // invisible wall of its rectangle (and its room) is what the text goes round.
                auto shape = std::make_shared<ShapeItem>();
                shape->id = QStringLiteral("flow-wall-%1-%2-%3").arg(d->pages[0]->id).arg(id).arg(made);
                shape->rect = item->bounds();
                shape->fill = Fill::none();
                shape->stroke = Stroke::none();
                shape->wrap = item->wrap;
                item->wrap.mode = Wrap::None;
                made_.push_back({o.z, item, o.behind});
                made_.push_back({o.z, shape, false});
            } else {
                made_.push_back({o.z, item, o.behind});
            }
            // A table set at a place is drawn there: its rows, in a fragment that the page paints with the others (the object is the room it takes, which draws nothing).
            if (a.table >= 0 && page) {
                const std::shared_ptr<TableLayout> table = m_tables->layout(a.table);
                if (table && table->rowCount() > 0) {
                    if (const std::shared_ptr<TableFragment> fragment = m_tables->fragment(table, 0, table->rowCount(), 1e9)) {
                        fragment->origin = placed.topLeft();
                        fragment->floating = true;
                        page->tables.push_back(fragment);
                    }
                }
            }
        }
    }
    std::stable_sort(made_.begin(), made_.end(), [](const Made &a, const Made &b) { return a.z < b.z; });
    auto &items = d->pages[0]->items;
    size_t behindCount = 0;
    for (const Made &m : made_) {
        if (m.behind) items.insert(items.begin() + behindCount++, m.item);
        else items.push_back(m.item);
    }
    Q_UNUSED(number);
}


std::unique_ptr<QTextDocument> Paginator::frameStory(int from, int to) const
{
    const QTextDocument *main = m_doc->story();
    auto sd = std::make_unique<QTextDocument>();
    sd->setDocumentMargin(0);
    sd->setDefaultFont(main->defaultFont());
    sd->setDefaultTextOption(main->defaultTextOption());
    sd->setUndoRedoEnabled(false);
    QTextCursor dst(sd.get());
    QHash<QTextList *, QTextList *> lists;
    bool firstBlock = true;
    int paragraphs = 0;
    for (QTextBlock b = main->findBlock(from); b.isValid() && b.position() < to && ++paragraphs <= kMostFrameParagraphs; b = b.next()) {
        QTextBlockFormat bf = b.blockFormat();
        bf.clearProperty(QTextFormat::ObjectIndex);   // (the list is the document's, not this story's: it's made again below)
        bf.clearProperty(tp::OutOfLayout);
        bf.clearProperty(fp::Frame);
        bf.clearProperty(tp::WidowControl);
        bf.clearProperty(tp::KeepTogether);
        bf.clearProperty(tp::KeepWithNext);
        bf.setPageBreakPolicy(QTextFormat::PageBreak_Auto);
        QTextList *list = b.textList();
        if (firstBlock) {
            dst.setBlockFormat(bf);
            dst.setBlockCharFormat(b.charFormat());
        } else {
            dst.insertBlock(bf, b.charFormat());
        }
        firstBlock = false;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            QTextCharFormat cf = frag.charFormat();
            // (A note's reference is not copied: its text is for the page of the text.)
            if (cf.stringProperty(tp::Field).startsWith(QLatin1String("footnote:")) || cf.stringProperty(tp::Field).startsWith(QLatin1String("endnote:"))) continue;
            dst.insertText(frag.text(), cf);
        }
        if (list) {
            QTextList *made = lists.value(list);
            if (!made) {
                QTextListFormat lf = list->format();
                if (list->itemNumber(b) > 0) lf.setStart(lf.start() + list->itemNumber(b));
                made = dst.createList(lf);
                lists.insert(list, made);
            } else {
                made->add(dst.block());
            }
        }
    }
    return sd;
}

std::shared_ptr<const FloatingObject> Paginator::frameObject(int key, int from, int to, double columnWidth)
{
    if (key <= 0 || key > m_doc->frames.size()) return nullptr;
    const int id = m_doc->frames[key - 1];
    if (id <= 0 || id > m_doc->objects.size()) return nullptr;
    const QTextDocument *story = m_doc->story();
    const uint env = qHash(m_doc->env.key()) ^ uint(qHash(story->characterCount()) * 31u);
    const auto cacheKey = std::make_tuple(key, from, to, story->revision(), int(std::lround(columnWidth * 100)), env);
    const auto found = m_frames.find(cacheKey);
    if (found != m_frames.end()) return found->second;
    if (m_frames.size() > 400) m_frames.clear();
    FloatingObject o = m_doc->objects[id - 1];
    o.story = frameStory(from, to);
    // As wide as it says; else as wide as the column, or (a frame that shrinks) as the longest line of its text where nothing is wrapped (to the column's width).
    double width = o.frameWidth > 0 ? o.frameWidth : std::max(1.0, columnWidth);
    if (o.frameWidth <= 0 && o.frameShrinks) {
        width = 0;
        FrameSpec wide;
        wide.size = QSizeF(100000, 100000);
        wide.insets = QMarginsF(0, 0, 0, 0);
        wide.hyphenate = false;
        StoryLayout natural;
        natural.build(o.story.get(), {wide}, m_doc->env);
        for (const QRectF &r : natural.rangeRects(0, 0, std::max(0, o.story->characterCount() - 1))) width = std::max(width, r.right());
        width = std::clamp(width, 1.0, std::max(1.0, columnWidth));
    }
    FrameSpec fs;
    fs.size = QSizeF(width, 100000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout lay;
    lay.build(o.story.get(), {fs}, m_doc->env);
    // (As tall as its text, and the lower edge of the box of its last paragraph, borders and shading, which the text's height does not take in.)
    double height = lay.usedHeight(0) + std::max(0.0, lay.trailingRoom() - lay.trailingSpace());
    if (qEnvironmentVariableIsSet("JP_DEBUG_FRAME")) qInfo("FRAME used %.2f room %.2f space %.2f asked %.2f exact %d width %.2f", lay.usedHeight(0), lay.trailingRoom(), lay.trailingSpace(), o.frameHeight, o.frameExactHeight, width);   // DEBUGTMP
    if (o.frameExactHeight && o.frameHeight > 0) height = o.frameHeight;
    else height = std::max(height, o.frameHeight);
    std::shared_ptr<TextItem> box = o.item && o.item->type() == ItemType::Text ? std::static_pointer_cast<TextItem>(o.item->clone()) : std::make_shared<TextItem>();
    box->rect = QRectF(0, 0, width, std::max(1.0, height));
    box->insets = QMarginsF(0, 0, 0, 0);
    box->columns = 1;
    box->hyphenate = false;
    o.item = box;
    o.insets = QMarginsF(0, 0, 0, 0);
    return m_frames.emplace(cacheKey, std::make_shared<const FloatingObject>(std::move(o))).first->second;
}

QVector<Paginator::Anchor> Paginator::frameAnchors(const QVector<AnchorLine> &lines, double columnWidth)
{
    QVector<Anchor> out;
    if (m_doc->frames.isEmpty()) return out;
    const QTextDocument *story = m_doc->story();
    for (const AnchorLine &l : lines) {
        if (!l.first) continue;
        // The frames before the paragraph (a frame is the paragraphs next to one another that have its key), the nearest first.
        QVector<Anchor> found;
        QTextBlock b = story->findBlock(l.block).previous();
        while (b.isValid() && found.size() < 16) {
            const int key = b.blockFormat().intProperty(fp::Frame);
            if (key <= 0) break;
            QTextBlock first = b;
            for (int back = 0; back < kMostFrameParagraphs && first.previous().isValid() && first.previous().blockFormat().intProperty(fp::Frame) == key; ++back) first = first.previous();
            if (std::shared_ptr<const FloatingObject> o = frameObject(key, first.position(), b.position() + b.length(), columnWidth)) {
                Anchor a;
                a.object = m_doc->frames[key - 1];
                a.pos = first.position();
                a.columnLeft = l.columnLeft;
                a.paragraphTop = a.lineTop = l.rect.top() - l.pushed;
                a.x = l.rect.left();
                a.frameFrom = first.position();
                a.frameTo = b.position() + b.length();
                a.frame = o;
                found.prepend(a);
            }
            b = first.previous();
        }
        out += found;
    }
    return out;
}


bool Paginator::hasFloaters()
{
    if (!m_doc->objects.isEmpty()) return true;
    if (!m_tables->any()) return false;
    const QTextDocument *story = m_doc->story();
    if (m_floatCheck.revision != story->revision() || m_floatCheck.length != story->characterCount()) {
        m_floatCheck.revision = story->revision();
        m_floatCheck.length = story->characterCount();
        m_floatCheck.tables = false;
        for (const TableIndex::Span &sp : m_tables->index().spans()) m_floatCheck.tables |= sp.table && sp.table->place.has_value();   // (cheap: whether any may be)
    }
    return m_floatCheck.tables;
}

bool Paginator::floatsTable(int span)
{
    const TableIndex &ix = m_tables->index();
    if (span < 0 || size_t(span) >= ix.spans().size()) return false;
    const TableIndex::Span &sp = ix.spans()[size_t(span)];
    if (!sp.table || !sp.table->place) return false;
    const std::shared_ptr<TableLayout> table = m_tables->layout(span);
    if (!table || table->rowCount() < 1 || table->rowCount() > kMostFloatingRows) return false;
    double height = 0;
    for (int r = 0; r < table->rowCount(); ++r) height += table->rowHeight(r);
    // (One that is taller than a page's text area, or wider than a page, is in the text: it could not be set at a place that the page has room for.)
    double tallest = 0, widest = 0;
    for (const Section &sec : m_doc->sections) {
        tallest = std::max(tallest, sec.content().height());
        widest = std::max(widest, sec.content().width());
    }
    return height <= tallest * 0.9 && table->width() <= widest;
}

std::shared_ptr<const FloatingObject> Paginator::tableObject(int span, double columnWidth)
{
    const TableIndex &ix = m_tables->index();
    if (!floatsTable(span)) return nullptr;
    const TableIndex::Span &sp = ix.spans()[size_t(span)];
    const QTextDocument *story = m_doc->story();
    const uint env = qHash(m_doc->env.key()) ^ uint(qHash(story->characterCount()) * 31u);
    const auto cacheKey = std::make_tuple(sp.from, sp.to, story->revision(), int(std::lround(columnWidth * 100)), env);
    const auto found = m_tableObjects.find(cacheKey);
    if (found != m_tableObjects.end()) return found->second;
    if (m_tableObjects.size() > 400) m_tableObjects.clear();
    const std::shared_ptr<TableLayout> table = m_tables->layout(span);
    double height = 0;
    for (int r = 0; r < table->rowCount(); ++r) height += table->rowHeight(r);
    using Place = TableData::Place;
    using Rel = FloatingObject::Rel;
    using Align = FloatingObject::Align;
    const Place &p = *sp.table->place;
    FloatingObject o;
    auto shape = std::make_shared<ShapeItem>();
    shape->rect = QRectF(0, 0, table->width(), std::max(1.0, height));
    shape->fill = Fill::none();
    shape->stroke = Stroke::none();
    o.item = shape;
    o.hRel = p.across == Place::From::Margin ? Rel::Margin : p.across == Place::From::Page ? Rel::Page : Rel::Column;
    o.vRel = p.down == Place::From::Margin ? Rel::Margin : p.down == Place::From::Page ? Rel::Page : Rel::Paragraph;
    auto align = [](Place::Spec s) { return s == Place::Spec::Start ? Align::Start : s == Place::Spec::Center ? Align::Center : s == Place::Spec::End ? Align::End : Align::None; };
    o.hAlign = align(p.acrossSpec);
    o.vAlign = align(p.downSpec);
    o.dx = p.x;
    o.dy = p.y;
    o.gap = p.room;
    o.wrap = FloatingObject::Wrap::Square;
    return m_tableObjects.emplace(cacheKey, std::make_shared<const FloatingObject>(std::move(o))).first->second;
}

QVector<Paginator::Anchor> Paginator::tableAnchors(const QVector<AnchorLine> &lines, double columnWidth)
{
    QVector<Anchor> out;
    if (!m_tables->any()) return out;
    const QTextDocument *story = m_doc->story();
    const TableIndex &ix = m_tables->index();
    for (const AnchorLine &l : lines) {
        if (!l.first) continue;
        // The tables set at places that come right before the paragraph, the nearest first.
        QVector<Anchor> found;
        QTextBlock b = story->findBlock(l.block).previous();
        int ends = story->findBlock(l.block).position();   // (where the table that is next must end: at the paragraph, or at the table after it)
        while (b.isValid() && b.blockFormat().intProperty(fp::Cell) && found.size() < 8) {
            const int span = ix.spanAt(b.position());
            if (span < 0) break;
            const TableIndex::Span &sp = ix.spans()[size_t(span)];
            if (sp.to != ends) break;   // (not a table that ends there: that paragraph is not where it is set)
            ends = sp.from;
            if (std::shared_ptr<const FloatingObject> o = tableObject(span, columnWidth)) {
                Anchor a;
                a.object = 1;   // (not the document's own: `frame` is the object)
                a.pos = sp.from;
                a.columnLeft = l.columnLeft;
                a.paragraphTop = a.lineTop = l.rect.top() - l.pushed;
                a.x = l.rect.left();
                a.frameFrom = sp.from;
                a.frameTo = sp.to;
                a.frame = o;
                a.table = span;
                found.prepend(a);
            }
            b = story->findBlock(sp.from).previous();
        }
        out += found;
    }
    return out;
}

} // namespace jp::flow
