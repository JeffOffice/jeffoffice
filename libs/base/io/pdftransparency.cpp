#include "io/pdftransparency.h"

#include "io/qtpdf.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>

namespace jp {

namespace {

constexpr int kMostPages = 100000;

bool seeThroughState(const QString &dict)
{
    static const QRegularExpression re(QStringLiteral("/(?:ca|CA)\\s+(-?[0-9.]+)"));
    auto it = re.globalMatch(dict);
    while (it.hasNext())
        if (it.next().captured(1).toDouble() < 0.9995) return true;
    return false;
}

bool hasSoftMask(const QString &dict)
{
    static const QRegularExpression re(QStringLiteral("/SMask\\s+(?!/None)(\\d+ 0 R|<<)"));
    return re.match(dict).hasMatch();
}

// The names a resource dictionary gives to objects ("/Name N 0 R"), looking one level into the dictionaries it points to ("/ExtGState N 0 R").
void namesIn(const QHash<int, const QtPdf::Obj *> &byId, const QString &text, QHash<QString, int> &names, int depth)
{
    static const QRegularExpression re(QStringLiteral("/([A-Za-z0-9_.#-]+)\\s+(\\d+) 0 R"));
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        const int id = m.captured(2).toInt();
        names.insert(m.captured(1), id);
        if (depth > 0 && (m.captured(1) == QLatin1String("ExtGState") || m.captured(1) == QLatin1String("XObject")) && byId.contains(id))
            namesIn(byId, QString::fromLatin1(byId.value(id)->body), names, depth - 1);
    }
}

} // namespace

PdfTransparency pdfTransparency(const QByteArray &bytes)
{
    PdfTransparency out;
    QtPdf pdf;
    if (!pdf.loadBytes(bytes)) return out;
    QSet<int> bad;
    QHash<int, const QtPdf::Obj *> byId;
    for (const QtPdf::Obj &o : pdf.objects) {
        byId.insert(o.id, &o);
        const QString dict = QString::fromLatin1(QtPdf::dictOf(o.body));
        if (seeThroughState(dict) || hasSoftMask(dict)) bad.insert(o.id);
    }
    if (bad.isEmpty()) return out;
    out.found = true;
    int page = 0;
    static const QRegularExpression isPage(QStringLiteral("/Type\\s*/Page(?![a-zA-Z])"));
    static const QRegularExpression use(QStringLiteral("/([A-Za-z0-9_.#-]+)\\s+(?:gs|Do)\\b"));
    for (const QtPdf::Obj &o : pdf.objects) {
        const QByteArray dictBytes = QtPdf::dictOf(o.body);
        const QString dict = QString::fromLatin1(dictBytes);
        if (!isPage.match(dict).hasMatch()) continue;
        if (++page > kMostPages) break;
        QHash<QString, int> names;
        const int res = QtPdf::ref(dictBytes, "/Resources");
        if (res && byId.contains(res)) namesIn(byId, QString::fromLatin1(byId.value(res)->body), names, 1);
        else namesIn(byId, dict, names, 1);
        const int contents = QtPdf::ref(dictBytes, "/Contents");
        if (!contents || !byId.contains(contents)) continue;
        bool ok = false;
        const QByteArray stream = pdf.streamData(*byId.value(contents), &ok);
        if (!ok) continue;
        auto it = use.globalMatch(QString::fromLatin1(stream));
        while (it.hasNext()) {
            const int id = names.value(it.next().captured(1), 0);
            if (id && bad.contains(id)) {
                out.pages << page;
                break;
            }
        }
    }
    return out;
}

} // namespace jp
