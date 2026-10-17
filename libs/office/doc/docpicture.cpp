// The pictures of an old binary document: a picture sits in the data stream as a header with the size it is shown at, then a drawing record
// that says how it is cropped, then the picture itself in a record of the binary drawing format.

#include "doc/docconvert.h"

#include "office/odraw.h"
#include "office/odraw_read.h"

#include <QCoreApplication>
#include <QBuffer>
#include <QImageReader>
#include <algorithm>

namespace jp::office::doc {

namespace {

// The picture record at `offset` of `s`, with its header, if it holds one.
QByteArray blipAt(const Span &s, qint64 offset)
{
    if (!s.fits(offset, 8)) return QByteArray();
    const quint16 type = s.u16(offset + 2);
    const qint64 len = s.u32(offset + 4);
    if (type < 0xF018 || type > 0xF117 || len > kMaxPictureBytes || !s.fits(offset, 8 + len)) return QByteArray();
    return s.bytes(offset, 8 + len);
}

// The cuts a drawing's property table states (the fractions of the picture's sides that are cut off).
void cropOf(const QByteArray &record, double *l, double *t, double *r, double *b)
{
    const QVector<OdrawRecord> top = odrawRecords(record);
    for (const OdrawRecord &c : top) {
        if (c.type != 0xF004) continue;
        for (const OdrawRecord &k : odrawRecords(record, c.offset, qsizetype(qint64(c.offset) + c.length))) {
            if (k.type != 0xF00B) continue;
            for (int i = 0; i < int(k.instance); ++i) {
                const qint64 at = qint64(k.offset) + 6 * qint64(i);
                if (!odraw::fits(record, at, 6)) break;
                const int pid = odraw::u16(record, at) & 0x3FFF;
                const double v = double(odraw::i32(record, at + 2)) / 65536.0;
                if (pid == 0x0100) *t = v;
                else if (pid == 0x0101) *b = v;
                else if (pid == 0x0102) *l = v;
                else if (pid == 0x0103) *r = v;
            }
        }
        return;
    }
}

} // namespace

bool Converter::readPicture(qint32 position, Picture *out)
{
    const Span d = data;
    if (position < 0 || !d.fits(position, 68)) return false;
    const qint64 lcb = d.u32(position), cbHeader = d.u16(position + 4);
    if (cbHeader < 68 || lcb < cbHeader || lcb > kMaxPictureBytes || !d.fits(position, lcb)) return false;
    qint64 at = position + cbHeader;
    if (d.u16(position + 6) == 0x66) at += 1 + d.u8(at);   // (the name of a linked file)
    const qint64 end = position + lcb;
    if (at + 8 > end) return false;
    const QByteArray rec = d.bytes(at, end - at);
    // The size it is shown at: its own size in twips, scaled in thousandths.
    const double goalW = d.u16(position + 28), goalH = d.u16(position + 30), mx = d.u16(position + 32), my = d.u16(position + 34);
    out->width = goalW * mx / 1000.0 / 20.0;
    out->height = goalH * my / 1000.0 / 20.0;
    QByteArray blip;
    for (const OdrawStoreEntry &e : odrawStore(rec)) {
        blip = e.embedded;
        if (blip.isEmpty() && e.delayOffset) blip = blipAt(wd, e.delayOffset);
        if (!blip.isEmpty()) break;
    }
    if (blip.isEmpty()) {
        // A shape that has no picture in the record: a rule or a box (or a picture the drawing's own store has).
        const QVector<OdrawRecord> first = odrawRecords(rec);
        if (first.isEmpty() || first.first().type != 0xF004) return false;
        out->shape = rec;
        out->width = std::clamp(out->width, 0.0, 1584.0);
        out->height = std::clamp(out->height, 0.0, 1584.0);
        return true;
    }
    QString format;
    out->bytes = odrawBlip(blip, &format, &m_pictureWork);
    if (out->bytes.isEmpty() || out->bytes.size() > kMaxPictureBytes) return false;
    out->extension = format == QLatin1String("jpeg") ? QStringLiteral("jpeg") : format;
    if (out->extension.isEmpty()) return false;
    cropOf(rec, &out->cropL, &out->cropT, &out->cropR, &out->cropB);
    if (out->width < 1 || out->height < 1) {
        // (Its own size, from its header: the image is not decoded, which for a small file could take a great deal of memory and time.)
        QSize size;
        if (out->extension != QLatin1String("emf") && out->extension != QLatin1String("wmf")) {
            QBuffer buffer(&out->bytes);
            if (buffer.open(QIODevice::ReadOnly)) size = QImageReader(&buffer).size();
        }
        if (size.isValid() && size.width() > 0 && size.height() > 0) {
            out->width = size.width() * 0.75;
            out->height = size.height() * 0.75;
        } else {
            out->width = out->height = 72;
        }
    }
    out->width = std::clamp(out->width, 1.0, 1584.0);
    out->height = std::clamp(out->height, 1.0, 1584.0);
    return true;
}

QString Converter::addMedia(const QString &part, const Picture &pic, const QString &key)
{
    // One media part for each picture, and a relationship to it from each part that shows it.
    QString media = m_mediaOfKey.value(key);
    if (media.isEmpty()) {
        media = QStringLiteral("/word/media/image%1.%2").arg(++m_mediaCount).arg(pic.extension);
        static const QHash<QString, QString> types = {{"png", "image/png"}, {"jpeg", "image/jpeg"}, {"emf", "image/x-emf"}, {"wmf", "image/x-wmf"}, {"bmp", "image/bmp"}, {"tiff", "image/tiff"}};
        pkg.addPart(media, pic.bytes, types.value(pic.extension, QStringLiteral("application/octet-stream")));
        m_mediaOfKey.insert(key, media);
    }
    const QString relKey = part + QLatin1Char('|') + media;
    QString rid = m_relOfMedia.value(relKey);
    if (rid.isEmpty()) {
        rid = pkg.addRelationship(part, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"), media);
        m_relOfMedia.insert(relKey, rid);
    }
    return rid;
}

QByteArray Converter::pictureXml(qint32 position, const QString &part)
{
    if (m_pictures >= kMaxPictures) {
        left(QStringLiteral("pictures"), QCoreApplication::translate("jp::office::doc", "Some pictures could not be shown, and were left out."));
        return QByteArray();
    }
    ++m_pictures;
    // (A picture that many characters point to is read once.)
    auto cached = m_pictureAt.constFind(position);
    if (cached == m_pictureAt.constEnd()) {
        // (The pictures held are counted as they are read, and one that would take them past the limit is not kept.)
        Picture read;
        if (m_pictureBytes > kMaxPictureTotal || !readPicture(position, &read) || m_pictureBytes + read.bytes.size() > kMaxPictureTotal) read = Picture();
        m_pictureBytes += read.bytes.size();
        cached = m_pictureAt.insert(position, read);
    }
    if (cached->bytes.isEmpty() && cached->shape.isEmpty()) {
        left(QStringLiteral("pictures"), QCoreApplication::translate("jp::office::doc", "Some pictures could not be shown, and were left out."));
        return QByteArray();
    }
    const Picture &pic = cached.value();
    if (pic.bytes.isEmpty()) return inlineShapeXml(pic, part);
    return drawingXml(pic, addMedia(part, pic, QStringLiteral("d%1").arg(position)));
}

QByteArray Converter::drawingXml(const Picture &pic, const QString &rid)
{
    const qint64 cx = qint64(pic.width * 12700), cy = qint64(pic.height * 12700);
    const QByteArray id = QByteArray::number(++m_drawingId);
    auto pct = [](double v) { return QByteArray::number(qint64(std::clamp(v, -1.0, 1.0) * 100000)); };
    QByteArray x = "<w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"" + QByteArray::number(cx) + "\" cy=\"" + QByteArray::number(cy) +
                   "\"/><wp:docPr id=\"" + id + "\" name=\"Picture " + id + "\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic>"
                   "<pic:nvPicPr><pic:cNvPr id=\"" + id + "\" name=\"Picture " + id + "\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"" + rid.toUtf8() + "\"/>";
    if (pic.cropL || pic.cropT || pic.cropR || pic.cropB)
        x += "<a:srcRect l=\"" + pct(pic.cropL) + "\" t=\"" + pct(pic.cropT) + "\" r=\"" + pct(pic.cropR) + "\" b=\"" + pct(pic.cropB) + "\"/>";
    x += "<a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(cx) + "\" cy=\"" + QByteArray::number(cy) +
         "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>";
    return x;
}


} // namespace jp::office::doc
