#include "office/mathobject.h"

#include "mathml/mathml.h"

#include <QXmlStreamReader>

namespace jp::office {

namespace {

constexpr QLatin1String kMathmlUri("http://www.w3.org/1998/Math/MathML");

// A document that is MathML alone and runs nothing: its root is `math`, no element is one that runs or loads, and no attribute is a link or an event handler.
bool safeMathml(const QByteArray &xml)
{
    QXmlStreamReader r(xml);
    bool first = true;
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType token = r.readNext();
        if (token == QXmlStreamReader::DTD) return false;
        if (token != QXmlStreamReader::StartElement) continue;
        const QString name = r.name().toString().toLower();
        if (first) {
            if (name != QLatin1String("math")) return false;
            const QString uri = r.namespaceUri().toString();
            if (!uri.isEmpty() && uri != kMathmlUri) return false;
            first = false;
        }
        static const QStringList loud = {QStringLiteral("script"), QStringLiteral("iframe"), QStringLiteral("object"), QStringLiteral("embed"), QStringLiteral("foreignobject"),
                                         QStringLiteral("maction"), QStringLiteral("a"), QStringLiteral("link"), QStringLiteral("applet")};
        if (loud.contains(name)) return false;
        const QXmlStreamAttributes attrs = r.attributes();
        for (const QXmlStreamAttribute &a : attrs) {
            const QString local = a.name().toString().toLower();
            if (local == QLatin1String("href") || local == QLatin1String("src") || local.startsWith(QLatin1String("on"))) return false;
        }
    }
    return !first && !r.hasError();
}

} // namespace

QByteArray mathObjectToOmml(const QByteArray &contentXml)
{
    if (contentXml.isEmpty() || contentXml.size() > mathml::kMostBytes || !safeMathml(contentXml)) return QByteArray();
    return mathml::ommlFromMathml(contentXml);
}

QByteArray mathObjectFromOmml(const QByteArray &markup)
{
    if (markup.isEmpty()) return QByteArray();
    return mathml::mathmlFromOmml(markup);
}

} // namespace jp::office
