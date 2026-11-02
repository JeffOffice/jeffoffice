// .docx to OpenDocument text: the package, the styles, the numbering, and the page layouts.

#include "odt/odtwrite.h"

#include "io/zip.h"
#include "odt/odtread.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>

namespace jp::office::odt {

namespace {

const char kOdfNamespaces[] =
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
    " xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
    " xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\" xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\""
    " xmlns:math=\"http://www.w3.org/1998/Math/MathML\" xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\""
    " xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\" xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\""
    " xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\"";

const char kXml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

QByteArray num(qint64 v)
{
    return QByteArray::number(v);
}

} // namespace

QString StyleSink::make(const char *base, const char *family, const QByteArray &attrs, const QByteArray &inner)
{
    const QByteArray key = QByteArray(family) + '|' + attrs + '|' + inner;
    const auto it = byKey.constFind(key);
    if (it != byKey.constEnd()) return it.value();
    const QString b = prefix + QLatin1String(base);
    const QString name = b + QString::number(++counts[b]);
    xml += "<style:style style:name=\"" + name.toUtf8() + "\" style:family=\"" + family + "\"" + attrs + (inner.isEmpty() ? "/>" : ">" + inner + "</style:style>");
    byKey.insert(key, name);
    return name;
}

void Writer::left(const QString &what)
{
    if (!m_notes || m_leftKeys.contains(what)) return;
    m_leftKeys.insert(what);
    m_notes->push_back(what);
}

wml::XmlTree *Writer::parse(const QString &partName)
{
    if (!m_pkg.hasPart(partName)) return nullptr;
    Part p;
    p.tree = std::make_unique<T>();
    p.name = partName;
    if (!p.tree->parse(m_pkg.part(partName), nullptr)) return nullptr;
    m_parts.push_back(std::move(p));
    return m_parts.back().tree.get();
}

QString Writer::partOf(const QString &rid, const QString &type) const
{
    const Relationship r = m_pkg.relationship(m_source, rid);
    if (r.id.isEmpty() || r.external) return QString();
    if (!type.isEmpty() && !r.type.endsWith(type)) return QString();
    return r.target;
}

bool Writer::loadParts(QString *error)
{
    m_main = m_pkg.mainPart();
    if (m_main.isEmpty() || !m_pkg.hasPart(m_main)) {
        if (error) *error = QCoreApplication::translate("odt", "The document has no main part.");
        return false;
    }
    m_parts.reserve(12);
    m_doc = parse(m_main);
    if (!m_doc) {
        if (error) *error = QCoreApplication::translate("odt", "The document's text can't be read.");
        return false;
    }
    auto related = [&](const char *type) {
        const auto rels = m_pkg.relationshipsOfType(m_main, QString::fromLatin1(type));
        return rels.isEmpty() ? QString() : rels.first().target;
    };
    if (const QString s = related("/odfSettings"); !s.isEmpty() && m_pkg.hasPart(s)) {
        T st;
        if (st.parse(m_pkg.part(s), nullptr))
            for (int c = st.firstChild(st.root()); c >= 0; c = st.next(c))
                if (st.localName(c) == "config-item") m_odfItems += "<config:config-item config:name=\"" + esc(st.attrAny(c, "name")) + "\" config:type=\"" + esc(st.attrAny(c, "type")) + "\">" + esc(st.text(c), false) + "</config:config-item>";
    }
    if (const QString s = related("/fontTable"); !s.isEmpty() && m_pkg.hasPart(s)) {
        T ft;
        if (ft.parse(m_pkg.part(s), nullptr))
            for (int c = ft.firstChild(ft.root()); c >= 0; c = ft.next(c))
                if (ft.is(c, "font")) {
                    const int fam = ft.child(c, "family"), pit = ft.child(c, "pitch");
                    m_fontKinds.insert(ft.attr(c, "name"), {fam >= 0 ? ft.val(fam) : QString(), pit >= 0 ? ft.val(pit) : QString()});
                }
    }
    if (const QString s = related("/styles"); !s.isEmpty()) m_styles = parse(s);
    if (const QString s = related("/numbering"); !s.isEmpty()) m_numbering = parse(s);
    if (const QString s = related("/settings"); !s.isEmpty()) m_settings = parse(s);
    if (const QString s = related("/footnotes"); !s.isEmpty()) m_footnotes = parse(s);
    if (const QString s = related("/endnotes"); !s.isEmpty()) m_endnotes = parse(s);
    if (const QString s = related("/comments"); !s.isEmpty()) m_comments = parse(s);
    return true;
}

// ---- styles

namespace {

struct Map {
    const char *docx, *odf;
};

const Map kNames[] = {
    {"Normal", "Standard"}, {"heading 1", "Heading 1"}, {"heading 2", "Heading 2"}, {"heading 3", "Heading 3"}, {"heading 4", "Heading 4"}, {"heading 5", "Heading 5"},
    {"heading 6", "Heading 6"}, {"heading 7", "Heading 7"}, {"heading 8", "Heading 8"}, {"heading 9", "Heading 9"}, {"Title", "Title"}, {"Subtitle", "Subtitle"},
    {"Body Text", "Text body"}, {"header", "Header"}, {"footer", "Footer"}, {"footnote text", "Footnote"}, {"endnote text", "Endnote"}, {"caption", "Caption"},
    {"footnote reference", "Footnote Symbol"}, {"endnote reference", "Endnote Symbol"}, {"Hyperlink", "Internet link"}, {"FollowedHyperlink", "Visited Internet Link"},
    {"toc 1", "Contents 1"}, {"toc 2", "Contents 2"}, {"toc 3", "Contents 3"}, {"toc 4", "Contents 4"}, {"toc 5", "Contents 5"}, {"toc 6", "Contents 6"},
    {"toc 7", "Contents 7"}, {"toc 8", "Contents 8"}, {"toc 9", "Contents 9"}, {"TOC Heading", "Contents Heading"}, {"Quote", "Quotations"}, {"Strong", "Strong Emphasis"},
};

} // namespace

QString Writer::odfStyleName(const DStyle &s) const
{
    QString display = s.name.isEmpty() ? s.id : s.name;
    for (const Map &m : kNames)
        if (display == QLatin1String(m.docx)) {
            display = QString::fromLatin1(m.odf);
            break;
        }
    return encodeStyleName(display);
}

void Writer::collectStyles()
{
    if (!m_styles) return;
    const T &t = *m_styles;
    QSet<QString> taken;
    for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c)) {
        if (!t.is(c, "style")) continue;
        DStyle s;
        s.id = t.attr(c, "styleId");
        if (s.id.isEmpty()) continue;
        s.type = t.attr(c, "type");
        s.node = c;
        for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
            if (t.is(k, "name")) s.name = t.val(k);
            else if (t.is(k, "basedOn")) s.basedOn = t.val(k);
            else if (t.is(k, "next")) s.next = t.val(k);
        }
        if (s.name.isEmpty()) s.name = s.id;
        s.odfName = odfStyleName(s);
        // (Two styles of the same family that map to one name: the second is made different.)
        const QString key = s.type + QLatin1Char('|') + s.odfName;
        if (taken.contains(key)) s.odfName += QStringLiteral("_%1").arg(m_dstyles.size());
        taken.insert(s.type + QLatin1Char('|') + s.odfName);
        if (t.attr(c, "default") == QLatin1String("1") || t.attr(c, "default") == QLatin1String("true")) {
            if (s.type == QLatin1String("paragraph")) m_defaultParaStyle = s.id;
            else if (s.type == QLatin1String("character")) m_defaultCharStyle = s.id;
        }
        m_dstyles.insert(s.id, s);
        m_styleOrder << s.id;
    }
}

