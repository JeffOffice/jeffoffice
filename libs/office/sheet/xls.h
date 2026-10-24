#pragma once
// A workbook in the old binary format (BIFF8, the form of 1997 to 2003, BIFF5, the form of 1993 to 1995, and BIFF3 and BIFF4, the single sheets of 1991 to 1992), read into the same model the XML reader fills: the sheets' names and
// states, the strings, the styles, the defined names, and each sheet's cells, formulas (turned back into text), merged ranges, sizes, and view. The stream of a
// workbook is a string of records, each a number, a length, and that many bytes; a record that is too long for its length is continued in the records that follow it.
// The sheets are found by their place in the stream and read one at a time when asked for, as the XML reader does. Every length and count a file gives is checked
// against the bytes there are and against the limits, nothing recurses, and nothing is made in the size a file claims before the bytes for it are seen.
//
// Not read (kept out of the way, and said so by skipped()): charts, drawing objects and pictures, pivot tables, comments, and the macros of a workbook. A workbook that
// is encrypted is refused.

#include "sheet/sheetreader.h"
#include "sheet/xlsformula.h"

#include <QByteArray>
#include <QStringList>

#include <functional>
#include <memory>

namespace jp::office::sheet {

// True if the bytes begin as a compound file (the container the old format lives in): the signature of 8 bytes.
bool startsCompoundFile(const QByteArray &head);

class XlsBook {
public:
    // Reads the workbook stream's globals: fills `info` (sheets, names, date system, how it calculates, the view), `shared`, and `styles`. False, with a message, if the
    // stream is not a workbook, is encrypted, or is past a limit. `hasMacros` says the container held a macro project.
    static std::shared_ptr<const XlsBook> open(const QByteArray &stream, bool hasMacros, const Limits &limits, WorkbookInfo *info, SharedStrings *shared, Styles *styles,
                                               QString *error);

    // Reads sheet `index` into `out` (cleared first). True if all of its records were read; else `out` has what came before the problem and out->error says what it was.
    bool readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, Sheet *out) const;

    // What the file holds that is not read, in words ("charts", "pictures and other drawing objects", ...), for the person to be told.
    QStringList skipped() const { return m_skipped; }
    // 4 for BIFF3 and BIFF4 (one sheet to a workbook), 5 for BIFF5, 8 for BIFF8.
    int version() const { return m_biff; }

private:
    XlsBook() = default;
    friend class XlsGlobals;
    friend class XlsSheetReader;

    struct Place {
        qsizetype offset = -1;
        int type = 0;   // the substream type: 0x10 a worksheet, 0x40 a macro sheet, 0x20 a chart, 0x06 a module
    };
    QByteArray m_data;
    int m_biff = 8;
    int m_codepage = 1252;
    QVector<Place> m_places;
    QStringList m_skipped;
    XlsContext m_cx;
    qint64 m_sharedStringCount = 0;
    qsizetype m_xfCount = 0;   // the cell formats of the file (the places of the first and the 16th are exchanged: see mapXf)
};

} // namespace jp::office::sheet
