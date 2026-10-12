#include "office/xml.h"

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <cmath>

namespace jp::office {

namespace {

// Deeper than any real file; see parseXml.
const int kMaxDepth = 1000;

// Which standard's namespace a URI is, strict or transitional alike: 0 if it
// is none of ours.
int family(const QString &uri)
{
    if (uri == ns::A || uri == ns::AStrict) return 1;
    if (uri == ns::P || uri == ns::PStrict) return 2;
    if (uri == ns::R || uri == ns::RStrict) return 3;
    return 0;
}

bool matches(const QDomElement &e, const QString &ns, const QString &local)
{
    return e.localName() == local && sameNamespace(e.namespaceURI(), ns);
}

// The attribute of that local name in that namespace (empty for none). Not
// the DOM's own lookup by name, which would find a namespaced attribute
// ("r:id") for a plain one ("id").
bool findAttr(const QDomElement &e, const QString &ns, const QString &local, QString *value)
{
    const QDomNamedNodeMap attributes = e.attributes();
    for (int i = 0; i < attributes.length(); ++i) {
        const QDomAttr a = attributes.item(i).toAttr();
        if (a.localName() == local && sameNamespace(a.namespaceURI(), ns)) {
            *value = a.value();
            return true;
        }
    }
    return false;
}

// A number as the schema writes one: digits with a sign and a decimal point
// at most; no exponent, no "inf", no spaces.
bool plainNumber(const QString &s, double *value)
{
    int digits = 0, points = 0;
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s[i];
        if (c.isDigit() && c.unicode() < 128) ++digits;
        else if (c == QLatin1Char('.') && ++points == 1) continue;
        else if ((c == QLatin1Char('-') || c == QLatin1Char('+')) && i == 0) continue;
        else return false;
    }
    if (digits == 0) return false;
    bool ok = false;
    *value = s.toDouble(&ok);
    return ok;
}

double finish(bool good, double value, double fallback, bool *ok)
{
    if (ok) *ok = good;
    return good ? value : fallback;
}

} // namespace

bool sameNamespace(const QString &a, const QString &b)
{
    const int fa = family(a);
    return fa == 0 ? a == b : fa == family(b);
}

bool parseXml(const QByteArray &xml, QDomDocument *doc, QString *error)
{
    auto fail = [&](const QString &message) {
        if (error) *error = message;
        return false;
    };
    // One quick pass for what the DOM would be slow or unsafe with: a DOCTYPE
    // (nothing here has a use for entities), and nesting deep enough to make
    // building the tree take minutes.
    QXmlStreamReader r(xml);
    int depth = 0;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isDTD()) return fail(QCoreApplication::translate("Xml", "The XML has a DOCTYPE, which is not allowed."));
        if (r.isStartElement() && ++depth > kMaxDepth) return fail(QCoreApplication::translate("Xml", "The XML is nested too deeply."));
        if (r.isEndElement()) --depth;
    }
    if (r.hasError()) return fail(r.errorString());
    QDomDocument::ParseOptions options(QDomDocument::ParseOption::UseNamespaceProcessing);
    options |= QDomDocument::ParseOption::PreserveSpacingOnlyNodes;
    const QDomDocument::ParseResult result = doc->setContent(xml, options);
    if (!result) return fail(result.errorMessage);
    return true;
}

QDomElement child(const QDomElement &e, const QString &ns, const QString &local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (matches(c, ns, local)) return c;
    return {};
}

QVector<QDomElement> children(const QDomElement &e, const QString &ns, const QString &local)
{
    QVector<QDomElement> out;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (matches(c, ns, local)) out << c;
    return out;
}

QVector<QDomElement> children(const QDomElement &e)
{
    QVector<QDomElement> out;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) out << c;
    return out;
}

bool isElement(const QDomElement &e, const QString &ns, const QString &local)
{
    return !e.isNull() && matches(e, ns, local);
}

QString attr(const QDomElement &e, const QString &local, const QString &fallback)
{
    QString value;
    return findAttr(e, QString(), local, &value) ? value : fallback;
}

bool hasAttr(const QDomElement &e, const QString &local)
{
    QString value;
    return findAttr(e, QString(), local, &value);
}

QString attrNs(const QDomElement &e, const QString &ns, const QString &local, const QString &fallback)
{
    QString value;
    return findAttr(e, ns, local, &value) ? value : fallback;
}

double emuToPoints(qint64 emu)
{
    return double(emu) / double(EmuPerPoint);
}

qint64 pointsToEmu(double points)
{
    return qint64(std::llround(points * double(EmuPerPoint)));
}

double parseLength(const QString &s, double fallback, bool *ok)
{
    static const struct { QLatin1String unit; double points; } units[] = {
        {QLatin1String("pt"), 1}, {QLatin1String("in"), 72}, {QLatin1String("cm"), 72 / 2.54},
        {QLatin1String("mm"), 72 / 25.4}, {QLatin1String("pc"), 12}, {QLatin1String("pi"), 12}};
    const QString t = s.trimmed();
    double v = 0;
    for (const auto &u : units)
        if (t.endsWith(u.unit)) {
            const bool good = plainNumber(t.chopped(2), &v);
            return finish(good, v * u.points, fallback, ok);
        }
    const bool good = plainNumber(t, &v);
    return finish(good, v / double(EmuPerPoint), fallback, ok);
}

double parsePercent(const QString &s, double fallback, bool *ok)
{
    const QString t = s.trimmed();
    double v = 0;
    if (t.endsWith(QLatin1Char('%'))) {
        const bool good = plainNumber(t.chopped(1), &v);
        return finish(good, v / 100, fallback, ok);
    }
    const bool good = plainNumber(t, &v);
    return finish(good, v / 100000, fallback, ok);
}

double parseAngle(const QString &s, double fallback, bool *ok)
{
    double v = 0;
    const bool good = plainNumber(s.trimmed(), &v);
    return finish(good, v / 60000, fallback, ok);
}

bool parseBool(const QString &s, bool fallback, bool *ok)
{
    const QString t = s.trimmed().toLower();
    if (t == QLatin1String("1") || t == QLatin1String("true") || t == QLatin1String("on")) {
        if (ok) *ok = true;
        return true;
    }
    if (t == QLatin1String("0") || t == QLatin1String("false") || t == QLatin1String("off")) {
        if (ok) *ok = true;
        return false;
    }
    if (ok) *ok = false;
    return fallback;
}

qint64 parseInt(const QString &s, qint64 fallback, bool *ok)
{
    bool good = false;
    const qint64 v = s.trimmed().toLongLong(&good);
    if (ok) *ok = good;
    return good ? v : fallback;
}

} // namespace jp::office
