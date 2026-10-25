#pragma once
// The printing records of a sheet of the old binary workbook form (SETUP, HEADER and FOOTER, the margins, the print options, the fit-to-page flag, the breaks, and the extra record for the
// first and even pages), read into the page setup. The print area and the titles are names (built-in names 6 and 7), read with the workbook's other names.

#include "sheet/pagesetup.h"

#include <QByteArray>
#include <QString>

#include <functional>

namespace jp::office::sheet {

// Reads the record `id` whose bytes are data[pos, pos + len) into the setup. Gives true when it is one of the printing records (whether or not it could be read: a record that is cut short
// changes nothing). `bytesToText` turns the bytes of a text of BIFF5 into text (the workbook's code page).
bool readXlsPrintRecord(quint16 id, const QByteArray &data, qsizetype pos, qsizetype len, bool biff8, const std::function<QString(const char *, int)> &bytesToText, PageSetup *out);

} // namespace jp::office::sheet
