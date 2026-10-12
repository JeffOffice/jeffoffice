// The text of an open document presentation (odpimport_p.h). Paragraphs and
// spans are resolved through their styles (the object's, its presentation
// style's outline level, the paragraph's, the span's) to a DrawingML text
// body, which the shared reader (office/drawingml.h) turns into a story: the
// bullets, numbers, indents, spacing, and runs are made the one way for every
// format that has them.

#include "io/odpimport_p.h"

#include "core/fonts.h"

#include <QRegularExpression>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::deck::odp {

namespace {

constexpr int kMaxListDepth = 9;       // the levels a story keeps
constexpr int kMaxNesting = 64;        // elements inside a paragraph, and lists inside lists
constexpr int kMaxParagraphs = 100000;
constexpr qsizetype kMaxChars = 20000000;
const QString kA = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main");
const QString kR = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");

QString escaped(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (c == QLatin1Char('"')) out += QLatin1String("&quot;");
        else if (u < 0x20 && u != 9 && u != 10 && u != 13) continue;
        else out += c;
    }
    return out;
}

QString hundredths(double v) { return QString::number(qint64(std::llround(v * 100))); }

// A length for an attribute of DrawingML, in points.
QString points(double v) { return QString::number(v, 'f', 2) + QLatin1String("pt"); }

// The label of a bullet in a font of private-use letters, as a letter of ours.
QString bulletLetter(const QString &text, const QString &font, bool *dropFont)
{
    if (text.isEmpty()) return QStringLiteral("•");
    const QChar c = text[0];
    const QString f = font.toLower();
    // The letters of the symbol fonts that bullets use, as the letters of ours that look like them.
    if (isSymbolFont(font)) {
        const QString mapped = symbolToUnicode(font, c.unicode());
        if (!mapped.isEmpty()) {
            if (dropFont) *dropFont = true;
            return mapped;
        }
    }
    if (f.contains(QLatin1String("opensymbol")) || f.contains(QLatin1String("starsymbol"))) {
        switch (c.unicode()) {
        case 0xE00A: return QStringLiteral("■");
        case 0xE00C: return QStringLiteral("◆");
        case 0xE00D: return QStringLiteral("●");
        case 0xE011: return QStringLiteral("✔");
        case 0xE012: return QStringLiteral("✘");
        case 0x2794: case 0x27A2: case 0x2714: case 0x2717: case 0x25A0: case 0x25CF: case 0x2022: case 0x2013: return QString(c);
        default: return c.unicode() >= 0xE000 && c.unicode() < 0xF900 ? QStringLiteral("•") : QString(c);
        }
    }
    return text.left(1);
}

} // namespace

// The level of a list style: its label and its indents.
static ListLevel levelOf(const QDomElement &listStyle, int level)
{
    ListLevel out;
    if (listStyle.isNull()) return out;
    for (QDomElement c = listStyle.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString t = tagOf(c);
        if (t != QLatin1String("text:list-level-style-bullet") && t != QLatin1String("text:list-level-style-number") && t != QLatin1String("text:list-level-style-image")) continue;
        const Attrs a(c);
        if (int(lengthOr(a.get("text:level"), 0)) != level) continue;
        out.type = t.endsWith(QLatin1String("bullet")) ? ListLevel::Bullet : t.endsWith(QLatin1String("number")) ? ListLevel::Number : ListLevel::Image;
        out.bulletChar = a.get("text:bullet-char");
        out.numFormat = a.get("style:num-format");
        out.prefix = a.get("style:num-prefix");
        out.suffix = a.get("style:num-suffix");
        double v = 0;
        if (parseNumber(a.get("text:start-value"), &v)) out.startValue = std::clamp(int(v), 1, 32767);
        if (double p = 0; parsePercent(a.get("text:bullet-relative-size"), &p)) out.relativeSize = p;
        if (const QDomElement props = kid(c, "style:list-level-properties"); !props.isNull()) {
            const Attrs p(props);
            out.spaceBefore = lengthOr(p.get("text:space-before"), 0);
            out.minLabelWidth = lengthOr(p.get("text:min-label-width"), 0);
            if (p.get("text:list-level-position-and-space-mode") == QLatin1String("label-alignment")) {
                out.labelAlignment = true;
                if (const QDomElement al = kid(props, "style:list-level-label-alignment"); !al.isNull()) {
                    out.marginLeft = lengthOr(at(al, "fo:margin-left"), 0);
                    out.textIndent = lengthOr(at(al, "fo:text-indent"), 0);
                }
            }
        }
        if (const QDomElement text = kid(c, "style:text-properties"); !text.isNull()) out.text = Attrs(text).all();
        return out;
    }
    return out;
}

