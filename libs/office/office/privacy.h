#pragma once
// "Remove personal information from this file when saving", the same in all four apps. What it takes out: the names in the properties (the author, the person who last saved, the
// company, and the manager); the names and initials on comments and notes; the names and times on tracked changes; the names that threads of comments list; the user a scenario names;
// and the name of the person who shared a file for editing. What the file says is not touched: the words, numbers, pictures, and the dates of the file itself stay as they were.

#include "office/packageprops.h"

#include <QByteArray>

namespace jp::office {

// What a name is made into where a file needs one (a comment must have an author).
extern const char *const kAnonymousAuthor;

// Clears the four names of the properties; how many were there.
int removePersonalNames(PackageProps &p);

// The bytes of a package file (a ZIP: the .docx, .pptx, .xlsx kinds and the open document kinds) with the names, the initials, and the times of changes taken out of the parts that hold them.
// Every other entry is copied as it is, to the last bit. Bytes that are not a readable ZIP are given back as they were. `parts`, when given, says how many parts were changed.
QByteArray removePersonalFromPackage(const QByteArray &zip, int *parts = nullptr);

} // namespace jp::office
