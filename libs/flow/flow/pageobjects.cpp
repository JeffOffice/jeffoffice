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

namespace {

// The most objects one page is given (a file with more of them has the rest left off its pages).
constexpr int kMostObjects = 500;

// The prefix of the picture a story's inline object names, until the page changes it to the Id of the picture in its own document.
const QLatin1String kImagePrefix("flow-img:");

} // namespace

QVector<Paginator::Anchor> Paginator::anchorsIn(const QTextDocument *story, int from, int to, const QVector<AnchorLine> &lines) const
{
    QVector<Anchor> out;
    if (m_doc->objects.isEmpty()) return out;
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to && out.size() < kMostObjects; b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const int id = frag.charFormat().intProperty(fp::AnchorId);
            if (id <= 0 || id > m_doc->objects.size() || frag.position() >= to || frag.position() + frag.length() <= from) continue;
            for (int k = std::max(frag.position(), from); k < std::min(frag.position() + frag.length(), to); ++k) {
                Anchor a;
                a.object = id;
                bool paragraphSeen = false, found = false;
                for (const AnchorLine &l : lines) {
                    if (l.block != b.position()) continue;
                    if (!paragraphSeen) {
                        a.paragraphTop = l.rect.top();
                        a.columnLeft = l.columnLeft;
                        a.lineTop = l.rect.top();
                        a.x = l.rect.left();
                        paragraphSeen = true;
                    }
                    if (!found && k >= l.pos && k < l.end) {
                        a.lineTop = l.rect.top();
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
            if (json.isEmpty() || !json.contains(kImagePrefix)) continue;
            QJsonObject o = QJsonDocument::fromJson(json.toUtf8()).object();
            const QString ref = o.value(QStringLiteral("image")).toString();
            if (!ref.startsWith(kImagePrefix)) continue;
            o.insert(QStringLiteral("image"), imageIdOf(d, ref.mid(kImagePrefix.size()).toInt()));
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

void Paginator::addObjects(Document *d, const QVector<Anchor> &anchors, const Section &sec, int number)
{
    const QSizeF pageSize = sec.pageSize();
    const QRectF page(0, 0, pageSize.width(), pageSize.height());
    const QRectF margin(sec.margins.left(), sec.margins.top(), std::max(1.0, pageSize.width() - sec.margins.left() - sec.margins.right()),
                        std::max(1.0, pageSize.height() - sec.margins.top() - sec.margins.bottom()));
    int made = 0;
    for (const Anchor &a : anchors) {
        for (int id = a.object; id > 0 && id <= m_doc->objects.size() && made < kMostObjects; id = m_doc->objects[id - 1].next, ++made) {
            const FloatingObject &o = m_doc->objects[id - 1];
            if (!o.item) continue;
            using Rel = FloatingObject::Rel;
            using Align = FloatingObject::Align;
            const QSizeF size = o.item->rect.size();
            const QRectF column(a.columnLeft, margin.top(), std::max(1.0, sec.columnWidth()), margin.height());
            // Across: from the page's edge, the margin's, the column's, or the anchor's place; or put at the start, the middle, or the end of one of them.
            const QRectF across = o.hRel == Rel::Page ? page : o.hRel == Rel::Margin ? margin : column;
            double x = o.hRel == Rel::Character ? a.x + o.dx : across.left() + o.dx;
            if (o.hAlign == Align::Start) x = across.left();
            else if (o.hAlign == Align::Center) x = across.left() + (across.width() - size.width()) / 2;
            else if (o.hAlign == Align::End) x = across.right() - size.width();
            // Down: the same, from the paragraph's top or the line's.
            double y = o.vRel == Rel::Page ? o.dy : o.vRel == Rel::Margin ? margin.top() + o.dy : o.vRel == Rel::Line ? a.lineTop + o.dy : a.paragraphTop + o.dy;
            if (o.vAlign != Align::None && (o.vRel == Rel::Page || o.vRel == Rel::Margin)) {
                const QRectF down = o.vRel == Rel::Page ? page : margin;
                y = o.vAlign == Align::Start ? down.top() : o.vAlign == Align::Center ? down.top() + (down.height() - size.height()) / 2 : down.bottom() - size.height();
            } else if (o.vAlign == Align::End) {
                y = (o.vRel == Rel::Line ? a.lineTop : a.paragraphTop) - size.height();
            }
            ItemPtr item = o.item->clone();
            item->id = QStringLiteral("flow-object-%1-%2-%3").arg(d->pages[0]->id).arg(id).arg(made);
            if (item->type() == ItemType::Line) {
                // A line's ends are from the place it is measured from.
                auto *line = static_cast<LineItem *>(item.get());
                const double fromX = (o.hRel == Rel::Character ? a.x : across.left()) + o.dx;
                const double fromY = o.vRel == Rel::Page ? o.dy : o.vRel == Rel::Margin ? margin.top() + o.dy : o.vRel == Rel::Line ? a.lineTop + o.dy : a.paragraphTop + o.dy;
                line->p1 += QPointF(fromX, fromY);
                line->p2 += QPointF(fromX, fromY);
                line->syncRect();
            } else {
                item->rect = QRectF(QPointF(x, y), size);
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
                s->doc.reset(o.story->clone());
                resolveImages(s->doc.get(), d);
                d->stories.insert(s->id, s);
                box->storyId = s->id;
                box->insets = o.insets;
                box->columns = 1;
                box->hyphenate = false;
            }
            item->wrap.mode = m_wrapObjects ? (o.wrap == FloatingObject::Wrap::Square ? Wrap::Square : o.wrap == FloatingObject::Wrap::Tight ? Wrap::Tight
                              : o.wrap == FloatingObject::Wrap::TopBottom ? Wrap::TopBottom : Wrap::None) : Wrap::None;
            item->wrap.left = o.gap.left();
            item->wrap.top = o.gap.top();
            item->wrap.right = o.gap.right();
            item->wrap.bottom = o.gap.bottom();
            if (o.behind) d->pages[0]->items.insert(d->pages[0]->items.begin(), item);
            else d->pages[0]->items.push_back(item);
        }
    }
    Q_UNUSED(number);
}

} // namespace jp::flow