QString Writer::themeFont(const QString &theme) const
{
    // The typeface a theme names for a script (the theme part, found from the main part).
    static const QRegularExpression minor(QStringLiteral("<a:minorFont>\\s*<a:latin typeface=\"([^\"]*)\""));
    static const QRegularExpression major(QStringLiteral("<a:majorFont>\\s*<a:latin typeface=\"([^\"]*)\""));
    const auto rels = m_pkg.relationshipsOfType(m_main, QStringLiteral("/theme"));
    if (rels.isEmpty()) return QString();
    const QString xml = QString::fromUtf8(m_pkg.part(rels.first().target));
    const auto m = (theme.startsWith(QLatin1String("major")) ? major : minor).match(xml);
    return m.hasMatch() ? m.captured(1) : QString();
}

QString Writer::fontNameOf(const QString &family, QString *generic)
{
    if (family.isEmpty()) return QString();
    if (generic && *generic == QLatin1String("system")) {
        // (A font for East Asian or complex script is a face of its own, of the "system" kind, as the other programs name it.)
        const QString face = family + QLatin1Char('1');
        m_systemFaces.insert(face, family);
        if (!m_fontFaces.contains(face)) m_fontFaces.insert(face, QString());
        return face;
    }
    if (!m_fontFaces.contains(family)) m_fontFaces.insert(family, QString());
    return family;
}

QString Writer::shadeColor(const T &t, int shd) const
{
    const QString fill = t.attr(shd, "fill");
    if (fill.isEmpty() || fill == QLatin1String("auto")) return QString();
    const QString val = t.attr(shd, "val");
    if (val == QLatin1String("nil")) return QString();
    const QString hex = hexColorOrEmpty(fill);
    return hex.isEmpty() ? QString() : QStringLiteral("#") + hex;
}

QString Writer::borderValue(const T &t, int side, int *space, QString *lineWidth) const
{
    const QString val = t.attr(side, "val");
    if (val.isEmpty()) return QString();
    if (val == QLatin1String("nil") || val == QLatin1String("none")) return QStringLiteral("none");
    const double pt = std::max(0.25, std::min(12.0, t.attr(side, "sz").toDouble() / 8.0));
    QString color = t.attr(side, "color");
    color = color.isEmpty() || color == QLatin1String("auto") ? QStringLiteral("#000000") : QStringLiteral("#") + (hexColorOrEmpty(color).isEmpty() ? QStringLiteral("000000") : hexColorOrEmpty(color));
    QString style = QStringLiteral("solid");
    if (val == QLatin1String("double")) style = QStringLiteral("double");
    else if (val == QLatin1String("dotted")) style = QStringLiteral("dotted");
    else if (val.startsWith(QLatin1String("dash"))) style = QStringLiteral("dashed");
    if (space) *space = t.attr(side, "space").toInt();
    if (lineWidth && style == QLatin1String("double")) {
        const QString third = QString::number(pt / 3.0, 'f', 3) + QStringLiteral("pt");
        *lineWidth = third + QLatin1Char(' ') + third + QLatin1Char(' ') + third;
    }
    return QString::number(pt, 'f', 3) + QStringLiteral("pt ") + style + QLatin1Char(' ') + color;
}

QByteArray Writer::bordersAttrs(const T &t, int parent, bool paragraph)
{
    QByteArray x;
    static const char *const sides[][2] = {{"top", "top"}, {"left", "left"}, {"bottom", "bottom"}, {"right", "right"}};
    for (const auto &s : sides) {
        int side = t.child(parent, s[0]);
        if (side < 0 && std::strcmp(s[0], "left") == 0) side = t.child(parent, "start");
        if (side < 0 && std::strcmp(s[0], "right") == 0) side = t.child(parent, "end");
        if (side < 0) continue;
        int space = 0;
        QString lw;
        const QString v = borderValue(t, side, &space, &lw);
        if (v.isEmpty()) continue;
        x += " fo:border-" + QByteArray(s[1]) + "=\"" + v.toUtf8() + "\"";
        if (!lw.isEmpty()) x += " style:border-line-width-" + QByteArray(s[1]) + "=\"" + lw.toUtf8() + "\"";
        if (paragraph && v != QLatin1String("none")) x += " fo:padding-" + QByteArray(s[1]) + "=\"" + QByteArray::number(std::max(0, std::min(space, 31))) + "pt\"";
    }
    return x;
}

