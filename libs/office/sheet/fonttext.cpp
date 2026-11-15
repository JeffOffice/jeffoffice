#include "sheet/fonttext.h"

#include "sheet/xmlscan.h"

#include <QRegularExpression>

namespace jp::office::sheet {

namespace {

// `typeface="X"` of the letters' fonts, and `val="X"` of a rich run's `rFont` (with any prefix).
const QRegularExpression &element()
{
    static const QRegularExpression re(QStringLiteral("<(?:[A-Za-z0-9]+:)?(?:latin|ea|cs|rFont)\\b[^>]*>"));
    return re;
}
const QRegularExpression &attribute()
{
    static const QRegularExpression re(QStringLiteral("(\\b(?:typeface|val))=\"([^\"]*)\""));
    return re;
}

} // namespace

bool partNamesFontsInText(const QString &partName)
{
    const QString n = partName.toLower();
    if (!n.endsWith(QLatin1String(".xml"))) return false;
    return n.endsWith(QLatin1String("/sharedstrings.xml")) || n.contains(QLatin1String("/drawings/drawing")) || n.contains(QLatin1String("/charts/chart")) || n.contains(QLatin1String("/comments"));
}

void countFontsInText(const QByteArray &xml, QMap<QString, qint64> *counts)
{
    const QString text = QString::fromUtf8(xml);
    auto it = element().globalMatch(text);
    while (it.hasNext()) {
        const QString tag = it.next().captured(0);
        const bool rFont = tag.contains(QLatin1String("rFont"));
        auto slots = attribute().globalMatch(tag);
        while (slots.hasNext()) {
            const auto a = slots.next();
            if ((a.captured(1) == QLatin1String("val")) != rFont) continue;
            const QString name = xmlscan::unescape(a.captured(2).toUtf8());
            if (!name.isEmpty() && !name.startsWith(QLatin1Char('+'))) ++(*counts)[name];
        }
    }
}

QByteArray renameFontsInText(const QByteArray &xml, const QString &from, const QString &to, bool *changedOut)
{
    if (changedOut) *changedOut = false;
    const QString text = QString::fromUtf8(xml);
    QString out;
    qsizetype last = 0;
    bool changed = false;
    auto it = element().globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        const QString tag = m.captured(0);
        const bool rFont = tag.contains(QLatin1String("rFont"));
        auto slots = attribute().globalMatch(tag);
        QString rewritten;
        qsizetype taken = 0;
        while (slots.hasNext()) {
            const auto a = slots.next();
            if ((a.captured(1) == QLatin1String("val")) != rFont) continue;
            const QString name = xmlscan::unescape(a.captured(2).toUtf8());
            if (name.compare(from, Qt::CaseInsensitive) != 0 || name == to) continue;
            rewritten += tag.mid(taken, a.capturedStart() - taken) + a.captured(1) + QStringLiteral("=\"") + QString::fromUtf8(xmlscan::escapeAttribute(to)) + QLatin1Char('"');
            taken = a.capturedEnd();
        }
        if (taken == 0) continue;
        rewritten += tag.mid(taken);
        out += text.mid(last, m.capturedStart() - last) + rewritten;
        last = m.capturedEnd();
        changed = true;
    }
    if (!changed) return xml;
    out += text.mid(last);
    if (changedOut) *changedOut = true;
    return out.toUtf8();
}

} // namespace jp::office::sheet
