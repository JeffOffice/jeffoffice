#include "core/csvexport.h"

#include <QCoreApplication>

namespace jp::crunch {

using jp::office::sheet::Cell;
using jp::office::sheet::CellType;

CsvExport::CsvExport(const jp::grid::GridSheet *sheet, QChar separator) : m_sheet(sheet), m_sep(separator)
{
    // the last row and column that hold something
    if (const jp::office::sheet::Sheet *s = sheet->data().sheet) {
        s->cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            if (c.type == CellType::Empty && !c.hasFormula()) return;
            m_lastRow = std::max<qint64>(m_lastRow, cellRow);
            m_lastCol = std::max<qint64>(m_lastCol, cellCol);
        });
    }
}

bool CsvExport::begin(const QString &path, QString *error)
{
    m_file = std::make_unique<QSaveFile>(path);
    if (!m_file->open(QIODevice::WriteOnly)) {
        if (error) *error = m_file->errorString();
        m_file.reset();
        return false;
    }
    m_buffer = QByteArray("\xEF\xBB\xBF", 3);
    m_begun = true;
    return true;
}

void CsvExport::field(const QString &text)
{
    const bool quote = text.contains(m_sep) || text.contains(QLatin1Char('"')) || text.contains(QLatin1Char('\n')) || text.contains(QLatin1Char('\r'));
    if (!quote) {
        m_buffer += text.toUtf8();
        return;
    }
    QString q = text;
    q.replace(QLatin1Char('"'), QLatin1String("\"\""));
    m_buffer += '"' + q.toUtf8() + '"';
}

bool CsvExport::flush(bool force)
{
    if (m_buffer.isEmpty() || (!force && m_buffer.size() < (1 << 20))) return true;
    if (m_file->write(m_buffer) != m_buffer.size()) {
        m_error = m_file->errorString();
        if (m_error.isEmpty()) m_error = QCoreApplication::translate("jp::crunch::CsvExport", "The file could not be written.");
        return false;
    }
    m_buffer.clear();
    return true;
}

bool CsvExport::step(qint64 budget)
{
    if (!m_begun || failed()) return true;
    const jp::office::sheet::Sheet *s = m_sheet->data().sheet;
    const QByteArray sep = QString(m_sep).toUtf8();
    qint64 looked = 0;
    while (s && m_row <= m_lastRow) {
        qint64 seps = 0;
        s->cells.forEachInRange(quint32(m_row), 0, quint32(m_row), quint32(m_lastCol), [&](quint32, quint32 col, const Cell &c) {
            for (; seps < qint64(col); ++seps) m_buffer += sep;
            if (c.type != CellType::Empty || c.hasFormula()) field(m_sheet->textOf(quint32(m_row), col, c));
            ++looked;
            return true;
        });
        for (; seps < m_lastCol; ++seps) m_buffer += sep;
        m_buffer += "\r\n";
        ++looked;
        ++m_row;
        if (!flush(false)) return true;
        if (budget > 0 && looked >= budget && m_row <= m_lastRow) return false;
    }
    flush(true);
    return true;
}

bool CsvExport::commit(QString *error)
{
    if (!m_file || failed()) {
        if (error) *error = m_error;
        return false;
    }
    if (!m_file->commit()) {
        if (error) *error = m_file->errorString();
        return false;
    }
    return true;
}

void CsvExport::cancel()
{
    if (m_file) m_file->cancelWriting();
    m_file.reset();
}

} // namespace jp::crunch