QByteArray Writer::textAttrs(const T &t, int rPr)
{
    QByteArray x;
    if (rPr < 0) return x;
    auto on = [&](int n) {
        const QString v = t.val(n);
        return v.isEmpty() || v == QLatin1String("1") || v == QLatin1String("true") || v == QLatin1String("on");
    };
    for (int c = t.firstChild(rPr); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (name == "rFonts") {
            auto face = [&](const char *a, const char *theme) {
                QString f = t.attr(c, a);
                if (f.isEmpty() && t.has(c, theme)) f = themeFont(t.attr(c, theme));
                return f;
            };
            const QString ascii = face("ascii", "asciiTheme"), hAnsi = face("hAnsi", "hAnsiTheme"), east = face("eastAsia", "eastAsiaTheme"), cs = face("cs", "cstheme");
            const QString latin = !ascii.isEmpty() ? ascii : hAnsi;
            if (!latin.isEmpty()) x += " style:font-name=\"" + esc(fontNameOf(latin)) + "\"";
            if (!east.isEmpty()) x += " style:font-name-asian=\"" + esc(fontNameOf(east)) + "\"";
            if (!cs.isEmpty()) x += " style:font-name-complex=\"" + esc(fontNameOf(cs)) + "\"";
        } else if (name == "b") x += on(c) ? " fo:font-weight=\"bold\" style:font-weight-asian=\"bold\" style:font-weight-complex=\"bold\"" : " fo:font-weight=\"normal\" style:font-weight-asian=\"normal\" style:font-weight-complex=\"normal\"";
        else if (name == "i") x += on(c) ? " fo:font-style=\"italic\" style:font-style-asian=\"italic\" style:font-style-complex=\"italic\"" : " fo:font-style=\"normal\" style:font-style-asian=\"normal\" style:font-style-complex=\"normal\"";
        else if (name == "caps") x += on(c) ? " fo:text-transform=\"uppercase\"" : " fo:text-transform=\"none\"";
        else if (name == "smallCaps") x += on(c) ? " fo:font-variant=\"small-caps\"" : " fo:font-variant=\"normal\"";
        else if (name == "strike") x += on(c) ? " style:text-line-through-style=\"solid\" style:text-line-through-type=\"single\"" : " style:text-line-through-style=\"none\"";
        else if (name == "dstrike") x += on(c) ? " style:text-line-through-style=\"solid\" style:text-line-through-type=\"double\"" : "";
        else if (name == "vanish") x += on(c) ? " text:display=\"none\"" : "";
        else if (name == "shadow") x += on(c) ? " fo:text-shadow=\"1pt 1pt\"" : "";
        else if (name == "emboss") x += on(c) ? " style:font-relief=\"embossed\"" : "";
        else if (name == "imprint") x += on(c) ? " style:font-relief=\"engraved\"" : "";
        else if (name == "color") {
            const QString v = t.val(c);
            if (v == QLatin1String("auto")) x += " style:use-window-font-color=\"true\"";
            else if (!hexColorOrEmpty(v).isEmpty()) x += " fo:color=\"#" + hexColorOrEmpty(v).toUtf8() + "\"";
        } else if (name == "spacing") {
            bool ok = false;
            const qint64 tw = t.val(c).toLongLong(&ok);
            if (ok) x += " fo:letter-spacing=\"" + inchesFromTwips(double(tw)).toUtf8() + "\"";
        } else if (name == "w") {
            const int v = t.val(c).toInt();
            if (v >= 1 && v <= 600) x += " style:text-scale=\"" + num(v) + "%\"";
        } else if (name == "sz") {
            const double v = t.val(c).toDouble();
            if (v >= 2 && v <= 3276) x += " fo:font-size=\"" + QByteArray::number(v / 2.0) + "pt\" style:font-size-asian=\"" + QByteArray::number(v / 2.0) + "pt\"";
        } else if (name == "szCs") {
            const double v = t.val(c).toDouble();
            if (v >= 2 && v <= 3276) x += " style:font-size-complex=\"" + QByteArray::number(v / 2.0) + "pt\"";
        } else if (name == "highlight") {
            static const QHash<QString, QString> colors = {{QStringLiteral("yellow"), QStringLiteral("FFFF00")}, {QStringLiteral("green"), QStringLiteral("00FF00")},
                                                           {QStringLiteral("cyan"), QStringLiteral("00FFFF")},   {QStringLiteral("magenta"), QStringLiteral("FF00FF")},
                                                           {QStringLiteral("blue"), QStringLiteral("0000FF")},   {QStringLiteral("red"), QStringLiteral("FF0000")},
                                                           {QStringLiteral("darkBlue"), QStringLiteral("000080")}, {QStringLiteral("darkCyan"), QStringLiteral("008080")},
                                                           {QStringLiteral("darkGreen"), QStringLiteral("008000")}, {QStringLiteral("darkMagenta"), QStringLiteral("800080")},
                                                           {QStringLiteral("darkRed"), QStringLiteral("800000")}, {QStringLiteral("darkYellow"), QStringLiteral("808000")},
                                                           {QStringLiteral("darkGray"), QStringLiteral("808080")}, {QStringLiteral("lightGray"), QStringLiteral("C0C0C0")},
                                                           {QStringLiteral("black"), QStringLiteral("000000")}, {QStringLiteral("white"), QStringLiteral("FFFFFF")}};
            const QString v = t.val(c);
            if (v == QLatin1String("none")) x += " fo:background-color=\"transparent\"";
            else if (colors.contains(v)) x += " fo:background-color=\"#" + colors.value(v).toLatin1() + "\"";
        } else if (name == "shd") {
            const QString c2 = shadeColor(t, c);
            if (!c2.isEmpty() && !x.contains("fo:background-color")) x += " fo:background-color=\"" + c2.toUtf8() + "\"";
        } else if (name == "u") {
            const QString v = t.val(c);
            if (v == QLatin1String("none")) x += " style:text-underline-style=\"none\"";
            else if (!v.isEmpty()) {
                QByteArray style = "solid", type = "single", width;
                if (v == QLatin1String("double")) type = "double";
                else if (v == QLatin1String("thick")) width = "bold";
                else if (v.startsWith(QLatin1String("dotted"))) style = "dotted";
                else if (v == QLatin1String("dash") || v == QLatin1String("dashedHeavy")) style = "dash";
                else if (v.startsWith(QLatin1String("dashLong"))) style = "long-dash";
                else if (v == QLatin1String("dotDash")) style = "dot-dash";
                else if (v == QLatin1String("dotDotDash")) style = "dot-dot-dash";
                else if (v.startsWith(QLatin1String("wav"))) style = "wave";
                if (v.endsWith(QLatin1String("Heavy"))) width = "bold";
                x += " style:text-underline-style=\"" + style + "\" style:text-underline-type=\"" + type + "\"" + (width.isEmpty() ? QByteArray() : " style:text-underline-width=\"" + width + "\"");
                const QString col = t.attr(c, "color");
                if (!hexColorOrEmpty(col).isEmpty()) x += " style:text-underline-color=\"#" + hexColorOrEmpty(col).toUtf8() + "\"";
                else x += " style:text-underline-color=\"font-color\"";
            }
        } else if (name == "vertAlign") {
            const QString v = t.val(c);
            if (v == QLatin1String("superscript")) x += " style:text-position=\"super 58%\"";
            else if (v == QLatin1String("subscript")) x += " style:text-position=\"sub 58%\"";
            else if (v == QLatin1String("baseline")) x += " style:text-position=\"0% 100%\"";
        } else if (name == "position") {
            const double v = t.val(c).toDouble();
            if (v != 0 && std::abs(v) < 1000) x += " style:text-position=\"" + QByteArray::number(v > 0 ? 33 : -33) + "% 100%\"";
        } else if (name == "lang") {
            auto split = [&](const QString &tag, const char *langAttr, const char *countryAttr) {
                if (tag.isEmpty()) return QByteArray();
                const QStringList parts = tag.split(QLatin1Char('-'));
                QByteArray r = " " + QByteArray(langAttr) + "=\"" + esc(parts.value(0)) + "\"";
                if (parts.size() > 1 && parts[1].size() == 2) r += " " + QByteArray(countryAttr) + "=\"" + esc(parts[1].toUpper()) + "\"";
                return r;
            };
            x += split(t.attr(c, "val"), "fo:language", "fo:country");
            x += split(t.attr(c, "eastAsia"), "style:language-asian", "style:country-asian");
            x += split(t.attr(c, "bidi"), "style:language-complex", "style:country-complex");
        }
    }
    return x;
}

QByteArray Writer::paragraphAttrs(const T &t, int pPr, bool inStyle, bool *breakBefore, QByteArray *tabs, int *outline)
{
    Q_UNUSED(inStyle);
    QByteArray x;
    if (pPr < 0) return x;
    auto on = [&](int n) {
        const QString v = t.val(n);
        return v.isEmpty() || v == QLatin1String("1") || v == QLatin1String("true") || v == QLatin1String("on");
    };
    for (int c = t.firstChild(pPr); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (name == "jc") {
            const QString v = t.val(c);
            const char *a = v == QLatin1String("center") ? "center" : v == QLatin1String("both") || v == QLatin1String("distribute") ? "justify" : (v == QLatin1String("right") || v == QLatin1String("end")) ? "end" : "start";
            x += QByteArray(" fo:text-align=\"") + a + "\"";
        } else if (name == "spacing") {
            if (t.has(c, "before")) x += " fo:margin-top=\"" + inchesFromTwips(t.attr(c, "before").toDouble()).toUtf8() + "\"";
            if (t.has(c, "after")) x += " fo:margin-bottom=\"" + inchesFromTwips(t.attr(c, "after").toDouble()).toUtf8() + "\"";
            if (t.has(c, "line")) {
                const double line = t.attr(c, "line").toDouble();
                const QString rule = t.attr(c, "lineRule");
                if (line > 0) {
                    if (rule == QLatin1String("exact")) x += " fo:line-height=\"" + inchesFromTwips(line).toUtf8() + "\"";
                    else if (rule == QLatin1String("atLeast")) x += " style:line-height-at-least=\"" + inchesFromTwips(line).toUtf8() + "\"";
                    else x += " fo:line-height=\"" + QByteArray::number(std::round(line / 240.0 * 100.0)) + "%\"";
                }
            }
        } else if (name == "ind") {
            const QString left = t.has(c, "left") ? t.attr(c, "left") : t.attr(c, "start"), right = t.has(c, "right") ? t.attr(c, "right") : t.attr(c, "end");
            if (!left.isEmpty()) x += " fo:margin-left=\"" + inchesFromTwips(left.toDouble()).toUtf8() + "\"";
            if (!right.isEmpty()) x += " fo:margin-right=\"" + inchesFromTwips(right.toDouble()).toUtf8() + "\"";
            if (t.has(c, "hanging") && t.attr(c, "hanging").toDouble() != 0) x += " fo:text-indent=\"" + inchesFromTwips(-t.attr(c, "hanging").toDouble()).toUtf8() + "\"";
            else if (t.has(c, "firstLine")) x += " fo:text-indent=\"" + inchesFromTwips(t.attr(c, "firstLine").toDouble()).toUtf8() + "\"";
            else if (t.has(c, "hanging")) x += " fo:text-indent=\"0in\"";
        } else if (name == "contextualSpacing") x += on(c) ? " style:contextual-spacing=\"true\"" : " style:contextual-spacing=\"false\"";
        else if (name == "keepNext") x += on(c) ? " fo:keep-with-next=\"always\"" : " fo:keep-with-next=\"auto\"";
        else if (name == "keepLines") x += on(c) ? " fo:keep-together=\"always\"" : " fo:keep-together=\"auto\"";
        else if (name == "pageBreakBefore") {
            if (on(c) && breakBefore) *breakBefore = true;
            else if (on(c)) x += " fo:break-before=\"page\"";
        } else if (name == "widowControl") x += on(c) ? " fo:orphans=\"2\" fo:widows=\"2\"" : " fo:orphans=\"0\" fo:widows=\"0\"";
        else if (name == "bidi") x += on(c) ? " style:writing-mode=\"rl-tb\"" : "";
        else if (name == "shd") {
            const QString col = shadeColor(t, c);
            if (!col.isEmpty()) x += " fo:background-color=\"" + col.toUtf8() + "\"";
        } else if (name == "pBdr") x += bordersAttrs(t, c, true);
        else if (name == "outlineLvl") {
            if (outline) *outline = t.val(c).toInt() + 1;
        } else if (name == "tabs" && tabs) {
            QByteArray tx;
            for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                if (!t.is(k, "tab")) continue;
                const QString val = t.attr(k, "val");
                if (val == QLatin1String("clear") || !t.has(k, "pos")) continue;
                const char *type = val == QLatin1String("center") ? "center" : val == QLatin1String("right") || val == QLatin1String("end") ? "right" : val == QLatin1String("decimal") ? "char" : "left";
                tx += "<style:tab-stop style:position=\"" + inchesFromTwips(t.attr(k, "pos").toDouble()).toUtf8() + "\" style:type=\"" + type + "\"";
                if (std::strcmp(type, "char") == 0) tx += " style:char=\".\"";
                const QString lead = t.attr(k, "leader");
                if (lead == QLatin1String("dot")) tx += " style:leader-style=\"dotted\" style:leader-text=\".\"";
                else if (lead == QLatin1String("hyphen")) tx += " style:leader-style=\"solid\" style:leader-text=\"-\"";
                else if (lead == QLatin1String("underscore") || lead == QLatin1String("heavy")) tx += " style:leader-style=\"solid\" style:leader-text=\"_\"";
                tx += "/>";
            }
            if (!tx.isEmpty()) *tabs = "<style:tab-stops>" + tx + "</style:tab-stops>";
        }
    }
    return x;
}

