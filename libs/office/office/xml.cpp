#include "office/xml.h"

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QHash>
#include <QPair>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <string>

namespace jp::office {

namespace {

QString escapedXml(const QString &text, bool attribute)
{
    QString out;
    out.reserve(text.size());
    for (const QChar c : text) {
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (attribute && c == QLatin1Char('"')) out += QLatin1String("&quot;");
        else if (attribute && c == QLatin1Char('\n')) out += QLatin1String("&#10;");
        else if (attribute && c == QLatin1Char('\t')) out += QLatin1String("&#9;");
        else out += c;
    }
    return out;
}

QString qualifiedName(const QString &prefix, const QString &local) { return prefix.isEmpty() ? local : prefix + QLatin1Char(':') + local; }

// The element's attributes in a fixed order: declarations of namespaces first, then by qualified name. (The DOM keeps them in a hash, whose order changes from run to run, and
// what is saved must not.)
QVector<QDomAttr> sortedAttributes(const QDomElement &e)
{
    const QDomNamedNodeMap map = e.attributes();
    QVector<QPair<QString, QDomAttr>> keyed;
    for (int i = 0; i < map.length(); ++i) {
        const QDomAttr a = map.item(i).toAttr();
        const QString name = qualifiedName(a.prefix(), a.localName());
        keyed.append({(name == QLatin1String("xmlns") || a.prefix() == QLatin1String("xmlns") ? QStringLiteral("0") : QStringLiteral("1")) + name, a});
    }
    std::sort(keyed.begin(), keyed.end(), [](const auto &x, const auto &y) { return x.first < y.first; });
    QVector<QDomAttr> out;
    for (const auto &k : keyed) out.append(k.second);
    return out;
}

// Every prefix the element and what is in it use, with its namespace.
void collectPrefixes(const QDomElement &e, QHash<QString, QString> *known, int depth = 0)
{
    if (depth > 256) return;
    if (!e.prefix().isEmpty()) known->insert(e.prefix(), e.namespaceURI());
    for (const QDomAttr &a : sortedAttributes(e))
        if (!a.prefix().isEmpty()) known->insert(a.prefix(), a.namespaceURI());
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) collectPrefixes(c, known, depth + 1);
}

// Writes the element and what is in it, declaring each namespace where it is
// first used (`scope`: the prefixes declared so far, "" the default). The
// prefixes markup compatibility names in attribute values (mc:Choice's
// Requires, mc:Ignorable) are declared on that element too, from `known`:
// readers look them up there, and a file whose Requires names an undeclared
// prefix is refused whole.
void writeElement(const QDomElement &e, QString *out, QHash<QString, QString> *scope, const QHash<QString, QString> &known)
{
    QVector<QPair<QString, QString>> declared;
    auto need = [&](const QString &prefix, const QString &uri) {
        const auto it = scope->constFind(prefix);
        const QString current = it == scope->cend() ? QString() : it.value();
        if (current == uri) return;
        for (const auto &d : declared)
            if (d.first == prefix) return;
        declared << qMakePair(prefix, uri);
    };
    need(e.prefix(), e.namespaceURI());
    const QVector<QDomAttr> attrs = sortedAttributes(e);
    for (const QDomAttr &a : attrs) {
        if (!a.prefix().isEmpty()) need(a.prefix(), a.namespaceURI());
        const QString name = a.localName();
        if (name == QLatin1String("Requires") || name == QLatin1String("Ignorable") || name == QLatin1String("ProcessContent") || name == QLatin1String("MustUnderstand"))
            for (const QString &listed : a.value().split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                const QString prefix = listed.section(QLatin1Char(':'), 0, 0);   // ProcessContent names prefix:element
                if (known.contains(prefix)) need(prefix, known.value(prefix));
            }
    }
    *out += QLatin1Char('<') + qualifiedName(e.prefix(), e.localName());
    struct Saved {
        QString prefix, uri;
        bool had;
    };
    QVector<Saved> saved;
    for (const auto &d : declared) {
        saved << Saved{d.first, scope->value(d.first), scope->contains(d.first)};
        *out += d.first.isEmpty() ? QStringLiteral(" xmlns=\"%1\"").arg(escapedXml(d.second, true)) : QStringLiteral(" xmlns:%1=\"%2\"").arg(d.first, escapedXml(d.second, true));
        scope->insert(d.first, d.second);
    }
    for (const QDomAttr &a : attrs)
        *out += QLatin1Char(' ') + qualifiedName(a.prefix(), a.localName()) + QLatin1String("=\"") + escapedXml(a.value(), true) + QLatin1Char('"');
    if (!e.hasChildNodes()) {
        *out += QLatin1String("/>");
    } else {
        *out += QLatin1Char('>');
        for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.isElement()) writeElement(n.toElement(), out, scope, known);
            else if (n.isCharacterData() && !n.isComment()) *out += escapedXml(n.toCharacterData().data(), false);
        }
        *out += QLatin1String("</") + qualifiedName(e.prefix(), e.localName()) + QLatin1Char('>');
    }
    // The namespaces go back to what they were outside the element.
    for (const Saved &v : saved) {
        if (v.had) scope->insert(v.prefix, v.uri);
        else scope->remove(v.prefix);
    }
}

