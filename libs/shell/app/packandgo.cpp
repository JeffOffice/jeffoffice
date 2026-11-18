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
#include <QTemporaryDir>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>
#include <cstring>
#include <utility>

namespace jp {

namespace {

constexpr int kMostLinks = 100000;                        // links found in a package
constexpr int kMostFamilies = 2000;                       // fonts named in a package
constexpr int kMostLinkText = 4096;                       // one link's text
constexpr int kMostInstructionText = 65536;               // one field's instruction, joined from its runs
constexpr int kMostListed = 20;                           // links named in the report
constexpr qint64 kFileChunk = qint64(1) << 20;            // a linked file is read this much at a time

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

// The scans below work on the part's bytes (UTF-8), never on a copy of the whole part as text.
bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
bool isNameChar(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ':' || c == '_' || c == '.' || c == '-'; }

// One attribute of a tag that begins at `at` (just after its name): where its value is in the part (not including the quotes).
struct Attr {
    QByteArray name;
    qsizetype valueStart = 0, valueLength = 0;
};

// The attributes of the tag whose name ends at `at`; `end` gets the place of the tag's closing '>' (-1 when the part ends first).
QVector<Attr> attributesAt(const QByteArray &xml, qsizetype at, qsizetype *end)
{
    QVector<Attr> out;
    const qsizetype n = xml.size();
    qsizetype i = at;
    *end = -1;
    while (i < n) {
        const char c = xml[i];
        if (c == '>') {
            *end = i;
            break;
        }
        if (!isNameChar(c)) {
            ++i;
            continue;
        }
        const qsizetype nameStart = i;
        while (i < n && isNameChar(xml[i])) ++i;
        qsizetype j = i;
        while (j < n && isSpace(xml[j])) ++j;
        if (j >= n || xml[j] != '=') continue;
        ++j;
        while (j < n && isSpace(xml[j])) ++j;
        if (j >= n || (xml[j] != '"' && xml[j] != '\'')) {
            i = j;
            continue;
        }
        const qsizetype close = xml.indexOf(xml[j], j + 1);
        if (close < 0) break;
        Attr a;
        a.name = xml.mid(nameStart, i - nameStart);
        a.valueStart = j + 1;
        a.valueLength = close - j - 1;
        out.push_back(a);
        i = close + 1;
    }
    return out;
}

QString valueOf(const QByteArray &xml, const Attr &a) { return unescapeXml(QString::fromUtf8(xml.constData() + a.valueStart, a.valueLength)); }

// One Relationship element: its attributes, and where the value of Target is in the part.
struct Rel {
    QString type, target;
    bool external = false;
    qsizetype targetStart = -1, targetLength = 0;
};

QVector<Rel> relationshipsIn(const QByteArray &xml)
{
    static const QByteArray tag("<Relationship");
    QVector<Rel> out;
    qsizetype pos = 0;
    while (out.size() < kMostLinks) {
        pos = xml.indexOf(tag, pos);
        if (pos < 0) break;
        const qsizetype nameEnd = pos + tag.size();
        if (nameEnd >= xml.size()) break;
        if (isNameChar(xml[nameEnd])) {   // (<Relationships>)
            pos = nameEnd;
            continue;
        }
        qsizetype end = -1;
        const QVector<Attr> attrs = attributesAt(xml, nameEnd, &end);
        Rel r;
        for (const Attr &a : attrs) {
            if (a.name == "Type") r.type = valueOf(xml, a);
            else if (a.name == "TargetMode") r.external = valueOf(xml, a).compare(QLatin1String("External"), Qt::CaseInsensitive) == 0;
            else if (a.name == "Target") {
                r.target = valueOf(xml, a);
                r.targetStart = a.valueStart;
                r.targetLength = a.valueLength;
            }
        }
        out.push_back(r);
        if (end < 0) break;
        pos = end + 1;
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

// A field's instruction: the text of the runs between one field mark and the next, joined (a field's text is split across runs wherever the program that wrote it chose), or the
// instr attribute of a simple field.
struct Instr {
    QVector<std::pair<qsizetype, qsizetype>> runs;   // where each piece of the value is in the part (start, length), as the part spells it
    QString value;                                   // joined, unescaped
    bool attribute = false;
};

QVector<Instr> instructionsIn(const QByteArray &xml)
{
    static const QByteArray endTag("</w:instrText>");
    QVector<Instr> out;
    Instr cur;
    auto closeField = [&] {
        if (!cur.runs.isEmpty() && cur.value.contains(QLatin1String("INCLUDE"), Qt::CaseInsensitive) && out.size() < kMostLinks) out.push_back(cur);
        cur = Instr();
    };
    auto isTag = [&](qsizetype at, const char *name) {
        const qsizetype len = qsizetype(strlen(name));
        return xml.size() > at + len && QByteArrayView(xml).mid(at, len) == QByteArrayView(name, len) && !isNameChar(xml[at + len]);
    };
    qsizetype pos = 0;
    while (pos >= 0 && pos < xml.size()) {
        pos = xml.indexOf("<w:", pos);
        if (pos < 0) break;
        const qsizetype at = pos + 3;
        if (isTag(at, "instrText")) {
            const qsizetype open = xml.indexOf('>', at);
            if (open < 0) break;
            pos = open + 1;
            if (xml[open - 1] == '/') continue;
            const qsizetype close = xml.indexOf('<', open + 1);
            if (close < 0) break;
            if (QByteArrayView(xml).mid(close, endTag.size()) != QByteArrayView(endTag)) {
                pos = close;
                continue;
            }
            pos = close + endTag.size();
            const qsizetype length = close - open - 1;
            if (cur.value.size() + length > kMostInstructionText) continue;
            cur.runs.push_back({open + 1, length});
            cur.value += unescapeXml(QString::fromUtf8(xml.constData() + open + 1, length));
        } else if (isTag(at, "fldChar")) {
            closeField();
            pos = at;
        } else if (isTag(at, "fldSimple")) {
            closeField();
            qsizetype end = -1;
            const QVector<Attr> attrs = attributesAt(xml, at + 9, &end);
            for (const Attr &a : attrs)
                if (a.name == "w:instr") {
                    cur.runs.push_back({a.valueStart, a.valueLength});
                    cur.value = valueOf(xml, a);
                    cur.attribute = true;
                    closeField();
                    break;
                }
            if (end < 0) break;
            pos = end + 1;
        } else {
            pos = at;
        }
    }
    closeField();
    return out;
}

// The parts that can hold a field with a file in it: the body and the headers and footers.
bool isFieldPart(const QString &name)
{
    if (!name.endsWith(QLatin1String(".xml"))) return false;
    return name == QLatin1String("word/document.xml") || (name.startsWith(QLatin1String("word/header")) && name.indexOf(QLatin1Char('/'), 5) < 0) ||
           (name.startsWith(QLatin1String("word/footer")) && name.indexOf(QLatin1Char('/'), 5) < 0);
}

bool mayHoldFields(const QByteArray &xml) { return xml.contains("<w:instrText") || xml.contains("<w:fldSimple"); }

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

PackStats &packStats()
{
    static PackStats stats;
    return stats;
}

PackReadCaps &packReadCaps()
{
    static PackReadCaps caps;
    return caps;
}

// ---------------- the package scan ----------------

namespace {

// What a scan of a package finds: the files it links to and the fonts it names.
struct Found {
    QVector<PackLink> links;
    QStringList fonts;
};

// Collects the links of one part (relationships, and fields in the parts that can have them), each target once.
struct LinkCollector {
    QVector<PackLink> out;
    QSet<QString> seen;
    void add(const QString &name, const QByteArray &data)
    {
        if (out.size() >= kMostLinks) return;
        if (name.endsWith(QLatin1String(".rels"), Qt::CaseInsensitive)) {
            for (const Rel &r : relationshipsIn(data)) {
                PackKind kind;
                if (!r.external || r.target.isEmpty() || r.target.size() > kMostLinkText || hasOtherScheme(r.target) || !kindOfRelationship(r.type, &kind)) continue;
                if (kind == PackKind::Movie && allowedSuffixes(PackKind::Sound).contains(QFileInfo(r.target).suffix().toLower()) && !allowedSuffixes(PackKind::Movie).contains(QFileInfo(r.target).suffix().toLower()))
                    kind = PackKind::Sound;
                if (seen.contains(r.target)) continue;
                seen.insert(r.target);
                out.push_back({r.target, kind});
            }
        } else if (isFieldPart(name) && mayHoldFields(data)) {
            for (const Instr &i : instructionsIn(data))
                for (const FieldLink &f : fieldLinksIn(i.value)) {
                    if (f.path.isEmpty() || f.path.size() > kMostLinkText || hasOtherScheme(f.path) || seen.contains(f.path)) continue;
                    seen.insert(f.path);
                    out.push_back({f.path, f.text ? PackKind::Document : PackKind::Picture});
                }
        }
    }
};

// The value of the attribute `name` in the tag at `at` (after the tag's name), or an empty array.
QByteArray attributeValue(const QByteArray &xml, qsizetype at, const char *name)
{
    qsizetype end = -1;
    for (const Attr &a : attributesAt(xml, at, &end))
        if (a.name == name) return xml.mid(a.valueStart, a.valueLength);
    return QByteArray();
}

// Collects the font families a part names: drawing typefaces, run fonts, and the names in cell styles.
struct FontCollector {
    QStringList out;
    QSet<QString> seen;
    void addValue(const QByteArray &raw)
    {
        const QString f = unescapeXml(QString::fromUtf8(raw)).trimmed();
        if (f.isEmpty() || f.startsWith(QLatin1Char('+')) || out.size() >= kMostFamilies || seen.contains(f.toLower())) return;
        seen.insert(f.toLower());
        out.push_back(f);
    }
    void add(const QString &name, const QByteArray &xml)
    {
        if (!name.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) return;
        const bool word = name.startsWith(QLatin1String("word/")), sheet = name.startsWith(QLatin1String("xl/"));
        if (!(word || sheet || name.startsWith(QLatin1String("ppt/")))) return;
        auto tagIs = [&](qsizetype at, const char *tag) {
            const qsizetype len = qsizetype(strlen(tag));
            return xml.size() > at + len && QByteArrayView(xml).mid(at, len) == QByteArrayView(tag, len) && !isNameChar(xml[at + len]);
        };
        qsizetype pos = 0;
        while (out.size() < kMostFamilies) {
            pos = xml.indexOf('<', pos);
            if (pos < 0 || pos + 1 >= xml.size()) break;
            const qsizetype at = pos + 1;
            ++pos;
            if (xml[at] == 'a' && at + 1 < xml.size() && xml[at + 1] == ':') {
                for (const char *tag : {"a:latin", "a:ea", "a:cs", "a:sym"})
                    if (tagIs(at, tag)) {
                        addValue(attributeValue(xml, at + qsizetype(strlen(tag)), "typeface"));
                        break;
                    }
            } else if (word && tagIs(at, "w:rFonts")) {
                for (const char *attr : {"w:ascii", "w:hAnsi", "w:eastAsia", "w:cs"}) addValue(attributeValue(xml, at + 8, attr));
            } else if (sheet && (tagIs(at, "name") || tagIs(at, "rFont"))) {
                addValue(attributeValue(xml, at + (xml[at] == 'n' ? 4 : 5), "val"));
            }
        }
    }
};

bool partProblem(const ZipScanEntry &e, QString *error)
{
    if (e.whole) return false;
    if (error) *error = QCoreApplication::translate("PackAndGo", "Pack and Go could not read the part \"%1\" of the file (%2), so nothing was saved.").arg(e.name, e.problem.isEmpty() ? QCoreApplication::translate("PackAndGo", "damaged") : e.problem);
    return true;
}

// One pass over a package's parts, a part at a time (the part is not kept). `each` is given a part that was read whole. False, with `error`, when the package is not one it can read: a part
// that does not inflate (damaged, or over a cap) is a failure, never a part left out.
bool eachPart(const QByteArray &zip, const std::function<void(ZipScanEntry &)> &each, QString *error)
{
    const PackReadCaps caps = packReadCaps();
    ++packStats().zipReads;
    ZipScanStats stats;
    QString problem;
    bool any = false;
    scanZipEach(
        zip,
        [&](ZipScanEntry &e) {
            any = true;
            if (!problem.isEmpty()) return;
            if (partProblem(e, &problem)) return;
            packStats().largestPartHeld = std::max<qint64>(packStats().largestPartHeld, e.data.size());
            each(e);
        },
        caps.entries, caps.part, caps.total, &stats);
    if (!problem.isEmpty()) {
        if (error) *error = problem;
        return false;
    }
    if (!any || stats.entriesLeftOut || stats.sizeLeftOut) {
        if (error) *error = QCoreApplication::translate("PackAndGo", "The file is too large or is not a package Pack and Go can read.");
        return false;
    }
    return true;
}

bool scanPackage(const QByteArray &zip, Found *found, QString *error)
{
    LinkCollector links;
    FontCollector fonts;
    if (!eachPart(zip, [&](ZipScanEntry &e) {
            links.add(e.name, e.data);
            fonts.add(e.name, e.data);
        }, error))
        return false;
    found->links = links.out;
    found->fonts = fonts.out;
    return true;
}

// A part with each link in `moved` changed: one forward pass that puts the pieces together (the text between the links, and the new targets).
QByteArray linksMovedInRelationships(const QByteArray &xml, const QHash<QString, QString> &moved, bool *changed)
{
    QByteArray out;
    qsizetype from = 0;
    for (const Rel &r : relationshipsIn(xml)) {
        if (!r.external || r.targetStart < 0 || !moved.contains(r.target)) continue;
        if (!*changed) out.reserve(xml.size() + 1024);
        *changed = true;
        out.append(xml.constData() + from, r.targetStart - from);
        out.append(escapeXml(moved.value(r.target)).toUtf8());
        from = r.targetStart + r.targetLength;
        packStats().rewriteSlices += 2;
    }
    if (!*changed) return QByteArray();
    out.append(xml.constData() + from, xml.size() - from);
    ++packStats().rewriteSlices;
    packStats().rewriteBytesIn += xml.size();
    return out;
}

QByteArray linksMovedInFields(const QByteArray &xml, const QHash<QString, QString> &moved, bool *changed)
{
    QByteArray out;
    qsizetype from = 0;
    for (const Instr &i : instructionsIn(xml)) {
        const QVector<FieldLink> fields = fieldLinksIn(i.value);
        QString value;
        qsizetype valueFrom = 0;
        bool here = false;
        for (const FieldLink &f : fields) {
            if (!moved.contains(f.path)) continue;
            // (the escaping a quoted name has is undone when read: write it back the same way)
            value += QStringView(i.value).mid(valueFrom, f.start - valueFrom);
            value += f.quoted ? fieldPathText(moved.value(f.path)) : moved.value(f.path);
            valueFrom = f.start + f.length;
            here = true;
        }
        if (!here) continue;
        value += QStringView(i.value).mid(valueFrom);
        if (!*changed) out.reserve(xml.size() + 1024);
        *changed = true;
        // (the whole instruction goes in its first run, and the others are emptied)
        for (qsizetype k = 0; k < i.runs.size(); ++k) {
            out.append(xml.constData() + from, i.runs[k].first - from);
            if (k == 0) out.append(escapeXml(value, i.attribute).toUtf8());
            from = i.runs[k].first + i.runs[k].second;
            packStats().rewriteSlices += 2;
        }
    }
    if (!*changed) return QByteArray();
    out.append(xml.constData() + from, xml.size() - from);
    ++packStats().rewriteSlices;
    packStats().rewriteBytesIn += xml.size();
    return out;
}

// The package with each link in `moved` changed, a part at a time.
QByteArray movedPackage(const QByteArray &zip, const QHash<QString, QString> &moved, QString *error)
{
    ZipWriter z;
    if (!eachPart(zip, [&](ZipScanEntry &e) {
            bool changed = false;
            QByteArray out;
            if (e.name.endsWith(QLatin1String(".rels"), Qt::CaseInsensitive)) out = linksMovedInRelationships(e.data, moved, &changed);
            else if (isFieldPart(e.name) && mayHoldFields(e.data)) out = linksMovedInFields(e.data, moved, &changed);
            z.add(e.name, changed ? out : e.data, true);
        }, error))
        return QByteArray();
    return z.finish();
}

} // namespace

bool OpcPackSource::have(QString *error)
{
    if (m_have) return true;
    QString err;
    const QByteArray bytes = savedBytes(&err);
    if (bytes.isEmpty()) {
        if (error) *error = err.isEmpty() ? QCoreApplication::translate("PackAndGo", "The copy couldn't be made.") : err;
        return false;
    }
    Found found;
    if (!scanPackage(bytes, &found, &err)) {
        if (error) *error = err;
        return false;
    }
    m_bytes = bytes;
    m_links = found.links;
    m_fonts = found.fonts;
    m_have = true;
    return true;
}

QVector<PackLink> OpcPackSource::linksOf(const QByteArray &package, QString *error)
{
    Found found;
    return scanPackage(package, &found, error) ? found.links : QVector<PackLink>();
}

QStringList OpcPackSource::fontsOf(const QByteArray &package)
{
    Found found;
    return scanPackage(package, &found, nullptr) ? found.fonts : QStringList();
}

QByteArray OpcPackSource::withLinks(const QByteArray &package, const QHash<QString, QString> &moved, QString *error)
{
    return movedPackage(package, moved, error);
}

QStringList OpcPackSource::fontFamilies()
{
    return have(nullptr) ? m_fonts : QStringList();
}

QVector<PackLink> OpcPackSource::links()
{
    return have(nullptr) ? m_links : QVector<PackLink>();
}

bool OpcPackSource::saveCopy(const QHash<QString, QString> &moved, QByteArray *bytes, QString *error)
{
    if (!have(error)) return false;
    // (with nothing to move, the package is not read again or made again)
    QByteArray out = moved.isEmpty() ? m_bytes : movedPackage(m_bytes, moved, error);
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
            // (a file that cannot be inspected, a collection or a damaged or very large one, is packed as it always was: only a license that is read and forbids it leaves a font out)
            FontFileInfo info;
            if (inspectFont(data, &info) && !fontAllowsEmbedding(info.fsType)) {
                const QString family = info.family.isEmpty() ? stem : info.family;
                if (!out.leftOut.contains(family)) out.leftOut.push_back(family);
                continue;
            }
            out.files.push_back({fi.fileName(), data});
        }
        if (!out.files.isEmpty()) break;
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
    // A picture, movie, or sound may be anywhere (a whole path is the usual link); a document, workbook, or object travels only from the document's own folder, so that a file cannot
    // pull in the person's other documents.
    const bool stays = kind == PackKind::Document || kind == PackKind::Workbook || kind == PackKind::Object;
    auto leaves = [&] { return no(QCoreApplication::translate("PackAndGo", "The link leaves the document's folder.")); };
    if (relative) {
        if (base.isEmpty()) return no(QCoreApplication::translate("PackAndGo", "The document has not been saved, so there is no folder to find the file in."), true);
        path = QDir::cleanPath(base + QLatin1Char('/') + path);
        // (a relative link stays in the document's folder: it may not go up out of it)
        if (!path.startsWith(base.endsWith(QLatin1Char('/')) ? base : base + QLatin1Char('/'))) return leaves();
    } else {
        path = QDir::cleanPath(path);
        if (stays && base.isEmpty()) return leaves();
    }
    if (isOnAnotherComputer(path) || isWindowsSpecialPath(path)) return no(QCoreApplication::translate("PackAndGo", "The link is not a file Pack and Go takes."));
    const QFileInfo info(path);
    if (!info.exists()) return no(QCoreApplication::translate("PackAndGo", "The file was not found."), true);
    if (!info.isFile()) return no(QCoreApplication::translate("PackAndGo", "The link is not a file."));
    if (!documentPath.isEmpty() && info.canonicalFilePath() == QFileInfo(documentPath).canonicalFilePath()) return no(QCoreApplication::translate("PackAndGo", "The link is to the document itself."));
    if (!allowedSuffixes(kind).contains(info.suffix().toLower())) return no(QCoreApplication::translate("PackAndGo", "This is not a kind of file Pack and Go takes."));
    // (where the file really is, past any symbolic link, is what is looked at: a link may not lead out of the folder, nor to another kind of file)
    const QString real = info.canonicalFilePath();
    if (real.isEmpty()) return no(QCoreApplication::translate("PackAndGo", "The file was not found."), true);
    if (relative || stays) {
        const QString realBase = QDir(base).canonicalPath();
        if (realBase.isEmpty() || !real.startsWith(realBase.endsWith(QLatin1Char('/')) ? realBase : realBase + QLatin1Char('/'))) return leaves();
    }
    if (!allowedSuffixes(kind).contains(QFileInfo(real).suffix().toLower())) return no(QCoreApplication::translate("PackAndGo", "This is not a kind of file Pack and Go takes."));
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
    lines << QCoreApplication::translate("PackAndGo", "Linked files packed: %1.").arg(packed());
    lines << QCoreApplication::translate("PackAndGo", "Font files packed: %1.").arg(fontsPacked.size());
    if (!fontsLeftOut.isEmpty()) lines << fontsLeftOutText(fontsLeftOut);
    QStringList left;
    for (const PackItem &i : items)
        if (i.status != PackItem::Packed) left << QStringLiteral("%1 (%2)").arg(i.target.size() > 80 ? i.target.left(40) + QStringLiteral("...") + i.target.right(37) : i.target, i.why);
    if (!left.isEmpty()) {
        lines << QCoreApplication::translate("PackAndGo", "Links that could not be packed, left as they were (%1):").arg(left.size());
        const int shown = int(std::min<qsizetype>(left.size(), kMostListed));
        for (int i = 0; i < shown; ++i) lines << QStringLiteral("  ") + left[i];
        if (left.size() > shown) lines << QCoreApplication::translate("PackAndGo", "And %1 more.").arg(left.size() - shown);
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
        const QString question = QCoreApplication::translate("PackAndGo", "This will pack %1 linked files, %2 MB in all. Pack them all?").arg(planned.size()).arg(mebibytes(totalBytes));
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

    auto notPacked = [&](const Planned &p, const QString &why) {
        PackItem &i = report.items[p.item];
        i.status = PackItem::Refused;
        i.why = why;
    };
    // A file is read once, in pieces, as it is written (it is never held whole); one that is not the size it was when it was found has changed under us, and stops the Pack and Go.
    auto streamFile = [&](QFile &f, qint64 size, const std::function<bool(const char *, qint64)> &sink) {
        QByteArray buffer(int(std::min<qint64>(std::max<qint64>(size, 1), kFileChunk)), Qt::Uninitialized);
        packStats().largestFileChunk = std::max<qint64>(packStats().largestFileChunk, buffer.size());
        qint64 left = size;
        while (left > 0) {
            const qint64 got = f.read(buffer.data(), std::min<qint64>(left, buffer.size()));
            if (got <= 0 || !sink(buffer.constData(), got)) return false;
            left -= got;
        }
        char more = 0;
        return f.read(&more, 1) == 0;
    };
    auto changed = [&](const Planned &p) {
        report.error = QCoreApplication::translate("PackAndGo", "Pack and Go stopped, because %1 changed while it was being copied. Nothing was saved.").arg(QDir::toNativeSeparators(p.file));
        return report;
    };

    if (mode == PackMode::Zip) {
        ZipWriter z;
        z.add(docName, document);
        for (const PackFontFile &f : fonts.files) z.add(QStringLiteral("fonts/") + f.name, f.data);
        for (const Planned &p : planned) {
            QFile f(p.file);
            if (!f.open(QIODevice::ReadOnly)) {
                notPacked(p, QCoreApplication::translate("PackAndGo", "The file could not be read."));
                continue;
            }
            z.beginStream(report.items[p.item].packedAs, false);
            if (!streamFile(f, p.size, [&](const char *data, qint64 n) {
                    z.write(data, qsizetype(n));
                    return true;
                }))
                return changed(p);
            z.endStream();
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
        // Written in a folder of its own beside the place it is going, and moved there when whole: a failure leaves nothing half made (the folder removes itself).
        QDir().mkpath(destination);
        QTemporaryDir work(QDir(destination).filePath(QStringLiteral(".packandgo-XXXXXX")));
        if (QFileInfo::exists(folder) || !work.isValid() || !QDir().mkpath(work.path() + QStringLiteral("/linked")) || !QDir().mkpath(work.path() + QStringLiteral("/fonts")))
            return fail(QCoreApplication::translate("PackAndGo", "The folder couldn't be made."));
        auto put = [&](const QString &name, const QByteArray &data) {
            QFile out(work.path() + QLatin1Char('/') + name);
            const bool ok = out.open(QIODevice::WriteOnly) && out.write(data) == data.size();
            return ok && out.flush() && (out.close(), out.error() == QFileDevice::NoError);
        };
        if (!put(docName, document)) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
        for (const PackFontFile &f : fonts.files)
            if (!put(QStringLiteral("fonts/") + f.name, f.data)) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
        for (const Planned &p : planned) {
            QFile f(p.file);
            if (!f.open(QIODevice::ReadOnly)) {
                notPacked(p, QCoreApplication::translate("PackAndGo", "The file could not be read."));
                continue;
            }
            QFile out(work.path() + QLatin1Char('/') + report.items[p.item].packedAs);
            if (!out.open(QIODevice::WriteOnly)) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
            bool wrote = true;
            const bool whole = streamFile(f, p.size, [&](const char *data, qint64 n) { return wrote = out.write(data, n) == n; });
            out.close();
            if (!wrote) return fail(QCoreApplication::translate("PackAndGo", "The file couldn't be written."));
            if (!whole) return changed(p);
        }
        // (no empty folders left behind)
        QDir(work.path()).rmdir(QStringLiteral("linked"));
        QDir(work.path()).rmdir(QStringLiteral("fonts"));
        bool moved = false;
        for (int n = 2; !moved && n < 1000; ++n) {
            if (!QFileInfo::exists(folder) && QDir().rename(work.path(), folder)) moved = true;
            else folder = QDir(destination).filePath(base + QLatin1Char(' ') + QString::number(n));
        }
        if (!moved) return fail(QCoreApplication::translate("PackAndGo", "The folder couldn't be made."));
        work.setAutoRemove(false);
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
