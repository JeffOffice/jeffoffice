#include "office/package.h"

#include "io/zip.h"
#include "office/xml.h"

#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <algorithm>

namespace jp::office {

namespace {

// A package of more than this many parts is refused (and, unless the caller
// says less, one of more than Package::kDefaultMaxInflated bytes inflated):
// no real file is near either. The most parts of 47 decks is 247, and the
// most inflated is 524 MB, which is pictures.
const int kMaxParts = 10000;

const QString kContentTypesItem = QStringLiteral("[Content_Types].xml");
const QString kTypesNs = ns::ContentTypes;
const QString kRelsNs = ns::PackageRelationships;
const QString kRelsContentType = QStringLiteral("application/vnd.openxmlformats-package.relationships+xml");

QString translate(const char *text) { return QCoreApplication::translate("Package", text); }

// The key part names are compared by: absolute, without regard to case.
QString nameKey(const QString &name)
{
    return name.startsWith(QLatin1Char('/')) ? name.toLower() : QLatin1Char('/') + name.toLower();
}

// The extension of a part name, without its dot; empty if it has none.
QString extensionOf(const QString &name)
{
    const QString file = name.mid(name.lastIndexOf(QLatin1Char('/')) + 1);
    const int dot = file.lastIndexOf(QLatin1Char('.'));
    return dot < 0 ? QString() : file.mid(dot + 1);
}

// Percent escapes read as the UTF-8 characters they stand for; an escape
// that isn't one stays as it is.
QString decodePercent(const QString &s)
{
    if (!s.contains(QLatin1Char('%'))) return s;
    auto hex = [](char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'A' && c <= 'F' ? c - 'A' + 10 : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; };
    const QByteArray in = s.toUtf8();
    QByteArray out;
    for (qsizetype i = 0; i < in.size(); ++i) {
        const int hi = i + 2 < in.size() ? hex(in[i + 1]) : -1;
        const int lo = hi >= 0 ? hex(in[i + 2]) : -1;
        if (in[i] == '%' && lo >= 0) {
            out.append(char(hi * 16 + lo));
            i += 2;
        } else {
            out.append(in[i]);
        }
    }
    return QString::fromUtf8(out);
}

// A segment of a part name (already decoded) that can be trusted: something
// in it, not "." or "..", no slash or backslash or control character.
bool usableSegment(const QString &s)
{
    if (s.isEmpty() || s == QLatin1String(".") || s == QLatin1String("..")) return false;
    for (const QChar c : s)
        if (c == QLatin1Char('/') || c == QLatin1Char('\\') || c.category() == QChar::Other_Control) return false;
    return true;
}

// A ZIP entry's name with the slashes that some tools write the other way
// round, and a slash in front of it, made as the standard has them.
QString plainItemName(QString item)
{
    item.replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (item.startsWith(QLatin1Char('/'))) item.remove(0, 1);
    return item;
}

// The part name a ZIP entry's name (made plain) stands for; empty if it is no
// good. Here not even "." or ".." is let through to be followed.
QString partNameOfItem(const QString &item)
{
    QStringList segments;
    for (const QString &raw : item.split(QLatin1Char('/'))) {
        const QString s = decodePercent(raw);
        if (!usableSegment(s)) return {};
        segments << s;
    }
    return QLatin1Char('/') + segments.join(QLatin1Char('/'));
}

// A relationship part is a ".rels" file in a "_rels" folder...
bool isRelationshipPart(const QString &name)
{
    const QStringList segments = name.split(QLatin1Char('/'));
    return segments.size() >= 3 && segments[segments.size() - 2].compare(QLatin1String("_rels"), Qt::CaseInsensitive) == 0
        && segments.last().endsWith(QLatin1String(".rels"), Qt::CaseInsensitive);
}

// ...and the source it belongs to is the part of that name in the folder
// above ("/" for /_rels/.rels); empty if it names none.
QString relationshipSource(const QString &name)
{
    const QStringList segments = name.split(QLatin1Char('/'));
    QString file = segments.last();
    file.chop(5);
    QStringList path = segments.mid(1, segments.size() - 3);
    if (file.isEmpty()) return path.isEmpty() ? QStringLiteral("/") : QString();
    path << file;
    return QLatin1Char('/') + path.join(QLatin1Char('/'));
}

// Calls `child` with the reader at each child of the root element, which
// must be {namespaceUri}root. False if the XML isn't well formed, has a DOCTYPE (which
// is how an entity could be defined), has another root, or `child` says so.
template <class F>
bool forEachChild(const QByteArray &xml, const QString &namespaceUri, const QString &root, F child)
{
    QXmlStreamReader r(xml);
    int depth = 0;
    bool rootSeen = false;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isDTD()) return false;
        if (r.isStartElement()) {
            ++depth;
            if (depth == 1) {
                if (r.name() != root || r.namespaceUri() != namespaceUri) return false;
                rootSeen = true;
            } else if (depth == 2 && r.namespaceUri() == namespaceUri && !child(r)) {
                return false;
            }
        } else if (r.isEndElement()) {
            --depth;
        }
    }
    return rootSeen && !r.hasError();
}

