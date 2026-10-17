// The lists of an old binary document, and the numbering part they make.

#include "doc/doclists.h"

#include <QHash>
#include <algorithm>

namespace jp::office::doc {

namespace {

// A level, read from `s` at `*o` (advanced past it): returns its XML (the w:lvl element for level `ilvl`), empty if it runs past the data.
QByteArray readLevel(const Span &s, qint64 *o, int ilvl, const FontNames &fonts, const StyleSheet &styles, int linkedStyle)
{
    if (!s.fits(*o, 28)) {
        *o = s.size();
        return QByteArray();
    }
    const Span f = s.sub(*o, 28);
    const int cbChpx = f.u8(24), cbPapx = f.u8(25);
    const qint64 papxAt = *o + 28, chpxAt = papxAt + cbPapx, xstAt = chpxAt + cbChpx;
    const int cch = std::min<int>(s.u16(xstAt), 255);
    const Span papx = s.sub(papxAt, cbPapx), chpx = s.sub(chpxAt, cbChpx);
    QString text;
    for (int i = 0; i < cch; ++i) {
        const ushort c = s.u16(xstAt + 2 + 2 * qint64(i));
        if (c < 9) text += QStringLiteral("%") + QString::number(c + 1);
        else text += QChar(c);
    }
    *o = xstAt + 2 + 2 * qint64(cch);
    if (*o > s.size()) *o = s.size();

    const int flags = f.u8(5);
    const int nfc = f.u8(4);
    QByteArray x = "<w:lvl w:ilvl=\"" + QByteArray::number(ilvl) + "\">";
    x += "<w:start w:val=\"" + QByteArray::number(f.i32(0)) + "\"/>";
    x += "<w:numFmt w:val=\"" + QByteArray(numberFormatName(nfc)) + "\"/>";
    if (flags & 0x08) x += "<w:lvlRestart w:val=\"0\"/>";
    if (linkedStyle >= 0 && !styles.idOf(linkedStyle).isEmpty()) x += "<w:pStyle w:val=\"" + styles.idOf(linkedStyle).toUtf8() + "\"/>";
    if (flags & 0x04) x += "<w:isLgl/>";
    const int follow = f.u8(15);
    x += follow == 1 ? "<w:suff w:val=\"space\"/>" : follow == 2 ? "<w:suff w:val=\"nothing\"/>" : "";
    x += "<w:lvlText w:val=\"" + text.toHtmlEscaped().toUtf8() + "\"/>";
    const int jc = flags & 3;
    x += jc == 1 ? "<w:lvlJc w:val=\"center\"/>" : jc == 2 ? "<w:lvlJc w:val=\"right\"/>" : "<w:lvlJc w:val=\"left\"/>";
    Pap p;
    p.apply(papx);
    ParaContext ctx;
    const QByteArray ppr = paragraphPropertiesXml(p, ctx);
    if (!ppr.isEmpty()) x += "<w:pPr>" + ppr + "</w:pPr>";
    Chp c;
    c.apply(chpx);
    const QByteArray rpr = runPropertiesXml(c, fonts, nullptr);
    if (!rpr.isEmpty()) x += "<w:rPr>" + rpr + "</w:rPr>";
    x += "</w:lvl>";
    return x;
}

} // namespace

Lists readLists(const Span &plfLst, const Span &plfLfo, const FontNames &fonts, const StyleSheet &styles)
{
    Lists out;
    if (plfLst.size() < 2 || plfLfo.size() < 4) return out;
    const int cLst = std::min<int>(plfLst.u16(0), kMaxLists);
    QHash<quint32, int> byId;
    QByteArray abstracts;
    qint64 o = 2;
    // The definitions (28 bytes each), then the levels of each in the same order.
    const qint64 levelsAt = o + 28 * qint64(cLst);
    qint64 lo = levelsAt;
    int made = 0;
    for (int i = 0; i < cLst && plfLst.fits(o + 28 * qint64(i), 28); ++i) {
        const Span l = plfLst.sub(o + 28 * qint64(i), 28);
        const quint32 lsid = l.u32(0);
        const bool simple = l.u8(26) & 1;
        const int levels = simple ? 1 : 9;
        QByteArray x = "<w:abstractNum w:abstractNumId=\"" + QByteArray::number(i) + "\">";
        x += simple ? "<w:multiLevelType w:val=\"singleLevel\"/>" : "<w:multiLevelType w:val=\"multilevel\"/>";
        for (int k = 0; k < levels && lo < plfLst.size(); ++k) {
            const int linked = l.u16(8 + 2 * qint64(k));
            x += readLevel(plfLst, &lo, k, fonts, styles, linked == 0x0FFF ? -1 : linked);
        }
        x += "</w:abstractNum>";
        abstracts += x;
        if (!byId.contains(lsid)) byId.insert(lsid, i);
        ++made;
    }
    // The instances.
    const int lfoMac = int(std::min<quint32>(plfLfo.u32(0), kMaxListInstances));
    QByteArray nums;
    qint64 d = 4 + 16 * qint64(lfoMac);
    for (int i = 0; i < lfoMac && plfLfo.fits(4 + 16 * qint64(i), 16); ++i) {
        const Span lfo = plfLfo.sub(4 + 16 * qint64(i), 16);
        const auto it = byId.constFind(lfo.u32(0));
        const int clfolvl = lfo.u8(12);
        if (plfLfo.u32(d) == 0xFFFFFFFFu) d += 4;
        QByteArray overrides;
        for (int k = 0; k < clfolvl && d < plfLfo.size(); ++k) {
            const int start = plfLfo.i32(d);
            const quint32 bits = plfLfo.u32(d + 4);
            d += 8;
            const int ilvl = bits & 0xF;
            const bool hasStart = bits & 0x10, hasFormat = bits & 0x20;
            QByteArray lvl;
            if (hasFormat) lvl = readLevel(plfLfo, &d, ilvl, fonts, styles, -1);
            if (hasStart || !lvl.isEmpty()) {
                overrides += "<w:lvlOverride w:ilvl=\"" + QByteArray::number(ilvl) + "\">";
                if (hasStart) overrides += "<w:startOverride w:val=\"" + QByteArray::number(start) + "\"/>";
                overrides += lvl + "</w:lvlOverride>";
            }
        }
        nums += "<w:num w:numId=\"" + QByteArray::number(i + 1) + "\"><w:abstractNumId w:val=\"" + QByteArray::number(it == byId.constEnd() ? 0 : it.value()) + "\"/>" + overrides + "</w:num>";
        out.instances = i + 1;
    }
    if (made == 0 || out.instances == 0) return Lists();
    out.xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
              "<w:numbering xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">" +
              abstracts + nums + "</w:numbering>";
    return out;
}

} // namespace jp::office::doc
