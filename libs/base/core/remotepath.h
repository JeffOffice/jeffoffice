#pragma once
// Whether an address or path names a file on another computer (a share). Such a file is never looked at, opened, or read unless the person agrees: touching it sends their
// credentials to that computer. One check for every place that follows a link (a slide's file link, a workbook's linked book).

#include "core/export.h"

#include <QString>

namespace jp {

// Whatever way it is written: a path with two slashes or backslashes in front (mixed, doubled, or percent-encoded, "\\?\UNC\host\share"), or a file: address with a host that is not
// this computer's ("file://host/share", "file://///host/share", "file:///\\host\share"). A "file:///path", a plain path, a drive path, a relative name, and a web address are not.
JO_COMMON_EXPORT bool isOnAnotherComputer(const QString &address);

// Names Windows reads as something other than a plain file on a drive: a path with a colon after the drive letter (an alternate data stream, "a.mp3:stream"), and a name that is a device
// (CON, PRN, AUX, NUL, COM1 to COM9, LPT1 to LPT9, with any ending, in any folder; trailing dots and spaces do not hide it). `streams` false leaves the colon alone (a system where a
// colon is an ordinary character in a name, but the device names are still refused: they are no name anyone gives a file). A drive letter's own colon, and the \\?\ and \\.\ forms of a
// drive, are fine.
JO_COMMON_EXPORT bool isWindowsSpecialPath(const QString &path, bool streams = true);

} // namespace jp
