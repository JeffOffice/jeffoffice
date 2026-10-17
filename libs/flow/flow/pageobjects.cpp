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

const QVector<std::pair<int, int>> &Paginator::anchorCharsOf(const QTextBlock &b)
{
    // (Kept for the paragraphs of the document's story, by place in it; the header's and the footer's, and any other story's, are found each time they are asked for.)
    const bool mine = b.document() == m_doc->story();
    static thread_local QVector<std::pair<int, int>> loose;
    if (mine) {
        const auto found = m_anchorChars.constFind(b.position());
        if (found != m_anchorChars.constEnd()) return found.value();
    }
    // The characters that anchor an object, in order, found once for the paragraph (not for each of its lines, which would look at all of its runs again): no more than a page
    // is given.
    QVector<std::pair<int, int>> chars;
    for (auto it = b.begin(); !it.atEnd() && chars.size() < kMostObjects; ++it) {
        const QTextFragment frag = it.fragment();
        const int id = frag.charFormat().intProperty(fp::AnchorId);
        if (id <= 0 || id > m_doc->objects.size()) continue;
        for (int k = frag.position(); k < frag.position() + frag.length() && chars.size() < kMostObjects; ++k) chars.push_back({k, id});
    }
    if (!mine) {
        loose = chars;
        return loose;
    }
    if (m_anchorChars.size() >= 4096) m_anchorChars.clear();
    return m_anchorChars.insert(b.position(), chars).value();
}

