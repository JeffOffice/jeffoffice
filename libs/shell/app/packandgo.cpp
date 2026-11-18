#include "app/packandgo.h"

#include "app/filedialogs.h"
#include "app/filescreenparts.h"
#include "app/plaintext.h"
#include "core/fonts.h"
#include "core/remotepath.h"
#include "core/sfnt.h"
#include "io/zip.h"
#include "office/hyperlink.h"
#include "office/officecrypt.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QMessageBox>
#include <QSet>
#include <QUrl>
#include <QVBoxLayout>

#include <utility>

namespace jp {

namespace {

constexpr int kMostEntries = 10000;                       // parts of a package read
constexpr qint64 kMostPartBytes = qint64(512) << 20;      // one part inflated
constexpr qint64 kMostPackageBytes = qint64(2) << 30;     // all the parts inflated
constexpr int kMostLinks = 100000;                        // links found in a package
constexpr int kMostFamilies = 2000;                       // fonts named in a package
constexpr int kMostLinkText = 4096;                       // one link's text
constexpr int kMostListed = 20;                           // links named in the report

using Part = PackPart;

bool readParts(const QByteArray &zip, QVector<Part> *out, QString *error)
{
    ZipScanStats stats;
    bool any = false;
    scanZipEach(
        zip,
        [&](ZipScanEntry &e) {
            any = true;
            if (!e.whole) return;
            out->push_back({e.name, std::move(e.data)});
        },
        kMostEntries, kMostPartBytes, kMostPackageBytes, &stats);
    if (!any || stats.entriesLeftOut || stats.sizeLeftOut) {
        if (error) *error = QCoreApplication::translate("PackAndGo", "The file is too large or is not a package Pack and Go can read.");
        return false;
    }
    return true;
}

QByteArray writeParts(const QVector<Part> &parts)
{
    ZipWriter z;
    // ([Content_Types].xml first, as packages have it)
    for (const Part &p : parts)
        if (p.name == QLatin1String("[Content_Types].xml")) z.add(p.name, p.data, true);
    for (const Part &p : parts)
        if (p.name != QLatin1String("[Content_Types].xml")) z.add(p.name, p.data, true);
    return z.finish();
}

QString unescapeXml(QString s)
{
    s.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\""))
        .replace(QLatin1String("&apos;"), QLatin1String("'")).replace(QLatin1String("&amp;"), QLatin1String("&"));
    return s;
}

QString escapeXml(QString s, bool attribute = true)
{
    s.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('>'), QLatin1String("&gt;"));
    if (attribute) s.replace(QLatin1Char('"'), QLatin1String("&quot;"));
    return s;
}

// A scheme (http:, mailto:) that is not a file's; "C:" is a drive.
bool hasOtherScheme(const QString &t)
{
    static const QRegularExpression scheme(QStringLiteral("^([A-Za-z][A-Za-z0-9+.-]+):"));
    const QRegularExpressionMatch m = scheme.match(t);
    return m.hasMatch() && m.captured(1).compare(QLatin1String("file"), Qt::CaseInsensitive) != 0;
}

bool kindOfRelationship(const QString &type, PackKind *kind)
{
    const QString t = type.section(QLatin1Char('/'), -1).toLower();
    if (t == QLatin1String("image")) *kind = PackKind::Picture;
    else if (t == QLatin1String("video")) *kind = PackKind::Movie;
    else if (t == QLatin1String("audio")) *kind = PackKind::Sound;
    else if (t == QLatin1String("media")) *kind = PackKind::Movie;   // (told from a sound by its name when it is resolved)
    else if (t == QLatin1String("externallinkpath")) *kind = PackKind::Workbook;
    else if (t == QLatin1String("oleobject") || t == QLatin1String("package")) *kind = PackKind::Object;
    else if (t == QLatin1String("subdocument")) *kind = PackKind::Document;
    else return false;
    return true;
}

// One Relationship element: where it is in the part, and its attributes.
struct Rel {
    qsizetype start = 0, length = 0;
    QString type, target;
    bool external = false;
    qsizetype targetStart = -1, targetLength = 0;   // the value of Target, inside the element
};

