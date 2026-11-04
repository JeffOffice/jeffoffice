#include "office/repair.h"

#include "io/zip.h"
#include "office/xml.h"
#include "office/xmlstream.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QXmlStreamReader>

#include <algorithm>
#include <array>
#include <vector>

namespace jp::office {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::office::Repair", text); }

bool isXmlName(const QString &name)
{
    const QString lower = name.toLower();
    return lower.endsWith(QLatin1String(".xml")) || lower.endsWith(QLatin1String(".rels")) || lower.endsWith(QLatin1String(".vml"));
}

QString plainName(QString name)
{
    name.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (name.startsWith(QLatin1Char('/'))) name.remove(0, 1);
    return name;
}

// A name that stays inside the package: not empty, and no "." or ".." among its folders (a name from a stranger's file is never used to reach another place).
bool usableName(const QString &name)
{
    if (name.isEmpty() || name.contains(QChar(0))) return false;
    for (const QString &part : name.split(QLatin1Char('/')))
        if (part == QLatin1String(".") || part == QLatin1String("..")) return false;
    return true;
}

// Parts a program reads as a stream, so it opens them at any size a file can have (an XML part of another kind is held to kMaxXmlBytes, as an open holds it).
bool readAsStream(const QString &name)
{
    const QString lower = name.toLower();
    return lower.startsWith(QLatin1String("xl/worksheets/")) || lower == QLatin1String("xl/sharedstrings.xml");
}

const char kRelsNs[] = "http://schemas.openxmlformats.org/package/2006/relationships";
const char kRelBase[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

struct KnownPart {
    const char *pattern;   // the part's name, whole
    const char *type;
    const char *relation;  // the type of the link from the main part, when it has one a reader needs
};

const KnownPart kKnown[] = {
    {"word/document\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml", nullptr},
    {"word/styles\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml", "styles"},
    {"word/numbering\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml", "numbering"},
    {"word/settings\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml", "settings"},
    {"word/fontTable\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml", "fontTable"},
    {"word/footnotes\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml", "footnotes"},
    {"word/endnotes\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.endnotes+xml", "endnotes"},
    {"word/comments\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.comments+xml", "comments"},
    {"word/header[0-9]*\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml", nullptr},
    {"word/footer[0-9]*\\.xml", "application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml", nullptr},
    {"(word|xl|ppt)/theme/theme[0-9]*\\.xml", "application/vnd.openxmlformats-officedocument.theme+xml", "theme"},
    {"xl/workbook\\.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml", nullptr},
    {"xl/worksheets/sheet[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml", nullptr},
    {"xl/sharedStrings\\.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml", "sharedStrings"},
    {"xl/styles\\.xml", "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml", "styles"},
    {"xl/drawings/drawing[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.drawing+xml", nullptr},
    {"xl/charts/chart[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.drawingml.chart+xml", nullptr},
    {"ppt/presentation\\.xml", "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml", nullptr},
    {"ppt/slides/slide[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.presentationml.slide+xml", nullptr},
    {"ppt/slideLayouts/slideLayout[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml", nullptr},
    {"ppt/slideMasters/slideMaster[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml", nullptr},
    {"ppt/notesSlides/notesSlide[0-9]+\\.xml", "application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml", nullptr},
    {"docProps/core\\.xml", "application/vnd.openxmlformats-package.core-properties+xml", nullptr},
    {"docProps/app\\.xml", "application/vnd.openxmlformats-officedocument.extended-properties+xml", nullptr},
};

const KnownPart *knownPart(const QString &name)
{
    // (each pattern is compiled once: a package has hundreds of parts, and each is looked up here)
    static const std::vector<QRegularExpression> patterns = [] {
        std::vector<QRegularExpression> all;
        for (const KnownPart &k : kKnown) all.emplace_back(QStringLiteral("^(%1)$").arg(QLatin1String(k.pattern)), QRegularExpression::CaseInsensitiveOption);
        return all;
    }();
    for (size_t i = 0; i < patterns.size(); ++i)
        if (patterns[i].match(name).hasMatch()) return &kKnown[i];
    return nullptr;
}

QString mainPartOf(const QSet<QString> &parts)
{
    for (const char *name : {"word/document.xml", "xl/workbook.xml", "ppt/presentation.xml"})
        if (parts.contains(QLatin1String(name))) return QLatin1String(name);
    return QString();
}

QByteArray typesXml(const QStringList &parts)
{
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                   "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                   "<Default Extension=\"png\" ContentType=\"image/png\"/><Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/><Default Extension=\"jpg\" ContentType=\"image/jpeg\"/>"
                   "<Default Extension=\"gif\" ContentType=\"image/gif\"/><Default Extension=\"bmp\" ContentType=\"image/bmp\"/><Default Extension=\"tiff\" ContentType=\"image/tiff\"/>"
                   "<Default Extension=\"emf\" ContentType=\"image/x-emf\"/><Default Extension=\"wmf\" ContentType=\"image/x-wmf\"/><Default Extension=\"svg\" ContentType=\"image/svg+xml\"/>";
    for (const QString &name : parts)
        if (const KnownPart *k = knownPart(name)) x += "<Override PartName=\"/" + name.toUtf8() + "\" ContentType=\"" + k->type + "\"/>";
    return x + "</Types>";
}

QByteArray relsXml(const QList<std::array<QString, 3>> &links)   // id, type suffix, target
{
    QByteArray x = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"") + kRelsNs + "\">";
    for (const auto &l : links)
        x += "<Relationship Id=\"" + l[0].toUtf8() + "\" Type=\"" + kRelBase + l[1].toUtf8() + "\" Target=\"" + l[2].toUtf8() + "\"/>";
    return x + "</Relationships>";
}

// The links a main part needs and a reader looks for, made from the parts beside it when its own list of links is gone.
QByteArray mainRels(const QString &main, const QStringList &parts)
{
    const QString folder = main.left(main.lastIndexOf(QLatin1Char('/')) + 1);
    QList<std::array<QString, 3>> links;
    int n = 0;
    auto add = [&](const QString &type, const QString &target) { links.append({QStringLiteral("rId%1").arg(++n), type, target}); };
    if (main == QLatin1String("xl/workbook.xml")) {
        // (the sheets are named in the workbook by the links' Ids: they are taken in the order the sheets' parts are numbered, which is how a program writes them)
        QStringList sheets;
        static const QRegularExpression sheetName(QStringLiteral("^xl/worksheets/sheet[0-9]+\\.xml$"));
        for (const QString &name : parts)
            if (sheetName.match(name).hasMatch()) sheets << name;
        std::sort(sheets.begin(), sheets.end(), [](const QString &a, const QString &b) {
            static const QRegularExpression digits(QStringLiteral("[0-9]+"));
            return digits.match(a, 12).captured().toInt() < digits.match(b, 12).captured().toInt();
        });
        for (const QString &s : sheets) add(QStringLiteral("worksheet"), s.mid(folder.size()));
    }
    for (const QString &name : parts)
        if (const KnownPart *k = knownPart(name); k && k->relation && name.startsWith(folder) && name.indexOf(QLatin1Char('/'), folder.size()) < 0) add(QLatin1String(k->relation), name.mid(folder.size()));
    return relsXml(links);
}

// The byte of UTF-8 text that is `chars` UTF-16 units in (the unit a reader counts in), after a byte order mark if there is one. The text must be valid UTF-8.
qsizetype byteOfChar(const QByteArray &x, qint64 chars)
{
    qsizetype i = x.startsWith("\xEF\xBB\xBF") ? 3 : 0;
    qint64 n = 0;
    while (i < x.size() && n < chars) {
        const uchar c = uchar(x[i]);
        const int length = c < 0x80 ? 1 : c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : 2;
        n += length == 4 ? 2 : 1;
        i += length;
    }
    return std::min<qsizetype>(i, x.size());
}

// How many bytes at the end of `x` are the start of a letter that is not all there.
int incompleteTail(const QByteArray &x)
{
    for (int back = 1; back <= 3 && back <= x.size(); ++back) {
        const uchar c = uchar(x[x.size() - back]);
        if ((c & 0xC0) == 0x80) continue;   // (a continuation byte: look further back for its start)
        const int length = c < 0x80 ? 1 : c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
        return length > back ? back : 0;
    }
    return 0;
}

// (a cut in the middle of a letter leaves the replacement character at the end: it goes)
void chopReplacements(QByteArray &x)
{
    while (x.endsWith("\xEF\xBF\xBD")) x.chop(3);
}

constexpr qint64 kMaxXmlTokenBytes = qint64(64) << 20;   // one piece of XML (a text, a tag), as a sheet's is held: a real one is far smaller
constexpr int kMaxXmlNameChars = 1024;                    // an element's name: real names are under 50 characters, and the names of the open elements are kept

} // namespace

