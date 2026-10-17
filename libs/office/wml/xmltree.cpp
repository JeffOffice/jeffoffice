#include "wml/xmltree.h"

#include "office/xml.h"

#include <QSet>
#include <QStringConverter>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace jp::office::wml {

namespace {

constexpr const char *kWTransitional = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
constexpr const char *kWStrict = "http://purl.oclc.org/ooxml/wordprocessingml/main";
constexpr const char *kRTransitional = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
constexpr const char *kRStrict = "http://purl.oclc.org/ooxml/officeDocument/relationships";
constexpr const char *kXmlNs = "http://www.w3.org/XML/1998/namespace";

inline bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }

// Where `pat` first occurs in the n bytes at s, or null.
const char *findBytes(const char *s, quint32 n, const char *pat)
{
    const quint32 m = quint32(strlen(pat));
    if (n < m) return nullptr;
    const char *end = s + n;
    const char *at = std::search(s, end, pat, pat + m);
    return at == end ? nullptr : at;
}

// Whether the '&' at s[i] begins one of the entities XML has without a DTD, or a character reference.
bool entityOk(const char *s, quint32 i, quint32 n)
{
    quint32 j = i + 1;
    const quint32 stop = std::min<quint32>(n, i + 12);
    while (j < stop && s[j] != ';') ++j;
    if (j >= stop || s[j] != ';') return false;
    const QByteArrayView body(s + i + 1, j - i - 1);
    if (body == "amp" || body == "lt" || body == "gt" || body == "quot" || body == "apos") return true;
    if (body.size() < 2 || body[0] != '#') return false;
    if (body[1] == 'x') {
        if (body.size() < 3) return false;
        for (qsizetype k = 2; k < body.size(); ++k)
            if (!std::isxdigit(static_cast<unsigned char>(body[k]))) return false;
        return true;
    }
    for (qsizetype k = 1; k < body.size(); ++k)
        if (body[k] < '0' || body[k] > '9') return false;
    return true;
}

} // namespace

QString decodeXmlText(const char *s, quint32 n)
{
    const char *amp = static_cast<const char *>(memchr(s, '&', n));
    if (!amp) return QString::fromUtf8(s, qsizetype(n));
    QString out;
    quint32 i = 0;
    while (i < n) {
        const char *next = static_cast<const char *>(memchr(s + i, '&', n - i));
        if (!next) {
            out += QString::fromUtf8(s + i, qsizetype(n - i));
            break;
        }
        const quint32 at = quint32(next - s);
        out += QString::fromUtf8(s + i, qsizetype(at - i));
        quint32 j = at + 1;
        while (j < n && s[j] != ';' && j - at < 12) ++j;
        if (j >= n || s[j] != ';') {   // (not an entity: kept as it is)
            out += QLatin1Char('&');
            i = at + 1;
            continue;
        }
        const QByteArrayView body(s + at + 1, j - at - 1);
        if (body == "amp") out += QLatin1Char('&');
        else if (body == "lt") out += QLatin1Char('<');
        else if (body == "gt") out += QLatin1Char('>');
        else if (body == "quot") out += QLatin1Char('"');
        else if (body == "apos") out += QLatin1Char('\'');
        else if (body.size() >= 2 && body[0] == '#') {
            bool ok = false;
            const uint cp = body[1] == 'x' ? QByteArray(body.mid(2)).toUInt(&ok, 16) : QByteArray(body.mid(1)).toUInt(&ok, 10);
            if (ok && cp > 0 && cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF)) out += QString::fromUcs4(reinterpret_cast<const char32_t *>(&cp), 1);
            else out += QChar(QChar::ReplacementCharacter);
        } else {
            out += QString::fromUtf8(s + at, qsizetype(j - at + 1));
        }
        i = j + 1;
    }
    return out;
}

