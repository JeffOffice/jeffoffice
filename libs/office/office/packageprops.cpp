#include "office/packageprops.h"

#include <QSet>
#include <QTimeZone>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <functional>

namespace jp::office {

namespace {

const QString kCp = QStringLiteral("http://schemas.openxmlformats.org/package/2006/metadata/core-properties");
const QString kDc = QStringLiteral("http://purl.org/dc/elements/1.1/");
const QString kDcterms = QStringLiteral("http://purl.org/dc/terms/");
const QString kXsi = QStringLiteral("http://www.w3.org/2001/XMLSchema-instance");
const QString kExt = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/extended-properties");
const QString kVt = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes");
const QString kCustomNs = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/custom-properties");
constexpr int kMostSteps = 200000;   // (the most tokens of a part that are read)

QString dateText(const QDateTime &d) { return d.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'")); }

QDateTime dateOf(const QString &text)
{
    QDateTime d = QDateTime::fromString(text.trimmed(), Qt::ISODate);
    if (d.isValid() && d.timeSpec() == Qt::LocalTime) d.setTimeZone(QTimeZone::UTC);
    return d;
}

using Name = QPair<QString, QString>;   // namespace and local name

// Copies `existing` and puts `append`'s elements at the end of its root, leaving out the root's children named in `managed`. Without a root of the right name, the part is made new.
QByteArray rewrite(const QByteArray &existing, const QString &rootNs, const QString &rootLocal, const QVector<QPair<QString, QString>> &newPrefixes, const QSet<Name> &managed,
                   const std::function<void(QXmlStreamWriter &)> &append)
{
    QByteArray out;
    QXmlStreamWriter w(&out);
    w.setAutoFormatting(false);
    w.writeStartDocument(QStringLiteral("1.0"), true);
    bool copied = false;
    if (!existing.isEmpty()) {
        QXmlStreamReader r(existing);
        int depth = 0, steps = 0;
        bool good = true, rooted = false;
        QByteArray trial;
        QXmlStreamWriter t(&trial);
        t.writeStartDocument(QStringLiteral("1.0"), true);
        while (!r.atEnd() && good && ++steps < kMostSteps) {
            const auto token = r.readNext();
            if (token == QXmlStreamReader::StartDocument || token == QXmlStreamReader::EndDocument || token == QXmlStreamReader::Comment || token == QXmlStreamReader::DTD) continue;
            if (token == QXmlStreamReader::StartElement) {
                if (depth == 0) {
                    if (r.namespaceUri() != rootNs || r.name() != rootLocal) { good = false; break; }
                    rooted = true;
                } else if (depth == 1 && managed.contains({r.namespaceUri().toString(), r.name().toString()})) {
                    r.skipCurrentElement();
                    continue;
                }
                t.writeCurrentToken(r);
                ++depth;
                continue;
            }
            if (token == QXmlStreamReader::EndElement) {
                --depth;
                if (depth == 0) append(t);
                t.writeCurrentToken(r);
                continue;
            }
            if (depth == 0) continue;   // (text outside the root)
            t.writeCurrentToken(r);
        }
        t.writeEndDocument();
        if (good && rooted && !r.hasError() && steps < kMostSteps) {
            out = trial;
            copied = true;
        }
    }
    if (!copied) {
        out.clear();
        QXmlStreamWriter n(&out);
        n.writeStartDocument(QStringLiteral("1.0"), true);
        n.writeStartElement(rootNs, rootLocal);
        for (const auto &p : newPrefixes) n.writeNamespace(p.second, p.first);
        append(n);
        n.writeEndElement();
        n.writeEndDocument();
    }
    return out;
}

void textElement(QXmlStreamWriter &w, const QString &ns, const QString &local, const QString &text)
{
    if (text.isEmpty()) return;
    w.writeTextElement(ns, local, text);
}

void dateElement(QXmlStreamWriter &w, const QString &local, const QDateTime &d, bool terms)
{
    if (!d.isValid()) return;
    w.writeStartElement(terms ? kDcterms : kCp, local);
    if (terms) w.writeAttribute(kXsi, QStringLiteral("type"), QStringLiteral("dcterms:W3CDTF"));
    w.writeCharacters(dateText(d));
    w.writeEndElement();
}

QString elementText(QXmlStreamReader &r) { return r.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement).trimmed(); }

bool validType(const QString &t) { return t == QLatin1String("lpwstr") || t == QLatin1String("i4") || t == QLatin1String("r8") || t == QLatin1String("bool") || t == QLatin1String("filetime"); }

} // namespace

const char *const kMarkAsFinalName = "_MarkAsFinal";

PackageProps readPackageProps(const QByteArray &core, const QByteArray &app, const QByteArray &custom)
{
    PackageProps p;
    // Each part is read into a list of (local name, text) of its root's children.
    auto children = [](const QByteArray &bytes) {
        QVector<QPair<QString, QString>> list;
        if (bytes.isEmpty()) return list;
        QXmlStreamReader r(bytes);
        int depth = 0, steps = 0;
        while (!r.atEnd() && ++steps < kMostSteps) {
            const auto token = r.readNext();
            if (token == QXmlStreamReader::StartElement) {
                if (++depth == 2) {
                    const QString name = r.name().toString();
                    // (the text of a child; a child with children of its own gives its first text)
                    QString text;
                    int inner = 1;
                    while (!r.atEnd() && inner > 0 && ++steps < kMostSteps) {
                        const auto t = r.readNext();
                        if (t == QXmlStreamReader::StartElement) ++inner;
                        else if (t == QXmlStreamReader::EndElement) --inner;
                        else if (t == QXmlStreamReader::Characters && text.isEmpty()) text = r.text().toString().trimmed();
                    }
                    list.push_back({name, text});
                    --depth;
                }
            } else if (token == QXmlStreamReader::EndElement) {
                --depth;
            }
        }
        return list;
    };
    for (const auto &c : children(core)) {
        const QString &n = c.first, &v = c.second;
        if (n == QLatin1String("title")) p.title = v;
        else if (n == QLatin1String("subject")) p.subject = v;
        else if (n == QLatin1String("creator")) p.creator = v;
        else if (n == QLatin1String("keywords")) p.keywords = v;
        else if (n == QLatin1String("description")) p.comments = v;
        else if (n == QLatin1String("lastModifiedBy")) p.lastModifiedBy = v;
        else if (n == QLatin1String("category")) p.category = v;
        else if (n == QLatin1String("revision")) p.revision = v;
        else if (n == QLatin1String("created")) p.created = dateOf(v);
        else if (n == QLatin1String("modified")) p.modified = dateOf(v);
        else if (n == QLatin1String("lastPrinted")) p.printed = dateOf(v);
    }
    auto count = [](const QString &v) {
        bool ok = false;
        const qint64 n = v.toLongLong(&ok);
        return ok && n >= 0 && n <= 2000000000 ? int(n) : -1;
    };
    for (const auto &c : children(app)) {
        const QString &n = c.first, &v = c.second;
        if (n == QLatin1String("Company")) p.company = v;
        else if (n == QLatin1String("Manager")) p.manager = v;
        else if (n == QLatin1String("Pages")) p.pages = count(v);
        else if (n == QLatin1String("Words")) p.words = count(v);
        else if (n == QLatin1String("Characters")) p.characters = count(v);
        else if (n == QLatin1String("CharactersWithSpaces")) p.charactersWithSpaces = count(v);
        else if (n == QLatin1String("Lines")) p.lines = count(v);
        else if (n == QLatin1String("Paragraphs")) p.paragraphs = count(v);
        else if (n == QLatin1String("TotalTime")) p.totalMinutes = v.toLongLong() >= 0 ? v.toLongLong() : -1;
    }
    if (!custom.isEmpty()) {
        QXmlStreamReader r(custom);
        int depth = 0, steps = 0;
        while (!r.atEnd() && ++steps < kMostSteps && p.custom.size() < 5000) {
            const auto token = r.readNext();
            if (token == QXmlStreamReader::EndElement) {
                --depth;
                continue;
            }
            if (token != QXmlStreamReader::StartElement) continue;
            ++depth;
            if (depth != 2 || r.name() != QLatin1String("property")) continue;
            CustomProperty c;
            c.name = r.attributes().value(QLatin1String("name")).toString();
            int inner = 1;
            while (!r.atEnd() && inner > 0 && ++steps < kMostSteps) {
                const auto t = r.readNext();
                if (t == QXmlStreamReader::StartElement) {
                    if (++inner == 2 && c.type.isEmpty()) {
                        c.type = r.name().toString();
                        c.value = elementText(r);
                        --inner;
                    }
                } else if (t == QXmlStreamReader::EndElement) {
                    --inner;
                }
            }
            --depth;
            // (a value of a type this reads as text, whatever its type, so that it is kept as the file had it)
            if (c.name.isEmpty()) continue;
            if (!validType(c.type)) continue;
            bool dup = false;
            for (const CustomProperty &o : std::as_const(p.custom))
                if (o.name == c.name) dup = true;
            if (!dup) p.custom.push_back(c);
        }
    }
    return p;
}

PackageProps readPackageProps(const Package &package)
{
    auto partOfType = [&package](const char *suffix) {
        const QVector<Relationship> rels = package.relationshipsOfType(QStringLiteral("/"), QString::fromLatin1(suffix));
        for (const Relationship &r : rels)
            if (!r.external && package.hasPart(r.target)) return package.part(r.target);
        return QByteArray();
    };
    return readPackageProps(partOfType("/core-properties"), partOfType("/extended-properties"), partOfType("/custom-properties"));
}

QByteArray writeCoreProps(const QByteArray &existing, const PackageProps &p)
{
    const QSet<Name> managed = {{kDc, QStringLiteral("title")}, {kDc, QStringLiteral("subject")}, {kDc, QStringLiteral("creator")}, {kCp, QStringLiteral("keywords")},
                                {kDc, QStringLiteral("description")}, {kCp, QStringLiteral("lastModifiedBy")}, {kCp, QStringLiteral("category")}, {kCp, QStringLiteral("revision")},
                                {kDcterms, QStringLiteral("created")}, {kDcterms, QStringLiteral("modified")}, {kCp, QStringLiteral("lastPrinted")}};
    return rewrite(existing, kCp, QStringLiteral("coreProperties"),
                   {{QStringLiteral("cp"), kCp}, {QStringLiteral("dc"), kDc}, {QStringLiteral("dcterms"), kDcterms}, {QStringLiteral("xsi"), kXsi}}, managed, [&p](QXmlStreamWriter &w) {
                       textElement(w, kDc, QStringLiteral("title"), p.title);
                       textElement(w, kDc, QStringLiteral("subject"), p.subject);
                       textElement(w, kDc, QStringLiteral("creator"), p.creator);
                       textElement(w, kCp, QStringLiteral("keywords"), p.keywords);
                       textElement(w, kDc, QStringLiteral("description"), p.comments);
                       textElement(w, kCp, QStringLiteral("lastModifiedBy"), p.lastModifiedBy);
                       textElement(w, kCp, QStringLiteral("category"), p.category);
                       textElement(w, kCp, QStringLiteral("revision"), p.revision);
                       dateElement(w, QStringLiteral("created"), p.created, true);
                       dateElement(w, QStringLiteral("modified"), p.modified, true);
                       dateElement(w, QStringLiteral("lastPrinted"), p.printed, false);
                   });
}

QByteArray writeAppProps(const QByteArray &existing, const PackageProps &p)
{
    QSet<Name> managed = {{kExt, QStringLiteral("Company")}, {kExt, QStringLiteral("Manager")}};
    auto number = [&managed](const char *name, qint64 v) {
        if (v >= 0) managed.insert({kExt, QString::fromLatin1(name)});
    };
    number("Pages", p.pages);
    number("Words", p.words);
    number("Characters", p.characters);
    number("CharactersWithSpaces", p.charactersWithSpaces);
    number("Lines", p.lines);
    number("Paragraphs", p.paragraphs);
    number("TotalTime", p.totalMinutes);
    return rewrite(existing, kExt, QStringLiteral("Properties"), {{QString(), kExt}, {QStringLiteral("vt"), kVt}}, managed, [&p](QXmlStreamWriter &w) {
        auto num = [&w](const char *name, qint64 v) {
            if (v >= 0) w.writeTextElement(kExt, QString::fromLatin1(name), QString::number(v));
        };
        num("TotalTime", p.totalMinutes);
        num("Pages", p.pages);
        num("Words", p.words);
        num("Characters", p.characters);
        num("CharactersWithSpaces", p.charactersWithSpaces);
        num("Lines", p.lines);
        num("Paragraphs", p.paragraphs);
        textElement(w, kExt, QStringLiteral("Company"), p.company);
        textElement(w, kExt, QStringLiteral("Manager"), p.manager);
    });
}

QByteArray writeCustomProps(const PackageProps &p)
{
    QByteArray out;
    QXmlStreamWriter w(&out);
    w.writeStartDocument(QStringLiteral("1.0"), true);
    w.writeStartElement(kCustomNs, QStringLiteral("Properties"));
    w.writeDefaultNamespace(kCustomNs);
    w.writeNamespace(kVt, QStringLiteral("vt"));
    QSet<QString> seen;
    int pid = 2;
    for (const CustomProperty &c : p.custom) {
        if (c.name.isEmpty() || !validType(c.type) || seen.contains(c.name)) continue;
        seen.insert(c.name);
        w.writeStartElement(kCustomNs, QStringLiteral("property"));
        w.writeAttribute(QStringLiteral("fmtid"), QStringLiteral("{D5CDD505-2E9C-101B-9397-08002B2CF9AE}"));
        w.writeAttribute(QStringLiteral("pid"), QString::number(pid++));
        w.writeAttribute(QStringLiteral("name"), c.name);
        w.writeTextElement(kVt, c.type, c.value);
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeEndDocument();
    return out;
}

bool isMarkedFinal(const QVector<CustomProperty> &custom)
{
    for (const CustomProperty &c : custom)
        if (c.name == QLatin1String(kMarkAsFinalName)) return c.type == QLatin1String("bool") && (c.value == QLatin1String("true") || c.value == QLatin1String("1"));
    return false;
}

QVector<CustomProperty> withMarkAsFinal(QVector<CustomProperty> custom, bool on)
{
    for (qsizetype i = custom.size() - 1; i >= 0; --i)
        if (custom[i].name == QLatin1String(kMarkAsFinalName)) custom.removeAt(i);
    if (on) custom.push_back({QString::fromLatin1(kMarkAsFinalName), QStringLiteral("bool"), QStringLiteral("true")});
    return custom;
}

} // namespace jp::office
