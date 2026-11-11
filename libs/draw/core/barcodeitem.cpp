#include "core/barcodeitem.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QHash>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include <algorithm>
#include <atomic>
#include <cmath>

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
    o.type = Type(std::clamp(int(std::clamp(s.value(QLatin1String("type")).toDouble(0), 0.0, 100.0)), 0, int(Type::Qr)));
    o.data = s.value(QLatin1String("data")).toString();
    // (What a file says is held to what the dialog allows: 50% to 300% of the size, 40% to 100% of the bar height, a price up to 99.99.)
    auto number = [&](const char *key, double standard, double least, double most) {
        const double v = s.value(QLatin1String(key)).toDouble(standard);
        return std::isfinite(v) ? std::clamp(v, least, most) : standard;
    };
    o.magnification = std::round(number("magnification", 100, 50, 300)) / 100.0;
    o.barHeight = std::round(number("height", 100, 40, 100)) / 100.0;
    o.text = s.value(QLatin1String("text")).toBool(true);
    o.code39Check = s.value(QLatin1String("check39")).toBool(false);
    o.qrLevel = std::clamp(int(number("qrLevel", 1, 0, 3)), 0, 3);
    if (o.type == Type::Isbn) {
        const int mode = std::clamp(int(number("priceMode", 1, 0, 2)), 0, 2);
        QString err;
        if (mode == 1) o.addOn = priceAddOn(Currency(std::clamp(int(number("currency", 0, 0, 4)), 0, 4) + 1), number("price", 19.95, 0, 99.99), &err);
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

namespace {
QMutex g_layoutLock;
QHash<QByteArray, barcode::Layout> g_layouts;
std::atomic<int> g_encodes{0};
} // namespace

barcode::Layout barcodeLayout(const QJsonObject &settings)
{
    const QByteArray key = QCryptographicHash::hash(QJsonDocument(settings).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256);
    {
        QMutexLocker lock(&g_layoutLock);
        if (const auto found = g_layouts.constFind(key); found != g_layouts.constEnd()) return found.value();
    }
    const barcode::Layout l = barcode::make(barcodeOptions(settings));
    ++g_encodes;
    QMutexLocker lock(&g_layoutLock);
    if (g_layouts.size() >= 64) g_layouts.clear();
    g_layouts.insert(key, l);
    return l;
}

int barcodeEncodes() { return g_encodes.load(); }

ItemPtr barcodeFromSettings(const QJsonObject &settings, const QPointF &topLeft, QString *error)
{
    const barcode::Layout l = barcodeLayout(settings);
    if (!l.error.isEmpty() || l.bars.isEmpty()) {
        if (error) *error = l.error;
        return nullptr;
    }
    return barcodeItem(l, topLeft, barcodeWhite(settings), barcodeDescription(settings, l), settings);
}

bool isBarcodeItem(const Item *it) { return it && it->type() == ItemType::Group && !static_cast<const GroupItem *>(it)->barcode.isEmpty(); }

QRectF barcodeFrame(const GroupItem &g)
{
    QRectF whole;
    for (const ItemPtr &c : g.children)
        if (c->type() == ItemType::Shape && !c->hidden) whole = whole.isNull() ? c->rect : whole.united(c->rect);
    return whole;
}

void barcodeTurn(const GroupItem &g, double *rotation, bool *flipH, bool *flipV)
{
    *rotation = 0;
    *flipH = *flipV = false;
    for (const ItemPtr &c : g.children)
        if (c->type() == ItemType::Shape && !c->hidden) {
            *rotation = c->rotation;
            *flipH = c->flipH;
            *flipV = c->flipV;
            return;
        }
}

void setBarcodeTurn(GroupItem &g, double rotation, bool flipH, bool flipV)
{
    for (const ItemPtr &c : g.children) {
        c->rotation = rotation;
        c->flipH = flipH;
        c->flipV = flipV;
    }
    g.syncRect();
}

QByteArray barcodeSvg(const GroupItem &g)
{
    const QRectF whole = barcodeFrame(g);
    if (g.barcode.isEmpty() || whole.isEmpty()) return {};
    QString svg = QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%1pt\" height=\"%2pt\" viewBox=\"0 0 %1 %2\">").arg(num(whole.width()), num(whole.height()));
    for (const ItemPtr &c : g.children) {
        if (c->type() != ItemType::Shape || c->hidden) continue;
        const auto *s = static_cast<const ShapeItem *>(c.get());
        const QPointF at = s->rect.topLeft() - whole.topLeft();
        if (s->customPath.isEmpty()) {
            svg += QStringLiteral("<rect x=\"%1\" y=\"%2\" width=\"%3\" height=\"%4\" fill=\"#ffffff\"/>").arg(num(at.x()), num(at.y()), num(s->rect.width()), num(s->rect.height()));
        } else {
            QTransform m;
            m.translate(at.x(), at.y());
            svg += QStringLiteral("<path d=\"%1\" fill=\"#000000\" fill-rule=\"nonzero\"/>").arg(pathData(m.map(s->customPath)));
        }
    }
    svg += QStringLiteral("</svg>");
    return svg.toUtf8();
}

QByteArray barcodePng(const GroupItem &g, int pixels, bool *capped)
{
    if (capped) *capped = false;
    const QByteArray svg = barcodeSvg(g);
    if (svg.isEmpty()) return {};
    if (pixels <= 0) {
        // Whole pixels to each module: 4 for a QR code, 3 for the others, more for a small code (about 1200 across), held to the largest side.
        const barcode::Layout l = barcodeLayout(g.barcode);
        double module = 0;
        for (const QRectF &r : l.bars) module = module == 0 ? r.width() : std::min(module, r.width());
        const double modules = module > 0 ? std::ceil(l.size.width() / module - 0.001) : 0;
        if (modules < 1) return {};
        const int least = barcodeOptions(g.barcode).type == barcode::Type::Qr ? 4 : 3;
        const int each = std::max(least, int(std::ceil(1200.0 / modules)));
        const double want = modules * each;
        pixels = int(std::min<double>(want, kMostBarcodePngSide));
        if (capped) *capped = want > kMostBarcodePngSide;
    }
    static QMutex lock;
    static QHash<QByteArray, QByteArray> kept;
    const QByteArray key = QCryptographicHash::hash(svg, QCryptographicHash::Sha256) + QByteArray::number(pixels);
    {
        QMutexLocker held(&lock);
        if (const auto found = kept.constFind(key); found != kept.constEnd()) return found.value();
    }
    QSvgRenderer r(svg);
    if (!r.isValid()) return {};
    QSizeF size = r.defaultSize();
    if (size.isEmpty()) return {};
    size.scale(std::clamp(pixels, 16, kMostBarcodePngSide), std::clamp(pixels, 16, kMostBarcodePngSide), Qt::KeepAspectRatio);
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
    QMutexLocker held(&lock);
    if (kept.size() >= 16) kept.clear();
    kept.insert(key, bytes);
    return bytes;
}

} // namespace jp
