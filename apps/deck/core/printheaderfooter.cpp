#include "core/printheaderfooter.h"

#include <QJsonObject>
#include <QByteArray>
#include <QJsonArray>

namespace jp::deck {

namespace {
const QString kKey = QStringLiteral("printHeaderFooter");
QString keyOf(PrintedPages pages) { return pages == PrintedPages::Notes ? QStringLiteral("notes") : QStringLiteral("handouts"); }
} // namespace

bool hasPrintHeaderFooter(const Document &d, PrintedPages pages) { return d.appData.value(kKey).toObject().contains(keyOf(pages)); }

PrintHeaderFooter printHeaderFooter(const Document &d, PrintedPages pages)
{
    PrintHeaderFooter hf;
    if (!hasPrintHeaderFooter(d, pages)) {
        if (pages == PrintedPages::Handouts) {
            hf.date = true;
            hf.header = !d.props.title.isEmpty();
            hf.headerText = d.props.title;
        }
        return hf;
    }
    const QJsonObject o = d.appData.value(kKey).toObject().value(keyOf(pages)).toObject();
    hf.date = o.value(QStringLiteral("date")).toBool();
    hf.dateText = o.value(QStringLiteral("dateText")).toString();
    hf.header = o.value(QStringLiteral("header")).toBool();
    hf.headerText = o.value(QStringLiteral("headerText")).toString();
    hf.footer = o.value(QStringLiteral("footer")).toBool();
    hf.footerText = o.value(QStringLiteral("footerText")).toString();
    hf.pageNumber = o.value(QStringLiteral("pageNumber")).toBool(true);
    return hf;
}

void setPrintHeaderFooter(Document &d, PrintedPages pages, const PrintHeaderFooter &hf)
{
    QJsonObject all = d.appData.value(kKey).toObject();
    all[keyOf(pages)] = QJsonObject{{QStringLiteral("date"), hf.date},
                                    {QStringLiteral("dateText"), hf.dateText},
                                    {QStringLiteral("header"), hf.header},
                                    {QStringLiteral("headerText"), hf.headerText},
                                    {QStringLiteral("footer"), hf.footer},
                                    {QStringLiteral("footerText"), hf.footerText},
                                    {QStringLiteral("pageNumber"), hf.pageNumber}};
    d.appData[kKey] = all;
}

namespace {
QJsonObject toJson(const PrintHeaderFooter &hf)
{
    return QJsonObject{{QStringLiteral("date"), hf.date},       {QStringLiteral("dateText"), hf.dateText},     {QStringLiteral("header"), hf.header},   {QStringLiteral("headerText"), hf.headerText},
                       {QStringLiteral("footer"), hf.footer},   {QStringLiteral("footerText"), hf.footerText}, {QStringLiteral("pageNumber"), hf.pageNumber}};
}
const QString kMasterKey = QStringLiteral("handoutMasterXml"), kReadKey = QStringLiteral("handoutMasterRead");
} // namespace

void keepHandoutMaster(Document &d, const QByteArray &xml, const PrintHeaderFooter *read)
{
    d.appData[kMasterKey] = QString::fromLatin1(xml.toBase64());
    if (read) d.appData[kReadKey] = toJson(*read);
    else d.appData.remove(kReadKey);
}

void keepHandoutRelationships(Document &d, const QJsonArray &rels) { d.appData[QStringLiteral("handoutMasterRels")] = rels; }
QJsonArray keptHandoutRelationships(const Document &d) { return d.appData.value(QStringLiteral("handoutMasterRels")).toArray(); }

QByteArray keptHandoutMaster(const Document &d) { return QByteArray::fromBase64(d.appData.value(kMasterKey).toString().toLatin1()); }

PrintHeaderFooter handoutMasterRead(const Document &d)
{
    Document scratch;
    scratch.appData[kKey] = QJsonObject{{keyOf(PrintedPages::Handouts), d.appData.value(kReadKey).toObject()}};
    return printHeaderFooter(scratch, PrintedPages::Handouts);
}

bool handoutMasterEdited(const Document &d)
{
    if (!hasPrintHeaderFooter(d, PrintedPages::Handouts)) return false;
    if (!d.appData.contains(kReadKey)) return true;
    return !(printHeaderFooter(d, PrintedPages::Handouts) == handoutMasterRead(d));
}

} // namespace jp::deck
