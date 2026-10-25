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
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <memory>

class QIODevice;

namespace jp::office {

// One link from a part (or from the package) to another part or to an
// address outside. For an internal link `target` is the part name it was
// resolved to; for an external one it is the address as written.
struct Relationship {
    QString id, type, target;
    bool external = false;
};

// The most a file the apps open may be: each is read whole into memory before
// it is read (a .docx, .pptx, or .odp is a ZIP read from memory, a .doc or
// .ppt a compound file), so a larger one is refused before anything is read.
// 1 GB, as the .doc reader allows.
constexpr qint64 kMaxOpenFileBytes = qint64(1) << 30;

// The whole of the file at `path` in `out`; false, with a message in `error`,
// when it can't be opened or is larger than `most` (nothing is read then).
bool readWholeFile(const QString &path, QByteArray *out, QString *error = nullptr, qint64 most = kMaxOpenFileBytes);

// Reads a package into memory. These files come from strangers: a damaged or
// hostile one (more than 10,000 parts, more than 2 GB inflated, a DOCTYPE or
// bad XML in [Content_Types].xml or a relationship part) is refused with a
// message, never partly read. What is wrong with one part or one link only
// leaves that out, and the rest is read: a part whose name climbs out of the
// package or holds odd characters (an empty segment, a "." or "..", a control
// character, or an encoded slash) is never readable as a part, a part whose
// name differs only in case from an earlier one's is dropped (the first in
// the file stands), and a relationship whose target leads out of the package
// or is no name a part could have is dropped as if the file had not listed it.
// Names a tool wrote with backslashes, or with a slash in front, are read as
// the standard writes them. skippedCount() says how many were left out.
class Package {
public:
    // What all the parts together may come to when inflated, unless the
    // caller says less.
    static constexpr qint64 kDefaultMaxInflated = qint64(1) << 31;

    // False, with a message in `error`, if the data isn't a usable package.
    // A failed read leaves the package empty. `maxInflated` is the most the
    // parts may come to together, once inflated: a caller that opens small
    // packages (one that was inside another file) gives less. A package that
    // would pass it is refused from the sizes the ZIP directory gives, before
    // anything is inflated, and an entry that inflates to other than the size
    // the directory gives is damage, so the directory cannot understate it.
    bool read(const QByteArray &zip, QString *error = nullptr, qint64 maxInflated = kDefaultMaxInflated);
    bool readFile(const QString &path, QString *error = nullptr);
    // The same for a package of the open document kind (.odp and its kin):
    // the ZIP of named parts ("mimetype", "content.xml", "Pictures/a.png")
    // with no [Content_Types].xml or relationships, so it has no content types,
    // no relationships, and no main part. The refusals and what is left out
    // are the same.
    bool readOpenDocument(const QByteArray &zip, QString *error = nullptr);

    // The same refusals and the same names, but a part is kept compressed in
    // the file's bytes until it is asked for, and inflated then: part() gives
    // it whole and openPart() as a stream. [Content_Types].xml and the
    // relationship parts are read now, as they are what a package is made of.
    // The total size, the parts count, and the names are refused as read()
    // refuses them; what read() finds wrong with a part while inflating it (a
    // size or CRC that doesn't match the directory) is found only when the
    // part is asked for, which gives an empty part and (openPart) a failed
    // stream. For a package whose parts may be far larger than the rest of
    // the file needs, such as the sheets of a spreadsheet.
    // What all the parts of a lazy package may come to, together and over its life: at least ten times the most that
    // real workbooks have (302 MB). Nothing is held in memory but what a caller asks to be (part()); openPart() streams.
    static constexpr qint64 kLazyMaxInflated = qint64(4) << 30;
    bool readLazy(const QByteArray &zip, QString *error = nullptr, qint64 maxInflated = kLazyMaxInflated);
    bool readLazyFile(const QString &path, QString *error = nullptr);
    // The same for a package of the open document kind (see readOpenDocument), kept compressed until a part is asked for.
    bool readLazyOpenDocument(const QByteArray &zip, QString *error = nullptr, qint64 maxInflated = kLazyMaxInflated);
    bool isLazy() const;
    // For a lazy package: the file's bytes (shared, not copied), and the name the file gives the entry of a part (as the ZIP directory spells it, without the slash a part name
    // starts with, perhaps); empty for a part it does not have. Saving copies the entries of the parts that did not change from these bytes as they are.
    QByteArray fileBytes() const;
    QString entryName(const QString &partName) const;