QByteArray escapeXml(const QString &text, bool attribute)
{
    const QByteArray utf8 = text.toUtf8();
    QByteArray out;
    out.reserve(utf8.size() + utf8.size() / 8);
    for (const char c : utf8) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"':
            if (attribute) out += "&quot;";
            else out += c;
            break;
        default: out += c;
        }
    }
    return out;
}

void XmlTree::clear()
{
    m_src.clear();
    m_root = -1;
    m_nodes.clear();
    m_attrs.clear();
    m_decls.clear();
    m_names.clear();
    m_nameIndex.clear();
    m_uriIndex.clear();
    m_uriCount = 3;
    m_names << QByteArray();   // (name 0 is the empty one: no prefix)
    m_nameIndex.insert(QByteArray(), 0);
    m_uriIndex.insert(kWTransitional, NsW);
    m_uriIndex.insert(kWStrict, NsW);
    m_uriIndex.insert(kRTransitional, NsR);
    m_uriIndex.insert(kRStrict, NsR);
}

int XmlTree::intern(const QByteArray &name)
{
    auto it = m_nameIndex.constFind(name);
    if (it != m_nameIndex.constEnd()) return it.value();
    const int id = int(m_names.size());
    m_names << name;
    m_nameIndex.insert(name, id);
    return id;
}

int XmlTree::internUri(const QByteArray &uri)
{
    auto it = m_uriIndex.constFind(uri);
    if (it != m_uriIndex.constEnd()) return it.value();
    const int id = m_uriCount++;
    m_uriIndex.insert(uri, id);
    return id;
}

