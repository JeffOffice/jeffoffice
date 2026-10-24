#pragma once
// A sheet's values written as a .csv file: UTF-8 with the byte order mark (so that the program people read such files with takes accented letters right), each row ended by a
// carriage return and a line feed, each cell as it is shown (a number in its format, a date as a date, a formula as what it came to), a text in quotes when it holds the separator,
// a quote (written twice), or a line break. The rows are the sheet's, up to the last row and column that hold something, hidden ones too. It is written in slices of a few milliseconds,
// to a file of its own that takes the name when it is whole, so that a file that is there is never half written over.

#include "grid/gridsheet.h"

#include <QSaveFile>
#include <QString>

#include <memory>

namespace jp::crunch {

class CsvExport {
public:
    // The sheet is read as it is (its own model, not the window's view of it), and not changed.
    CsvExport(const jp::grid::GridSheet *sheet, QChar separator);
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

private:
    void field(const QString &text);
    bool flush(bool force);

    const jp::grid::GridSheet *m_sheet;
    QChar m_sep;
    std::unique_ptr<QSaveFile> m_file;
    QByteArray m_buffer;
    qint64 m_lastRow = -1, m_lastCol = -1, m_row = 0;
    QString m_error;
    bool m_begun = false;
};

} // namespace jp::crunch
