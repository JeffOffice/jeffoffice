#pragma once
// The text of bytes in a code page, for the old workbook format's text that is not Unicode (BIFF5 writes every text in the workbook's code page).
//
// The single-byte pages are tables of this program's own. The double-byte pages (932 Japanese, 936 simplified Chinese, 949 Korean, 950 traditional Chinese, 1361 Korean Johab) are
// the system's to convert, so that they do not depend on how Qt was built: Windows' own conversion, CoreFoundation's on a Mac, and elsewhere ICU through Qt, or the system's iconv
// when Qt was built without it.

#include <QString>

namespace jp::office::sheet {

// Which converter reads the double-byte pages: the one this platform prefers (Automatic), the system's own (Windows' conversion, CoreFoundation, or iconv), or Qt's (ICU or iconv,
// whatever Qt was built with). Only a test names one.
enum class CodePageBackend { Automatic, System, Qt };

// The characters the bytes stand for in the code page (its number, as the format's code page record gives it); Latin-1 for a page this does not know (or the converter lacks).
QString xlsBytesToText(int codepage, const char *bytes, qsizetype count, CodePageBackend backend = CodePageBackend::Automatic);

// Whether the converter has the double-byte page on this machine.
bool xlsCanConvert(int codepage, CodePageBackend backend);

} // namespace jp::office::sheet
