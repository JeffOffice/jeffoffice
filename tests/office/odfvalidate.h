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
    QStringList warnings;         // what the validator said below the level of an error, apart
    int parts = 0;                // how many parts were really validated (a test asserts it is above zero)
    int wrappers = 0;             // how many extension elements were taken out that had standard elements inside (their children stay in place)
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

// Where a message of the validator goes: level 1 (a warning) apart, level 2 and up (an error, a fatal error) with the errors. (The collector of the check uses this.)
void sortMessage(int level, const QString &text, QStringList *errors, QStringList *warnings);

// How many parts this program has validated so far (each test class asserts that its own share is above zero when the validator is present).
int validatedParts();

// Whether the program was started with the names of test functions (a run of a few tests, which need not validate anything).
bool filteredRun();

// JO_REQUIRE_ODF_VALIDATE=1 in the environment: a machine with no validator is then a failure, not a skip (set where the gate and CI run the tests).
bool required();

// Whether a namespace is one of ours or of another suite's listed extensions (exact addresses); the part check takes these names out, and any other maker's name is an error.
bool isExtensionNamespace(const QString &ns);

// The first name in `r.privateNames` that is not in a listed extension namespace (empty if there is none).
QString unlistedPrivateName(const Result &r);

// The errors that are not a known gap's: those that mention none of `needles`, and are not the validator's report of a failed container (a "failed to validate", "extra content", "Extra element ... in
// interleave" message) of one of the named `containers`, which the gap causes.
QStringList errorsWithout(const QStringList &errors, const QStringList &needles, const QStringList &containers = QStringList());

// Whether the check can run here, and if not, why (naming the platform and what is needed).
QString unavailableReason();

} // namespace odfcheck

// In a test: the check did not run. A skip, unless the environment requires the validator (then it fails, so that a validator that silently stopped being found is noticed).
#define ODF_SKIP_OR_FAIL(reason)                                                                                                                  \
    do {                                                                                                                                         \
        if (odfcheck::required()) QFAIL(qPrintable(QStringLiteral("JO_REQUIRE_ODF_VALIDATE is set, and the validator did not run: ") + (reason))); \
        QSKIP(qPrintable(QStringLiteral("not checked: ") + (reason)));                                                                           \
    } while (0)

// In a test class that validates: at the end, if the validator is present, more parts were checked than at the start (`before` is validatedParts() from initTestCase).
#define ODF_ASSERT_VALIDATED(before)                                                                                                             \
    do {                                                                                                                                         \
        if (odfcheck::unavailableReason().isEmpty() && !odfcheck::filteredRun()) QVERIFY2(odfcheck::validatedParts() > (before), "the validator is present, but no part was validated in this class"); \
    } while (0)
