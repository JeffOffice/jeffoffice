#pragma once
// The text of bytes in a code page, for the old workbook format's text that is not Unicode (BIFF5 writes every text in the workbook's code page).

#include <QString>

namespace jp::office::sheet {

// The characters the bytes stand for in the code page (its number, as the format's code page record gives it); Latin-1 for a page this does not know.
QString xlsBytesToText(int codepage, const char *bytes, qsizetype count);

} // namespace jp::office::sheet