// The links one relationship part holds, their targets resolved against the
// part they start from. False if the part is no XML. An internal link that
// leads out of the package or to a name nothing could have is left out, as if
// it were not there, and counted in `skipped`.
bool parseRelationships(const QByteArray &xml, const QString &source, QVector<Relationship> *out, int *skipped)
{
    static const QRegularExpression scheme(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]+:"));
    QSet<QString> ids;
    return forEachChild(xml, kRelsNs, QStringLiteral("Relationships"), [&](const QXmlStreamReader &r) {
        if (r.name() != QLatin1String("Relationship")) return true;
        const QXmlStreamAttributes a = r.attributes();
        Relationship rel;
        rel.id = a.value(QLatin1String("Id")).toString();
        rel.type = a.value(QLatin1String("Type")).toString();
        rel.target = a.value(QLatin1String("Target")).toString();
        // A link with no Id or no Target says nothing; a repeated Id is
        // ignored (the first stands).
        if (rel.id.isEmpty() || rel.target.isEmpty() || ids.contains(rel.id)) return true;
        // An address with a scheme is outside whether or not it says so.
        rel.external = a.value(QLatin1String("TargetMode")).compare(QLatin1String("External"), Qt::CaseInsensitive) == 0
            || scheme.match(rel.target).hasMatch();
        if (!rel.external) {
            rel.target = resolvePartName(source, rel.target);
            if (rel.target.isEmpty()) {
                ++*skipped;
                return true;
            }
        }
        ids.insert(rel.id);
        out->append(rel);
        return true;
    });
}

bool parseContentTypes(const QByteArray &xml, QHash<QString, QString> *defaults, QHash<QString, QString> *overrides)
{
    return forEachChild(xml, kTypesNs, QStringLiteral("Types"), [&](const QXmlStreamReader &r) {
        const QXmlStreamAttributes a = r.attributes();
        const QString type = a.value(QLatin1String("ContentType")).toString();
        if (r.name() == QLatin1String("Default")) {
            const QString extension = a.value(QLatin1String("Extension")).toString().toLower();
            if (!extension.isEmpty() && !defaults->contains(extension)) defaults->insert(extension, type);
        } else if (r.name() == QLatin1String("Override")) {
            // A name nothing could have names no part: ignored.
            const QString name = resolvePartName(QStringLiteral("/"), a.value(QLatin1String("PartName")).toString());
            if (!name.isEmpty() && !overrides->contains(name.toLower())) overrides->insert(name.toLower(), type);
        }
        return true;
    });
}

quint16 le16(const QByteArray &a, qsizetype at)
{
    return at >= 0 && at + 2 <= a.size() ? quint16(uchar(a[at]) | uchar(a[at + 1]) << 8) : 0;
}

