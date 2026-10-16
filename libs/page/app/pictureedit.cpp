#include "app/pictureedit.h"

namespace jp {

void clearPictureCrop(PictureItem *p)
{
    // Grow the frame to show the whole picture.
    const QPointF tl = p->transform().map(p->imgRect.topLeft());
    const QPointF br = p->transform().map(p->imgRect.bottomRight());
    const QPointF c = (tl + br) / 2;
    p->rect = QRectF(c.x() - p->imgRect.width() / 2, c.y() - p->imgRect.height() / 2, p->imgRect.width(), p->imgRect.height());
    p->imgRect = QRectF(QPointF(0, 0), p->rect.size());
}

void cropPictureToAspect(PictureItem *p, double width, double height)
{
    if (width <= 0 || height <= 0 || p->rect.isEmpty()) return;
    const QSizeF frame = p->rect.size();
    double w = frame.width(), h = frame.height();
    if (w * height > h * width) w = h * width / height;
    else h = w * height / width;
    // The frame shrinks around its middle, so the picture moves by half of what the frame lost, in the frame's own coordinates.
    p->imgRect.translate(-QPointF(frame.width() - w, frame.height() - h) / 2);
    p->rect = QRectF(p->rect.center() - QPointF(w / 2, h / 2), QSizeF(w, h));
}

} // namespace jp
