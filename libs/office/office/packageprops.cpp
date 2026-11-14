#include "office/packageprops.h"
#include "office/package.h"

#include <QSet>
#include <algorithm>
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
constexpr int kMostCustom = 5000;
// (the most custom properties that are read: kMostCustom above)

QString dateText(const QDateTime &d) { return d.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'")); }

QDateTime dateOf(const QString &text)
{
    QDateTime d = QDateTime::fromString(text.trimmed(), Qt::ISODate);
    if (d.isValid() && d.timeSpec() == Qt::LocalTime) d.setTimeZone(QTimeZone::UTC);
    return d;
}

using Name = QPair<QString, QString>;   // namespace and local name

// Copies `existing` and puts `append`'s elements at the end of its root, leaving out the root's children named in `managed`. What the part holds besides is copied as it is, also from a part
// that breaks part way (the rest of it is closed). Without a root of the right name, the part is made new. A part too large to be read through is left as it is (the result is `existing`).
QByteArray rewrite(const QByteArray &existing, const QString &rootNs, const QString &rootLocal, const QVector<QPair<QString, QString>> &newPrefixes, const QSet<Name> &managed,
                   const std::function<void(QXmlStreamWriter &)> &append)
{
    if (!existing.isEmpty()) {
        QXmlStreamReader r(existing);
        int depth = 0, steps = 0;
        bool good = true, rooted = false, appended = false;
        QByteArray trial;
        QXmlStreamWriter t(&trial);
        t.writeStartDocument(QStringLiteral("1.0"), true);
        while (!r.atEnd() && good && steps < kMostSteps) {
            ++steps;
            const auto token = r.readNext();
            if (token == QXmlStreamReader::StartDocument || token == QXmlStreamReader::EndDocument || token == QXmlStreamReader::Comment || token == QXmlStreamReader::DTD) continue;
            if (token == QXmlStreamReader::Invalid) break;
            if (token == QXmlStreamReader::StartElement) {
                if (depth == 0) {
                    if (r.namespaceUri() != rootNs || r.name() != rootLocal) {
                        good = false;
                        break;
                    }
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
                if (depth == 0) {
                    append(t);
                    appended = true;
                }
                t.writeCurrentToken(r);
                continue;
            }
            if (depth == 0) continue;   // (text outside the root)
            t.writeCurrentToken(r);
        }
        if (steps >= kMostSteps) return existing;
        if (good && rooted && !r.hasError()) {
            t.writeEndDocument();
            return trial;
        }
        // A part that breaks part way: what was copied is kept, closed down to the root, and the properties are put in.
        if (good && rooted && depth >= 1 && !appended) {
            while (depth > 1) {
                t.writeEndElement();
                --depth;
            }
            append(t);
            t.writeEndDocument();
            return trial;
        }
    }
    QByteArray out;
    QXmlStreamWriter n(&out);
    n.writeStartDocument(QStringLiteral("1.0"), true);
    n.writeStartElement(rootNs, rootLocal);
    for (const auto &p : newPrefixes) n.writeNamespace(p.second, p.first);
    append(n);
    n.writeEndElement();
    n.writeEndDocument();
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

bool validType(const QString &t) { return t == QLatin1String("lpwstr") || t == QLatin1String("i4") || t == QLatin1String("r8") || t == QLatin1String("bool") || t == QLatin1String("filetime"); }

} // namespace

const char *const kMarkAsFinalName = "_MarkAsFinal";

PackageProps readPackageProps(const QByteArray &core, const QByteArray &app, const QByteArray &custom)
{
    PackageProps p;
    // Each part is read into a list of (local name, text) of its root's children.
    auto children = [&p](const QByteArray &bytes) {
        QVector<QPair<QString, QString>> list;
        if (bytes.isEmpty()) return list;
        QXmlStreamReader r(bytes);
        int depth = 0, steps = 0;
        while (!r.atEnd() && steps < kMostSteps) {
            ++steps;
            const auto token = r.readNext();
            if (token == QXmlStreamReader::StartElement) {
                if (++depth == 2) {
                    const QString name = r.name().toString();
                    // (the text of a child; a child with children of its own gives its first text)
                    QString text;
                    int inner = 1;
                    while (!r.atEnd() && inner > 0 && steps < kMostSteps) {
                        ++steps;
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
        if (steps >= kMostSteps) p.truncated = true;
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
        // (read from the text, so that the places of the elements are places in it: a property this program does not model is cut out of it as it was written)
        // (the reader counts "\r\n" as one character, which would move every place: it reads a copy with a space for each carriage return, and what is cut out is cut from the original)
        const QString original = QString::fromUtf8(custom);
        QString text = original;
        text.replace(QLatin1Char('\r'), QLatin1Char(' '));
        QXmlStreamReader r(text);
        int depth = 0, steps = 0;
        while (!r.atEnd() && steps < kMostSteps) {
            ++steps;
            const auto token = r.readNext();
            if (token == QXmlStreamReader::EndElement) {
                --depth;
                continue;
            }
            if (token != QXmlStreamReader::StartElement) continue;
            ++depth;
            if (depth != 2 || r.name() != QLatin1String("property")) continue;
            if (p.custom.size() >= kMostCustom) {
                p.truncated = true;
                break;
            }
            // (where the element begins: the last "<" before the end of its start tag, which a value of an attribute can't hold)
            const qint64 before = original.lastIndexOf(QLatin1Char('<'), int(r.characterOffset()) - 1);
            CustomProperty c;
            c.name = r.attributes().value(QLatin1String("name")).toString();
            c.fmtid = r.attributes().value(QLatin1String("fmtid")).toString();
            c.pid = r.attributes().value(QLatin1String("pid")).toString().toInt();
            QString valueType, valueText;
            int inner = 1, valueChildren = 0, nested = 0;
            while (!r.atEnd() && inner > 0 && steps < kMostSteps) {
                ++steps;
                const auto t = r.readNext();
                if (t == QXmlStreamReader::StartElement) {
                    ++inner;
                    if (inner == 2) {
                        ++valueChildren;
                        if (valueType.isEmpty()) valueType = r.name().toString();
                    } else if (inner > 2) {
                        ++nested;
                    }
                } else if (t == QXmlStreamReader::EndElement) {
                    --inner;
                } else if (t == QXmlStreamReader::Characters && inner == 2) {
                    valueText += r.text().toString();
                }
            }
            if (inner > 0) {
                p.truncated = true;   // (the part ends inside the property, or is too long to read through)
                break;
            }
            --depth;
            const qint64 end = r.characterOffset();
            c.type = valueType;
            if (validType(valueType) && valueChildren == 1 && nested == 0 && !c.name.isEmpty()) {
                c.value = valueText.trimmed();
                if (valueType == QLatin1String("lpwstr")) c.value = valueText;
            } else {
                // not modeled here: kept as it was
                c.raw = original.mid(int(before), int(end - before)).toUtf8();
            }
            p.custom.push_back(c);
        }
        if (steps >= kMostSteps) p.truncated = true;
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
    if (p.truncated) return existing;
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
    if (p.truncated) return existing;
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

QByteArray writeCustomProps(const QByteArray &existing, const PackageProps &p)
{
    static const QString kFmtid = QStringLiteral("{D5CDD505-2E9C-101B-9397-08002B2CF9AE}");
    if (p.truncated) return existing;
    // The root as the part had it (its start tag, with the prefixes it binds), else a new one.
    QString head, tail, vtPrefix;
    if (!existing.isEmpty()) {
        const QString original = QString::fromUtf8(existing);
        QString text = original;
        text.replace(QLatin1Char('\r'), QLatin1Char(' '));
        QXmlStreamReader r(text);
        while (!r.atEnd()) {
            if (r.readNext() != QXmlStreamReader::StartElement) continue;
            if (r.namespaceUri() == kCustomNs && r.name() == QLatin1String("Properties")) {
                head = original.left(int(r.characterOffset()));
                tail = QStringLiteral("</") + r.qualifiedName().toString() + QLatin1Char('>');
                for (const auto &d : r.namespaceDeclarations())
                    if (d.namespaceUri() == kVt) vtPrefix = d.prefix().toString();
                if (head.endsWith(QLatin1String("/>"))) {
                    head.chop(2);
                    head += QLatin1Char('>');
                }
            }
            break;
        }
    }
    const bool fresh = head.isEmpty();
    if (fresh) {
        head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Properties xmlns=\"%1\" xmlns:vt=\"%2\">").arg(kCustomNs, kVt);
        tail = QStringLiteral("</Properties>");
        vtPrefix = QStringLiteral("vt");
    }
    // The places a new property may have: above every one the file uses.
    int nextPid = 2;
    for (const CustomProperty &c : p.custom) nextPid = std::max(nextPid, c.pid + 1);
    QByteArray out = head.toUtf8();
    QSet<QString> seen;
    for (const CustomProperty &c : p.custom) {
        if (!c.raw.isEmpty()) {
            out += c.raw;
            continue;
        }
        if (c.name.isEmpty() || !validType(c.type) || seen.contains(c.name)) continue;
        seen.insert(c.name);
        const QString tag = vtPrefix.isEmpty() ? c.type : vtPrefix + QLatin1Char(':') + c.type;
        QString element = QStringLiteral("<property fmtid=\"%1\" pid=\"%2\" name=\"%3\"><%4%5>%6</%4></property>")
                              .arg(c.fmtid.isEmpty() ? kFmtid : c.fmtid.toHtmlEscaped())
                              .arg(c.pid > 0 ? c.pid : nextPid++)
                              .arg(c.name.toHtmlEscaped(), tag, vtPrefix.isEmpty() ? QStringLiteral(" xmlns=\"%1\"").arg(kVt) : QString(), c.value.toHtmlEscaped());
        out += element.toUtf8();
    }
    out += tail.toUtf8();
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
