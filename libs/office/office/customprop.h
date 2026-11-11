#pragma once
// A custom property of a package file (see packageprops.h), kept apart so a header that only holds a list of them need not include the rest.

#include <QByteArray>
#include <QString>

namespace jp::office {

// A property of a person's own. `type` is the name of its value's type in the file: "lpwstr" (text), "i4" (a whole number), "r8" (a number), "bool" (true or false), or "filetime" (a date).
// A property this program does not model (a vector, a date of another form, a type it does not know) is kept as the XML the file had, `raw`, and written back as it was; `pid` and `fmtid`
// are the file's own (0 and empty for a property made here).
struct CustomProperty {
    QString name, type, value;
    int pid = 0;
    QString fmtid;
    QByteArray raw;
    bool operator==(const CustomProperty &o) const { return name == o.name && type == o.type && value == o.value && pid == o.pid && fmtid == o.fmtid && raw == o.raw; }
};

} // namespace jp::office