QVector<Rel> relationshipsIn(const QString &xml)
{
    static const QRegularExpression element(QStringLiteral("<Relationship\\b[^>]*>"));
    static const QRegularExpression attribute(QStringLiteral("([A-Za-z:]+)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    QVector<Rel> out;
    auto it = element.globalMatch(xml);
    while (it.hasNext() && out.size() < kMostLinks) {
        const QRegularExpressionMatch m = it.next();
        Rel r;
        r.start = m.capturedStart();
        r.length = m.capturedLength();
        auto at = attribute.globalMatch(m.captured());
        while (at.hasNext()) {
            const QRegularExpressionMatch a = at.next();
            const QString name = a.captured(1);
            const bool dq = a.capturedStart(2) >= 0;
            const QString value = dq ? a.captured(2) : a.captured(3);
            if (name == QLatin1String("Type")) r.type = value;
            else if (name == QLatin1String("TargetMode")) r.external = value.compare(QLatin1String("External"), Qt::CaseInsensitive) == 0;
            else if (name == QLatin1String("Target")) {
                r.target = unescapeXml(value);
                r.targetStart = m.capturedStart() + a.capturedStart(dq ? 2 : 3);
                r.targetLength = value.size();
            }
        }
        out.push_back(r);
    }
    return out;
}

// An INCLUDEPICTURE or INCLUDETEXT field's file in an instruction (already unescaped): where the name is, and the name.
struct FieldLink {
    qsizetype start = 0, length = 0;
    QString path;
    bool quoted = false, text = false;
};

QVector<FieldLink> fieldLinksIn(const QString &instruction)
{
    static const QRegularExpression field(QStringLiteral("\\b(INCLUDEPICTURE|INCLUDETEXT)\\s+(?:\"((?:[^\"\\\\]|\\\\.)*)\"|([^\\s\"\\\\]\\S*))"), QRegularExpression::CaseInsensitiveOption);
    QVector<FieldLink> out;
    auto it = field.globalMatch(instruction);
    while (it.hasNext() && out.size() < 64) {
        const QRegularExpressionMatch m = it.next();
        FieldLink f;
        f.text = m.captured(1).compare(QLatin1String("INCLUDETEXT"), Qt::CaseInsensitive) == 0;
        f.quoted = m.capturedStart(2) >= 0;
        const int group = f.quoted ? 2 : 3;
        f.start = m.capturedStart(group);
        f.length = m.capturedLength(group);
        f.path = m.captured(group);
        if (f.quoted) f.path.replace(QLatin1String("\\\\"), QLatin1String("\\")).replace(QLatin1String("\\\""), QLatin1String("\""));
        out.push_back(f);
    }
    return out;
}

// The parts that hold field instructions: the instruction text and the instr attribute.
struct Instr {
    qsizetype start = 0, length = 0;   // the value, as the part spells it
    QString value;                     // unescaped
    bool attribute = false;
};

QVector<Instr> instructionsIn(const QString &xml)
{
    static const QRegularExpression found(QStringLiteral("<w:instrText\\b[^>]*>([^<]*)</w:instrText>|<w:fldSimple\\b[^>]*?\\bw:instr=\"([^\"]*)\""));
    QVector<Instr> out;
    auto it = found.globalMatch(xml);
    while (it.hasNext() && out.size() < kMostLinks) {
        const QRegularExpressionMatch m = it.next();
        const int g = m.capturedStart(1) >= 0 ? 1 : 2;
        const QString v = unescapeXml(m.captured(g));
        if (!v.contains(QLatin1String("INCLUDE"), Qt::CaseInsensitive)) continue;
        out.push_back({m.capturedStart(g), m.capturedLength(g), v, g == 2});
    }
    return out;
}

bool isWordPart(const QString &name) { return name.startsWith(QLatin1String("word/")) && name.endsWith(QLatin1String(".xml")) && !name.contains(QLatin1String("/_rels/")); }

QString fieldPathText(const QString &p) { return QString(p).replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char('"'), QLatin1String("\\\"")); }

// What the kinds of file may be called (a link is followed only to these, by the name of the file it comes to).
const QSet<QString> &allowedSuffixes(PackKind kind)
{
    static const QSet<QString> pictures = {"png", "jpg", "jpeg", "jpe", "jfif", "gif", "bmp", "dib", "tif", "tiff", "svg", "svgz", "emf", "wmf", "wdp", "jxr", "webp", "ico", "eps", "pict"};
    static const QSet<QString> movies = {"mp4", "m4v", "mov", "mkv", "webm", "avi", "wmv", "mpg", "mpeg", "mpe", "3gp", "3g2", "asf", "flv", "ogv"};
    static const QSet<QString> sounds = {"mp3", "wav", "m4a", "wma", "ogg", "oga", "opus", "flac", "aac", "mid", "midi", "aif", "aiff", "aifc", "au", "snd", "mka"};
    static const QSet<QString> documents = {"docx", "docm", "dotx", "dotm", "doc", "odt", "rtf", "txt", "htm", "html", "pdf", "pptx", "pptm", "ppsx", "ppt", "odp", "xlsx", "xlsm", "xlsb", "xls",
                                            "xltx", "xltm", "ods", "csv", "tsv", "xml", "bin"};
    static const QSet<QString> media = [] { return QSet<QString>(movies).unite(sounds).unite(pictures); }();
    static const QSet<QString> everything = [] { return QSet<QString>(media).unite(documents); }();
    switch (kind) {
    case PackKind::Picture: return pictures;
    case PackKind::Movie:
    case PackKind::Sound: return media;
    case PackKind::Workbook:
    case PackKind::Document: return documents;
    case PackKind::Object: return everything;
    }
    return everything;
}

QString plainName(const QString &file, QSet<QString> *taken)
{
    QString base = QFileInfo(file).fileName();
    QString out;
    for (const QChar c : base) out += (c.isLetterOrNumber() || c == QLatin1Char('.') || c == QLatin1Char('-') || c == QLatin1Char('_')) ? c : QLatin1Char('_');
    while (out.startsWith(QLatin1Char('.'))) out.remove(0, 1);
    if (out.size() > 100) out = out.left(60) + out.right(40);
    if (out.isEmpty()) out = QStringLiteral("file");
    static const QRegularExpression reserved(QStringLiteral("^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(\\.|$)"), QRegularExpression::CaseInsensitiveOption);
    if (reserved.match(out).hasMatch()) out.prepend(QLatin1Char('_'));
    QString unique = out;
    const QString stem = QFileInfo(out).completeBaseName(), suffix = QFileInfo(out).suffix();
    for (int n = 2; taken->contains(unique.toLower()); ++n) unique = stem + QLatin1Char('_') + QString::number(n) + (suffix.isEmpty() ? QString() : QLatin1Char('.') + suffix);
    taken->insert(unique.toLower());
    return unique;
}

} // namespace

