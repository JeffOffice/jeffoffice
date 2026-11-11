#pragma once
// "Remove personal information from this file when saving", the same in all four apps. What it takes out: the names in the properties (the author, the person who last saved, the company,
// and the manager); the names and initials on comments and notes, with the name that begins an Excel-style note's text; the names on tracked changes and in revision logs; the people that
// threads of comments list; the user a scenario names; the person who printed an open document; and the name of the person who shared a file for editing; and every time of a comment or a
// change. What the file says is not touched: the words, numbers, pictures, and the dates of the file itself stay as they were. The file's custom properties are left as they are.

#include "office/packageprops.h"

#include <QByteArray>
#include <QStringList>
#include <functional>

namespace jp::office {

// What a name is made into where a file needs one (a comment must have an author).
extern const char *const kAnonymousAuthor;

// Clears the four names of the properties; how many were there.
int removePersonalNames(PackageProps &p);

// The bytes of a package file (a ZIP: the .docx, .pptx, .xlsx kinds and the open document kinds) with the names, initials, and times taken out of the parts that hold them. Only real attributes
// and elements are changed (never text that looks like one). Every other entry is copied as it is, to the last bit. `parts`, when given, says how many parts were changed.
// When something can't be done (the bytes are not a readable ZIP, a part that has to be rewritten can't be inflated or is not UTF-8, or the parts passed the budget), the entry or the file is
// kept as it was, and `problems` (when given) gets a plain sentence for each: the caller tells the person that the names were not all taken out.
QByteArray removePersonalFromPackage(const QByteArray &zip, int *parts = nullptr, QStringList *problems = nullptr);

// For tests: a function called with the package's bytes before they are read, which may damage them (the way a part that can't be read is made). None clears it.
void setPackageScrubHookForTest(std::function<QByteArray(const QByteArray &)> hook);

} // namespace jp::office
