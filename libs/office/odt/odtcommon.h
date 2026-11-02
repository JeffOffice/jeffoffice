#pragma once
// What the OpenDocument reader and writer share: the names of the standard's namespaces, an element and attribute lookup by the standard's own prefixes (whatever prefix a file
// chose), and the conversions of lengths, colors, and names. Internal to libs/office/odt.

#include "wml/xmltree.h"

#include <QByteArray>
#include <QHash>
#include <QString>

namespace jp::office::odt {

// The namespaces of an OpenDocument file, by the prefix the standard gives each.
extern const char *const kNsOffice, *const kNsStyle, *const kNsText, *const kNsTable, *const kNsDraw, *const kNsFo, *const kNsSvg, *const kNsXlink, *const kNsDc, *const kNsMeta,
    *const kNsNumber, *const kNsManifest, *const kNsLoext, *const kNsChart, *const kNsOf;

// An XML tree looked at with OpenDocument's names: "text:p" is the element p of the text namespace, in the file's own prefix or none.
class Odf {
public:
    explicit Odf(const wml::XmlTree &tree);
    const wml::XmlTree &tree() const { return m_tree; }
    bool is(int n, const char *qname) const;
    bool has(int n, const char *qname) const;
    QString at(int n, const char *qname) const;
    int child(int n, const char *qname) const;
    // The standard's own "prefix:local" of an element or attribute.
    QString canonical(int ns, const QByteArray &local) const;
    QString nameOf(int n) const;   // of an element
private:
    int split(const char *qname, const char **local) const;   // the namespace number (-1 if the tree has none like it), and the local name
    const wml::XmlTree &m_tree;
    int m_ids[16] = {};   // the tree's namespace number of each of the standard's prefixes (in the order of the table in the .cpp)
    QHash<int, QByteArray> m_prefixes;
};

// A length ("2.5cm", "10pt", "0.5in") in points, twips, EMU; false (and 0) if it is none.
bool lengthToPoints(const QString &text, double *points);
qint64 lengthToTwips(const QString &text, bool *ok = nullptr);
qint64 lengthToEmu(const QString &text, bool *ok = nullptr);
// "#rrggbb" as "RRGGBB"; empty for anything else (and for "transparent").
QString colorHex(const QString &text);
// A length in inches written for the standard, from twips or EMU.
QString inchesFromTwips(double twips);
QString inchesFromEmu(double emu);
// A percentage "120%" as a number; false if it is none.
bool percentOf(const QString &text, double *value);

// The name an OpenDocument file gives a style (spaces written _20_) as the person sees it.
QString decodeStyleName(const QString &name);
// A name fit for an OpenDocument style: letters, digits, and _ . - only.
QString encodeStyleName(const QString &display);

QByteArray esc(const QString &text, bool attribute = true);

} // namespace jp::office::odt
