#include "core/barcodeitem.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include <algorithm>

namespace jp {

namespace {
// The digits' font: OCR-B (the standard for EAN and ISBN digits) when the
// computer has it, otherwise Arimo.
QString digitFamily()
{
    static const QString f = [] {
        for (const QString &name : {QStringLiteral("OCR-B"), QStringLiteral("OCR B"), QStringLiteral("OCRB"), QStringLiteral("OCR-B 10 BT")})
            if (QFontDatabase::hasFamily(name)) return name;
        return QStringLiteral("Arimo");
    }();
    return f;
}

QString num(double v) { return QString::number(v, 'f', 3); }

QString pathData(const QPainterPath &p)
{
    QString d;
    for (int i = 0; i < p.elementCount(); ++i) {
        const QPainterPath::Element e = p.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement: d += QStringLiteral("M%1 %2").arg(num(e.x), num(e.y)); break;
        case QPainterPath::LineToElement: d += QStringLiteral("L%1 %2").arg(num(e.x), num(e.y)); break;
        case QPainterPath::CurveToElement:
            if (i + 2 < p.elementCount())
                d += QStringLiteral("C%1 %2 %3 %4 %5 %6").arg(num(e.x), num(e.y), num(p.elementAt(i + 1).x), num(p.elementAt(i + 1).y), num(p.elementAt(i + 2).x), num(p.elementAt(i + 2).y));
            i += 2;
            break;
        default: break;
        }
    }
    return d;
}
} // namespace

QPainterPath barcodePath(const barcode::Layout &l)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    for (const QRectF &r : l.bars) path.addRect(r);
    // Digits as letter outlines (the same everywhere, whatever fonts are
    // installed), made at 1000 px and scaled: Qt sizes fonts in whole pixels.
    QFont f(digitFamily());
    f.setPixelSize(1000);
    const QFontMetricsF fm(f);
    for (const barcode::Label &lb : l.labels) {
        QPainterPath t;
        t.addText(QPointF(-fm.horizontalAdvance(lb.text) / 2, 0), f, lb.text);
        QTransform m;
        m.translate(lb.baseline.x(), lb.baseline.y());
        m.scale(lb.size / 1000, lb.size / 1000);
        path.addPath(m.map(t));
    }
    return path;
}

ItemPtr barcodeItem(const barcode::Layout &l, const QPointF &topLeft, bool whiteBackground, const QString &description, const QJsonObject &settings)
{
    auto group = std::make_shared<GroupItem>();
    group->barcode = settings;
    const QRectF frame(topLeft, l.size);
    if (whiteBackground) {
        auto bg = std::make_shared<ShapeItem>();
        bg->rect = frame;
        bg->fill = Fill::solid(ColorRef::rgb(Qt::white));
        bg->stroke = Stroke::none();
        bg->name = QStringLiteral("Quiet zone");
        group->children.push_back(bg);
    }
    auto bars = std::make_shared<ShapeItem>();
    bars->rect = frame;
    bars->customPath = barcodePath(l);
    bars->fill = Fill::solid(ColorRef::rgb(Qt::black));
    bars->stroke = Stroke::none();
    bars->name = QStringLiteral("Bars");
    group->children.push_back(bars);
    group->name = QStringLiteral("Barcode");
    group->altText = description;
    group->syncRect();
    return group;
}

barcode::Options barcodeOptions(const QJsonObject &s)
{
    using namespace barcode;
    Options o;
    o.type = Type(std::clamp(s.value(QLatin1String("type")).toInt(), 0, int(Type::Qr)));
    o.data = s.value(QLatin1String("data")).toString();
    o.magnification = s.value(QLatin1String("magnification")).toInt(100) / 100.0;
    o.barHeight = s.value(QLatin1String("height")).toInt(100) / 100.0;
    o.text = s.value(QLatin1String("text")).toBool(true);
    o.code39Check = s.value(QLatin1String("check39")).toBool(false);
    o.qrLevel = std::clamp(s.value(QLatin1String("qrLevel")).toInt(1), 0, 3);
    if (o.type == Type::Isbn) {
        const int mode = s.value(QLatin1String("priceMode")).toInt(1);
        QString err;
        if (mode == 1) o.addOn = priceAddOn(Currency(std::clamp(s.value(QLatin1String("currency")).toInt(0), 0, 4) + 1), s.value(QLatin1String("price")).toDouble(19.95), &err);
        else if (mode == 2) o.addOn = priceAddOn(Currency::None, 0, &err);
    } else if (o.type == Type::Ean13 || o.type == Type::UpcA || o.type == Type::Ean8) {
        o.addOn = s.value(QLatin1String("addOn")).toString();
    }
    return o;
}

