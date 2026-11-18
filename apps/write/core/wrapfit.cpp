#include "core/wrapfit.h"

#include "core/wrapoutline.h"
#include "office/drawingml.h"
#include "render/shapes.h"

#include <QBuffer>
#include <QImageReader>

#include <algorithm>
#include <atomic>

namespace jp::write {

namespace {
std::atomic<quint64> g_decodes{0};
}

quint64 wrapSeedDecodes() { return g_decodes; }

QPolygonF seededWrapOutline(const QPainterPath &shape, const QString &cropShape, const QImage &image, const QSizeF &frame, const WrapSeedView &view)
{
    if (!(frame.width() > 0) || !(frame.height() > 0)) return QPolygonF();
    QPolygonF out = jp::wrapOutlineOfPath(shape, frame);
    if (out.isEmpty() && !cropShape.isEmpty() && cropShape != QLatin1String("rect")) {
        const QString id = office::shapeForPreset(cropShape);
        if (!id.isEmpty() && shapeDef(id) && !shapeDef(id)->open) out = jp::wrapOutlineOfPath(shapePath(id, frame), frame);
    }
    if (out.isEmpty() && shape.isEmpty() && !image.isNull()) {
        // (The part of the picture that shows: what is cut from the edges is left out.)
        QImage shown = image;
        const double l = std::clamp(view.cutLeft, 0.0, 1.0), t = std::clamp(view.cutTop, 0.0, 1.0), r = std::clamp(view.cutRight, 0.0, 1.0), b = std::clamp(view.cutBottom, 0.0, 1.0);
        if (l > 0 || t > 0 || r > 0 || b > 0) {
            const QRect part(int(l * image.width()), int(t * image.height()), int((1 - l - r) * image.width()), int((1 - t - b) * image.height()));
            if (part.width() < 1 || part.height() < 1) return QPolygonF();
            shown = image.copy(part);
        }
        out = jp::wrapOutlineOfAlpha(shown);
    }
    for (QPointF &p : out) p = QPointF(view.flipH ? 1 - p.x() : p.x(), view.flipV ? 1 - p.y() : p.y());
    return out;
}

QImage wrapSeedImage(const QByteArray &bytes)
{
    if (bytes.isEmpty()) return QImage();
    QByteArray copy = bytes;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    const QByteArray format = reader.format().toLower();
    if (format != "png" && format != "gif" && format != "tiff" && format != "tif") return QImage();   // (the others have no transparency)
    const QSize size = reader.size();
    if (!size.isValid() || size.isEmpty() || qint64(size.width()) * size.height() > kMostSeedPixels) return QImage();
    ++g_decodes;
    return reader.read();
}

} // namespace jp::write