QByteArray Writer::defaultStyles()
{
    QByteArray x;
    // The document's defaults.
    QByteArray pAttrs, tAttrs, tabs;
    if (m_styles) {
        const T &t = *m_styles;
        const int dd = t.child(t.root(), "docDefaults");
        if (dd >= 0) {
            const int rd = t.child(dd, "rPrDefault"), pd = t.child(dd, "pPrDefault");
            if (rd >= 0) tAttrs = textAttrs(t, t.child(rd, "rPr"));
            if (pd >= 0) pAttrs = paragraphAttrs(t, t.child(pd, "pPr"), true, nullptr, &tabs, nullptr);
        }
    }
    if (!tAttrs.contains("fo:font-size")) tAttrs += " fo:font-size=\"10pt\"";
    QByteArray tabDistance;
    if (m_settings) {
        const int dt = m_settings->child(m_settings->root(), "defaultTabStop");
        if (dt >= 0) tabDistance = " style:tab-stop-distance=\"" + inchesFromTwips(m_settings->val(dt).toDouble()).toUtf8() + "\"";
    }
    x += "<style:default-style style:family=\"paragraph\"><style:paragraph-properties" + pAttrs + tabDistance + "/><style:text-properties" + tAttrs + "/></style:default-style>";
    x += "<style:default-style style:family=\"table-row\"><style:table-row-properties fo:keep-together=\"auto\"/></style:default-style>";
    return x;
}

QByteArray Writer::stylesPart()
{
    QByteArray x;
    x += defaultStyles();
    bool standard = false;
    for (const QString &id : std::as_const(m_styleOrder)) {
        const DStyle &s = m_dstyles[id];
        const bool paragraph = s.type == QLatin1String("paragraph"), character = s.type == QLatin1String("character");
        if (!paragraph && !character) continue;
        if (character && id == m_defaultCharStyle) continue;
        const T &t = *m_styles;
        if (paragraph && s.odfName == QLatin1String("Standard")) standard = true;
        QString display = s.name;
        for (const Map &m : kNames)
            if (display == QLatin1String(m.docx)) display = QString::fromLatin1(m.odf);
        x += "<style:style style:name=\"" + esc(s.odfName) + "\" style:display-name=\"" + esc(display) + "\" style:family=\"" + (paragraph ? "paragraph" : "text") + "\"";
        if (!s.basedOn.isEmpty() && m_dstyles.contains(s.basedOn) && s.basedOn != m_defaultCharStyle) x += " style:parent-style-name=\"" + esc(m_dstyles[s.basedOn].odfName) + "\"";
        if (paragraph && !s.next.isEmpty() && m_dstyles.contains(s.next)) x += " style:next-style-name=\"" + esc(m_dstyles[s.next].odfName) + "\"";
        QByteArray pAttrs, tabs;
        int outline = 0;
        int pPr = t.child(s.node, "pPr"), rPr = t.child(s.node, "rPr");
        if (paragraph) pAttrs = paragraphAttrs(t, pPr, true, nullptr, &tabs, &outline);
        // A style whose paragraphs are numbered: the list style it names.
        QByteArray listAttr;
        if (paragraph && pPr >= 0) {
            const int np = t.child(pPr, "numPr");
            if (np >= 0) {
                const int idn = t.child(np, "numId");
                const int numId = idn >= 0 ? t.val(idn).toInt() : -1;
                if (numId > 0 && m_numDefs.contains(numId)) {
                    m_numsUsed.insert(numId);
                    listAttr = " style:list-style-name=\"L" + num(numId) + "\"";
                }
            }
        }
        if (paragraph && outline > 0 && outline <= 10) x += " style:default-outline-level=\"" + num(outline) + "\"";
        x += " style:class=\"" + QByteArray(paragraph ? (outline > 0 ? "text" : "text") : "text") + "\"";
        x += listAttr + ">";
        if (paragraph && (!pAttrs.isEmpty() || !tabs.isEmpty())) x += "<style:paragraph-properties" + pAttrs + (tabs.isEmpty() ? "/>" : ">" + tabs + "</style:paragraph-properties>");
        const QByteArray tAttrs = textAttrs(t, rPr);
        if (!tAttrs.isEmpty()) x += "<style:text-properties" + tAttrs + "/>";
        x += "</style:style>";
    }
    if (!standard) x = "<style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\"/>" + x;
    // The styles a frame and a picture are given.
    x += "<style:style style:name=\"Graphics\" style:family=\"graphic\"><style:graphic-properties text:anchor-type=\"paragraph\" svg:x=\"0in\" svg:y=\"0in\" style:wrap=\"none\" "
         "style:vertical-pos=\"top\" style:vertical-rel=\"paragraph\" style:horizontal-pos=\"center\" style:horizontal-rel=\"paragraph\"/></style:style>";
    x += "<style:style style:name=\"Frame\" style:family=\"graphic\"><style:graphic-properties text:anchor-type=\"paragraph\" svg:x=\"0in\" svg:y=\"0in\" fo:margin-left=\"0.0791in\" "
         "fo:margin-right=\"0.0791in\" fo:margin-top=\"0.0791in\" fo:margin-bottom=\"0.0791in\" style:wrap=\"parallel\" style:number-wrapped-paragraphs=\"no-limit\" "
         "style:vertical-pos=\"top\" style:vertical-rel=\"paragraph-content\" style:horizontal-pos=\"center\" style:horizontal-rel=\"paragraph-content\"/></style:style>";
    return x;
}

// ---- numbering