QVector<Paginator::Anchor> Paginator::anchorsIn(const QTextDocument *story, int from, int to, const QVector<AnchorLine> &lines, const QHash<int, QVector<int>> *byBlock)
{
    QVector<Anchor> out;
    if (m_doc->objects.isEmpty()) return out;
    // The lines by the paragraph they are in (found once for a page, not for each anchor).
    QHash<int, QVector<int>> own;
    if (!byBlock) {
        for (int i = 0; i < lines.size(); ++i) own[lines[i].block] << i;
        byBlock = &own;
    }
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to && out.size() < kMostObjects; b = b.next()) {
        const auto held = byBlock->constFind(b.position());
        if (held == byBlock->constEnd() || held.value().isEmpty()) continue;
        const QVector<std::pair<int, int>> &chars = anchorCharsOf(b);
        auto at = std::lower_bound(chars.cbegin(), chars.cend(), from, [](const std::pair<int, int> &c, int position) { return c.first < position; });
        if (at == chars.cend() || at->first >= to) continue;
        const QVector<int> &mine = held.value();
        const AnchorLine &top = lines[mine.first()];
        for (; at != chars.cend() && at->first < to && out.size() < kMostObjects; ++at) {
            Anchor a;
            a.object = at->second;
            a.pos = at->first;
            a.paragraph = b.position();
            a.paragraphTop = top.rect.top() - top.pushed;
            a.columnLeft = top.columnLeft;
            a.lineTop = top.rect.top() - top.pushed;
            a.x = top.rect.left();
            // The first line that holds the character (the lines of a paragraph are in order: the last that begins at or before it, or the one before that if it holds it too).
            int line = int(std::upper_bound(mine.cbegin(), mine.cend(), a.pos, [&](int position, int i) { return position < lines[i].pos; }) - mine.cbegin()) - 1;
            while (line > 0 && a.pos >= lines[mine[line - 1]].pos && a.pos < lines[mine[line - 1]].upTo()) --line;
            if (line >= 0 && a.pos < lines[mine[line]].upTo()) {
                const AnchorLine &l = lines[mine[line]];
                a.lineTop = l.rect.top() - l.pushed;
                a.x = l.rect.left();
                a.columnLeft = l.columnLeft;
            }
            out << a;
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

QRectF Paginator::placeOf(const FloatingObject &o, const Anchor &a, const Section &sec, bool clamp, double limit) const
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
    if (!fromArea && a.table >= 0 && limit >= 0 && y + size.height() > limit) {
        const double top = o.vRel == Rel::Line ? a.lineTop : a.paragraphTop;
        if (limit - size.height() >= top - 0.01) y = limit - size.height();
    }
    if (!fromArea && clamp) y = std::max(0.0, std::min(y, pageSize.height() - size.height()));
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

void Paginator::addObjects(Document *d, const QVector<Anchor> &anchors, const Section &sec, int number, const QVector<Wrapped> &wrapped, const QVector<QRectF> &columns, MaterializedPage *page, size_t front)
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
                if (page && a.table < 0 && a.frameTo > a.frameFrom) page->frames.push_back({a.frameFrom, a.frameTo, item->id});
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
            // The text goes round the objects the page was laid out with as walls (and only those: the pages' lines, and so the page layer's, are of them). A wall is a shape of its
            // own that draws nothing, after the page's text and clipped to the text area: the header and the footer, which are drawn before the page's objects, are in the way of
            // none of them, and the objects (which wrap nothing themselves) are over the header and the footer as they are over the text.
            item->wrap.mode = Wrap::None;
            if (wall) {
                const Wrap::Side side = o.side == FloatingObject::Side::Left ? Wrap::LeftOnly : o.side == FloatingObject::Side::Right ? Wrap::RightOnly : o.side == FloatingObject::Side::Largest ? Wrap::Largest : Wrap::Both;
                QRectF room = wall->rect.adjusted(-o.gap.left(), -o.gap.top(), o.gap.right(), o.gap.bottom());
                if (o.wrap == FloatingObject::Wrap::TopBottom) {
                    // No text beside it: the room round it reaches across the columns it is in (a square wall that wide, for the page layer's wall of this kind is as wide as the page),
                    // to the columns' edges and no further (a wall that reached 0.5 point into a column that the room touches only, as the paginator's does not, would make its
                    // lines narrower).
                    double lo = room.left(), hi = room.right();
                    for (const QRectF &col : columns)
                        if (room.intersects(col)) {
                            lo = std::min(lo, col.left());
                            hi = std::max(hi, col.right());
                        }
                    room.setLeft(lo);
                    room.setRight(hi);
                }
                double top = room.top(), bottom = room.bottom();
                if (!columns.isEmpty()) {
                    double bodyTop = columns.first().top(), bodyBottom = columns.first().bottom();
                    for (const QRectF &col : columns) {
                        bodyTop = std::min(bodyTop, col.top());
                        bodyBottom = std::max(bodyBottom, col.bottom());
                    }
                    top = std::max(top, bodyTop);
                    bottom = std::min(bottom, bodyBottom);
                }
                if (bottom > top) {
                    auto shape = std::make_shared<ShapeItem>();
                    shape->id = QStringLiteral("flow-wall-%1-%2-%3").arg(d->pages[0]->id).arg(id).arg(made);
                    shape->rect = QRectF(QPointF(room.left(), top), QPointF(room.right(), bottom));
                    shape->fill = Fill::none();
                    shape->stroke = Stroke::none();
                    shape->wrap.mode = Wrap::Square;
                    shape->wrap.side = o.wrap == FloatingObject::Wrap::TopBottom ? Wrap::Both : side;
                    shape->wrap.left = shape->wrap.top = shape->wrap.right = shape->wrap.bottom = 0;
                    made_.push_back({o.z, shape, false});
                }
            }
            made_.push_back({o.z, item, o.behind});
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
        if (m.behind) items.insert(items.begin() + std::min(front, items.size()) + behindCount++, m.item);
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
    const auto cacheKey = std::make_tuple(key, from, to, int(std::lround(columnWidth * 100)), uint(qHash(m_doc->env.key())));
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

// Whether a paragraph has no lines: its mark and all of its text are hidden.
static bool hasNoLines(const QTextBlock &b)
{
    if (!b.charFormat().boolProperty(tp::HiddenText)) return false;
    for (auto it = b.begin(); !it.atEnd(); ++it)
        if (!it.fragment().charFormat().boolProperty(tp::HiddenText)) return false;
    return true;
}

const Paginator::FloatIndex &Paginator::floatIndex()
{
    if (m_floats.made) return m_floats;
    FloatIndex ix;
    ix.made = true;
    const QTextDocument *story = m_doc->story();
    const TableIndex *tables = m_tables->any() ? &m_tables->index() : nullptr;
    bool placed = false;
    if (tables)
        for (const TableIndex::Span &sp : tables->spans()) placed |= sp.table && sp.table->place.has_value();
    ix.anyTable = placed;
    if (m_doc->frames.isEmpty() && !placed) {
        m_floats = std::move(ix);
        return m_floats;
    }
    auto frameKeyOf = [&](const QTextBlockFormat &bf) {
        // (A paragraph set out of the text that is no paragraph of a frame that is made is in the text: nothing of a file is left off its pages.)
        const int key = bf.intProperty(fp::Frame);
        if (key <= 0 || key > m_doc->frames.size() || !bf.boolProperty(tp::OutOfLayout) || bf.intProperty(fp::Cell)) return 0;
        const int id = m_doc->frames[key - 1];
        return id > 0 && id <= m_doc->objects.size() ? key : 0;
    };
    QVector<int> pending;   // the refs since the last paragraph in the text
    QVector<int> strays;    // the paragraphs set out of the text that are no paragraphs of a frame that is made: lines of the text
    int section = 0;        // the section the paragraph being looked at is in (a paragraph that ends a section is the last of it)
    auto reach = [&](int anchor) {
        if (pending.isEmpty()) return;
        for (const int i : pending) ix.refs[size_t(i)].section = section;
        // The nearest to the paragraph are set from it; the others stay in the text.
        const int skip = std::max(0, int(pending.size()) - kMostFloatsAtAParagraph);
        QVector<int> mine;
        for (int i = skip; i < pending.size(); ++i) {
            ix.refs[size_t(pending[i])].anchor = anchor;
            mine << pending[i];
        }
        ix.at.insert(anchor, mine);
        pending.clear();
    };
    auto ends = [](const QTextBlock &x) { return x.blockFormat().boolProperty(fp::SectionBreakAfter) ? 1 : 0; };
    for (QTextBlock b = story->begin(); b.isValid();) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.intProperty(fp::Cell)) {
            const int span = tables ? tables->spanAt(b.position()) : -1;
            if (span < 0) {
                section += ends(b);
                b = b.next();
                continue;
            }
            const TableIndex::Span &sp = tables->spans()[size_t(span)];
            if (sp.table && sp.table->place.has_value()) {
                FloatRef r;
                r.from = sp.from;
                r.to = sp.to;
                r.span = span;
                ix.tableRef.insert(span, int(ix.refs.size()));
                pending << int(ix.refs.size());
                ix.refs.push_back(r);
            } else {
                // A table in the text: the first of its rows is where those before it are set from.
                reach(sp.from);
            }
            for (QTextBlock t = b; t.isValid() && t.position() < sp.to; t = t.next()) section += ends(t);
            b = story->findBlock(sp.to);
            if (b.position() < sp.to) b = b.next();   // (the end of the story: no block)
            continue;
        }
        if (const int key = frameKeyOf(bf)) {
            // The frame: the paragraphs next to one another with its key.
            QTextBlock e = b;
            int paragraphs = 1;
            while (e.next().isValid() && frameKeyOf(e.next().blockFormat()) == key) {
                e = e.next();
                ++paragraphs;
            }
            FloatRef r;
            r.from = b.position();
            r.to = e.position() + e.length();
            r.key = key;
            if (paragraphs > kMostFrameParagraphs) {
                // (More than a frame has: paragraphs of the text, which the ones before are set from.)
                reach(r.from);
            } else {
                pending << int(ix.refs.size());
            }
            ix.refs.push_back(r);
            for (QTextBlock t = b; t.isValid() && t.position() <= e.position(); t = t.next()) section += ends(t);
            b = e.next();
            continue;
        }
        if (bf.boolProperty(tp::OutOfLayout)) strays << b.position();
        if (!hasNoLines(b)) reach(b.position());
        section += ends(b);
        b = b.next();
    }
    // The paragraphs of frames that are not set from a paragraph are laid out as the text's own (and so are the strays).
    for (const FloatRef &r : ix.refs) {
        if (r.key <= 0 || r.anchor >= 0) continue;
        for (QTextBlock b = story->findBlock(r.from); b.isValid() && b.position() < r.to; b = b.next()) ix.inText << b.position();
    }
    if (!strays.isEmpty()) {
        ix.inText += strays;
        std::sort(ix.inText.begin(), ix.inText.end());
        ix.inText.erase(std::unique(ix.inText.begin(), ix.inText.end()), ix.inText.end());
    }
    m_floats = std::move(ix);
    return m_floats;
}

