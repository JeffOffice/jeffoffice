#include "ole/odfobject.h"

#include "io/zip.h"
#include "office/mathobject.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp::office::ole {

namespace {

constexpr QLatin1String kBinaryType("application/vnd.sun.star.oleobject");
constexpr QLatin1String kSheetType("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
constexpr QLatin1String kPageType("application/vnd.openxmlformats-officedocument.wordprocessingml.document");

QString mediaTypeOfKind(Kind kind) { return kind == Kind::Sheet ? QString(kSheetType) : kind == Kind::Page ? QString(kPageType) : QString(); }

QString cleanName(QString name)
{
    if (name == QLatin1String("/")) return name;
    while (name.startsWith(QLatin1Char('/'))) name.remove(0, 1);
    if (name.startsWith(QLatin1String("./"))) name.remove(0, 2);
    return name;
}

} // namespace

namespace {

// The files of an OpenDocument file that are its own, never an object's part.
bool isOuterPart(const QString &path)
{
    const QString p = path.toLower();
    return p == QLatin1String("content.xml") || p == QLatin1String("styles.xml") || p == QLatin1String("settings.xml") || p == QLatin1String("meta.xml") || p == QLatin1String("mimetype") ||
           p == QLatin1String("meta-inf") || p.startsWith(QLatin1String("meta-inf/"));
}

QStringList sortedKeys(const QHash<QString, QString> &types)
{
    QStringList keys = types.keys();
    std::sort(keys.begin(), keys.end());
    return keys;
}

} // namespace

OdfObjectTestHooks &odfObjectTestHooks()
{
    static OdfObjectTestHooks hooks;
    return hooks;
}

QHash<QString, QString> odfManifestTypes(const QByteArray &manifestXml)
{
    QHash<QString, QString> types;
    QXmlStreamReader r(manifestXml);
    while (!r.atEnd() && types.size() < 100000) {
        if (r.readNext() != QXmlStreamReader::StartElement || r.name() != QLatin1String("file-entry")) continue;
        const QXmlStreamAttributes a = r.attributes();
        QString path, type;
        for (const QXmlStreamAttribute &at : a) {
            if (at.name() == QLatin1String("full-path")) path = at.value().toString();
            else if (at.name() == QLatin1String("media-type")) type = at.value().toString();
        }
        if (!path.isEmpty()) types.insert(cleanName(path), type);
    }
    return types;
}

OdfSource odfSourceOfPackage(const Package &package)
{
    OdfSource s;
    const Package *p = &package;
    s.names = [p] {
        QStringList out;
        for (const QString &n : p->partNames()) out << cleanName(n);
        return out;
    };
    s.size = [p](const QString &n) { return p->partSize(QLatin1Char('/') + n); };
    s.bytes = [p](const QString &n) {
        ++odfObjectTestHooks().inflatedParts;
        return p->part(QLatin1Char('/') + n);
    };
    s.head = [p](const QString &n) { return peekPart(*p, QLatin1Char('/') + n); };
    if (p->hasPart(QStringLiteral("/META-INF/manifest.xml"))) s.types = odfManifestTypes(p->part(QStringLiteral("/META-INF/manifest.xml")));
    return s;
}

bool odfPathIsSafe(const QString &path)
{
    if (path.isEmpty() || path.size() > 512 || path.startsWith(QLatin1Char('/')) || path.contains(QLatin1Char('\\')) || path.contains(QLatin1Char(':'))) return false;
    for (const QChar c : path)
        if (c.unicode() < 0x20) return false;
    const QStringList parts = path.split(QLatin1Char('/'));
    for (int i = 0; i < parts.size(); ++i) {
        const QString &s = parts[i];
        if (s == QLatin1String(".") || s == QLatin1String("..")) return false;
        if (s.isEmpty() && i != parts.size() - 1) return false;
    }
    return true;
}

QString OdfObjectReader::pathOf(const QString &href)
{
    QString path = href.trimmed();
    if (path.startsWith(QLatin1String("./"))) path.remove(0, 2);
    while (path.endsWith(QLatin1Char('/'))) path.chop(1);
    if (path.isEmpty() || path.contains(QLatin1String("..")) || path.contains(QLatin1Char(':')) || path.startsWith(QLatin1Char('/')) || path.startsWith(QLatin1Char('#')) || path.contains(QLatin1Char('\\')) ||
        path.contains(QLatin1String("//")) || path.size() > 256)
        return QString();
    return path;
}

const OdfObjectReader::Plan &OdfObjectReader::planOf(const QString &path, bool binary)
{
    const QString key = (binary ? QStringLiteral("b|") : QStringLiteral("f|")) + path;
    const auto known = m_plans.constFind(key);
    if (known != m_plans.constEnd()) return known.value();
    Plan plan;
    if (isOuterPart(path)) return m_plans.insert(key, plan).value();
    if (!m_named) {
        m_names = m_source.names ? m_source.names() : QStringList();
        std::sort(m_names.begin(), m_names.end());
        m_named = true;
    }
    const QString prefix = path + QLatin1Char('/');
    const auto first = std::lower_bound(m_names.cbegin(), m_names.cend(), prefix);
    const bool isFolder = first != m_names.cend() && first->startsWith(prefix);
    const qint64 oneSize = m_source.size ? m_source.size(path) : -1;
    if (isFolder && !binary) {
        const QString type = m_source.types.value(prefix);
        const QByteArray head = m_source.head ? m_source.head(prefix + QStringLiteral("content.xml")) : QByteArray();
        // (a formula or a chart is not an object here: its paths stay as they are)
        const bool formula = type.endsWith(QLatin1String(".formula")) || type.endsWith(QLatin1String(".formula-template")) || isMathObjectDocument(head);
        const bool chart = type.endsWith(QLatin1String(".chart")) || type.endsWith(QLatin1String(".chart-template")) || head.contains("office:chart");
        if (!formula && !chart) {
            plan.kind = Plan::Folder;
            plan.mediaType = type;
            if (plan.mediaType.isEmpty()) {
                const QByteArray mime = (m_source.head ? m_source.head(prefix + QStringLiteral("mimetype")) : QByteArray()).left(256).trimmed();
                plan.mediaType = mime.isEmpty() || mime.contains('<') ? QStringLiteral("application/octet-stream") : QString::fromLatin1(mime);
            }
            for (auto it = first; it != m_names.cend() && it->startsWith(prefix); ++it) {
                if (plan.files.size() >= kMostFolderEntries) {
                    plan.over = true;
                    break;
                }
                const QString inner = it->mid(prefix.size());
                if (inner == QLatin1String("mimetype") || inner == QLatin1String("META-INF/manifest.xml")) continue;   // (the object's package gets its own)
                const qint64 size = m_source.size ? m_source.size(*it) : -1;
                if (size < 0) continue;
                plan.files << *it;
                plan.total += size;
                if (plan.total > limits().objectBytes) {
                    plan.over = true;
                    break;
                }
            }
            if (plan.files.isEmpty()) plan.kind = Plan::Not;
        }
    } else if (!isFolder && oneSize >= 0 && (binary || m_source.types.contains(path))) {
        plan.kind = Plan::Binary;
        plan.files << path;
        plan.total = oneSize;
        plan.over = oneSize > limits().objectBytes;
        plan.mediaType = m_source.types.value(path);
        if (plan.mediaType.isEmpty()) plan.mediaType = kBinaryType;
    }
    return m_plans.insert(key, plan).value();
}

qint64 OdfObjectReader::sizeOf(const QString &href, bool binary)
{
    const QString path = pathOf(href);
    if (path.isEmpty()) return -1;
    const Plan &plan = planOf(path, binary);
    return plan.kind == Plan::Not ? -1 : plan.total;
}

OdfObjectReader::Result OdfObjectReader::read(const QString &href, bool binary, qint64 widthEmu, qint64 heightEmu, const QByteArray &preview, ObjectBudget &budget, EmbeddedObject *out)
{
    const QString path = pathOf(href);
    if (path.isEmpty()) return Result::NotObject;
    const Plan plan = planOf(path, binary);
    if (plan.kind == Plan::Not) return Result::NotObject;
    const QString key = (binary ? QStringLiteral("b|") : QStringLiteral("f|")) + path;
    auto blob = m_blobs.find(key);
    // (the count and the bytes are looked at before any part is inflated)
    if (plan.over || !budget.fits(plan.total, preview.size())) {
        ++m_past;
        return Result::PastLimit;
    }
    if (blob == m_blobs.end()) {
        ++odfObjectTestHooks().objectReads;
        Blob made;
        QByteArray bytes;
        if (plan.kind == Plan::Binary) {
            bytes = m_source.bytes(path);
            const Kind kind = plan.mediaType == kSheetType ? Kind::Sheet : plan.mediaType == kPageType ? Kind::Page : Kind::Other;
            if (kind != Kind::Other) made.progId = progIdOf(kind);
            else {
                made.form = OdfForm::Binary;
                made.mediaType = plan.mediaType;
            }
            if (bytes.size() != plan.total) bytes.clear();
        } else {
            jp::ZipWriter zip;
            zip.add(QStringLiteral("mimetype"), plan.mediaType.toUtf8(), false);
            QByteArray manifest = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.3\">";
            const QString prefix = path + QLatin1Char('/');
            auto entry = [&manifest](const QString &p, const QString &type) {
                manifest += "<manifest:file-entry manifest:full-path=\"" + p.toHtmlEscaped().toUtf8() + "\" manifest:media-type=\"" + (type.isEmpty() ? QStringLiteral("application/octet-stream") : type).toHtmlEscaped().toUtf8() + "\"/>";
            };
            entry(QStringLiteral("/"), plan.mediaType);
            bool good = true;
            qint64 total = 0;
            QByteArray all;
            for (const QString &f : plan.files) {
                const QByteArray b = m_source.bytes(f);
                total += b.size();
                if (b.size() != m_source.size(f) || total > plan.total) {
                    good = false;
                    break;
                }
                const QString rel = f.mid(prefix.size());
                zip.add(rel, b, false);
                entry(rel, m_source.types.value(f));
            }
            // (the folders the manifest names inside this one, with their types)
            for (const QString &key : sortedKeys(m_source.types))
                if (key.startsWith(prefix) && key.endsWith(QLatin1Char('/')) && key.size() > prefix.size()) entry(key.mid(prefix.size()), m_source.types.value(key));
            manifest += "</manifest:manifest>";
            zip.add(QStringLiteral("META-INF/manifest.xml"), manifest, false);
            if (good) bytes = zip.finish();
            made.form = OdfForm::Folder;
            made.mediaType = plan.mediaType;
        }
        made.bytes = bytes;
        blob = m_blobs.insert(key, made);
    }
    if (blob.value().bytes.isEmpty()) return Result::NotObject;
    std::optional<EmbeddedObject> kept = keepObject(blob.value().progId, blob.value().bytes, preview, widthEmu, heightEmu, budget, path);
    if (!kept) {
        ++m_past;
        return Result::PastLimit;
    }
    kept->odfForm = blob.value().form;
    kept->mediaType = blob.value().mediaType;
    *out = *kept;
    return Result::Kept;
}

QString odfPartsFailureNote(OdfPartsFailure why)
{
    if (why == OdfPartsFailure::TooLarge) return QCoreApplication::translate("jp::office::ole::OdfObject", "an embedded object holds more files or bytes than a file may keep (kept as its picture)");
    if (why == OdfPartsFailure::UnsafeName) return QCoreApplication::translate("jp::office::ole::OdfObject", "an embedded object holds a file with a name that is not safe to save (kept as its picture)");
    return QString();
}

QVector<OdfOutPart> odfPartsOf(const EmbeddedObject &object, const QString &dir, OdfPartsFailure *why)
{
    QVector<OdfOutPart> out;
    if (why) *why = OdfPartsFailure::None;
    if (object.partBytes.isEmpty()) return out;
    if (object.partBytes.size() > limits().objectBytes) {
        if (why) *why = OdfPartsFailure::TooLarge;
        return out;
    }
    if (object.isOurs && object.kind() != Kind::Other) {
        out.push_back({dir, object.partBytes, mediaTypeOfKind(object.kind()), false});
        return out;
    }
    if (object.odfForm != OdfForm::Folder) {
        out.push_back({dir, object.partBytes, object.mediaType.isEmpty() ? QString(kBinaryType) : object.mediaType, false});
        return out;
    }
    auto fail = [&](OdfPartsFailure reason) {
        if (why) *why = reason;
        return QVector<OdfOutPart>();
    };
    if (!packageWithinLimits(object.partBytes)) return fail(OdfPartsFailure::TooLarge);
    // (read lazily: each part's size is looked at before it is inflated)
    Package pkg;
    if (!pkg.readLazyOpenDocument(object.partBytes, nullptr, limits().objectBytes + (qint64(1) << 20))) return fail(OdfPartsFailure::TooLarge);
    struct Count {
        const Package &p;
        ~Count() { odfObjectTestHooks().partsInflated += p.inflatedBytes(); }
    } count{pkg};
    const QString manifestPart = QStringLiteral("/META-INF/manifest.xml");
    const QHash<QString, QString> types = pkg.hasPart(manifestPart) && pkg.partSize(manifestPart) <= (qint64(1) << 20) ? odfManifestTypes(pkg.part(manifestPart)) : QHash<QString, QString>();
    QString rootType = !object.mediaType.isEmpty() ? object.mediaType : types.value(QStringLiteral("/"));
    if (rootType.isEmpty()) {
        const QString mimePart = QStringLiteral("/mimetype");
        const QByteArray mime = pkg.hasPart(mimePart) && pkg.partSize(mimePart) <= 256 ? pkg.part(mimePart).trimmed() : QByteArray();
        rootType = mime.isEmpty() || mime.contains('<') ? QStringLiteral("application/octet-stream") : QString::fromLatin1(mime);
    }
    out.push_back({dir + QLatin1Char('/'), QByteArray(), rootType, false});
    for (const QString &key : sortedKeys(types))
        if (key.endsWith(QLatin1Char('/')) && key != QLatin1String("/") && odfPathIsSafe(key) && out.size() < kMostFolderEntries)
            out.push_back({dir + QLatin1Char('/') + key, QByteArray(), types.value(key).isEmpty() ? QStringLiteral("application/octet-stream") : types.value(key), false});
    qint64 total = 0;
    int files = 0;
    QStringList names = pkg.partNames();
    std::sort(names.begin(), names.end());
    for (const QString &n : names) {
        const QString rel = cleanName(n);
        if (rel == QLatin1String("mimetype") || rel == QLatin1String("META-INF/manifest.xml")) continue;
        if (!odfPathIsSafe(rel) || rel.endsWith(QLatin1Char('/'))) return fail(OdfPartsFailure::UnsafeName);
        if (++files > kMostFolderEntries) return fail(OdfPartsFailure::TooLarge);
        const qint64 size = pkg.partSize(n);
        total += size;
        if (size < 0 || total > limits().objectBytes) return fail(OdfPartsFailure::TooLarge);
        const QString type = !types.value(rel).isEmpty() ? types.value(rel) : rel.endsWith(QLatin1String(".xml")) ? QStringLiteral("text/xml") : QStringLiteral("application/octet-stream");
        ++odfObjectTestHooks().inflatedParts;
        out.push_back({dir + QLatin1Char('/') + rel, pkg.part(n), type, rel.endsWith(QLatin1String(".xml"))});
    }
    return out;
}

} // namespace jp::office::ole
