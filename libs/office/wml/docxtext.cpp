#include "wml/docxtext.h"

#include <QMap>
#include <QSet>
#include <QStringList>
#include <QVector>
#include <QXmlStreamReader>

#include <algorithm>

namespace jp {

// ---------------- inflate (for .docx and .xlsx, which use deflate) ----------------
// A compact raw-DEFLATE decoder (RFC 1951) so .docx and .xlsx files can be read without zlib headers.
namespace {
struct Inflater {
    // Text and table parts of an office document stay far below this; more
    // is a decompression bomb (a few MB that inflate to gigabytes).
    const uchar *in;
    qsizetype n, pos = 0;
    quint32 bitbuf = 0;
    int bitcnt = 0;
    QByteArray out;
    bool ok = true;
    qsizetype maxOut = 256 * 1024 * 1024;
    int bits(int need)
    {
        quint32 v = bitbuf;
        while (bitcnt < need) {
            if (pos >= n) { ok = false; return 0; }
            v |= quint32(in[pos++]) << bitcnt;
            bitcnt += 8;
        }
        bitbuf = v >> need;
        bitcnt -= need;
        return int(v & ((1u << need) - 1));
    }
    struct Huff { QVector<short> count, symbol; };
    int decode(const Huff &h)
    {
        int code = 0, first = 0, index = 0;
        for (int len = 1; len <= 15; ++len) {
            code |= bits(1);
            const int count = h.count[len];
            if (code - count < first) return h.symbol[index + (code - first)];
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
            if (!ok) return -1;
        }
        ok = false;
        return -1;
    }
    static Huff build(const short *lengths, int n)
    {
        Huff h;
        h.count = QVector<short>(16, 0);
        h.symbol = QVector<short>(n, 0);
        for (int s = 0; s < n; ++s) h.count[lengths[s]]++;
        QVector<short> offs(16, 0);
        for (int len = 1; len < 15; ++len) offs[len + 1] = offs[len] + h.count[len];
        for (int s = 0; s < n; ++s)
            if (lengths[s]) h.symbol[offs[lengths[s]]++] = short(s);
        h.count[0] = 0;
        return h;
    }
    bool codes(const Huff &lencode, const Huff &distcode)
    {
        static const short lbase[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
        static const short lext[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
        static const short dbase[] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
        static const short dext[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
        while (ok) {
            int sym = decode(lencode);
            if (sym < 0) return false;
            if (out.size() > maxOut) return false;
            if (sym < 256) out.append(char(sym));
            else if (sym == 256) return true;
            else {
                sym -= 257;
                if (sym >= 29) return false;
                const int len = lbase[sym] + bits(lext[sym]);
                const int ds = decode(distcode);
                if (ds < 0 || ds >= 30) return false;
                const int dist = dbase[ds] + bits(dext[ds]);
                if (dist > out.size()) return false;
                for (int i = 0; i < len; ++i) out.append(out[out.size() - dist]);
            }
        }
        return false;
    }
    bool run()
    {
        int last;
        do {
            last = bits(1);
            const int type = bits(2);
            if (type == 0) {
                bitbuf = 0; bitcnt = 0;
                if (pos + 4 > n) return false;
                const int len = in[pos] | (in[pos + 1] << 8);
                pos += 4;
                if (pos + len > n || out.size() + len > maxOut) return false;
                out.append(reinterpret_cast<const char *>(in + pos), len);
                pos += len;
            } else if (type == 1) {
                static Huff fl, fd;
                static bool init = false;
                if (!init) {
                    short l[288];
                    for (int s = 0; s < 144; ++s) l[s] = 8;
                    for (int s = 144; s < 256; ++s) l[s] = 9;
                    for (int s = 256; s < 280; ++s) l[s] = 7;
                    for (int s = 280; s < 288; ++s) l[s] = 8;
                    fl = build(l, 288);
                    short d[30];
                    for (short &x : d) x = 5;
                    fd = build(d, 30);
                    init = true;
                }
                if (!codes(fl, fd)) return false;
            } else if (type == 2) {
                const int nlen = bits(5) + 257, ndist = bits(5) + 1, ncode = bits(4) + 4;
                static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
                short lengths[320] = {0};
                for (int i = 0; i < ncode; ++i) lengths[order[i]] = short(bits(3));
                const Huff lencode = build(lengths, 19);
                int idx = 0;
                while (idx < nlen + ndist && ok) {
                    int sym = decode(lencode);
                    if (sym < 0) return false;
                    if (sym < 16) lengths[idx++] = short(sym);
                    else {
                        int len = 0, rep;
                        if (sym == 16) { if (idx == 0) return false; len = lengths[idx - 1]; rep = 3 + bits(2); }
                        else if (sym == 17) rep = 3 + bits(3);
                        else rep = 11 + bits(7);
                        while (rep-- && idx < nlen + ndist) lengths[idx++] = short(len);
                    }
                }
                const Huff lc = build(lengths, nlen), dc = build(lengths + nlen, ndist);
                if (!codes(lc, dc)) return false;
            } else {
                return false;
            }
        } while (!last && ok);
        return ok;
    }
};

// Reads the named parts of a zip, inflating deflated ones, and nothing else: a document's other parts are never inflated (a few KB can
// inflate to gigabytes, and entries may share one stream). A part is refused if its data is also another entry's, or is larger than
// `maxPart` once inflated. Only the first entry of a name counts.
QMap<QString, QByteArray> readOfficeZip(const QByteArray &zip, const QStringList &wanted, qsizetype maxPart = 64 * 1024 * 1024)
{
    QMap<QString, QByteArray> out;
    auto u16 = [&](qsizetype at) { return at >= 0 && at + 2 <= zip.size() ? quint16(uchar(zip[at]) | (uchar(zip[at + 1]) << 8)) : quint16(0); };
    auto u32 = [&](qsizetype at) { return quint32(u16(at)) | (quint32(u16(at + 2)) << 16); };
    qsizetype eocd = -1;
    for (qsizetype i = zip.size() - 22; i >= std::max<qsizetype>(0, zip.size() - 65557); --i)
        if (u32(i) == 0x06054b50) { eocd = i; break; }
    if (eocd < 0) return out;
    struct Part { QString name; int method; qsizetype data, csize; };
    QVector<Part> parts;
    const int count = u16(eocd + 10);
    qsizetype p = u32(eocd + 16);
    for (int i = 0; i < count && p + 46 <= zip.size(); ++i) {
        const int method = u16(p + 10);
        const quint32 csize = u32(p + 20);
        const int nlen = u16(p + 28), xlen = u16(p + 30), clen = u16(p + 32);
        const quint32 local = u32(p + 42);
        if (p + 46 + nlen + xlen + clen > zip.size()) break;   // the name and extras must fit
        const QString name = QString::fromUtf8(zip.constData() + p + 46, nlen);
        p += 46 + nlen + xlen + clen;
        if (qsizetype(local) + 30 > zip.size()) continue;
        const qsizetype data = local + 30 + u16(local + 26) + u16(local + 28);
        if (data + qsizetype(csize) > zip.size()) continue;
        parts << Part{name, method, data, qsizetype(csize)};
    }
    QSet<QString> seen;
    for (int i = 0; i < parts.size(); ++i) {
        const Part &pt = parts[i];
        if (!wanted.contains(pt.name) || seen.contains(pt.name)) continue;
        seen.insert(pt.name);
        bool shared = false;
        for (int k = 0; k < parts.size() && !shared; ++k)
            shared = k != i && parts[k].data < pt.data + pt.csize && pt.data < parts[k].data + parts[k].csize;
        if (shared) continue;
        if (pt.method == 0) { if (pt.csize <= maxPart) out.insert(pt.name, zip.mid(pt.data, pt.csize)); }
        else if (pt.method == 8) {
            Inflater inf{reinterpret_cast<const uchar *>(zip.constData() + pt.data), pt.csize};
            inf.maxOut = maxPart;
            if (inf.run()) out.insert(pt.name, inf.out);
        }
    }
    return out;
}
} // namespace

QString docxToHtml(const QByteArray &docx, int *left, bool *cut)
{
    const auto files = readOfficeZip(docx, {QStringLiteral("word/document.xml")});
    const QByteArray xml = files.value("word/document.xml");
    if (xml.isEmpty()) return {};
    QXmlStreamReader r(xml);
    QString html = "<html><body>";
    bool inPara = false, bold = false, italic = false, underline = false;
    QString align;
    // What the text alone leaves out, counted: pictures (one inside another, or beside its stand-in, once), tables, lists (each list once, however many paragraphs it has), comments, and notes.
    int picture = 0, fallback = 0, leftOut = 0;
    QSet<QString> lists;
    int paragraphs = 0;
    if (cut) *cut = false;
    while (!r.atEnd()) {
        r.readNext();
        const QStringView n = r.name();
        if (r.isStartElement()) {
            if (n == u"Fallback") ++fallback;
            else if ((n == u"drawing" || n == u"pict" || n == u"object") && ++picture == 1 && fallback == 0) ++leftOut;
            else if (n == u"tbl" || n == u"footnoteReference" || n == u"endnoteReference" || n == u"commentReference") ++leftOut;
            else if (n == u"numId" && inPara && r.attributes().value("w:val") != u"0") lists.insert(r.attributes().value("w:val").toString());
            if (n == u"p") {
                if (++paragraphs > kMaxInsertParagraphs) {
                    if (cut) *cut = true;
                    break;
                }
                inPara = true;
                align.clear();
                html += "<p>";
            } else if (n == u"jc") align = r.attributes().value("w:val").toString();
            else if (n == u"r") { bold = italic = underline = false; }
            else if (n == u"b") bold = r.attributes().value("w:val") != u"0";
            else if (n == u"i") italic = r.attributes().value("w:val") != u"0";
            else if (n == u"u") underline = r.attributes().value("w:val") != u"none";
            else if (n == u"t") {
                QString t = r.readElementText().toHtmlEscaped();
                if (bold) t = "<b>" + t + "</b>";
                if (italic) t = "<i>" + t + "</i>";
                if (underline) t = "<u>" + t + "</u>";
                html += t;
            } else if (n == u"tab") html += "\t";
            else if (n == u"br") html += "<br>";
        } else if (r.isEndElement() && (n == u"drawing" || n == u"pict" || n == u"object")) {
            if (picture > 0) --picture;
        } else if (r.isEndElement() && n == u"Fallback") {
            if (fallback > 0) --fallback;
        } else if (r.isEndElement() && n == u"p" && inPara) {
            if (!align.isEmpty()) {
                const QString a = align == u"center" ? "center" : align == u"right" || align == u"end" ? "right" : align == u"both" ? "justify" : "left";
                const int at = html.lastIndexOf("<p>");
                if (at >= 0) html.replace(at, 3, QStringLiteral("<p align=\"%1\">").arg(a));
            }
            html += "</p>";
            inPara = false;
        }
    }
    if (left) *left = leftOut + int(lists.size());
    return html + "</body></html>";
}

} // namespace jp