bool XmlTree::parse(const QByteArray &xmlIn, QString *error)
{
    clear();
    auto fail = [&](const QString &why, quint32 at) {
        if (error) *error = QStringLiteral("%1 (at byte %2)").arg(why).arg(at);
        clear();
        return false;
    };
    if (xmlIn.size() > kMaxTreeBytes) return fail(QStringLiteral("The XML part is too large"), 0);
    QByteArray xml = xmlIn;
    if (const auto enc = QStringConverter::encodingForData(xml); enc && *enc != QStringConverter::Utf8) {
        QStringDecoder dec(*enc);
        const QString text = dec(xml);
        if (dec.hasError()) return fail(QStringLiteral("The XML part is not in a text encoding that can be read"), 0);
        xml = text.toUtf8();
    }
    m_src = xml;
    const char *s = m_src.constData();
    const quint32 n = quint32(m_src.size());
    quint32 i = (n >= 3 && memcmp(s, "\xEF\xBB\xBF", 3) == 0) ? 3 : 0;

    struct Open { int node; qsizetype scope; };
    QVector<Open> stack;
    struct Scoped { int prefix, ns; };
    QVector<Scoped> scope;
    const int xmlPrefix = intern("xml");
    const int xmlNsId = internUri(kXmlNs);

    auto lookup = [&](int prefix, int *ns) {
        if (prefix == xmlPrefix) {
            *ns = xmlNsId;
            return true;
        }
        for (qsizetype k = scope.size() - 1; k >= 0; --k)
            if (scope[k].prefix == prefix) {
                *ns = scope[k].ns;
                return true;
            }
        if (prefix == 0) {
            *ns = NsNone;
            return true;
        }
        return false;
    };
    // A name's prefix and local part, interned.
    auto split = [&](quint32 b, quint32 e, int *prefix, int *local) {
        const char *colon = static_cast<const char *>(memchr(s + b, ':', e - b));
        if (!colon) {
            *prefix = 0;
            *local = intern(QByteArray(s + b, qsizetype(e - b)));
        } else {
            const quint32 c = quint32(colon - s);
            *prefix = intern(QByteArray(s + b, qsizetype(c - b)));
            *local = intern(QByteArray(s + c + 1, qsizetype(e - c - 1)));
        }
    };

    struct PendingAttr { quint32 nameBegin, nameEnd, valBegin, valEnd; };
    QVector<PendingAttr> pending;

    while (i < n) {
        if (s[i] != '<') {
            // Character data (only white space outside the root).
            quint32 j = i;
            const char *lt = static_cast<const char *>(memchr(s + i, '<', n - i));
            const quint32 stop = lt ? quint32(lt - s) : n;
            for (; j < stop; ++j) {
                if (s[j] == '&') {
                    if (stack.isEmpty() || !entityOk(s, j, n)) return fail(QStringLiteral("Text that is not XML"), j);
                } else if (stack.isEmpty() && !isSpace(s[j])) {
                    return fail(QStringLiteral("Text outside the root element"), j);
                }
            }
            i = stop;
            continue;
        }
        if (i + 1 >= n) return fail(QStringLiteral("The XML ends inside a tag"), i);
        const char c = s[i + 1];
        if (c == '?') {
            const char *end = findBytes(s + i + 2, n - i - 2, "?>");
            if (!end) return fail(QStringLiteral("A processing instruction is not closed"), i);
            i = quint32(end - s) + 2;
            continue;
        }
        if (c == '!') {
            if (n - i >= 4 && memcmp(s + i, "<!--", 4) == 0) {
                const char *end = findBytes(s + i + 4, n - i - 4, "-->");
                if (!end) return fail(QStringLiteral("A comment is not closed"), i);
                i = quint32(end - s) + 3;
                continue;
            }
            if (n - i >= 9 && memcmp(s + i, "<![CDATA[", 9) == 0 && !stack.isEmpty()) {
                const char *end = findBytes(s + i + 9, n - i - 9, "]]>");
                if (!end) return fail(QStringLiteral("A CDATA section is not closed"), i);
                i = quint32(end - s) + 3;
                continue;
            }
            return fail(QStringLiteral("A DOCTYPE or other declaration is not allowed"), i);
        }
        if (c == '/') {
            // An end tag.
            quint32 j = i + 2;
            const quint32 nameBegin = j;
            while (j < n && !isSpace(s[j]) && s[j] != '>') ++j;
            const quint32 nameEnd = j;
            while (j < n && isSpace(s[j])) ++j;
            if (j >= n || s[j] != '>') return fail(QStringLiteral("An end tag is not closed"), i);
            if (stack.isEmpty()) return fail(QStringLiteral("An end tag with no start tag"), i);
            Node &nd = m_nodes[stack.back().node];
            if (nd.nameEnd - nd.nameBegin != nameEnd - nameBegin || memcmp(s + nd.nameBegin, s + nameBegin, nameEnd - nameBegin) != 0)
                return fail(QStringLiteral("An end tag does not match its start tag"), i);
            nd.innerEnd = i;
            nd.end = j + 1;
            scope.resize(stack.back().scope);
            stack.removeLast();
            i = j + 1;
            continue;
        }
        // A start tag.
        if (stack.isEmpty() && m_root >= 0) return fail(QStringLiteral("More than one root element"), i);
        if (m_nodes.size() >= kMaxTreeNodes) return fail(QStringLiteral("The XML has too many elements"), i);
        if (stack.size() >= kMaxXmlDepth) return fail(QStringLiteral("The XML is nested too deeply"), i);
        quint32 j = i + 1;
        const quint32 nameBegin = j;
        while (j < n && !isSpace(s[j]) && s[j] != '>' && s[j] != '/') ++j;
        const quint32 nameEnd = j;
        if (nameEnd == nameBegin) return fail(QStringLiteral("A tag with no name"), i);
        pending.clear();
        bool selfClosing = false, closed = false;
        const qsizetype declStart = m_decls.size();
        while (j < n) {
            while (j < n && isSpace(s[j])) ++j;
            if (j >= n) break;
            if (s[j] == '>') {
                ++j;
                closed = true;
                break;
            }
            if (s[j] == '/') {
                if (j + 1 < n && s[j + 1] == '>') {
                    j += 2;
                    selfClosing = closed = true;
                    break;
                }
                return fail(QStringLiteral("A stray / in a tag"), j);
            }
            const quint32 an = j;
            while (j < n && !isSpace(s[j]) && s[j] != '=' && s[j] != '>' && s[j] != '/') ++j;
            const quint32 ae = j;
            if (ae == an) return fail(QStringLiteral("An attribute with no name"), j);
            while (j < n && isSpace(s[j])) ++j;
            if (j >= n || s[j] != '=') return fail(QStringLiteral("An attribute with no value"), j);
            ++j;
            while (j < n && isSpace(s[j])) ++j;
            if (j >= n || (s[j] != '"' && s[j] != '\'')) return fail(QStringLiteral("An attribute value with no quotes"), j);
            const char quote = s[j++];
            const quint32 vb = j;
            const char *q = static_cast<const char *>(memchr(s + j, quote, n - j));
            if (!q) return fail(QStringLiteral("An attribute value is not closed"), j);
            const quint32 ve = quint32(q - s);
            for (quint32 k = vb; k < ve; ++k) {
                if (s[k] == '<') return fail(QStringLiteral("A < in an attribute value"), k);
                if (s[k] == '&' && !entityOk(s, k, ve + 1)) return fail(QStringLiteral("An attribute value that is not XML"), k);
            }
            j = ve + 1;
            if (pending.size() >= kMaxAttributesPerElement || m_attrs.size() + pending.size() >= kMaxTreeAttributes) return fail(QStringLiteral("An element has too many attributes"), an);
            pending.push_back({an, ae, vb, ve});
        }
        if (!closed) return fail(QStringLiteral("A tag is not closed"), i);
        // Namespace declarations first, which the names of the same tag may use.
        const qsizetype scopeBefore = scope.size();
        for (const PendingAttr &a : std::as_const(pending)) {
            const quint32 len = a.nameEnd - a.nameBegin;
            int prefix = -1;
            if (len == 5 && memcmp(s + a.nameBegin, "xmlns", 5) == 0) prefix = 0;
            else if (len > 6 && memcmp(s + a.nameBegin, "xmlns:", 6) == 0) prefix = intern(QByteArray(s + a.nameBegin + 6, qsizetype(len - 6)));
            if (prefix < 0) continue;
            const int ns = internUri(QByteArray(s + a.valBegin, qsizetype(a.valEnd - a.valBegin)));
            scope.push_back({prefix, ns});
            m_decls.push_back({prefix, ns, a.valBegin, a.valEnd});
        }
        Node nd;
        nd.begin = i;
        nd.nameBegin = nameBegin;
        nd.nameEnd = nameEnd;
        int prefix = 0, local = 0;
        split(nameBegin, nameEnd, &prefix, &local);
        if (!lookup(prefix, &nd.ns)) return fail(QStringLiteral("An undeclared namespace prefix"), i);
        nd.name = local;
        nd.attrBegin = quint32(m_attrs.size());
        for (const PendingAttr &a : std::as_const(pending)) {
            const quint32 len = a.nameEnd - a.nameBegin;
            if ((len == 5 && memcmp(s + a.nameBegin, "xmlns", 5) == 0) || (len > 6 && memcmp(s + a.nameBegin, "xmlns:", 6) == 0)) continue;
            Attr at;
            int ap = 0, al = 0;
            split(a.nameBegin, a.nameEnd, &ap, &al);
            if (ap != 0 && !lookup(ap, &at.ns)) return fail(QStringLiteral("An undeclared namespace prefix"), a.nameBegin);
            if (ap == 0) at.ns = NsNone;
            at.name = al;
            at.begin = a.valBegin;
            at.end = a.valEnd;
            at.nameBegin = a.nameBegin;
            at.nameEnd = a.nameEnd;
            m_attrs.push_back(at);
        }
        nd.attrCount = quint32(m_attrs.size()) - nd.attrBegin;
        nd.declBegin = quint32(declStart);
        nd.declCount = quint32(m_decls.size() - declStart);
        nd.innerBegin = j;
        nd.parent = stack.isEmpty() ? -1 : stack.back().node;
        const int id = int(m_nodes.size());
        if (selfClosing) {
            nd.end = nd.innerEnd = j;
            scope.resize(scopeBefore);
        }
        m_nodes.push_back(nd);
        if (nd.parent >= 0) {
            Node &p = m_nodes[nd.parent];
            if (p.last >= 0) m_nodes[p.last].next = id;
            else p.first = id;
            p.last = id;
        } else {
            m_root = id;
        }
        if (!selfClosing) stack.push_back({id, scopeBefore});
        i = j;
    }
    if (!stack.isEmpty()) return fail(QStringLiteral("An element is not closed"), n);
    if (m_root < 0) return fail(QStringLiteral("The XML has no root element"), n);
    return true;
}