int Paginator::anchorOfPosition(int position)
{
    if (m_doc->frames.isEmpty() && !m_tables->any()) return position;
    const FloatIndex &ix = floatIndex();
    if (ix.refs.empty()) return position;
    const auto it = std::upper_bound(ix.refs.begin(), ix.refs.end(), position, [](int pos, const FloatRef &r) { return pos < r.from; });
    if (it == ix.refs.begin()) return position;
    const FloatRef &r = *(it - 1);
    if (position >= r.to || r.anchor < 0) return position;
    if (r.key == 0 && !floatsTable(r.span)) return position;
    return r.anchor;
}

QVector<Paginator::Anchor> Paginator::floatAnchors(const QVector<AnchorLine> &lines, double columnWidth)
{
    QVector<Anchor> out;
    if (m_doc->frames.isEmpty() && !m_tables->any()) return out;
    const FloatIndex &ix = floatIndex();
    if (ix.at.isEmpty()) return out;
    const QTextDocument *story = m_doc->story();
    for (const AnchorLine &l : lines) {
        if (!l.first) continue;
        const auto set = ix.at.constFind(l.block);
        if (set == ix.at.constEnd()) continue;
        for (const int index : set.value()) {
            const FloatRef &r = ix.refs[size_t(index)];
            Anchor a;
            a.pos = r.from;
            a.columnLeft = l.columnLeft;
            a.paragraphTop = a.lineTop = l.rect.top() - l.pushed;
            a.x = l.rect.left();
            a.frameFrom = r.from;
            a.frameTo = r.to;
            a.paragraph = l.block;
            if (r.key > 0) {
                a.frame = frameObject(r.key, r.from, r.to, columnWidth);
                if (!a.frame) continue;
                a.object = m_doc->frames[r.key - 1];
            } else {
                a.frame = tableObject(r.span, columnWidth);
                if (!a.frame) continue;
                a.object = 1;   // (not the document's own: `frame` is the object)
                a.table = r.span;
            }
            out << a;
            if (r.key <= 0 || m_doc->objects.isEmpty()) continue;
            // The objects anchored in a frame's text are set where the frame is (it has no lines of its own to set them from).
            for (QTextBlock b = story->findBlock(r.from); b.isValid() && b.position() < r.to && out.size() < kMostObjects; b = b.next())
                for (auto it = b.begin(); !it.atEnd() && out.size() < kMostObjects; ++it) {
                    const QTextFragment frag = it.fragment();
                    const int id = frag.charFormat().intProperty(fp::AnchorId);
                    if (id <= 0 || id > m_doc->objects.size()) continue;
                    for (int k = frag.position(); k < frag.position() + frag.length() && out.size() < kMostObjects; ++k) {
                        Anchor o = a;
                        o.object = id;
                        o.pos = k;
                        o.frame.reset();
                        o.frameFrom = o.frameTo = 0;
                        o.table = -1;
                        out << o;
                    }
                }
        }
    }
    return out;
}

