#include "core/objectclip.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace jp::crunch {

QByteArray ObjectClip::toBytes() const
{
    QJsonObject o;
    o.insert(QStringLiteral("kind"), kind);
    o.insert(QStringLiteral("origin"), origin);
    o.insert(QStringLiteral("sheet"), sheetName);
    o.insert(QStringLiteral("width"), width);
    o.insert(QStringLiteral("height"), height);
    o.insert(QStringLiteral("chart"), QString::fromLatin1(chartXml.toBase64()));
    if (!elementXml.isEmpty()) {
        o.insert(QStringLiteral("element"), QString::fromLatin1(elementXml.toBase64()));
        o.insert(QStringLiteral("namespaces"), QString::fromLatin1(namespaces.toBase64()));
        QJsonArray list;
        for (const ClipMedia &m : media) list.append(QJsonObject{{QStringLiteral("token"), m.token}, {QStringLiteral("ext"), m.extension}, {QStringLiteral("type"), m.contentType}, {QStringLiteral("size"), double(m.bytes.size())}});
        o.insert(QStringLiteral("media"), list);
    }
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

bool ObjectClip::fromBytes(const QByteArray &bytes, ObjectClip *out)
{
    if (bytes.isEmpty() || bytes.size() > kMaxObjectClipBytes * 2) return false;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject()) return false;
    const QJsonObject o = doc.object();
    ObjectClip c;
    c.kind = o.value(QStringLiteral("kind")).toString();
    c.origin = o.value(QStringLiteral("origin")).toString().left(128);
    c.sheetName = o.value(QStringLiteral("sheet")).toString().left(256);
    c.width = o.value(QStringLiteral("width")).toDouble();
    c.height = o.value(QStringLiteral("height")).toDouble();
    c.chartXml = QByteArray::fromBase64(o.value(QStringLiteral("chart")).toString().toLatin1());
    c.elementXml = QByteArray::fromBase64(o.value(QStringLiteral("element")).toString().toLatin1());
    c.namespaces = QByteArray::fromBase64(o.value(QStringLiteral("namespaces")).toString().toLatin1());
    if (c.chartXml.size() > kMaxObjectClipBytes || c.elementXml.size() > kMaxClipElementBytes || c.namespaces.size() > 8192) return false;
    if (!(c.width >= 4 && c.width <= 20000 && c.height >= 4 && c.height <= 20000)) return false;
    const QJsonArray list = o.value(QStringLiteral("media")).toArray();
    if (list.size() > kMaxClipMedia) return false;
    for (const QJsonValue &v : list) {
        const QJsonObject m = v.toObject();
        ClipMedia media;
        media.token = m.value(QStringLiteral("token")).toString().left(64);
        media.extension = m.value(QStringLiteral("ext")).toString().left(8);
        media.contentType = m.value(QStringLiteral("type")).toString().left(64);
        if (media.token.isEmpty() || media.extension.isEmpty() || media.contentType.isEmpty()) return false;
        for (const QChar ch : media.token)
            if (!ch.isLetterOrNumber()) return false;
        for (const QChar ch : media.extension)
            if (!ch.isLetterOrNumber()) return false;
        c.media.append(media);
    }
    if (!c.valid()) return false;
    *out = c;
    return true;
}

void ObjectClip::toMime(QMimeData *md) const
{
    md->setData(QLatin1String(kObjectMime), toBytes());
    for (int i = 0; i < media.size(); ++i) md->setData(QStringLiteral("application/x-jeffcrunch-object-media-%1").arg(i), media[i].bytes);
}

bool ObjectClip::fromMime(const QMimeData *md, ObjectClip *out)
{
    if (!md || !md->hasFormat(QLatin1String(kObjectMime))) return false;
    ObjectClip c;
    if (!fromBytes(md->data(QLatin1String(kObjectMime)), &c)) return false;
    qint64 total = 0;
    for (int i = 0; i < c.media.size(); ++i) {
        const QString format = QStringLiteral("application/x-jeffcrunch-object-media-%1").arg(i);
        if (!md->hasFormat(format)) return false;
        c.media[i].bytes = md->data(format);
        total += c.media[i].bytes.size();
        if (c.media[i].bytes.isEmpty() || c.media[i].bytes.size() > kMaxClipMediaBytes || total > kMaxClipMediaTotal) return false;
    }
    *out = c;
    return true;
}

void dropReferences(jp::office::Chart *chart)
{
    for (jp::office::ChartPlot &p : chart->plots)
        for (jp::office::ChartSeries &s : p.series) {
            s.nameRef.clear();
            s.categoryRef.clear();
            s.valueRef.clear();
            s.xRef.clear();
        }
    chart->title.ref.clear();
    for (jp::office::ChartAxis &a : chart->axes) a.title.ref.clear();
}

bool ObjectClip::chart(const jp::office::Theme &theme, bool sameBook, jp::office::Chart *out) const
{
    jp::office::Chart c;
    if (!jp::office::readChart(chartXml, theme, jp::office::ColorMap(), &c)) return false;
    if (!sameBook) dropReferences(&c);
    *out = c;
    return true;
}

} // namespace jp::crunch