namespace {

// How DrawingML names a number scheme.
QString numberScheme(const ListLevel &lvl)
{
    QString kind;
    const QString f = lvl.numFormat;
    if (f == QLatin1String("a")) kind = QStringLiteral("alphaLc");
    else if (f == QLatin1String("A")) kind = QStringLiteral("alphaUc");
    else if (f == QLatin1String("i")) kind = QStringLiteral("romanLc");
    else if (f == QLatin1String("I")) kind = QStringLiteral("romanUc");
    else kind = QStringLiteral("arabic");
    if (lvl.prefix == QLatin1String("(") && lvl.suffix == QLatin1String(")")) return kind + QLatin1String("ParenBoth");
    if (lvl.prefix.isEmpty() && lvl.suffix == QLatin1String(")")) return kind + QLatin1String("ParenR");
    if (lvl.prefix.isEmpty() && lvl.suffix == QLatin1String(".")) return kind + QLatin1String("Period");
    return kind + QLatin1String("Plain");
}

} // namespace

bool Reader::hasText(const QDomElement &body) const
{
    struct Walk {
        static bool any(const QDomElement &e, int depth)
        {
            if (depth > kMaxNesting) return false;
            for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (n.isText() || n.isCDATASection()) {
                    if (!n.nodeValue().trimmed().isEmpty()) return true;
                } else if (n.isElement()) {
                    const QDomElement c = n.toElement();
                    const QString t = tagOf(c);
                    if (t == QLatin1String("text:page-number") || t == QLatin1String("presentation:date-time") || t == QLatin1String("text:date")) return true;
                    if (t == QLatin1String("draw:frame") || t == QLatin1String("draw:custom-shape") || t == QLatin1String("office:annotation")) continue;
                    if (any(c, depth + 1)) return true;
                }
            }
            return false;
        }
    };
    return !body.isNull() && Walk::any(body, 0);
}

namespace {

// The run properties of a bag as the attributes and children of an a:rPr.
class Runs {
public:
    Runs(const StyleSheet &styles, const QHash<QString, QString> *links, const QColor &autoColor) : m_styles(styles), m_links(links), m_auto(autoColor) {}

    // The face the bag names, resolved through the font declarations.
    QString face(const Bag &b) const
    {
        QString name = b.get(Text, "style:font-name");
        if (!name.isEmpty()) return m_styles.fontFamily(name);
        QString family = b.get(Text, "fo:font-family");
        if (family.size() >= 2 && (family.startsWith(QLatin1Char('\'')) || family.startsWith(QLatin1Char('"'))) && family.endsWith(family[0])) family = family.mid(1, family.size() - 2);
        return family.section(QLatin1Char(','), 0, 0).trimmed();
    }

    double sizeOf(const Bag &b) const
    {
        double pt = 0;
        const QString s = b.get(Text, "fo:font-size");
        if (parseLength(s, &pt)) return std::clamp(pt, 1.0, 4000.0);
        if (double pct = 0; parsePercent(s, &pct)) return std::clamp(18.0 * pct, 1.0, 4000.0);
        return 18;
    }

