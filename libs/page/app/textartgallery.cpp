#include "app/textartgallery.h"

#include "app/editor.h"
#include "app/icons.h"
#include "render/textart.h"

#include <QPainter>

namespace jp {

QVector<GalleryItem> textArtGalleryItems(Editor *ed)
{
    QVector<GalleryItem> v;
    Editor *e = ed;
    for (const auto &s : textArtStyles()) {
        const QString sid = s.id;
        v << GalleryItem{sid, s.name, drawnIcon([e, sid](QPainter *p, const QRectF &rc) {
            TextArtItem w;
            for (const auto &st : textArtStyles()) if (st.id == sid) applyTextArtStyle(w, st);
            w.text = QStringLiteral("Aa");
            w.rect = QRectF(QPointF(0, 0), rc.size() * 0.8);
            const QPainterPath path = textArtPath(w, w.rect.size()).translated(rc.topLeft() + QPointF(rc.width() * 0.1, rc.height() * 0.1));
            p->fillPath(path, w.fill.brush(path.boundingRect(), e->doc()->colors));
            if (!w.stroke.isNone()) p->strokePath(path, QPen(w.stroke.color.resolve(e->doc()->colors), std::max(0.5, w.stroke.width / 2)));
        }), QString()};
    }
    return v;
}

std::shared_ptr<TextArtItem> newTextArt(const QString &styleId)
{
    auto w = std::make_shared<TextArtItem>();
    for (const auto &st : textArtStyles()) if (st.id == styleId) applyTextArtStyle(*w, st);
    return w;
}

} // namespace jp
