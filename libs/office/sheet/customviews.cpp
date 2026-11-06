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

// The text as an attribute value: characters XML does not allow are taken out (a tab, a line end, or another control becomes a space).
QByteArray escaped(const QString &text)
{
    QString clean;
    clean.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        const ushort u = c.unicode();
        if (u < 0x20) clean += QLatin1Char(' ');
        else if (u == 0xFFFE || u == 0xFFFF) continue;
        else if (c.isHighSurrogate() && i + 1 < text.size() && text[i + 1].isLowSurrogate()) clean += text.mid(i++, 2);
        else if (c.isSurrogate()) continue;
        else clean += c;
    }
    return clean.toHtmlEscaped().replace(QLatin1Char('"'), QStringLiteral("&quot;")).toUtf8();
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

QVector<QPair<quint32, quint32>> spansFromText(const QString &text, quint32 lines, int most)
{
    QVector<QPair<quint32, quint32>> out;
    const qsizetype n = text.size();
    qsizetype at = 0;
    auto number = [&](qint64 *v) {
        qint64 x = 0;
        int digits = 0;
        while (at < n && text[at].unicode() >= '0' && text[at].unicode() <= '9') {
            if (digits < 12) x = x * 10 + (text[at].unicode() - '0');
            ++digits;
            ++at;
        }
        *v = digits > 12 ? Q_INT64_C(1) << 40 : x;
        return digits > 0;
    };
    while (at < n && out.size() < most) {
        while (at < n && text[at] == QLatin1Char(' ')) ++at;
        if (at >= n) break;
        qint64 a = 0, b = 0;
        bool ok = number(&a);
        b = a;
        if (ok && at < n && text[at] == QLatin1Char(':')) {
            ++at;
            ok = number(&b);
        }
        const bool endsRight = at >= n || text[at] == QLatin1Char(' ');
        while (at < n && text[at] != QLatin1Char(' ')) ++at;   // (what follows a span that is not one is skipped to the next space)
        if (!ok || !endsRight || a < 1 || b < a || a > qint64(lines)) continue;
        out.append({quint32(a - 1), quint32(std::min<qint64>(b, lines) - 1)});
    }
    return out;
}

int customViewSpanCount(const CustomView &view)
{
    int n = 0;
    for (const CustomViewSheet &s : view.sheets) n += int(s.hiddenRows.size() + s.hiddenCols.size());
    return n;
}

CustomViewForeign::CustomViewForeign() = default;
CustomViewForeign::~CustomViewForeign() = default;

namespace {
QByteArray plainEscape(const QString &text, bool attribute)
{
    QString t = text.toHtmlEscaped();
    if (attribute) t.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    return t.toUtf8();
}
}

bool CustomViewForeign::feed(const QXmlStreamReader &r)
{
    switch (r.tokenType()) {
    case QXmlStreamReader::StartElement:
        ++m_level;
        if (m_level == 2) {
            const QStringView uri = r.attributes().value(QLatin1String("uri"));
            m_copying = uri != QLatin1String(kCustomViewExtUri) && m_bytes.size() < 65536;
        }
        if (m_copying) {
            // (written by hand: the names as the file has them, and the namespaces the element declares)
            m_bytes += "<" + r.qualifiedName().toUtf8();
            for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations())
                m_bytes += (d.prefix().isEmpty() ? QByteArray(" xmlns=\"") : " xmlns:" + d.prefix().toUtf8() + "=\"") + plainEscape(d.namespaceUri().toString(), true) + "\"";
            for (const QXmlStreamAttribute &a : r.attributes()) m_bytes += " " + a.qualifiedName().toUtf8() + "=\"" + plainEscape(a.value().toString(), true) + "\"";
            m_bytes += ">";
        }
        break;
    case QXmlStreamReader::EndElement:
        if (m_level == 0) return true;
        if (m_copying) m_bytes += "</" + r.qualifiedName().toUtf8() + ">";
        if (m_level == 2) m_copying = false;
        --m_level;
        break;
    case QXmlStreamReader::Characters:
        if (m_copying && !r.isWhitespace()) m_bytes += plainEscape(r.text().toString(), false);
        break;
    default: break;
    }
    return false;
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

void renameCustomViewSheet(QVector<CustomView> *views, const QString &from, const QString &to)
{
    for (CustomView &v : *views) {
        if (v.activeSheet.compare(from, Qt::CaseInsensitive) == 0) v.activeSheet = to;
        for (CustomViewSheet &s : v.sheets)
            if (s.name.compare(from, Qt::CaseInsensitive) == 0) s.name = to;
    }
}

void dropCustomViewSheet(QVector<CustomView> *views, const QString &name)
{
    for (CustomView &v : *views) {
        if (v.activeSheet.compare(name, Qt::CaseInsensitive) == 0) v.activeSheet.clear();
        for (int i = int(v.sheets.size()) - 1; i >= 0; --i)
            if (v.sheets[i].name.compare(name, Qt::CaseInsensitive) == 0) v.sheets.removeAt(i);
    }
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
        if ((a.namespaceUri().isEmpty() && knownAttribute(n)) || view->otherAttributes.size() >= 32) continue;
        view->otherAttributes.append({a.qualifiedName().toString().left(64), a.value().toString().left(256)});
    }
    // (a prefix the view declares itself, for the attributes and the elements it keeps)
    for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations())
        if (!d.prefix().isEmpty() && view->otherAttributes.size() < 32) view->otherAttributes.append({QStringLiteral("xmlns:") + d.prefix().toString().left(32), d.namespaceUri().toString().left(256)});
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
    // (a sheet a view names twice is read once: the first entry)
    for (const CustomViewSheet &have : view->sheets)
        if (have.name.compare(s.name, Qt::CaseInsensitive) == 0) return;
    s.zoom = number(r, "zoom", 100, 10, 400);
    s.frozenRows = quint32(number(r, "frozenRows", 0, 0, int(kCustomViewRows) - 1));
    s.frozenCols = quint32(number(r, "frozenCols", 0, 0, int(kCustomViewColumns) - 1));
    if (s.frozenRows > kCustomViewMaxFrozenRows || s.frozenCols > kCustomViewMaxFrozenCols) s.frozenRows = s.frozenCols = 0;
    s.frozen = s.frozenRows || s.frozenCols;
    s.activeRow = quint32(number(r, "row", 0, 0, int(kCustomViewRows) - 1));
    s.activeCol = quint32(number(r, "col", 0, 0, int(kCustomViewColumns) - 1));
    const int room = std::max(0, kMaxCustomViewSpans - customViewSpanCount(*view));
    s.hiddenRows = normalizedSpans(spansFromText(r.attributes().value(QLatin1String("hiddenRows")).toString(), kCustomViewRows, room));
    s.hiddenCols = normalizedSpans(spansFromText(r.attributes().value(QLatin1String("hiddenCols")).toString(), kCustomViewColumns, std::max(0, room - int(s.hiddenRows.size()))));
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
        if (!v.hasState && v.foreignExt.isEmpty()) {
            e += "/>";
            continue;
        }
        e += "><" + prefix + "extLst>" + v.foreignExt;
        if (!v.hasState) {
            e += "</" + prefix + "extLst></" + prefix + "customWorkbookView>";
            continue;
        }
        e += "<" + prefix + "ext uri=\"" + kCustomViewExtUri + "\"><jocv:view xmlns:jocv=\"" + kCustomViewNs + "\" gridlines=\"" + (v.gridlines ? "1" : "0") + "\" headings=\""
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
