#pragma once
// Open Packaging Conventions (OPC, ECMA-376 part 2): the ZIP package of named
// parts that the presentation, word processor, and spreadsheet files all are.
// A package holds parts (XML, pictures, ...), says what each one is in
// [Content_Types].xml, and links them with relationships kept in "_rels"
// parts: one per source part, and /_rels/.rels for the package itself.
//
// Part names here are always absolute and decoded ("/ppt/slides/slide1.xml",
// a percent escape such as %20 read as the character it stands for) and are
// compared without regard to case, as OPC says.

#include <QByteArray>
#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::office {

// One link from a part (or from the package) to another part or to an
// address outside. For an internal link `target` is the part name it was
// resolved to; for an external one it is the address as written.
struct Relationship {
    QString id, type, target;
    bool external = false;
};

// Reads a package into memory. These files come from strangers: a damaged or
// hostile one (part names that climb out of the package or hold odd
// characters, more than 10,000 parts, more than 2 GB inflated, a DOCTYPE or
// bad XML in [Content_Types].xml or a relationship part, two parts whose
// names differ only in case) is refused with a message, never partly read.
class Package {
public:
    // False, with a message in `error`, if the data isn't a usable package.
    // A failed read leaves the package empty.
    bool read(const QByteArray &zip, QString *error = nullptr);
    bool readFile(const QString &path, QString *error = nullptr);
    // The same for a package of the open document kind (.odp and its kin):
    // the ZIP of named parts ("mimetype", "content.xml", "Pictures/a.png")
    // with no [Content_Types].xml or relationships, so it has no content types,
    // no relationships, and no main part. The refusals are the same.
    bool readOpenDocument(const QByteArray &zip, QString *error = nullptr);

    // Every part, in order of name. [Content_Types].xml and the relationship
    // parts are not listed: they have their own accessors below.
    QStringList partNames() const;
    bool hasPart(const QString &name) const;
    // The part's bytes; empty if there is no such part.
    QByteArray part(const QString &name) const;
    // The Override for the part, else the Default for its extension; empty if
    // the package says neither.
    QString contentType(const QString &name) const;

    // The relationships a part has; the package's own for source "/". Empty
    // if there are none.
    QVector<Relationship> relationships(const QString &source) const;
    // One by its Id; a Relationship with an empty id if there is none.
    Relationship relationship(const QString &source, const QString &id) const;
    // Those whose type ends with `typeSuffix` (say "/slideLayout"), so the
    // type URIs of both the transitional and the strict standard match.
    QVector<Relationship> relationshipsOfType(const QString &source, const QString &typeSuffix) const;
    // What the package's officeDocument relationship points to: the
    // presentation, document, or workbook. Empty if it has none.
    QString mainPart() const;

private:
    bool load(const QByteArray &zip, QString *error, bool opc);

    QMap<QString, QByteArray> m_parts;              // by part name, relationship parts too
    QHash<QString, QString> m_names;                // lower-cased part name -> name as stored
    QHash<QString, QString> m_defaults;             // lower-cased extension -> content type
    QHash<QString, QString> m_overrides;            // lower-cased part name -> content type
    QHash<QString, QVector<Relationship>> m_rels;   // lower-cased source ("/" or part name) -> its links
};

// Builds a package: add the parts and their relationships, then finish().
class PackageWriter {
public:
    // The part gets an Override for its content type unless the Default for
    // its extension is that type already (decided when the package is
    // finished, so the order of calls doesn't matter). Part names are
    // absolute and plain: letters, digits, and "._-" are safe, and a space is
    // fine, but no "..", "%", or backslash; one that isn't usable is left out,
    // with a warning. Adding a name again (in any case) replaces the part.
    void addPart(const QString &name, const QByteArray &data, const QString &contentType);
    // "xml" and "rels" have theirs from the start; this adds more (or
    // replaces one), such as ("png", "image/png"), which parts of that type
    // then need no Override for.
    void addDefault(const QString &extension, const QString &contentType);
    // Links `source` ("/" for the package) to `target` and returns the new
    // link's Id, "rId1" and on, counted for each source. An internal target
    // is the absolute name of a part and is written relative to the source
    // part's folder ("../media/image1.png"); an external one is written as
    // given.
    QString addRelationship(const QString &source, const QString &type, const QString &target, bool external = false);
    // The ZIP: [Content_Types].xml first, then the package's relationships,
    // the parts in the order added, and the other relationships; deflated.
    QByteArray finish();

private:
    struct Part { QString name; QByteArray data; QString contentType; };
    QVector<Part> m_parts;
    QMap<QString, QString> m_defaults;              // lower-cased extension -> content type
    QMap<QString, QVector<Relationship>> m_rels;    // by source; targets as written to the file
};

// The part a relationship target names, from the part that holds the link
// ("/" for the package): relative to that part's folder, or absolute when it
// starts with "/". "." and ".." are followed, a #fragment or ?query dropped,
// and percent escapes decoded. Empty if the result is no usable part name: it
// climbs above the root, has an empty segment or a backslash or a control
// character, or (after decoding) a "." or ".." or a "/" in a segment.
QString resolvePartName(const QString &sourcePart, const QString &target);

} // namespace jp::office
