#include "core/objectclip.h"

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
    if (c.chartXml.size() > kMaxObjectClipBytes) return false;
    if (!(c.width >= 4 && c.width <= 20000 && c.height >= 4 && c.height <= 20000)) return false;
    if (!c.valid()) return false;
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