QByteArray repairXml(const QByteArray &xml, bool *trimmed, QString *stoppedBecause)
{
    if (trimmed) *trimmed = false;
    if (stoppedBecause) stoppedBecause->clear();
    if (xml.contains("<!DOCTYPE") || xml.contains("<!ENTITY")) return QByteArray();
    // The text is read from its bytes (no copy of it as UTF-16), through the guard that bounds what a parser is given.
    QBuffer buffer;
    buffer.setData(xml);
    buffer.open(QIODevice::ReadOnly);
    XmlGuardDevice guard(&buffer, kMaxXmlTokenBytes);
    QXmlStreamReader r(&guard);
    QStringList open;
    qint64 good = 0;
    bool rootSeen = false, tooDeep = false, tooLongName = false;
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType t = r.readNext();
        if (r.hasError()) break;
        if (t == QXmlStreamReader::StartElement) {
            if (open.size() >= kMaxXmlDepth) {
                tooDeep = true;
                break;
            }
            if (r.qualifiedName().size() > kMaxXmlNameChars) {
                tooLongName = true;
                break;
            }
            open << r.qualifiedName().toString();
            rootSeen = true;
            good = r.characterOffset();
        } else if (t == QXmlStreamReader::EndElement) {
            if (!open.isEmpty()) open.removeLast();
            good = r.characterOffset();
        } else if (t == QXmlStreamReader::Characters || t == QXmlStreamReader::Comment || t == QXmlStreamReader::ProcessingInstruction || t == QXmlStreamReader::StartDocument) {
            good = r.characterOffset();
        } else if (t == QXmlStreamReader::DTD) {
            return QByteArray();
        }
    }
    const bool broken = r.hasError() || tooDeep || tooLongName || guard.refused();
    if (!broken) return rootSeen ? xml : QByteArray();
    if (!rootSeen) return QByteArray();
    // What breaks is cut at the last whole piece. (The offsets are in characters; they are made bytes, which needs text that is UTF-8 all through: a part that is not is left out.)
    const QByteArray whole = QByteArray::fromRawData(xml.constData(), xml.size() - incompleteTail(xml));
    if (!QByteArrayView(whole).isValidUtf8()) return QByteArray();
    const qsizetype at = byteOfChar(whole, good);
    QByteArray kept = xml.left(at);
    chopReplacements(kept);
    // (text the reader had not yet handed over when it ran out: kept up to the next tag, less an entity cut in two)
    if (!open.isEmpty()) {
        QByteArray tail = xml.mid(at);
        if (const qsizetype lt = tail.indexOf('<'); lt >= 0) tail.truncate(lt);
        if (const qsizetype amp = tail.lastIndexOf('&'); amp >= 0 && tail.indexOf(';', amp) < 0) tail.truncate(amp);
        tail.chop(incompleteTail(tail));
        chopReplacements(tail);
        kept += tail;
    }
    for (qsizetype i = open.size() - 1; i >= 0; --i) kept += "</" + open[i].toUtf8() + ">";
    if (trimmed) *trimmed = true;
    if (stoppedBecause && (tooDeep || tooLongName)) *stoppedBecause = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "it has items inside items more deeply than any real file has"));
    return kept;
}

