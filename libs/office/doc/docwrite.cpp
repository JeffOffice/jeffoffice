// The property sets of the old binary format, written as the XML of a .docx: run properties, paragraph properties, and section properties.

#include "doc/docprops.h"

#include <QDate>
#include <QStringList>
#include <algorithm>

namespace jp::office::doc {

QByteArray xmlEscape(const QString &s)
{
    QByteArray out;
    out.reserve(s.size() + 16);
    // (The text between the marks that need changing goes in whole.)
    qsizetype from = 0;
    auto flush = [&](qsizetype to) {
        if (to > from) out += QStringView(s).mid(from, to - from).toUtf8();
    };
    for (qsizetype i = 0; i < s.size(); ++i) {
        const ushort u = s[i].unicode();
        if (u >= 0x20 && u < 0xD800 && u != '&' && u != '<' && u != '>' && u != '"') continue;
        const char *replacement = nullptr;
        if (u == '&') replacement = "&amp;";
        else if (u == '<') replacement = "&lt;";
        else if (u == '>') replacement = "&gt;";
        else if (u == '"') replacement = "&quot;";
        else if (u < 0x20 && u != 0x09 && u != 0x0A && u != 0x0D) replacement = "";
        else if (u == 0xFFFE || u == 0xFFFF) replacement = "";
        else if (QChar::isHighSurrogate(u) && !(i + 1 < s.size() && QChar::isLowSurrogate(s[i + 1].unicode()))) replacement = "";
        else if (QChar::isLowSurrogate(u) && !(i > 0 && QChar::isHighSurrogate(s[i - 1].unicode()))) replacement = "";
        else continue;
        flush(i);
        from = i + 1;
        out += replacement;
    }
    flush(s.size());
    return out;
}

QByteArray revisionAttributes(int id, const QString &author, quint32 date)
{
    QByteArray x = " w:id=\"" + QByteArray::number(id) + "\" w:author=\"" + xmlEscape(author) + "\"";
    if (date) {
        // Minutes in the low six bits, then hours (5), day (5), month (4), and the years since 1900 (9).
        const int minute = int(date & 0x3F), hour = int((date >> 6) & 0x1F), day = int((date >> 11) & 0x1F), month = int((date >> 16) & 0xF), year = 1900 + int((date >> 20) & 0x1FF);
        if (QDate::isValid(year, month, day) && hour < 24 && minute < 60) x += QString::asprintf(" w:date=\"%04d-%02d-%02dT%02d:%02d:00Z\"", year, month, day, hour, minute).toLatin1();
    }
    return x;
}

namespace {

void attr(QByteArray &x, const char *name, const QString &value)
{
    x += ' ';
    x += name;
    x += "=\"";
    x += xmlEscape(value);
    x += '"';
}
void attr(QByteArray &x, const char *name, int value)
{
    x += ' ';
    x += name;
    x += "=\"";
    x += QByteArray::number(value);
    x += '"';
}

const char *borderName(int type)
{
    switch (type) {
    case 0: return "none";
    case 1: case 5: return "single";
    case 2: return "thick";
    case 3: return "double";
    case 6: return "dotted";
    case 7: return "dashed";
    case 8: return "dotDash";
    case 9: return "dotDotDash";
    case 10: return "triple";
    case 11: return "thinThickSmallGap";
    case 12: return "thickThinSmallGap";
    case 13: return "thinThickThinSmallGap";
    case 14: return "thinThickMediumGap";
    case 15: return "thickThinMediumGap";
    case 16: return "thinThickThinMediumGap";
    case 17: return "thinThickLargeGap";
    case 18: return "thickThinLargeGap";
    case 19: return "thinThickThinLargeGap";
    case 20: return "wave";
    case 21: return "doubleWave";
    case 22: return "dashSmallGap";
    case 23: return "dashDotStroked";
    case 24: return "threeDEmboss";
    case 25: return "threeDEngrave";
    case 26: return "outset";
    case 27: return "inset";
    default: return "single";
    }
}

const char *patternName(int p)
{
    switch (p) {
    case 0: return "clear";
    case 1: return "solid";
    case 2: return "pct5";
    case 3: return "pct10";
    case 4: return "pct20";
    case 5: return "pct25";
    case 6: return "pct30";
    case 7: return "pct40";
    case 8: return "pct50";
    case 9: return "pct60";
    case 10: return "pct70";
    case 11: return "pct75";
    case 12: return "pct80";
    case 13: return "pct90";
    case 14: return "horzStripe";
    case 15: return "vertStripe";
    case 16: return "reverseDiagStripe";
    case 17: return "diagStripe";
    case 18: return "horzCross";
    case 19: return "diagCross";
    case 20: return "thinHorzStripe";
    case 21: return "thinVertStripe";
    case 22: return "thinReverseDiagStripe";
    case 23: return "thinDiagStripe";
    case 24: return "thinHorzCross";
    case 25: return "thinDiagCross";
    case 0x2B: return "pct5";
    case 0x2C: return "pct10";
    case 0x2D: return "pct10";
    case 0x2E: return "pct10";
    default: return p >= 0x30 && p <= 0x3E ? "pct90" : "clear";
    }
}

const char *underlineName(int kul)
{
    switch (kul) {
    case 1: return "single";
    case 2: return "words";
    case 3: return "double";
    case 4: return "dotted";
    case 5: return "thick";
    case 6: return "dash";
    case 7: return "dotDash";
    case 8: return "dotDotDash";
    case 9: return "wave";
    case 20: return "dottedHeavy";
    case 21: return "dashedHeavy";
    case 23: return "dashDotHeavy";
    case 25: return "dashDotDotHeavy";
    case 26: return "dashLongHeavy";
    case 27: return "dashLong";
    case 39: return "wavyHeavy";
    case 43: return "wavyDouble";
    default: return "single";
    }
}

const char *highlightName(int ico)
{
    static const char *const names[17] = {"none", "black", "blue", "cyan", "green", "magenta", "red", "yellow", "white", "darkBlue", "darkCyan", "darkGreen", "darkMagenta", "darkRed", "darkYellow", "darkGray", "lightGray"};
    return ico >= 0 && ico <= 16 ? names[ico] : "none";
}

} // namespace

const char *numberFormatName(int nfc)
{
    switch (nfc) {
    case 0: return "decimal";
    case 1: return "upperRoman";
    case 2: return "lowerRoman";
    case 3: return "upperLetter";
    case 4: return "lowerLetter";
    case 5: return "ordinal";
    case 6: return "cardinalText";
    case 7: return "ordinalText";
    case 8: return "hex";
    case 9: return "chicago";
    case 22: return "decimalZero";
    case 23: return "bullet";
    case 255: return "none";
    default: return "decimal";
    }
}

QByteArray borderXml(const char *side, const Brc &b)
{
    if (!b.stated) return QByteArray();
    QByteArray x = "<w:";
    x += side;
    if (b.nil || (b.type == 0 && b.width == 0)) {
        attr(x, "w:val", QStringLiteral("nil"));
    } else {
        attr(x, "w:val", QString::fromLatin1(borderName(b.type)));
        attr(x, "w:sz", std::clamp(b.width, 2, 96));
        attr(x, "w:space", std::clamp(b.space, 0, 31));
        attr(x, "w:color", hexColor(b.color));
        if (b.shadow) attr(x, "w:shadow", QStringLiteral("1"));
        if (b.frame) attr(x, "w:frame", QStringLiteral("1"));
    }
    x += "/>";
    return x;
}

QByteArray shadingXml(const Shd &s)
{
    // (No pattern and no colors is no shading.)
    if (!s.stated || (s.pattern == 0 && s.fore == -1 && s.back == -1)) return QByteArray();
    QByteArray x = "<w:shd";
    attr(x, "w:val", QString::fromLatin1(patternName(s.pattern)));
    attr(x, "w:color", hexColor(s.fore));
    attr(x, "w:fill", hexColor(s.back));
    x += "/>";
    return x;
}

QByteArray runPropertiesXml(const Chp &c, const FontNames &fonts, const quint8 *parent, const QString &charStyle)
{
    QByteArray x;
    // (The elements are in the order the schema gives.)
    if (!charStyle.isEmpty()) {
        x += "<w:rStyle";
        attr(x, "w:val", charStyle);
        x += "/>";
    }
    {
        const QString ascii = c.ftcAscii ? fonts.name(*c.ftcAscii) : QString(), other = c.ftcOther ? fonts.name(*c.ftcOther) : QString();
        const QString fe = c.ftcFE ? fonts.name(*c.ftcFE) : QString(), bi = c.ftcBi ? fonts.name(*c.ftcBi) : QString();
        if (!ascii.isEmpty() || !other.isEmpty() || !fe.isEmpty() || !bi.isEmpty()) {
            x += "<w:rFonts";
            if (!ascii.isEmpty()) attr(x, "w:ascii", ascii);
            if (!other.isEmpty()) attr(x, "w:hAnsi", other);
            if (!fe.isEmpty()) attr(x, "w:eastAsia", fe);
            if (!bi.isEmpty()) attr(x, "w:cs", bi);
            x += "/>";
        }
    }
    // A toggle: nothing if it leaves the style's value as it is, else the value (on, or off with val 0).
    auto toggle = [&](Toggle t, const char *element) {
        if (!c.tog[t]) return;
        const quint8 inherited = parent ? parent[t] : 0;
        const quint8 now = settleToggle(*c.tog[t], inherited);
        if (now == inherited) return;
        x += "<w:";
        x += element;
        if (!now) x += " w:val=\"0\"";
        x += "/>";
    };
    toggle(TogBold, "b");
    toggle(TogBoldBi, "bCs");
    toggle(TogItalic, "i");
    toggle(TogItalicBi, "iCs");
    toggle(TogCaps, "caps");
    toggle(TogSmallCaps, "smallCaps");
    toggle(TogStrike, "strike");
    toggle(TogDStrike, "dstrike");
    toggle(TogOutline, "outline");
    toggle(TogShadow, "shadow");
    toggle(TogEmboss, "emboss");
    toggle(TogImprint, "imprint");
    if (c.noProof && *c.noProof) x += "<w:noProof/>";
    toggle(TogVanish, "vanish");
    if (c.webHidden && *c.webHidden) x += "<w:webHidden/>";
    if (c.color) {
        x += "<w:color";
        attr(x, "w:val", hexColor(*c.color));
        x += "/>";
    }
    if (c.dxaSpace) {
        x += "<w:spacing";
        attr(x, "w:val", std::clamp(*c.dxaSpace, -31680, 31680));
        x += "/>";
    }
    if (c.scale && *c.scale >= 1 && *c.scale <= 600) {
        x += "<w:w";
        attr(x, "w:val", *c.scale);
        x += "/>";
    }
    if (c.hpsKern) {
        x += "<w:kern";
        attr(x, "w:val", std::min(*c.hpsKern, 3277));
        x += "/>";
    }
    if (c.hpsPos) {
        x += "<w:position";
        attr(x, "w:val", std::clamp(*c.hpsPos, -3168, 3168));
        x += "/>";
    }
    if (c.hps && *c.hps > 0) {
        x += "<w:sz";
        attr(x, "w:val", std::min(*c.hps, 3276));
        x += "/>";
    }
    if (c.hpsBi && *c.hpsBi > 0) {
        x += "<w:szCs";
        attr(x, "w:val", std::min(*c.hpsBi, 3276));
        x += "/>";
    }
    if (c.highlight && *c.highlight >= 1 && *c.highlight <= 16) {
        x += "<w:highlight";
        attr(x, "w:val", QString::fromLatin1(highlightName(*c.highlight)));
        x += "/>";
    }
    if (c.kul) {
        x += "<w:u";
        attr(x, "w:val", *c.kul == 0 ? QStringLiteral("none") : QString::fromLatin1(underlineName(*c.kul)));
        if (c.underlineColor && *c.kul != 0) attr(x, "w:color", hexColor(*c.underlineColor));
        x += "/>";
    }
    x += borderXml("bdr", c.brc);
    x += shadingXml(c.shd);
    if (c.iss) {
        x += "<w:vertAlign";
        attr(x, "w:val", *c.iss == 1 ? QStringLiteral("superscript") : *c.iss == 2 ? QStringLiteral("subscript") : QStringLiteral("baseline"));
        x += "/>";
    }
    if (c.bidi && *c.bidi) x += "<w:rtl/>";
    if (c.complexScripts && *c.complexScripts) x += "<w:cs/>";
    return x;
}

QByteArray paragraphPropertiesXml(const Pap &p, const ParaContext &ctx)
{
    QByteArray x;
    if (!ctx.styleId.isEmpty()) {
        x += "<w:pStyle";
        attr(x, "w:val", ctx.styleId);
        x += "/>";
    }
    auto flag = [&](const std::optional<bool> &v, const char *element) {
        if (!v) return;
        x += "<w:";
        x += element;
        if (!*v) x += " w:val=\"0\"";
        x += "/>";
    };
    flag(p.keepNext, "keepNext");
    flag(p.keep, "keepLines");
    flag(p.pageBreakBefore, "pageBreakBefore");
    if (p.frame.stated && !p.inTable) {
        // A paragraph set in a frame: its anchors and place (special places are named), its width and height, and the text's wrap.
        const int pc = p.frame.anchors;
        const int vert = (pc >> 4) & 3, horz = (pc >> 6) & 3;
        x += "<w:framePr";
        if (p.frame.w > 0) attr(x, "w:w", p.frame.w);
        if (p.frame.hStated && (p.frame.h & 0x7FFF) > 0) {
            attr(x, "w:h", p.frame.h & 0x7FFF);
            if (p.frame.h & 0x8000) attr(x, "w:hRule", QStringLiteral("atLeast"));
        }
        if (p.frame.distY) attr(x, "w:vSpace", p.frame.distY);
        if (p.frame.distX) attr(x, "w:hSpace", p.frame.distX);
        attr(x, "w:wrap", p.frame.wrap == 1 ? QStringLiteral("none") : p.frame.wrap == 2 ? QStringLiteral("around") : p.frame.wrap == 3 ? QStringLiteral("notBeside") : p.frame.wrap == 4 ? QStringLiteral("through") : QStringLiteral("auto"));
        attr(x, "w:vAnchor", vert == 0 ? QStringLiteral("margin") : vert == 1 ? QStringLiteral("page") : QStringLiteral("text"));
        attr(x, "w:hAnchor", horz == 0 ? QStringLiteral("text") : horz == 1 ? QStringLiteral("margin") : QStringLiteral("page"));
        if (p.frame.xStated) {
            switch (p.frame.x) {
            case 0: attr(x, "w:xAlign", QStringLiteral("left")); break;
            case -4: attr(x, "w:xAlign", QStringLiteral("center")); break;
            case -8: attr(x, "w:xAlign", QStringLiteral("right")); break;
            case -12: attr(x, "w:xAlign", QStringLiteral("inside")); break;
            case -16: attr(x, "w:xAlign", QStringLiteral("outside")); break;
            default: attr(x, "w:x", p.frame.x); break;
            }
        }
        if (p.frame.yStated) {
            switch (p.frame.y) {
            case 0: break;
            case -4: attr(x, "w:yAlign", QStringLiteral("top")); break;
            case -8: attr(x, "w:yAlign", QStringLiteral("center")); break;
            case -12: attr(x, "w:yAlign", QStringLiteral("bottom")); break;
            case -16: attr(x, "w:yAlign", QStringLiteral("inside")); break;
            case -20: attr(x, "w:yAlign", QStringLiteral("outside")); break;
            default: attr(x, "w:y", p.frame.y); break;
            }
        }
        x += "/>";
    }
    flag(p.widow, "widowControl");
    if (p.ilfo && *p.ilfo > 0 && *p.ilfo <= ctx.listInstances) {
        x += "<w:numPr><w:ilvl";
        attr(x, "w:val", std::clamp(p.ilvl.value_or(0), 0, 8));
        x += "/><w:numId";
        attr(x, "w:val", *p.ilfo);
        x += "/></w:numPr>";
    } else if (p.ilfo && *p.ilfo == 0) {
        x += "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"0\"/></w:numPr>";
    }
    flag(p.noLineNumber, "suppressLineNumbers");
    {
        const QByteArray top = borderXml("top", p.brcTop), left = borderXml("left", p.brcLeft), bottom = borderXml("bottom", p.brcBottom), right = borderXml("right", p.brcRight);
        const QByteArray between = borderXml("between", p.brcBetween), bar = borderXml("bar", p.brcBar);
        if (!top.isEmpty() || !left.isEmpty() || !bottom.isEmpty() || !right.isEmpty() || !between.isEmpty() || !bar.isEmpty())
            x += "<w:pBdr>" + top + left + bottom + right + between + bar + "</w:pBdr>";
    }
    x += shadingXml(p.shd);
    if (p.tabsStated && (!p.tabsCleared.isEmpty() || !p.tabsAdded.isEmpty())) {
        static const char *const kinds[] = {"left", "center", "right", "decimal", "bar", "left", "num", "left"};
        static const char *const leaders[] = {"none", "dot", "hyphen", "underscore", "heavy", "middleDot", "none", "none"};
        x += "<w:tabs>";
        for (int pos : p.tabsCleared) {
            x += "<w:tab w:val=\"clear\"";
            attr(x, "w:pos", pos);
            x += "/>";
        }
        for (const TabStop &t : p.tabsAdded) {
            x += "<w:tab";
            attr(x, "w:val", QString::fromLatin1(kinds[t.jc & 7]));
            if (t.leader) attr(x, "w:leader", QString::fromLatin1(leaders[t.leader & 7]));
            attr(x, "w:pos", t.pos);
            x += "/>";
        }
        x += "</w:tabs>";
    }
    flag(p.noAutoHyph, "suppressAutoHyphens");
    flag(p.bidi, "bidi");
    if (p.dyaBefore || p.dyaAfter || p.dyaLine || p.beforeAuto || p.afterAuto) {
        x += "<w:spacing";
        if (p.dyaBefore) attr(x, "w:before", *p.dyaBefore);
        if (p.beforeAuto) attr(x, "w:beforeAutospacing", *p.beforeAuto ? 1 : 0);
        if (p.dyaAfter) attr(x, "w:after", *p.dyaAfter);
        if (p.afterAuto) attr(x, "w:afterAutospacing", *p.afterAuto ? 1 : 0);
        if (p.dyaLine) {
            const int line = *p.dyaLine;
            if (p.multiLine.value_or(false)) {
                attr(x, "w:line", std::max(line, 0));
                attr(x, "w:lineRule", QStringLiteral("auto"));
            } else {
                attr(x, "w:line", std::abs(line));
                attr(x, "w:lineRule", line < 0 ? QStringLiteral("exact") : QStringLiteral("atLeast"));
            }
        }
        x += "/>";
    }
    if (p.dxaLeft || p.dxaLeft1 || p.dxaRight) {
        x += "<w:ind";
        if (p.dxaLeft) attr(x, "w:left", *p.dxaLeft);
        if (p.dxaRight) attr(x, "w:right", *p.dxaRight);
        if (p.dxaLeft1) {
            if (*p.dxaLeft1 < 0) attr(x, "w:hanging", -*p.dxaLeft1);
            else attr(x, "w:firstLine", *p.dxaLeft1);
        }
        x += "/>";
    }
    flag(p.contextual, "contextualSpacing");
    if (p.jc) {
        static const char *const values[] = {"left", "center", "right", "both", "distribute", "left", "left", "left"};
        x += "<w:jc";
        attr(x, "w:val", QString::fromLatin1(values[*p.jc & 7]));
        x += "/>";
    }
    if (p.outline && *p.outline <= 9) {
        x += "<w:outlineLvl";
        attr(x, "w:val", *p.outline);
        x += "/>";
    }
    return x;
}

QByteArray sectionPropertiesXml(const Sep &s, const QByteArray &refs)
{
    QByteArray x = refs;
    // (The elements are in the order the schema gives.)
    auto noteProps = [&](const char *tag, const std::optional<int> &format, const std::optional<int> &start, int numbering) {
        if (!format && !start && numbering < 0) return;
        x += QByteArray("<w:") + tag + ">";
        if (format) x += QByteArray("<w:numFmt w:val=\"") + numberFormatName(*format) + "\"/>";
        if (start && *start > 1) x += "<w:numStart w:val=\"" + QByteArray::number(*start) + "\"/>";
        if (numbering == 1) x += "<w:numRestart w:val=\"eachSect\"/>";
        else if (numbering == 2) x += "<w:numRestart w:val=\"eachPage\"/>";
        x += QByteArray("</w:") + tag + ">";
    };
    noteProps("footnotePr", s.footnoteFormat, s.footnoteStart, s.footnoteNumbering);
    noteProps("endnotePr", s.endnoteFormat, s.endnoteStart, s.endnoteNumbering);
    if (s.type) {
        static const char *const types[] = {"continuous", "nextColumn", "nextPage", "evenPage", "oddPage"};
        x += "<w:type";
        attr(x, "w:val", QString::fromLatin1(types[std::clamp(*s.type, 0, 4)]));
        x += "/>";
    }
    x += "<w:pgSz";
    attr(x, "w:w", std::clamp(s.pageW, 720, 63360));
    attr(x, "w:h", std::clamp(s.pageH, 720, 63360));
    if (s.landscape) attr(x, "w:orient", QStringLiteral("landscape"));
    x += "/>";
    x += "<w:pgMar";
    attr(x, "w:top", std::clamp(s.top, -63360, 63360));
    attr(x, "w:right", std::clamp(s.right, 0, 63360));
    attr(x, "w:bottom", std::clamp(s.bottom, -63360, 63360));
    attr(x, "w:left", std::clamp(s.left, 0, 63360));
    attr(x, "w:header", std::clamp(s.header, 0, 63360));
    attr(x, "w:footer", std::clamp(s.footer, 0, 63360));
    attr(x, "w:gutter", std::clamp(s.gutter, 0, 63360));
    x += "/>";
    if (s.pageBorderProps || s.pageBorder[0].stated || s.pageBorder[1].stated || s.pageBorder[2].stated || s.pageBorder[3].stated) {
        const QByteArray b = borderXml("top", s.pageBorder[0]) + borderXml("left", s.pageBorder[1]) + borderXml("bottom", s.pageBorder[2]) + borderXml("right", s.pageBorder[3]);
        if (!b.isEmpty()) {
            x += "<w:pgBorders";
            attr(x, "w:offsetFrom", (s.pageBorderProps >> 5 & 7) == 1 ? QStringLiteral("text") : QStringLiteral("page"));
            x += ">" + b + "</w:pgBorders>";
        }
    }
    if (s.hasLineNumbers && s.lineNumberModulus > 0) {
        x += "<w:lnNumType";
        attr(x, "w:countBy", s.lineNumberModulus);
        if (s.lineNumberStart != 1) attr(x, "w:start", s.lineNumberStart - 1);   // (the format's start is how many numbers are skipped; the reader added the one)
        if (s.lineNumberDistance) attr(x, "w:distance", s.lineNumberDistance);
        attr(x, "w:restart", s.lineNumbering == 0 ? QStringLiteral("newPage") : s.lineNumbering == 1 ? QStringLiteral("newSection") : QStringLiteral("continuous"));
        x += "/>";
    }
    if ((s.pageStart && s.restart) || s.numberFormat) {
        static const char *const formats[] = {"decimal", "upperRoman", "lowerRoman", "upperLetter", "lowerLetter", "decimal"};
        x += "<w:pgNumType";
        if (s.numberFormat > 0 && s.numberFormat <= 4) attr(x, "w:fmt", QString::fromLatin1(formats[s.numberFormat]));
        if (s.pageStart && s.restart) attr(x, "w:start", std::clamp(*s.pageStart, 0, 32767));
        x += "/>";
    }
    {
        x += "<w:cols";
        if (s.columns > 1) {
            attr(x, "w:num", s.columns);
            attr(x, "w:space", std::clamp(s.colSpace, 0, 31680));
            if (s.lineBetween) attr(x, "w:sep", QStringLiteral("1"));
            if (!s.evenlySpaced && !s.colWidth.isEmpty()) {
                attr(x, "w:equalWidth", QStringLiteral("0"));
                x += ">";
                for (int i = 0; i < s.columns; ++i) {
                    x += "<w:col";
                    attr(x, "w:w", i < s.colWidth.size() ? s.colWidth[i] : 0);
                    if (i + 1 < s.columns) attr(x, "w:space", i < s.colGap.size() ? s.colGap[i] : s.colSpace);
                    x += "/>";
                }
                x += "</w:cols>";
            } else {
                x += "/>";
            }
        } else {
            attr(x, "w:space", std::clamp(s.colSpace, 0, 31680));
            x += "/>";
        }
    }
    if (s.vAlign) {
        x += "<w:vAlign";
        attr(x, "w:val", s.vAlign == 1 ? QStringLiteral("center") : s.vAlign == 2 ? QStringLiteral("both") : QStringLiteral("bottom"));
        x += "/>";
    }
    if (s.titlePage) x += "<w:titlePg/>";
    if (s.bidi) x += "<w:bidi/>";
    return x;
}

} // namespace jp::office::doc