quint32 le32(const QByteArray &a, qsizetype at)
{
    return quint32(le16(a, at)) | quint32(le16(a, at + 2)) << 16;
}

// What the ZIP directory says its entries come to when inflated (as stored,
// for stored ones), before any of them is inflated, and the entries' names in
// the order the directory lists them. False if the directory can't be read,
// which readZip then reports in its own way.
bool scanDirectory(const QByteArray &zip, qint64 *total, QStringList *names)
{
    if (zip.size() < 22) return false;
    qsizetype eocd = -1;
    for (qsizetype i = zip.size() - 22; i >= std::max<qsizetype>(0, zip.size() - 65557); --i)
        if (le32(zip, i) == 0x06054b50) {
            eocd = i;
            break;
        }
    if (eocd < 0) return false;
    *total = 0;
    qsizetype p = le32(zip, eocd + 16);
    for (int i = 0, count = le16(zip, eocd + 10); i < count; ++i) {
        if (p + 46 > zip.size() || le32(zip, p) != 0x02014b50) return false;
        *total += std::max(le32(zip, p + 20), le32(zip, p + 24));
        const int nameLength = le16(zip, p + 28);
        if (p + 46 + nameLength > zip.size()) return false;
        names->append(QString::fromUtf8(zip.constData() + p + 46, nameLength));
        p += 46 + nameLength + le16(zip, p + 30) + le16(zip, p + 32);
    }
    return true;
}

} // namespace

QString resolvePartName(const QString &sourcePart, const QString &target)
{
    QString path = target;
    for (const QChar cut : {QLatin1Char('#'), QLatin1Char('?')}) {
        const int at = path.indexOf(cut);
        if (at >= 0) path.truncate(at);
    }
    if (path.isEmpty()) return {};
    const QStringList raw = path.split(QLatin1Char('/'));
    // A name is of a file: not a folder, which "." or ".." at the end would be.
    if (raw.last() == QLatin1String(".") || raw.last() == QLatin1String("..")) return {};
    QStringList segments;
    if (!path.startsWith(QLatin1Char('/'))) {
        // Relative to the folder the source part is in.
        segments = sourcePart.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        if (!segments.isEmpty() && !sourcePart.endsWith(QLatin1Char('/'))) segments.removeLast();
    }
    for (int i = path.startsWith(QLatin1Char('/')) ? 1 : 0; i < raw.size(); ++i) {
        if (raw[i] == QLatin1String(".")) continue;
        if (raw[i] == QLatin1String("..")) {
            if (segments.isEmpty()) return {};
            segments.removeLast();
            continue;
        }
        // Checked once decoded, so "%2e%2e" is no way round "..".
        const QString s = decodePercent(raw[i]);
        if (!usableSegment(s)) return {};
        segments << s;
    }
    return segments.isEmpty() ? QString() : QLatin1Char('/') + segments.join(QLatin1Char('/'));
}

bool Package::read(const QByteArray &zip, QString *error, qint64 maxInflated) { return load(zip, error, true, maxInflated); }

bool Package::readOpenDocument(const QByteArray &zip, QString *error) { return load(zip, error, false, kDefaultMaxInflated); }

int Package::skippedCount() const { return m_skipped; }

