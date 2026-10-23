#pragma once
// What a reader of XML that is read as a stream (QXmlStreamReader on a device, never a tree) needs from the bytes it is given, so that no file can make the
// parser take time or memory in proportion to more than its own size: the same refusal xml.h makes of an element with more than kMaxAttributesPerElement
// attributes (a parser takes time in proportion to their square), the refusal of what is not UTF-8 (the guard reads bytes, and a parser would read UTF-16 as
// other bytes than the guard does), and a bound on each piece of XML the parser is given at one time, which it holds whole.

#include <QIODevice>

namespace jp::office {

// Passes the bytes of `source` on, and counts them. It is a filter for the XML a spreadsheet's parts are, which is UTF-8 as all of them are in practice,
// and well formed; each of these ends what it passes, at the byte before the one that is wrong, and fails the read after that, saying why:
//  - a part that is not UTF-8: a byte order mark of another form, a zero byte, or a declaration naming another encoding (decided from the first 1,024 bytes,
//    before anything is passed on);
//  - an element with more attributes (or namespace declarations) than kMaxAttributesPerElement: crowded();
//  - a piece of more than `maxToken` bytes (a text, a tag with all its attributes, a comment, a CDATA section, a processing instruction): tooLong();
//  - a DOCTYPE or any other declaration, which is how an entity is defined: doctype();
//  - XML that is not well formed in a way the parser might read otherwise than the guard does, and so count the attributes of a tag where the guard
//    sees none: a "<" in a tag or a value, a comment that has "--" inside it or begins "<!-->" or "<!--->", a "<![" that isn't CDATA, a "<!" or "<?" with
//    nothing after it: malformed().
// A read that fails in the source fails here too, and sourceFailed() says so.
class XmlGuardDevice : public QIODevice {
public:
    // The source is not owned, and must be open for reading. This device is opened for reading.
    explicit XmlGuardDevice(QIODevice *source, qint64 maxToken = 1 << 20);
    bool isSequential() const override { return true; }
    bool crowded() const { return m_reason == Crowded; }
    bool notUtf8() const { return m_reason == NotUtf8; }
    bool tooLong() const { return m_reason == TooLong; }
    bool doctype() const { return m_reason == Doctype; }
    bool malformed() const { return m_reason == Malformed; }
    bool sourceFailed() const { return m_sourceFailed; }
    bool refused() const { return m_reason != None || m_sourceFailed; }
    // The bytes passed on.
    qint64 bytesPassed() const { return m_passed; }
    // The most bytes in one piece passed on, and the most asked for in one read.
    qint64 largestToken() const { return m_largestToken; }
    qint64 largestRead() const { return m_largestRead; }

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    enum State : quint8 { Text, Lt, Tag, Quote, Bang, Dash, CommentOpen, CommentOpen2, Comment, CDataOpen, CData, PiStart, Pi };
    enum Reason : quint8 { None, Crowded, NotUtf8, TooLong, Doctype, Malformed };
    bool begin();
    // Looks at bytes as they pass: the index of the first that is wrong, or -1; sets m_reason.
    qint64 scan(const char *data, qint64 size);

    QIODevice *m_source;
    qint64 m_maxToken;
    qint64 m_passed = 0, m_tokenBytes = 0, m_largestToken = 0, m_largestRead = 0;
    QByteArray m_head;         // the first bytes, held while the encoding is decided
    qint64 m_headPos = 0;
    bool m_begun = false;
    State m_state = Text;
    Reason m_reason = None;
    int m_count = 0;           // "=" seen in this tag
    char m_quote = 0;
    quint8 m_tail = 0;         // how far into the closing characters of a comment, CDATA, or processing instruction
    quint8 m_matched = 0;      // how many characters of "CDATA[" have come
    bool m_sourceFailed = false;
};

// Whether XML held whole passes the guard from end to end, as the parts of a spreadsheet's package that are read whole (its content types and relationships)
// are asked to when the package is lazy.
bool xmlPassesGuard(const QByteArray &xml, qint64 maxToken = 1 << 20);

} // namespace jp::office
