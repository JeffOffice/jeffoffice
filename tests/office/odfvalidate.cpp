#include "odfvalidate.h"

#include "io/zip.h"

#include <QCoreApplication>
#include <QFile>
#include <QLibrary>
#include <QRegularExpression>
#include <QSysInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifndef JO_ODF_SCHEMA_DIR
#error "JO_ODF_SCHEMA_DIR names third_party/odf_schema (the test's CMake target defines it)"
#endif

namespace odfcheck {
namespace {

int validated = 0;
constexpr char ourNamespace[] = "urn:jeffoffice:odf-extension";
constexpr char ourObjectNamespace[] = "urn:jeffoffice:names:extension:1.0";
constexpr char ourNoteNamespace[] = "urn:jeffoffice:note";
constexpr char calcextNamespace[] = "urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0";
constexpr char loextNamespace[] = "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";

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

// What the validator said: levels 1 (warning) and up are told apart, so that a warning is not counted as an error.
struct Said {
    QStringList errors, warnings;
};

void collect(void *user, const XmlError *e)
{
    Said *said = static_cast<Said *>(user);
    if (!e || said->errors.size() + said->warnings.size() > 40) return;
    QString text = QString::fromUtf8(e->message).trimmed();
    if (e->node) {
        const XmlNode *n = static_cast<const XmlNode *>(e->node);
        if (n->name && (n->type == 1)) text += QStringLiteral(" [element ") + QString::fromUtf8(reinterpret_cast<const char *>(n->name)) + QLatin1Char(']');
    }
    sortMessage(e->level, QString::number(e->line) + QLatin1Char('\t') + text, &said->errors, &said->warnings);
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
    Said said;
    void *parser = x.newDocParser(doc);
    x.setParserErrors(parser, collect, &said);
    void *schema = x.parse(parser);
    x.freeParser(parser);
    x.freeDoc(doc);
    if (!schema) *why = QStringLiteral("libxml2 could not compile %1: %2").arg(QLatin1String(file), said.errors.value(0));
    return schema;
}

Xml2 &xml2()
{
    static Xml2 x;
    static bool tried = false;
    if (tried) return x;
    tried = true;
    // (the system's own: the sonames of Linux, the Mac's system library, and the names a Windows toolchain gives it; the newer releases changed the number: libxml2.so.16, libxml2.16.dylib, libxml2-16.dll)
    const char *const names[] = {"libxml2.so.16", "libxml2.so.2", "libxml2.so", "/usr/lib/libxml2.2.dylib", "/usr/lib/libxml2.16.dylib", "/usr/lib/libxml2.dylib", "libxml2.16.dylib", "libxml2.2.dylib", "libxml2.dylib", "libxml2-16", "libxml2-2", "libxml2"};
    QStringList attempts;
    for (const char *name : names) {
        attempts << QString::fromLatin1(name);
        x.lib.setFileName(attempts.last());
        if (x.lib.load()) break;
    }
    const QString platform = QSysInfo::prettyProductName();
    if (!x.lib.isLoaded()) {
        x.why = QStringLiteral("libxml2 is not on this machine (%1): it needs the system's libxml2 2.9 or later; tried: %2").arg(platform, attempts.join(QLatin1String(", ")));
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

// The part without the names of ours and of the other suites' extension namespaces (exact addresses, see isExtensionNamespace), each element on a line of its own (so that a message's line is the element's), and what was taken out.
// An element of an extension namespace is taken out but its standard children stay, in its place (counted in `wrappers`, so that a test can bound them); a name of any other namespace is an error. False if the part is not well formed.
bool strip(const QByteArray &xml, QByteArray *out, QStringList *privateNames, QStringList *errors, int *wrappers, const QString &part, QString *why, bool asVersion12)
{
    QXmlStreamReader r(xml);
    QXmlStreamWriter w(out);
    w.setAutoFormatting(false);
    const auto note = [&](const QString &ns, const QString &local) {
        const QString name = QLatin1Char('{') + ns + QLatin1Char('}') + local;
        if (!privateNames->contains(name)) privateNames->append(name);
        if (!isExtensionNamespace(ns)) {
            const QString error = part + QLatin1String(": a name of a namespace that is neither the standard's nor a listed extension: ") + name;
            if (!errors->contains(error)) errors->append(error);
        }
    };
    bool first = true;
    // for each open element: 0 kept, 1 taken out with its children kept, and whether a standard element was seen inside it
    struct Open {
        bool wrapper;
        bool standardInside;
    };
    QList<Open> open;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            if (!standard(r.namespaceUri())) {
                note(r.namespaceUri().toString(), r.name().toString());
                open.append({true, false});
                continue;
            }
            for (Open &o : open) o.standardInside = true;
            open.append({false, false});
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
            const Open o = open.takeLast();
            if (o.wrapper) {
                if (o.standardInside) ++*wrappers;
            } else {
                w.writeEndElement();
            }
        } else if (r.isCharacters()) {
            if (open.isEmpty() || !open.last().wrapper) w.writeCharacters(r.text().toString());
        }
    }
    if (r.hasError()) {
        *why = QStringLiteral("not well formed: ") + r.errorString();
        return false;
    }
    return true;
}

} // namespace

bool filteredRun()
{
    const QStringList args = QCoreApplication::arguments();
    for (qsizetype i = 1; i < args.size(); ++i)
        if (!args.at(i).startsWith(QLatin1Char('-'))) return true;
    return false;
}

void sortMessage(int level, const QString &text, QStringList *errors, QStringList *warnings)
{
    (level <= 1 ? warnings : errors)->append(text);
}

int validatedParts()
{
    return validated;
}

bool required()
{
    return qEnvironmentVariable("JO_REQUIRE_ODF_VALIDATE") == QLatin1String("1");
}

bool isExtensionNamespace(const QString &ns)
{
    return ns == QLatin1String(ourNamespace) || ns == QLatin1String(ourObjectNamespace) || ns == QLatin1String(ourNoteNamespace) || ns == QLatin1String(calcextNamespace) || ns == QLatin1String(loextNamespace);
}

QString unlistedPrivateName(const Result &r)
{
    for (const QString &name : r.privateNames) {
        const qsizetype end = name.indexOf(QLatin1Char('}'));
        if (!name.startsWith(QLatin1Char('{')) || end < 0 || !isExtensionNamespace(name.mid(1, end - 1))) return name;
    }
    return QString();
}

QStringList errorsWithout(const QStringList &errors, const QStringList &needles, const QStringList &containers)
{
    // The validator also reports the elements that hold the offending one ("Extra element X in interleave", "Element X failed to validate content", ...): those of the named containers belong to the gap.
    static const QRegularExpression phrase(QStringLiteral("Extra element|Invalid sequence in interleave|failed to validate|has extra content|Did not expect element"));
    QStringList left;
    for (const QString &e : errors) {
        bool known = false;
        for (const QString &n : needles) known = known || e.contains(n);
        if (!known && e.contains(phrase))
            for (const QString &c : containers) known = known || e.contains(QRegularExpression(QStringLiteral("[eE]lement ") + QRegularExpression::escape(c) + QStringLiteral("(?![\\w-])")));
        if (!known) left << e;
    }
    return left;
}

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
    if (!strip(xml, &clean, &r.privateNames, &r.errors, &r.wrappers, part, &why, asVersion12)) {
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
    Said said;
    void *valid = x.newValid(kind == Kind::Manifest ? x.manifest : x.document);
    x.setValidErrors(valid, collect, &said);
    const int bad = x.validate(valid, doc);
    x.freeValid(valid);
    x.freeDoc(doc);
    r.ran = true;
    r.parts = 1;
    ++validated;
    const QList<QByteArray> lines = clean.split('\n');
    const auto describe = [&](const QString &e) {
        const qsizetype tab = e.indexOf(QLatin1Char('\t'));
        const int line = e.left(tab).toInt();
        const QString at = line >= 1 && line <= lines.size() ? QString::fromUtf8(lines.at(line - 1).left(200)) : QString();
        return QStringLiteral("%1 line %2: %3 | %4").arg(part).arg(line).arg(e.mid(tab + 1), at);
    };
    for (const QString &e : said.errors) r.errors << describe(e);
    for (const QString &e : said.warnings) r.warnings << describe(e);
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
        all.warnings += one.warnings;
        all.parts += one.parts;
        all.wrappers += one.wrappers;
        for (const QString &n : one.privateNames)
            if (!all.privateNames.contains(n)) all.privateNames << n;
    }
    return all;
}

} // namespace odfcheck