namespace {

// One entry of a package made ready to be a part: its name, and its bytes made what a reader can open (XML cut at the break). `drop` says why it can't be a part.
struct Prepared {
    QString name;
    QByteArray data;
    bool whole = false;      // read whole, and (for XML) not cut
    QString drop;            // empty when it is a part
    QString stopped;         // when XML was cut for its nesting: why
};

Prepared prepare(const ZipScanEntry &e, const RepairLimits &limits)
{
    Prepared p;
    p.name = plainName(e.name);
    if (!usableName(p.name)) {
        p.drop = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (the name is not usable)")).arg(e.name);
        return p;
    }
    if (e.problem == QLatin1String("too large")) {
        p.drop = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (too large to repair)")).arg(p.name);
        return p;
    }
    if (isXmlName(p.name)) {
        if (!readAsStream(p.name) && e.data.size() > limits.maxXmlPartBytes) {
            p.drop = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (larger than this program opens)")).arg(p.name);
            return p;
        }
        bool cut = false;
        p.data = repairXml(e.data, &cut, &p.stopped);
        if (p.data.isEmpty()) {
            p.drop = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (could not be read)")).arg(p.name);
            return p;
        }
        p.whole = e.whole && !cut;
    } else if (e.whole) {
        p.data = e.data;
        p.whole = true;
    } else {
        p.drop = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (%2)")).arg(p.name, e.problem.isEmpty() ? tr(QT_TRANSLATE_NOOP("jp::office::Repair", "could not be read")) : e.problem);
    }
    return p;
}

constexpr qsizetype kMaxTypesBytes = 16 << 20;   // the list of types, kept to put it first: a real one is a few thousand bytes

void noteLimits(const ZipScanStats &stats, RepairReport &rep, const RepairLimits &limits)
{
    if (stats.entriesLeftOut) rep.notes << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "The file has more than %1 entries. The ones after them were not looked at.")).arg(limits.maxEntries);
    if (stats.sizeLeftOut) rep.notes << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "The file holds more than can be read at once. The entries after that point were not looked at."));
}

} // namespace