    QString attrs(const Bag &b) const
    {
        QString a;
        a += QStringLiteral(" sz=\"%1\"").arg(qint64(std::llround(sizeOf(b) * 100)));
        if (b.has(Text, "fo:font-weight")) {
            const QString w = b.get(Text, "fo:font-weight");
            double n = 0;
            const bool bold = w == QLatin1String("bold") || (parseNumber(w, &n) && n >= 600);
            a += bold ? QLatin1String(" b=\"1\"") : QLatin1String(" b=\"0\"");
        }
        if (b.has(Text, "fo:font-style")) a += b.get(Text, "fo:font-style") == QLatin1String("normal") ? QLatin1String(" i=\"0\"") : QLatin1String(" i=\"1\"");
        const QString us = b.get(Text, "style:text-underline-style");
        if (!us.isEmpty() && us != QLatin1String("none")) {
            const bool dbl = b.get(Text, "style:text-underline-type") == QLatin1String("double");
            const bool heavy = b.get(Text, "style:text-underline-width").contains(QLatin1String("bold")) || b.get(Text, "style:text-underline-width") == QLatin1String("thick");
            QString u = us == QLatin1String("dotted") ? QStringLiteral("dotted") : us == QLatin1String("dash") ? QStringLiteral("dash")
                      : us == QLatin1String("long-dash") ? QStringLiteral("dashLong") : us == QLatin1String("dot-dash") ? QStringLiteral("dotDash")
                      : us == QLatin1String("dot-dot-dash") ? QStringLiteral("dotDotDash") : us == QLatin1String("wave") ? QStringLiteral("wavy") : QStringLiteral("sng");
            if (dbl && u == QLatin1String("sng")) u = QStringLiteral("dbl");
            else if (dbl && u == QLatin1String("wavy")) u = QStringLiteral("wavyDbl");
            else if (heavy) u = u == QLatin1String("sng") ? QStringLiteral("heavy") : u + QLatin1String("Heavy");
            a += QStringLiteral(" u=\"%1\"").arg(u);
        } else if (us == QLatin1String("none")) {
            a += QLatin1String(" u=\"none\"");
        }
        const QString lt = b.get(Text, "style:text-line-through-style");
        if (!lt.isEmpty() && lt != QLatin1String("none")) a += b.get(Text, "style:text-line-through-type") == QLatin1String("double") ? QLatin1String(" strike=\"dblStrike\"") : QLatin1String(" strike=\"sngStrike\"");
        const QString transform = b.get(Text, "fo:text-transform");
        if (transform == QLatin1String("uppercase")) a += QLatin1String(" cap=\"all\"");
        else if (b.get(Text, "fo:font-variant") == QLatin1String("small-caps")) a += QLatin1String(" cap=\"small\"");
        const QString position = b.get(Text, "style:text-position").trimmed();
        if (!position.isEmpty()) {
            const QString first = position.section(QLatin1Char(' '), 0, 0);
            double pct = 0;
            if (first == QLatin1String("super")) a += QLatin1String(" baseline=\"30000\"");
            else if (first == QLatin1String("sub")) a += QLatin1String(" baseline=\"-25000\"");
            else if (parsePercent(first, &pct) && pct > 0.005) a += QLatin1String(" baseline=\"30000\"");
            else if (parsePercent(first, &pct) && pct < -0.005) a += QLatin1String(" baseline=\"-25000\"");
        }
        double spacing = 0;
        if (parseLength(b.get(Text, "fo:letter-spacing"), &spacing) && std::fabs(spacing) > 0.001) a += QStringLiteral(" spc=\"%1\"").arg(hundredths(std::clamp(spacing, -1584.0, 1584.0)));
        const QString lang = b.get(Text, "fo:language");
        if (!lang.isEmpty() && lang != QLatin1String("none")) {
            const QString country = b.get(Text, "fo:country");
            a += QStringLiteral(" lang=\"%1\"").arg(escaped(country.isEmpty() || country == QLatin1String("none") ? lang : lang + QLatin1Char('-') + country));
        }
        return a;
    }

