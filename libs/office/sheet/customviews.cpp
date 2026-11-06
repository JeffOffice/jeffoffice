#include "sheet/customviews.h"

#include <QUuid>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp::office::sheet {

const char kCustomViewNs[] = "https://jeffoffice.com/xlsx/customview";
const char kCustomViewExtUri[] = "{7D5A2E1C-4B7B-4D55-9C2E-6A0F6C4B1E21}";

namespace {

bool flag(const QXmlStreamReader &r, const char *name, bool fallback)
{
    const QStringView v = r.attributes().value(QLatin1String(name));
    if (v.isEmpty()) return fallback;
    return v == QLatin1String("1") || v == QLatin1String("true");
}

int number(const QXmlStreamReader &r, const char *name, int fallback, int lo, int hi)
{
    bool ok = false;
    const int v = r.attributes().value(QLatin1String(name)).toInt(&ok);
    return ok ? std::clamp(v, lo, hi) : fallback;
}

QByteArray escaped(const QString &text)
{
    return text.toHtmlEscaped().replace(QLatin1Char('"'), QStringLiteral("&quot;")).toUtf8();
}

bool knownAttribute(const QString &n)
{
    static const QStringList known = {"name", "guid", "windowWidth", "windowHeight", "xWindow", "yWindow", "activeSheetId", "showFormulaBar"};
    return known.contains(n);
}

} // namespace

QString newCustomViewGuid()
{
    return QUuid::createUuid().toString(QUuid::WithBraces).toUpper();
}

QString spansToText(const QVector<QPair<quint32, quint32>> &spans)
{
    QStringList parts;
    for (const auto &s : spans) parts << (s.first == s.second ? QString::number(qint64(s.first) + 1) : QStringLiteral("%1:%2").arg(qint64(s.first) + 1).arg(qint64(s.second) + 1));
    return parts.join(QLatin1Char(' '));
}

QVector<QPair<quint32, quint32>> spansFromText(const QString &text)
{
    QVector<QPair<quint32, quint32>> out;
    const QStringList parts = text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString &p : parts) {
        if (out.size() >= kMaxCustomViewSpans) break;
        const QStringList ends = p.split(QLatin1Char(':'));
        bool ok1 = false, ok2 = true;
        const qint64 a = ends[0].toLongLong(&ok1);
        const qint64 b = ends.size() > 1 ? ends[1].toLongLong(&ok2) : a;
        if (ends.size() > 2 || !ok1 || !ok2 || a < 1 || b < a || b > 16777216) continue;
        out.append({quint32(a - 1), quint32(b - 1)});
    }
    return out;
}

QVector<QPair<quint32, quint32>> normalizedSpans(QVector<QPair<quint32, quint32>> spans)
{
    std::sort(spans.begin(), spans.end());
    QVector<QPair<quint32, quint32>> out;
    for (const auto &s : spans) {
        if (!out.isEmpty() && qint64(s.first) <= qint64(out.last().second) + 1) out.last().second = std::max(out.last().second, s.second);
        else out.append(s);
    }
    return out;
}

QVector<QPair<quint32, quint32>> spansWithout(const QVector<QPair<quint32, quint32>> &a, const QVector<QPair<quint32, quint32>> &b)
{
    QVector<QPair<quint32, quint32>> out;
    int j = 0;
    for (const auto &s : a) {
        qint64 from = s.first;
        const qint64 to = s.second;
        while (j < b.size() && qint64(b[j].second) < from) ++j;
        for (int k = j; k < b.size() && qint64(b[k].first) <= to; ++k) {
            if (qint64(b[k].first) > from) out.append({quint32(from), quint32(qint64(b[k].first) - 1)});
            from = std::max<qint64>(from, qint64(b[k].second) + 1);
        }
        if (from <= to) out.append({quint32(from), quint32(to)});
    }
    return out;
}

void readCustomViewAttributes(const QXmlStreamReader &r, CustomView *view)
{
    view->name = r.attributes().value(QLatin1String("name")).toString().left(kMaxCustomViewNameChars);
    view->guid = r.attributes().value(QLatin1String("guid")).toString().left(64);
    view->windowWidth = number(r, "windowWidth", 0, 0, 100000);
    view->windowHeight = number(r, "windowHeight", 0, 0, 100000);
    view->xWindow = number(r, "xWindow", 0, -100000, 100000);
    view->yWindow = number(r, "yWindow", 0, -100000, 100000);
    view->activeSheetId = number(r, "activeSheetId", 1, 0, 1000000);
    view->formulaBar = flag(r, "showFormulaBar", true);
    for (const QXmlStreamAttribute &a : r.attributes()) {
        const QString n = a.name().toString();
        if (!a.namespaceUri().isEmpty() || knownAttribute(n) || view->otherAttributes.size() >= 32) continue;
        view->otherAttributes.append({n, a.value().toString().left(256)});
    }
}

