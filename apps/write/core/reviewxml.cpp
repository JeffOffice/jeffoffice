// Accepting and rejecting the changes inside a piece of XML that is kept as it was written (a text box's content): the same changes as in the text, done on the elements.

#include "core/reviewops.h"

#include "wml/xmltree.h"

#include <QSet>

using namespace jp::office::wml;

namespace jp::write {

namespace {

constexpr int kMostDepth = 64;

class Resolver {
public:
    Resolver(const XmlTree &t, bool accept) : m_t(t), m_accept(accept) {}

    QByteArray whole(int n) { return element(n, 0); }

private:
    bool is(int n, const char *name) const { return m_t.ns(n) == NsW && m_t.is(n, name); }

    static bool marked(const QByteArray &raw) { return xmlHasChanges(raw); }

    // The children of an element, as the rewriting gives them.
    QByteArray children(int n, int depth)
    {
        QByteArray out;
        QByteArray carried;   // (what a paragraph whose mark is gone brings to the one after it)
        for (int c = m_t.firstChild(n); c >= 0; c = m_t.next(c)) {
            if (is(c, "p") && !carried.isEmpty()) {
                out += paragraph(c, depth + 1, &carried);
                continue;
            }
            out += element(c, depth + 1, &carried);
        }
        out += carried;
        return out;
    }

    // Whether the paragraph's mark goes: deleted and accepted, or inserted and rejected.
    bool markGoes(int p) const
    {
        const int pr = m_t.child(p, "pPr");
        if (pr < 0) return false;
        const int rpr = m_t.child(pr, "rPr");
        if (rpr < 0) return false;
        const bool deleted = m_t.child(rpr, "del") >= 0 || m_t.child(rpr, "moveFrom") >= 0, inserted = m_t.child(rpr, "ins") >= 0 || m_t.child(rpr, "moveTo") >= 0;
        return (m_accept && deleted) || (!m_accept && inserted);
    }

    QByteArray paragraph(int p, int depth, QByteArray *carried)
    {
        const bool goes = markGoes(p);
        QByteArray body;
        for (int c = m_t.firstChild(p); c >= 0; c = m_t.next(c)) {
            if (is(c, "pPr")) continue;
            body += element(c, depth + 1, nullptr);
        }
        if (goes && carried) {
            // (Joined to the paragraph after it: its words go in front of that one's.)
            *carried += body;
            return QByteArray();
        }
        QByteArray out = m_t.startTag(p);
        const int pr = m_t.child(p, "pPr");
        if (pr >= 0) out += element(pr, depth + 1, nullptr);
        out += (carried ? *carried : QByteArray()) + body;
        if (carried) carried->clear();
        return out + m_t.endTag(p);
    }

    QByteArray element(int n, int depth, QByteArray *carried = nullptr)
    {
        const QByteArray raw = m_t.raw(n);
        if (depth > kMostDepth || !marked(raw)) return carriedFirst(raw, carried);
        const bool inserts = is(n, "ins") || is(n, "moveTo"), deletes = is(n, "del") || is(n, "moveFrom");
        if (inserts) return m_accept ? children(n, depth) : QByteArray();
        if (deletes) return m_accept ? QByteArray() : deletedWords(n, depth);
        if (is(n, "p")) return paragraph(n, depth, carried);
        if (is(n, "rPr") || is(n, "pPr")) return properties(n, depth);
        return m_t.startTag(n) + children(n, depth) + m_t.endTag(n);
    }

    QByteArray carriedFirst(const QByteArray &raw, QByteArray *) { return raw; }

    // What a rejected deletion has: its words as words, and the code of a field as code.
    QByteArray deletedWords(int n, int depth)
    {
        QByteArray inner = children(n, depth);
        inner.replace("<w:delText", "<w:t").replace("</w:delText>", "</w:t>").replace("<w:delInstrText", "<w:instrText").replace("</w:delInstrText>", "</w:instrText>");
        return inner;
    }

    // The properties of a run or a paragraph: the record of a change goes (accepted), or the old properties take the place of the new (rejected); the marks of an inserted or deleted paragraph mark go.
    QByteArray properties(int n, int depth)
    {
        const bool paragraphProps = is(n, "pPr");
        const int record = m_t.child(n, paragraphProps ? "pPrChange" : "rPrChange");
        QByteArray out = m_t.startTag(n);
        if (record >= 0 && !m_accept) {
            // The old ones, then what the record does not hold (a paragraph's mark and section).
            const int old = m_t.child(record, paragraphProps ? "pPr" : "rPr");
            if (old >= 0)
                for (int c = m_t.firstChild(old); c >= 0; c = m_t.next(c)) out += m_t.raw(c);
            if (paragraphProps)
                for (int c = m_t.firstChild(n); c >= 0; c = m_t.next(c))
                    if (is(c, "rPr") || is(c, "sectPr")) out += element(c, depth + 1);
        } else {
            for (int c = m_t.firstChild(n); c >= 0; c = m_t.next(c)) {
                if (is(c, paragraphProps ? "pPrChange" : "rPrChange")) continue;
                if (!paragraphProps && (is(c, "ins") || is(c, "del") || is(c, "moveFrom") || is(c, "moveTo"))) continue;   // (the marks of a paragraph mark)
                out += is(c, "rPr") ? element(c, depth + 1) : m_t.raw(c);
            }
        }
        return out + m_t.endTag(n);
    }

    const XmlTree &m_t;
    bool m_accept;
};

} // namespace

bool xmlHasChanges(const QByteArray &xml)
{
    // (An element named so: "<w:del" is also the start of w:delText, and "<w:ins" of w:instrText.)
    static const char *const names[] = {"<w:ins", "<w:del", "<w:moveFrom", "<w:moveTo"};
    for (const char *n : names) {
        const qsizetype len = qstrlen(n);
        for (qsizetype at = xml.indexOf(n); at >= 0; at = xml.indexOf(n, at + len)) {
            const char next = at + len < xml.size() ? xml.at(at + len) : '\0';
            if (next == ' ' || next == '>' || next == '/' || next == '\t' || next == '\n' || next == '\r') return true;
        }
    }
    return xml.contains("rPrChange") || xml.contains("pPrChange");
}

QByteArray resolveChangesInXml(const QByteArray &xml, bool accept)
{
    if (!xmlHasChanges(xml)) return xml;
    // (The piece is one element, with the namespaces it uses declared on it: it is read under a root that declares the usual ones.)
    XmlTree t;
    static const QByteArray decls = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
        "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" "
        "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
        "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" "
        "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" "
        "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
    const QByteArray wrapped = "<w:keep " + decls + ">" + xml + "</w:keep>";
    if (!t.parse(wrapped, nullptr)) return xml;
    const int first = t.firstChild(t.root());
    if (first < 0 || t.next(first) >= 0) return xml;
    Resolver r(t, accept);
    return r.whole(first);
}

} // namespace jp::write
