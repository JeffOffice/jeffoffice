#pragma once
// Whether an address or path names a file on another computer (a share). Such a file is never looked at, opened, or read unless the person agrees: touching it sends their
// credentials to that computer. One check for every place that follows a link (a slide's file link, a workbook's linked book).

#include "core/export.h"

#include <QString>

namespace jp {

// Whatever way it is written: a path with two slashes or backslashes in front (mixed, doubled, or percent-encoded, "\\?\UNC\host\share"), or a file: address with a host that is not
// this computer's ("file://host/share", "file://///host/share", "file:///\\host\share"). A "file:///path", a plain path, a drive path, a relative name, and a web address are not.
JO_COMMON_EXPORT bool isOnAnotherComputer(const QString &address);

} // namespace jp
