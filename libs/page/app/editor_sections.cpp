// Sections of a publication: where the page numbers begin again and change their format (Insert > Pages > Section); and the glue points of a shape (connection sites of its own).
#include "app/editor.h"

#include "office/connectionsites.h"

#include <algorithm>

namespace jp {

bool Editor::startSection(int page, int start, int format, QString *why)
{
    auto refuse = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    if (!m_master.isEmpty()) return refuse(tr("Go to a page first: a master page has no sections."));
    if (page < 0 || page >= int(m_doc->pages.size())) return refuse(tr("That page is not in the publication."));
    if (page == 0) return refuse(tr("The first page starts the first section. Use Format Page Numbers to set how it counts."));
    const int most = int(Document::pageNumberFormats().size()) - 1;
    start = std::clamp(start, 0, 1000);   // (as far as Format Page Numbers goes)
    format = std::clamp(format, -1, most);
    Page &p = *m_doc->pages[page];
    if (p.startsSection && p.sectionStart == start && p.sectionFormat == format) return refuse(tr("This page starts a section like that already."));
    beginChange(tr("Insert Section"));
    p.startsSection = true;
    p.sectionStart = start;
    p.sectionFormat = format;
    endChange();
    return true;
}

bool Editor::mergeSection(int page, QString *why)
{
    if (!m_master.isEmpty() || page < 0 || page >= int(m_doc->pages.size()) || !m_doc->pages[page]->startsSection) {
        if (why) *why = tr("This page does not start a section.");
        return false;
    }
    beginChange(tr("Merge with Previous Section"));
    Page &p = *m_doc->pages[page];
    p.startsSection = false;
    p.sectionStart = 0;
    p.sectionFormat = -1;
    endChange();
    return true;
}

bool Editor::addGluePoint(const QString &shapeId, const QPointF &at, QString *why)
{
    auto refuse = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    auto *s = dynamic_cast<ShapeItem *>(m_doc->item(shapeId));
    if (!s || s->locked) return refuse(tr("Glue points go on a shape that is not locked."));
    if (s->gluePoints.size() >= kMostGluePoints) return refuse(tr("A shape has no more than 16 glue points."));
    const QRectF frame(QPointF(0, 0), s->rect.size());
    bool invertible = false;
    const QTransform back = s->transform().inverted(&invertible);
    const QPointF local = invertible ? back.map(at) : QPointF(-1, -1);
    if (frame.width() <= 0 || frame.height() <= 0 || !frame.adjusted(-2, -2, 2, 2).contains(local)) return refuse(tr("Click on the shape to put a glue point there."));
    beginChange(tr("Add Glue Point"));
    s->gluePoints.push_back(QPointF(std::clamp(local.x() / frame.width(), 0.0, 1.0), std::clamp(local.y() / frame.height(), 0.0, 1.0)));
    endChange();
    return true;
}

int Editor::gluePointAt(const QString &shapeId, const QPointF &at, double reach) const
{
    const auto *s = dynamic_cast<const ShapeItem *>(m_doc->item(shapeId));
    if (!s) return -1;
    int best = -1;
    double bestAway = reach;
    for (int i = 0; i < s->gluePoints.size(); ++i) {
        const QPointF p = s->transform().map(QPointF(s->gluePoints[i].x() * s->rect.width(), s->gluePoints[i].y() * s->rect.height()));
        const double away = QLineF(p, at).length();
        if (away <= bestAway) {
            best = i;
            bestAway = away;
        }
    }
    return best;
}

bool Editor::removeGluePoint(const QString &shapeId, int index, QString *why)
{
    auto *s = dynamic_cast<ShapeItem *>(m_doc->item(shapeId));
    if (!s || s->locked || index < 0 || index >= s->gluePoints.size()) {
        if (why) *why = tr("There is no such glue point.");
        return false;
    }
    const int site = geometrySiteCount(*s) + index;
    beginChange(tr("Remove Glue Point"));
    s->gluePoints.removeAt(index);
    m_doc->forEachItem([&](Item *it, int, const QString &) {
        if (it->type() != ItemType::Line) return;
        for (LineItem::Glue *g : {&static_cast<LineItem *>(it)->start, &static_cast<LineItem *>(it)->end}) {
            if (g->id != shapeId) continue;
            if (g->site == site) *g = LineItem::Glue();
            else if (g->site > site) --g->site;
        }
    });
    endChange();
    return true;
}

} // namespace jp