    QString children(const Bag &b, const QString &href = QString()) const
    {
        QString c;
        QColor color = parseColor(b.get(Text, "fo:color"));
        if (!color.isValid()) color = m_auto;
        if (color.isValid()) c += QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(color.name().mid(1).toUpper());
        const QColor back = parseColor(b.get(Text, "fo:background-color"));
        if (back.isValid()) c += QStringLiteral("<a:highlight><a:srgbClr val=\"%1\"/></a:highlight>").arg(back.name().mid(1).toUpper());
        const QString f = face(b);
        if (!f.isEmpty()) c += QStringLiteral("<a:latin typeface=\"%1\"/>").arg(escaped(f));
        if (!href.isEmpty() && m_links && m_links->contains(href)) c += QStringLiteral("<a:hlinkClick r:id=\"%1\"/>").arg(m_links->value(href));
        return c;
    }

    QString props(const char *element, const Bag &b, const QString &href = QString()) const
    {
        return QStringLiteral("<a:%1%2>%3</a:%1>").arg(QLatin1String(element), attrs(b), children(b, href));
    }

private:
    const StyleSheet &m_styles;
    const QHash<QString, QString> *m_links;
    QColor m_auto;
};

class Paragraphs {
public:
    Paragraphs(const StyleSheet &styles, const QHash<QString, QString> &links, const QString &outlinePrefix, bool outline, const Bag &base, const QColor &autoColor)
        : m_styles(styles), m_links(links), m_prefix(outlinePrefix), m_outline(outline), m_base(base), m_runs(styles, &m_links, autoColor)
    {
    }

    QString out;
    int count = 0;

    void body(const QDomElement &e, int depth, const QString &listStyle)
    {
        for (QDomElement c = e.firstChildElement(); !c.isNull() && count < kMaxParagraphs && m_chars < kMaxChars; c = c.nextSiblingElement()) {
            const QString t = tagOf(c);
            if (t == QLatin1String("text:p") || t == QLatin1String("text:h")) {
                paragraph(c, depth, listStyle);
            } else if (t == QLatin1String("text:list")) {
                if (depth >= kMaxNesting) continue;
                const QString style = at(c, "text:style-name", listStyle);
                for (QDomElement item = c.firstChildElement(); !item.isNull(); item = item.nextSiblingElement()) {
                    const QString it = tagOf(item);
                    if (it == QLatin1String("text:list-item") || it == QLatin1String("text:list-header")) body(item, depth + 1, style);
                }
            } else if (t == QLatin1String("text:section") || t == QLatin1String("text:soft-page-break") ) {
                if (t == QLatin1String("text:section")) body(c, depth, listStyle);
            }
        }
    }

private:
    // The bag a paragraph at that outline level starts from.
    Bag levelBag(int level) const
    {
        Bag b = m_base;
        if (m_outline && !m_prefix.isNull() && level >= 1) {
            Bag lvl;
            m_styles.apply(lvl, QStringLiteral("presentation"), m_prefix + QStringLiteral("outline%1").arg(std::min(level, 9)));
            b.overlay(lvl);
        }
        return b;
    }

    struct Label {
        bool active = false;
        QString xml;          // the bullet's elements for a:pPr
        double marL = 0, indent = 0;
        bool indents = false;
    };