QByteArray repairPackage(const QByteArray &zip, QString *error, RepairReport *report, const RepairLimits &limits)
{
    RepairReport local;
    RepairReport &rep = report ? *report : local;
    rep = RepairReport();
    // The first reading chooses: each entry is made ready, and the one that stands for each name is noted (by its place in the file), with its size; no part is kept, so what is held is one
    // entry at a time. The second writes the package from the entries chosen.
    struct Chosen {
        qint64 ordinal = 0;
        bool whole = false;
        qint64 size = 0;
        QByteArray types;   // the list of types only
    };
    QMap<QString, Chosen> chosen;
    qint64 keptBytes = 0, ordinal = 0;
    const QString typesName = QStringLiteral("[Content_Types].xml");
    ZipScanStats stats;
    scanZipEach(zip, [&](ZipScanEntry &e) {
        const qint64 mine = ordinal++;
        const QString name = plainName(e.name);
        if (usableName(name) && chosen.contains(name) && chosen.value(name).whole) return;   // (the first whole entry of a name stands)
        Prepared p = prepare(e, limits);
        if (!p.drop.isEmpty()) {
            rep.dropped << p.drop;
            return;
        }
        const qint64 replaced = chosen.contains(p.name) ? chosen.value(p.name).size : 0;
        if (keptBytes - replaced + p.data.size() > limits.maxKeptBytes) {
            rep.dropped << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (more than can be repaired at once)")).arg(p.name);
            return;
        }
        if (p.name == typesName && p.data.size() > kMaxTypesBytes) {
            rep.dropped << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (too large to repair)")).arg(p.name);
            return;
        }
        keptBytes += p.data.size() - replaced;
        Chosen c{mine, p.whole, p.data.size(), p.name == typesName ? p.data : QByteArray()};
        chosen.insert(p.name, c);
        if (p.whole) rep.kept << p.name;
        else rep.trimmed << p.name;
        if (!p.stopped.isEmpty()) rep.notes << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 was cut short: %2.")).arg(p.name, p.stopped);
    }, limits.maxEntries, limits.maxPartBytes, limits.maxWorkBytes, &stats);
    noteLimits(stats, rep, limits);
    const QStringList names = chosen.keys();
    const QString main = mainPartOf(QSet<QString>(names.cbegin(), names.cend()));
    if (main.isEmpty()) {
        if (error) *error = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "No document could be found in this file: its main part is gone or can't be read."));
        return QByteArray();
    }
    ZipWriter z;
    // The list of types and the links to the main part, made again when they are gone.
    if (chosen.contains(typesName)) {
        z.add(typesName, chosen.value(typesName).types, true);
    } else {
        z.add(typesName, typesXml(names), true);
        rep.typesMade = true;
    }
    if (!chosen.contains(QStringLiteral("_rels/.rels"))) {
        QList<std::array<QString, 3>> root;
        root.append({QStringLiteral("rId1"), QStringLiteral("officeDocument"), main});
        z.add(QStringLiteral("_rels/.rels"), relsXml(root), true);
        rep.linksMade = true;
    }
    const QString relsName = main.left(main.lastIndexOf(QLatin1Char('/')) + 1) + QStringLiteral("_rels/") + main.mid(main.lastIndexOf(QLatin1Char('/')) + 1) + QStringLiteral(".rels");
    if (!chosen.contains(relsName)) {
        // (a main part with nothing beside it to link to needs no list of links)
        const QByteArray rels = mainRels(main, names);
        if (rels.contains("<Relationship ")) {
            z.add(relsName, rels, true);
            rep.linksMade = true;
        }
    }
    QHash<qint64, QString> wanted;
    for (auto it = chosen.cbegin(); it != chosen.cend(); ++it)
        if (it.key() != typesName) wanted.insert(it.value().ordinal, it.key());
    qint64 at = 0;
    scanZipEach(zip, [&](ZipScanEntry &e) {
        const auto it = wanted.constFind(at++);
        if (it == wanted.cend()) return;
        const Prepared p = prepare(e, limits);
        if (p.drop.isEmpty()) z.add(p.name, p.data, true);
    }, limits.maxEntries, limits.maxPartBytes, limits.maxWorkBytes);
    return z.finish();
}