bool Package::load(const QByteArray &zip, QString *error, bool opc, qint64 maxInflated)
{
    *this = Package();
    auto fail = [&](const QString &message) {
        *this = Package();
        if (error) *error = message;
        return false;
    };
    // Too much is refused from the sizes in the directory, before anything is
    // inflated.
    auto tooMuch = [&] {
        return fail(translate("The file would take more than %1 of memory to open, which is too much.")
                        .arg(QLocale().formattedDataSize(maxInflated, 0, QLocale::DataSizeTraditionalFormat)));
    };
    qint64 declared = 0;
    QStringList order;
    const bool haveOrder = scanDirectory(zip, &declared, &order);
    if (haveOrder && declared > maxInflated) return tooMuch();
    QMap<QString, QByteArray> items;
    if (!readZip(zip, items)) return fail(translate("The file is damaged, or is not a package of the kind this program opens."));
    // Each entry was inflated into a buffer of the size the directory gave,
    // and refused if it did not fit exactly, so the total is no more than was
    // checked; looking again at what is there keeps that true whatever the
    // directory said.
    qint64 inflated = 0;
    for (const QByteArray &data : std::as_const(items)) inflated += data.size();
    if (inflated > maxInflated) return tooMuch();

    QByteArray types;
    bool haveTypes = false;
    QMap<QString, QByteArray> relationshipParts;
    QSet<QString> keys, seen;
    // In the order of the file, so that of two parts with one name the first
    // is kept; by name if the directory could not be listed.
    const QStringList entries = haveOrder ? order : items.keys();
    for (const QString &entry : entries) {
        if (!items.contains(entry) || seen.contains(entry)) continue;
        seen.insert(entry);
        // A folder entry holds nothing.
        if (entry.endsWith(QLatin1Char('/')) || entry.endsWith(QLatin1Char('\\'))) continue;
        const QString item = plainItemName(entry);
        if (opc && item.compare(kContentTypesItem, Qt::CaseInsensitive) == 0) {
            if (haveTypes) {
                ++m_skipped;
                continue;
            }
            types = items.value(entry);
            haveTypes = true;
            continue;
        }
        // A part with a name nothing could have, or one that differs from an
        // earlier part's only in case (and would be the same part), is left
        // out; the rest of the file is still good.
        const QString name = partNameOfItem(item);
        if (name.isEmpty() || keys.contains(name.toLower())) {
            ++m_skipped;
            continue;
        }
        keys.insert(name.toLower());
        if (isRelationshipPart(name)) {
            relationshipParts.insert(name, items.value(entry));
        } else {
            m_parts.insert(name, items.value(entry));
            m_names.insert(name.toLower(), name);
        }
    }
    if (m_parts.size() > kMaxParts) return fail(translate("The file holds more parts than a document can, which is 10,000."));
    if (!opc) return true;
    if (!haveTypes) return fail(translate("The file has no [Content_Types].xml, so it is not a package."));
    if (!parseContentTypes(types, &m_defaults, &m_overrides)) return fail(translate("The file's [Content_Types].xml is damaged."));
    for (auto it = relationshipParts.cbegin(); it != relationshipParts.cend(); ++it) {
        const QString source = relationshipSource(it.key());
        QVector<Relationship> links;
        if (!parseRelationships(it.value(), source.isEmpty() ? QStringLiteral("/") : source, &links, &m_skipped))
            return fail(translate("A part that holds the file's relationships, %1, is damaged.").arg(it.key()));
        if (!source.isEmpty()) {
            QHash<QString, int> &ids = m_relIds[source.toLower()];
            for (int i = 0; i < int(links.size()); ++i) ids.insert(links[i].id, i);
            m_rels.insert(source.toLower(), links);
        }
    }
    return true;
}

bool Package::readFile(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        *this = Package();
        if (error) *error = translate("The file couldn't be opened: %1").arg(f.errorString());
        return false;
    }
    return read(f.readAll(), error);
}

QStringList Package::partNames() const { return m_parts.keys(); }

bool Package::hasPart(const QString &name) const { return m_names.contains(nameKey(name)); }

QByteArray Package::part(const QString &name) const { return m_parts.value(m_names.value(nameKey(name))); }

QString Package::contentType(const QString &name) const
{
    const auto found = m_overrides.constFind(nameKey(name));
    if (found != m_overrides.cend()) return found.value();
    return m_defaults.value(extensionOf(name).toLower());
}

QVector<Relationship> Package::relationships(const QString &source) const
{
    return m_rels.value(nameKey(source.isEmpty() ? QStringLiteral("/") : source));
}

Relationship Package::relationship(const QString &source, const QString &id) const
{
    const QString key = nameKey(source.isEmpty() ? QStringLiteral("/") : source);
    const auto ids = m_relIds.constFind(key);
    if (ids == m_relIds.cend()) return {};
    const auto at = ids->constFind(id);
    return at == ids->cend() ? Relationship() : m_rels.value(key).at(*at);
}