    Label labelFor(const Bag &pb, int depth, const QString &listStyleName, int level, double marginLeft, const Bag &levelBagForOutline) const
    {
        Label label;
        QDomElement style;
        // A paragraph in a list has the bullets of the list's style, or, where the list names none, those the placeholder's outline has.
        if (depth > 0) {
            if (!listStyleName.isEmpty()) style = m_styles.list(listStyleName);
            if (style.isNull() && m_outline) style = !levelBagForOutline.graphicListStyle.isNull() ? levelBagForOutline.graphicListStyle : m_base.graphicListStyle;
        }
        if (style.isNull()) return label;
        const ListLevel lvl = levelOf(style, std::clamp(level, 1, 10));
        if (lvl.type == ListLevel::None) return label;
        label.active = true;
        label.indents = true;
        if (lvl.labelAlignment) {
            label.marL = lvl.marginLeft;
            label.indent = lvl.textIndent;
        } else {
            label.marL = marginLeft + lvl.spaceBefore + lvl.minLabelWidth;
            label.indent = -lvl.minLabelWidth;
        }
        // The label's color, font, and size.
        Bag text;
        text.values[Text] = lvl.text;
        QString xml;
        const QColor color = parseColor(lvl.text.value(QStringLiteral("fo:color")));
        if (color.isValid() && lvl.text.value(QStringLiteral("style:use-window-font-color")) != QLatin1String("true"))
            xml += QStringLiteral("<a:buClr><a:srgbClr val=\"%1\"/></a:buClr>").arg(color.name().mid(1).toUpper());
        QString font = m_runs.face(text);
        if (lvl.type == ListLevel::Bullet) {
            const QString lower = font.toLower();
            const bool symbols = lower.contains(QLatin1String("opensymbol")) || lower.contains(QLatin1String("starsymbol"));
            bool dropFont = false;
            QString letter = bulletLetter(lvl.bulletChar, font, &dropFont);
            double size = lvl.relativeSize;
            if (double pct = 0; parsePercent(lvl.text.value(QStringLiteral("fo:font-size")), &pct)) size = pct;
            // The big round bullet of the symbol font, drawn small, is the usual bullet at full size.
            if (symbols && letter == QStringLiteral("●") && size > 0 && size <= 0.6) {
                letter = QStringLiteral("•");
                size = 1;
            } else if (symbols && letter == QStringLiteral("–") && size > 0 && size <= 0.8) {
                size = 1;
            }
            if (size > 0 && std::fabs(size - 1) > 0.01) xml += QStringLiteral("<a:buSzPct val=\"%1\"/>").arg(std::clamp(qint64(std::llround(size * 100000)), qint64(25000), qint64(400000)));
            if (!font.isEmpty() && !symbols && !dropFont) xml += QStringLiteral("<a:buFont typeface=\"%1\"/>").arg(escaped(font));
            xml += QStringLiteral("<a:buChar char=\"%1\"/>").arg(escaped(letter));
        } else if (lvl.type == ListLevel::Number) {
            if (lvl.numFormat.isEmpty()) {
                xml = QStringLiteral("<a:buNone/>");
            } else {
                if (!font.isEmpty()) xml += QStringLiteral("<a:buFont typeface=\"%1\"/>").arg(escaped(font));
                xml += QStringLiteral("<a:buAutoNum type=\"%1\" startAt=\"%2\"/>").arg(numberScheme(lvl)).arg(lvl.startValue);
            }
        } else {
            xml += QStringLiteral("<a:buChar char=\"•\"/>");
        }
        label.xml = xml;
        return label;
    }