bool barcodeWhite(const QJsonObject &settings) { return settings.value(QLatin1String("white")).toBool(true); }

QString barcodeDescription(const QJsonObject &settings, const barcode::Layout &l)
{
    using namespace barcode;
    const Type t = barcodeOptions(settings).type;
    // The description goes into the document, so it names the type in English
    // whatever language the combo box shows.
    static const char *const kKind[] = {"", "EAN-13", "UPC-A", "EAN-8", "Code", "Code", "QR code"};
    QString what;
    if (t == Type::Isbn) what = isbnCaption(settings.value(QLatin1String("data")).toString());
    else if (t == Type::Qr) what = QStringLiteral("QR code ") + (l.encoded.size() > 120 ? l.encoded.left(120) + QStringLiteral("...") : l.encoded);
    else what = QString::fromLatin1(kKind[int(t)]) + QLatin1Char(' ') + l.encoded;
    return QStringLiteral("Barcode: %1").arg(what);
}

ItemPtr barcodeFromSettings(const QJsonObject &settings, const QPointF &topLeft, QString *error)
{
    const barcode::Layout l = barcode::make(barcodeOptions(settings));
    if (!l.error.isEmpty() || l.bars.isEmpty()) {
        if (error) *error = l.error;
        return nullptr;
    }
    return barcodeItem(l, topLeft, barcodeWhite(settings), barcodeDescription(settings, l), settings);
}

bool isBarcodeItem(const Item *it) { return it && it->type() == ItemType::Group && !static_cast<const GroupItem *>(it)->barcode.isEmpty(); }

QByteArray barcodeSvg(const GroupItem &g)
{
    const QRectF whole = g.bounds();
    if (g.barcode.isEmpty() || whole.isEmpty()) return {};
    QString svg = QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%1pt\" height=\"%2pt\" viewBox=\"0 0 %1 %2\">").arg(num(whole.width()), num(whole.height()));
    for (const ItemPtr &c : g.children) {
        if (c->type() != ItemType::Shape || c->hidden) continue;
        const auto *s = static_cast<const ShapeItem *>(c.get());
        const QPointF at = s->rect.topLeft() - whole.topLeft();
        const bool white = s->customPath.isEmpty();
        QString turn;
        if (s->rotation != 0) turn = QStringLiteral(" transform=\"rotate(%1 %2 %3)\"").arg(num(s->rotation), num(at.x() + s->rect.width() / 2), num(at.y() + s->rect.height() / 2));
        if (white) {
            svg += QStringLiteral("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#ffffff\"%5/>").arg(num(at.x()), num(at.y()), num(s->rect.width()), num(s->rect.height()), turn);
        } else {
            QTransform m;
            m.translate(at.x(), at.y());
            svg += QStringLiteral("<path d=\"%1\" fill=\"#000000\" fill-rule=\"nonzero\"%2/>").arg(pathData(m.map(s->customPath)), turn);
        }
    }
    svg += QStringLiteral("</svg>");
    return svg.toUtf8();
}

QByteArray barcodePng(const GroupItem &g, int pixels)
{
    const QByteArray svg = barcodeSvg(g);
    if (svg.isEmpty()) return {};
    QSvgRenderer r(svg);
    if (!r.isValid()) return {};
    QSizeF size = r.defaultSize();
    if (size.isEmpty()) return {};
    size.scale(std::clamp(pixels, 16, 4000), std::clamp(pixels, 16, 4000), Qt::KeepAspectRatio);
    QImage image(size.toSize().expandedTo(QSize(1, 1)), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    {
        QPainter p(&image);
        p.setRenderHint(QPainter::Antialiasing);
        r.render(&p, QRectF(QPointF(0, 0), size));
    }
    QByteArray bytes;
    QBuffer out(&bytes);
    out.open(QIODevice::WriteOnly);
    image.save(&out, "PNG");
    return bytes;
}

} // namespace jp