QByteArray repairZip(const QByteArray &zip, QString *error, RepairReport *report, const RepairLimits &limits)
{
    RepairReport local;
    RepairReport &rep = report ? *report : local;
    rep = RepairReport();
    ZipWriter z;
    QSet<QString> seen;
    qint64 keptBytes = 0;
    ZipScanStats stats;
    scanZipEach(zip, [&](ZipScanEntry &e) {
        const QString name = plainName(e.name);
        if (!usableName(name)) {
            rep.dropped << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (the name is not usable)")).arg(e.name);
            return;
        }
        if (seen.contains(name)) return;
        if (!e.whole) {
            rep.dropped << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (%2)")).arg(name, e.problem.isEmpty() ? tr(QT_TRANSLATE_NOOP("jp::office::Repair", "could not be read")) : e.problem);
            return;
        }
        if (keptBytes + e.data.size() > limits.maxKeptBytes) {
            rep.dropped << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "%1 (more than can be repaired at once)")).arg(name);
            return;
        }
        keptBytes += e.data.size();
        seen.insert(name);
        rep.kept << name;
        z.add(name, e.data, false);
    }, limits.maxEntries, limits.maxPartBytes, limits.maxWorkBytes, &stats);
    noteLimits(stats, rep, limits);
    if (rep.kept.isEmpty()) {
        if (error) *error = tr(QT_TRANSLATE_NOOP("jp::office::Repair", "Nothing in this file could be read."));
        return QByteArray();
    }
    return z.finish();
}

QString repairSummary(const RepairReport &r)
{
    QStringList lines;
    if (!r.trimmed.isEmpty()) lines << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "Cut short, kept as far as it could be read: %1")).arg(r.trimmed.join(QStringLiteral(", ")));
    if (!r.dropped.isEmpty()) lines << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "Left out, because it could not be read: %1")).arg(r.dropped.join(QStringLiteral(", ")));
    for (const QString &note : r.notes) lines << note;
    if (r.typesMade) lines << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "The list of what each part is was gone, and was made again."));
    if (r.linksMade) lines << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "The links between the parts were gone, and were made again, so what depends on them (such as pictures) may be missing."));
    if (lines.isEmpty()) lines << tr(QT_TRANSLATE_NOOP("jp::office::Repair", "Every part could be read. Nothing was left out."));
    return lines.join(QLatin1Char('\n'));
}

} // namespace jp::office