void Writer::collectNumbering()
{
    if (!m_numbering) return;
    const T &t = *m_numbering;
    for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c)) {
        if (t.is(c, "abstractNum")) {
            AbstractDef def;
            for (int l = t.firstChild(c); l >= 0; l = t.next(l)) {
                if (!t.is(l, "lvl")) continue;
                const int ilvl = t.attr(l, "ilvl").toInt();
                if (ilvl < 0 || ilvl > 8) continue;
                LevelDef &ld = def.level[ilvl];
                ld.defined = true;
                for (int k = t.firstChild(l); k >= 0; k = t.next(k)) {
                    if (t.is(k, "start")) ld.start = t.val(k).toInt();
                    else if (t.is(k, "numFmt")) ld.fmt = t.val(k);
                    else if (t.is(k, "lvlText")) ld.text = t.val(k);
                    else if (t.is(k, "lvlJc")) ld.jc = t.val(k);
                    else if (t.is(k, "suff")) ld.suff = t.val(k);
                    else if (t.is(k, "pPr")) {
                        const int ind = t.child(k, "ind");
                        if (ind >= 0) {
                            ld.left = clampedInt(t.has(ind, "left") ? t.attr(ind, "left").toDouble() : t.attr(ind, "start").toDouble());
                            ld.hanging = clampedInt(t.attr(ind, "hanging").toDouble());
                            if (t.has(ind, "firstLine") && ld.hanging == 0) ld.hanging = -clampedInt(t.attr(ind, "firstLine").toDouble());
                        }
                    }
                }
            }
            m_abstracts.insert(t.attr(c, "abstractNumId").toInt(), def);
        } else if (t.is(c, "num")) {
            NumDef nd;
            for (int i = 0; i < 9; ++i) nd.startOverride[i] = -1;
            const int an = t.child(c, "abstractNumId");
            nd.abstractId = an >= 0 ? t.val(an).toInt() : -1;
            for (int o = t.firstChild(c); o >= 0; o = t.next(o)) {
                if (!t.is(o, "lvlOverride")) continue;
                const int ilvl = t.attr(o, "ilvl").toInt();
                if (ilvl < 0 || ilvl > 8) continue;
                const int so = t.child(o, "startOverride");
                if (so >= 0) nd.startOverride[ilvl] = t.val(so).toInt();
            }
            m_numDefs.insert(t.attr(c, "numId").toInt(), nd);
        }
    }
}

namespace {

QString bulletFor(const QString &text)
{
    if (text.isEmpty()) return QStringLiteral("•");
    const ushort u = text[0].unicode();
    switch (u) {
    case 0xF0B7: return QStringLiteral("•");
    case 0xF0A7: return QStringLiteral("▪");
    case 0xF0D8: return QStringLiteral("➢");
    case 0xF0FC: return QStringLiteral("✔");
    case 0xF076: return QStringLiteral("❖");
    case 0xF0A8: return QStringLiteral("◻");
    case 'o': return QStringLiteral("◦");
    default: break;
    }
    if (u >= 0xE000 && u <= 0xF8FF) return QStringLiteral("•");
    if (u < 0x20) return QStringLiteral("•");
    return text.left(1);
}

} // namespace

QByteArray Writer::listStyleXml(int numId)
{
    const NumDef nd = m_numDefs.value(numId);
    const AbstractDef def = m_abstracts.value(nd.abstractId);
    QByteArray x = "<text:list-style style:name=\"L" + num(numId) + "\">";
    for (int lv = 0; lv < 9; ++lv) {
        LevelDef l = def.level[lv];
        if (!l.defined) {
            l.defined = true;
            l.fmt = QStringLiteral("bullet");
            l.text = QStringLiteral("•");
            l.left = 720 * (lv + 1);
            l.hanging = 360;
        }
        const QByteArray indent = "<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\"><style:list-level-label-alignment text:label-followed-by=\"" +
                                  QByteArray(l.suff == QLatin1String("space") ? "space" : l.suff == QLatin1String("nothing") ? "nothing" : "listtab") + "\" text:list-tab-stop-position=\"" +
                                  inchesFromTwips(double(l.left)).toUtf8() + "\" fo:text-indent=\"" + inchesFromTwips(double(-l.hanging)).toUtf8() + "\" fo:margin-left=\"" +
                                  inchesFromTwips(double(l.left)).toUtf8() + "\"/></style:list-level-properties>";
        if (l.fmt == QLatin1String("bullet")) {
            x += "<text:list-level-style-bullet text:level=\"" + num(lv + 1) + "\" text:bullet-char=\"" + esc(bulletFor(l.text)) + "\">" + indent + "</text:list-level-style-bullet>";
            continue;
        }
        // The label: the text round the %n marks, and how many levels it shows.
        QString prefix, suffix;
        int shown = 0;
        {
            static const QRegularExpression re(QStringLiteral("%([1-9])"));
            const QString text = l.text;
            qsizetype firstAt = -1, lastEnd = 0;
            int lowest = lv + 1;
            for (auto it = re.globalMatch(text); it.hasNext();) {
                const auto m = it.next();
                if (firstAt < 0) firstAt = m.capturedStart();
                lastEnd = m.capturedEnd();
                lowest = std::min(lowest, m.captured(1).toInt());
                ++shown;
            }
            if (firstAt >= 0) {
                prefix = text.left(firstAt);
                suffix = text.mid(lastEnd);
                shown = std::max(1, lv + 1 - lowest + 1);
            } else {
                prefix = text;
                shown = 1;
            }
        }
        static const QHash<QString, QByteArray> fmts = {{QStringLiteral("decimal"), "1"},      {QStringLiteral("lowerLetter"), "a"}, {QStringLiteral("upperLetter"), "A"},
                                                         {QStringLiteral("lowerRoman"), "i"},   {QStringLiteral("upperRoman"), "I"},  {QStringLiteral("decimalZero"), "01"},
                                                         {QStringLiteral("none"), ""}};
        const QByteArray fmt = fmts.value(l.fmt, "1");
        int start = l.start;
        if (nd.startOverride[lv] >= 0) start = nd.startOverride[lv];
        x += "<text:list-level-style-number text:level=\"" + num(lv + 1) + "\" style:num-format=\"" + fmt + "\"" + (prefix.isEmpty() ? QByteArray() : " style:num-prefix=\"" + esc(prefix) + "\"") +
             (suffix.isEmpty() ? QByteArray() : " style:num-suffix=\"" + esc(suffix) + "\"") + " text:display-levels=\"" + num(std::min(shown, lv + 1)) + "\" text:start-value=\"" + num(std::max(0, start)) + "\">" +
             indent + "</text:list-level-style-number>";
    }
    return x + "</text:list-style>";
}

// ---- the sections

SectionInfo Writer::sectionOf(const T &t, int sectPr)
{
    SectionInfo s;
    s.sectPr = sectPr;
    if (sectPr < 0) return s;
    auto tw = [&](int n, const char *a, qint64 fallback) {
        const QString v = t.attr(n, a);
        bool ok = false;
        const double d = v.toDouble(&ok);
        return ok ? clampedInt(std::round(d)) : fallback;
    };
    for (int c = t.firstChild(sectPr); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if (name == "pgSz") {
            s.pageW = std::max<qint64>(720, tw(c, "w", s.pageW));
            s.pageH = std::max<qint64>(720, tw(c, "h", s.pageH));
            s.landscape = t.attr(c, "orient") == QLatin1String("landscape");
        } else if (name == "pgMar") {
            s.top = tw(c, "top", s.top);
            s.bottom = tw(c, "bottom", s.bottom);
            s.left = tw(c, "left", s.left);
            s.right = tw(c, "right", s.right);
            s.header = tw(c, "header", s.header);
            s.footer = tw(c, "footer", s.footer);
        } else if (name == "type") s.continuous = t.val(c) == QLatin1String("continuous");
        else if (name == "titlePg") s.titlePg = t.val(c).isEmpty() || t.val(c) == QLatin1String("1") || t.val(c) == QLatin1String("true");
        else if (name == "cols") {
            s.columns = std::max(1, t.has(c, "num") ? t.attr(c, "num").toInt() : 1);
            s.columnGap = tw(c, "space", 720);
            s.separator = t.attr(c, "sep") == QLatin1String("1") || t.attr(c, "sep") == QLatin1String("true");
        } else if (name == "pgNumType") {
            if (t.has(c, "start")) s.pageStart = t.attr(c, "start").toInt();
        } else if (name == "headerReference" || name == "footerReference") {
            const QString type = t.attr(c, "type");
            const int slot = type == QLatin1String("even") ? 1 : type == QLatin1String("first") ? 2 : 0;
            (name == "headerReference" ? s.headerRef : s.footerRef)[slot] = t.attr(c, "id", wml::NsR);
        }
    }
    return s;
}

