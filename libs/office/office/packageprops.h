#pragma once
// The properties of a package file (.docx, .xlsx, .pptx): the core part (title, author, dates...), the extended part (company, manager, counts), and the custom part (names, types,
// and values of a person's own). Reading takes the three parts' bytes; writing takes the part as it was and gives it back with only the properties here changed, so what the part holds
// besides (elements this program doesn't know) stays as it was.

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVector>

#include "office/package.h"

namespace jp::office {

// A property of a person's own. `type` is the name of its value's type in the file: "lpwstr" (text), "i4" (a whole number), "r8" (a number), "bool" (true or false), or "filetime" (a date).
struct CustomProperty {
    QString name, type, value;
    bool operator==(const CustomProperty &o) const { return name == o.name && type == o.type && value == o.value; }
};

struct PackageProps {
    QString title, subject, creator, keywords, comments, lastModifiedBy, category, revision;
    QString company, manager;
    QDateTime created, modified, printed;   // (kept as UTC in the file)
    // The counts the extended part holds; -1 is "not known", and a count that is -1 is left as the part has it when written.
    int pages = -1, words = -1, characters = -1, charactersWithSpaces = -1, lines = -1, paragraphs = -1;
    qint64 totalMinutes = -1;
    QVector<CustomProperty> custom;
};

// The properties of the three parts (any may be empty: a package that has none of that part).
PackageProps readPackageProps(const QByteArray &core, const QByteArray &app, const QByteArray &custom);

// The properties of a package, read through the relationships that name the three parts (the parts of a package that has none are empty).
PackageProps readPackageProps(const Package &package);

// Each part with the properties of `p` put in: the text properties are set, and taken out when empty; a date is taken out when it isn't valid; a count that is -1 is left as it was.
QByteArray writeCoreProps(const QByteArray &existing, const PackageProps &p);
QByteArray writeAppProps(const QByteArray &existing, const PackageProps &p);
// The custom part holding exactly `p.custom` (a name used twice keeps the first; a name that is empty or a type that isn't one of the five is left out).
QByteArray writeCustomProps(const PackageProps &p);

// The custom property that says the file was marked as final, and whether it is there and true.
extern const char *const kMarkAsFinalName;
bool isMarkedFinal(const QVector<CustomProperty> &custom);
// The list with the mark set or taken off.
QVector<CustomProperty> withMarkAsFinal(QVector<CustomProperty> custom, bool on);

constexpr const char *kCorePartType = "application/vnd.openxmlformats-package.core-properties+xml";
constexpr const char *kAppPartType = "application/vnd.openxmlformats-officedocument.extended-properties+xml";
constexpr const char *kCustomPartType = "application/vnd.openxmlformats-officedocument.custom-properties+xml";
constexpr const char *kCoreRelationship = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";
constexpr const char *kAppRelationship = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties";
constexpr const char *kCustomRelationship = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties";

} // namespace jp::office