    void paragraph(const QDomElement &p, int depth, const QString &listStyleName)
    {
        ++count;
        const int level = std::min(depth, kMaxListDepth);   // 0 outside lists: the first level
        // Level 1 is the first of a placeholder's outline, whether or not it is in a list.
        const int outlineLevel = std::max(1, level);
        Bag pb = levelBag(outlineLevel);
        const QString styleName = at(p, "text:style-name");
        m_styles.apply(pb, QStringLiteral("paragraph"), styleName);
        const Bag levelOnly = levelBag(outlineLevel);

        // Paragraph settings
        QString attrsXml, childXml;
        const QString align = pb.get(Para, "fo:text-align");
        const bool rtl = pb.get(Para, "style:writing-mode").startsWith(QLatin1String("rl"));
        if (align == QLatin1String("center")) attrsXml += QLatin1String(" algn=\"ctr\"");
        else if (align == QLatin1String("justify")) attrsXml += QLatin1String(" algn=\"just\"");
        else if (align == QLatin1String("end")) attrsXml += rtl ? QLatin1String(" algn=\"l\"") : QLatin1String(" algn=\"r\"");
        else if (align == QLatin1String("right")) attrsXml += QLatin1String(" algn=\"r\"");
        else if (align == QLatin1String("start")) attrsXml += rtl ? QLatin1String(" algn=\"r\"") : QLatin1String(" algn=\"l\"");
        else if (align == QLatin1String("left")) attrsXml += QLatin1String(" algn=\"l\"");
        if (rtl) attrsXml += QLatin1String(" rtl=\"1\"");
        double marL = lengthOr(pb.get(Para, "fo:margin-left"), 0), marR = lengthOr(pb.get(Para, "fo:margin-right"), 0);
        double indent = lengthOr(pb.get(Para, "fo:text-indent"), 0);
        const Label label = labelFor(pb, depth, listStyleName, outlineLevel, marL, levelOnly);
        if (label.indents) {
            marL = label.marL;
            indent = label.indent;
        }
        attrsXml += QStringLiteral(" marL=\"%1\" marR=\"%2\" indent=\"%3\"").arg(points(std::clamp(marL, 0.0, 20000.0)), points(std::clamp(marR, 0.0, 20000.0)), points(std::clamp(indent, -20000.0, 20000.0)));
        if (outlineLevel > 1 || depth > 0) attrsXml += QStringLiteral(" lvl=\"%1\"").arg(std::clamp(outlineLevel - 1, 0, 8));
        if (double tab = 0; parseLength(pb.get(Para, "loext:tab-stop-distance"), &tab) && tab >= 1) attrsXml += QStringLiteral(" defTabSz=\"%1\"").arg(points(tab));

        const QString lineHeight = pb.get(Para, "fo:line-height");
        if (double pct = 0; parsePercent(lineHeight, &pct) && pct > 0) childXml += QStringLiteral("<a:lnSpc><a:spcPct val=\"%1\"/></a:lnSpc>").arg(qint64(std::llround(pct * 100000)));
        else if (double len = 0; parseLength(lineHeight, &len) && len > 0.1) childXml += QStringLiteral("<a:lnSpc><a:spcPts val=\"%1\"/></a:lnSpc>").arg(hundredths(std::min(len, 1584.0)));
        if (double v = 0; parseLength(pb.get(Para, "fo:margin-top"), &v)) childXml += QStringLiteral("<a:spcBef><a:spcPts val=\"%1\"/></a:spcBef>").arg(hundredths(std::clamp(v, 0.0, 1584.0)));
        if (double v = 0; parseLength(pb.get(Para, "fo:margin-bottom"), &v)) childXml += QStringLiteral("<a:spcAft><a:spcPts val=\"%1\"/></a:spcAft>").arg(hundredths(std::clamp(v, 0.0, 1584.0)));
        childXml += label.active ? label.xml : QStringLiteral("<a:buNone/>");
        if (!pb.tabStops.isNull()) {
            QString tabs;
            int n = 0;
            for (const QDomElement &t : kids(pb.tabStops, "style:tab-stop")) {
                double pos = 0;
                if (!parseLength(at(t, "style:position"), &pos) || pos < 0 || pos > 20000 || ++n > 32) continue;
                const QString type = at(t, "style:type");
                tabs += QStringLiteral("<a:tab pos=\"%1\" algn=\"%2\"/>").arg(points(pos), type == QLatin1String("right") ? QStringLiteral("r") : type == QLatin1String("center") ? QStringLiteral("ctr") : type == QLatin1String("char") ? QStringLiteral("dec") : QStringLiteral("l"));
            }
            if (!tabs.isEmpty()) childXml += QStringLiteral("<a:tabLst>%1</a:tabLst>").arg(tabs);
        }

        // The runs.
        m_pieces.clear();
        m_pendingProps.clear();
        m_pendingText.clear();
        m_space = true;   // leading white space goes
        inline_(p, pb, QString(), 0);
        flush();
        out += QStringLiteral("<a:p><a:pPr%1>%2</a:pPr>%3%4</a:p>").arg(attrsXml, childXml, m_pieces, m_runs.props("endParaRPr", pb));
    }

    // A run of text with one set of properties is held until the properties change.
    void flush()
    {
        if (m_pendingText.isEmpty()) return;
        m_chars += m_pendingText.size();
        m_pieces += QStringLiteral("<a:r>%1<a:t>%2</a:t></a:r>").arg(m_pendingProps, escaped(m_pendingText));
        m_pendingText.clear();
    }