QString Writer::masterFor(const SectionInfo &s)
{
    // A master page for each distinct page: its size, margins, columns, and what its headers and footers are.
    QString key = QStringLiteral("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12|").arg(s.pageW).arg(s.pageH).arg(s.top).arg(s.bottom).arg(s.left).arg(s.right).arg(s.header).arg(s.footer).arg(s.landscape).arg(s.columns).arg(s.columnGap).arg(s.separator);
    for (int i = 0; i < 3; ++i) key += s.headerRef[i] + QLatin1Char('/') + s.footerRef[i] + QLatin1Char(';');
    key += s.titlePg ? QLatin1Char('T') : QLatin1Char('-');
    key += QString::number(s.pageStart);
    const auto it = m_masterByKey.constFind(key);
    if (it != m_masterByKey.constEnd()) return it.value();
    const QString name = QStringLiteral("MP%1").arg(m_masters.size() + 1);
    SectionInfo copy = s;
    copy.master = name;
    m_masters.push_back(copy);
    m_masterByKey.insert(key, name);
    return name;
}

QByteArray Writer::masterPages()
{
    QByteArray layouts, masters;
    int n = 0;
    for (const SectionInfo &s : std::as_const(m_masters)) {
        ++n;
        const bool hasHeader = !s.headerRef[0].isEmpty() || !s.headerRef[1].isEmpty() || !s.headerRef[2].isEmpty();
        const bool hasFooter = !s.footerRef[0].isEmpty() || !s.footerRef[1].isEmpty() || !s.footerRef[2].isEmpty();
        // The page's top margin is the header's distance from the edge, and the header sits in what the text's margin has beyond it.
        const qint64 top = hasHeader ? std::min(s.header, s.top) : s.top, bottom = hasFooter ? std::min(s.footer, s.bottom) : s.bottom;
        const qint64 headerHeight = hasHeader ? std::max<qint64>(0, s.top - top) : 0, footerHeight = hasFooter ? std::max<qint64>(0, s.bottom - bottom) : 0;
        layouts += "<style:page-layout style:name=\"pm" + num(n) + "\"><style:page-layout-properties fo:page-width=\"" + inchesFromTwips(double(s.pageW)).toUtf8() + "\" fo:page-height=\"" +
                   inchesFromTwips(double(s.pageH)).toUtf8() + "\" style:num-format=\"1\" style:print-orientation=\"" + (s.landscape ? "landscape" : "portrait") + "\" fo:margin-top=\"" +
                   inchesFromTwips(double(top)).toUtf8() + "\" fo:margin-bottom=\"" + inchesFromTwips(double(bottom)).toUtf8() + "\" fo:margin-left=\"" + inchesFromTwips(double(s.left)).toUtf8() +
                   "\" fo:margin-right=\"" + inchesFromTwips(double(s.right)).toUtf8() + "\"" + (s.pageStart > 0 ? " style:first-page-number=\"" + num(s.pageStart) + "\"" : QByteArray()) +
                   " style:writing-mode=\"lr-tb\" style:footnote-max-height=\"0in\">";
        if (s.columns > 1) layouts += "<style:columns fo:column-count=\"" + num(s.columns) + "\" fo:column-gap=\"" + inchesFromTwips(double(s.columnGap)).toUtf8() + "\">" + (s.separator ? "<style:column-sep style:width=\"0.0069in\" style:color=\"#000000\" style:height=\"100%\" style:style=\"solid\"/>" : "") + "</style:columns>";
        layouts += "</style:page-layout-properties>";
        layouts += "<style:header-style><style:header-footer-properties fo:min-height=\"" + inchesFromTwips(double(headerHeight)).toUtf8() + "\" fo:margin-left=\"0in\" fo:margin-right=\"0in\" fo:margin-bottom=\"0in\"/></style:header-style>";
        layouts += "<style:footer-style><style:header-footer-properties fo:min-height=\"" + inchesFromTwips(double(footerHeight)).toUtf8() + "\" fo:margin-left=\"0in\" fo:margin-right=\"0in\" fo:margin-top=\"0in\"/></style:footer-style>";
        layouts += "</style:page-layout>";
        masters += "<style:master-page style:name=\"" + s.master.toUtf8() + "\" style:page-layout-name=\"pm" + num(n) + "\">";
        auto part = [&](const char *tag, const QString &rid, bool header) {
            if (rid.isEmpty()) return;
            masters += "<style:" + QByteArray(tag) + ">" + headerBody(rid, header, 1) + "</style:" + tag + ">";
        };
        part("header", s.headerRef[0], true);
        if (m_evenOdd) part("header-left", s.headerRef[1].isEmpty() ? s.headerRef[0] : s.headerRef[1], true);
        if (s.titlePg) part("header-first", s.headerRef[2], true);
        part("footer", s.footerRef[0], false);
        if (m_evenOdd) part("footer-left", s.footerRef[1].isEmpty() ? s.footerRef[0] : s.footerRef[1], false);
        if (s.titlePg) part("footer-first", s.footerRef[2], false);
        masters += "</style:master-page>";
    }
    if (m_masters.isEmpty()) {
        layouts = "<style:page-layout style:name=\"pm1\"><style:page-layout-properties fo:page-width=\"8.5in\" fo:page-height=\"11in\" style:print-orientation=\"portrait\" fo:margin-top=\"1in\" "
                  "fo:margin-bottom=\"1in\" fo:margin-left=\"1in\" fo:margin-right=\"1in\"/></style:page-layout>";
        masters = "<style:master-page style:name=\"MP1\" style:page-layout-name=\"pm1\"/>";
    }
    m_layouts = layouts;
    return masters;
}

QByteArray Writer::headerBody(const QString &rid, bool header, int depth)
{
    Q_UNUSED(header);
    const QString key = rid;
    const auto it = m_headerCache.constFind(key);
    if (it != m_headerCache.constEnd()) return it.value();
    const QString saved = m_source;
    m_source = m_main;
    const QString part = partOf(rid);
    m_source = saved;
    QByteArray out;
    if (!part.isEmpty()) {
        T *tree = parse(part);
        if (tree) {
            const QString oldSource = m_source;
            StyleSink *oldSink = m_sink;
            m_source = part;
            m_sink = &m_stylesSink;
            emitBlocks(*tree, tree->root(), &out, depth, false);
            m_source = oldSource;
            m_sink = oldSink;
        }
    }
    if (out.isEmpty()) out = "<text:p text:style-name=\"Standard\"/>";
    m_headerCache.insert(key, out);
    return out;
}

// ---- the whole