// ---------------- the package scan ----------------

static QVector<PackLink> linksInParts(const QVector<Part> &parts);
static QStringList fontsInParts(const QVector<Part> &parts);
static QByteArray linksMovedIn(QVector<Part> parts, const QHash<QString, QString> &moved);

bool OpcPackSource::have(QString *error)
{
    if (m_have) return true;
    QString err;
    const QByteArray bytes = savedBytes(&err);
    if (bytes.isEmpty()) {
        if (error) *error = err.isEmpty() ? QCoreApplication::translate("PackAndGo", "The copy couldn't be made.") : err;
        return false;
    }
    QVector<Part> parts;
    if (!readParts(bytes, &parts, &err)) {
        if (error) *error = err;
        return false;
    }
    m_bytes = bytes;
    m_parts = std::move(parts);
    m_have = true;
    return true;
}

QVector<PackLink> OpcPackSource::linksOf(const QByteArray &package, QString *error)
{
    QVector<Part> parts;
    if (!readParts(package, &parts, error)) return QVector<PackLink>();
    return linksInParts(parts);
}

static QVector<PackLink> linksInParts(const QVector<Part> &parts)
{
    QVector<PackLink> out;
    QSet<QString> seen;
    for (const Part &p : parts) {
        if (out.size() >= kMostLinks) break;
        if (p.name.endsWith(QLatin1String(".rels"), Qt::CaseInsensitive)) {
            const QString xml = QString::fromUtf8(p.data);
            for (const Rel &r : relationshipsIn(xml)) {
                PackKind kind;
                if (!r.external || r.target.isEmpty() || r.target.size() > kMostLinkText || hasOtherScheme(r.target) || !kindOfRelationship(r.type, &kind)) continue;
                if (kind == PackKind::Movie && allowedSuffixes(PackKind::Sound).contains(QFileInfo(r.target).suffix().toLower()) && !allowedSuffixes(PackKind::Movie).contains(QFileInfo(r.target).suffix().toLower()))
                    kind = PackKind::Sound;
                if (seen.contains(r.target)) continue;
                seen.insert(r.target);
                out.push_back({r.target, kind});
            }
        } else if (isWordPart(p.name) && p.data.contains("INCLUDE")) {
            for (const Instr &i : instructionsIn(QString::fromUtf8(p.data)))
                for (const FieldLink &f : fieldLinksIn(i.value)) {
                    if (f.path.isEmpty() || f.path.size() > kMostLinkText || hasOtherScheme(f.path) || seen.contains(f.path)) continue;
                    seen.insert(f.path);
                    out.push_back({f.path, f.text ? PackKind::Document : PackKind::Picture});
                }
        }
    }
    return out;
}

