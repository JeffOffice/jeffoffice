// What the files that write DrawingML share (drawingml_write_util.h).

#include "office/drawingml_write_util.h"

#include "office/drawingml_style.h"
#include "office/xml.h"

#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QRegularExpression>
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

void writeDecorative(QXmlStreamWriter &w)
{
    w.writeStartElement(QStringLiteral("a:extLst"));
    w.writeStartElement(QStringLiteral("a:ext"));
    w.writeAttribute(QStringLiteral("uri"), QStringLiteral("{C183D7F6-B498-43B3-948B-1728B52AA6E4}"));
    w.writeStartElement(QStringLiteral("adec:decorative"));
    w.writeAttribute(QStringLiteral("xmlns:adec"), QStringLiteral("http://schemas.microsoft.com/office/drawing/2017/decorative"));
    w.writeAttribute(QStringLiteral("val"), QStringLiteral("1"));
    w.writeEndElement();
    w.writeEndElement();
    w.writeEndElement();
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

void writeTextEffects(QXmlStreamWriter &w, const TextEffects &fx)
{
    // As the file had it: its tokens, with the prefixes it wrote them in (the part declares them).
    if (!fx.xml.isEmpty()) {
        QXmlStreamReader check(fx.xml);
        check.setNamespaceProcessing(false);
        while (!check.atEnd()) check.readNext();
        if (!check.hasError()) {
            QXmlStreamReader in(fx.xml);
            in.setNamespaceProcessing(false);
            // (token by token: with no namespaces processed, the reader's own copy would drop the prefix)
            while (!in.atEnd()) {
                in.readNext();
                if (in.tokenType() == QXmlStreamReader::StartElement) {
                    w.writeStartElement(in.qualifiedName().toString());
                    for (const QXmlStreamAttribute &a : in.attributes()) w.writeAttribute(a.qualifiedName().toString(), a.value().toString());
                } else if (in.tokenType() == QXmlStreamReader::EndElement) {
                    w.writeEndElement();
                }
            }
            return;
        }
    }
    // Effects that say there are none: a list with nothing in it, so that a run takes off what its style gives.
    if (!fx.any()) {
        w.writeStartElement(QStringLiteral("a:effectLst"));
        w.writeEndElement();
        return;
    }
    auto shadow = [&](const char *tag, const TextShadow &s, bool outer) {
        w.writeStartElement(QLatin1String(tag));
        w.writeAttribute(QStringLiteral("blurRad"), QString::number(extent(s.blur)));
        w.writeAttribute(QStringLiteral("dist"), QString::number(extent(s.distance)));
        w.writeAttribute(QStringLiteral("dir"), QString::number(angle(s.angle)));
        if (s.sx != 1) w.writeAttribute(QStringLiteral("sx"), QString::number(percent(s.sx)));
        if (s.sy != 1) w.writeAttribute(QStringLiteral("sy"), QString::number(percent(s.sy)));
        if (s.kx != 0) w.writeAttribute(QStringLiteral("kx"), QString::number(angle(s.kx)));
        if (s.ky != 0) w.writeAttribute(QStringLiteral("ky"), QString::number(angle(s.ky)));
        if (outer) {
            w.writeAttribute(QStringLiteral("algn"), s.align);
            w.writeAttribute(QStringLiteral("rotWithShape"), QStringLiteral("0"));
        }
        writeColor(w, s.color);
        w.writeEndElement();
    };
    w.writeStartElement(QStringLiteral("a:effectLst"));
    if (fx.glow.on) {
        w.writeStartElement(QStringLiteral("a:glow"));
        w.writeAttribute(QStringLiteral("rad"), QString::number(extent(fx.glow.radius)));
        writeColor(w, fx.glow.color);
        w.writeEndElement();
    }
    if (fx.inner.on) shadow("a:innerShdw", fx.inner, false);
    if (fx.outer.on) shadow("a:outerShdw", fx.outer, true);
    if (fx.reflection.on) {
        const TextReflection &r = fx.reflection;
        w.writeStartElement(QStringLiteral("a:reflection"));
        w.writeAttribute(QStringLiteral("blurRad"), QString::number(extent(r.blur)));
        w.writeAttribute(QStringLiteral("stA"), QString::number(percent(r.startAlpha)));
        w.writeAttribute(QStringLiteral("stPos"), QString::number(percent(r.startPos)));
        w.writeAttribute(QStringLiteral("endA"), QString::number(percent(r.endAlpha)));
        w.writeAttribute(QStringLiteral("endPos"), QString::number(percent(r.endPos)));
        w.writeAttribute(QStringLiteral("dist"), QString::number(extent(r.distance)));
        w.writeAttribute(QStringLiteral("dir"), QString::number(angle(r.angle)));
        w.writeAttribute(QStringLiteral("fadeDir"), QString::number(angle(r.fadeAngle)));
        w.writeAttribute(QStringLiteral("sx"), QString::number(percent(r.sx)));
        w.writeAttribute(QStringLiteral("sy"), QString::number(percent(r.sy)));
        if (r.kx != 0) w.writeAttribute(QStringLiteral("kx"), QString::number(angle(r.kx)));
        if (r.ky != 0) w.writeAttribute(QStringLiteral("ky"), QString::number(angle(r.ky)));
        w.writeAttribute(QStringLiteral("algn"), r.align);
        w.writeAttribute(QStringLiteral("rotWithShape"), QStringLiteral("0"));
        w.writeEndElement();
    }
    if (fx.softEdge > 0) {
        w.writeStartElement(QStringLiteral("a:softEdge"));
        w.writeAttribute(QStringLiteral("rad"), QString::number(extent(fx.softEdge)));
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
    const QString uri = type.contains(QLatin1String("://")) ? type : QString(ns::R) + QLatin1Char('/') + type;
    const QString id = ctx.package->addRelationship(ctx.part, uri, target, external);
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

QString relateSvg(DrawingWriteContext &ctx, const QString &imageId)
{
    if (!ctx.package || !ctx.mediaParts || !ctx.imageBytes || imageId.isEmpty()) return QString();
    const QString key = QStringLiteral("svg:") + imageId;
    QString part = ctx.mediaParts->value(key);
    if (part.isEmpty()) {
        QString format;
        const QByteArray bytes = ctx.imageBytes(imageId, &format);
        // What the bytes are, whatever the format named, else the name.
        const QString sniffed = imageFormat(bytes);
        if (bytes.isEmpty() || (sniffed.isEmpty() ? format.toLower() : sniffed) != QLatin1String("svg")) return QString();
        int n = ctx.mediaParts->size() + 1;
        const QStringList taken = ctx.mediaParts->values();
        do {
            part = QStringLiteral("/ppt/media/image%1.svg").arg(n++);
        } while (taken.contains(part));
        ctx.package->addDefault(QStringLiteral("svg"), QStringLiteral("image/svg+xml"));
        ctx.package->addPart(part, bytes, QStringLiteral("image/svg+xml"));
        ctx.mediaParts->insert(key, part);
    }
    return relate(ctx, QStringLiteral("image"), part);
}

QString relateAddress(DrawingWriteContext &ctx, const QString &address)
{
    return relate(ctx, QStringLiteral("hyperlink"), address, true);
}

MediaRelations relateMedia(DrawingWriteContext &ctx, const PictureItem &p)
{
    MediaRelations r;
    const bool video = p.mediaKind == QLatin1String("video");
    if (!video && p.mediaKind != QLatin1String("audio")) return r;
    const QString type = video ? QStringLiteral("video") : QStringLiteral("audio");
    if (!p.mediaId.isEmpty() && ctx.package && ctx.mediaParts && ctx.mediaBytes) {
        // Stored once for each media id, beside the pictures, as the presentation program does.
        const QString key = QStringLiteral("media:") + p.mediaId;
        QString part = ctx.mediaParts->value(key);
        if (part.isEmpty()) {
            QString contentType, extension;
            const QByteArray bytes = ctx.mediaBytes(p.mediaId, &contentType, &extension);
            static const QRegularExpression plain(QStringLiteral("^[a-z0-9]{1,8}$"));
            extension = extension.toLower();
            if (!plain.match(extension).hasMatch()) extension = QStringLiteral("bin");
            if (contentType.isEmpty()) contentType = video ? QStringLiteral("video/unknown") : QStringLiteral("audio/unknown");
            if (!bytes.isEmpty()) {
                const QStringList taken = ctx.mediaParts->values();
                int n = 1;
                do {
                    part = QStringLiteral("/ppt/media/media%1.%2").arg(n++).arg(extension);
                } while (taken.contains(part));
                ctx.package->addPart(part, bytes, contentType);
                ctx.mediaParts->insert(key, part);
            }
        }
        if (!part.isEmpty()) {
            r.link = relate(ctx, type, part);
            r.copy = relate(ctx, QStringLiteral("http://schemas.microsoft.com/office/2007/relationships/media"), part);
            return r;
        }
    }
    if (!p.mediaLink.isEmpty()) r.link = relate(ctx, type, p.mediaLink, true);
    return r;
}

} // namespace jp::office::dmlwrite
