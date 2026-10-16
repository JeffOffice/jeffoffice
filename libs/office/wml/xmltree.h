#pragma once
// A light tree of one XML part, for the word processor's documents. It is made to read parts that can be large (a document of
// thousands of pages has millions of nodes) and to hand back any element exactly as it was written: every element knows its place
// in the bytes it was read from, so what a reader doesn't understand is kept as a slice of those bytes and written back unchanged.
//
// The tree holds the bytes and, for each element, its name, its namespace, its attributes' places, its children, and its place in
// the bytes; text is read from the bytes when asked for. A document of 2,000,000 elements takes about 150 MB, against the 2 GB
// that the same file takes as a DOM.
//
// What a part may hold is limited: more than kMaxTreeBytes, kMaxTreeNodes elements, or kMaxTreeAttributes attributes, more than
// kMaxAttributesPerElement attributes on one element, or nesting past kMaxXmlDepth (office/xml.h has the last two) is refused, as is a
// DOCTYPE (so no entity is ever defined or loaded) or XML that is not well formed. Nothing here recurses. Real files, measured in 1,795
// .docx files of public collections: word/document.xml is 55 KB in the middle, 4 MB for 99 of 100, and 20.6 MB at the most, with
// 736,000 elements and 840,000 attributes, and the eight largest are nested 25 deep at the most. Each limit is more than 10 times the
// most, and bounds the tree at about a gigabyte (64 bytes an element, 24 an attribute).

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

namespace jp::office::wml {

// The namespaces the tree knows by number; the transitional and the strict forms of each are one. Any other namespace has a number
// of 3 or more.
enum : int { NsNone = 0, NsW = 1, NsR = 2 };

// The limits of what a part may hold (see above).
constexpr qint64 kMaxTreeBytes = qint64(256) << 20;
constexpr qint64 kMaxTreeNodes = 8000000;
constexpr qint64 kMaxTreeAttributes = 10000000;

class XmlTree {
public:
    // False, with a message, when the part is refused. The tree is empty then.
    bool parse(const QByteArray &xml, QString *error = nullptr);

    int root() const { return m_root; }
    int count() const { return int(m_nodes.size()); }
    int parent(int n) const { return m_nodes[n].parent; }
    int firstChild(int n) const { return m_nodes[n].first; }
    int next(int n) const { return m_nodes[n].next; }

    // The element is `local` in the namespace (the word processing one by default).
    bool is(int n, const char *local, int ns = NsW) const;
    QByteArray localName(int n) const;
    int ns(int n) const { return m_nodes[n].ns; }
    // The first child element with that name, or -1.
    int child(int n, const char *local, int ns = NsW) const;

    // An attribute by its local name: in the word processing namespace (or none, as some tools write it) unless `ns` says another.
    bool has(int n, const char *local, int ns = NsW) const;
    QString attr(int n, const char *local, int ns = NsW) const;
    // w:val, the usual one.
    QString val(int n) const { return attr(n, "val"); }
    // Every attribute of an element that is not named in `known` (local names), as ` name="value"` text exactly as written, for
    // attributes a reader keeps without understanding, with the namespaces the element declares itself first (what its prefixes
    // may be bound by).
    QByteArray otherAttributes(int n, const QVector<const char *> &known) const;

    // The text of an element: its character data (entities decoded, CDATA included), the elements inside it flattened away.
    QString text(int n) const;

    // The element as written, from its start tag to its end tag, made whole: a prefix that an element outside the root declared is
    // declared again on it, so the slice reads the same wherever it is put under the root's declarations.
    QByteArray raw(int n) const;
    // Its start tag alone (so too made whole) and its end tag alone; the end tag is empty for an element written as `<x/>`.
    QByteArray startTag(int n) const;
    QByteArray endTag(int n) const;
    // Everything between the start tag and the end tag, as written.
    QByteArray inner(int n) const;
    bool isEmptyElement(int n) const { return m_nodes[n].innerBegin == m_nodes[n].end; }

    // The bytes it was read from.
    const QByteArray &source() const { return m_src; }

private:
    struct Attr {
        int ns = 0, name = 0;
        quint32 begin = 0, end = 0;     // the value, without its quotes
        quint32 nameBegin = 0, nameEnd = 0;   // the name as written
    };
    struct Decl {
        int prefix = 0, ns = 0;         // the prefix's name (0: the default namespace)
        quint32 begin = 0, end = 0;     // the URI as written
    };
    struct Node {
        int parent = -1, first = -1, last = -1, next = -1;
        int ns = 0, name = 0;
        quint32 attrBegin = 0, attrCount = 0;
        quint32 declBegin = 0, declCount = 0;
        quint32 begin = 0, end = 0;               // the whole element
        quint32 innerBegin = 0, innerEnd = 0;     // between its tags
        quint32 nameBegin = 0, nameEnd = 0;       // its name as written (with prefix)
    };

    void clear();
    int intern(const QByteArray &name);
    int internUri(const QByteArray &uri);
    QString decode(quint32 begin, quint32 end) const;
    QByteArray withDeclarations(int n, quint32 from, quint32 to) const;   // [from, to) of the source, the start tag at its front

    QByteArray m_src;
    int m_root = -1;
    QVector<Node> m_nodes;
    QVector<Attr> m_attrs;
    QVector<Decl> m_decls;
    QVector<QByteArray> m_names;
    QHash<QByteArray, int> m_nameIndex;
    QHash<QByteArray, int> m_uriIndex;
    int m_uriCount = 3;
};

// The text of an attribute or character data with its entities decoded (&lt; &gt; &amp; &quot; &apos; &#N; &#xN;).
QString decodeXmlText(const char *s, quint32 n);
// Text made fit to write into XML character data or an attribute value (& < > and, for an attribute, the quotes).
QByteArray escapeXml(const QString &text, bool attribute = false);

} // namespace jp::office::wml
