// The style sheet of an old binary document, and the styles part it makes.

#include "doc/docstyles.h"

#include <QCoreApplication>
#include <QSet>
#include <algorithm>

namespace jp::office::doc {

namespace {

// The names of the built-in styles by number, for a file that doesn't write them.
const char *const kBuiltInNames[] = {
    "Normal", "heading 1", "heading 2", "heading 3", "heading 4", "heading 5", "heading 6", "heading 7", "heading 8", "heading 9",
    "index 1", "index 2", "index 3", "index 4", "index 5", "index 6", "index 7", "index 8", "index 9",
    "toc 1", "toc 2", "toc 3", "toc 4", "toc 5", "toc 6", "toc 7", "toc 8", "toc 9",
    "Normal Indent", "footnote text", "annotation text", "header", "footer", "index heading", "caption", "table of figures", "envelope address",
    "envelope return", "footnote reference", "annotation reference", "line number", "page number", "endnote reference", "endnote text",
    "table of authorities", "macro", "toa heading", "List", "List Bullet", "List Number", "List 2", "List 3", "List 4", "List 5",
    "List Bullet 2", "List Bullet 3", "List Bullet 4", "List Bullet 5", "List Number 2", "List Number 3", "List Number 4", "List Number 5",
    "Title", "Closing", "Signature", "Default Paragraph Font", "Body Text", "Body Text Indent", "List Continue", "List Continue 2",
    "List Continue 3", "List Continue 4", "List Continue 5", "Message Header", "Subtitle", "Salutation", "Date", "Body Text First Indent",
    "Body Text First Indent 2", "Note Heading", "Body Text 2", "Body Text 3", "Body Text Indent 2", "Body Text Indent 3", "Block Text",
    "Hyperlink", "FollowedHyperlink", "Strong", "Emphasis", "Document Map", "Plain Text", "E-mail Signature"};

// An identifier for a style name: its letters, digits, and hyphens (a classic built-in style's with the first letter in capitals).
QString idOfName(const QString &name, bool builtIn)
{
    QString id;
    for (const QChar c : name)
        if (c.isLetterOrNumber() || c == QLatin1Char('-')) id += c;
    if (id.isEmpty()) id = QStringLiteral("Style");
    if (builtIn) id[0] = id[0].toUpper();
    return id;
}

QString xmlText(const QString &s) { return s.toHtmlEscaped(); }

} // namespace

bool StyleSheet::read(const Span &stsh)
{
    styles.clear();
    const int cbStshi = stsh.u16(0);
    if (cbStshi < 18 || !stsh.fits(2, cbStshi)) return false;
    const Span h = stsh.sub(2, cbStshi);
    const int cstd = std::min<int>(h.u16(0), kMaxStyles);
    const int cbBase = h.u16(2);
    const bool namesWritten = h.u16(4) & 1;
    ftcAscii = h.u16(12);
    ftcFE = h.u16(14);
    ftcOther = h.u16(16);
    ftcBi = cbStshi >= 20 ? h.u16(18) : -1;
    if (cbBase < 10) return false;
    styles.resize(cstd);
    qint64 o = 2 + cbStshi;
    for (int i = 0; i < cstd; ++i) {
        if (!stsh.fits(o, 2)) break;
        const int cb = stsh.u16(o);
        o += 2;
        const Span std = stsh.sub(o, cb);
        o += cb;
        if (cb == 0 || std.size() < cbBase + 2) continue;
        StyleDef &d = styles[i];
        d.valid = true;
        d.sti = std.u16(0) & 0xFFF;
        d.sgc = std.u16(2) & 0xF;
        d.base = std.u16(2) >> 4;
        if (d.base == 0xFFF) d.base = -1;
        const int cupx = std.u16(4) & 0xF;
        d.next = std.u16(4) >> 4;
        if (d.next == 0xFFF) d.next = -1;
        const int cch = std.u16(cbBase);
        QString name;
        for (int k = 0; k < cch && k < 255; ++k) name += QChar(std.u16(cbBase + 2 + 2 * qint64(k)));
        if (name.isEmpty() && d.sti < int(sizeof(kBuiltInNames) / sizeof(kBuiltInNames[0]))) name = QString::fromLatin1(kBuiltInNames[d.sti]);
        if (name.isEmpty() && d.sti == 105) name = QStringLiteral("Table Normal");
        if (name.isEmpty() && d.sti == 106) name = QStringLiteral("Table Grid");
        if (name.isEmpty() && d.sti == 107) name = QStringLiteral("No List");
        if (name.isEmpty()) name = QStringLiteral("Style %1").arg(i);
        d.name = name;
        Q_UNUSED(namesWritten)
        qint64 p = cbBase + 2 + 2 * qint64(cch) + 2;
        QVector<Span> upx;
        for (int k = 0; k < cupx; ++k) {
            const int cu = std.u16(p);
            p += 2;
            upx.push_back(std.sub(p, cu));
            p += cu + (cu & 1);
        }
        auto papxOf = [&](const Span &u) {
            if (u.size() < 2) return;
            d.pap.apply(u.from(2));
            d.pap.istd = u.u16(0);
        };
        if (d.sgc == 1) {
            if (upx.size() > 0) papxOf(upx[0]);
            if (upx.size() > 1) d.chp.apply(upx[1]);
        } else if (d.sgc == 2) {
            if (upx.size() > 0) d.chp.apply(upx[0]);
        } else if (d.sgc == 3) {
            if (upx.size() > 0) d.tap.apply(upx[0]);
            if (upx.size() > 1) papxOf(upx[1]);
            if (upx.size() > 2) d.chp.apply(upx[2]);
        } else if (d.sgc == 4) {
            if (upx.size() > 0) papxOf(upx[0]);
        }
    }
    // The ids: from the names, each once.
    QSet<QString> used;
    for (StyleDef &d : styles) {
        if (!d.valid) continue;
        QString id = idOfName(d.name, d.sti < 100);
        QString unique = id;
        for (int n = 2; used.contains(unique.toLower()); ++n) unique = id + QString::number(n);
        used.insert(unique.toLower());
        d.id = unique;
    }
    return !styles.isEmpty();
}

void StyleSheet::settle(int istd, int depth) const
{
    StyleDef &d = const_cast<StyleDef &>(styles[istd]);
    if (d.settledDone) return;
    d.settledDone = true;   // (a style that is its own ancestor stops here)
    quint8 parent[TogCount] = {};
    if (d.base >= 0 && d.base < styles.size() && styles[d.base].valid && depth < kMaxStyleDepth) {
        settle(d.base, depth + 1);
        std::copy(styles[d.base].settled, styles[d.base].settled + TogCount, parent);
    }
    for (int t = 0; t < TogCount; ++t) d.settled[t] = d.chp.tog[t] ? settleToggle(*d.chp.tog[t], parent[t]) : parent[t];
}

void StyleSheet::runParent(int paragraphStyle, int characterStyle, quint8 *out) const
{
    std::fill(out, out + TogCount, quint8(0));
    if (paragraphStyle >= 0 && paragraphStyle < styles.size() && styles[paragraphStyle].valid) {
        settle(paragraphStyle, 0);
        std::copy(styles[paragraphStyle].settled, styles[paragraphStyle].settled + TogCount, out);
    }
    if (characterStyle >= 0 && characterStyle < styles.size() && styles[characterStyle].valid) {
        const StyleDef &c = styles[characterStyle];
        for (int t = 0; t < TogCount; ++t)
            if (c.chp.tog[t]) out[t] = settleToggle(*c.chp.tog[t], out[t]);
    }
}

QByteArray StyleSheet::xml(const FontNames &fonts, int listInstances) const
{
    QByteArray x =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    {
        Chp d;
        d.ftcAscii = ftcAscii;
        d.ftcOther = ftcOther;
        d.ftcFE = ftcFE;
        d.ftcBi = ftcBi;
        const QByteArray rpr = runPropertiesXml(d, fonts, nullptr);
        x += "<w:docDefaults><w:rPrDefault><w:rPr>" + rpr + "</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>";
    }
    bool haveNormal = false, haveChar = false, haveTable = false, haveList = false;
    for (int i = 0; i < styles.size(); ++i) {
        const StyleDef &d = styles[i];
        if (!d.valid) continue;
        const bool isDefault = (d.sgc == 1 && d.sti == 0) || (d.sgc == 2 && d.sti == 65) || (d.sgc == 3 && d.sti == 105) || (d.sgc == 4 && d.sti == 107);
        haveNormal = haveNormal || (d.sgc == 1 && d.sti == 0);
        haveChar = haveChar || (d.sgc == 2 && d.sti == 65);
        haveTable = haveTable || (d.sgc == 3 && d.sti == 105);
        haveList = haveList || (d.sgc == 4 && d.sti == 107);
        const char *type = d.sgc == 1 ? "paragraph" : d.sgc == 2 ? "character" : d.sgc == 3 ? "table" : "numbering";
        x += "<w:style w:type=\"";
        x += type;
        x += "\"";
        if (isDefault) x += " w:default=\"1\"";
        if (d.sti == 0xFFE) x += " w:customStyle=\"1\"";
        x += " w:styleId=\"" + d.id.toUtf8() + "\"><w:name w:val=\"" + xmlText(d.name).toUtf8() + "\"/>";
        // (A style is based on another only where the chain of them is no longer than the limit and comes to an end.)
        bool basedOn = d.base >= 0 && d.base != i && d.base < styles.size() && styles[d.base].valid && styles[d.base].sgc == d.sgc;
        if (basedOn) {
            int at = d.base, steps = 0;
            while (at >= 0 && at < styles.size() && styles[at].valid && steps <= kMaxStyleDepth) {
                if (at == i) {
                    steps = kMaxStyleDepth + 1;   // (it comes round to itself)
                    break;
                }
                at = styles[at].base;
                ++steps;
            }
            basedOn = steps <= kMaxStyleDepth;
        }
        if (basedOn) x += "<w:basedOn w:val=\"" + styles[d.base].id.toUtf8() + "\"/>";
        if (d.sgc == 1 && d.next >= 0 && d.next != i && d.next < styles.size() && styles[d.next].valid && styles[d.next].sgc == 1)
            x += "<w:next w:val=\"" + styles[d.next].id.toUtf8() + "\"/>";
        if (d.sti <= 9 || d.sti == 62 || d.sti == 74 || d.sti == 0xFFE) x += "<w:qFormat/>";
        if (d.sgc == 1 || d.sgc == 4) {
            ParaContext ctx;
            ctx.listInstances = listInstances;
            const QByteArray p = paragraphPropertiesXml(d.pap, ctx);
            if (!p.isEmpty()) x += "<w:pPr>" + p + "</w:pPr>";
        }
        quint8 parent[TogCount] = {};
        if (d.base >= 0 && d.base < styles.size() && styles[d.base].valid) {
            settle(d.base, 1);
            std::copy(styles[d.base].settled, styles[d.base].settled + TogCount, parent);
        }
        const QByteArray r = runPropertiesXml(d.chp, fonts, parent);
        if (!r.isEmpty()) x += "<w:rPr>" + r + "</w:rPr>";
        if (d.sgc == 3) {
            // A table style: its borders and the space inside its cells.
            QByteArray t;
            const QByteArray borders = borderXml("top", d.tap.top) + borderXml("left", d.tap.left) + borderXml("bottom", d.tap.bottom) + borderXml("right", d.tap.right) +
                                       borderXml("insideH", d.tap.insideH) + borderXml("insideV", d.tap.insideV);
            if (!borders.isEmpty()) t += "<w:tblBorders>" + borders + "</w:tblBorders>";
            auto pad = [&](const char *side, int v) { return v >= 0 ? QByteArray("<w:") + side + " w:w=\"" + QByteArray::number(v) + "\" w:type=\"dxa\"/>" : QByteArray(); };
            const QByteArray mar = pad("top", d.tap.padTop) + pad("left", d.tap.padLeft >= 0 ? d.tap.padLeft : d.tap.gapHalf) + pad("bottom", d.tap.padBottom) +
                                   pad("right", d.tap.padRight >= 0 ? d.tap.padRight : d.tap.gapHalf);
            if (!mar.isEmpty()) t += "<w:tblCellMar>" + mar + "</w:tblCellMar>";
            if (!t.isEmpty()) x += "<w:tblPr>" + t + "</w:tblPr>";
        }
        x += "</w:style>";
    }
    // The styles every document has, where the file left them out.
    if (!haveNormal) x += "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>";
    if (!haveChar) x += "<w:style w:type=\"character\" w:default=\"1\" w:styleId=\"DefaultParagraphFont\"><w:name w:val=\"Default Paragraph Font\"/><w:semiHidden/></w:style>";
    if (!haveTable)
        x += "<w:style w:type=\"table\" w:default=\"1\" w:styleId=\"TableNormal\"><w:name w:val=\"Normal Table\"/><w:semiHidden/><w:tblPr><w:tblInd w:w=\"0\" w:type=\"dxa\"/><w:tblCellMar><w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"108\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"108\" w:type=\"dxa\"/></w:tblCellMar></w:tblPr></w:style>";
    if (!haveList) x += "<w:style w:type=\"numbering\" w:default=\"1\" w:styleId=\"NoList\"><w:name w:val=\"No List\"/><w:semiHidden/></w:style>";
    x += "</w:styles>";
    return x;
}

} // namespace jp::office::doc
