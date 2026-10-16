// The table styles a presentation file defines (pptxtablestyles.h).

#include "io/pptxtablestyles.h"

#include "office/tablestyles.h"
#include "office/xml.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

namespace jp::deck {

namespace {

QString say(const char *text) { return QCoreApplication::translate("PptxTableStyles", text); }

const QString kKey = QStringLiteral("tableStyles");
constexpr int kMaxStyles = 64;
constexpr qsizetype kMaxStyleBytes = 32 * 1024, kMaxTotalBytes = 256 * 1024;

bool isGuid(const QString &id)
{
    static const QRegularExpression guid(QStringLiteral("^\\{[0-9A-Fa-f]{8}(-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}\\}$"));
    return guid.match(id).hasMatch();
}

} // namespace

void readTableStyleDefs(const QDomElement &list, const std::function<QString(const QDomElement &)> &standalone, Document &doc, QStringList *notes)
{
    doc.appData.remove(kKey);
    if (list.isNull()) return;
    QJsonArray styles;
    QSet<QString> seen;
    qsizetype bytes = 0;
    bool tooMany = false, tooLarge = false;
    for (QDomElement c = list.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (c.localName() != QLatin1String("tblStyle")) continue;
        const QString id = office::attr(c, QStringLiteral("styleId")).trimmed();
        if (id.isEmpty() || id.size() > 80 || seen.contains(id.toUpper())) continue;   // the first of an id is the one a table gets
        const QString xml = standalone(c);
        // A definition that points at a part of the package (a picture fill) has nothing to point at once it is written elsewhere.
        if (xml.contains(QLatin1String(" r:embed=")) || xml.contains(QLatin1String(" r:link=")) || xml.contains(QLatin1String(" r:id="))) continue;
        if (styles.size() >= kMaxStyles) {
            tooMany = true;
            break;
        }
        if (xml.size() > kMaxStyleBytes || bytes + xml.size() > kMaxTotalBytes) {
            tooLarge = true;
            continue;
        }
        bytes += xml.size();
        seen.insert(id.toUpper());
        styles.append(QJsonObject{{"id", id}, {"xml", xml}});
    }
    const QString def = office::attr(list, QStringLiteral("def")).trimmed();
    if (styles.isEmpty() && !isGuid(def)) return;
    QJsonObject kept{{"styles", styles}};
    if (isGuid(def)) kept["def"] = def;
    doc.appData[kKey] = kept;
    if (notes && (tooMany || tooLarge)) *notes << say("Some of the presentation's table styles are too many or too large to keep, so tables that use them take the usual style when they are changed.");
}

office::TableStyleDefs tableStyleDefsOf(const Document &doc)
{
    office::TableStyleDefs out;
    for (const QJsonValue &v : doc.appData.value(kKey).toObject().value(QStringLiteral("styles")).toArray()) {
        const QJsonObject o = v.toObject();
        const QString id = o.value(QStringLiteral("id")).toString(), xml = o.value(QStringLiteral("xml")).toString();
        if (!id.isEmpty() && !xml.isEmpty() && !out.contains(id.toUpper())) out.insert(id.toUpper(), xml.toUtf8());
    }
    return out;
}

QString tableStylesPartXml(const Document &doc)
{
    const QJsonObject kept = doc.appData.value(kKey).toObject();
    QString def = kept.value(QStringLiteral("def")).toString();
    if (!isGuid(def)) def = office::defaultTableStyleId();
    QString xml = QStringLiteral("<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" def=\"%1\"").arg(def);
    QString inner;
    for (const QJsonValue &v : kept.value(QStringLiteral("styles")).toArray()) {
        const QString style = v.toObject().value(QStringLiteral("xml")).toString();
        // Only what still reads as one element of this part goes back.
        QDomDocument dom;
        if (!style.isEmpty() && office::parseXml(style.toUtf8(), &dom) && dom.documentElement().localName() == QLatin1String("tblStyle")) inner += style;
    }
    return inner.isEmpty() ? xml + QStringLiteral("/>") : xml + QLatin1Char('>') + inner + QStringLiteral("</a:tblStyleLst>");
}

} // namespace jp::deck
