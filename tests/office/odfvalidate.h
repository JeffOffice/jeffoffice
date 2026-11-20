// Checks the parts of a written OpenDocument package against the standard's RELAX NG schemas (version 1.2, third_party/odf_schema), with the system's libxml2 loaded when the test runs (nothing
// is linked: a machine without it reports why, and the test that asked skips there). Names and attributes of a maker that is not the standard's (ours, jo:, and the other program's names the
// format has no words for) make a document an "extended" one by the standard's own terms: they are taken out before the check and listed apart, so that what is left is the standard's own part.
#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>

namespace odfcheck {

struct Result {
    bool ran = false;
    QString skip;                 // why the check did not run (the machine has no libxml2, or the schema folder is missing)
    QStringList errors;           // "part line N: the validator's message | the offending element's start"
    QStringList privateNames;     // "{namespace}name" of each element and attribute taken out
};

// The schema is shared by the parts that use the document schema; the manifest has its own.
enum class Kind { Document, Manifest };

// Checks one part's bytes. `part` names it in the messages.
// `asVersion12`: the part says it is a later version (1.3) of the format; its version attribute is read as 1.2 (the schema here is 1.2's), and what the later version added shows as errors.
Result checkPart(const QString &part, const QByteArray &xml, Kind kind = Kind::Document, bool asVersion12 = false);

// Checks the parts of a package that the schema covers: content.xml, styles.xml, settings.xml, and the manifest, each when `wanted` names it ("content.xml", "styles.xml", "settings.xml",
// "META-INF/manifest.xml"); a wanted part that is missing is an error.
Result checkPackage(const QByteArray &zip, const QStringList &wanted, bool asVersion12 = false);

// Whether the check can run here, and if not, why (naming the platform and what is needed).
QString unavailableReason();

} // namespace odfcheck