bool XmlTree::is(int n, const char *local, int ns) const
{
    const Node &nd = m_nodes[n];
    return nd.ns == ns && m_names[nd.name] == local;
}

QByteArray XmlTree::localName(int n) const { return m_names[m_nodes[n].name]; }

int XmlTree::child(int n, const char *local, int ns) const
{
    for (int c = m_nodes[n].first; c >= 0; c = m_nodes[c].next)
        if (is(c, local, ns)) return c;
    return -1;
}

bool XmlTree::has(int n, const char *local, int ns) const
{
    const Node &nd = m_nodes[n];
    for (quint32 k = 0; k < nd.attrCount; ++k) {
        const Attr &a = m_attrs[nd.attrBegin + k];
        if ((a.ns == ns || (ns == NsW && a.ns == NsNone)) && m_names[a.name] == local) return true;
    }
    return false;
}

QString XmlTree::attr(int n, const char *local, int ns) const
{
    const Node &nd = m_nodes[n];
    for (quint32 k = 0; k < nd.attrCount; ++k) {
        const Attr &a = m_attrs[nd.attrBegin + k];
        if ((a.ns == ns || (ns == NsW && a.ns == NsNone)) && m_names[a.name] == local) return decode(a.begin, a.end);
    }
    return QString();
}

QString XmlTree::attrAny(int n, const char *local) const
{
    const Node &nd = m_nodes[n];
    for (quint32 k = 0; k < nd.attrCount; ++k) {
        const Attr &a = m_attrs[nd.attrBegin + k];
        if (m_names[a.name] == local) return decode(a.begin, a.end);
    }
    return QString();
}