static QStringList fontsInParts(const QVector<Part> &parts)
{
    static const QRegularExpression drawing(QStringLiteral("<a:(?:latin|ea|cs|sym)\\b[^>]*?\\btypeface=\"([^\"]*)\""));
    static const QRegularExpression runFonts(QStringLiteral("<w:rFonts\\b[^>]*>"));
    static const QRegularExpression wordAttr(QStringLiteral("\\bw:(?:ascii|hAnsi|eastAsia|cs)=\"([^\"]*)\""));
    static const QRegularExpression cell(QStringLiteral("<(?:name|rFont)\\s+val=\"([^\"]*)\""));
    QStringList out;
    QSet<QString> seen;
    auto add = [&](const QString &raw) {
        const QString f = unescapeXml(raw).trimmed();
        if (f.isEmpty() || f.startsWith(QLatin1Char('+')) || out.size() >= kMostFamilies || seen.contains(f.toLower())) return;
        seen.insert(f.toLower());
        out.push_back(f);
    };
    for (const Part &p : parts) {
        if (!p.name.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) continue;
        if (!(p.name.startsWith(QLatin1String("word/")) || p.name.startsWith(QLatin1String("ppt/")) || p.name.startsWith(QLatin1String("xl/")))) continue;
        const QString xml = QString::fromUtf8(p.data);
        for (auto it = drawing.globalMatch(xml); it.hasNext();) add(it.next().captured(1));
        if (p.name.startsWith(QLatin1String("word/")))
            for (auto it = runFonts.globalMatch(xml); it.hasNext();)
                for (auto at = wordAttr.globalMatch(it.next().captured()); at.hasNext();) add(at.next().captured(1));
        if (p.name.startsWith(QLatin1String("xl/")))
            for (auto it = cell.globalMatch(xml); it.hasNext();) add(it.next().captured(1));
    }
    return out;
}

QStringList OpcPackSource::fontsOf(const QByteArray &package)
{
    QVector<Part> parts;
    return readParts(package, &parts, nullptr) ? fontsInParts(parts) : QStringList();
}

static QByteArray linksMovedIn(QVector<Part> parts, const QHash<QString, QString> &moved)
{
    for (Part &p : parts) {
        if (p.name.endsWith(QLatin1String(".rels"), Qt::CaseInsensitive)) {
            QString xml = QString::fromUtf8(p.data);
            const QVector<Rel> rels = relationshipsIn(xml);
            bool changed = false;
            for (qsizetype i = rels.size() - 1; i >= 0; --i) {
                const Rel &r = rels[i];
                if (!r.external || r.targetStart < 0 || !moved.contains(r.target)) continue;
                xml.replace(r.targetStart, r.targetLength, escapeXml(moved.value(r.target)));
                changed = true;
            }
            if (changed) p.data = xml.toUtf8();
        } else if (isWordPart(p.name) && p.data.contains("INCLUDE")) {
            QString xml = QString::fromUtf8(p.data);
            const QVector<Instr> instrs = instructionsIn(xml);
            bool changed = false;
            for (qsizetype i = instrs.size() - 1; i >= 0; --i) {
                QString value = instrs[i].value;
                const QVector<FieldLink> fields = fieldLinksIn(value);
                bool here = false;
                for (qsizetype k = fields.size() - 1; k >= 0; --k) {
                    if (!moved.contains(fields[k].path)) continue;
                    // (the escaping a quoted name has is undone when read: write it back the same way)
                    value.replace(fields[k].start, fields[k].length, fields[k].quoted ? fieldPathText(moved.value(fields[k].path)) : moved.value(fields[k].path));
                    here = true;
                }
                if (!here) continue;
                xml.replace(instrs[i].start, instrs[i].length, escapeXml(value, instrs[i].attribute));
                changed = true;
            }
            if (changed) p.data = xml.toUtf8();
        }
    }
    return writeParts(parts);
}

