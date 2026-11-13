#include "core/barcodeclip.h"

#include "core/barcodeitem.h"
#include "office/barcodemarker.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QImage>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

bool barcodeClip(const QJsonObject &settings, ObjectClip *out, QString *why, bool provisional)
{
    QString problem = barcode::tooLong(barcodeOptions(settings));
    const ItemPtr code = problem.isEmpty() ? barcodeFromSettings(settings, QPointF(), &problem) : ItemPtr();
    if (!code || code->type() != ItemType::Group) {
        if (why) *why = problem.isEmpty() ? QCoreApplication::translate("jp::crunch::Barcode", "That is not a barcode JeffCrunch can draw.") : problem;
        return false;
    }
    const auto &group = static_cast<const GroupItem &>(*code);
    ClipMedia png, svg;
    png.token = QStringLiteral("rIdBarcodePng");
    png.extension = QStringLiteral("png");
    png.contentType = QStringLiteral("image/png");
    if (provisional) {
        static const QByteArray standIn = [] {
            QImage one(1, 1, QImage::Format_ARGB32);
            one.fill(Qt::transparent);
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            one.save(&buffer, "PNG");
            return bytes;
        }();
        png.bytes = standIn;
    } else {
        png.bytes = barcodePng(group);
    }
    svg.token = QStringLiteral("rIdBarcodeSvg");
    svg.extension = QStringLiteral("svg");
    svg.contentType = QStringLiteral("image/svg+xml");
    svg.bytes = barcodeSvg(group);
    if (png.bytes.isEmpty() || svg.bytes.isEmpty()) {
        if (why) *why = QCoreApplication::translate("jp::crunch::Barcode", "That is not a barcode JeffCrunch can draw.");
        return false;
    }
    // (The grid's unit is a pixel at 100%: three quarters of a point.)
    const double w = std::max(8.0, std::round(group.bounds().width() / 0.75)), h = std::max(8.0, std::round(group.bounds().height() / 0.75));
    constexpr double kEmuPerUnit = 9525;
    const QByteArray description = " descr=\"" + group.altText.toHtmlEscaped().toUtf8() + "\"";
    ObjectClip clip;
    clip.kind = QStringLiteral("picture");
    clip.width = w;
    clip.height = h;
    clip.media = {png, svg};
    clip.namespaces = " xmlns:asvg=\"http://schemas.microsoft.com/office/drawing/2016/SVG/main\"";
    clip.elementXml = "<xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"0\" name=\"\"" + description + ">" + office::barcodeMarkerXml(settings, office::barcodeHash(png.bytes), office::barcodeHash(svg.bytes))
                      + "</xdr:cNvPr><xdr:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></xdr:cNvPicPr></xdr:nvPicPr><xdr:blipFill><a:blip r:embed=\"" + png.token.toUtf8()
                      + "\"><a:extLst><a:ext uri=\"{96DAC541-7B7A-43D3-8B79-37D633B846F1}\"><asvg:svgBlip r:embed=\"" + svg.token.toUtf8()
                      + "\"/></a:ext></a:extLst></a:blip><a:stretch><a:fillRect/></a:stretch></xdr:blipFill><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\""
                      + QByteArray::number(qint64(w * kEmuPerUnit)) + "\" cy=\"" + QByteArray::number(qint64(h * kEmuPerUnit))
                      + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic>";
    clip.provisional = provisional;
    *out = clip;
    return true;
}

} // namespace jp::crunch
