#include "core/csvexport.h"

#include "sheet/xlscodepage.h"

#include <QCoreApplication>

namespace jp::crunch {

using jp::office::sheet::Cell;
using jp::office::sheet::CellType;

bool csvCanWriteCodePage(int codePage)
{
    QByteArray ignored;
    return jp::office::sheet::xlsTextToBytes(codePage, QString(), &ignored);
}

CsvExport::CsvExport(const jp::grid::GridSheet *sheet, QChar separator) : CsvExport(sheet, [&] {
    CsvWrite how;
    how.separator = separator;
    return how;
}())
{
}

CsvExport::CsvExport(const jp::grid::GridSheet *sheet, const CsvWrite &how) : m_sheet(sheet), m_how(how)
{
    // (a page this cannot write is written as UTF-8 with the mark)
    if (m_how.encoding == CsvWrite::Encoding::CodePage && !csvCanWriteCodePage(m_how.codePage)) m_how.encoding = CsvWrite::Encoding::Utf8Mark;
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
    if (m_how.encoding == CsvWrite::Encoding::Utf8Mark) m_buffer = QByteArray("\xEF\xBB\xBF", 3);
    else if (m_how.encoding == CsvWrite::Encoding::Utf16Le) m_buffer = QByteArray("\xFF\xFE", 2);
    m_begun = true;
    return true;
}

// The bytes of a piece of text in the encoding chosen.
QByteArray CsvExport::encoded(const QString &text)
{
    switch (m_how.encoding) {
    case CsvWrite::Encoding::Utf16Le: {
        QByteArray out(text.size() * 2, '\0');
        for (qsizetype i = 0; i < text.size(); ++i) {
            out[2 * i] = char(text[i].unicode() & 0xFF);
            out[2 * i + 1] = char(text[i].unicode() >> 8);
        }
        return out;
    }
    case CsvWrite::Encoding::CodePage: {
        QByteArray out;
        qsizetype lost = 0;
        jp::office::sheet::xlsTextToBytes(m_how.codePage, text, &out, &lost);
        m_replaced += lost;
        return out;
    }
    default: return text.toUtf8();
    }
}

void CsvExport::field(const QString &text)
{
    const bool quote = text.contains(m_how.separator) || text.contains(QLatin1Char('"')) || text.contains(QLatin1Char('\n')) || text.contains(QLatin1Char('\r'))
                       || text.startsWith(QLatin1Char(' ')) || text.endsWith(QLatin1Char(' '));
    if (!quote) {
        m_text += text;
        return;
    }
    QString q = text;
    q.replace(QLatin1Char('"'), QLatin1String("\"\""));
    m_text += QLatin1Char('"') + q + QLatin1Char('"');
}

bool CsvExport::flush(bool force)
{
    if (!m_text.isEmpty() && (force || m_text.size() >= (1 << 19))) {
        m_buffer += encoded(m_text);
        m_text.clear();
    }
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
    const QString sep(m_how.separator);
    qint64 looked = 0;
    while (s && m_row <= m_lastRow) {
        qint64 seps = 0;
        s->cells.forEachInRange(quint32(m_row), 0, quint32(m_row), quint32(m_lastCol), [&](quint32, quint32 col, const Cell &c) {
            for (; seps < qint64(col); ++seps) m_text += sep;
            if (c.type != CellType::Empty || c.hasFormula()) field(m_how.asStored ? m_sheet->storedTextOf(quint32(m_row), col, c) : m_sheet->textOf(quint32(m_row), col, c));
            ++looked;
            return true;
        });
        for (; seps < m_lastCol; ++seps) m_text += sep;
        m_text += m_how.lineEnd;
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