QByteArray XmlTree::otherAttributes(int n, const QVector<const char *> &known) const
{
    const Node &nd = m_nodes[n];
    QByteArray out;
    // (The namespaces the element declares itself go with its attributes: they may be what the attributes' prefixes are bound by.)
    for (quint32 k = 0; k < nd.declCount; ++k) {
        const Decl &d = m_decls[nd.declBegin + k];
        out += d.prefix == 0 ? QByteArray(" xmlns=\"") : " xmlns:" + m_names[d.prefix] + "=\"";
        out += m_src.mid(d.begin, d.end - d.begin) + '"';
    }
    for (quint32 k = 0; k < nd.attrCount; ++k) {
        const Attr &a = m_attrs[nd.attrBegin + k];
        bool isKnown = false;
        for (const char *name : known)
            if (m_names[a.name] == name) isKnown = true;
        if (isKnown) continue;
        QByteArray value = m_src.mid(a.begin, a.end - a.begin);
        value.replace('"', "&quot;");
        out += ' ' + m_src.mid(a.nameBegin, a.nameEnd - a.nameBegin) + "=\"" + value + '"';
    }
    return out;
}

QString XmlTree::decode(quint32 begin, quint32 end) const
{
    return decodeXmlText(m_src.constData() + begin, end - begin);
}

