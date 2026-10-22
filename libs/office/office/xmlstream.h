#pragma once
// What a reader of XML that is read as a stream (QXmlStreamReader on a device, never a tree) needs from the bytes it is given: the same refusal xml.h
// makes of an element with more than kMaxAttributesPerElement attributes, which a parser takes time in proportion to the square of, made on the way
// in, so that no more of such an element than that reaches the parser.

#include <QIODevice>

namespace jp::office {

// Passes the bytes of `source` on, and counts them. An element with more attributes (or namespace declarations) than kMaxAttributesPerElement
// ends what it passes at the "=" that is one too many, and the read after that fails: crowded() says so. Comments, CDATA, processing instructions, and
// declarations are skipped as xml.h skips them, so that what looks like an attribute in one is not counted; the quotes of a value are followed. A read
// that fails in the source fails here too, and sourceFailed() says so.
class XmlGuardDevice : public QIODevice {
public:
    // The source is not owned, and must be open for reading. This device is opened for reading.
    explicit XmlGuardDevice(QIODevice *source);
    bool isSequential() const override { return true; }
    bool crowded() const { return m_crowded; }
    bool sourceFailed() const { return m_sourceFailed; }
    // The bytes passed on.
    qint64 bytesPassed() const { return m_passed; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    enum State : quint8 { Text, Lt, Tag, Quote, Bang, Dash, Comment, CData, Pi, Decl };
    QIODevice *m_source;
    qint64 m_passed = 0;
    State m_state = Text;
    int m_count = 0;           // "=" seen in this tag
    char m_quote = 0;
    quint8 m_tail = 0;         // how many of the closing characters of a comment, CDATA, or processing instruction have come
    bool m_crowded = false, m_sourceFailed = false;
};

} // namespace jp::office