QByteArray OpcPackSource::withLinks(const QByteArray &package, const QHash<QString, QString> &moved, QString *error)
{
    QVector<Part> parts;
    return readParts(package, &parts, error) ? linksMovedIn(std::move(parts), moved) : QByteArray();
}

QStringList OpcPackSource::fontFamilies()
{
    return have(nullptr) ? fontsInParts(m_parts) : QStringList();
}

QVector<PackLink> OpcPackSource::links()
{
    return have(nullptr) ? linksInParts(m_parts) : QVector<PackLink>();
}

bool OpcPackSource::saveCopy(const QHash<QString, QString> &moved, QByteArray *bytes, QString *error)
{
    if (!have(error)) return false;
    QByteArray out = moved.isEmpty() ? m_bytes : linksMovedIn(m_parts, moved);
    if (out.isEmpty()) return false;
    const QString secret = password();
    if (!secret.isEmpty()) {
        out = office::encryptPackage(out, secret);
        if (out.isEmpty()) {
            if (error) *error = QCoreApplication::translate("PackAndGo", "The copy couldn't be locked with its password.");
            return false;
        }
    }
    *bytes = out;
    return true;
}

// ---------------- fonts ----------------

PackedFonts packFonts(const QStringList &families, const QStringList &dirs)
{
    PackedFonts out;
    QSet<QString> taken;
    const QStringList fontDirs = dirs.isEmpty() ? bundledFontDirs() : dirs;
    for (const QString &d : fontDirs) {
        for (const QFileInfo &fi : QDir(d).entryInfoList({"*.ttf", "*.otf"}, QDir::Files)) {
            const QString stem = fi.completeBaseName().section('-', 0, 0);
            bool wanted = false;
            for (const QString &fam : families)
                if (QString(fam).remove(' ').compare(stem, Qt::CaseInsensitive) == 0) wanted = true;
            if (!wanted || taken.contains(fi.fileName())) continue;
            taken.insert(fi.fileName());
            QFile f(fi.absoluteFilePath());
            if (!f.open(QIODevice::ReadOnly)) continue;
            const QByteArray data = f.readAll();
            FontFileInfo info;
            if (!inspectFont(data, &info) || !fontAllowsEmbedding(info.fsType)) {
                const QString family = info.family.isEmpty() ? stem : info.family;
                if (!out.leftOut.contains(family)) out.leftOut.push_back(family);
                continue;
            }
            out.files.push_back({fi.fileName(), data});
        }
        if (!out.files.isEmpty() || !out.leftOut.isEmpty()) break;
    }
    return out;
}

QString fontsLeftOutText(const QStringList &families)
{
    if (families.isEmpty()) return QString();
    return QCoreApplication::translate("PackAndGo", "These fonts were not packed, because their licenses do not allow it: %1.").arg(families.join(QStringLiteral(", ")));
}

bool writeZipFile(const QString &path, const QByteArray &bytes, QString *error)
{
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size() || !out.commit()) {
        if (error) *error = out.errorString();
        return false;
    }
    return true;
}

// ---------------- links ----------------

