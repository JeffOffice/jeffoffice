#pragma once
// What a person chose when a text file was opened: how its fields are separated and quoted, the code page, the row to begin at, and how each column is read. The reader (csv.h) applies
// them; with none given it detects them. (Kept apart so that the limits every reader takes can carry the choices.)

#include <QChar>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

enum class CsvEncoding { Utf8, Utf8Bom, Utf16Le, Utf16Be, Legacy };

struct CsvColumn {
    enum class Kind : quint8 {
        General,   // read as a person would type it into the cell
        Text,      // kept as the text it is (a part number like 00123, a long account number)
        Date,      // a date in the order `dateOrder` says
        Skip,      // not brought in
    } kind = Kind::General;
    QString dateOrder;   // "MDY", "DMY", or "YMD", for a date
    bool operator==(const CsvColumn &o) const { return kind == o.kind && dateOrder == o.dateOrder; }
};

struct CsvImport {
    // The characters that separate fields (each one does); empty: the file's own, as the reader finds it. Several in a row count as one if `mergeSeparators`.
    QString separators;
    bool mergeSeparators = false;
    // The character that quotes a field that holds a separator or a line break (a quote inside is written twice); null for none.
    QChar quote = QLatin1Char('"');
    // The code page the file is read in, if the person chose one (else it is detected).
    bool encodingSet = false;
    CsvEncoding encoding = CsvEncoding::Utf8;
    int codePage = 1252;
    // The row (from 0) the data begins at; the rows before it are left out.
    int firstRow = 0;
    // How each column is read (the first is column 0), and the columns past the list.
    QVector<CsvColumn> columns;
    CsvColumn rest;
    CsvColumn columnAt(qsizetype col) const { return col >= 0 && col < columns.size() ? columns[col] : rest; }
    bool operator==(const CsvImport &o) const
    {
        return separators == o.separators && mergeSeparators == o.mergeSeparators && quote == o.quote && encodingSet == o.encodingSet && encoding == o.encoding && codePage == o.codePage
               && firstRow == o.firstRow && columns == o.columns && rest == o.rest;
    }
};

// The most columns and rows a person's choices are looked at for, and the most text of a field the preview shows.
constexpr int kMaxCsvColumnChoices = 256;
constexpr int kCsvPreviewRows = 200;
constexpr int kCsvPreviewColumns = 100;
constexpr int kCsvPreviewFieldChars = 120;
constexpr qint64 kCsvPreviewBytes = qint64(1) << 20;

} // namespace jp::office::sheet