QVector<Relationship> Package::relationshipsOfType(const QString &source, const QString &typeSuffix) const
{
    QVector<Relationship> out;
    for (const Relationship &r : relationships(source))
        if (r.type.endsWith(typeSuffix)) out << r;
    return out;
}

QString Package::mainPart() const
{
    for (const Relationship &r : relationshipsOfType(QStringLiteral("/"), QStringLiteral("/officeDocument")))
        if (!r.external) return r.target;
    return {};
}

namespace {

// The Default every package has for these, unless the writer is told another.
QString builtinDefault(const QString &extension)
{
    if (extension == QLatin1String("rels")) return kRelsContentType;
    if (extension == QLatin1String("xml")) return QStringLiteral("application/xml");
    return {};
}

// A part name (decoded, as it is here) as the package writes it: each segment escaped as a URI needs, a space as %20 and a % as %25 (the names
// of the ZIP's items, the Overrides', and a link's target are those of URIs).
QString encodedName(const QString &name)
{
    QStringList segments = name.split(QLatin1Char('/'));
    for (QString &s : segments) s = QString::fromLatin1(QUrl::toPercentEncoding(s, "!$&'()*+,;=@"));
    return segments.join(QLatin1Char('/'));
}

bool usableWriterName(const QString &name)
{
    return name.startsWith(QLatin1Char('/')) && resolvePartName(QStringLiteral("/"), encodedName(name)) == name
        && !isRelationshipPart(name) && name.compare(QLatin1Char('/') + kContentTypesItem, Qt::CaseInsensitive) != 0;
}

QString relationshipPartName(const QString &source)
{
    if (source == QLatin1String("/")) return QStringLiteral("/_rels/.rels");
    const int slash = source.lastIndexOf(QLatin1Char('/'));
    return source.left(slash) + QStringLiteral("/_rels/") + source.mid(slash + 1) + QStringLiteral(".rels");
}

// `target` (a part name) as a path from the folder `source` is in, each
// segment escaped as a URI needs.
QString relativeTarget(const QString &source, const QString &target)
{
    if (!target.startsWith(QLatin1Char('/'))) return target;
    QStringList from = source.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (!from.isEmpty()) from.removeLast();   // the source's own file name
    const QStringList to = target.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    // The folders they share, never counting the target's file name.
    int common = 0;
    while (common < from.size() && common < to.size() - 1 && from[common] == to[common]) ++common;
    QStringList out;
    for (int i = common; i < from.size(); ++i) out << QStringLiteral("..");
    for (int i = common; i < to.size(); ++i) out << QString::fromLatin1(QUrl::toPercentEncoding(to[i], "!$&'()*+,;=@"));
    return out.join(QLatin1Char('/'));
}

QByteArray relationshipsXml(const QVector<Relationship> &links)
{
    QByteArray xml;
    QXmlStreamWriter w(&xml);
    w.writeStartDocument(QStringLiteral("1.0"), true);
    w.writeStartElement(QStringLiteral("Relationships"));
    w.writeDefaultNamespace(kRelsNs);
    for (const Relationship &r : links) {
        w.writeEmptyElement(QStringLiteral("Relationship"));
        w.writeAttribute(QStringLiteral("Id"), r.id);
        w.writeAttribute(QStringLiteral("Type"), r.type);
        w.writeAttribute(QStringLiteral("Target"), r.target);
        if (r.external) w.writeAttribute(QStringLiteral("TargetMode"), QStringLiteral("External"));
    }
    w.writeEndElement();
    w.writeEndDocument();
    return xml;
}

} // namespace