QString resolvePackLink(const QString &targetIn, PackKind kind, const QString &baseFolder, const QString &documentPath, QString *why, bool *missing)
{
    auto no = [&](const QString &reason, bool isMissing = false) {
        if (why) *why = reason;
        if (missing) *missing = isMissing;
        return QString();
    };
    QString path = targetIn.trimmed();
    if (path.isEmpty()) return no(QCoreApplication::translate("PackAndGo", "The link is empty."));
    if (path.size() > kMostLinkText) return no(QCoreApplication::translate("PackAndGo", "The link is too long."));
    if (path.contains(QChar(0))) return no(QCoreApplication::translate("PackAndGo", "The link is not a file name."));
    if (path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        path = QUrl(path).toLocalFile();
        if (path.isEmpty()) return no(QCoreApplication::translate("PackAndGo", "The link is not a file on this computer."));
    } else if (hasOtherScheme(path)) {
        return no(QCoreApplication::translate("PackAndGo", "The link is a web address."));
    }
    if (isOnAnotherComputer(path)) return no(QCoreApplication::translate("PackAndGo", "The file is on another computer."));
    if (isWindowsSpecialPath(path) || office::isProgramFile(path)) return no(QCoreApplication::translate("PackAndGo", "The link is not a file Pack and Go takes."));
    path = QDir::fromNativeSeparators(path);
    const bool relative = QDir::isRelativePath(path);
    const QString base = baseFolder.isEmpty() ? QString() : QDir::cleanPath(QFileInfo(baseFolder).absoluteFilePath());
    if (relative) {
        if (base.isEmpty()) return no(QCoreApplication::translate("PackAndGo", "The document has not been saved, so there is no folder to find the file in."), true);
        path = QDir::cleanPath(base + QLatin1Char('/') + path);
        // (a relative link stays in the document's folder: it may not go up out of it)
        if (!path.startsWith(base.endsWith(QLatin1Char('/')) ? base : base + QLatin1Char('/'))) return no(QCoreApplication::translate("PackAndGo", "The link leaves the document's folder."));
    } else {
        path = QDir::cleanPath(path);
    }
    if (isOnAnotherComputer(path) || isWindowsSpecialPath(path)) return no(QCoreApplication::translate("PackAndGo", "The link is not a file Pack and Go takes."));
    const QFileInfo info(path);
    if (!info.exists()) return no(QCoreApplication::translate("PackAndGo", "The file was not found."), true);
    if (!info.isFile()) return no(QCoreApplication::translate("PackAndGo", "The link is not a file."));
    if (!documentPath.isEmpty() && info.canonicalFilePath() == QFileInfo(documentPath).canonicalFilePath()) return no(QCoreApplication::translate("PackAndGo", "The link is to the document itself."));
    if (!allowedSuffixes(kind).contains(info.suffix().toLower())) return no(QCoreApplication::translate("PackAndGo", "This is not a kind of file Pack and Go takes."));
    // (a link to a file that is really another kind is looked at by what it comes to)
    const QString real = info.canonicalFilePath();
    if (real.isEmpty() || !allowedSuffixes(kind).contains(QFileInfo(real).suffix().toLower())) return no(QCoreApplication::translate("PackAndGo", "This is not a kind of file Pack and Go takes."));
    return info.absoluteFilePath();
}

// ---------------- the report ----------------

int PackReport::packed() const
{
    int n = 0;
    for (const PackItem &i : items)
        if (i.status == PackItem::Packed) ++n;
    return n;
}

qint64 PackReport::packedBytes() const
{
    qint64 n = 0;
    for (const PackItem &i : items)
        if (i.status == PackItem::Packed) n += i.size;
    return n;
}

QString PackReport::text() const
{
    if (!ok) return error;
    QStringList lines;
    lines << QCoreApplication::translate("PackAndGo", "Saved %1.").arg(QDir::toNativeSeparators(path));
    lines << QCoreApplication::translate("PackAndGo", "%n linked file(s) packed.", nullptr, packed());
    lines << QCoreApplication::translate("PackAndGo", "%n font file(s) packed.", nullptr, int(fontsPacked.size()));
    if (!fontsLeftOut.isEmpty()) lines << fontsLeftOutText(fontsLeftOut);
    QStringList left;
    for (const PackItem &i : items)
        if (i.status != PackItem::Packed) left << QStringLiteral("%1 (%2)").arg(i.target.size() > 80 ? i.target.left(40) + QStringLiteral("...") + i.target.right(37) : i.target, i.why);
    if (!left.isEmpty()) {
        lines << QCoreApplication::translate("PackAndGo", "%n link(s) could not be packed and are as they were:", nullptr, int(left.size()));
        const int shown = int(std::min<qsizetype>(left.size(), kMostListed));
        for (int i = 0; i < shown; ++i) lines << QStringLiteral("  ") + left[i];
        if (left.size() > shown) lines << QCoreApplication::translate("PackAndGo", "and %n more.", nullptr, int(left.size() - shown));
    }
    lines << notes;
    return lines.join(QLatin1Char('\n'));
}

// ---------------- the package ----------------

namespace {

struct Planned {
    int item = -1;          // into the report
    QString file;
    qint64 size = 0;
};

QString mebibytes(qint64 bytes) { return QString::number(double(bytes) / (1024.0 * 1024.0), 'f', 1); }

} // namespace