    void add(const QString &text, const QString &props)
    {
        if (text.isEmpty()) return;
        if (props != m_pendingProps) flush();
        m_pendingProps = props;
        m_pendingText += text;
    }

    void inline_(const QDomElement &parent, const Bag &bag, const QString &href, int depth)
    {
        if (depth > kMaxNesting) return;
        const QString props = m_runs.props("rPr", bag, href);
        for (QDomNode n = parent.firstChild(); !n.isNull() && m_chars < kMaxChars; n = n.nextSibling()) {
            if (n.isText() || n.isCDATASection()) {
                add(collapse(n.nodeValue()), props);
                continue;
            }
            if (!n.isElement()) continue;
            const QDomElement c = n.toElement();
            const QString t = tagOf(c);
            if (t == QLatin1String("text:span") || t == QLatin1String("text:meta") || t == QLatin1String("text:ruby") || t == QLatin1String("text:ruby-base")) {
                Bag b = bag;
                m_styles.apply(b, QStringLiteral("text"), at(c, "text:style-name"));
                if (at(c, "text:display") == QLatin1String("none") || b.get(Text, "text:display") == QLatin1String("none")) continue;
                inline_(c, b, href, depth + 1);
            } else if (t == QLatin1String("text:a")) {
                Bag b = bag;
                m_styles.apply(b, QStringLiteral("text"), at(c, "text:style-name"));
                const QString target = at(c, "xlink:href");
                inline_(c, b, isWebLink(target) ? target : QString(), depth + 1);
            } else if (t == QLatin1String("text:s")) {
                double count = 1;
                parseNumber(at(c, "text:c", QStringLiteral("1")), &count);
                add(QString(std::clamp(int(count), 1, 1000), QLatin1Char(' ')), props);
                m_space = false;
            } else if (t == QLatin1String("text:tab")) {
                add(QStringLiteral("\t"), props);
                m_space = true;
            } else if (t == QLatin1String("text:line-break")) {
                add(QString(QChar(QChar::LineSeparator)), props);
                m_space = true;
            } else if (t == QLatin1String("text:page-number")) {
                field(QStringLiteral("slidenum"), c, bag, href);
            } else if (t == QLatin1String("presentation:date-time") || (t == QLatin1String("text:date") && at(c, "text:fixed") != QLatin1String("true"))
                       || (t == QLatin1String("text:time") && at(c, "text:fixed") != QLatin1String("true"))) {
                field(t == QLatin1String("text:time") ? QStringLiteral("datetime10") : QStringLiteral("datetime1"), c, bag, href);
            } else if (t == QLatin1String("presentation:footer") || t == QLatin1String("presentation:header")) {
                // The text of a footer is the page's, kept where the page says it.
                continue;
            } else if (t == QLatin1String("text:soft-page-break") || t == QLatin1String("text:note") || t == QLatin1String("office:annotation") || t == QLatin1String("draw:frame")
                       || t == QLatin1String("draw:custom-shape") || t == QLatin1String("text:bookmark") || t == QLatin1String("text:bookmark-start")
                       || t == QLatin1String("text:bookmark-end") || t == QLatin1String("text:alphabetical-index-mark") || t == QLatin1String("text:change")
                       || t == QLatin1String("text:change-start") || t == QLatin1String("text:change-end") || t == QLatin1String("office:annotation-end")) {
                continue;
            } else {
                // A field or other inline element shows the text it last had.
                inline_(c, bag, href, depth + 1);
            }
        }
    }

    void field(const QString &type, const QDomElement &c, const Bag &bag, const QString &href)
    {
        flush();
        const QString shown = c.text().trimmed();
        m_pieces += QStringLiteral("<a:fld id=\"{00000000-0000-0000-0000-000000000000}\" type=\"%1\">%2<a:t>%3</a:t></a:fld>")
                        .arg(type, m_runs.props("rPr", bag, href), escaped(shown.isEmpty() ? QStringLiteral("‹#›") : shown));
        m_space = false;
    }

