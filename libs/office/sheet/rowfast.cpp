#include "sheet/rowfast.h"

#include <algorithm>
#include <cstring>

namespace jp::office::sheet {

namespace {
constexpr qint64 kBlock = 16384;
constexpr qsizetype kMaxHeld = 64 << 20;   // a row of more bytes than this is the parser's
constexpr qsizetype kMaxTagBytes = 65536;
constexpr char kTag[] = "<sheetData";
constexpr qsizetype kTagSize = sizeof(kTag) - 1;
} // namespace

RowFeed::RowFeed(QIODevice *source) : m_src(source)
{
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
}

// More bytes from the source behind those held; the bytes given are let go of first, so that what is held does not grow with the part.
qint64 RowFeed::pull()
{
    if (m_pos > 0 && m_pos >= m_buf.size() / 2) {
        m_buf.remove(0, m_pos);
        if (m_pauseAt >= 0) m_pauseAt -= m_pos;
        m_searched = std::max<qsizetype>(0, m_searched - m_pos);
        m_pos = 0;
    }
    const qsizetype old = m_buf.size();
    m_buf.resize(old + kBlock);
    const qint64 got = m_src->read(m_buf.data() + old, kBlock);
    m_buf.resize(old + std::max<qint64>(0, got));
    if (got <= 0) m_eof = true;
    return got;
}

bool RowFeed::more()
{
    if (m_eof || m_buf.size() - m_pos > kMaxHeld) return false;
    return pull() > 0;
}

bool RowFeed::takeOver()
{
    if (m_state != State::Paused) return false;
    m_state = State::Scan;
    return true;
}

void RowFeed::giveBack() { m_state = State::Pass; }

void RowFeed::giveBackThenPause(qsizetype length)
{
    m_state = State::Serve;
    m_pauseAt = m_pos + length;
}

// Hands out held bytes up to `upTo`.
qint64 RowFeed::give(char *data, qint64 maxlen, qsizetype upTo)
{
    const qint64 n = std::min<qint64>(maxlen, upTo - m_pos);
    std::memcpy(data, m_buf.constData() + m_pos, size_t(n));
    m_pos += n;
    return n;
}

qint64 RowFeed::readData(char *data, qint64 maxlen)
{
    if (maxlen <= 0) return 0;
    if (m_state == State::Paused || m_state == State::Scan) m_state = State::Pass;   // (the parser wants more: it is not stopped where the reader wanted)
    if (m_state == State::Serve) {
        const qint64 n = give(data, maxlen, m_pauseAt);
        if (m_pos >= m_pauseAt) {
            m_state = State::Paused;
            m_pauseAt = -1;
        }
        return n;
    }
    if (m_state == State::Pass) {
        if (m_pos < m_buf.size()) return give(data, maxlen, m_buf.size());
        m_buf.clear();
        m_pos = 0;
        return m_src->read(data, maxlen);
    }
    // Watch: the bytes go to the parser as they come, but a start of `sheetData` is looked for in them, and the parser is stopped after it.
    for (;;) {
        if (m_pauseAt >= 0) {
            const qint64 n = give(data, maxlen, m_pauseAt);
            if (m_pos >= m_pauseAt) {
                m_state = State::Paused;
                m_pauseAt = -1;
            }
            return n;
        }
        const QByteArrayView held = QByteArrayView(m_buf).sliced(m_pos);
        const qsizetype from = std::max<qsizetype>(m_searched - m_pos, 0);
        const qsizetype at = held.indexOf(QByteArrayView(kTag, kTagSize), from);
        if (at < 0) {
            // (the last bytes may be the start of one that is cut short: they wait for more)
            const qsizetype keep = m_eof ? 0 : std::min<qsizetype>(held.size(), kTagSize - 1);
            const qsizetype free = held.size() - keep;
            if (free > 0) {
                m_searched = m_pos + free;
                return give(data, maxlen, m_pos + free);
            }
            if (held.isEmpty() && m_eof) return 0;
            if (pull() <= 0 && m_eof && held.isEmpty()) return 0;
            continue;
        }
        if (at + kTagSize >= held.size()) {
            // the end of the name has not come
            if (!m_eof && pull() > 0) continue;
            m_state = State::Pass;
            return readData(data, maxlen);
        }
        const char after = held[at + kTagSize];
        if (after != '>' && after != ' ' && after != '/') {
            m_searched = m_pos + at + kTagSize;   // (some other element, as `sheetDataX`)
            continue;
        }
        qsizetype end = -1;
        bool plain = true;
        for (qsizetype i = at + kTagSize; i < held.size(); ++i) {
            const char c = held[i];
            if (c == '>') {
                end = i;
                break;
            }
            if (c == '"' || c == '\'' || c == '<') {
                plain = false;
                break;
            }
        }
        if (end < 0 && plain) {
            // the end of the tag has not come
            if (!m_eof && held.size() - at < kMaxTagBytes && pull() > 0) continue;
            plain = false;
        }
        if (!plain || held[end - 1] == '/') {
            m_state = State::Pass;   // (a tag that closes itself has no rows; one with quotes is not looked into)
            return readData(data, maxlen);
        }
        m_pauseAt = m_pos + end + 1;
    }
}

} // namespace jp::office::sheet
