#include "office/repair.h"

#include "io/zip.h"

#include <QCoreApplication>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QXmlStreamReader>

#include <algorithm>
#include <array>

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
    for (const KnownPart &k : kKnown)
        if (QRegularExpression(QStringLiteral("^(%1)$").arg(QLatin1String(k.pattern)), QRegularExpression::CaseInsensitiveOption).match(name).hasMatch()) return &k;
    return nullptr;
}

QString mainPartOf(const QMap<QString, QByteArray> &parts)
{
    for (const char *name : {"word/document.xml", "xl/workbook.xml", "ppt/presentation.xml"})
        if (parts.contains(QLatin1String(name))) return QLatin1String(name);
    return QString();
}

QByteArray typesXml(const QMap<QString, QByteArray> &parts)
{
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                   "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/><Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                   "<Default Extension=\"png\" ContentType=\"image/png\"/><Default Extension=\"jpeg\" ContentType=\"image/jpeg\"/><Default Extension=\"jpg\" ContentType=\"image/jpeg\"/>"
                   "<Default Extension=\"gif\" ContentType=\"image/gif\"/><Default Extension=\"bmp\" ContentType=\"image/bmp\"/><Default Extension=\"tiff\" ContentType=\"image/tiff\"/>"
                   "<Default Extension=\"emf\" ContentType=\"image/x-emf\"/><Default Extension=\"wmf\" ContentType=\"image/x-wmf\"/><Default Extension=\"svg\" ContentType=\"image/svg+xml\"/>";
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (const KnownPart *k = knownPart(it.key())) x += "<Override PartName=\"/" + it.key().toUtf8() + "\" ContentType=\"" + k->type + "\"/>";
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
QByteArray mainRels(const QString &main, const QMap<QString, QByteArray> &parts)
{
    const QString folder = main.left(main.lastIndexOf(QLatin1Char('/')) + 1);
    QList<std::array<QString, 3>> links;
    int n = 0;
    auto add = [&](const QString &type, const QString &target) { links.append({QStringLiteral("rId%1").arg(++n), type, target}); };
    if (main == QLatin1String("xl/workbook.xml")) {
        // (the sheets are named in the workbook by the links' Ids: they are taken in the order the sheets' parts are numbered, which is how a program writes them)
        QStringList sheets;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            if (QRegularExpression(QStringLiteral("^xl/worksheets/sheet[0-9]+\\.xml$")).match(it.key()).hasMatch()) sheets << it.key();
        std::sort(sheets.begin(), sheets.end(), [](const QString &a, const QString &b) {
            static const QRegularExpression digits(QStringLiteral("[0-9]+"));
            return digits.match(a, 12).captured().toInt() < digits.match(b, 12).captured().toInt();
        });
        for (const QString &s : sheets) add(QStringLiteral("worksheet"), s.mid(folder.size()));
    }
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (const KnownPart *k = knownPart(it.key()); k && k->relation && it.key().startsWith(folder) && it.key().indexOf(QLatin1Char('/'), folder.size()) < 0) add(QLatin1String(k->relation), it.key().mid(folder.size()));
    return relsXml(links);
}

} // namespace

