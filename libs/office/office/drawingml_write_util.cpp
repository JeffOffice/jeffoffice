// What the files that write DrawingML share (drawingml_write_util.h).

#include "office/drawingml_write_util.h"

#include "office/drawingml_style.h"
#include "office/xml.h"

#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <algorithm>
#include <cmath>

namespace jp::office::dmlwrite {

namespace {

constexpr double kMaxPoint = 1e6;     // points: as far as the reading takes a position or a size

// The extension of a picture's file, and what the package calls its type.
struct MediaKind {
    const char *extension, *contentType;
};

MediaKind mediaKindOf(const QString &format)
{
    if (format == QLatin1String("png")) return {"png", "image/png"};
    if (format == QLatin1String("jpeg") || format == QLatin1String("jpg")) return {"jpeg", "image/jpeg"};
    if (format == QLatin1String("gif")) return {"gif", "image/gif"};
    if (format == QLatin1String("bmp")) return {"bmp", "image/bmp"};
    if (format == QLatin1String("tiff") || format == QLatin1String("tif")) return {"tiff", "image/tiff"};
    if (format == QLatin1String("emf")) return {"emf", "image/x-emf"};
    if (format == QLatin1String("wmf")) return {"wmf", "image/x-wmf"};
    return {nullptr, nullptr};
}

// A vector picture the presentation program has no use for, drawn as pixels.
QByteArray rasterOf(const QByteArray &svg)
{
    QSvgRenderer renderer(svg);
    if (!renderer.isValid()) return QByteArray();
    QSizeF size = renderer.defaultSize();
    if (size.isEmpty()) size = QSizeF(512, 512);
    const double scale = 2048.0 / std::max(size.width(), size.height());
    QImage image(QSize(std::max(1, int(size.width() * scale)), std::max(1, int(size.height() * scale))), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return png;
}

} // namespace

QString emu(double points)
{
    return QString::number(position(points));
}

qint64 position(double points)
{
    return pointsToEmu(std::isfinite(points) ? std::clamp(points, -kMaxPoint, kMaxPoint) : 0.0);
}

qint64 extent(double points)
{
    return std::max<qint64>(0, position(points));
}

qint64 rounded(double value)
{
    return std::isfinite(value) ? std::llround(std::clamp(value, -1e15, 1e15)) : 0;
}

qint64 angle(double degrees)
{
    if (!std::isfinite(degrees)) return 0;
    double d = std::fmod(degrees, 360.0);
    if (d < 0) d += 360;
    const qint64 v = rounded(d * 60000.0);
    return v >= 21600000 ? 0 : v;
}

qint64 percent(double fraction)
{
    return rounded(fraction * 100000.0);
}

QString clean(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        const ushort u = c.unicode();
        if (c.isHighSurrogate() && i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
            out += c;
            out += text[++i];
        } else if (u == '\t' || u == '\n' || u == '\r' || (u >= 0x20 && (u < 0xD800 || u > 0xDFFF) && u != 0xFFFE && u != 0xFFFF)) {
            out += c;
        }
    }
    return out;
}

QColor resolve(const ColorRef &color, const DrawingWriteContext &ctx)
{
    if (color.isNone()) return QColor();
    QColor c = color.resolve(ctx.colors);
    // A context with no scheme of its own: the program's first.
    if (!c.isValid() && color.kind() == ColorRef::Scheme) c = color.resolve(builtinColorSchemes().value(0));
    return c;
}

void writeColor(QXmlStreamWriter &w, const QColor &color, double transparency)
{
    w.writeStartElement(QStringLiteral("a:srgbClr"));
    w.writeAttribute(QStringLiteral("val"), color.name(QColor::HexRgb).mid(1).toUpper());
    const double whole = color.alphaF() * (1.0 - std::clamp(transparency, 0.0, 1.0));
    if (whole < 0.99995) {
        w.writeStartElement(QStringLiteral("a:alpha"));
        w.writeAttribute(QStringLiteral("val"), QString::number(std::clamp<qint64>(percent(whole), 0, 100000)));
        w.writeEndElement();
    }
    w.writeEndElement();
}

QString relate(DrawingWriteContext &ctx, const QString &type, const QString &target, bool external)
{
    if (!ctx.package || ctx.part.isEmpty() || target.isEmpty()) return QString();
    const QString key = type + (external ? QLatin1String(" external ") : QLatin1String(" internal ")) + target;
    const auto known = ctx.relations.constFind(key);
    if (known != ctx.relations.constEnd()) return *known;
    const QString id = ctx.package->addRelationship(ctx.part, QString(ns::R) + QLatin1Char('/') + type, target, external);
    ctx.relations.insert(key, id);
    return id;
}

QString relatePicture(DrawingWriteContext &ctx, const QString &imageId)
{
    if (!ctx.package || !ctx.mediaParts || !ctx.imageBytes || imageId.isEmpty()) return QString();
    QString part = ctx.mediaParts->value(imageId);
    if (part.isEmpty()) {
        QString format;
        QByteArray bytes = ctx.imageBytes(imageId, &format);
        if (bytes.isEmpty()) return QString();
        // What the bytes are, whatever the format named, else the name.
        const QString sniffed = imageFormat(bytes);
        format = sniffed.isEmpty() ? format.toLower() : sniffed;
        if (format == QLatin1String("svg")) {
            bytes = rasterOf(bytes);
            format = QStringLiteral("png");
        }
        const MediaKind kind = mediaKindOf(format);
        if (!kind.extension || bytes.isEmpty()) return QString();
        int n = ctx.mediaParts->size() + 1;
        const QStringList taken = ctx.mediaParts->values();
        do {
            part = QStringLiteral("/ppt/media/image%1.%2").arg(n++).arg(QLatin1String(kind.extension));
        } while (taken.contains(part));
        ctx.package->addDefault(QLatin1String(kind.extension), QLatin1String(kind.contentType));
        ctx.package->addPart(part, bytes, QLatin1String(kind.contentType));
        ctx.mediaParts->insert(imageId, part);
    }
    return relate(ctx, QStringLiteral("image"), part);
}

QString relateAddress(DrawingWriteContext &ctx, const QString &address)
{
    return relate(ctx, QStringLiteral("hyperlink"), address, true);
}

} // namespace jp::office::dmlwrite