void PackageWriter::addPart(const QString &name, const QByteArray &data, const QString &contentType)
{
    if (!usableWriterName(name)) {
        qWarning("PackageWriter: \"%s\" is not a usable part name; the part was left out", qPrintable(name));
        return;
    }
    const QString key = name.toLower();
    const auto at = m_partAt.constFind(key);
    if (at != m_partAt.cend()) {
        m_parts[*at] = Part{name, data, contentType};
        return;
    }
    m_partAt.insert(key, int(m_parts.size()));
    m_parts.append(Part{name, data, contentType});
}

void PackageWriter::addDefault(const QString &extension, const QString &contentType)
{
    m_defaults.insert(extension.toLower(), contentType);
}

QString PackageWriter::addRelationship(const QString &source, const QString &type, const QString &target, bool external)
{
    const QString from = source.isEmpty() ? QStringLiteral("/") : source;
    QVector<Relationship> &links = m_rels[from];
    QSet<QString> &ids = m_relIds[from];
    Relationship r;
    // (The next number no link of the source has, which is the count plus one unless links came with Ids of their own; where the last was found
    // is where to look from, so that adding links takes time in proportion to how many there are.)
    int &next = m_nextRelId[from];
    for (int n = std::max(next, int(links.size()) + 1);; ++n) {
        r.id = QStringLiteral("rId%1").arg(n);
        if (!ids.contains(r.id)) {
            next = n + 1;
            break;
        }
    }
    ids.insert(r.id);
    r.type = type;
    r.target = external ? target : relativeTarget(from, target);
    r.external = external;
    links.append(r);
    return r.id;
}

bool PackageWriter::addRelationshipWithId(const QString &source, const QString &id, const QString &type, const QString &target, bool external)
{
    const QString from = source.isEmpty() ? QStringLiteral("/") : source;
    QVector<Relationship> &links = m_rels[from];
    QSet<QString> &ids = m_relIds[from];
    if (ids.contains(id)) return false;
    ids.insert(id);
    Relationship r;
    r.id = id;
    r.type = type;
    r.target = external ? target : relativeTarget(from, target);
    r.external = external;
    links.append(r);
    return true;
}

QByteArray PackageWriter::finish()
{
    jp::ZipWriter zip;

    // Which extensions have a Default: the two every package has, then the
    // ones added (a replacement for either of the two included).
    QMap<QString, QString> defaults = m_defaults;
    for (const QString &extension : {QStringLiteral("rels"), QStringLiteral("xml")})
        if (!defaults.contains(extension)) defaults.insert(extension, builtinDefault(extension));
    QByteArray types;
    QXmlStreamWriter w(&types);
    w.writeStartDocument(QStringLiteral("1.0"), true);
    w.writeStartElement(QStringLiteral("Types"));
    w.writeDefaultNamespace(kTypesNs);
    for (auto it = defaults.cbegin(); it != defaults.cend(); ++it) {
        w.writeEmptyElement(QStringLiteral("Default"));
        w.writeAttribute(QStringLiteral("Extension"), it.key());
        w.writeAttribute(QStringLiteral("ContentType"), it.value());
    }
    for (const Part &p : m_parts) {
        if (defaults.value(extensionOf(p.name).toLower()) == p.contentType) continue;
        w.writeEmptyElement(QStringLiteral("Override"));
        w.writeAttribute(QStringLiteral("PartName"), encodedName(p.name));
        w.writeAttribute(QStringLiteral("ContentType"), p.contentType);
    }
    w.writeEndElement();
    w.writeEndDocument();
    zip.add(kContentTypesItem, types, true);

    if (m_rels.contains(QStringLiteral("/"))) zip.add(relationshipPartName(QStringLiteral("/")).mid(1), relationshipsXml(m_rels.value(QStringLiteral("/"))), true);
    for (const Part &p : m_parts) zip.add(encodedName(p.name).mid(1), p.data, true);
    for (auto it = m_rels.cbegin(); it != m_rels.cend(); ++it)
        if (it.key() != QLatin1String("/")) zip.add(encodedName(relationshipPartName(it.key())).mid(1), relationshipsXml(it.value()), true);
    return zip.finish();
}

} // namespace jp::office
