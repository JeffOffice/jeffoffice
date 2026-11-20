#include "odfvalidate.h"

#include "io/zip.h"

#include <QFile>
#include <QLibrary>
#include <QSysInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifndef JO_ODF_SCHEMA_DIR
#error "JO_ODF_SCHEMA_DIR names third_party/odf_schema (the test's CMake target defines it)"
#endif

namespace odfcheck {
namespace {

// The few libxml2 entry points the check uses, found by name when the test runs. Only opaque pointers are passed, but for the two small structures read back (the error and the node), whose first fields are
// the same in every 2.x release.
struct XmlError {
    int domain, code;
    char *message;
    int level;
    char *file;
    int line;
    char *str1, *str2, *str3;
    int int1, int2;
    void *ctxt, *node;
};
struct XmlNode {
    void *priv;
    int type;
    const unsigned char *name;
};
using ErrorFn = void (*)(void *, const XmlError *);

struct Xml2 {
    QLibrary lib;
    bool ok = false;
    QString why;
    void *(*readMemory)(const char *, int, const char *, const char *, int) = nullptr;
    void (*freeDoc)(void *) = nullptr;
    void *(*newDocParser)(void *) = nullptr;
    void (*setParserErrors)(void *, ErrorFn, void *) = nullptr;
    void *(*parse)(void *) = nullptr;
    void (*freeParser)(void *) = nullptr;
    void (*freeSchema)(void *) = nullptr;
    void *(*newValid)(void *) = nullptr;
    void (*setValidErrors)(void *, ErrorFn, void *) = nullptr;
    int (*validate)(void *, void *) = nullptr;
    void (*freeValid)(void *) = nullptr;
    void *document = nullptr, *manifest = nullptr;   // the compiled schemas
    QString schemaWhy;