std::atomic<qint64> g_domBytes{0};

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

// Whether some tag in the text has more attributes (or namespace
// declarations) than kMaxAttributesPerElement: a count of the "=" outside
// quotes in each tag. Comments, CDATA, processing instructions, and a
// declaration are skipped, so what looks like an attribute in one is not
// counted. XML that is not well formed may be counted wrong either way, which
// the parsers then say.
template <class C>
bool crowdedTag(const C *p, const C *const end)
{
    auto at = [&](const char *text) {
        for (const C *q = p; *text; ++text, ++q)
            if (q >= end || *q != C(*text)) return false;
        return true;
    };
    auto skipPast = [&](const char *closer) {
        while (p < end && !at(closer)) ++p;
        p = size_t(end - p) < std::strlen(closer) ? end : p + std::strlen(closer);
    };
    while (p < end) {
        if (*p++ != '<') continue;
        if (at("!--")) {
            skipPast("-->");
        } else if (at("![CDATA[")) {
            skipPast("]]>");
        } else if (at("?")) {
            skipPast("?>");
        } else if (at("!")) {
            skipPast(">");
        } else {
            int count = 0;
            C quote = 0;
            for (; p < end && (quote || *p != '>'); ++p) {
                if (quote) {
                    if (*p == quote) quote = 0;
                } else if (*p == '"' || *p == '\'') {
                    quote = *p;
                } else if (*p == '=' && ++count > kMaxAttributesPerElement) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Whether some tag in the XML is crowded (see above). This is done first, on
// the bytes, because both parsers below take time in proportion to the square
// of the attributes in a tag and cannot be stopped partway through one. The
// XML is read as UTF-16 if it is that (a byte order mark, or a "<" in the
// first two bytes), and as bytes otherwise.
bool hasCrowdedElement(const QByteArray &xml)
{
    const auto *b = reinterpret_cast<const uchar *>(xml.constData());
    const bool little = xml.size() >= 2 && ((b[0] == 0xff && b[1] == 0xfe) || (b[0] == '<' && b[1] == 0));
    const bool big = xml.size() >= 2 && ((b[0] == 0xfe && b[1] == 0xff) || (b[0] == 0 && b[1] == '<'));
    if (!little && !big) return crowdedTag(xml.constData(), xml.constData() + xml.size());
    std::u16string text(size_t(xml.size()) / 2, u'\0');
    for (size_t i = 0; i < text.size(); ++i) text[i] = char16_t(little ? b[2 * i] | b[2 * i + 1] << 8 : b[2 * i] << 8 | b[2 * i + 1]);
    return crowdedTag(text.data(), text.data() + text.size());
}

} // namespace

bool sameNamespace(const QString &a, const QString &b)
{
    const int fa = family(a);
    return fa == 0 ? a == b : fa == family(b);
}

qint64 xmlBytesParsedByTheDomForTest() { return g_domBytes.load(); }

bool parseXml(const QByteArray &xml, QDomDocument *doc, QString *error)
{
    auto fail = [&](const QString &message) {
        if (error) *error = message;
        return false;
    };
    if (xml.size() > kMaxXmlBytes) return fail(QCoreApplication::translate("Xml", "The XML is too large to open."));
    if (hasCrowdedElement(xml)) return fail(QCoreApplication::translate("Xml", "The XML has an element with too many attributes."));
    // One quick pass for what the DOM would be slow or unsafe with: a DOCTYPE
    // (nothing here has a use for entities), nesting deep enough to make
    // building the tree take minutes, and more nodes than memory is there for.
    QXmlStreamReader r(xml);
    int depth = 0;
    qint64 nodes = 0;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isDTD()) return fail(QCoreApplication::translate("Xml", "The XML has a DOCTYPE, which is not allowed."));
        if (r.isStartElement()) {
            if (++depth > kMaxXmlDepth) return fail(QCoreApplication::translate("Xml", "The XML is nested too deeply."));
            nodes += 1 + r.attributes().size();
        } else if (r.isEndElement()) {
            --depth;
        } else if (r.isCharacters() || r.isComment() || r.isProcessingInstruction()) {
            ++nodes;
        }
        if (nodes > kMaxXmlNodes) return fail(QCoreApplication::translate("Xml", "The XML is too large to open."));
    }
    if (r.hasError()) return fail(r.errorString());
    QDomDocument::ParseOptions options(QDomDocument::ParseOption::UseNamespaceProcessing);
    options |= QDomDocument::ParseOption::PreserveSpacingOnlyNodes;
    g_domBytes += xml.size();
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

// The element as XML that can be read by itself: every namespace it uses is
// declared in it (the DOM does not keep the declarations of the elements around it).
QString standaloneXml(const QDomElement &e)
{
    QString out;
    QHash<QString, QString> scope, known;
    collectPrefixes(e, &known);
    writeElement(e, &out, &scope, known);
    return out;
}

} // namespace jp::office
