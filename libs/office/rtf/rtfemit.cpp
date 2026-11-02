#include "rtf/rtfmodel.h"

#include "doc/docprops.h"

#include <QSet>
#include <algorithm>

namespace jp::office::rtf {

const char *const kBorderNames[] = {"nil", "single", "thick", "double", "dotted", "dashed", "wave", "triple", "dotDash", "dotDotDash", "inset", "outset"};
const char *const kUnderlineNames[] = {"none", "single", "double", "dotted", "words", "thick", "wave", "dash", "dotDash", "dotDotDash", "dottedHeavy", "dashedHeavy", "wavyDouble", "dashLong"};

namespace {

using doc::xmlEscape;

QByteArray num(qint64 v) { return QByteArray::number(v); }

void toggle(QByteArray &x, const char *tag, int v)
{
    if (v == kUnset) return;
    x += QByteArray("<w:") + tag + (v ? "/>" : " w:val=\"0\"/>");
}

const char *languageTag(int lcid)
{
    switch (lcid) {
    case 1033: return "en-US";
    case 2057: return "en-GB";
    case 3081: return "en-AU";
    case 4105: return "en-CA";
    case 1031: return "de-DE";
    case 1036: return "fr-FR";
    case 1034: case 3082: return "es-ES";
    case 1040: return "it-IT";
    case 1043: return "nl-NL";
    case 1046: return "pt-BR";
    case 2070: return "pt-PT";
    case 1053: return "sv-SE";
    case 1030: return "da-DK";
    case 1044: return "nb-NO";
    case 1035: return "fi-FI";
    case 1049: return "ru-RU";
    case 1045: return "pl-PL";
    case 1029: return "cs-CZ";
    case 1038: return "hu-HU";
    case 1041: return "ja-JP";
    case 1042: return "ko-KR";
    case 2052: return "zh-CN";
    case 1028: return "zh-TW";
    case 1081: return "hi-IN";
    default: return nullptr;
    }
}

struct Highlight { const char *name; qint32 rgb; };
const Highlight kHighlights[] = {{"black", 0x000000}, {"blue", 0x0000FF}, {"cyan", 0x00FFFF}, {"green", 0x00FF00}, {"magenta", 0xFF00FF}, {"red", 0xFF0000},
                                 {"yellow", 0xFFFF00}, {"white", 0xFFFFFF}, {"darkBlue", 0x000080}, {"darkCyan", 0x008080}, {"darkGreen", 0x008000},
                                 {"darkMagenta", 0x800080}, {"darkRed", 0x800000}, {"darkYellow", 0x808000}, {"darkGray", 0x808080}, {"lightGray", 0xC0C0C0}};

QByteArray fontName(const Tables &t, int font)
{
    const auto it = t.fontAt.constFind(font);
    return it == t.fontAt.constEnd() ? QByteArray() : xmlEscape(t.fonts[it.value()].name);
}

QByteArray fontsXml(const Tables &t, int font)
{
    const QByteArray n = fontName(t, font);
    if (n.isEmpty()) return QByteArray();
    return "<w:rFonts w:ascii=\"" + n + "\" w:hAnsi=\"" + n + "\" w:eastAsia=\"" + n + "\" w:cs=\"" + n + "\"/>";
}

} // namespace

int borderStyleOf(const char *word)
{
    const QByteArray w(word);
    if (w == "brdrs" || w == "brdrhair") return 1;
    if (w == "brdrth") return 2;
    if (w == "brdrdb") return 3;
    if (w == "brdrdot") return 4;
    if (w == "brdrdash" || w == "brdrdashsm") return 5;
    if (w == "brdrwavy" || w == "brdrwavydb") return 6;
    if (w == "brdrtriple") return 7;
    if (w == "brdrdashd") return 8;
    if (w == "brdrdashdd") return 9;
    if (w == "brdrinset") return 10;
    if (w == "brdroutset") return 11;
    if (w == "brdrnone" || w == "brdrnil") return 0;
    return 1;
}

QString colorHex(const Tables &t, int index)
{
    if (index < 0 || index >= t.colors.size() || t.colors[index] < 0) return QStringLiteral("auto");
    return QStringLiteral("%1").arg(t.colors[index] & 0xFFFFFF, 6, 16, QLatin1Char('0')).toUpper();
}

QByteArray borderXml(const char *side, const Border &b, const Tables &t)
{
    if (!b.stated()) return QByteArray();
    const int style = std::clamp(b.style, 0, 11);
    if (style == 0) return QByteArray("<w:") + side + " w:val=\"nil\"/>";
    // (The width in eighths of a point, 2 to 96.)
    const int sz = std::clamp(b.width * 2 / 5, 2, 96);
    return QByteArray("<w:") + side + " w:val=\"" + kBorderNames[style] + "\" w:sz=\"" + num(sz) + "\" w:space=\"" + num(std::clamp(b.space / 20, 0, 31)) + "\" w:color=\"" +
           colorHex(t, b.color).toLatin1() + "\"/>";
}

QByteArray runPropertiesXml(const Chr &c, const Tables &t)
{
    QByteArray x;
    if (c.style != kUnset) {
        const auto it = t.charStyleAt.constFind(c.style);
        if (it != t.charStyleAt.constEnd()) x += "<w:rStyle w:val=\"" + xmlEscape(t.styles[it.value()].docxId) + "\"/>";
    }
    if (c.font != kUnset) x += fontsXml(t, c.font);
    toggle(x, "b", c.bold);
    toggle(x, "bCs", c.bold);
    toggle(x, "i", c.italic);
    toggle(x, "iCs", c.italic);
    toggle(x, "caps", c.caps);
    toggle(x, "smallCaps", c.scaps);
    toggle(x, "strike", c.strike);
    toggle(x, "dstrike", c.dstrike);
    toggle(x, "outline", c.outline);
    toggle(x, "shadow", c.shadow);
    toggle(x, "emboss", c.emboss);
    toggle(x, "imprint", c.imprint);
    toggle(x, "vanish", c.hidden);
    if (c.color != kUnset) x += "<w:color w:val=\"" + colorHex(t, c.color).toLatin1() + "\"/>";
    if (c.spacing != kUnset) x += "<w:spacing w:val=\"" + num(std::clamp(c.spacing, -31680, 31680)) + "\"/>";
    if (c.scale != kUnset && c.scale >= 1 && c.scale <= 600) x += "<w:w w:val=\"" + num(c.scale) + "\"/>";
    if (c.pos != kUnset) x += "<w:position w:val=\"" + num(std::clamp(c.pos, -3168, 3168)) + "\"/>";
    if (c.size != kUnset && c.size > 0) x += "<w:sz w:val=\"" + num(std::min(c.size, 3276)) + "\"/><w:szCs w:val=\"" + num(std::min(c.size, 3276)) + "\"/>";
    bool shaded = false;
    if (c.hl != kUnset && c.hl > 0 && c.hl < t.colors.size() && t.colors[c.hl] >= 0) {
        const qint32 rgb = t.colors[c.hl] & 0xFFFFFF;
        const char *name = nullptr;
        for (const Highlight &h : kHighlights)
            if (h.rgb == rgb) name = h.name;
        if (name) x += QByteArray("<w:highlight w:val=\"") + name + "\"/>";
        else {
            x += "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + colorHex(t, c.hl).toLatin1() + "\"/>";
            shaded = true;
        }
    }
    if (c.ul != kUnset) {
        const int u = std::clamp(c.ul, 0, 13);
        x += QByteArray("<w:u w:val=\"") + kUnderlineNames[u] + "\"" + (c.ulColor != kUnset && u != 0 ? " w:color=\"" + colorHex(t, c.ulColor).toLatin1() + "\"" : QByteArray()) + "/>";
    }
    if (!shaded && c.shd != kUnset && c.shd > 0) x += "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + colorHex(t, c.shd).toLatin1() + "\"/>";
    if (c.vert != kUnset) x += c.vert == 1 ? "<w:vertAlign w:val=\"superscript\"/>" : c.vert == 2 ? "<w:vertAlign w:val=\"subscript\"/>" : "<w:vertAlign w:val=\"baseline\"/>";
    if (c.lang != kUnset) {
        if (const char *tag = languageTag(c.lang)) x += QByteArray("<w:lang w:val=\"") + tag + "\"/>";
    }
    return x;
}

QByteArray paragraphPropertiesXml(const Par &p, const Tables &t, const QString &styleId, bool inStyle)
{
    QByteArray x;
    if (!inStyle && !styleId.isEmpty()) x += "<w:pStyle w:val=\"" + xmlEscape(styleId) + "\"/>";
    toggle(x, "keepNext", p.keepn);
    toggle(x, "keepLines", p.keep);
    toggle(x, "pageBreakBefore", p.pagebb);
    toggle(x, "widowControl", p.widow);
    if (p.ls != kUnset) {
        const auto it = t.numAt.constFind(p.ls);
        if (p.ls == 0) x += "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"0\"/></w:numPr>";
        else if (it != t.numAt.constEnd())
            x += "<w:numPr><w:ilvl w:val=\"" + num(std::clamp(p.ilvl == kUnset ? 0 : p.ilvl, 0, 8)) + "\"/><w:numId w:val=\"" + num(it.value()) + "\"/></w:numPr>";
    }
    {
        QByteArray b;
        static const char *const sides[] = {"top", "left", "bottom", "right", "between"};
        for (int i = 0; i < 5; ++i) b += borderXml(sides[i], p.border[i], t);
        if (!b.isEmpty()) x += "<w:pBdr>" + b + "</w:pBdr>";
    }
    if (p.shd != kUnset && p.shd > 0) x += "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + colorHex(t, p.shd).toLatin1() + "\"/>";
    if (!p.tabs.isEmpty()) {
        QByteArray tabs;
        static const char *const types[] = {"left", "center", "right", "decimal", "bar"};
        static const char *const leaders[] = {"none", "dot", "hyphen", "underscore", "heavy", "middleDot"};
        for (const Tab &tab : p.tabs)
            tabs += QByteArray("<w:tab w:val=\"") + types[std::clamp(tab.type, 0, 4)] + "\" w:leader=\"" + leaders[std::clamp(tab.leader, 0, 5)] + "\" w:pos=\"" + num(tab.pos) + "\"/>";
        x += "<w:tabs>" + tabs + "</w:tabs>";
    }
    toggle(x, "suppressAutoHyphens", p.noHyph);
    if (p.sb != kUnset || p.sa != kUnset || p.sl != kUnset) {
        QByteArray s = "<w:spacing";
        if (p.sb != kUnset) s += " w:before=\"" + num(std::clamp(p.sb, 0, 31680)) + "\"";
        if (p.sa != kUnset) s += " w:after=\"" + num(std::clamp(p.sa, 0, 31680)) + "\"";
        if (p.sl != kUnset && p.sl != 0) {
            const bool multiple = p.slmult == 1;
            const int v = std::abs(p.sl);
            s += " w:line=\"" + num(std::min(v, 31680)) + "\" w:lineRule=\"" + (multiple ? "auto" : p.sl < 0 ? "exact" : "atLeast") + "\"";
        } else if (p.sl == 0) {
            s += " w:line=\"240\" w:lineRule=\"auto\"";
        }
        x += s + "/>";
    }
    if (p.li != kUnset || p.ri != kUnset || p.fi != kUnset) {
        QByteArray s = "<w:ind";
        if (p.li != kUnset) s += " w:left=\"" + num(std::clamp(p.li, -31680, 31680)) + "\"";
        if (p.ri != kUnset) s += " w:right=\"" + num(std::clamp(p.ri, -31680, 31680)) + "\"";
        if (p.fi != kUnset) s += p.fi < 0 ? " w:hanging=\"" + num(std::min(-p.fi, 31680)) + "\"" : " w:firstLine=\"" + num(std::min(p.fi, 31680)) + "\"";
        x += s + "/>";
    }
    toggle(x, "contextualSpacing", p.contextual);
    if (p.align != kUnset) {
        static const char *const jc[] = {"left", "center", "right", "both"};
        x += QByteArray("<w:jc w:val=\"") + jc[std::clamp(p.align, 0, 3)] + "\"/>";
    }
    if (p.outline != kUnset && p.outline >= 0 && p.outline <= 8) x += "<w:outlineLvl w:val=\"" + num(p.outline) + "\"/>";
    return x;
}

QByteArray sectionPropertiesXml(const Section &s, const QByteArray &refs)
{
    QByteArray x = refs;
    if (s.type != kUnset) {
        static const char *const types[] = {"continuous", "nextColumn", "nextPage", "evenPage", "oddPage"};
        x += QByteArray("<w:type w:val=\"") + types[std::clamp(s.type, 0, 4)] + "\"/>";
    }
    const int w = std::clamp(s.pageW, 240, 31680), h = std::clamp(s.pageH, 240, 31680);
    x += "<w:pgSz w:w=\"" + num(w) + "\" w:h=\"" + num(h) + "\"" + (s.landscape ? " w:orient=\"landscape\"" : "") + "/>";
    auto m = [](int v) { return num(std::clamp(v, 0, 31680)); };
    x += "<w:pgMar w:top=\"" + num(std::clamp(s.top, -31680, 31680)) + "\" w:right=\"" + m(s.right) + "\" w:bottom=\"" + num(std::clamp(s.bottom, -31680, 31680)) + "\" w:left=\"" + m(s.left) +
         "\" w:header=\"" + m(s.header) + "\" w:footer=\"" + m(s.footer) + "\" w:gutter=\"" + m(s.gutter) + "\"/>";
    if (s.pageStart != kUnset || s.pageFormat != kUnset) {
        static const char *const fmts[] = {"decimal", "upperRoman", "lowerRoman", "upperLetter", "lowerLetter"};
        x += "<w:pgNumType";
        if (s.pageFormat != kUnset) x += QByteArray(" w:fmt=\"") + fmts[std::clamp(s.pageFormat, 0, 4)] + "\"";
        if (s.pageStart != kUnset) x += " w:start=\"" + num(std::clamp(s.pageStart, 0, 32767)) + "\"";
        x += "/>";
    }
    if (s.cols > 1) {
        const int n = std::min(s.cols, 45);
        x += "<w:cols w:num=\"" + num(n) + "\" w:space=\"" + m(s.colSpace) + "\"" + (s.lineBetween ? " w:sep=\"1\"" : "");
        if (s.colWidth.size() >= n) {
            x += " w:equalWidth=\"0\">";
            for (int i = 0; i < n; ++i) x += "<w:col w:w=\"" + m(s.colWidth[i]) + "\"" + (i + 1 < n && i < s.colGap.size() ? " w:space=\"" + m(s.colGap[i]) + "\"" : QByteArray()) + "/>";
            x += "</w:cols>";
        } else {
            x += "/>";
        }
    }
    if (s.vAlign) x += s.vAlign == 1 ? "<w:vAlign w:val=\"center\"/>" : s.vAlign == 2 ? "<w:vAlign w:val=\"bottom\"/>" : "<w:vAlign w:val=\"both\"/>";
    if (s.titlePage) x += "<w:titlePg/>";
    return x;
}

QByteArray fontTableXml(const Tables &t)
{
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<w:fonts xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">";
    static const char *const families[] = {"auto", "roman", "swiss", "modern", "script", "decorative", "auto", "auto"};
    for (const Font &f : t.fonts) {
        if (f.name.isEmpty()) continue;
        x += "<w:font w:name=\"" + xmlEscape(f.name) + "\">";
        x += QByteArray("<w:charset w:val=\"") + QByteArray::number(f.charset & 0xFF, 16).rightJustified(2, '0') + "\"/>";
        x += QByteArray("<w:family w:val=\"") + families[f.family & 7] + "\"/>";
        x += f.prq == 1 ? "<w:pitch w:val=\"fixed\"/>" : f.prq == 2 ? "<w:pitch w:val=\"variable\"/>" : "";
        x += "</w:font>";
    }
    return x + "</w:fonts>";
}

QByteArray stylesXml(const Tables &t)
{
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                   "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    Chr d;
    d.font = t.defaultFont;
    d.size = t.defaultSize;
    d.lang = t.defaultLang;
    x += "<w:docDefaults><w:rPrDefault><w:rPr>" + runPropertiesXml(d, t) + "</w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults>";
    bool haveNormal = false;
    for (int i = 0; i < t.styles.size(); ++i) {
        const StyleDef &s = t.styles[i];
        if (s.kind == 2) continue;
        const bool para = s.kind == 0;
        const bool isDefault = para && s.id == 0;
        haveNormal = haveNormal || isDefault;
        x += QByteArray("<w:style w:type=\"") + (para ? "paragraph" : "character") + "\"" + (isDefault ? " w:default=\"1\"" : "") + " w:styleId=\"" + xmlEscape(s.docxId) + "\"><w:name w:val=\"" +
             xmlEscape(s.name.isEmpty() ? s.docxId : s.name) + "\"/>";
        // (Based on another of its kind only where the chain of them comes to an end within the limit.)
        if (s.basedOn != kUnset && s.basedOn != s.id) {
            const auto &index = para ? t.paraStyleAt : t.charStyleAt;
            const auto found = index.constFind(s.basedOn);
            if (found != index.constEnd()) {
                int at = s.basedOn;
                bool ok = true;
                for (int steps = 0;; ++steps) {
                    if (steps > 40 || at == s.id) {
                        ok = false;
                        break;
                    }
                    const auto it = index.constFind(at);
                    if (it == index.constEnd()) break;
                    const int up = t.styles[it.value()].basedOn;
                    if (up == kUnset) break;
                    at = up;
                }
                if (ok) x += "<w:basedOn w:val=\"" + xmlEscape(t.styles[found.value()].docxId) + "\"/>";
            }
        }
        if (para && s.next != kUnset && s.next != s.id) {
            const auto found = t.paraStyleAt.constFind(s.next);
            if (found != t.paraStyleAt.constEnd()) x += "<w:next w:val=\"" + xmlEscape(t.styles[found.value()].docxId) + "\"/>";
        }
        if (s.hidden) x += "<w:semiHidden/>";
        x += "<w:qFormat/>";
        if (para) {
            const QByteArray p = paragraphPropertiesXml(s.par, t, QString(), true);
            if (!p.isEmpty()) x += "<w:pPr>" + p + "</w:pPr>";
        }
        Chr c = s.chr;
        c.style = kUnset;
        const QByteArray r = runPropertiesXml(c, t);
        if (!r.isEmpty()) x += "<w:rPr>" + r + "</w:rPr>";
        x += "</w:style>";
    }
    if (!haveNormal) x += "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>";
    return x + "</w:styles>";
}

namespace {

QByteArray levelXml(const ListLevel &l, int ilvl, const Tables &t)
{
    static const char *const formats[] = {"decimal", "upperRoman", "lowerRoman", "upperLetter", "lowerLetter", "ordinal", "cardinalText", "ordinalText"};
    QByteArray x = "<w:lvl w:ilvl=\"" + num(ilvl) + "\"><w:start w:val=\"" + num(std::clamp(l.start, 0, 32767)) + "\"/>";
    const char *fmt = l.nfc >= 0 && l.nfc <= 7 ? formats[l.nfc] : l.nfc == 22 ? "decimalZero" : l.nfc == 23 ? "bullet" : l.nfc == 255 ? "none" : l.nfc == 18 ? "decimalEnclosedCircle" : "decimal";
    x += QByteArray("<w:numFmt w:val=\"") + fmt + "\"/>";
    if (l.noRestart) x += "<w:lvlRestart w:val=\"0\"/>";
    if (l.legal) x += "<w:isLgl/>";
    if (l.follow == 1) x += "<w:suff w:val=\"space\"/>";
    else if (l.follow == 2) x += "<w:suff w:val=\"nothing\"/>";
    x += "<w:lvlText w:val=\"" + xmlEscape(l.lvlText) + "\"/>";
    x += QByteArray("<w:lvlJc w:val=\"") + (l.jc == 1 ? "center" : l.jc == 2 ? "right" : "left") + "\"/>";
    if (l.li != kUnset || l.fi != kUnset) {
        const int fi = l.fi == kUnset ? 0 : l.fi;
        x += "<w:pPr><w:ind w:left=\"" + num(std::clamp(l.li == kUnset ? 0 : l.li, -31680, 31680)) + "\"" + (fi < 0 ? " w:hanging=\"" + num(std::min(-fi, 31680)) + "\"" : " w:firstLine=\"" + num(std::min(fi, 31680)) + "\"") + "/></w:pPr>";
    }
    Chr c = l.chr;
    c.style = kUnset;
    const QByteArray r = runPropertiesXml(c, t);
    if (!r.isEmpty()) x += "<w:rPr>" + r + "</w:rPr>";
    return x + "</w:lvl>";
}

} // namespace

QByteArray numberingXml(const Tables &t)
{
    if (t.lists.isEmpty() || t.nums.isEmpty()) return QByteArray();
    QByteArray x = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<w:numbering xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
                   "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    for (int i = 0; i < t.lists.size(); ++i) {
        const ListDef &l = t.lists[i];
        x += "<w:abstractNum w:abstractNumId=\"" + num(i) + "\"><w:multiLevelType w:val=\"" + (l.simple ? "singleLevel" : "multilevel") + "\"/>";
        for (int k = 0; k < l.levels.size() && k < 9; ++k)
            if (l.levels[k].present) x += levelXml(l.levels[k], k, t);
        x += "</w:abstractNum>";
    }
    for (int i = 0; i < t.nums.size(); ++i) {
        const Tables::Num &n = t.nums[i];
        x += "<w:num w:numId=\"" + num(i + 1) + "\"><w:abstractNumId w:val=\"" + num(n.list) + "\"/>";
        for (int k = 0; k < n.overrides.size() && k < 9; ++k)
            if (n.overrides[k].present) x += "<w:lvlOverride w:ilvl=\"" + num(k) + "\"><w:startOverride w:val=\"" + num(std::clamp(n.overrides[k].start, 0, 32767)) + "\"/></w:lvlOverride>";
        x += "</w:num>";
    }
    return x + "</w:numbering>";
}

} // namespace jp::office::rtf
