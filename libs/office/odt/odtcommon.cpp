#include "odt/odtcommon.h"

#include "sheet/odsstyles.h"

#include <QRegularExpression>
#include <cmath>
#include <cstring>

namespace jp::office::odt {

const char *const kNsOffice = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
const char *const kNsStyle = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
const char *const kNsText = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
const char *const kNsTable = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
const char *const kNsDraw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
const char *const kNsFo = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
const char *const kNsSvg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
const char *const kNsXlink = "http://www.w3.org/1999/xlink";
const char *const kNsDc = "http://purl.org/dc/elements/1.1/";
const char *const kNsMeta = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
const char *const kNsNumber = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";
const char *const kNsManifest = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
const char *const kNsLoext = "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";
const char *const kNsChart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
const char *const kNsOf = "urn:oasis:names:tc:opendocument:xmlns:of:1.2";

namespace {
const struct {
    const char *prefix;
    const char *const *uri;
} kAll[] = {{"office", &kNsOffice}, {"style", &kNsStyle}, {"text", &kNsText},     {"table", &kNsTable},       {"draw", &kNsDraw},     {"fo", &kNsFo},
            {"svg", &kNsSvg},       {"xlink", &kNsXlink}, {"dc", &kNsDc},         {"meta", &kNsMeta},         {"number", &kNsNumber}, {"manifest", &kNsManifest},
            {"loext", &kNsLoext},   {"chart", &kNsChart}, {"of", &kNsOf}};
constexpr int kAllCount = int(sizeof(kAll) / sizeof(kAll[0]));
}

Odf::Odf(const wml::XmlTree &tree) : m_tree(tree)
{
    for (int i = 0; i < kAllCount; ++i) {
        const int id = tree.namespaceId(*kAll[i].uri);
        m_ids[i] = id;
        if (id >= 0) m_prefixes.insert(id, kAll[i].prefix);
    }
}

int Odf::split(const char *qname, const char **local) const
{
    const char *colon = std::strchr(qname, ':');
    if (!colon) {
        *local = qname;
        return -1;
    }
    *local = colon + 1;
    const size_t len = size_t(colon - qname);
    for (int i = 0; i < kAllCount; ++i)
        if (std::strlen(kAll[i].prefix) == len && std::memcmp(kAll[i].prefix, qname, len) == 0) return m_ids[i];
    return -1;
}

bool Odf::is(int n, const char *qname) const
{
    const char *local = nullptr;
    const int ns = split(qname, &local);
    return ns >= 0 && m_tree.is(n, local, ns);
}

bool Odf::has(int n, const char *qname) const
{
    const char *local = nullptr;
    const int ns = split(qname, &local);
    return ns >= 0 && m_tree.has(n, local, ns);
}

QString Odf::at(int n, const char *qname) const
{
    const char *local = nullptr;
    const int ns = split(qname, &local);
    return ns >= 0 ? m_tree.attr(n, local, ns) : QString();
}

int Odf::child(int n, const char *qname) const
{
    const char *local = nullptr;
    const int ns = split(qname, &local);
    return ns >= 0 ? m_tree.child(n, local, ns) : -1;
}

QString Odf::canonical(int ns, const QByteArray &local) const
{
    const QByteArray p = m_prefixes.value(ns);
    if (p.isEmpty()) return QString::fromLatin1(local);
    return QString::fromLatin1(p) + QLatin1Char(':') + QString::fromLatin1(local);
}

QString Odf::nameOf(int n) const
{
    return canonical(m_tree.ns(n), m_tree.localName(n));
}

bool lengthToPoints(const QString &text, double *points)
{
    return sheet::ods::lengthPoints(text, points);
}

qint64 lengthToTwips(const QString &text, bool *ok)
{
    double pt = 0;
    const bool good = lengthToPoints(text, &pt);
    if (ok) *ok = good;
    return good ? qint64(std::llround(clampPoints(pt) * 20)) : 0;
}

qint64 lengthToEmu(const QString &text, bool *ok)
{
    double pt = 0;
    const bool good = lengthToPoints(text, &pt);
    if (ok) *ok = good;
    return good ? qint64(std::llround(clampPoints(pt) * 12700)) : 0;
}

QString colorHex(const QString &text)
{
    quint32 rgb = 0;
    if (!sheet::ods::colorOf(text, &rgb)) return QString();
    return QStringLiteral("%1").arg(rgb, 6, 16, QLatin1Char('0')).toUpper();
}

QString inchesFromTwips(double twips)
{
    return QString::number(std::max(-1e9, std::min(1e9, std::isfinite(twips) ? twips : 0.0)) / 1440.0, 'f', 4) + QStringLiteral("in");
}

QString inchesFromEmu(double emu)
{
    return QString::number(std::max(-1e12, std::min(1e12, std::isfinite(emu) ? emu : 0.0)) / 914400.0, 'f', 4) + QStringLiteral("in");
}

bool percentOf(const QString &text, double *value)
{
    const QString t = text.trimmed();
    if (!t.endsWith(QLatin1Char('%'))) return false;
    bool ok = false;
    const double v = t.left(t.size() - 1).toDouble(&ok);
    if (!ok || !std::isfinite(v)) return false;
    *value = v;
    return true;
}

QString decodeStyleName(const QString &name)
{
    static const QRegularExpression re(QStringLiteral("_([0-9a-fA-F]{2,4})_"));
    QString out;
    qsizetype last = 0;
    for (auto it = re.globalMatch(name); it.hasNext();) {
        const auto m = it.next();
        bool ok = false;
        const uint c = m.captured(1).toUInt(&ok, 16);
        out += name.mid(last, m.capturedStart() - last);
        if (ok && c >= 0x20 && c < 0xD800) out += QChar(c);
        else out += m.captured(0);
        last = m.capturedEnd();
    }
    return out + name.mid(last);
}

QString encodeStyleName(const QString &display)
{
    QString out;
    for (const QChar c : display) {
        if (c.isLetterOrNumber() && c.unicode() < 128) out += c;
        else if (c == QLatin1Char('.') || c == QLatin1Char('-')) out += c;
        else out += QStringLiteral("_%1_").arg(uint(c.unicode()), 2, 16, QLatin1Char('0'));
    }
    if (out.isEmpty() || out[0].isDigit() || out[0] == QLatin1Char('-') || out[0] == QLatin1Char('.')) out.prepend(QLatin1Char('S'));
    return out;
}

double clampPoints(double points)
{
    if (!std::isfinite(points)) return 0;
    return std::max(-kMaxLengthPoints, std::min(kMaxLengthPoints, points));
}

qint64 clampedInt(double v)
{
    if (!std::isfinite(v)) return 0;
    return qint64(std::max(-1e12, std::min(1e12, v)));
}

QString xmlSafeText(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        const ushort u = text[i].unicode();
        if (u < 0x20 && u != 0x9 && u != 0xA && u != 0xD) continue;
        if (u == 0xFFFE || u == 0xFFFF) continue;
        if (QChar::isHighSurrogate(u)) {
            if (i + 1 < text.size() && QChar::isLowSurrogate(text[i + 1].unicode())) {
                out += text[i];
                out += text[++i];
            }
            continue;
        }
        if (QChar::isLowSurrogate(u)) continue;
        out += text[i];
    }
    return out;
}

QString hexColorOrEmpty(const QString &text)
{
    static const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f]{6}$"));
    return re.match(text).hasMatch() ? text.toUpper() : QString();
}

QString quoteFieldArgument(const QString &name)
{
    QString q = xmlSafeText(name);
    q.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    q.replace(QLatin1Char('"'), QLatin1String("\\\""));
    return QLatin1Char('"') + q + QLatin1Char('"');
}

QByteArray esc(const QString &text, bool attribute)
{
    return wml::escapeXml(xmlSafeText(text), attribute);
}

} // namespace jp::office::odt
