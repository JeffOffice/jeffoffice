#pragma once
// What one Open and Repair of a workbook looks at, for a test of its bounds. The program never reads it: a caller that wants the numbers passes its own object to salvageXlsStream, so
// no count is shared between calls or threads.

#include <QtGlobal>

namespace jp::office::sheet {

constexpr int kSalvageSheetsReported = 8;   // the sheets the notes list one by one (and read, to say how far their cells go)

struct XlsSalvageWork {
    qint64 recordsWalked = 0;   // the records walked to tell whether each sheet ends where it should, and to cut the stream where its records stop (a few times the stream and a megabyte, in all)
    qint64 streamBytes = 0;     // the bytes of the stream the records were walked in
    qint64 joinSteps = 0;       // the records and joins gone through to leave out what lies across a guessed join (once each: the records of the stream plus the joins)
    int sheetsRead = 0;         // the sheets read to tell how far their cells go (at most kSalvageSheetsReported)
};

} // namespace jp::office::sheet