    ~Xml2()
    {
        if (!freeSchema) return;
        if (document) freeSchema(document);
        if (manifest) freeSchema(manifest);
    }
};

void collect(void *user, const XmlError *e)
{
    QStringList *into = static_cast<QStringList *>(user);
    if (!e || into->size() > 40) return;
    QString text = QString::fromUtf8(e->message).trimmed();
    if (e->node) {
        const XmlNode *n = static_cast<const XmlNode *>(e->node);
        if (n->name && (n->type == 1)) text += QStringLiteral(" [element ") + QString::fromUtf8(reinterpret_cast<const char *>(n->name)) + QLatin1Char(']');
    }
    into->append(QString::number(e->line) + QLatin1Char('\t') + text);
}

QByteArray readSchema(const char *name)
{
    QFile f(QStringLiteral(JO_ODF_SCHEMA_DIR) + QLatin1Char('/') + QLatin1String(name));
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

void *compile(Xml2 &x, const char *file, QString *why)
{
    const QByteArray bytes = readSchema(file);
    if (bytes.isEmpty()) {
        *why = QStringLiteral("the schema file %1 is not in %2").arg(QLatin1String(file), QStringLiteral(JO_ODF_SCHEMA_DIR));
        return nullptr;
    }
    void *doc = x.readMemory(bytes.constData(), int(bytes.size()), "schema.rng", nullptr, (1 << 11) | (1 << 5) | (1 << 6));   // no network, no messages
    if (!doc) {
        *why = QStringLiteral("libxml2 could not read %1").arg(QLatin1String(file));
        return nullptr;
    }
    QStringList errors;
    void *parser = x.newDocParser(doc);
    x.setParserErrors(parser, collect, &errors);
    void *schema = x.parse(parser);
    x.freeParser(parser);
    x.freeDoc(doc);
    if (!schema) *why = QStringLiteral("libxml2 could not compile %1: %2").arg(QLatin1String(file), errors.value(0));
    return schema;
}

Xml2 &xml2()
{
    static Xml2 x;
    static bool tried = false;
    if (tried) return x;
    tried = true;
    // (the system's own: the sonames of Linux, the Mac's system library, and the names a Windows toolchain gives it)
    const char *const names[] = {"libxml2.so.2", "libxml2.so", "/usr/lib/libxml2.2.dylib", "/usr/lib/libxml2.dylib", "libxml2-2", "libxml2-16", "libxml2"};
    for (const char *name : names) {
        x.lib.setFileName(QString::fromLatin1(name));
        if (x.lib.load()) break;
    }
    const QString platform = QSysInfo::prettyProductName();
    if (!x.lib.isLoaded()) {
        x.why = QStringLiteral("libxml2 is not on this machine (%1): it needs the system's libxml2 2.9 or later (libxml2.so.2, libxml2.2.dylib, or libxml2-2.dll on the path)").arg(platform);
        return x;
    }
#define RESOLVE(field, symbol) *reinterpret_cast<void **>(&x.field) = reinterpret_cast<void *>(x.lib.resolve(symbol))
    RESOLVE(readMemory, "xmlReadMemory");
    RESOLVE(freeDoc, "xmlFreeDoc");
    RESOLVE(newDocParser, "xmlRelaxNGNewDocParserCtxt");
    RESOLVE(setParserErrors, "xmlRelaxNGSetParserStructuredErrors");
    RESOLVE(parse, "xmlRelaxNGParse");
    RESOLVE(freeParser, "xmlRelaxNGFreeParserCtxt");
    RESOLVE(freeSchema, "xmlRelaxNGFree");
    RESOLVE(newValid, "xmlRelaxNGNewValidCtxt");
    RESOLVE(setValidErrors, "xmlRelaxNGSetValidStructuredErrors");
    RESOLVE(validate, "xmlRelaxNGValidateDoc");
    RESOLVE(freeValid, "xmlRelaxNGFreeValidCtxt");
#undef RESOLVE
    if (!x.readMemory || !x.freeDoc || !x.newDocParser || !x.setParserErrors || !x.parse || !x.freeParser || !x.freeSchema || !x.newValid || !x.setValidErrors || !x.validate || !x.freeValid) {
        x.why = QStringLiteral("the libxml2 of this machine (%1) has no RELAX NG validator").arg(platform);
        return x;
    }
    x.document = compile(x, "OpenDocument-v1.2-os-schema.rng", &x.schemaWhy);
    x.manifest = x.document ? compile(x, "OpenDocument-v1.2-os-manifest-schema.rng", &x.schemaWhy) : nullptr;
    x.ok = x.document && x.manifest;
    if (!x.ok) x.why = x.schemaWhy;
    return x;
}

bool standard(QStringView ns)
{
    return ns.isEmpty() || ns.startsWith(QLatin1String("urn:oasis:names:tc:opendocument:")) || ns == QLatin1String("http://www.w3.org/1999/xlink") || ns == QLatin1String("http://purl.org/dc/elements/1.1/") ||
           ns == QLatin1String("http://www.w3.org/1998/Math/MathML") || ns == QLatin1String("http://www.w3.org/XML/1998/namespace");
}

// The part without the names of other makers, each element on a line of its own (so that a message's line is the element's), and what was taken out. False if the part is not well formed.
bool strip(const QByteArray &xml, QByteArray *out, QStringList *privateNames, QString *why, bool asVersion12)
{
    QXmlStreamReader r(xml);
    QXmlStreamWriter w(out);
    w.setAutoFormatting(false);
    const auto note = [privateNames](const QString &ns, const QString &local) {
        const QString name = QLatin1Char('{') + ns + QLatin1Char('}') + local;
        if (!privateNames->contains(name)) privateNames->append(name);
    };
    bool first = true;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            if (!standard(r.namespaceUri())) {
                note(r.namespaceUri().toString(), r.name().toString());
                int depth = 1;
                while (depth > 0 && !r.atEnd()) {
                    r.readNext();
                    if (r.isStartElement()) ++depth;
                    else if (r.isEndElement()) --depth;
                }
                continue;
            }
            if (!first) w.writeCharacters(QStringLiteral("\n"));
            first = false;
            w.writeStartElement(r.qualifiedName().toString());
            for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations()) {
                if (d.prefix().isEmpty()) w.writeDefaultNamespace(d.namespaceUri().toString());
                else w.writeNamespace(d.namespaceUri().toString(), d.prefix().toString());
            }
            for (const QXmlStreamAttribute &a : r.attributes()) {
                if (!standard(a.namespaceUri())) {
                    note(a.namespaceUri().toString(), a.name().toString());
                    continue;
                }
                const bool version = asVersion12 && a.name() == QLatin1String("version") && a.value() == QLatin1String("1.3");
                w.writeAttribute(a.qualifiedName().toString(), version ? QStringLiteral("1.2") : a.value().toString());
            }
        } else if (r.isEndElement()) {
            w.writeEndElement();
        } else if (r.isCharacters()) {
            w.writeCharacters(r.text().toString());
        }
    }
    if (r.hasError()) {
        *why = QStringLiteral("not well formed: ") + r.errorString();
        return false;
    }
    return true;
}

} // namespace

