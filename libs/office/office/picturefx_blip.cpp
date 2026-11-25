#include "office/picturefx_blip.h"

#include "core/blipeffects.h"
#include "core/picturefx.h"
#include "office/drawingml_write_util.h"

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace jp::office {

namespace {

QString xmlEscaped(const QString &s)
{
    QString out = s;
    out.replace(QLatin1Char('&'), QStringLiteral("&amp;")).replace(QLatin1Char('<'), QStringLiteral("&lt;")).replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    return out;
}

// The first blip: where its start tag and its end are in the text.
struct Spot {
    qsizetype start = -1, startEnd = -1, end = -1;   // [start, startEnd) the start tag; [startEnd, innerEnd) the inside; end after the end tag (startEnd == end when it closes itself)
    qsizetype innerEnd = -1;
    bool selfClosing = false;
};

bool findBlip(const QString &xml, Spot *out)
{
    static const QRegularExpression open(QStringLiteral("<a:blip(?=[\\s/>])[^>]*>"));
    const QRegularExpressionMatch m = open.match(xml);
    if (!m.hasMatch()) return false;
    out->start = m.capturedStart();
    out->startEnd = m.capturedEnd();
    out->selfClosing = xml.at(out->startEnd - 2) == QLatin1Char('/');
    if (out->selfClosing) {
        out->innerEnd = out->end = out->startEnd;
        return true;
    }
    const qsizetype close = xml.indexOf(QStringLiteral("</a:blip>"), out->startEnd);
    if (close < 0) return false;
    out->innerEnd = close;
    out->end = close + 9;
    return true;
}

QString embedOf(const QString &tag)
{
    static const QRegularExpression embed(QStringLiteral("\\br:embed=\"([^\"]*)\""));
    const QRegularExpressionMatch m = embed.match(tag);
    return m.hasMatch() ? m.captured(1) : QString();
}

// The extension of ours inside a blip's inside: where it is, and the filters element in it.
bool findExtension(const QString &inner, qsizetype *from, qsizetype *to)
{
    const QString uri = QStringLiteral("<a:ext uri=\"") + QString::fromLatin1(dmlwrite::kPictureFxExtension) + QStringLiteral("\"");
    const qsizetype at = inner.indexOf(uri);
    if (at < 0) return false;
    const qsizetype close = inner.indexOf(QStringLiteral("</a:ext>"), at);
    if (close < 0) return false;
    *from = at;
    *to = close + 8;
    return true;
}

} // namespace

QString pictureFxData(const QVector<BlipEffect> &effects)
{
    return QString::fromLatin1(QJsonDocument(QJsonObject{{"effects", blipEffectsToJson(effects)}}).toJson(QJsonDocument::Compact).toBase64());
}

QVector<BlipEffect> pictureFxFromData(const QString &data)
{
    if (data.isEmpty() || data.size() > kMostPictureFxDataBytes) return {};
    return blipFilters(blipEffectsFromJson(QJsonDocument::fromJson(QByteArray::fromBase64(data.toLatin1())).object()["effects"].toArray()));
}

bool readPictureFxBlip(const QByteArray &xml, PictureFxBlip *out)
{
    const QString text = QString::fromUtf8(xml);
    Spot s;
    if (!findBlip(text, &s)) return false;
    *out = PictureFxBlip();
    out->shownRid = out->originalRid = embedOf(text.mid(s.start, s.startEnd - s.start));
    const QString inner = text.mid(s.startEnd, s.innerEnd - s.startEnd);
    qsizetype from = 0, to = 0;
    if (!findExtension(inner, &from, &to)) return true;
    const QString ext = inner.mid(from, to - from);
    static const QRegularExpression data(QStringLiteral("\\bdata=\"([^\"]*)\""));
    const QRegularExpressionMatch d = data.match(ext);
    const QVector<BlipEffect> listed = d.hasMatch() ? pictureFxFromData(d.captured(1)) : QVector<BlipEffect>();
    const QString original = embedOf(ext);
    if (!blipEffectsChange(listed) || original.isEmpty()) return true;
    out->originalRid = original;
    out->filters = listed;
    out->present = true;
    return true;
}

QByteArray withPictureFxBlip(const QByteArray &xml, const QString &shownRid, const QString &originalRid, const QVector<BlipEffect> &filters)
{
    QString text = QString::fromUtf8(xml);
    Spot s;
    if (!findBlip(text, &s)) return xml;
    QString start = text.mid(s.start, s.startEnd - s.start);
    QString inner = text.mid(s.startEnd, s.innerEnd - s.startEnd);
    static const QRegularExpression embed(QStringLiteral("\\br:embed=\"[^\"]*\""));
    const QString shown = QStringLiteral("r:embed=\"") + xmlEscaped(shownRid) + QStringLiteral("\"");
    if (embed.match(start).hasMatch()) start.replace(embed, shown);
    else start.insert(start.size() - (s.selfClosing ? 2 : 1), QLatin1Char(' ') + shown);
    if (s.selfClosing) start.chop(2), start += QLatin1Char('>');
    qsizetype from = 0, to = 0;
    if (findExtension(inner, &from, &to)) inner.remove(from, to - from);
    // (An extension list that held only ours is taken off with it.)
    static const QRegularExpression emptyList(QStringLiteral("<a:extLst\\s*/>|<a:extLst>\\s*</a:extLst>"));
    inner.remove(emptyList);
    if (blipEffectsChange(filters)) {
        const QString ext = QStringLiteral("<a:ext uri=\"") + QString::fromLatin1(dmlwrite::kPictureFxExtension) + QStringLiteral("\"><jfx:filters xmlns:jfx=\"") + QString::fromLatin1(dmlwrite::kPictureFxNamespace)
                            + QStringLiteral("\" r:embed=\"") + xmlEscaped(originalRid) + QStringLiteral("\" data=\"") + pictureFxData(filters) + QStringLiteral("\"/></a:ext>");
        const qsizetype listEnd = inner.lastIndexOf(QStringLiteral("</a:extLst>"));
        if (listEnd >= 0) inner.insert(listEnd, ext);
        else inner += QStringLiteral("<a:extLst>") + ext + QStringLiteral("</a:extLst>");
    }
    // (A blip with nothing inside closes itself, as it did before the extension was put in it.)
    const QString end = inner.isEmpty() ? QString() : QStringLiteral("</a:blip>");
    if (inner.isEmpty()) {
        start.chop(1);
        start += QStringLiteral("/>");
    }
    text.replace(s.start, s.end - s.start, start + inner + end);
    return text.toUtf8();
}

const char *pictureFxExtensionUri() { return dmlwrite::kPictureFxExtension; }

QString pictureFxKeyFor(const QByteArray &originalBytes)
{
    return QStringLiteral("fx|%1|%2").arg(originalBytes.size()).arg(qHash(originalBytes));
}

QByteArray pictureFxFinishedPng(const QImage &original, const QVector<BlipEffect> &filters, const QString &sourceKey, const std::atomic<bool> *stop)
{
    if (original.isNull()) return {};
    const QImage finished = blipAdjusted(original, filters, stop, sourceKey);
    if (stop && stop->load()) return {};
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    return finished.save(&buffer, "PNG") ? png : QByteArray();
}

} // namespace jp::office
