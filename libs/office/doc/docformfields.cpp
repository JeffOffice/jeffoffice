// The legacy form fields of an old binary document (a text field, a check box, a drop-down list): their data is kept in the data stream, in the place the field's mark names.

#include "doc/docconvert.h"

#include <QCoreApplication>
#include <algorithm>

namespace jp::office::doc {

namespace {

// Reads the strings of the form field's data one after another, never past it.
class Reader {
public:
    explicit Reader(const Span &s, qint64 at) : m_s(s), m_at(at) {}
    bool ok() const { return m_ok; }
    quint16 u16()
    {
        const quint16 v = m_s.u16(m_at);
        if (!m_s.fits(m_at, 2)) m_ok = false;
        m_at += 2;
        return v;
    }
    // A string with its length first, and a null after it (section 2.9.354), of at most `most` characters.
    QString xstz(int most)
    {
        const int cch = u16();
        if (!m_ok || cch > most || !m_s.fits(m_at, 2 * qint64(cch) + 2)) {
            m_ok = false;
            return QString();
        }
        QString t;
        for (int i = 0; i < cch; ++i) t += QChar(m_s.u16(m_at + 2 * i));
        m_at += 2 * qint64(cch) + 2;
        return t;
    }
    // The list of a drop-down (an STTB of strings with no extra data), at most `most` entries.
    QStringList list(int most)
    {
        QStringList out;
        if (u16() != 0xFFFF) {
            m_ok = false;
            return out;
        }
        const int count = u16();
        const int extra = u16();
        for (int i = 0; i < count && i < most && m_ok; ++i) {
            const int cch = u16();
            if (!m_ok || cch > 255 || !m_s.fits(m_at, 2 * qint64(cch) + extra)) {
                m_ok = false;
                break;
            }
            QString t;
            for (int k = 0; k < cch; ++k) t += QChar(m_s.u16(m_at + 2 * k));
            out << t;
            m_at += 2 * qint64(cch) + extra;
        }
        return out;
    }

private:
    Span m_s;
    qint64 m_at;
    bool m_ok = true;
};

QByteArray val(const char *tag, const QString &v) { return QByteArray("<w:") + tag + " w:val=\"" + xmlEscape(v) + "\"/>"; }

} // namespace

QByteArray Converter::formFieldXml(qint32 position)
{
    if (position < 0) return QByteArray();
    const auto known = m_formFieldAt.constFind(position);
    if (known != m_formFieldAt.constEnd()) return known.value();
    if (m_formFieldAt.size() >= kMaxFormFields) return QByteArray();
    QByteArray xml;
    // The header (section 2.9.158): the size of the whole, and where the field's data begins. (The place is widened first: it is a stranger's 32-bit number, and nothing added to it may wrap.)
    const qint64 at = position;
    const quint32 lcb = data.u32(at);
    const int header = data.u16(at + 4);
    if (data.fits(at, header) && header >= 6 && lcb >= quint32(header) && qint64(lcb) <= data.size() - at) {
        const Span b = data.sub(at + header, std::min<qint64>(qint64(lcb) - header, kMaxFormFieldBytes));
        // The data (section 2.9.78, 2.9.79): a version, the bits (the kind, the state, whether it is protected, the kind of text), the longest text, the size of a check box.
        const int bits = b.u16(4), type = bits & 3, state = (bits >> 2) & 0x1F, textType = (bits >> 11) & 7;
        const bool ownHelp = bits & 0x80, ownStatus = bits & 0x100, locked = bits & 0x200, autoSize = bits & 0x400;
        if (b.u32(0) == 0xFFFFFFFFu && type <= 2) {
            const int longest = b.u16(6), size = b.u16(8);
            Reader r(b, 10);
            const QString name = r.xstz(255);
            QString text, format;
            int wDef = 0;
            if (type == 0) text = r.xstz(255);
            else wDef = r.u16();
            format = r.xstz(255);
            const QString help = r.xstz(255), status = r.xstz(255);
            r.xstz(255);   // (the macros run on entering and leaving: not carried)
            r.xstz(255);
            QStringList entries;
            if (type == 2) entries = r.list(kMaxFormFieldEntries);
            if (r.ok()) {
                xml = "<w:ffData>";
                if (!name.isEmpty()) xml += val("name", name);
                xml += locked ? "<w:enabled w:val=\"0\"/>" : "<w:enabled/>";
                if (ownHelp && !help.isEmpty()) xml += "<w:helpText w:type=\"text\" w:val=\"" + xmlEscape(help) + "\"/>";
                if (ownStatus && !status.isEmpty()) xml += "<w:statusText w:type=\"text\" w:val=\"" + xmlEscape(status) + "\"/>";
                if (type == 1) {
                    xml += "<w:checkBox>";
                    xml += autoSize ? QByteArray("<w:sizeAuto/>") : "<w:size w:val=\"" + QByteArray::number(std::clamp(size, 2, 3168)) + "\"/>";
                    xml += "<w:default w:val=\"" + QByteArray(wDef == 1 ? "1" : "0") + "\"/>";
                    if (state <= 1) xml += "<w:checked w:val=\"" + QByteArray::number(state) + "\"/>";   // (25: the state is not said)
                    xml += "</w:checkBox>";
                } else if (type == 2) {
                    xml += "<w:ddList>";
                    if (state < entries.size()) xml += "<w:result w:val=\"" + QByteArray::number(state) + "\"/>";
                    if (wDef < entries.size()) xml += "<w:default w:val=\"" + QByteArray::number(wDef) + "\"/>";
                    for (const QString &e : std::as_const(entries)) xml += val("listEntry", e);
                    xml += "</w:ddList>";
                } else {
                    static const char *const kinds[] = {"regular", "number", "date", "currentDate", "currentTime", "calculated"};
                    xml += "<w:textInput>";
                    xml += QByteArray("<w:type w:val=\"") + kinds[std::min(textType, 5)] + "\"/>";
                    if (!text.isEmpty()) xml += val("default", text);
                    if (longest > 0) xml += "<w:maxLength w:val=\"" + QByteArray::number(longest) + "\"/>";
                    if (!format.isEmpty()) xml += val("format", format);
                    xml += "</w:textInput>";
                }
                xml += "</w:ffData>";
            }
        }
    }
    m_formFieldAt.insert(position, xml);
    return xml;
}

} // namespace jp::office::doc
