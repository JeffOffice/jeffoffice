#include "office/xmlstream.h"

#include "office/xml.h"

#include <algorithm>

namespace jp::office {

XmlGuardDevice::XmlGuardDevice(QIODevice *source) : m_source(source)
{
    open(QIODevice::ReadOnly);
}

qint64 XmlGuardDevice::readData(char *data, qint64 maxlen)
{
    if (m_crowded || m_sourceFailed) return -1;
    if (maxlen <= 0) return 0;
    const qint64 got = m_source->read(data, maxlen);
    if (got < 0) {
        m_sourceFailed = true;
        return -1;
    }
    for (qint64 i = 0; i < got; ++i) {
        const char c = data[i];
        switch (m_state) {
        case Text:
            if (c == '<') m_state = Lt;
            break;
        case Lt:
            m_count = 0;
            m_tail = 0;
            if (c == '!') m_state = Bang;
            else if (c == '?') m_state = Pi;
            else m_state = c == '>' ? Text : Tag;
            break;
        case Tag:
            if (c == '>') {
                m_state = Text;
            } else if (c == '"' || c == '\'') {
                m_quote = c;
                m_state = Quote;
            } else if (c == '=' && ++m_count > kMaxAttributesPerElement) {
                // This "=" and what follows are not passed on.
                m_crowded = true;
                m_passed += i;
                return i > 0 ? i : -1;
            }
            break;
        case Quote:
            if (c == m_quote) m_state = Tag;
            break;
        case Bang:
            m_tail = 0;
            if (c == '-') m_state = Dash;
            else if (c == '[') m_state = CData;
            else m_state = c == '>' ? Text : Decl;
            break;
        case Dash:
            if (c == '-') {
                m_state = Comment;
                m_tail = 0;
            } else {
                m_state = c == '>' ? Text : Decl;
            }
            break;
        case Comment:
            // "-->"
            if (c == '>' && m_tail >= 2) m_state = Text;
            m_tail = c == '-' ? std::min<quint8>(m_tail + 1, 2) : 0;
            break;
        case CData:
            // "]]>"
            if (c == '>' && m_tail >= 2) m_state = Text;
            m_tail = c == ']' ? std::min<quint8>(m_tail + 1, 2) : 0;
            break;
        case Pi:
            // "?>"
            if (c == '>' && m_tail == 1) m_state = Text;
            m_tail = c == '?' ? 1 : 0;
            break;
        case Decl:
            if (c == '>') m_state = Text;
            break;
        }
    }
    m_passed += got;
    return got;
}

} // namespace jp::office
