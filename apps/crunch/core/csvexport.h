#pragma once
// A sheet's values written as a file of delimited values (.csv, .tsv, .txt). By default: UTF-8 with the byte order mark (so that the program people read such files with takes accented
// letters right), each row ended by a carriage return and a line feed, each cell as it is shown (a number in its format, a date as a date, a formula as what it came to), a text in quotes
// when it holds the separator, a quote (written twice), a line break, or a space at either end. The person may choose the separator, the encoding (UTF-8 with or without the mark, UTF-16, or
// a single-byte code page), the line ends, and whether the cells are written as shown or as stored (every digit of a number, a date as ISO text). The rows are the sheet's, up to the last
// row and column that hold something, hidden ones too. It is written in slices of a few milliseconds, to a file of its own that takes the name when it is whole, so that a file that is there
// is never half written over.

#include "grid/gridsheet.h"

#include <QSaveFile>
#include <QString>

#include <memory>

namespace jp::crunch {

// How a sheet is written as a file of values.
struct CsvWrite {
    enum class Encoding { Utf8Mark, Utf8, Utf16Le, CodePage } encoding = Encoding::Utf8Mark;
    int codePage = 1252;                       // for CodePage: a single-byte page
    QChar separator = QLatin1Char(',');
    QString lineEnd = QStringLiteral("\r\n");   // "\r\n", "\n", or "\r"
    bool asStored = false;                      // every digit of a number and a date as ISO text, not as the cells show them
    // These are the choices of the file the workbook came from (its separator, line ends, and encoding), unchanged: a sheet not edited is written as the file's own bytes.
    bool sameAsFile = false;
    bool operator==(const CsvWrite &o) const
    {
        return encoding == o.encoding && (encoding != Encoding::CodePage || codePage == o.codePage) && separator == o.separator && lineEnd == o.lineEnd && asStored == o.asStored;
    }
    bool operator!=(const CsvWrite &o) const { return !(*this == o); }
};

class CsvExport {
public:
    // The sheet is read as it is (its own model, not the window's view of it), and not changed.
    CsvExport(const jp::grid::GridSheet *sheet, QChar separator);
    CsvExport(const jp::grid::GridSheet *sheet, const CsvWrite &how);
    // Opens the file to write; false, with a message, if it cannot be.
    bool begin(const QString &path, QString *error);
    // Writes about `budget` cells (0 for all); true when everything is written (or it failed: see failed()).
    bool step(qint64 budget);
    // Puts the file in its place; false, with a message, if that fails.
    bool commit(QString *error);
    void cancel();
    bool failed() const { return !m_error.isEmpty(); }
    QString error() const { return m_error; }
    qint64 rows() const { return m_lastRow >= 0 ? m_lastRow + 1 : 0; }
    qint64 columns() const { return m_lastCol >= 0 ? m_lastCol + 1 : 0; }
    qint64 rowsWritten() const { return m_row; }
    // How many characters the encoding has no byte for (each written as "?"), once the file is written.
    qint64 replaced() const { return m_replaced; }
    // Whether the encoding is one this can write (a single-byte page is; a double-byte page is not: the file is written in UTF-8 with the mark, and encoding() says so).
    CsvWrite::Encoding encoding() const { return m_how.encoding; }

private:
    void field(const QString &text);
    bool flush(bool force);
    QByteArray encoded(const QString &text);

    const jp::grid::GridSheet *m_sheet;
    CsvWrite m_how;
    std::unique_ptr<QSaveFile> m_file;
    QString m_text;
    QByteArray m_buffer;
    qint64 m_lastRow = -1, m_lastCol = -1, m_row = 0, m_replaced = 0;
    QString m_error;
    bool m_begun = false;
};

// Whether the single-byte page can be written (it is one of the program's own tables).
bool csvCanWriteCodePage(int codePage);

} // namespace jp::crunch