QByteArray Writer::metaXml()
{
    QByteArray x = QByteArray(kXml) + "<office:document-meta" + kOdfNamespaces + " office:version=\"1.2\"><office:meta><meta:generator>JeffOffice</meta:generator>";
    const auto rels = m_pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/metadata/core-properties"));
    if (!rels.isEmpty() && m_pkg.hasPart(rels.first().target)) {
        T core;
        if (core.parse(m_pkg.part(rels.first().target), nullptr)) {
            QByteArray title, subject, creator, description, keywords, created, modified, last;
            for (int c = core.firstChild(core.root()); c >= 0; c = core.next(c)) {
                const QByteArray name = core.localName(c);
                const QByteArray v = esc(core.text(c), false);
                if (name == "title") title = v;
                else if (name == "subject") subject = v;
                else if (name == "creator") creator = v;
                else if (name == "description") description = v;
                else if (name == "keywords") keywords = v;
                else if (name == "created") created = v;
                else if (name == "modified") modified = v;
                else if (name == "lastModifiedBy") last = v;
            }
            if (!title.isEmpty()) x += "<dc:title>" + title + "</dc:title>";
            if (!description.isEmpty()) x += "<dc:description>" + description + "</dc:description>";
            if (!subject.isEmpty()) x += "<dc:subject>" + subject + "</dc:subject>";
            if (!keywords.isEmpty()) x += "<meta:keyword>" + keywords + "</meta:keyword>";
            if (!creator.isEmpty()) x += "<meta:initial-creator>" + creator + "</meta:initial-creator>";
            if (!created.isEmpty()) x += "<meta:creation-date>" + created.chopped(created.endsWith('Z') ? 1 : 0) + "</meta:creation-date>";
            if (!last.isEmpty() || !creator.isEmpty()) x += "<dc:creator>" + (last.isEmpty() ? creator : last) + "</dc:creator>";
            if (!modified.isEmpty()) x += "<dc:date>" + modified.chopped(modified.endsWith('Z') ? 1 : 0) + "</dc:date>";
        }
    }
    return x + "</office:meta></office:document-meta>";
}

QByteArray Writer::settingsXml() const
{
    // How the text is laid out: the settings a program that reads .docx files writes with the document (the spacing of paragraphs in tables, tabs, the look of an empty line's mark, line height), which is what these documents were made for; the settings of a file that says none are older ones.
    static const struct {
        const char *name, *type, *value;
    } kFlags[] = {{"AddExternalLeading", "boolean", "true"},
                  {"AddFrameOffsets", "boolean", "true"},
                  {"AddParaLineSpacingToTableCells", "boolean", "true"},
                  {"AddParaSpacingToTableCells", "boolean", "true"},
                  {"AddParaTableSpacing", "boolean", "false"},
                  {"AddParaTableSpacingAtStart", "boolean", "true"},
                  {"AddVerticalFrameOffsets", "boolean", "true"},
                  {"AlignTabStopPosition", "boolean", "true"},
                  {"ApplyParagraphMarkFormatToEmptyLineAtEndOfParagraph", "boolean", "true"},
                  {"ApplyParagraphMarkFormatToNumbering", "boolean", "true"},
                  {"ApplyTextAttrToEmptyLineAtEndOfParagraph", "boolean", "false"},
                  {"AutoFirstLineIndentDisregardLineSpace", "boolean", "true"},
                  {"BackgroundParaOverDrawings", "boolean", "true"},
                  {"CharacterCompressionType", "short", "0"},
                  {"ClipAsCharacterAnchoredWriterFlyFrames", "boolean", "false"},
                  {"ClippedPictures", "boolean", "true"},
                  {"CollapseEmptyCellPara", "boolean", "true"},
                  {"ConsiderTextWrapOnObjPos", "boolean", "true"},
                  {"ContinuousEndnotes", "boolean", "true"},
                  {"DisableOffPagePositioning", "boolean", "true"},
                  {"DoNotBreakWrappedTables", "boolean", "true"},
                  {"DoNotCaptureDrawObjsOnPage", "boolean", "true"},
                  {"DoNotJustifyLinesWithManualBreak", "boolean", "false"},
                  {"DoNotMirrorRtlDrawObjs", "boolean", "true"},
                  {"DoNotResetParaAttrsForNumFont", "boolean", "false"},
                  {"DropCapPunctuation", "boolean", "true"},
                  {"FootnoteInColumnToPageEnd", "boolean", "true"},
                  {"FrameAutowidthWithMorePara", "boolean", "true"},
                  {"GutterAtTop", "boolean", "false"},
                  {"HeaderSpacingBelowLastPara", "boolean", "true"},
                  {"HyphenateURLs", "boolean", "false"},
                  {"IgnoreFirstLineIndentInNumbering", "boolean", "false"},
                  {"IgnoreHiddenCharsForLineCalculation", "boolean", "true"},
                  {"IgnoreTabsAndBlanksForLineCalculation", "boolean", "true"},
                  {"ImagePreferredDPI", "int", "0"},
                  {"InvertBorderSpacing", "boolean", "true"},
                  {"IsKernAsianPunctuation", "boolean", "false"},
                  {"JustifyLinesWithShrinking", "boolean", "false"},
                  {"MathBaselineAlignment", "boolean", "true"},
                  {"MinRowHeightInclBorder", "boolean", "true"},
                  {"MsWordCompGridMetrics", "boolean", "true"},
                  {"MsWordCompMinLineHeightByFly", "boolean", "true"},
                  {"MsWordCompTrailingBlanks", "boolean", "true"},
                  {"NoClippingWithWrapPolygon", "boolean", "true"},
                  {"NoGapAfterNoteNumber", "boolean", "true"},
                  {"NoNumberingShowFollowBy", "boolean", "true"},
                  {"OutlineLevelYieldsNumbering", "boolean", "false"},
                  {"PaintHellOverHeaderFooter", "boolean", "true"},
                  {"PropLineSpacingShrinksFirstLine", "boolean", "true"},
                  {"SmallCapsPercentage66", "boolean", "false"},
                  {"SubtractFlysAnchoredAtFlys", "boolean", "false"},
                  {"SurroundTextWrapSmall", "boolean", "true"},
                  {"TabAtLeftIndentForParagraphsInList", "boolean", "true"},
                  {"TabOverMargin", "boolean", "true"},
                  {"TabOverSpacing", "boolean", "true"},
                  {"TabOverflow", "boolean", "true"},
                  {"TableRowKeep", "boolean", "true"},
                  {"TabsRelativeToIndent", "boolean", "false"},
                  {"TreatSingleColumnBreakAsPageBreak", "boolean", "true"},
                  {"UnbreakableNumberings", "boolean", "true"},
                  {"UnxForceZeroExtLeading", "boolean", "false"},
                  {"UseFormerLineSpacing", "boolean", "false"},
                  {"UseFormerObjectPositioning", "boolean", "false"},
                  {"UseFormerTextWrapping", "boolean", "false"},
                  {"UseOldNumbering", "boolean", "false"},
                  {"UseOldPrinterMetrics", "boolean", "false"},
                  {"UseVariableWidthNBSP", "boolean", "false"}};
    QByteArray x = QByteArray(kXml) + "<office:document-settings" + kOdfNamespaces + " xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\" xmlns:ooo=\"http://openoffice.org/2004/office\" office:version=\"1.2\"><office:settings>"
                                       "<config:config-item-set config:name=\"ooo:configuration-settings\">";
    if (!m_odfItems.isEmpty()) return x + m_odfItems + "</config:config-item-set></office:settings></office:document-settings>";
    for (const auto &f : kFlags) x += QByteArray("<config:config-item config:name=\"") + f.name + "\" config:type=\"" + f.type + "\">" + f.value + "</config:config-item>";
    return x + "</config:config-item-set></office:settings></office:document-settings>";
}