QVector<Paginator::Anchor> Paginator::anchorsOfLines(const QVector<AnchorLine> &held, const QVector<AnchorLine> &firstLines, double columnWidth)
{
    QVector<Anchor> anchors;
    QHash<int, QVector<int>> byBlock;
    for (int i = 0; i < held.size(); ++i) byBlock[held[i].block] << i;
    for (const AnchorLine &l : held) anchors << anchorsIn(m_doc->story(), l.pos, l.upTo(), held, &byBlock);
    anchors += floatAnchors(firstLines, columnWidth);
    return anchors;
}

bool Paginator::hasFloaters()
{
    if (!m_doc->objects.isEmpty()) return true;
    if (!m_tables->any()) return false;
    return floatIndex().anyTable;
}

bool Paginator::floatsTable(int span)
{
    const TableIndex &ix = m_tables->index();
    if (span < 0 || size_t(span) >= ix.spans().size()) return false;
    const TableIndex::Span &sp = ix.spans()[size_t(span)];
    if (!sp.table || !sp.table->place) return false;
    // (One with no paragraph to be set from is in the text.)
    const FloatIndex &fx = floatIndex();
    const auto ref = fx.tableRef.constFind(span);
    if (ref == fx.tableRef.constEnd() || fx.refs[size_t(ref.value())].anchor < 0) return false;
    const std::shared_ptr<TableLayout> table = m_tables->layout(span);
    if (!table || table->rowCount() < 1 || table->rowCount() > kMostFloatingRows) return false;
    double height = 0;
    for (int r = 0; r < table->rowCount(); ++r) height += table->rowHeight(r);
    // (One that is taller than a page's text area, or wider than a page, is in the text: it could not be set at a place that the page has room for. The pages are those of the
    // section the paragraph it is set from is in.)
    const Section &sec = m_doc->section(fx.refs[size_t(ref.value())].section);
    return height <= sec.content().height() * 0.9 && table->width() <= sec.content().width();
}

std::shared_ptr<const FloatingObject> Paginator::tableObject(int span, double columnWidth)
{
    const TableIndex &ix = m_tables->index();
    if (!floatsTable(span)) return nullptr;
    const TableIndex::Span &sp = ix.spans()[size_t(span)];
    const auto cacheKey = std::make_tuple(sp.from, sp.to, int(std::lround(columnWidth * 100)), uint(qHash(m_doc->env.key())));
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

} // namespace jp::flow