PackReport packAndGo(PackAndGoSource &source, const QString &destination, PackMode mode, const PackLimits &limits, const PackAsk &ask, const QStringList &fontDirs)
{
    PackReport report;
    const QString documentPath = source.documentPath();
    const QString baseFolder = documentPath.isEmpty() ? QString() : QFileInfo(documentPath).absolutePath();
    report.notes = source.notes();

    // The links, each file once.
    QHash<QString, QString> moved;
    QHash<QString, int> byFile;   // canonical file -> index in `planned`
    QVector<Planned> planned;
    QVector<QString> plannedAs;
    QSet<QString> names;
    qint64 totalBytes = 0;
    int skippedForLimit = 0;
    const QVector<PackLink> links = source.links();
    for (const PackLink &link : links) {
        PackItem item;
        item.target = link.target;
        QString why;
        bool missing = false;
        const QString file = resolvePackLink(link.target, link.kind, baseFolder, documentPath, &why, &missing);
        if (file.isEmpty()) {
            item.status = missing ? PackItem::Missing : PackItem::Refused;
            item.why = why;
            report.items.push_back(item);
            continue;
        }
        item.file = file;
        const QString real = QFileInfo(file).canonicalFilePath();
        if (const auto known = byFile.constFind(real); known != byFile.cend()) {
            item.status = PackItem::Packed;
            item.packedAs = plannedAs[known.value()];
            item.size = 0;   // (counted once, with the first link to it)
            moved.insert(link.target, item.packedAs);
            report.items.push_back(item);
            continue;
        }
        const qint64 size = QFileInfo(file).size();
        if (int(planned.size()) >= limits.mostFiles || totalBytes + size > limits.mostBytes) {
            item.status = PackItem::Refused;
            item.why = QCoreApplication::translate("PackAndGo", "Pack and Go does not take more than this many files or this much in all.");
            ++skippedForLimit;
            report.items.push_back(item);
            continue;
        }
        item.status = PackItem::Packed;
        item.size = size;
        item.packedAs = QStringLiteral("linked/") + plainName(file, &names);
        totalBytes += size;
        byFile.insert(real, int(planned.size()));
        plannedAs.push_back(item.packedAs);
        planned.push_back({int(report.items.size()), file, size});
        moved.insert(link.target, item.packedAs);
        report.items.push_back(item);
    }
    if (planned.size() > limits.askFiles || totalBytes > limits.askBytes) {
        const QString question = QCoreApplication::translate("PackAndGo", "This will pack %n linked file(s), %1 MB in all. Pack them all?", nullptr, int(planned.size())).arg(mebibytes(totalBytes));
        if (!ask || !ask(question)) {
            report.cancelled = true;
            report.error = QCoreApplication::translate("PackAndGo", "Nothing was saved.");
            return report;
        }
    }

    // The fonts.
    const PackedFonts fonts = packFonts(source.fontFamilies(), fontDirs);
    report.fontsLeftOut = fonts.leftOut;
    for (const PackFontFile &f : fonts.files) report.fontsPacked << f.name;

    // The copy with its links moved inside.
    QByteArray document;
    QString err;
    if (!source.saveCopy(moved, &document, &err)) {
        report.error = err.isEmpty() ? QCoreApplication::translate("PackAndGo", "The copy couldn't be made.") : err;
        return report;
    }
    const QString docName = [&] {
        QString n = QFileInfo(source.documentName()).fileName();
        return n.isEmpty() ? QStringLiteral("document") : n;
    }();

    // Read each file, once, as it is written (a file that is larger than it said is not taken).
    auto readFile = [&](const Planned &p, QByteArray *data) {
        QFile f(p.file);
        if (!f.open(QIODevice::ReadOnly)) return false;
        *data = f.read(p.size + 1);
        return data->size() == p.size;
    };
    auto notPacked = [&](const Planned &p, const QString &why) {
        PackItem &i = report.items[p.item];
        i.status = PackItem::Refused;
        i.why = why;
    };

    if (mode == PackMode::Zip) {
        ZipWriter z;
        z.add(docName, document);
        for (const PackFontFile &f : fonts.files) z.add(QStringLiteral("fonts/") + f.name, f.data);
        for (const Planned &p : planned) {
            QByteArray data;
            if (!readFile(p, &data)) {
                notPacked(p, QCoreApplication::translate("PackAndGo", "The file could not be read."));
                continue;
            }
            z.add(report.items[p.item].packedAs, data);
        }
        const QByteArray bytes = z.finish();
        QString detail;
        if (!writeZipFile(destination, bytes, &detail)) {
            report.error = QCoreApplication::translate("PackAndGo", "Pack and Go couldn't save %1.\n%2").arg(QDir::toNativeSeparators(destination), detail);
            return report;
        }
        report.path = destination;
    } else {
        const QString base = QFileInfo(docName).completeBaseName();
        QString folder = QDir(destination).filePath(base);
        for (int n = 2; QFileInfo::exists(folder) && n < 1000; ++n) folder = QDir(destination).filePath(base + QLatin1Char(' ') + QString::number(n));
        auto fail = [&](const QString &what) {
            report.error = QCoreApplication::translate("PackAndGo", "Pack and Go couldn't save %1.\n%2").arg(QDir::toNativeSeparators(folder), what);
            return report;
        };
        if (QFileInfo::exists(folder) || !QDir().mkpath(folder + QStringLiteral("/linked")) || !QDir().mkpath(folder + QStringLiteral("/fonts"))) return fail(QCoreApplication::translate("PackAndGo", "The folder couldn't be made."));
        auto put = [&](const QString &name, const QByteArray &data) {
            QSaveFile out(folder + QLatin1Char('/') + name);
            return out.open(QIODevice::WriteOnly) && out.write(data) == data.size() && out.commit();
        };
        if (!put(docName, document)) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
        for (const PackFontFile &f : fonts.files)
            if (!put(QStringLiteral("fonts/") + f.name, f.data)) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
        for (const Planned &p : planned) {
            QByteArray data;
            if (!readFile(p, &data)) {
                notPacked(p, QCoreApplication::translate("PackAndGo", "The file could not be read."));
                continue;
            }
            if (!put(report.items[p.item].packedAs, data)) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
        }
        // (no empty folders left behind)
        QDir(folder).rmdir(QStringLiteral("linked"));
        QDir(folder).rmdir(QStringLiteral("fonts"));
        report.path = folder;
    }
    report.ok = true;
    return report;
}