QByteArray Writer::fontFacesXml() const
{
    QByteArray faces;
    for (auto it = m_fontFaces.constBegin(); it != m_fontFaces.constEnd(); ++it) {
        if (m_systemFaces.contains(it.key())) {
            const QString family = m_systemFaces.value(it.key());
            faces += "<style:font-face style:name=\"" + esc(it.key()) + "\" svg:font-family=\"" + esc(family.contains(QLatin1Char(' ')) ? QStringLiteral("'") + family + QStringLiteral("'") : family) +
                     "\" style:font-family-generic=\"system\" style:font-pitch=\"variable\"/>";
            continue;
        }
        const QString n = it.key().toLower();
        QString kind = m_fontKinds.value(it.key()).first;
        const QString pitchIn = m_fontKinds.value(it.key()).second;
        if (kind.isEmpty() || kind == QLatin1String("auto")) {
            // (A font the table says nothing of: by its name.)
            static const char *const mono[] = {"courier", "mono", "consolas", "lucida console", "typewriter"};
            static const char *const serif[] = {"times", "serif", "georgia", "cambria", "garamond", "palatino", "bookman", "century", "minion", "baskerville", "caslon", "didot", "book antiqua"};
            kind = QStringLiteral("swiss");
            for (const char *m : mono)
                if (n.contains(QLatin1String(m))) kind = QStringLiteral("modern");
            if (kind == QLatin1String("swiss") && !n.contains(QLatin1String("sans")))
                for (const char *m : serif)
                    if (n.contains(QLatin1String(m))) kind = QStringLiteral("roman");
        }
        const bool fixed = pitchIn == QLatin1String("fixed") || (pitchIn.isEmpty() && kind == QLatin1String("modern"));
        static const QSet<QString> kinds = {QStringLiteral("roman"), QStringLiteral("swiss"), QStringLiteral("modern"), QStringLiteral("script"), QStringLiteral("decorative")};
        const QByteArray generic = kinds.contains(kind) ? " style:font-family-generic=\"" + kind.toLatin1() + "\" style:font-pitch=\"" + (fixed ? "fixed" : "variable") + "\"" : QByteArray();
        faces += "<style:font-face style:name=\"" + esc(it.key()) + "\" svg:font-family=\"" + esc(it.key().contains(QLatin1Char(' ')) ? QStringLiteral("'") + it.key() + QStringLiteral("'") : it.key()) + "\"" + generic + "/>";
    }
    return faces;
}

QByteArray Writer::contentXml()
{
    const QByteArray faces = fontFacesXml();
    QByteArray lists;
    {
        // (In the order of the numbers: a file is the same bytes whatever the order of a hash.)
        QList<int> ids = m_numsUsed.values();
        std::sort(ids.begin(), ids.end());
        for (int numId : std::as_const(ids)) lists += listStyleXml(numId);
    }
    QByteArray decls;
    if (!m_sequences.isEmpty()) {
        decls = "<text:sequence-decls>";
        QStringList names = m_sequences.values();
        names.sort();
        for (const QString &s : std::as_const(names)) decls += "<text:sequence-decl text:display-outline-level=\"0\" text:name=\"" + esc(s) + "\"/>";
        decls += "</text:sequence-decls>";
    }
    return QByteArray(kXml) + "<office:document-content" + kOdfNamespaces + " office:version=\"1.2\"><office:scripts/><office:font-face-decls>" + faces + "</office:font-face-decls><office:automatic-styles>" +
           m_content.xml + lists + "</office:automatic-styles><office:body><office:text>" + (m_changeRegions.isEmpty() ? QByteArray() : "<text:tracked-changes>" + m_changeRegions + "</text:tracked-changes>") + decls + m_bodyXml + "</office:text></office:body></office:document-content>";
}

bool Writer::run(const QByteArray &docx, QByteArray *out, QString *error, QStringList *notes)
{
    m_notes = notes;
    m_stylesSink.prefix = QStringLiteral("H");
    QString why;
    if (!m_pkg.read(docx, &why)) {
        if (error) *error = why;
        return false;
    }
    if (!loadParts(error)) return false;
    m_source = m_main;
    collectStyles();
    collectNumbering();
    if (m_settings) m_evenOdd = m_settings->child(m_settings->root(), "evenAndOddHeaders") >= 0;
    collectSections();
    // Tracked changes that an .odt has no place for stay as they stand now.
    {
        static const QSet<QByteArray> unkept = {"tblPrChange", "tcPrChange", "trPrChange", "tblGridChange", "sectPrChange", "cellIns", "cellDel", "cellMerge", "tblPrExChange", "numberingChange"};
        for (int i = 0; i < m_doc->count(); ++i) {
            const QByteArray local = m_doc->localName(i);
            if (local.size() > 6 && (local.endsWith("Change") || local.startsWith("cell")) && unkept.contains(local)) {
                left(QCoreApplication::translate("odt", "tracked changes to tables, columns, and page setup (saved as they stand now)"));
                break;
            }
        }
    }
    // The body (which makes the automatic styles, the list styles used, and the master pages' headers).
    m_sectionIndex = 0;
    emitBlocks(*m_doc, m_doc->child(m_doc->root(), "body"), &m_bodyXml, 0, true);
    if (!m_pendingBookmarks.isEmpty()) {
        // (Marks that wait for a paragraph to come: the last one ends where the last paragraph does, so that no mark is left without its mate.)
        const qsizetype at = std::max(m_bodyXml.lastIndexOf("</text:p>"), m_bodyXml.lastIndexOf("</text:h>"));
        if (at >= 0) m_bodyXml.insert(at, m_pendingBookmarks);
        m_pendingBookmarks.clear();
    }
    if (m_inToc) {
        m_bodyXml += "</text:index-body></text:table-of-content>";
        m_inToc = false;
    }
    while (m_sectionWrapperOpen) {
        m_bodyXml += "</text:section>";
        m_sectionWrapperOpen = false;
    }
    const QByteArray masters = masterPages();
    // Pictures and the parts.
    jp::ZipWriter zip;
    zip.add(QStringLiteral("mimetype"), QByteArray("application/vnd.oasis.opendocument.text"), false);
    const QByteArray styles = stylesPart();   // (after the body: it names the list styles it uses)
    const QByteArray faces = fontFacesXml();
    zip.add(QStringLiteral("content.xml"), contentXml(), true);
    QByteArray stylesXml = QByteArray(kXml) + "<office:document-styles" + kOdfNamespaces + " office:version=\"1.2\"><office:font-face-decls>" + faces + "</office:font-face-decls><office:styles>" + styles +
                           "</office:styles><office:automatic-styles>" + m_layouts + m_stylesSink.xml + "</office:automatic-styles><office:master-styles>" + masters + "</office:master-styles></office:document-styles>";
    zip.add(QStringLiteral("styles.xml"), stylesXml, true);
    zip.add(QStringLiteral("meta.xml"), metaXml(), true);
    zip.add(QStringLiteral("settings.xml"), settingsXml(), true);
    QByteArray manifest = QByteArray(kXml) + "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">"
                                             "<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.text\"/>"
                                             "<manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/>"
                                             "<manifest:file-entry manifest:full-path=\"styles.xml\" manifest:media-type=\"text/xml\"/>"
                                             "<manifest:file-entry manifest:full-path=\"meta.xml\" manifest:media-type=\"text/xml\"/>"
                                             "<manifest:file-entry manifest:full-path=\"settings.xml\" manifest:media-type=\"text/xml\"/>";
    for (const auto &p : std::as_const(m_pictures)) {
        const QString ext = QFileInfo(p.first).suffix().toLower();
        const char *type = ext == "png" ? "image/png" : ext == "jpeg" || ext == "jpg" ? "image/jpeg" : ext == "gif" ? "image/gif" : ext == "bmp" ? "image/bmp" : ext == "tif" || ext == "tiff" ? "image/tiff" : ext == "svg" ? "image/svg+xml" : ext == "emf" ? "image/x-emf" : ext == "wmf" ? "image/x-wmf" : "application/octet-stream";
        manifest += "<manifest:file-entry manifest:full-path=\"" + esc(p.first) + "\" manifest:media-type=\"" + type + "\"/>";
        zip.add(p.first, p.second, false);
    }
    manifest += "</manifest:manifest>";
    zip.add(QStringLiteral("META-INF/manifest.xml"), manifest, true);
    *out = zip.finish();
    return true;
}

QByteArray fromDocxPackage(const QByteArray &docx, QString *error, QStringList *notes)
{
    Writer w;
    QByteArray out;
    if (!w.run(docx, &out, error, notes)) return QByteArray();
    return out;
}

} // namespace jp::office::odt
