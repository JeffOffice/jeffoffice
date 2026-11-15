#include "core/printheaderfooter.h"

#include <QJsonObject>

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

} // namespace jp::deck
