#include "office/xmlstream.h"

#include "office/xml.h"

#include <QBuffer>

#include <algorithm>
#include <cstring>

namespace jp::office {

namespace {

bool isNameStart(unsigned char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == ':' || c >= 0x80; }
bool isLetter(unsigned char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

} // namespace

XmlGuardDevice::XmlGuardDevice(QIODevice *source, qint64 maxToken) : m_source(source), m_maxToken(maxToken)
{
    open(QIODevice::ReadOnly);
}

// The first bytes decide what the rest is: UTF-8 is read as it is, and anything a parser might read as another encoding (a byte order mark, the shape
// of a "<" in UTF-16 or UTF-32, a declaration naming one) is refused before a byte is passed on.
bool XmlGuardDevice::begin()
{
    m_begun = true;
    while (m_head.size() < 1024) {
        QByteArray chunk(int(1024 - m_head.size()), Qt::Uninitialized);
        const qint64 got = m_source->read(chunk.data(), chunk.size());
        if (got < 0) {
            m_sourceFailed = true;
            return false;
        }
        if (got == 0) break;
        m_head.append(chunk.constData(), int(got));
    }
    const QByteArray &h = m_head;
    const auto starts = [&](const char *prefix, int n) { return h.size() >= n && std::memcmp(h.constData(), prefix, size_t(n)) == 0; };
    int at = 0;
    if (starts("\xef\xbb\xbf", 3)) at = 3;
    // Another encoding's byte order mark, or a zero byte (which UTF-8 never has) where the first characters are.
    if (starts("\xff\xfe", 2) || starts("\xfe\xff", 2) || h.left(4).contains('\0')) {
        m_reason = NotUtf8;
        return false;
    }
    // A declaration at the start says what the encoding is: UTF-8 or nothing is read.
    if (h.mid(at, 5) == "<?xml" && h.size() > at + 5 && (h[at + 5] == ' ' || h[at + 5] == '\t' || h[at + 5] == '\r' || h[at + 5] == '\n')) {
        const int end = h.indexOf("?>", at);
        if (end < 0) {
            m_reason = NotUtf8;
            return false;
        }
        const QByteArray declaration = h.mid(at, end - at);
        int e = declaration.indexOf("encoding");
        if (e >= 0) {
            e += 8;
            while (e < declaration.size() && (declaration[e] == ' ' || declaration[e] == '\t' || declaration[e] == '\r' || declaration[e] == '\n')) ++e;
            if (e < declaration.size() && declaration[e] == '=') {
                ++e;
                while (e < declaration.size() && (declaration[e] == ' ' || declaration[e] == '\t' || declaration[e] == '\r' || declaration[e] == '\n')) ++e;
                if (e < declaration.size() && (declaration[e] == '"' || declaration[e] == '\'')) {
                    const char quote = declaration[e];
                    const int close = declaration.indexOf(quote, e + 1);
                    const QByteArray name = close < 0 ? QByteArray() : declaration.mid(e + 1, close - e - 1).toLower();
                    if (name != "utf-8") {
                        m_reason = NotUtf8;
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

qint64 XmlGuardDevice::scan(const char *data, qint64 size)
{
    // Each token is counted from its first byte to its last, and ends at the bound.
    for (qint64 i = 0; i < size; ++i) {
        const unsigned char c = static_cast<unsigned char>(data[i]);
        if (c == 0) {
            m_reason = NotUtf8;
            return i;
        }
        if (++m_tokenBytes > m_maxToken) {
            m_reason = TooLong;
            return i;
        }
        auto endToken = [&] {
            m_largestToken = std::max(m_largestToken, m_tokenBytes);
            m_tokenBytes = 0;
            m_state = Text;
        };
        auto refuse = [&](Reason why) {
            m_reason = why;
            return i;
        };
        switch (m_state) {
        case Text:
            if (c == '<') {
                // the text before it is a token of its own
                m_largestToken = std::max(m_largestToken, m_tokenBytes - 1);
                m_tokenBytes = 1;
                m_count = 0;
                m_state = Lt;
            }
            break;
        case Lt:
            if (c == '!') m_state = Bang;
            else if (c == '?') m_state = PiStart;
            else if (c == '/' || isNameStart(c)) m_state = Tag;
            else return refuse(Malformed);
            break;
        case Tag:
            if (c == '>') {
                endToken();
            } else if (c == '"' || c == '\'') {
                m_quote = char(c);
                m_state = Quote;
            } else if (c == '=') {
                // This "=" and what follows are not passed on.
                if (++m_count > kMaxAttributesPerElement) return refuse(Crowded);
            } else if (c == '<') {
                return refuse(Malformed);
            }
            break;
        case Quote:
            if (c == static_cast<unsigned char>(m_quote)) m_state = Tag;
            else if (c == '<') return refuse(Malformed);
            break;
        case Bang:
            if (c == '-') m_state = Dash;
            else if (c == '[') { m_state = CDataOpen; m_matched = 0; }
            else if (isLetter(c)) return refuse(Doctype);
            else return refuse(Malformed);
            break;
        case Dash:
            if (c != '-') return refuse(Malformed);
            m_state = CommentOpen;
            break;
        case CommentOpen:
            // "<!-->" and "<!--->" are what some parsers take for a whole comment and others for the start of one.
            if (c == '>') return refuse(Malformed);
            m_tail = 0;
            if (c == '-') m_state = CommentOpen2;
            else m_state = Comment;
            break;
        case CommentOpen2:
            if (c == '>') return refuse(Malformed);
            if (c == '-') m_tail = 2;
            else m_tail = 0;
            m_state = Comment;
            break;
        case Comment:
            // Inside a comment "--" is only the start of its end, "-->".
            if (c == '-') {
                if (m_tail >= 2) return refuse(Malformed);
                ++m_tail;
            } else if (c == '>' && m_tail == 2) {
                endToken();
            } else if (m_tail == 2) {
                return refuse(Malformed);
            } else {
                m_tail = 0;
            }
            break;
        case CDataOpen: {
            static const char open[] = "CDATA[";
            if (c != static_cast<unsigned char>(open[m_matched])) return refuse(Malformed);
            if (++m_matched == 6) {
                m_state = CData;
                m_tail = 0;
            }
            break;
        }
        case CData:
            // "]]>"
            if (c == '>' && m_tail >= 2) endToken();
            else m_tail = c == ']' ? std::min<quint8>(quint8(m_tail + 1), 2) : 0;
            break;
        case PiStart:
            if (!isNameStart(c)) return refuse(Malformed);
            m_state = Pi;
            m_tail = 0;
            break;
        case Pi:
            // "?>"
            if (c == '>' && m_tail == 1) endToken();
            else m_tail = c == '?' ? 1 : 0;
            break;
        }
    }
    return -1;
}

qint64 XmlGuardDevice::readData(char *data, qint64 maxlen)
{
    if (m_reason != None || m_sourceFailed) return -1;
    if (maxlen <= 0) return 0;
    m_largestRead = std::max(m_largestRead, maxlen);
    if (!m_begun && !begin()) return -1;
    qint64 got = 0;
    if (m_headPos < m_head.size()) {
        got = std::min<qint64>(maxlen, m_head.size() - m_headPos);
        std::memcpy(data, m_head.constData() + m_headPos, size_t(got));
        m_headPos += got;
    } else {
        got = m_source->read(data, maxlen);
        if (got < 0) {
            m_sourceFailed = true;
            return -1;
        }
    }
    const qint64 wrong = scan(data, got);
    if (wrong >= 0) {
        m_passed += wrong;
        return wrong > 0 ? wrong : -1;
    }
    m_passed += got;
    return got;
}

bool xmlPassesGuard(const QByteArray &xml, qint64 maxToken)
{
    QByteArray copy = xml;
    QBuffer source(&copy);
    source.open(QIODevice::ReadOnly);
    XmlGuardDevice guard(&source, maxToken);
    char chunk[65536];
    while (guard.read(chunk, sizeof(chunk)) > 0) {
    }
    return !guard.refused() && guard.bytesPassed() == xml.size();
}

} // namespace jp::office