QByteArray repairXml(const QByteArray &xml, bool *trimmed)
{
    if (trimmed) *trimmed = false;
    if (xml.contains("<!DOCTYPE") || xml.contains("<!ENTITY")) return QByteArray();
    const QString text = QString::fromUtf8(xml);
    QXmlStreamReader r(text);
    QStringList open;
    qint64 good = 0;
    bool rootSeen = false;
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType t = r.readNext();
        if (r.hasError()) break;
        if (t == QXmlStreamReader::StartElement) {
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
    if (!r.hasError()) return rootSeen ? xml : QByteArray();
    if (!rootSeen) return QByteArray();
    QString kept = text.left(qsizetype(good));
    // (a cut in the middle of a letter leaves the replacement character at the end: it goes)
    while (kept.endsWith(QChar(0xFFFD))) kept.chop(1);
    // (text the reader had not yet handed over when it ran out: kept up to the next tag, less an entity cut in two)
    if (!open.isEmpty()) {
        QString tail = text.mid(qsizetype(good));
        if (const qsizetype lt = tail.indexOf(QLatin1Char('<')); lt >= 0) tail.truncate(lt);
        if (const qsizetype amp = tail.lastIndexOf(QLatin1Char('&')); amp >= 0 && tail.indexOf(QLatin1Char(';'), amp) < 0) tail.truncate(amp);
        while (tail.endsWith(QChar(0xFFFD))) tail.chop(1);
        kept += tail;
    }
    for (qsizetype i = open.size() - 1; i >= 0; --i) kept += QStringLiteral("</") + open[i] + QLatin1Char('>');
    if (trimmed) *trimmed = true;
    return kept.toUtf8();
}

QByteArray repairPackage(const QByteArray &zip, QString *error, RepairReport *report)
{
    RepairReport local;
    RepairReport &rep = report ? *report : local;
    rep = RepairReport();
    QMap<QString, QByteArray> parts;
    QMap<QString, bool> whole;
    for (const ZipScanEntry &e : scanZip(zip)) {
        const QString name = plainName(e.name);
        if (name.isEmpty() || name.contains(QLatin1String("../")) || name.startsWith(QLatin1String("./"))) {
            rep.dropped << tr("%1 (the name is not usable)").arg(e.name);
            continue;
        }
        if (parts.contains(name) && whole.value(name)) continue;   // (the first whole entry of a name stands)
        if (isXmlName(name)) {
            bool cut = false;
            const QByteArray fixed = repairXml(e.data, &cut);
            if (fixed.isEmpty()) {
                rep.dropped << tr("%1 (could not be read)").arg(name);
                continue;
            }
            parts.insert(name, fixed);
            whole.insert(name, e.whole && !cut);
            if (e.whole && !cut) rep.kept << name;
            else rep.trimmed << name;
        } else if (e.whole) {
            parts.insert(name, e.data);
            whole.insert(name, true);
            rep.kept << name;
        } else {
            rep.dropped << tr("%1 (%2)").arg(name, e.problem.isEmpty() ? tr("could not be read") : e.problem);
        }
    }
    const QString main = mainPartOf(parts);
    if (main.isEmpty()) {
        if (error) *error = tr("No document could be found in this file: its main part is gone or can't be read.");
        return QByteArray();
    }
    // The list of types and the links to the main part, made again when they are gone.
    if (!parts.contains(QStringLiteral("[Content_Types].xml"))) {
        parts.insert(QStringLiteral("[Content_Types].xml"), typesXml(parts));
        rep.typesMade = true;
    }
    if (!parts.contains(QStringLiteral("_rels/.rels"))) {
        QList<std::array<QString, 3>> root;
        root.append({QStringLiteral("rId1"), QStringLiteral("officeDocument"), main});
        parts.insert(QStringLiteral("_rels/.rels"), relsXml(root));
        rep.linksMade = true;
    }
    const QString relsName = main.left(main.lastIndexOf(QLatin1Char('/')) + 1) + QStringLiteral("_rels/") + main.mid(main.lastIndexOf(QLatin1Char('/')) + 1) + QStringLiteral(".rels");
    if (!parts.contains(relsName)) {
        // (a main part with nothing beside it to link to needs no list of links)
        const QByteArray rels = mainRels(main, parts);
        if (rels.contains("<Relationship ")) {
            parts.insert(relsName, rels);
            rep.linksMade = true;
        }
    }
    ZipWriter z;
    z.add(QStringLiteral("[Content_Types].xml"), parts.take(QStringLiteral("[Content_Types].xml")), true);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

QByteArray repairZip(const QByteArray &zip, QString *error, RepairReport *report)
{
    RepairReport local;
    RepairReport &rep = report ? *report : local;
    rep = RepairReport();
    ZipWriter z;
    QSet<QString> seen;
    for (const ZipScanEntry &e : scanZip(zip)) {
        const QString name = plainName(e.name);
        if (name.isEmpty() || name.contains(QLatin1String("../")) || seen.contains(name)) continue;
        if (!e.whole) {
            rep.dropped << tr("%1 (%2)").arg(name, e.problem.isEmpty() ? tr("could not be read") : e.problem);
            continue;
        }
        seen.insert(name);
        rep.kept << name;
        z.add(name, e.data, false);
    }
    if (rep.kept.isEmpty()) {
        if (error) *error = tr("Nothing in this file could be read.");
        return QByteArray();
    }
    return z.finish();
}

QString repairSummary(const RepairReport &r)
{
    QStringList lines;
    if (!r.trimmed.isEmpty()) lines << tr("Cut short, kept as far as it could be read: %1").arg(r.trimmed.join(QStringLiteral(", ")));
    if (!r.dropped.isEmpty()) lines << tr("Left out, because it could not be read: %1").arg(r.dropped.join(QStringLiteral(", ")));
    if (r.typesMade) lines << tr("The list of what each part is was gone, and was made again.");
    if (r.linksMade) lines << tr("The links between the parts were gone, and were made again, so what depends on them (such as pictures) may be missing.");
    if (lines.isEmpty()) lines << tr("Every part could be read. Nothing was left out.");
    return lines.join(QLatin1Char('\n'));
}

} // namespace jp::office