    // White space as the standard has it: runs of it are a space, and there is none at the start of a paragraph or after a line break.
    QString collapse(const QString &s)
    {
        QString out;
        out.reserve(s.size());
        for (const QChar c : s) {
            const bool white = c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n') || c == QLatin1Char('\r');
            if (white) {
                if (!m_space) out += QLatin1Char(' ');
                m_space = true;
            } else {
                out += c;
                m_space = false;
            }
        }
        return out;
    }

    const StyleSheet &m_styles;
    QHash<QString, QString> m_links;
    QString m_prefix;
    bool m_outline;
    Bag m_base;
    Runs m_runs;
    QString m_pieces, m_pendingProps, m_pendingText;
    bool m_space = true;
    qsizetype m_chars = 0;
};

// The name an outline style has before its level: "Title-outline1" is of "Title-". Null when there is none.
QString outlinePrefixOf(const StyleSheet &styles, const QString &family, const QString &name)
{
    static const QRegularExpression pattern(QStringLiteral("^(.*)outline[1-9]$"));
    QString current = name;
    for (int i = 0; i < 16 && !current.isEmpty(); ++i) {
        const QRegularExpressionMatch m = pattern.match(current);
        if (m.hasMatch()) return m.captured(1);
        const StyleDef *s = styles.find(family, current);
        if (!s) break;
        current = s->parent;
    }
    return QString();
}

} // namespace

QString Reader::textXml(const QDomElement &body, const TextEnv &env)
{
    const QHash<QString, QString> &links = m_linkIds;
    QString prefix = env.outline ? outlinePrefixOf(*env.styles, QStringLiteral("presentation"), env.presentationStyle) : QString();
    // An outline whose style names no levels has no level styles.
    Paragraphs paragraphs(*env.styles, links, prefix, env.outline, env.base, env.autoColor);
    paragraphs.body(body, 0, QString());
    return QStringLiteral("<a:txBody xmlns:a=\"%1\" xmlns:r=\"%2\"><a:bodyPr/><a:lstStyle/>%3%4</a:txBody>")
        .arg(kA, kR, paragraphs.out, paragraphs.count == 0 ? QStringLiteral("<a:p><a:endParaRPr/></a:p>") : QString());
}

void Reader::fillStory(const QString &storyId, const QDomElement &body, const TextEnv &env)
{
    QTextDocument *doc = m_doc.storyDoc(storyId);
    if (!doc) return;
    QDomDocument dom;
    QString why;
    if (!office::parseXml(textXml(body, env).toUtf8(), &dom, &why)) return;
    const office::DrawingContext ctx = drawingContext();
    office::readTextBody(dom.documentElement(), doc, ctx);
}

Reader::TextEnv Reader::textEnvFor(const Scope &sc, const Attrs &frame, const Bag &shapeBag, const QString &cls)
{
    TextEnv env;
    env.styles = sc.styles;
    env.base = shapeBag;
    env.cls = cls;
    env.presentationStyle = frame.get("presentation:style-name");
    env.textStyle = frame.get("draw:text-style-name");
    // The text style of the object is a paragraph style that its paragraphs start from.
    if (!env.textStyle.isEmpty()) {
        Bag text;
        sc.styles->apply(text, QStringLiteral("paragraph"), env.textStyle);
        env.base.overlay(text);
    }
    env.outline = cls == QLatin1String("outline");
    // The color of text that names none: against the object's own fill, or the page behind it.
    QColor back = sc.background;
    Bag fillBag = shapeBag;
    const Fill fill = fillOf(fillBag, *sc.styles);
    if (fill.type == Fill::Solid && fill.transparency < 0.5 && fill.color.kind() == ColorRef::Rgb) back = fill.color.rgbValue();
    else if (fill.type == Fill::Gradient && !fill.stops.isEmpty() && fill.stops.first().color.kind() == ColorRef::Rgb) back = fill.stops.first().color.rgbValue();
    env.autoColor = back.lightness() < 128 ? QColor(Qt::white) : QColor(Qt::black);
    return env;
}

} // namespace jp::deck::odp
