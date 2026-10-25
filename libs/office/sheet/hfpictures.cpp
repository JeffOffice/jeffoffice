// The pictures of a sheet's headers and footers: the legacy drawing the sheet's `legacyDrawingHF` names holds a shape for each (its id says which part of which header or footer it is in, its
// style its size), and the shape's image is a part the drawing's own relationships name. Read here, bounded, into the page setup.

#include "sheet/hfpictures.h"

#include "sheet/sheetreader.h"

#include "office/xml.h"

#include <QXmlStreamReader>

#include <cmath>

namespace jp::office::sheet {

namespace {

constexpr QLatin1String kVml("urn:schemas-microsoft-com:vml");
constexpr QLatin1String kOffice("urn:schemas-microsoft-com:office:office");
// The most of a drawing part that is read, and the most shapes looked at in it (a file that repeats one is not read a million times over).
constexpr qint64 kMaxDrawingBytes = 1 << 20;
constexpr int kMaxShapesLooked = 4 * kMaxHeaderPictures;
// The most elements of any kind looked at in a drawing.
constexpr int kMaxVmlElements = 4096;

// A length from a style: a number with the unit after it (pt, in, cm, mm, px, pc); none is pixels. In points; negative when it is not one.
double lengthPoints(const QString &text)
{
    const QString t = text.trimmed().toLower();
    int n = 0;
    while (n < t.size() && (t[n].isDigit() || t[n] == QLatin1Char('.') || t[n] == QLatin1Char('-'))) ++n;
    bool ok = false;
    const double v = t.left(n).toDouble(&ok);
    if (!ok || !std::isfinite(v)) return -1;
    const QString unit = t.mid(n).trimmed();
    double factor = 0.75;   // (pixels)
    if (unit == QLatin1String("pt")) factor = 1;
    else if (unit == QLatin1String("in")) factor = 72;
    else if (unit == QLatin1String("cm")) factor = 72 / 2.54;
    else if (unit == QLatin1String("mm")) factor = 72 / 25.4;
    else if (unit == QLatin1String("pc")) factor = 12;
    else if (!unit.isEmpty() && unit != QLatin1String("px")) return -1;
    return v * factor;
}

QString extensionOf(const QString &part)
{
    const int dot = part.lastIndexOf(QLatin1Char('.'));
    const int slash = part.lastIndexOf(QLatin1Char('/'));
    return dot > slash ? part.mid(dot + 1).toLower() : QString();
}

} // namespace

void Workbook::readHeaderPictures(const QString &sheetPart, HeaderFooter *out) const
{
    out->pictures.clear();
    if (out->drawingRel.isEmpty() || sheetPart.isEmpty()) return;
    const Relationship rel = m_package.relationship(sheetPart, out->drawingRel);
    if (rel.id.isEmpty() || rel.external || !rel.type.endsWith(QLatin1String("/vmlDrawing"))) return;
    const QString key = rel.target.toLower();
    {
        // (a drawing is read once for the workbook, whoever asks: the sheets that share one, and a save that looks again)
        QMutexLocker lock(&m_headerImages->mutex);
        const auto have = m_headerImages->drawings.constFind(key);
        if (have != m_headerImages->drawings.constEnd()) {
            out->pictures = *have;
            return;
        }
    }
    QVector<HeaderPicture> found;
    const qint64 size = m_package.partSize(rel.target);
    if (size > 0 && size <= kMaxDrawingBytes) {
        const QByteArray xml = m_package.part(rel.target);
        QXmlStreamReader r(xml);
        int shapes = 0, elements = 0;
        struct Shape {
            QString id, style, relId, title;
            bool open = false;
        } shape;
        auto finish = [&] {
            if (!shape.open) return;
            shape.open = false;
            if (shape.relId.isEmpty() || shape.id.isEmpty() || found.size() >= kMaxHeaderPictures) return;
            const Relationship image = m_package.relationship(rel.target, shape.relId);
            if (image.id.isEmpty() || image.external || !image.type.endsWith(QLatin1String("/image"))) return;
            HeaderPicture p;
            p.place = canonicalHeaderPlace(shape.id.left(16));
            p.part = image.target;
            p.extension = extensionOf(image.target);
            p.title = shape.title.left(255);
            const QStringList properties = shape.style.split(QLatin1Char(';'));
            for (const QString &item : properties) {
                const int colon = item.indexOf(QLatin1Char(':'));
                if (colon < 0) continue;
                const QString name = item.left(colon).trimmed().toLower(), value = item.mid(colon + 1);
                if (name == QLatin1String("width")) p.width = lengthPoints(value);
                else if (name == QLatin1String("height")) p.height = lengthPoints(value);
            }
            if (!(p.width > 0 && p.height > 0)) return;
            p.width = std::min(p.width, kMaxHeaderPicturePoints);
            p.height = std::min(p.height, kMaxHeaderPicturePoints);
            // the image: read once for the workbook, and not past what the workbook may hold of them
            QMutexLocker lock(&m_headerImages->mutex);
            const auto have = m_headerImages->images.constFind(image.target);
            if (have != m_headerImages->images.constEnd()) {
                p.image = *have;
            } else {
                const qint64 bytes = m_package.partSize(image.target);
                if (bytes <= 0 || bytes > kMaxHeaderPictureBytes || m_headerImages->bytes + bytes > kMaxHeaderPictureBytesInBook) return;
                p.image = m_package.part(image.target);
                if (p.image.isEmpty()) return;
                m_headerImages->bytes += p.image.size();
                m_headerImages->images.insert(image.target, p.image);
            }
            found.append(p);
        };
        while (!r.atEnd()) {
            const QXmlStreamReader::TokenType t = r.readNext();
            if (t == QXmlStreamReader::StartElement) {
                // (every element is counted, not only the shapes: a drawing of a million empty elements is not read to its end)
                if (++elements > kMaxVmlElements) break;
                if (r.namespaceUri() == kVml && r.name() == QLatin1String("shape")) {
                    finish();
                    if (++shapes > kMaxShapesLooked) break;
                    shape = Shape();
                    shape.open = true;
                    shape.id = r.attributes().value(QLatin1String("id")).toString();
                    shape.style = r.attributes().value(QLatin1String("style")).toString();
                } else if (shape.open && r.namespaceUri() == kVml && r.name() == QLatin1String("imagedata") && shape.relId.isEmpty()) {
                    shape.relId = r.attributes().value(kOffice, QLatin1String("relid")).toString();
                    shape.title = r.attributes().value(kOffice, QLatin1String("title")).toString();
                }
            } else if (t == QXmlStreamReader::EndElement && r.namespaceUri() == kVml && r.name() == QLatin1String("shape")) {
                finish();
            }
        }
        finish();
        // (a drawing that is not well formed has its pictures left out, all of them)
        if (r.hasError()) found.clear();
    }
    QMutexLocker lock(&m_headerImages->mutex);
    ++m_headerImages->parses;
    m_headerImages->drawings.insert(key, found);
    out->pictures = found;
}

} // namespace jp::office::sheet