QString unavailableReason()
{
    const Xml2 &x = xml2();
    return x.ok ? QString() : x.why;
}

Result checkPart(const QString &part, const QByteArray &xml, Kind kind, bool asVersion12)
{
    Result r;
    Xml2 &x = xml2();
    if (!x.ok) {
        r.skip = x.why;
        return r;
    }
    QByteArray clean;
    QString why;
    if (!strip(xml, &clean, &r.privateNames, &why, asVersion12)) {
        r.ran = true;
        r.errors << part + QLatin1String(": ") + why;
        return r;
    }
    void *doc = x.readMemory(clean.constData(), int(clean.size()), "part.xml", nullptr, (1 << 11) | (1 << 5) | (1 << 6) | (1 << 22));
    if (!doc) {
        r.ran = true;
        r.errors << part + QLatin1String(": libxml2 could not read the part");
        return r;
    }
    QStringList raw;
    void *valid = x.newValid(kind == Kind::Manifest ? x.manifest : x.document);
    x.setValidErrors(valid, collect, &raw);
    const int bad = x.validate(valid, doc);
    x.freeValid(valid);
    x.freeDoc(doc);
    r.ran = true;
    const QList<QByteArray> lines = clean.split('\n');
    for (const QString &e : raw) {
        const qsizetype tab = e.indexOf(QLatin1Char('\t'));
        const int line = e.left(tab).toInt();
        const QString at = line >= 1 && line <= lines.size() ? QString::fromUtf8(lines.at(line - 1).left(200)) : QString();
        r.errors << QStringLiteral("%1 line %2: %3 | %4").arg(part).arg(line).arg(e.mid(tab + 1), at);
    }
    if (bad && r.errors.isEmpty()) r.errors << part + QLatin1String(": the part does not validate");
    return r;
}

Result checkPackage(const QByteArray &zip, const QStringList &wanted, bool asVersion12)
{
    Result all;
    const QString unavailable = unavailableReason();
    if (!unavailable.isEmpty()) {
        all.skip = unavailable;
        return all;
    }
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(zip, parts)) {
        all.ran = true;
        all.errors << QStringLiteral("the package cannot be read");
        return all;
    }
    all.ran = true;
    for (const QString &name : wanted) {
        if (!parts.contains(name)) {
            all.errors << name + QLatin1String(": the package has no such part");
            continue;
        }
        const Result one = checkPart(name, parts.value(name), name.startsWith(QLatin1String("META-INF/")) ? Kind::Manifest : Kind::Document, asVersion12);
        all.errors += one.errors;
        for (const QString &n : one.privateNames)
            if (!all.privateNames.contains(n)) all.privateNames << n;
    }
    return all;
}

} // namespace odfcheck
