#include "office/mathobject.h"

#include "mathml/mathml.h"

#include "office/package.h"

#include <QIODevice>
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

MathObjectTestHooks &mathObjectTestHooks()
{
    static MathObjectTestHooks hooks;
    return hooks;
}

QByteArray peekPart(const Package &package, const QString &part)
{
    if (!package.hasPart(part)) return QByteArray();
    const std::unique_ptr<QIODevice> stream = package.openPart(part);
    return stream && stream->isOpen() ? stream->read(4096) : QByteArray();
}

namespace {

// Where the root element's name ends in the document (-1 if there is none): the place its attributes begin.
qsizetype rootNameEnd(const QByteArray &head)
{
    qsizetype i = head.startsWith("\xEF\xBB\xBF") ? 3 : 0;
    while (true) {
        i = head.indexOf('<', i);
        if (i < 0 || i + 1 >= head.size()) return -1;
        if (head[i + 1] == '?') i = head.indexOf("?>", i);
        else if (head.mid(i, 4) == "<!--") i = head.indexOf("-->", i);
        else if (head[i + 1] == '!') i = head.indexOf('>', i);
        else break;
        if (i < 0) return -1;
    }
    qsizetype end = i + 1;
    while (end < head.size() && !QByteArray(" \t\r\n/>").contains(head[end])) ++end;
    return end;
}

} // namespace

QByteArray mathObjectWithColor(const QByteArray &contentXml, const QColor &color)
{
    const qsizetype end = rootNameEnd(contentXml.left(4096));
    if (end < 0 || !color.isValid() || isMathObjectDocument(contentXml.left(4096)) == false) return contentXml;
    QByteArray out = contentXml;
    out.insert(end, " mathcolor=\"" + color.name(QColor::HexRgb).toLatin1() + "\"");
    return out;
}

QColor mathObjectColor(const QByteArray &contentXml)
{
    const QByteArray head = contentXml.left(4096);
    const qsizetype end = rootNameEnd(head);
    if (end < 0 || !isMathObjectDocument(head)) return QColor();
    const qsizetype tag = head.indexOf('>', end), at = head.indexOf(" mathcolor=\"#", end);
    if (at < 0 || (tag >= 0 && at > tag) || at + 19 > head.size()) return QColor();
    const QColor c(QString::fromLatin1(head.mid(at + 12, 7)));
    return c;
}

bool isMathObjectDocument(const QByteArray &head)
{
    qsizetype i = head.startsWith("\xEF\xBB\xBF") ? 3 : 0;
    while (true) {
        i = head.indexOf('<', i);
        if (i < 0 || i + 1 >= head.size()) return false;
        if (head[i + 1] == '?') {
            i = head.indexOf("?>", i);
        } else if (head.mid(i, 4) == "<!--") {
            i = head.indexOf("-->", i);
        } else if (head[i + 1] == '!') {
            i = head.indexOf('>', i);
        } else {
            break;
        }
        if (i < 0) return false;
    }
    qsizetype end = i + 1;
    while (end < head.size() && !QByteArray(" \t\r\n/>").contains(head[end])) ++end;
    const QByteArray name = head.mid(i + 1, end - i - 1);
    return (name.contains(':') ? name.mid(name.lastIndexOf(':') + 1) : name) == "math";
}

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