// ---------------- the commands ----------------

void addPackAndGoCards(QVBoxLayout *layout, QWidget *page, const std::function<void(bool toFolder)> &run)
{
    auto *zip = new ActionCard(QStringLiteral("hard-drive"), QCoreApplication::translate("PackAndGo", "Pack and Go: Save for Another Computer"),
                               QCoreApplication::translate("PackAndGo", "The file, with the pictures, movies, sounds, and other files it links to and the fonts it uses that come with the program, in one ZIP file."), page);
    zip->setObjectName(QStringLiteral("packAndGoZip"));
    QObject::connect(zip, &QAbstractButton::clicked, page, [run] { run(false); });
    layout->addWidget(zip);
    auto *folder = new ActionCard(QStringLiteral("folder-output"), QCoreApplication::translate("PackAndGo", "Pack and Go: Save to a Folder"),
                                  QCoreApplication::translate("PackAndGo", "The same, in a new folder you can copy to a drive."), page);
    folder->setObjectName(QStringLiteral("packAndGoFolder"));
    QObject::connect(folder, &QAbstractButton::clicked, page, [run] { run(true); });
    layout->addWidget(folder);
}

QString askPackAndGoDestination(QWidget *parent, const QString &documentName, bool toFolder)
{
    if (toFolder) return QFileDialog::getExistingDirectory(parent, QCoreApplication::translate("PackAndGo", "Save to a Folder"));
    return askSavePath(parent, QCoreApplication::translate("PackAndGo", "Save for Another Computer"), QFileInfo(documentName).completeBaseName() + QStringLiteral(".zip"),
                       QCoreApplication::translate("PackAndGo", "ZIP (*.zip)"));
}

PackAsk packAndGoAsk(QWidget *parent)
{
    return [parent](const QString &question) { return plainQuestion(parent, QCoreApplication::translate("PackAndGo", "Pack and Go"), question) == QMessageBox::Yes; };
}

bool showPackAndGoReport(QWidget *parent, const PackReport &report)
{
    const QString title = QCoreApplication::translate("PackAndGo", "Pack and Go");
    if (report.cancelled) return false;
    if (!report.ok) {
        plainWarning(parent, title, report.text());
        return false;
    }
    plainInformation(parent, title, report.text());
    return true;
}

} // namespace jp