QString XmlTree::text(int n) const
{
    const Node &nd = m_nodes[n];
    const char *s = m_src.constData();
    quint32 i = nd.innerBegin;
    const quint32 stop = nd.innerEnd;
    if (i >= stop) return QString();
    if (!memchr(s + i, '<', stop - i)) return decode(i, stop);
    QString out;
    while (i < stop) {
        if (s[i] != '<') {
            const char *lt = static_cast<const char *>(memchr(s + i, '<', stop - i));
            const quint32 e = lt ? quint32(lt - s) : stop;
            out += decode(i, e);
            i = e;
            continue;
        }
        if (stop - i >= 9 && memcmp(s + i, "<![CDATA[", 9) == 0) {
            const char *end = findBytes(s + i + 9, stop - i - 9, "]]>");
            const quint32 e = end ? quint32(end - s) : stop;
            out += QString::fromUtf8(s + i + 9, qsizetype(e - i - 9));
            i = end ? e + 3 : stop;
        } else if (stop - i >= 4 && memcmp(s + i, "<!--", 4) == 0) {
            const char *end = findBytes(s + i + 4, stop - i - 4, "-->");
            i = end ? quint32(end - s) + 3 : stop;
        } else if (stop - i >= 2 && s[i + 1] == '?') {
            const char *end = findBytes(s + i + 2, stop - i - 2, "?>");
            i = end ? quint32(end - s) + 2 : stop;
        } else {
            // A tag: to its closing >, past any quoted value.
            char quote = 0;
            quint32 j = i + 1;
            for (; j < stop; ++j) {
                if (quote) {
                    if (s[j] == quote) quote = 0;
                } else if (s[j] == '"' || s[j] == '\'') {
                    quote = s[j];
                } else if (s[j] == '>') {
                    break;
                }
            }
            i = j + 1;
        }
    }
    return out;
}

QByteArray XmlTree::withDeclarations(int n, quint32 from, quint32 to) const
{
    const Node &nd = m_nodes[n];
    QVector<Decl> extra;
    QSet<int> seen;
    for (quint32 k = 0; k < nd.declCount; ++k) seen.insert(m_decls[nd.declBegin + k].prefix);
    for (int a = nd.parent; a >= 0 && a != m_root; a = m_nodes[a].parent) {
        const Node &an = m_nodes[a];
        for (quint32 k = 0; k < an.declCount; ++k) {
            const Decl &d = m_decls[an.declBegin + k];
            if (seen.contains(d.prefix)) continue;
            seen.insert(d.prefix);
            extra.push_back(d);
        }
    }
    if (extra.isEmpty()) return m_src.mid(from, to - from);
    QByteArray out = m_src.mid(from, nd.nameEnd - from);
    for (const Decl &d : std::as_const(extra)) {
        out += d.prefix == 0 ? QByteArray(" xmlns=\"") : " xmlns:" + m_names[d.prefix] + "=\"";
        out += m_src.mid(d.begin, d.end - d.begin) + '"';
    }
    out += m_src.mid(nd.nameEnd, to - nd.nameEnd);
    return out;
}

QByteArray XmlTree::raw(int n) const { return withDeclarations(n, m_nodes[n].begin, m_nodes[n].end); }

QByteArray XmlTree::startTag(int n) const { return withDeclarations(n, m_nodes[n].begin, m_nodes[n].innerBegin); }

QByteArray XmlTree::endTag(int n) const
{
    const Node &nd = m_nodes[n];
    return isEmptyElement(n) ? QByteArray() : m_src.mid(nd.innerEnd, nd.end - nd.innerEnd);
}

QByteArray XmlTree::inner(int n) const
{
    const Node &nd = m_nodes[n];
    return m_src.mid(nd.innerBegin, nd.innerEnd - nd.innerBegin);
}

} // namespace jp::office::wml
