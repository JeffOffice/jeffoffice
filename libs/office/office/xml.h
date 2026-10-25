#pragma once
// What every reader of these formats' XML needs: the namespaces, finding
// elements and attributes by namespace, and the simple types of ECMA-376
// (lengths in EMU, percentages, angles, booleans).

#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QLatin1String>
#include <QString>
#include <QVector>

namespace jp::office {

// The standard has two forms: transitional, which the files in use are
// written in, and strict, which has other namespace URIs. Everything below
// takes a namespace in either form and matches elements and attributes in
// both, so a reader is written once.
namespace ns {
inline constexpr QLatin1String A("http://schemas.openxmlformats.org/drawingml/2006/main");
inline constexpr QLatin1String AStrict("http://purl.oclc.org/ooxml/drawingml/main");
inline constexpr QLatin1String P("http://schemas.openxmlformats.org/presentationml/2006/main");
inline constexpr QLatin1String PStrict("http://purl.oclc.org/ooxml/presentationml/main");
inline constexpr QLatin1String R("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
inline constexpr QLatin1String RStrict("http://purl.oclc.org/ooxml/officeDocument/relationships");
// The package's own XML, which has one form only.
inline constexpr QLatin1String PackageRelationships("http://schemas.openxmlformats.org/package/2006/relationships");
inline constexpr QLatin1String ContentTypes("http://schemas.openxmlformats.org/package/2006/content-types");
}

// True if the two URIs are one namespace, whichever form each is in.
bool sameNamespace(const QString &a, const QString &b);

// What one XML part may hold. The tree it is read into takes 400 to 650 bytes
// a node, so a few bytes of XML repeated can fill the memory (5 million empty
// elements, 30 MB, took 2.3 GB), and the time grows with the square of the
// attributes on one element (20,000 took 10 seconds, and 100,000 minutes).
// Real files stay far below each: of 47 decks (10 theme files, 19 .pptx, and
// 18 .odp) the largest part is 1.4 MB, holds 54,000 nodes (elements,
// attributes, and runs of text, comments, or processing instructions), has 39
// attributes on its busiest element, and nests 23 deep. Each limit is at
// least 10 times that.
constexpr qint64 kMaxXmlBytes = qint64(64) << 20;
constexpr qint64 kMaxXmlNodes = 2000000;
constexpr int kMaxAttributesPerElement = 1000;
constexpr int kMaxXmlDepth = 256;

// Parses with namespaces on, keeping text that is only spaces (a run that is
// one space counts). False, with a message, if the XML is not well formed,
// has a DOCTYPE (so an entity is never defined, let alone loaded from
// outside), or passes one of the limits above (the parser is slow in depth,
// and a stranger's file could stall it or use up the memory).
bool parseXml(const QByteArray &xml, QDomDocument *doc, QString *error = nullptr);
// How many bytes of XML parseXml has handed to the DOM's parser since the program began (what it refused beforehand it did not): a count for tests, which a clock cannot give.
qint64 xmlBytesParsedByTheDomForTest();

// The first child element named `local` in namespace `ns`; null if none.
QDomElement child(const QDomElement &e, const QString &ns, const QString &local);
// All such children, in order; with no name, every child element.
QVector<QDomElement> children(const QDomElement &e, const QString &ns, const QString &local);
QVector<QDomElement> children(const QDomElement &e);
// Whether the element is `local` in `ns`.
bool isElement(const QDomElement &e, const QString &ns, const QString &local);

// An attribute with no namespace (the usual kind: val, w, name); `fallback`
// if there is none.
QString attr(const QDomElement &e, const QString &local, const QString &fallback = QString());
bool hasAttr(const QDomElement &e, const QString &local);
// One in a namespace, such as r:id; `ns` is the namespace's URI.
QString attrNs(const QDomElement &e, const QString &ns, const QString &local, const QString &fallback = QString());

// The simple types. Each returns `fallback` for an empty or malformed value
// and says so in `ok` when given one.
//
// A length is in EMU (English Metric Units, 12,700 to the point) as a plain
// number; strict files may also write a unit: 36pt, 1in, 2.54cm, 25.4mm,
// 1pc or 1pi (picas). Returned in points.
constexpr qint64 EmuPerPoint = 12700;
double emuToPoints(qint64 emu);
qint64 pointsToEmu(double points);
double parseLength(const QString &s, double fallback = 0, bool *ok = nullptr);
// A percentage: thousandths of a percent as a number ("100000" is 100%, and
// may be more), or in strict files with a percent sign ("100%"). Returned as
// a fraction: 1 is 100%.
double parsePercent(const QString &s, double fallback = 0, bool *ok = nullptr);
// An angle: 60,000ths of a degree ("5400000" is 90). Returned in degrees.
double parseAngle(const QString &s, double fallback = 0, bool *ok = nullptr);
// "1", "true", or "on" are true; "0", "false", or "off" are false.
bool parseBool(const QString &s, bool fallback = false, bool *ok = nullptr);
qint64 parseInt(const QString &s, qint64 fallback = 0, bool *ok = nullptr);

// The element and what is in it as XML that can be read by itself: every namespace it uses is declared in it, and each element's attributes come in a fixed order (namespace
// declarations first, then by qualified name), since the DOM's own order changes from run to run. What is saved from a DOM tree goes through this.
QString standaloneXml(const QDomElement &e);

} // namespace jp::office