    // How many parts and relationships the last read left out as unusable.
    int skippedCount() const;

    // Every part, in order of name. [Content_Types].xml and the relationship
    // parts are not listed: they have their own accessors below.
    QStringList partNames() const;
    bool hasPart(const QString &name) const;
    // The part's bytes; empty if there is no such part, or if a lazy package could not inflate it (it is damaged, or past what a part or the package may
    // come to). A lazy package inflates it now, each time it is asked for, and keeps nothing: a caller that needs it twice keeps what it was given. The whole
    // part is held in memory, so a part that may be large is read with openPart.
    QByteArray part(const QString &name) const;
    // How many bytes the part holds once inflated; -1 if there is no such part.
    qint64 partSize(const QString &name) const;
    // The part as a stream, inflated as it is read (see ZipEntryDevice); null
    // if there is no such part. The package must outlive the stream.
    std::unique_ptr<QIODevice> openPart(const QString &name) const;
    // How many bytes a lazy package has inflated so far: what read() did at
    // once, counted as it happens, and the most it may come to over the package's whole life
    // is what it was allowed to open with (maxInflated): a part asked for again and again, or
    // many parts that are one, can't multiply the work; what would pass it is refused.
    // (For a package read whole, all its parts' bytes.)
    qint64 inflatedBytes() const;
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
    struct Lazy;
    bool load(const QByteArray &zip, QString *error, bool opc, qint64 maxInflated, bool lazy = false);

    QMap<QString, QByteArray> m_parts;              // by part name, relationship parts too
    QHash<QString, QString> m_names;                // lower-cased part name -> name as stored
    QHash<QString, QString> m_defaults;             // lower-cased extension -> content type
    QHash<QString, QString> m_overrides;            // lower-cased part name -> content type
    QHash<QString, QVector<Relationship>> m_rels;   // lower-cased source ("/" or part name) -> its links
    QHash<QString, QHash<QString, int>> m_relIds;   // lower-cased source -> each link's Id -> where it is in m_rels
    int m_skipped = 0;                              // parts and relationships left out as unusable
    std::shared_ptr<Lazy> m_lazy;                   // the file's bytes and where each part is in them, for a lazy package
};

// Builds a package: add the parts and their relationships, then finish().
class PackageWriter {
public:
    // The part gets an Override for its content type unless the Default for
    // its extension is that type already (decided when the package is
    // finished, so the order of calls doesn't matter). Part names are
    // absolute and decoded, as the package reads them: any character may be
    // in one (a space, a "%"; the package writes each escaped as a URI needs),
    // but no "." or ".." or empty segment, control character, or "/" inside
    // a segment; one that isn't usable is left out, with a warning. Adding a
    // name again (in any case) replaces the part.
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
    // The same with an Id of its own, for a link that something in the source part refers to by that Id (a relationship read from a
    // package keeps its Id when it is written again). An Id the source already has is not added again; false then.
    bool addRelationshipWithId(const QString &source, const QString &id, const QString &type, const QString &target, bool external = false);
    // The ZIP: [Content_Types].xml first, then the package's relationships,
    // the parts in the order added, and the other relationships; deflated.
    QByteArray finish();

private:
    struct Part { QString name; QByteArray data; QString contentType; };
    QVector<Part> m_parts;
    QHash<QString, int> m_partAt;                   // lower-cased part name -> its place in m_parts
    QMap<QString, QString> m_defaults;              // lower-cased extension -> content type
    QMap<QString, QVector<Relationship>> m_rels;    // by source; targets as written to the file
    QHash<QString, QSet<QString>> m_relIds;         // the Ids each source's links have
    QHash<QString, int> m_nextRelId;                // where the next Id to try is, for each source
};

// The part a relationship target names, from the part that holds the link
// ("/" for the package): relative to that part's folder, or absolute when it
// starts with "/". "." and ".." are followed, a #fragment or ?query dropped,
// and percent escapes decoded. Empty if the result is no usable part name: it
// climbs above the root, has an empty segment or a backslash or a control
// character, or (after decoding) a "." or ".." or a "/" in a segment.
QString resolvePartName(const QString &sourcePart, const QString &target);

} // namespace jp::office
