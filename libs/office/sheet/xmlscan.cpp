#include "sheet/xmlscan.h"

#include "sheet/cellref.h"

#include <cstring>

namespace jp::office::sheet::xmlscan {

namespace {

bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

qsizetype find(QByteArrayView data, QByteArrayView what, qsizetype from) { return data.indexOf(what, from); }

} // namespace

qsizetype tagEnd(QByteArrayView data, qsizetype start)
{
    char quote = 0;
    for (qsizetype i = start + 1; i < data.size(); ++i) {
        const char c = data[i];
        if (quote) {
            if (c == quote) quote = 0;
        } else if (c == '"' || c == '\'') {
            quote = c;
        } else if (c == '>') {
            return i + 1;
        }
    }
    return -1;
}

bool nextTag(QByteArrayView data, qsizetype from, Tag *tag, bool *cutOff)
{
    if (cutOff) *cutOff = false;
    qsizetype at = from;
    for (;;) {
        at = data.indexOf('<', at);
        if (at < 0) return false;
        if (at + 1 >= data.size()) {
            if (cutOff) *cutOff = true;
            return false;
        }
        const char c = data[at + 1];
        auto skipTo = [&](QByteArrayView close) {
            const qsizetype e = find(data, close, at + 2);
            if (e < 0) {
                if (cutOff) *cutOff = true;
                return false;
            }
            at = e + close.size();
            return true;
        };
        if (c == '!') {
            if (data.sliced(at).startsWith("<!--")) {
                if (!skipTo("-->")) return false;
            } else if (data.sliced(at).startsWith("<![CDATA[")) {
                if (!skipTo("]]>")) return false;
            } else {
                if (!skipTo(">")) return false;
            }
            continue;
        }
        if (c == '?') {
            if (!skipTo("?>")) return false;
            continue;
        }
        const qsizetype end = tagEnd(data, at);
        if (end < 0) {
            if (cutOff) *cutOff = true;
            return false;
        }
        tag->start = at;
        tag->end = end;
        tag->closing = c == '/';
        tag->selfClosing = !tag->closing && end - at >= 2 && data[end - 2] == '/';
        qsizetype n = at + (tag->closing ? 2 : 1);
        const qsizetype nameStart = n;
        while (n < end - 1 && !isSpace(data[n]) && data[n] != '/' && data[n] != '>') ++n;
        tag->name = data.sliced(nameStart, n - nameStart);
        tag->local = localName(tag->name);
        return true;
    }
}

bool attributeValue(QByteArrayView tag, QByteArrayView name, qsizetype *start, qsizetype *end)
{
    qsizetype i = 1;
    // (the tag's own name)
    while (i < tag.size() && !isSpace(tag[i]) && tag[i] != '/' && tag[i] != '>') ++i;
    for (;;) {
        while (i < tag.size() && isSpace(tag[i])) ++i;
        if (i >= tag.size() || tag[i] == '/' || tag[i] == '>') return false;
        const qsizetype nameStart = i;
        while (i < tag.size() && tag[i] != '=' && !isSpace(tag[i]) && tag[i] != '>' && tag[i] != '/') ++i;
        const QByteArrayView attrName = tag.sliced(nameStart, i - nameStart);
        while (i < tag.size() && isSpace(tag[i])) ++i;
        if (i >= tag.size() || tag[i] != '=') continue;   // (an attribute with no value: not XML, passed over)
        ++i;
        while (i < tag.size() && isSpace(tag[i])) ++i;
        if (i >= tag.size() || (tag[i] != '"' && tag[i] != '\'')) return false;
        const char quote = tag[i++];
        const qsizetype valueStart = i;
        while (i < tag.size() && tag[i] != quote) ++i;
        if (i >= tag.size()) return false;
        if (attrName == name) {
            *start = valueStart;
            *end = i;
            return true;
        }
        ++i;
    }
}

QByteArrayView attribute(QByteArrayView tag, QByteArrayView name, bool *found)
{
    qsizetype s = 0, e = 0;
    const bool ok = attributeValue(tag, name, &s, &e);
    if (found) *found = ok;
    return ok ? tag.sliced(s, e - s) : QByteArrayView();
}

QByteArrayView prefixOf(QByteArrayView name)
{
    const qsizetype colon = name.indexOf(':');
    return colon < 0 ? QByteArrayView() : name.first(colon + 1);
}

QByteArrayView localName(QByteArrayView name)
{
    const qsizetype colon = name.indexOf(':');
    return colon < 0 ? name : name.sliced(colon + 1);
}

namespace {

bool isHex4At(const QString &s, int at)
{
    if (at + 7 > s.size()) return false;
    if (s[at] != QLatin1Char('_') || s[at + 1] != QLatin1Char('x') || s[at + 6] != QLatin1Char('_')) return false;
    for (int k = 2; k < 6; ++k) {
        const ushort u = s[at + k].unicode();
        if (!((u >= '0' && u <= '9') || (u >= 'a' && u <= 'f') || (u >= 'A' && u <= 'F'))) return false;
    }
    return true;
}

QByteArray escape(const QString &text, bool attribute)
{
    QString out;
    out.reserve(text.size() + text.size() / 8);
    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        const ushort u = ch.unicode();
        auto hex = [&](ushort v) { out += QLatin1String("_x") + QString::number(v, 16).rightJustified(4, QLatin1Char('0')).toUpper() + QLatin1Char('_'); };
        switch (u) {
        case '&': out += QLatin1String("&amp;"); break;
        case '<': out += QLatin1String("&lt;"); break;
        case '>': out += QLatin1String("&gt;"); break;
        case '"':
            if (attribute) out += QLatin1String("&quot;");
            else out += ch;
            break;
        case '_':
            if (isHex4At(text, i)) out += QLatin1String("_x005F_");
            else out += ch;
            break;
        default:
            if (u < 0x20 && u != '\t' && u != '\n') {
                hex(u);
            } else if (u == 0xFFFE || u == 0xFFFF) {
                hex(u);
            } else if (ch.isHighSurrogate() && i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
                out += ch;
                out += text[++i];
            } else if (ch.isSurrogate()) {
                hex(u);
            } else {
                out += ch;
            }
        }
    }
    return out.toUtf8();
}

} // namespace