void readCustomViewState(const QXmlStreamReader &r, CustomView *view)
{
    view->hasState = true;
    view->gridlines = flag(r, "gridlines", true);
    view->headings = flag(r, "headings", true);
    view->activeSheet = r.attributes().value(QLatin1String("activeSheet")).toString().left(64);
}

void readCustomViewSheet(const QXmlStreamReader &r, CustomView *view)
{
    if (view->sheets.size() >= kMaxCustomViewSheets) return;
    CustomViewSheet s;
    s.name = r.attributes().value(QLatin1String("name")).toString().left(64);
    s.zoom = number(r, "zoom", 100, 10, 400);
    s.frozenRows = quint32(number(r, "frozenRows", 0, 0, 1048576));
    s.frozenCols = quint32(number(r, "frozenCols", 0, 0, 16384));
    s.frozen = s.frozenRows || s.frozenCols;
    s.activeRow = quint32(number(r, "row", 0, 0, 16777216));
    s.activeCol = quint32(number(r, "col", 0, 0, 1048576));
    s.hiddenRows = normalizedSpans(spansFromText(r.attributes().value(QLatin1String("hiddenRows")).toString()));
    s.hiddenCols = normalizedSpans(spansFromText(r.attributes().value(QLatin1String("hiddenCols")).toString()));
    view->sheets.append(s);
}

QByteArray customWorkbookViewsElement(const QVector<CustomView> &views, const QByteArray &prefix)
{
    if (views.isEmpty()) return QByteArray();
    QByteArray e = "<" + prefix + "customWorkbookViews>";
    for (const CustomView &v : views) {
        e += "<" + prefix + "customWorkbookView name=\"" + escaped(v.name) + "\" guid=\"" + escaped(v.guid) + "\"";
        for (const auto &a : v.otherAttributes) e += " " + a.first.toUtf8() + "=\"" + escaped(a.second) + "\"";
        e += " windowWidth=\"" + QByteArray::number(std::max(v.windowWidth, 1)) + "\" windowHeight=\"" + QByteArray::number(std::max(v.windowHeight, 1)) + "\"";
        if (v.xWindow) e += " xWindow=\"" + QByteArray::number(v.xWindow) + "\"";
        if (v.yWindow) e += " yWindow=\"" + QByteArray::number(v.yWindow) + "\"";
        e += " activeSheetId=\"" + QByteArray::number(v.activeSheetId) + "\"";
        if (!v.formulaBar) e += " showFormulaBar=\"0\"";
        if (!v.hasState) {
            e += "/>";
            continue;
        }
        e += "><" + prefix + "extLst><" + prefix + "ext uri=\"" + kCustomViewExtUri + "\"><jocv:view xmlns:jocv=\"" + kCustomViewNs + "\" gridlines=\"" + (v.gridlines ? "1" : "0") + "\" headings=\""
            + (v.headings ? "1" : "0") + "\" activeSheet=\"" + escaped(v.activeSheet) + "\">";
        for (const CustomViewSheet &s : v.sheets) {
            e += "<jocv:sheet name=\"" + escaped(s.name) + "\" zoom=\"" + QByteArray::number(s.zoom) + "\" frozenRows=\"" + QByteArray::number(s.frozenRows) + "\" frozenCols=\"" + QByteArray::number(s.frozenCols)
                + "\" row=\"" + QByteArray::number(s.activeRow) + "\" col=\"" + QByteArray::number(s.activeCol) + "\"";
            if (!s.hiddenRows.isEmpty()) e += " hiddenRows=\"" + spansToText(s.hiddenRows).toUtf8() + "\"";
            if (!s.hiddenCols.isEmpty()) e += " hiddenCols=\"" + spansToText(s.hiddenCols).toUtf8() + "\"";
            e += "/>";
        }
        e += "</jocv:view></" + prefix + "ext></" + prefix + "extLst></" + prefix + "customWorkbookView>";
    }
    return e + "</" + prefix + "customWorkbookViews>";
}

} // namespace jp::office::sheet
