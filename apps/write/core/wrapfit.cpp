#include "core/wrapfit.h"

#include "core/wrapoutline.h"
#include "office/drawingml.h"
#include "render/shapes.h"

#include <QBuffer>
#include <QImageReader>

#include <algorithm>

namespace jp::write {

QPolygonF seededWrapOutline(const QPainterPath &shape, const QString &cropShape, const QImage &image, const QSizeF &frame, int *budget)
{
    if (budget && *budget <= 0) return QPolygonF();
    if (!(frame.width() > 0) || !(frame.height() > 0)) return QPolygonF();
    QPolygonF out = jp::wrapOutlineOfPath(shape, frame);
    if (out.isEmpty() && !cropShape.isEmpty() && cropShape != QLatin1String("rect")) {
        const QString id = office::shapeForPreset(cropShape);
        if (!id.isEmpty() && shapeDef(id) && !shapeDef(id)->open) out = jp::wrapOutlineOfPath(shapePath(id, frame), frame);
    }
    if (out.isEmpty() && shape.isEmpty()) out = jp::wrapOutlineOfAlpha(image);
    if (budget && !out.isEmpty()) --*budget;
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
    if (std::max(size.width(), size.height()) > kAlphaOutlineSide) {
        const double k = double(kAlphaOutlineSide) / std::max(size.width(), size.height());
        reader.setScaledSize(QSize(std::max(1, int(size.width() * k)), std::max(1, int(size.height() * k))));
    }
    return reader.read();
}

} // namespace jp::write
