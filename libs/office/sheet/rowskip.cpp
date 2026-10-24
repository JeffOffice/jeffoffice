#include "sheet/rowskip.h"

#include "sheet/xmlscan.h"

#include <QBuffer>

#include <algorithm>
#include <cstring>

namespace jp::office::sheet {

namespace {

constexpr qsizetype kChunk = 1 << 18;
// The head of the part (what comes before the cells) is never this long in a file that has one; past it the bytes are passed on as they are.
constexpr qsizetype kMaxHead = 8 << 20;
// A row that is longer than this is passed on as it is (it is not read to its end here).
constexpr qsizetype kMaxRowBytes = 64 << 20;

bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

} // namespace

RowSkipDevice::RowSkipDevice(QIODevice *in, const ReadOptions &options) : m_in(in), m_options(options)
{
    open(QIODevice::ReadOnly);
}

bool RowSkipDevice::fill()
{
    if (m_eof) return false;
    const qsizetype old = m_buf.size();
    m_buf.resize(old + kChunk);
    const qint64 n = m_in->read(m_buf.data() + old, kChunk);
    if (n <= 0) {
        m_buf.resize(old);
        m_eof = true;
        if (n < 0) {
            m_failed = true;
            setErrorString(m_in->errorString().isEmpty() ? QStringLiteral("The sheet's part could not be read.") : m_in->errorString());
        }
        return false;
    }
    m_buf.resize(old + qsizetype(n));
    return true;
}

void RowSkipDevice::passRest()
{
    m_out.append(m_buf.constData() + m_pos, m_buf.size() - m_pos);
    m_pos = m_buf.size();
    m_state = State::PassThrough;
}

// The part before the cells: found, and what the view in it leaves out is worked out.
bool RowSkipDevice::head()
{
    for (;;) {
        // not UTF-8 (a byte order mark of UTF-16, or its "<" in two bytes): everything is passed on
        if (m_buf.size() >= 2 && ((uchar(m_buf[0]) == 0xFF && uchar(m_buf[1]) == 0xFE) || (uchar(m_buf[0]) == 0xFE && uchar(m_buf[1]) == 0xFF) || (m_buf[0] == '<' && m_buf[1] == 0) || (m_buf[0] == 0 && m_buf[1] == '<'))) {
            passRest();
            return true;
        }
        // the name with a prefix (or anywhere but at the start of a tag) is not understood
        const qsizetype any = m_buf.indexOf("sheetData", m_pos);
        if (any >= 0 && (any == 0 || m_buf[any - 1] != '<')) {
            passRest();
            return true;
        }
        const qsizetype at = m_buf.indexOf("<sheetData", m_pos);
        if (at >= 0) {
            if (at + 11 > m_buf.size()) {
                if (!fill()) {
                    passRest();
                    return true;
                }
                continue;
            }
            const char c = m_buf[at + 10];
            if (!(isSpace(c) || c == '>' || c == '/')) {
                passRest();
                return true;
            }
            const qsizetype end = xmlscan::tagEnd(m_buf, at);
            if (end < 0) {
                if (!fill()) {
                    passRest();
                    return true;
                }
                continue;
            }
            const bool selfClosing = m_buf[end - 2] == '/';
            // what the view in the head leaves out, from the head read as a sheet of no rows
            QByteArray headBytes = m_buf.left(at) + "<sheetData/></worksheet>";
            Sheet tmp;
            ReadOptions mini;
            mini.limits = m_options.limits;
            QBuffer buffer(&headBytes);
            buffer.open(QIODevice::ReadOnly);
            readSheetPart(&buffer, mini, &tmp);
            const SheetView &v = tmp.view;
            m_skipTop = v.frozen ? qint64(v.topRow) : 0;
            m_frozenEnd = v.frozen ? m_skipTop + qint64(v.frozenRows) : 0;
            m_begin = std::max(m_frozenEnd, v.frozen ? qint64(v.paneRow) : qint64(v.topRow));
            m_zeroHeight = tmp.format.zeroHeight;
            if (selfClosing || (m_skipTop == 0 && m_begin <= m_frozenEnd) || !tmp.error.isEmpty()) {
                passRest();
                return true;
            }
            // the head, and the opening of the cells, as they are
            m_out.append(m_buf.constData(), end);
            m_pos = end;
            m_state = State::Rows;
            m_skippingFound = true;
            return true;
        }
        if (m_buf.size() - m_pos > kMaxHead) {
            passRest();
            return true;
        }
        // (the end of the data may cut the name in two)
        m_pos = std::max<qsizetype>(m_pos, m_buf.size() - 10);
        if (!fill()) {
            m_pos = 0;
            passRest();
            return true;
        }
    }
}

// The next row: given as it is, cut down, or not at all. False when there is nothing more to give.
bool RowSkipDevice::nextRow()
{
    for (;;) {
        // (what has been looked at is let go of now and then, here, where no place in the bytes is held)
        if (m_pos > kChunk * 4) {
            m_buf.remove(0, m_pos);
            m_pos = 0;
        }
        // white space between the rows goes
        while (m_pos < m_buf.size() && isSpace(m_buf[m_pos])) ++m_pos;
        if (m_pos >= m_buf.size()) {
            if (!fill()) return false;
            continue;
        }
        if (m_buf[m_pos] != '<') {
            passRest();   // (text between rows: not the form looked for)
            return true;
        }
        const qsizetype tagEnd = xmlscan::tagEnd(m_buf, m_pos);
        if (tagEnd < 0) {
            if (!fill()) {
                passRest();
                return true;
            }
            continue;
        }
        // (a copy: the bytes move when more are read)
        const QByteArray tagBytes = m_buf.mid(m_pos, tagEnd - m_pos);
        const QByteArrayView tag(tagBytes);
        if (tag.startsWith("</sheetData")) {
            passRest();
            return true;
        }
        const bool isRow = tag.startsWith("<row") && tag.size() > 4 && (isSpace(tag[4]) || tag[4] == '>' || tag[4] == '/');
        if (!isRow) {
            passRest();   // (not a row: not the form looked for)
            return true;
        }
        const bool selfClosing = tag[tag.size() - 2] == '/';
        // the row's number
        quint32 rowIndex = m_nextRow;
        bool haveR = false;
        const QByteArrayView r = xmlscan::attribute(tag, "r", &haveR);
        if (haveR) {
            bool ok = false;
            const qulonglong v = QByteArray(r).toULongLong(&ok);
            if (!ok || v < 1 || v > kMaxRows) {
                passRest();
                return true;
            }
            rowIndex = quint32(v - 1);
        }
        m_nextRow = rowIndex + 1;
        // its end
        qsizetype rowEnd = tagEnd;
        if (!selfClosing) {
            qsizetype close = -1;
            qsizetype from = tagEnd;
            for (;;) {
                close = m_buf.indexOf("</row>", from);
                if (close >= 0) break;
                from = std::max(from, qsizetype(m_buf.size()) - 5);
                if (m_buf.size() - m_pos > kMaxRowBytes || !fill()) {
                    passRest();   // (a row too long, or never closed: left to the parser)
                    return true;
                }
            }
            // (a comment or a CDATA section before the "</row>" that was found may hold it: not a tag, and the plain search can't say where the row ends: left to the parser)
            if (QByteArrayView(m_buf).sliced(tagEnd, close - tagEnd).indexOf("<!") >= 0) {
                passRest();
                return true;
            }
            rowEnd = close + 6;
        }
        const qint64 row = rowIndex;
        const bool skip = row < m_skipTop || (row >= m_frozenEnd && row < m_begin);
        if (!skip) {
            // given as it is (with its number, if it had none)
            if (!haveR) {
                m_out.append("<row r=\"" + QByteArray::number(qulonglong(rowIndex) + 1) + "\"");
                m_out.append(m_buf.constData() + m_pos + 4, rowEnd - (m_pos + 4));
            } else {
                m_out.append(m_buf.constData() + m_pos, rowEnd - m_pos);
            }
            ++m_kept;
            m_pos = rowEnd;
            return true;
        }
        // cut down: what the parser keeps of a row whose cells it does not read
        ++m_left;
        m_bytesLeft += rowEnd - m_pos;
        QByteArray cut = "<row r=\"" + QByteArray::number(qulonglong(rowIndex) + 1) + "\"";
        bool anything = m_zeroHeight;
        for (const char *name : {"ht", "hidden", "customHeight", "customFormat", "s"}) {
            bool found = false;
            const QByteArrayView value = xmlscan::attribute(tagBytes, name, &found);
            if (found) {
                cut += QByteArray(" ") + name + "=\"" + QByteArray(value) + "\"";
                if (name[0] != 's' || name[1] != '\0') anything = true;
            }
        }
        // the cells that hold the text of a shared formula (the cells that share it, below, name them)
        QByteArray masters;
        if (!selfClosing && QByteArrayView(m_buf).sliced(tagEnd, rowEnd - tagEnd).indexOf("t=\"shared\"") >= 0) {
            qsizetype at = tagEnd;
            for (;;) {
                const qsizetype f = m_buf.indexOf("<f ", at);
                if (f < 0 || f >= rowEnd) break;
                const qsizetype fEnd = xmlscan::tagEnd(m_buf, f);
                if (fEnd < 0 || fEnd > rowEnd) break;
                at = fEnd;
                const QByteArrayView fTag = QByteArrayView(m_buf).sliced(f, fEnd - f);
                bool found = false;
                const QByteArrayView type = xmlscan::attribute(fTag, "t", &found);
                if (!found || type != "shared" || fTag[fTag.size() - 2] == '/') continue;
                if (fEnd < m_buf.size() && m_buf[fEnd] == '<') continue;   // (no text: a cell that shares)
                const qsizetype cellStart = m_buf.lastIndexOf("<c", f);
                const qsizetype cellEnd = m_buf.indexOf("</c>", fEnd);
                if (cellStart < tagEnd || cellEnd < 0 || cellEnd >= rowEnd) continue;
                masters.append(m_buf.constData() + cellStart, cellEnd + 4 - cellStart);
            }
        }
        if (!masters.isEmpty()) {
            m_out.append(cut + ">" + masters + "</row>");
        } else if (anything) {
            m_out.append(cut + "/>");
        }
        m_pos = rowEnd;
        return true;
    }
}

bool RowSkipDevice::produce()
{
    switch (m_state) {
    case State::Head: return head();
    case State::Rows: return nextRow();
    case State::PassThrough:
        if (m_pos < m_buf.size()) {
            m_out.append(m_buf.constData() + m_pos, m_buf.size() - m_pos);
            m_pos = m_buf.size();
            return true;
        }
        if (!fill()) return false;
        return true;
    case State::Done: return false;
    }
    return false;
}

qint64 RowSkipDevice::readData(char *data, qint64 maxlen)
{
    while (m_outPos >= m_out.size()) {
        m_out.clear();
        m_outPos = 0;
        if (!produce() && m_out.isEmpty()) return m_failed ? -1 : 0;
    }
    const qint64 n = std::min<qint64>(maxlen, m_out.size() - m_outPos);
    std::memcpy(data, m_out.constData() + m_outPos, size_t(n));
    m_outPos += qsizetype(n);
    return n;
}

} // namespace jp::office::sheet