QByteArray escapeText(const QString &text) { return escape(text, false); }
QByteArray escapeAttribute(const QString &text) { return escape(text, true); }

QString unescape(QByteArrayView text)
{
    if (text.indexOf('&') < 0) return QString::fromUtf8(text.data(), text.size());
    QByteArray out;
    out.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text[i] != '&') {
            out.append(text[i]);
            continue;
        }
        const qsizetype semi = text.indexOf(';', i);
        if (semi < 0 || semi - i > 10) {
            out.append(text[i]);
            continue;
        }
        const QByteArrayView name = text.sliced(i + 1, semi - i - 1);
        if (name == "amp") out.append('&');
        else if (name == "lt") out.append('<');
        else if (name == "gt") out.append('>');
        else if (name == "quot") out.append('"');
        else if (name == "apos") out.append('\'');
        else if (name.size() > 1 && name[0] == '#') {
            bool ok = false;
            const uint code = name[1] == 'x' || name[1] == 'X' ? QByteArray(name.sliced(2)).toUInt(&ok, 16) : QByteArray(name.sliced(1)).toUInt(&ok, 10);
            if (!ok || code == 0 || code > 0x10FFFF) {
                out.append(text.sliced(i, semi - i + 1));
            } else {
                out.append(QString::fromUcs4(reinterpret_cast<const char32_t *>(&code), 1).toUtf8());
            }
        } else {
            out.append(text.sliced(i, semi - i + 1));
        }
        i = semi;
    }
    return QString::fromUtf8(out);
}

bool parseAddress(QByteArrayView text, quint32 *row, quint32 *col)
{
    qsizetype i = 0;
    quint64 c = 0;
    int letters = 0;
    for (; i < text.size(); ++i) {
        char ch = text[i];
        if (ch >= 'a' && ch <= 'z') ch = char(ch - 'a' + 'A');
        if (ch < 'A' || ch > 'Z') break;
        c = c * 26 + quint64(ch - 'A' + 1);
        if (++letters > 3) return false;
    }
    if (letters == 0 || c == 0 || c > kMaxColumns) return false;
    quint64 r = 0;
    int digits = 0;
    for (; i < text.size(); ++i) {
        if (text[i] < '0' || text[i] > '9') return false;
        r = r * 10 + quint64(text[i] - '0');
        if (++digits > 7) return false;
    }
    if (digits == 0 || r == 0 || r > kMaxRows) return false;
    *row = quint32(r - 1);
    *col = quint32(c - 1);
    return true;
}

} // namespace jp::office::sheet::xmlscan
