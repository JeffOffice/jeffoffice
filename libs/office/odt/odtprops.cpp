// OpenDocument text to .docx: the properties of paragraphs and text, the styles part, and the numbering part.

#include "odt/odtread.h"

#include <QRegularExpression>
#include <algorithm>
#include <atomic>
#include <cmath>

namespace jp::office::odt {

namespace {
std::atomic<qint64> g_looksMade{0};
}
qint64 Reader::looksMadeForTest() { return g_looksMade.load(); }

namespace {

QByteArray num(qint64 v)
{
    return QByteArray::number(v);
}

// A border's text ("0.5pt solid #000000") as the parts of a w:top...: false for none.
bool parseBorder(const QString &text, QString *val, int *sz, QString *color)
{
    const QString t = text.trimmed();
    if (t.isEmpty()) return false;
    if (t == QLatin1String("none") || t == QLatin1String("hidden")) {
        *val = QStringLiteral("nil");
        *sz = 0;
        color->clear();
        return true;
    }
    const QStringList parts = t.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    double width = 0.5;
    QString style = QStringLiteral("solid");
    QString col;
    for (const QString &p : parts) {
        double pt = 0;
        if (p.startsWith(QLatin1Char('#'))) col = colorHex(p);
        else if (lengthToPoints(p, &pt)) width = pt;
        else style = p;
    }
    static const QHash<QString, QString> kStyles = {{QStringLiteral("solid"), QStringLiteral("single")}, {QStringLiteral("double"), QStringLiteral("double")},
                                                    {QStringLiteral("dotted"), QStringLiteral("dotted")}, {QStringLiteral("dashed"), QStringLiteral("dashed")},
                                                    {QStringLiteral("groove"), QStringLiteral("single")}, {QStringLiteral("ridge"), QStringLiteral("single")},
                                                    {QStringLiteral("inset"), QStringLiteral("single")}, {QStringLiteral("outset"), QStringLiteral("single")}};
    *val = kStyles.value(style, QStringLiteral("single"));
    *sz = int(std::max(2.0, std::min(96.0, std::round(width * 8))));
    *color = col.isEmpty() ? QStringLiteral("auto") : col;
    return true;
}

} // namespace

QString Reader::fontsXmlValue(const Props &p, const QString &kind, const char *which) const
{
    QString name = p.value(kind + QLatin1String("style:font-name") + which);
    if (!name.isEmpty()) return fontFamily(name);
    QString family = p.value(kind + QLatin1String("fo:font-family"));
    if (!QLatin1String(which).isEmpty()) family = p.value(kind + QLatin1String("style:font-family") + which);
    family.remove(QLatin1Char('\''));
    family.remove(QLatin1Char('"'));
    return family.trimmed();
}

QByteArray Reader::fontsXml(const Props &p, const QString &kind) const
{
    const QString latin = fontsXmlValue(p, kind, ""), asian = fontsXmlValue(p, kind, "-asian"), complex = fontsXmlValue(p, kind, "-complex");
    if (latin.isEmpty() && asian.isEmpty() && complex.isEmpty()) return QByteArray();
    QByteArray x = "<w:rFonts";
    if (!latin.isEmpty()) x += " w:ascii=\"" + esc(latin) + "\" w:hAnsi=\"" + esc(latin) + "\"";
    if (!asian.isEmpty()) x += " w:eastAsia=\"" + esc(asian) + "\"";
    if (!complex.isEmpty()) x += " w:cs=\"" + esc(complex) + "\"";
    return x + "/>";
}

QByteArray Reader::shadingXml(const QString &color) const
{
    const QString hex = colorHex(color);
    if (hex.isEmpty()) {
        if (color == QLatin1String("transparent")) return "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"auto\"/>";
        return QByteArray();
    }
    return "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + hex.toLatin1() + "\"/>";
}

QByteArray Reader::bordersXml(const Props &p, const QString &kind, const char *tag, bool cell) const
{
    static const char *const sides[] = {"top", "left", "bottom", "right"};
    QByteArray x;
    for (const char *side : sides) {
        QString value = p.value(kind + QLatin1String("fo:border-") + QLatin1String(side));
        if (value.isEmpty()) value = p.value(kind + QLatin1String("fo:border"));
        QString val, color;
        int sz = 0;
        if (!parseBorder(value, &val, &sz, &color)) continue;
        // A double line's width is the three widths of it together.
        QString lw = p.value(kind + QLatin1String("style:border-line-width-") + QLatin1String(side));
        if (lw.isEmpty()) lw = p.value(kind + QLatin1String("style:border-line-width"));
        if (!lw.isEmpty() && val == QLatin1String("double")) {
            double total = 0;
            for (const QString &w : lw.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                double pt = 0;
                if (lengthToPoints(w, &pt)) total += pt;
            }
            if (total > 0) sz = int(std::max(2.0, std::min(96.0, std::round(total * 8))));
        }
        int space = 0;
        if (!cell) {
            QString pad = p.value(kind + QLatin1String("fo:padding-") + QLatin1String(side));
            if (pad.isEmpty()) pad = p.value(kind + QLatin1String("fo:padding"));
            double pt = 0;
            if (lengthToPoints(pad, &pt)) space = int(std::min(31.0, std::round(pt)));
        }
        const char *name = side;
        x += QByteArray("<w:") + name + " w:val=\"" + val.toLatin1() + "\"";
        if (val != QLatin1String("nil")) x += " w:sz=\"" + num(sz) + "\" w:space=\"" + num(space) + "\" w:color=\"" + color.toLatin1() + "\"";
        x += "/>";
    }
    return x.isEmpty() ? x : QByteArray("<w:") + tag + ">" + x + "</w:" + tag + ">";
}

QByteArray Reader::rPrXml(const Props &p, const QString &rStyle, bool inStyle) const
{
    Q_UNUSED(inStyle);
    ++g_looksMade;
    auto g = [&](const char *k) { return p.value(QStringLiteral("text:") + QLatin1String(k)); };
    auto has = [&](const char *k) { return p.contains(QStringLiteral("text:") + QLatin1String(k)); };
    QByteArray x;
    if (!rStyle.isEmpty()) x += "<w:rStyle w:val=\"" + esc(rStyle) + "\"/>";
    x += fontsXml(p, QStringLiteral("text:"));
    if (has("fo:font-weight")) {
        const QString w = g("fo:font-weight");
        bool numeric = false;
        const int n = w.toInt(&numeric);
        const bool bold = w == QLatin1String("bold") || (numeric && n >= 600);
        x += bold ? "<w:b/><w:bCs/>" : "<w:b w:val=\"0\"/><w:bCs w:val=\"0\"/>";
    }
    if (has("fo:font-style")) {
        const QString s = g("fo:font-style");
        const bool it = s == QLatin1String("italic") || s == QLatin1String("oblique");
        x += it ? "<w:i/><w:iCs/>" : "<w:i w:val=\"0\"/><w:iCs w:val=\"0\"/>";
    }
    if (has("fo:text-transform")) x += g("fo:text-transform") == QLatin1String("uppercase") ? "<w:caps/>" : "<w:caps w:val=\"0\"/>";
    if (has("fo:font-variant")) x += g("fo:font-variant") == QLatin1String("small-caps") ? "<w:smallCaps/>" : "<w:smallCaps w:val=\"0\"/>";
    {
        const QString style = g("style:text-line-through-style"), type = g("style:text-line-through-type");
        if (has("style:text-line-through-style") || has("style:text-line-through-type")) {
            const bool on = (!style.isEmpty() && style != QLatin1String("none")) || (style.isEmpty() && !type.isEmpty() && type != QLatin1String("none"));
            if (!on) x += "<w:strike w:val=\"0\"/>";
            else x += type == QLatin1String("double") ? "<w:dstrike/>" : "<w:strike/>";
        }
    }
    if (has("fo:text-shadow")) x += g("fo:text-shadow") == QLatin1String("none") ? "<w:shadow w:val=\"0\"/>" : "<w:shadow/>";
    if (has("style:font-relief")) {
        const QString r = g("style:font-relief");
        if (r == QLatin1String("embossed")) x += "<w:emboss/>";
        else if (r == QLatin1String("engraved")) x += "<w:imprint/>";
    }
    if (g("text:display") == QLatin1String("none")) x += "<w:vanish/>";
    if (has("fo:color") || g("style:use-window-font-color") == QLatin1String("true")) {
        const QString c = colorHex(g("fo:color"));
        if (g("style:use-window-font-color") == QLatin1String("true") || c.isEmpty()) x += "<w:color w:val=\"auto\"/>";
        else x += "<w:color w:val=\"" + c.toLatin1() + "\"/>";
    }
    if (has("fo:letter-spacing")) {
        bool ok = false;
        const qint64 tw = lengthToTwips(g("fo:letter-spacing"), &ok);
        if (ok) x += "<w:spacing w:val=\"" + num(tw) + "\"/>";
        else if (g("fo:letter-spacing") == QLatin1String("normal")) x += "<w:spacing w:val=\"0\"/>";
    }
    if (has("style:text-scale")) {
        double pct = 0;
        if (percentOf(g("style:text-scale"), &pct) && pct >= 1 && pct <= 600) x += "<w:w w:val=\"" + num(qint64(std::llround(pct))) + "\"/>";
    }
    QByteArray vertAlign;
    if (has("style:text-position")) {
        const QString pos = g("style:text-position").trimmed();
        const QString first = pos.split(QLatin1Char(' '), Qt::SkipEmptyParts).value(0);
        double v = 0;
        if (first == QLatin1String("super")) vertAlign = "superscript";
        else if (first == QLatin1String("sub")) vertAlign = "subscript";
        else if (percentOf(first, &v)) vertAlign = v > 0 ? "superscript" : v < 0 ? "subscript" : "baseline";
    }
    if (has("fo:font-size")) {
        bool ok = false;
        double pt = 0;
        ok = lengthToPoints(g("fo:font-size"), &pt);
        if (ok && pt > 0 && pt < 1639) x += "<w:sz w:val=\"" + num(qint64(std::llround(pt * 2))) + "\"/>";
    }
    {
        double pt = 0;
        if (has("style:font-size-complex") && lengthToPoints(g("style:font-size-complex"), &pt) && pt > 0 && pt < 1639) x += "<w:szCs w:val=\"" + num(qint64(std::llround(pt * 2))) + "\"/>";
        else if (has("fo:font-size") && lengthToPoints(g("fo:font-size"), &pt) && pt > 0 && pt < 1639) x += "<w:szCs w:val=\"" + num(qint64(std::llround(pt * 2))) + "\"/>";
    }
    if (has("style:text-underline-style") || has("style:text-underline-type")) {
        const QString style = g("style:text-underline-style"), type = g("style:text-underline-type"), width = g("style:text-underline-width");
        if (style == QLatin1String("none") || (style.isEmpty() && type == QLatin1String("none"))) x += "<w:u w:val=\"none\"/>";
        else {
            QByteArray val = "single";
            if (type == QLatin1String("double")) val = "double";
            else if (style == QLatin1String("dotted")) val = width == QLatin1String("bold") ? "dottedHeavy" : "dotted";
            else if (style == QLatin1String("dash")) val = width == QLatin1String("bold") ? "dashedHeavy" : "dash";
            else if (style == QLatin1String("long-dash")) val = "dashLong";
            else if (style == QLatin1String("dot-dash")) val = "dotDash";
            else if (style == QLatin1String("dot-dot-dash")) val = "dotDotDash";
            else if (style == QLatin1String("wave")) val = width == QLatin1String("bold") ? "wavyHeavy" : "wave";
            else if (width == QLatin1String("bold")) val = "thick";
            const QString c = colorHex(g("style:text-underline-color"));
            x += "<w:u w:val=\"" + val + "\"" + (c.isEmpty() ? QByteArray() : " w:color=\"" + c.toLatin1() + "\"") + "/>";
        }
    }
    if (has("fo:background-color")) x += shadingXml(g("fo:background-color"));
    if (!vertAlign.isEmpty()) x += "<w:vertAlign w:val=\"" + vertAlign + "\"/>";
    if (has("fo:language") || has("style:language-asian") || has("style:language-complex")) {
        QString lang = g("fo:language");
        if (!lang.isEmpty() && lang != QLatin1String("zxx") && !g("fo:country").isEmpty() && g("fo:country") != QLatin1String("none")) lang += QLatin1Char('-') + g("fo:country");
        auto tag = [&](const char *l, const char *c) {
            QString v = g(l);
            if (!v.isEmpty() && !g(c).isEmpty() && g(c) != QLatin1String("none")) v += QLatin1Char('-') + g(c);
            return v;
        };
        const QString asian = tag("style:language-asian", "style:country-asian"), complex = tag("style:language-complex", "style:country-complex");
        QByteArray l = "<w:lang";
        if (!lang.isEmpty() && lang != QLatin1String("zxx")) l += " w:val=\"" + esc(lang) + "\"";
        if (!asian.isEmpty() && asian != QLatin1String("zxx")) l += " w:eastAsia=\"" + esc(asian) + "\"";
        if (!complex.isEmpty() && complex != QLatin1String("zxx")) l += " w:bidi=\"" + esc(complex) + "\"";
        if (lang == QLatin1String("zxx")) l += " w:val=\"zxx\"";
        if (l.size() > 7) x += l + "/>";
    }
    return x;
}

QByteArray Reader::pPrXml(const Props &p, const QByteArray &tabs, const QByteArray &pStyle, const QByteArray &numPr, int outline, bool pageBreakBefore) const
{
    ++g_looksMade;
    auto g = [&](const char *k) { return p.value(QStringLiteral("para:") + QLatin1String(k)); };
    auto has = [&](const char *k) { return p.contains(QStringLiteral("para:") + QLatin1String(k)); };
    auto side = [&](const char *name, const char *s) {
        const QString full = QStringLiteral("para:fo:") + QLatin1String(name) + QLatin1Char('-') + QLatin1String(s);
        if (p.contains(full)) return p.value(full);
        return p.value(QStringLiteral("para:fo:") + QLatin1String(name));
    };
    QByteArray x = pStyle;
    if (has("fo:keep-with-next")) x += g("fo:keep-with-next") == QLatin1String("always") ? "<w:keepNext/>" : "<w:keepNext w:val=\"0\"/>";
    if (has("fo:keep-together")) x += g("fo:keep-together") == QLatin1String("always") ? "<w:keepLines/>" : "<w:keepLines w:val=\"0\"/>";
    if (pageBreakBefore || g("fo:break-before") == QLatin1String("page")) x += "<w:pageBreakBefore/>";
    if (has("fo:orphans") || has("fo:widows")) {
        const bool on = g("fo:orphans").toInt() >= 2 || g("fo:widows").toInt() >= 2;
        x += on ? "<w:widowControl/>" : "<w:widowControl w:val=\"0\"/>";
    }
    x += numPr;
    x += bordersXml(p, QStringLiteral("para:"), "pBdr", false);
    if (has("fo:background-color")) x += shadingXml(g("fo:background-color"));
    x += tabs;
    {
        QByteArray sp;
        bool ok = false;
        const QString before = side("margin", "top"), after = side("margin", "bottom");
        qint64 v = lengthToTwips(before, &ok);
        if (ok) sp += " w:before=\"" + num(std::max<qint64>(0, v)) + "\"";
        v = lengthToTwips(after, &ok);
        if (ok) sp += " w:after=\"" + num(std::max<qint64>(0, v)) + "\"";
        const QString lh = g("fo:line-height");
        double pct = 0;
        if (percentOf(lh, &pct)) {
            if (pct >= 1 && pct <= 1000) sp += " w:line=\"" + num(qint64(std::llround(pct * 2.4))) + "\" w:lineRule=\"auto\"";
        } else if (!lh.isEmpty() && lh != QLatin1String("normal")) {
            v = lengthToTwips(lh, &ok);
            if (ok && v > 0) sp += " w:line=\"" + num(v) + "\" w:lineRule=\"exact\"";
        } else if (has("style:line-height-at-least")) {
            v = lengthToTwips(g("style:line-height-at-least"), &ok);
            if (ok && v > 0) sp += " w:line=\"" + num(v) + "\" w:lineRule=\"atLeast\"";
        }
        if (!sp.isEmpty()) x += "<w:spacing" + sp + "/>";
    }
    {
        QByteArray ind;
        bool ok = false;
        qint64 v = lengthToTwips(side("margin", "left"), &ok);
        if (ok) ind += " w:left=\"" + num(v) + "\"";
        v = lengthToTwips(side("margin", "right"), &ok);
        if (ok) ind += " w:right=\"" + num(v) + "\"";
        v = lengthToTwips(g("fo:text-indent"), &ok);
        if (ok) ind += v >= 0 ? " w:firstLine=\"" + num(v) + "\"" : " w:hanging=\"" + num(-v) + "\"";
        if (!ind.isEmpty()) x += "<w:ind" + ind + "/>";
    }
    if (has("style:contextual-spacing")) x += g("style:contextual-spacing") == QLatin1String("true") ? "<w:contextualSpacing/>" : "<w:contextualSpacing w:val=\"0\"/>";
    if (has("fo:text-align")) {
        const QString a = g("fo:text-align");
        const char *jc = a == QLatin1String("center") ? "center" : a == QLatin1String("justify") ? "both" : (a == QLatin1String("end") || a == QLatin1String("right")) ? "right" : "left";
        x += QByteArray("<w:jc w:val=\"") + jc + "\"/>";
    }
    if (g("style:writing-mode") == QLatin1String("rl-tb") || g("style:writing-mode") == QLatin1String("rl")) x += "<w:bidi/>";
    if (outline > 0 && outline <= 9) x += "<w:outlineLvl w:val=\"" + num(outline - 1) + "\"/>";
    return x;
}

// ---- the styles part

QByteArray Reader::stylesXml()
{
    QByteArray x = QByteArray(kXmlHeader) + "<w:styles " + docxNamespaces() + ">";
    // The defaults of the document.
    {
        const Props d = m_defaults.value(QStringLiteral("paragraph"));
        Props t = d;
        QByteArray r = rPrXml(t, QString(), true);
        if (!t.contains(QStringLiteral("text:fo:font-size"))) r += "<w:sz w:val=\"24\"/><w:szCs w:val=\"24\"/>";
        x += "<w:docDefaults><w:rPrDefault><w:rPr>" + r + "</w:rPr></w:rPrDefault><w:pPrDefault><w:pPr>" + pPrXml(d, tabsOfDefault(), QByteArray(), QByteArray(), 0, false) + "</w:pPr></w:pPrDefault></w:docDefaults>";
    }
    x += "<w:style w:type=\"character\" w:default=\"1\" w:styleId=\"DefaultParagraphFont\"><w:name w:val=\"Default Paragraph Font\"/><w:uiPriority w:val=\"1\"/><w:semiHidden/><w:unhideWhenUsed/></w:style>";
    bool normal = false, footnoteRef = false, hyperlink = false;
    for (const QString &key : std::as_const(m_styleOrder)) {
        const StyleDef &d = m_styleDefs[key];
        const bool paragraph = d.family == QLatin1String("paragraph"), character = d.family == QLatin1String("text");
        if (!paragraph && !character) continue;
        const QString id = styleId(d.family, d.name);
        const QString display = displayOf(d.family, d.name);
        QString name = display;
        const bool b = builtInStyleName(d.family, display, &name);
        if (paragraph && id == QLatin1String("Normal")) normal = true;
        if (character && id == QLatin1String("FootnoteReference")) footnoteRef = true;
        if (character && id == QLatin1String("Hyperlink")) hyperlink = true;
        x += QByteArray("<w:style w:type=\"") + (paragraph ? "paragraph" : "character") + "\"" + (paragraph && id == QLatin1String("Normal") ? " w:default=\"1\"" : "") + (b ? "" : " w:customStyle=\"1\"") +
             " w:styleId=\"" + esc(id) + "\"><w:name w:val=\"" + esc(name) + "\"/>";
        if (!d.parent.isEmpty() && style(d.family, d.parent) && !style(d.family, d.parent)->automatic) x += "<w:basedOn w:val=\"" + esc(styleId(d.family, d.parent)) + "\"/>";
        if (paragraph && !d.next.isEmpty() && style(d.family, d.next)) x += "<w:next w:val=\"" + esc(styleId(d.family, d.next)) + "\"/>";
        x += "<w:qFormat/>";
        if (paragraph) {
            QByteArray numPr;
            if (!d.listStyle.isEmpty()) {
                const int numId = numberOf(d.listStyle, false);
                if (numId > 0) numPr = "<w:numPr><w:ilvl w:val=\"" + num(d.outlineLevel > 0 && d.listStyle == QLatin1String("Outline") ? d.outlineLevel - 1 : 0) + "\"/><w:numId w:val=\"" + num(numId) + "\"/></w:numPr>";
            }
            const QByteArray ppr = pPrXml(d.props, d.tabsXml, QByteArray(), numPr, d.outlineLevel, false);
            if (!ppr.isEmpty()) x += "<w:pPr>" + ppr + "</w:pPr>";
        }
        const QByteArray rpr = rPrXml(d.props, QString(), true);
        if (!rpr.isEmpty()) x += "<w:rPr>" + rpr + "</w:rPr>";
        x += "</w:style>";
    }
    if (!normal) x += "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>";
    if (!footnoteRef) x += "<w:style w:type=\"character\" w:styleId=\"FootnoteReference\"><w:name w:val=\"footnote reference\"/><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr></w:style>";
    if (!hyperlink) x += "<w:style w:type=\"character\" w:styleId=\"Hyperlink\"><w:name w:val=\"Hyperlink\"/><w:rPr><w:color w:val=\"0000FF\"/><w:u w:val=\"single\"/></w:rPr></w:style>";
    // The table style a table with no style of its own has.
    x += "<w:style w:type=\"table\" w:default=\"1\" w:styleId=\"TableNormal\"><w:name w:val=\"Normal Table\"/><w:uiPriority w:val=\"99\"/><w:semiHidden/><w:unhideWhenUsed/><w:tblPr><w:tblInd w:w=\"0\" w:type=\"dxa\"/><w:tblCellMar><w:top w:w=\"0\" w:type=\"dxa\"/><w:left w:w=\"108\" w:type=\"dxa\"/><w:bottom w:w=\"0\" w:type=\"dxa\"/><w:right w:w=\"108\" w:type=\"dxa\"/></w:tblCellMar></w:tblPr></w:style>";
    x += "</w:styles>";
    return x;
}

QByteArray Reader::tabsOfDefault() const
{
    return QByteArray();
}

// ---- numbering

namespace {

QString bulletOf(const QString &c)
{
    if (c.isEmpty()) return QStringLiteral("•");
    const ushort u = c[0].unicode();
    switch (u) {
    case 0xE00A: return QStringLiteral("■");
    case 0xE00C: return QStringLiteral("❖");
    case 0xE00D: return QStringLiteral("➢");
    case 0xE01B: return QStringLiteral("✔");
    case 0xE011: return QStringLiteral("➔");
    default: break;
    }
    if (u >= 0xE000 && u <= 0xF8FF) return QStringLiteral("•");
    return c.left(1);
}

const char *numFmtOf(const QString &fmt)
{
    if (fmt == QLatin1String("1")) return "decimal";
    if (fmt == QLatin1String("a")) return "lowerLetter";
    if (fmt == QLatin1String("A")) return "upperLetter";
    if (fmt == QLatin1String("i")) return "lowerRoman";
    if (fmt == QLatin1String("I")) return "upperRoman";
    if (fmt == QLatin1String("01")) return "decimalZero";
    if (fmt.isEmpty()) return "none";
    return "decimal";
}

} // namespace

int Reader::numberOf(const QString &listStyle, bool fresh)
{
    const QString key = m_listStyles.contains(listStyle) ? listStyle : QString();
    auto it = m_abstractOf.constFind(key);
    int abstractId;
    if (it == m_abstractOf.constEnd()) {
        if (m_abstractOrder.size() >= kMaxMadeParts) return 0;
        abstractId = int(m_abstractOrder.size());
        m_abstractOf.insert(key, abstractId);
        m_abstractOrder.push_back(key);
    } else abstractId = it.value();
    const auto first = m_firstNumOf.constFind(key);
    if (first == m_firstNumOf.constEnd()) {
        m_nums.push_back(NumUse{key, abstractId, false});
        m_lastNumOf.insert(key, int(m_nums.size()));
        m_firstNumOf.insert(key, int(m_nums.size()));
        return int(m_nums.size());
    }
    bool numbered = false;
    const ListStyleDef def = m_listStyles.value(key);
    for (const ListLevel &l : def.level)
        if (l.defined && !l.bullet) numbered = true;
    if (!fresh || !numbered || m_nums.size() >= kMaxMadeParts) return m_lastNumOf.value(key);
    m_nums.push_back(NumUse{key, abstractId, true});
    m_lastNumOf.insert(key, int(m_nums.size()));
    return int(m_nums.size());
}

QByteArray Reader::numberingXml() const
{
    QByteArray x = QByteArray(kXmlHeader) + "<w:numbering " + docxNamespaces() + ">";
    for (int i = 0; i < m_abstractOrder.size(); ++i) {
        const QString &name = m_abstractOrder[i];
        ListStyleDef def = m_listStyles.value(name);
        x += "<w:abstractNum w:abstractNumId=\"" + num(i) + "\"><w:multiLevelType w:val=\"multilevel\"/>";
        for (int lv = 0; lv < 9; ++lv) {
            ListLevel l = def.level[lv];
            if (!l.defined) {
                // A level the style does not give is a bullet, further in.
                l.defined = true;
                l.bullet = name.isEmpty() || !m_listStyles.contains(name) || true;
                l.bulletChar = QStringLiteral("•");
                l.left = 720 * (lv + 1);
                l.hanging = 360;
                l.hasIndent = true;
            }
            QByteArray lvlText;
            const char *fmt = "bullet";
            if (l.bullet) lvlText = esc(bulletOf(l.bulletChar));
            else {
                fmt = numFmtOf(l.numFmt);
                QString t = l.prefix;
                const int shown = std::min(l.display, lv + 1);
                for (int k = lv + 1 - shown + 1; k <= lv + 1; ++k) {
                    t += QStringLiteral("%") + QString::number(k);
                    if (k < lv + 1) t += QLatin1Char('.');
                }
                t += l.suffix;
                if (QLatin1String(fmt) == QLatin1String("none")) t = l.prefix + l.suffix;
                lvlText = esc(t);
            }
            x += "<w:lvl w:ilvl=\"" + num(lv) + "\"><w:start w:val=\"" + num(l.start) + "\"/><w:numFmt w:val=\"" + fmt + "\"/><w:lvlText w:val=\"" + lvlText + "\"/><w:lvlJc w:val=\"left\"/>";
            if (l.hasIndent) x += "<w:pPr><w:ind w:left=\"" + num(qint64(l.left)) + "\" w:hanging=\"" + num(qint64(std::max(0.0, l.hanging))) + "\"/></w:pPr>";
            x += "</w:lvl>";
        }
        x += "</w:abstractNum>";
    }
    for (int i = 0; i < m_nums.size(); ++i) {
        x += "<w:num w:numId=\"" + num(i + 1) + "\"><w:abstractNumId w:val=\"" + num(m_nums[i].abstractId) + "\"/>";
        if (m_nums[i].level >= 0) x += "<w:lvlOverride w:ilvl=\"" + num(std::min(m_nums[i].level, 8)) + "\"><w:startOverride w:val=\"" + num(m_nums[i].start) + "\"/></w:lvlOverride>";
        else if (m_nums[i].restart) {
            const ListStyleDef def = m_listStyles.value(m_nums[i].listStyle);
            for (int lv = 0; lv < 9; ++lv) x += "<w:lvlOverride w:ilvl=\"" + num(lv) + "\"><w:startOverride w:val=\"" + num(def.level[lv].defined ? def.level[lv].start : 1) + "\"/></w:lvlOverride>";
        }
        x += "</w:num>";
    }
    x += "</w:numbering>";
    return x;
}

} // namespace jp::office::odt