namespace jp::office::sheet {

namespace {

QByteArray escaped(const QString &text)
{
    QString out;
    for (const QChar c : text) {
        if (c == QLatin1Char('&')) out += QStringLiteral("&amp;");
        else if (c == QLatin1Char('<')) out += QStringLiteral("&lt;");
        else if (c == QLatin1Char('>')) out += QStringLiteral("&gt;");
        else if (c == QLatin1Char('"')) out += QStringLiteral("&quot;");
        else if (c.unicode() >= 0x20 || c == QLatin1Char('\t')) out += c;
    }
    return out.toUtf8();
}

} // namespace

QByteArray headerPicturesVml(const QVector<HeaderPicture> &pictures)
{
    QByteArray xml = "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\">"
                     "<o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout>"
                     "<v:shapetype id=\"_x0000_t75\" coordsize=\"21600,21600\" o:spt=\"75\" o:preferrelative=\"t\" path=\"m@4@5l@4@11@9@11@9@5xe\" filled=\"f\" stroked=\"f\">"
                     "<v:stroke joinstyle=\"miter\"/><v:formulas><v:f eqn=\"if lineDrawn pixelLineWidth 0\"/><v:f eqn=\"sum @0 1 0\"/><v:f eqn=\"sum 0 0 @1\"/><v:f eqn=\"prod @2 1 2\"/>"
                     "<v:f eqn=\"prod @3 21600 pixelWidth\"/><v:f eqn=\"prod @3 21600 pixelHeight\"/><v:f eqn=\"sum @0 0 1\"/><v:f eqn=\"prod @6 1 2\"/><v:f eqn=\"prod @7 21600 pixelWidth\"/>"
                     "<v:f eqn=\"sum @8 21600 0\"/><v:f eqn=\"prod @7 21600 pixelHeight\"/><v:f eqn=\"sum @10 21600 0\"/></v:formulas>"
                     "<v:path o:extrusionok=\"f\" gradientshapeok=\"t\" o:connecttype=\"rect\"/><o:lock v:ext=\"edit\" aspectratio=\"t\"/></v:shapetype>";
    for (int i = 0; i < pictures.size() && i < kMaxHeaderPictures; ++i) {
        const HeaderPicture &p = pictures[i];
        xml += "<v:shape id=\"" + escaped(p.place) + "\" o:spid=\"_x0000_s" + QByteArray::number(1025 + i) + "\" type=\"#_x0000_t75\" style=\"position:absolute;margin-left:0;margin-top:0;width:" + QByteArray::number(p.width, 'g', 8)
               + "pt;height:" + QByteArray::number(p.height, 'g', 8) + "pt;z-index:" + QByteArray::number(i + 1) + "\"><v:imagedata o:relid=\"rId" + QByteArray::number(i + 1) + "\" o:title=\"" + escaped(p.title)
               + "\"/><o:lock v:ext=\"edit\" rotation=\"t\"/></v:shape>";
    }
    return xml + "</xml>";
}

QString headerPictureContentType(const QString &extension)
{
    const QString e = extension.toLower();
    if (e == QLatin1String("png")) return QStringLiteral("image/png");
    if (e == QLatin1String("jpeg") || e == QLatin1String("jpg")) return QStringLiteral("image/jpeg");
    if (e == QLatin1String("gif")) return QStringLiteral("image/gif");
    if (e == QLatin1String("bmp")) return QStringLiteral("image/bmp");
    return QString();
}

} // namespace jp::office::sheet
