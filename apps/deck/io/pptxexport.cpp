#include "io/pptxexport.h"
#include "core/items.h"
#include "core/notesmaster.h"
#include "core/printheaderfooter.h"

#include "core/animation.h"
#include "core/comments.h"
#include "core/deck.h"
#include "core/masteredit.h"
#include "core/embeddedfonts.h"
#include "core/fonts.h"
#include "core/media.h"
#include "core/sections.h"
#include "core/themes.h"
#include "office/diagram.h"
#include "office/officecrypt.h"
#include "office/privacy.h"
#include "office/packageprops.h"
#include "render/diagramdrawing.h"
#include "io/pptxfonts.h"
#include "io/pptxshow.h"
#include "io/pptxview.h"
#include "io/pptxreview.h"
#include "io/pptxtablestyles.h"
#include "io/pptxtransition.h"
#include "office/chart.h"
#include "office/drawingml_table.h"
#include "office/drawingml_write.h"
#include "office/drawingml_write_util.h"
#include "office/package.h"
#include "ole/embedobj.h"
#include "office/theme.h"
#include "core/fonts.h"
#include "office/xml.h"
#include "app/equationedit.h"
#include "render/renderer.h"
#include "wml/wmlmathpath.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDomDocument>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <QXmlStreamWriter>
#include <climits>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>

namespace jp::deck {

using office::DiagramLinks;
using office::diagramDrawingLinkId;
using office::diagramFrameLinks;
using office::isDiagramFrame;
using office::kDiagramDrawingNs;
using office::withDiagramDrawingLinkId;

namespace {

const QString kDeclaration = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
const QString kNamespaces = QStringLiteral(
    " xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\""
    " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
    " xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"");
const QString kRelBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
const QString kPackageRelBase = QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships/");
const QString kTypeBase = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.");
const QString kPresentationPart = QStringLiteral("/ppt/presentation.xml");
const QString kNotesMasterPart = QStringLiteral("/ppt/notesMasters/notesMaster1.xml");
const QString kHandoutMasterPart = QStringLiteral("/ppt/handoutMasters/handoutMaster1.xml");

// The pixels of all the pictures an equation is written with, in one file (past it each is drawn smaller).
constexpr qint64 kMostEquationPixels = qint64(64) << 20;
// The ids of slide masters and layouts count up from here, in one range.
constexpr qint64 kFirstMasterId = 2147483648LL, kLastMasterId = 4294967295LL;
// A slide is numbered from here; the standard keeps these below the masters'.
constexpr qint64 kFirstSlideId = 256;
// The page notes are drawn on, and the notes master's places on it (EMU).
constexpr qint64 kNotesWidth = 6858000, kNotesHeight = 9144000;

// Text for an element or an attribute: markup escaped, and what XML 1.0
// can't hold (control characters, lone surrogates) left out.
QString esc(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        const ushort u = c.unicode();
        if (c.isHighSurrogate() && i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
            out += c;
            out += text[++i];
        } else if (u == '&') {
            out += QLatin1String("&amp;");
        } else if (u == '<') {
            out += QLatin1String("&lt;");
        } else if (u == '>') {
            out += QLatin1String("&gt;");
        } else if (u == '"') {
            out += QLatin1String("&quot;");
        } else if (u == '\t' || u == '\n' || u == '\r') {
            out += QStringLiteral("&#%1;").arg(u);
        } else if (u < 0x20 || (u >= 0xD800 && u <= 0xDFFF) || u == 0xFFFE || u == 0xFFFF) {
            continue;
        } else {
            out += c;
        }
    }
    return out;
}

QString emu(double points) { return QString::number(office::pointsToEmu(points)); }

QString hex(const QColor &c) { return c.name(QColor::HexRgb).mid(1).toUpper(); }

// What a drawing writer writes, as XML text to place in a part. The writer
// closes an empty element only when something follows it, so the last one is
// ended here.
QByteArray fragment(const std::function<void(QXmlStreamWriter &)> &write)
{
    QByteArray bytes;
    QXmlStreamWriter w(&bytes);
    write(w);
    w.writeCharacters(QString());
    return bytes;
}

// The sizes the presentation program names, by the name a slide size goes by
// here (setup.sizeName).
struct NamedSize {
    const char *name, *type;
};
const NamedSize kSizes[] = {
    {"Standard", "screen4x3"}, {"On-screen Show (4:3)", "screen4x3"}, {"On-screen Show (16:9)", "screen16x9"}, {"On-screen Show (16:10)", "screen16x10"},
    {"Letter Paper", "letter"}, {"A4 Paper", "A4"}, {"35mm Slides", "35mm"}, {"Overhead", "overhead"}, {"Banner", "banner"}, {"Ledger Paper", "ledger"},
    {"A3 Paper", "A3"}, {"B4 (ISO) Paper", "B4ISO"}, {"B5 (ISO) Paper", "B5ISO"}, {"B4 (JIS) Paper", "B4JIS"}, {"B5 (JIS) Paper", "B5JIS"}, {"Hagaki Card", "hagakiCard"}};

QString sizeType(const QString &name)
{
    for (const NamedSize &s : kSizes)
        if (name == QLatin1String(s.name)) return QString::fromLatin1(s.type);
    return QString();
}

// What a layout's "type" may say (the standard's list); another word is a custom layout.
bool isLayoutType(const QString &type)
{
    static const QSet<QString> types = {
        "title", "tx", "twoColTx", "tbl", "txAndChart", "chartAndTx", "dgm", "chart", "txAndClipArt", "clipArtAndTx", "titleOnly", "blank", "txAndObj", "objAndTx",
        "objOnly", "obj", "txAndMedia", "mediaAndTx", "objOverTx", "txOverObj", "txAndTwoObj", "twoObjAndTx", "twoObjOverTx", "fourObj", "vertTx", "clipArtAndVertTx",
        "vertTitleAndTx", "vertTitleAndTxOverChart", "twoObj", "objAndTwoObj", "twoObjAndObj", "secHead", "twoTxTwoObj", "objTx", "picTx"};
    return types.contains(type);
}

// The color map of a slide master: the usual one (the background is light 1, the text dark 1), or, for a master with a dark background, the
// one the presentation program's own dark themes have (the background is dark 1, the text light 1, and likewise for the second pair).
QString colorMapXml(bool dark)
{
    return QStringLiteral("<p:clrMap bg1=\"%1\" tx1=\"%2\" bg2=\"%3\" tx2=\"%4\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" "
                          "accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>")
        .arg(dark ? "dk1" : "lt1", dark ? "lt1" : "dk1", dark ? "dk2" : "lt2", dark ? "lt2" : "dk2");
}

// The theme color a color is, when it is one of the theme's, named as the master's color map reads it: on a dark master the text colors
// (dark 1 and 2 are its backgrounds, light 1 and 2 its text) change places.
QString schemeName(const QColor &c, const office::Theme &theme, bool dark)
{
    static const char *const names[office::Theme::ColorCount] = {"tx1", "bg1", "tx2", "bg2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
    static const char *const darkNames[4] = {"bg1", "tx1", "bg2", "tx2"};
    for (int i = 0; i < office::Theme::ColorCount; ++i)
        if (theme.colors[i].isValid() && theme.colors[i].rgb() == c.rgb()) return QString::fromLatin1(dark && i < 4 ? darkNames[i] : names[i]);
    return QString();
}

QString solidFill(const QColor &c, const office::Theme &theme, bool dark)
{
    const QString scheme = schemeName(c, theme, dark);
    return scheme.isEmpty() ? QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex(c))
                            : QStringLiteral("<a:solidFill><a:schemeClr val=\"%1\"/></a:solidFill>").arg(scheme);
}

// What the text of a level looks like in a master's text style.
struct Level {
    double size = 0;               // points; 0 for none said
    QString font;                  // a typeface, or "+mj-lt" or "+mn-lt" for the theme's
    bool bold = false, italic = false;
    QColor color;
    QString fx;                    // the effects of its letters (tp::TextFx), a shadow or a glow; empty for none
    QString align = QStringLiteral("l");
    double marL = -1, indent = 0;  // points; marL < 0 for none said
    double before = 0, after = 0;  // space above and below, points
    double lineHeight = 0;         // percent of a line (proportional), or points when `fixedHeight`; 0 for none said
    bool fixedHeight = false;
    bool bullet = false;
    QString bulletChar = QStringLiteral("\u2022");
    QString bulletFont;            // a typeface of its own; empty for the text's
    QColor bulletColor;            // a color of its own; invalid for the text's
    double bulletSize = 0;         // points; 0 for the text's
};

QString alignName(Qt::Alignment a)
{
    const Qt::Alignment h = a & Qt::AlignHorizontal_Mask;
    if (h.testFlag(Qt::AlignHCenter)) return QStringLiteral("ctr");
    if (h.testFlag(Qt::AlignRight)) return QStringLiteral("r");
    if (h.testFlag(Qt::AlignJustify)) return QStringLiteral("just");
    return QStringLiteral("l");
}

// The first paragraph at each level of a text, as the master's placeholder
// holds them; levels it has none for are left unset.
QVector<Level> levelsOf(const QTextDocument *doc, bool heading, const office::Theme &theme, QVector<bool> *said = nullptr)
{
    QVector<Level> levels(9);
    QVector<bool> found(9, false);
    if (said) *said = found;
    if (!doc) return levels;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        const int at = std::clamp(b.blockFormat().property(tp::ListLevel).toInt(), 0, 8);
        if (found[at]) continue;
        found[at] = true;
        QTextCharFormat cf = b.charFormat();
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            cf = it.fragment().charFormat();
            break;
        }
        Level &l = levels[at];
        if (cf.hasProperty(QTextFormat::FontPointSize)) l.size = cf.fontPointSize();
        // A family the text names wins over the font scheme's (as in the window).
        QString family = cf.hasProperty(QTextFormat::FontFamilies) ? cf.fontFamilies().toStringList().value(0) : QString();
        if (family.isEmpty() && cf.hasProperty(tp::ThemeFont)) {
            l.font = cf.stringProperty(tp::ThemeFont) == QLatin1String("major") ? QStringLiteral("+mj-lt") : QStringLiteral("+mn-lt");
        } else if (family.isEmpty()) {
            l.font = QString();
        } else if (heading && family == theme.majorLatin) {
            l.font = QStringLiteral("+mj-lt");
        } else if (family == theme.minorLatin) {
            l.font = QStringLiteral("+mn-lt");
        } else if (family == theme.majorLatin) {
            l.font = QStringLiteral("+mj-lt");
        } else {
            l.font = family;
        }
        l.bold = cf.fontWeight() >= QFont::DemiBold;
        l.italic = cf.fontItalic();
        const QBrush brush = cf.foreground();
        if (brush.style() != Qt::NoBrush && brush.color().isValid()) l.color = brush.color();
        l.fx = cf.stringProperty(tp::TextFx);
        const QTextBlockFormat bf = b.blockFormat();
        l.align = alignName(bf.alignment());
        if (bf.leftMargin() > 0) l.marL = bf.leftMargin();
        l.indent = bf.textIndent();
        l.before = std::max(0.0, bf.topMargin());
        l.after = std::max(0.0, bf.bottomMargin());
        if (bf.lineHeight() > 0 && bf.lineHeightType() == QTextBlockFormat::ProportionalHeight) l.lineHeight = bf.lineHeight();
        else if (bf.lineHeight() > 0 && bf.lineHeightType() == QTextBlockFormat::FixedHeight) l.lineHeight = bf.lineHeight(), l.fixedHeight = true;
        if (const QTextList *list = b.textList()) {
            l.bullet = true;
            const QTextListFormat lf = list->format();
            const QString c = bf.stringProperty(tp::BulletChar).isEmpty() ? lf.stringProperty(tp::BulletChar) : bf.stringProperty(tp::BulletChar);
            if (!c.isEmpty()) l.bulletChar = c;
            l.bulletFont = lf.stringProperty(tp::BulletFont);
            if (const ColorRef color = ColorRef::fromString(lf.stringProperty(tp::BulletColor)); color.kind() == ColorRef::Rgb) l.bulletColor = color.rgbValue();
            l.bulletSize = lf.property(tp::BulletSize).toDouble();
        }
    }
    if (said) *said = found;
    // A level the placeholder has none of looks like the first, smaller the deeper it goes.
    static const double standard[9] = {28, 24, 20, 18, 18, 18, 18, 18, 18};
    for (int i = 1; i < 9; ++i) {
        if (found[i]) continue;
        levels[i] = levels[0];
        if (!heading && levels[0].size > 0) {
            levels[i].size = levels[0].size * standard[i] / standard[0];
            levels[i].bulletSize = levels[0].bulletSize * standard[i] / standard[0];
        }
        levels[i].marL = -1;
    }
    return levels;
}

// `inList`: the level of a shape's list style, which says nothing where the paragraph had none (no margin: none, and no hanging indent).
QString levelXml(int level, const Level &l, const office::Theme &theme, bool dark, bool heading, bool body, double defaultSize, bool inList = false)
{
    const double marL = l.marL >= 0 ? l.marL : inList ? 0 : body ? 18 + 36 * level : (heading ? 0 : 36 * level);
    const double indent = l.marL >= 0 || !body ? l.indent : inList ? 0 : -18;
    QString out = QStringLiteral("<a:lvl%1pPr marL=\"%2\" indent=\"%3\" algn=\"%4\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">")
                      .arg(level + 1).arg(emu(std::clamp(marL, 0.0, 4000.0))).arg(emu(std::clamp(indent, -4000.0, 4000.0)), l.align);
    if (l.lineHeight > 0)
        out += l.fixedHeight ? QStringLiteral("<a:lnSpc><a:spcPts val=\"%1\"/></a:lnSpc>").arg(std::clamp(int(std::lround(l.lineHeight * 100)), 0, 158400))
                             : QStringLiteral("<a:lnSpc><a:spcPct val=\"%1\"/></a:lnSpc>").arg(std::clamp(int(std::lround(l.lineHeight * 1000)), 0, 13200000));
    out += QStringLiteral("<a:spcBef><a:spcPts val=\"%1\"/></a:spcBef>").arg(std::clamp(int(std::lround(l.before * 100)), 0, 158400));
    if (l.after > 0) out += QStringLiteral("<a:spcAft><a:spcPts val=\"%1\"/></a:spcAft>").arg(std::clamp(int(std::lround(l.after * 100)), 0, 158400));
    const double size = l.size > 0 ? l.size : defaultSize;
    if (body && l.bullet) {
        // The marker's own color, size (a share of the text's), and typeface, where the list has them: what a paragraph with no bullet of its own takes.
        if (l.bulletColor.isValid()) {
            const QString scheme = schemeName(l.bulletColor, theme, dark);
            out += QStringLiteral("<a:buClr>%1</a:buClr>")
                       .arg(scheme.isEmpty() ? QStringLiteral("<a:srgbClr val=\"%1\"/>").arg(hex(l.bulletColor)) : QStringLiteral("<a:schemeClr val=\"%1\"/>").arg(scheme));
        }
        if (l.bulletSize > 0) out += QStringLiteral("<a:buSzPct val=\"%1\"/>").arg(std::clamp(int(std::lround(l.bulletSize / size * 100000)), 25000, 400000));
        if (l.bulletFont.isEmpty())
            out += QStringLiteral("<a:buFont typeface=\"Arial\" panose=\"020B0604020202020204\" pitchFamily=\"34\" charset=\"0\"/>");
        else if (isSymbolFont(l.bulletFont))
            out += QStringLiteral("<a:buFont typeface=\"%1\" pitchFamily=\"2\" charset=\"2\"/>").arg(esc(l.bulletFont));
        else
            out += QStringLiteral("<a:buFont typeface=\"%1\"/>").arg(esc(l.bulletFont));
        out += QStringLiteral("<a:buChar char=\"%1\"/>").arg(esc(l.bulletChar));
    } else {
        out += QStringLiteral("<a:buNone/>");
    }
    out += QStringLiteral("<a:defRPr sz=\"%1\"%2%3 kern=\"1200\">").arg(std::clamp(int(std::lround(size * 100)), 100, 400000)).arg(l.bold ? QStringLiteral(" b=\"1\"") : QString(),
                                                                                                                           l.italic ? QStringLiteral(" i=\"1\"") : QString());
    out += l.color.isValid() ? solidFill(l.color, theme, dark) : QStringLiteral("<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>");
    if (!l.fx.isEmpty()) out += QString::fromUtf8(fragment([&](QXmlStreamWriter &w) { office::dmlwrite::writeTextEffects(w, TextEffects::fromJson(l.fx)); }));
    const QString face = l.font.isEmpty() ? (heading ? QStringLiteral("+mj-lt") : QStringLiteral("+mn-lt")) : l.font;
    if (face.startsWith(QLatin1Char('+'))) {
        const QString kind = face.mid(1, 2);
        out += QStringLiteral("<a:latin typeface=\"+%1-lt\"/><a:ea typeface=\"+%1-ea\"/><a:cs typeface=\"+%1-cs\"/>").arg(kind);
    } else {
        out += QStringLiteral("<a:latin typeface=\"%1\"/><a:ea typeface=\"+%2-ea\"/><a:cs typeface=\"+%2-cs\"/>").arg(esc(face), heading ? QStringLiteral("mj") : QStringLiteral("mn"));
    }
    out += QStringLiteral("</a:defRPr></a:lvl%1pPr>").arg(level + 1);
    return out;
}

// The levels' default look, for text no master placeholder describes.
QString plainLevels(const char *tag, double size)
{
    QString out;
    for (int i = 0; i < 9; ++i)
        out += QStringLiteral("<a:lvl%1pPr marL=\"%2\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\"><a:defRPr sz=\"%3\" kern=\"1200\">"
                              "<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl%1pPr>")
                   .arg(i + 1).arg(457200 * i).arg(int(size * 100));
    return QStringLiteral("<%1>%2</%1>").arg(QLatin1String(tag), out);
}

// Whether an element, or anything in it, has an attribute of the relationships' namespace (r:id, r:embed...): a reference to another part of the file.
bool refersToParts(const QDomElement &e)
{
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i)
        if (office::sameNamespace(attrs.item(i).namespaceURI(), office::ns::R)) return true;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (refersToParts(c)) return true;
    return false;
}

// The XML kept from a file, as a whole element that reads by itself. Empty if it does not.
bool parsesAlone(const QString &xml, QDomDocument *dom)
{
    return !xml.isEmpty() && office::parseXml(xml.toUtf8(), dom);
}

// The hyperlinks a frame holds point at parts that are not kept; they go, and what they were on stays.
QString withoutLinks(const QString &xml)
{
    static const QRegularExpression links(QStringLiteral("<(\\w+):hlink(?:Click|Hover|MouseOver)\\b[^>]*?\\b\\w+:id=\"[^\"]*\"[^>]*?(?:/>|>.*?</\\1:hlink(?:Click|Hover|MouseOver)>)"),
                                          QRegularExpression::DotMatchesEverythingOption);
    QString out = xml;
    out.remove(links);
    return out;
}

// A theme part as it can be written here. A theme's picture fills (its
// textures) point at pictures kept beside it in the file, which are not kept,
// so each becomes a plain fill in the color the style is used with. Empty if
// the part still refers to anything else after that.
QByteArray withoutThemePictures(const QByteArray &xml)
{
    QDomDocument dom;
    if (!office::parseXml(xml, &dom)) return QByteArray();
    if (!refersToParts(dom.documentElement())) return xml;
    static const QRegularExpression pictures(QStringLiteral("<a:blipFill\\b[^>]*>.*?</a:blipFill>"), QRegularExpression::DotMatchesEverythingOption);
    QString text = QString::fromUtf8(xml);
    QString out;
    qsizetype from = 0;
    for (auto it = pictures.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        out += text.mid(from, m.capturedStart() - from);
        out += QStringLiteral("<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>");
        from = m.capturedEnd();
    }
    out += text.mid(from);
    const QByteArray fixed = out.toUtf8();
    if (!office::parseXml(fixed, &dom) || refersToParts(dom.documentElement())) return QByteArray();
    return fixed;
}

// A theme part kept from a file, as written: with its pictures taken out, or,
// when that isn't enough, made from the colors and fonts it holds.
QByteArray keptThemePart(const QByteArray &kept, office::Theme *theme)
{
    const QByteArray fixed = withoutThemePictures(kept);
    theme->xml = fixed;
    return fixed.isEmpty() ? office::writeTheme(*theme) : fixed;
}

// The presentation's theme part: the one kept from the file, while the
// presentation's colors and fonts still say what it does, else one made from them.
QByteArray presentationTheme(const Document &doc, office::Theme *out)
{
    const DeckTheme deckTheme = themeOf(doc);
    const QByteArray kept = QByteArray::fromBase64(doc.appData.value(QStringLiteral("themeXml")).toString().toLatin1());
    office::Theme theme;
    const bool have = !kept.isEmpty() && office::readTheme(kept, &theme);
    if (have) {
        bool same = (theme.majorLatin.isEmpty() || theme.majorLatin == deckTheme.headingFont) && (theme.minorLatin.isEmpty() || theme.minorLatin == deckTheme.bodyFont);
        for (int i = 0; i < office::Theme::ColorCount && same; ++i) same = theme.colors[i] == deckTheme.colors[i];
        if (same) {
            const QByteArray part = keptThemePart(kept, &theme);
            *out = theme;
            return part;
        }
    } else {
        theme.name = theme.colorSchemeName = theme.fontSchemeName = deckTheme.name;
    }
    theme.xml.clear();
    for (int i = 0; i < office::Theme::ColorCount; ++i) theme.colors[i] = deckTheme.colors[i];
    theme.majorLatin = deckTheme.headingFont;
    theme.minorLatin = deckTheme.bodyFont;
    // (The part made here has the format styles the Quick Styles are drawn from, so a shape's style reference means what the shape shows.)
    theme.xml = themePart(deckTheme);
    *out = theme;
    return theme.xml;
}

// The same XML with the relationship ids in the attributes of the relationships' namespace (r:id, r:embed...) changed as `ids` says, all at once.
QString withRenumberedLinks(const QString &xml, const QHash<QString, QString> &ids)
{
    if (ids.isEmpty()) return xml;
    // The prefixes the part binds to that namespace.
    static const QRegularExpression declared(QStringLiteral("xmlns:(\\w+)=\"http://(?:schemas\\.openxmlformats\\.org|purl\\.oclc\\.org/ooxml)/officeDocument/(?:2006/)?relationships\""));
    QStringList prefixes;
    for (auto it = declared.globalMatch(xml); it.hasNext();) prefixes << it.next().captured(1);
    if (prefixes.isEmpty()) return xml;
    const QRegularExpression attribute(QStringLiteral("\\b(?:%1):\\w+=\"([^\"]*)\"").arg(prefixes.join(QLatin1Char('|'))));
    QString out;
    qsizetype from = 0;
    for (auto it = attribute.globalMatch(xml); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        const auto found = ids.constFind(m.captured(1));
        if (found == ids.cend()) continue;
        out += xml.mid(from, m.capturedStart(1) - from) + found.value();
        from = m.capturedEnd(1);
    }
    return out + xml.mid(from);
}

// A frame's p:xfrm set to a rectangle (points).
QString withFramePlace(const QString &frame, const QRectF &rect)
{
    static const QRegularExpression place(QStringLiteral("(<(\\w+):xfrm\\b[^>]*>)(.*?)(</\\2:xfrm>)"), QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch m = place.match(frame);
    if (!m.hasMatch()) return frame;
    static const QRegularExpression offPrefix(QStringLiteral("<(\\w+):off\\b"));
    const QRegularExpressionMatch a = offPrefix.match(m.captured(3));
    const QString p = a.hasMatch() ? a.captured(1) : QStringLiteral("a");
    return frame.left(m.capturedStart(3)) + QStringLiteral("<%1:off x=\"%2\" y=\"%3\"/><%1:ext cx=\"%4\" cy=\"%5\"/>").arg(p, emu(rect.x()), emu(rect.y()), emu(std::max(0.0, rect.width())), emu(std::max(0.0, rect.height()))) + frame.mid(m.capturedEnd(3));
}

// A frame's name and alternative text (on its p:cNvPr) set to those of the item that stands for it.
QString withFrameNames(const QString &frame, const QString &name, const QString &descr, bool hidden = false)
{
    static const QRegularExpression tag(QStringLiteral("<(\\w+):cNvPr\\b([^>]*?)(/?>)"));
    const QRegularExpressionMatch m = tag.match(frame);
    if (!m.hasMatch()) return frame;
    static const QRegularExpression named(QStringLiteral("\\s(?:name|descr|hidden)=\"[^\"]*\""));
    QString attrs = m.captured(2);
    attrs.remove(named);
    attrs += QStringLiteral(" name=\"%1\"").arg(esc(name));
    if (!descr.isEmpty()) attrs += QStringLiteral(" descr=\"%1\"").arg(esc(descr));
    if (hidden) attrs += QStringLiteral(" hidden=\"1\"");
    return frame.left(m.capturedStart()) + QStringLiteral("<%1:cNvPr%2%3").arg(m.captured(1), attrs, m.captured(3)) + frame.mid(m.capturedEnd());
}

// A frame's turn (its xfrm's rot) set to `degrees`, when the picture is turned.
QString withFrameTurn(const QString &frame, double degrees)
{
    static const QRegularExpression place(QStringLiteral("<(\\w+):xfrm\\b([^>]*)>"));
    const QRegularExpressionMatch m = place.match(frame);
    if (!m.hasMatch()) return frame;
    static const QRegularExpression turn(QStringLiteral("\\srot=\"[^\"]*\""));
    QString attrs = m.captured(2);
    attrs.remove(turn);
    // (Sixty-thousandths of a degree, clockwise, from 0 up to a full turn; a frame that is not turned says nothing.)
    const qint64 rot = std::llround(std::fmod(std::fmod(degrees, 360.0) + 360.0, 360.0) * 60000) % 21600000;
    if (rot) attrs += QStringLiteral(" rot=\"%1\"").arg(rot);
    return frame.left(m.capturedStart()) + QStringLiteral("<%1:xfrm%2>").arg(m.captured(1), attrs) + frame.mid(m.capturedEnd());
}

// The first slide, 256 pixels across, as the file's picture of itself (it draws with the text layout and the renderer, which only the window's thread may use).
QByteArray renderThumbnail(const Document &doc)
{
    const double width = doc.pageSize().width();
    if (doc.pages.isEmpty() || width <= 0) return QByteArray();
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &doc;
    ctx.cache = &cache;
    ctx.opt.output = true;
    const QImage image = Renderer::renderToImage(ctx, 0, 256.0 / width);
    if (image.isNull()) return QByteArray();
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.convertToFormat(QImage::Format_RGB32).save(&buffer, "JPEG", 85)) return QByteArray();
    return bytes;
}

class Exporter {
public:
    Exporter(const Document &doc, PresentationKind kind, QStringList *skipped, bool embedFonts, bool macros = false)
        : m_doc(doc), m_kind(kind), m_macros(macros), m_callerSkipped(skipped), m_embedFonts(embedFonts), m_keptFonts(EmbeddedFonts::facesOf(&doc)) {}
    // For a presentation made again from a snapshot (exportSnapshot): the fonts its file carried and the picture of its first slide are what the window had when the snapshot was taken.
    Exporter(const Document &doc, PresentationKind kind, bool embedFonts, const QVector<EmbeddedFace> &keptFonts, const QByteArray &thumbnail, bool macros = false)
        : m_doc(doc), m_kind(kind), m_macros(macros), m_callerSkipped(nullptr), m_embedFonts(embedFonts), m_keptFonts(keptFonts), m_thumbnail(thumbnail), m_thumbnailGiven(true) {}
    QByteArray run(QString *error);

private:
    struct MasterOut {
        const MasterPage *page = nullptr;
        QVector<int> layouts;     // in m_layouts
        QString part, themePart;
        qint64 id = 0;
        QByteArray themeXml;
        office::Theme theme;
        QString rel;              // its link in the presentation
    };
    struct LayoutOut {
        const MasterPage *page = nullptr;
        int master = 0;           // in m_masters
        QString part;
        qint64 id = 0;
    };

    void plan();
    const MasterPage *synthesized(bool master, const MasterPage *base);
    int layoutOf(const Page &slide) const;
    void prepareThemes();
    office::DrawingWriteContext context(const QString &part);
    QString tree(const PageBase &page, const QString &part, const QString &where, QSet<qint64> *frameIds, QSet<qint64> *mediaIds = nullptr);
    QString keptFrame(const QString &xml, const QString &where);
    QString chartFrame(const QJsonObject &kept, const Item &item, const QString &slidePart, const QString &where);
    QString diagramFrame(const QJsonObject &kept, const Item &item, const QString &slidePart, const QString &where);
    QString objectFrame(const QJsonObject &kept, const Item &item, const QString &slidePart, office::DrawingWriteContext &ctx, const QString &where, QSet<qint64> *frameIds);
    QString equationPair(const EquationItem &eq, office::DrawingWriteContext &ctx, const QString &where);
    QString equationBlock(const QJsonObject &kept, const Item &item, office::DrawingWriteContext &ctx, const QString &where);
    QHash<QString, QString> keptNames(const QJsonObject &parts, const QString &folder, bool keepMedia = false);
    void writeKeptParts(const QJsonObject &parts, const QHash<QString, QString> &names, const std::function<QByteArray(const QString &, const QByteArray &)> &rewrite = nullptr, const QString &asRead = QString());
    QString keptPartName(const QString &original, const QString &folder, bool keepMedia = false);
    QString keptSlideXml(const Page &slide, const char *key, const QString &what, const QString &where, const QSet<qint64> &frameIds,
                         const QSet<qint64> &mediaIds, const QString *xmlGiven = nullptr);
    QString background(const PageBase &page, const QString &part, bool required, const QString &where);
    QString textStyles(const MasterOut &master);
    QString withLevelStyles(const QString &xml, const MasterPage &page, const MasterOut &master);
    QString titleOf(const Page &slide, int number) const;
    QByteArray thumbnail() const;
    void note(const QString &text) { m_skipped << text; }
    int m_droppedLinks = 0;   // links to slides that are gone (see DrawingWriteContext::droppedLinks)

    void writeMaster(int i);
    void writeLayout(int i);
    void writeSlide(int i);
    void writeNotesSlide(int slide, int number);
    void writeNotesMaster();
    void writeHandoutMaster();
    void writeProperties();
    void writePresentation();
    void planComments();
    void writeCommentAuthors();
    void writeComments(int slide, const QString &slidePart);
    void add(const QString &part, const QString &type, const QString &xml);
    void scanFonts(const QString &part, const QByteArray &xml);

    const Document &m_doc;
    PresentationKind m_kind;
    bool m_macros;                               // written as a macro-enabled kind: the macro project the file had is kept, as it was
    QStringList *m_callerSkipped;
    bool m_embedFonts;                           // the program's own fonts that the text uses go in the file
    QVector<EmbeddedFace> m_keptFonts;           // the fonts the file this came from carried
    FontUse m_fontUse;                           // what the parts written use
    QHash<QString, int> m_partMaster;            // part -> the slide master (in m_masters) it follows: its theme and color map are the part's
    QHash<QString, QPair<QString, QString>> m_partFonts;   // part -> the heading and body font of the theme it follows (others follow the presentation's)
    FontsWritten m_fonts;
    QStringList m_skipped;
    office::PackageWriter m_pkg;
    QHash<QString, QString> m_media;
    QHash<QString, QByteArray> m_equationPictures;   // the pictures equations are written with, by "equation:" and a fingerprint of the math, color, and pixel size (what imageBytes gives for them)
    qint64 m_equationPixels = 0;                     // the pixels of all of them so far, held to kMostEquationPixels
    std::deque<MasterPage> m_synthesized;
    std::vector<MasterOut> m_masters;
    std::vector<LayoutOut> m_layouts;
    QVector<int> m_slideLayout, m_notesNumber;   // per slide: its layout; the number of its notes page (0 for none)
    QVector<QString> m_slideRel;
    QString m_notesMasterRel, m_handoutMasterRel;   // (the handout master only where the presentation chose its header and footer)
    office::Theme m_theme;                       // the presentation's, for drawings
    QByteArray m_themeXml;
    QString m_notesThemePart, m_handoutThemePart;
    QSet<QString> m_keptNames;                   // the names of kept chart parts written so far, lower-cased
    QByteArray m_thumbnail;
    bool m_thumbnailGiven = false;
    // The comments, as the file numbers them: authors by first appearance, and an index counting up for each author.
    struct CommentOut {
        int author = 0;
        int idx = 0;
        int parentAuthor = -1, parentIdx = 0;   // a reply names the comment it answers
        QString text;
        QDateTime date;
        QPointF at;
        bool done = false;
    };
    QVector<QPair<QString, QString>> m_commentAuthors;   // name, initials
    QVector<int> m_commentLastIdx;                       // per author
    QVector<QVector<CommentOut>> m_slideComments;        // per slide, each comment followed by its replies
    int m_commentParts = 0;
    QSizeF m_notesSize{kNotesWidth / 12700.0, kNotesHeight / 12700.0};
};

const MasterPage *Exporter::synthesized(bool master, const MasterPage *base)
{
    m_synthesized.emplace_back();
    MasterPage &m = m_synthesized.back();
    if (master) {
        m.id = QStringLiteral("SM1");
        m.name = QStringLiteral("Slide Master");
        m.abbr = QStringLiteral("SM");
        m.props[QStringLiteral("kind")] = QStringLiteral("master");
        m.background = Fill::solid(ColorRef::rgb(Qt::white));
    } else {
        m.id = QStringLiteral("L%1").arg(m_synthesized.size());
        m.name = QStringLiteral("Blank");
        m.abbr = QStringLiteral("1");
        m.baseId = base->id;
        m.props[QStringLiteral("kind")] = QStringLiteral("layout");
        m.props[QStringLiteral("type")] = QStringLiteral("blank");
    }
    return &m;
}

void Exporter::plan()
{
    // The slide masters, and under each the layouts that stand on it. A
    // layout whose master is gone stands on the first; a master with no
    // layout gets a blank one, as every slide needs a layout to stand on.
    for (const auto &m : m_doc.masters) {
        if (!isSlideMaster(*m)) continue;
        MasterOut o;
        o.page = m.get();
        m_masters.push_back(o);
    }
    if (m_masters.empty()) {
        MasterOut o;
        o.page = synthesized(true, nullptr);
        m_masters.push_back(o);
    }
    QVector<QVector<const MasterPage *>> under(int(m_masters.size()));
    for (const auto &m : m_doc.masters) {
        if (!isLayout(*m)) continue;
        int at = 0;
        for (int i = 0; i < int(m_masters.size()); ++i)
            if (m_masters[i].page->id == m->baseId) {
                at = i;
                break;
            }
        under[at] << m.get();
    }
    qint64 id = kFirstMasterId;
    for (int i = 0; i < int(m_masters.size()); ++i) {
        MasterOut &master = m_masters[i];
        master.id = id++;
        master.part = QStringLiteral("/ppt/slideMasters/slideMaster%1.xml").arg(i + 1);
        master.themePart = QStringLiteral("/ppt/theme/theme%1.xml").arg(i + 1);
        if (under[i].isEmpty()) under[i] << synthesized(false, master.page);
        for (const MasterPage *page : std::as_const(under[i])) {
            LayoutOut l;
            l.page = page;
            l.master = i;
            l.id = id++;
            l.part = QStringLiteral("/ppt/slideLayouts/slideLayout%1.xml").arg(m_layouts.size() + 1);
            master.layouts << int(m_layouts.size());
            m_layouts.push_back(l);
        }
    }
    m_notesThemePart = QStringLiteral("/ppt/theme/theme%1.xml").arg(m_masters.size() + 1);
    m_handoutThemePart = QStringLiteral("/ppt/theme/theme%1.xml").arg(m_masters.size() + 2);

    int notes = 0;
    for (const auto &page : m_doc.pages) {
        m_slideLayout << layoutOf(*page);
        const QTextDocument *sd = m_doc.storyDoc(page->notesStoryId);
        m_notesNumber << (sd && !sd->isEmpty() && !sd->toPlainText().isEmpty() ? ++notes : 0);
    }
    const QJsonObject size = m_doc.appData.value(QStringLiteral("notesSize")).toObject();
    if (size.value(QStringLiteral("w")).toDouble() > 0 && size.value(QStringLiteral("h")).toDouble() > 0)
        m_notesSize = QSizeF(size.value(QStringLiteral("w")).toDouble(), size.value(QStringLiteral("h")).toDouble());
}

int Exporter::layoutOf(const Page &slide) const
{
    const MasterPage *m = m_doc.masterFor(slide);
    for (int i = 0; i < int(m_layouts.size()); ++i)
        if (m_layouts[i].page == m) return i;
    // A slide that stands on a slide master directly takes that master's first layout.
    for (const MasterOut &master : m_masters)
        if (master.page == m && !master.layouts.isEmpty()) return master.layouts.first();
    return 0;
}

void Exporter::prepareThemes()
{
    m_themeXml = presentationTheme(m_doc, &m_theme);
    for (size_t i = 0; i < m_masters.size(); ++i) {
        MasterOut &master = m_masters[i];
        master.themeXml = m_themeXml;
        master.theme = m_theme;
        if (i == 0) continue;
        // Each master has a theme of its own; one kept from the file is used as it was.
        const QByteArray kept = QByteArray::fromBase64(master.page->props.value(QStringLiteral("themeXml")).toString().toLatin1());
        office::Theme theme;
        if (!kept.isEmpty() && office::readTheme(kept, &theme)) {
            master.themeXml = keptThemePart(kept, &theme);
            master.theme = theme;
        }
    }
    // Which master each master, layout, and slide follows (its theme and color map are what their drawings are written against).
    for (size_t i = 0; i < m_masters.size(); ++i) m_partMaster.insert(m_masters[i].part, int(i));
    for (const LayoutOut &layout : m_layouts) m_partMaster.insert(layout.part, layout.master);
    for (int i = 0; i < int(m_doc.pages.size()); ++i) m_partMaster.insert(QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1), m_layouts[m_slideLayout[i]].master);
    // The fonts of each theme, and which theme each master, layout, and slide follows.
    if (!m_embedFonts && m_keptFonts.isEmpty()) return;
    m_fontUse.addThemeFonts(m_theme.majorLatin, m_theme.minorLatin);
    for (const MasterOut &master : m_masters) {
        m_fontUse.addThemeFonts(master.theme.majorLatin, master.theme.minorLatin);
        m_partFonts.insert(master.part, qMakePair(master.theme.majorLatin, master.theme.minorLatin));
    }
    for (const LayoutOut &layout : m_layouts) {
        const office::Theme &theme = m_masters[layout.master].theme;
        m_partFonts.insert(layout.part, qMakePair(theme.majorLatin, theme.minorLatin));
    }
    for (int i = 0; i < int(m_doc.pages.size()); ++i) {
        const office::Theme &theme = m_masters[m_layouts[m_slideLayout[i]].master].theme;
        m_partFonts.insert(QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1), qMakePair(theme.majorLatin, theme.minorLatin));
    }
}

office::DrawingWriteContext Exporter::context(const QString &part)
{
    office::DrawingWriteContext ctx;
    ctx.package = &m_pkg;
    ctx.part = part;
    ctx.theme = m_theme;
    // The master the part follows: its theme, and its color map (the one colorMapXml writes: a dark master has the background as dark 1).
    if (const auto at = m_partMaster.constFind(part); at != m_partMaster.cend() && *at < int(m_masters.size())) {
        const MasterOut &master = m_masters[*at];
        ctx.theme = master.theme;
        if (isDarkBackground(master.page->background)) {
            ctx.colorMap.bg1 = office::Theme::Dark1;
            ctx.colorMap.tx1 = office::Theme::Light1;
            ctx.colorMap.bg2 = office::Theme::Dark2;
            ctx.colorMap.tx2 = office::Theme::Light2;
        }
    }
    ctx.imageBytes = [this](const QString &imageId, QString *format) {
        if (const auto made = m_equationPictures.constFind(imageId); made != m_equationPictures.cend()) {
            if (format) *format = QStringLiteral("png");
            return *made;
        }
        const auto it = m_doc.images.constFind(imageId);
        if (it == m_doc.images.cend()) return QByteArray();
        if (format) *format = it->format;
        return it->bytes;
    };
    ctx.storyDoc = [this](const QString &storyId) { return m_doc.storyDoc(storyId); };
    ctx.slideCount = int(m_doc.pages.size());
    ctx.droppedLinks = &m_droppedLinks;
    ctx.mediaBytes = [this](const QString &mediaId, QString *contentType, QString *extension) {
        const auto it = m_doc.media.constFind(mediaId);
        if (it == m_doc.media.cend()) return QByteArray();
        if (contentType) *contentType = it->contentType;
        if (extension) *extension = it->extension;
        return it->bytes;
    };
    ctx.mediaParts = &m_media;
    ctx.colors = m_doc.colors;
    return ctx;
}

// A frame the reader could not read, for writing back. Left out, with a
// note, if it points at parts of the file that were not kept (a chart's
// data, an embedded object); the hyperlinks in it are the one thing taken out
// of what stays.
QString Exporter::keptFrame(const QString &xml, const QString &where)
{
    QDomDocument dom;
    QString alone = withoutLinks(xml);
    if (!parsesAlone(alone, &dom)) {
        note(QCoreApplication::translate("PptxExport", "%1: a table, chart, or other frame that can't be read was left out.").arg(where));
        return QString();
    }
    if (refersToParts(dom.documentElement())) {
        note(QCoreApplication::translate("PptxExport", "%1: a chart, embedded object, or other frame that refers to parts of the file that aren't kept was left out.").arg(where));
        return QString();
    }
    return alone;
}

// Whether every movie or sound the timing plays, pauses, or stops (p:video,
// p:audio, p:cmd) is aimed at a shape that has one. The presentation program
// refuses a file whose timing plays the media of a shape that has none.
bool playsOnlyMedia(const QDomElement &e, const QSet<qint64> &mediaIds, bool aimsAtMedia = false)
{
    const QString name = e.localName();
    aimsAtMedia = aimsAtMedia || name == QLatin1String("video") || name == QLatin1String("audio") || name == QLatin1String("cmd");
    if (aimsAtMedia && name == QLatin1String("spTgt") && !mediaIds.contains(e.attribute(QStringLiteral("spid")).toLongLong())) return false;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (!playsOnlyMedia(c, mediaIds, aimsAtMedia)) return false;
    return true;
}

// A slide's transition or timing as the reader kept it. Left out, with a note,
// when it names parts that were not kept (a sound), aims an animation at
// shapes by numbers that no longer stand for the same shapes, or plays a
// movie or sound on a shape that no longer has one.
QString Exporter::keptSlideXml(const Page &slide, const char *key, const QString &what, const QString &where, const QSet<qint64> &frameIds,
                               const QSet<qint64> &mediaIds, const QString *xmlGiven)
{
    const QString given = xmlGiven ? *xmlGiven : slide.props.value(QLatin1String(key)).toString();
    // The movie or sound of a picture that was deleted is taken out of the timing; the rest of it stays.
    const QString xml = !given.isEmpty() && std::strcmp(key, "timingXml") == 0 ? deck::mediaTimingForShapes(given, frameIds) : given;
    if (xml.isEmpty()) return QString();
    QDomDocument dom;
    if (!parsesAlone(xml, &dom)) {
        note(QCoreApplication::translate("PptxExport", "%1: the %2 can't be read and was left out.").arg(where, what));
        return QString();
    }
    if (refersToParts(dom.documentElement())) {
        note(QCoreApplication::translate("PptxExport", "%1: the %2 uses a sound or other part of the file that isn't kept, and was left out.").arg(where, what));
        return QString();
    }
    static const QRegularExpression targets(QStringLiteral("\\bspid=\"(\\d+)\""));
    for (auto it = targets.globalMatch(xml); it.hasNext();) {
        // Shapes are numbered afresh in the file, but a frame keeps the number it had.
        if (!frameIds.contains(it.next().captured(1).toLongLong())) {
            note(QCoreApplication::translate("PptxExport", "%1: the %2 aims at shapes by their numbers in the old file, and was left out.").arg(where, what));
            return QString();
        }
    }
    if (!playsOnlyMedia(dom.documentElement(), mediaIds)) {
        note(QCoreApplication::translate("PptxExport", "%1: the %2 plays a movie or sound that wasn't kept, and was left out.").arg(where, what));
        return QString();
    }
    return xml;
}

// A part name for a kept part of a chart or diagram: the one it had when that is free and in a folder only they write to, else a free one in `folder`.
// `keepMedia`: a picture keeps its place among the pictures too (the pictures the slides show are written under names that leave it alone).
QString Exporter::keptPartName(const QString &original, const QString &folder, bool keepMedia)
{
    const QString lower = original.toLower();
    const bool media = keepMedia && lower.startsWith(QLatin1String("/ppt/media/"));
    const bool own = lower.startsWith(QLatin1String("/ppt/charts/")) || lower.startsWith(QLatin1String("/ppt/embeddings/")) || lower.startsWith(QLatin1String("/ppt/drawings/"))
                     || lower.startsWith(QLatin1String("/ppt/diagrams/")) || lower.startsWith(QLatin1String("/ppt/theme/themeoverride")) || media;
    const QString file = original.mid(original.lastIndexOf(QLatin1Char('/')) + 1);
    const QString name = own ? original : folder + file;
    // Free: no kept part has it, and no picture of a slide was written under it.
    auto free = [&](const QString &candidate) {
        if (m_keptNames.contains(candidate.toLower())) return false;
        for (const QString &taken : std::as_const(m_media))
            if (taken.compare(candidate, Qt::CaseInsensitive) == 0) return false;
        return true;
    };
    auto take = [&](const QString &chosen) {
        m_keptNames.insert(chosen.toLower());
        // The pictures a slide shows are written after this one's: they leave its name alone.
        if (media) m_media.insert(QStringLiteral("kept:") + chosen.toLower(), chosen);
        return chosen;
    };
    if (free(name)) return take(name);
    // Another has it: the same name with the next number.
    const QString dir = name.left(name.lastIndexOf(QLatin1Char('/')) + 1);
    const int dot = int(file.lastIndexOf(QLatin1Char('.')));
    QString stem = dot < 0 ? file : file.left(dot);
    const QString extension = dot < 0 ? QString() : file.mid(dot);
    while (!stem.isEmpty() && stem.back().isDigit()) stem.chop(1);
    for (int n = 1;; ++n) {
        const QString candidate = dir + stem + QString::number(n) + extension;
        if (free(candidate)) return take(candidate);
    }
}

// Whether every link of the kept parts leads to a part that is kept too.
bool keptPartsAreWhole(const QJsonObject &parts)
{
    for (auto it = parts.begin(); it != parts.end(); ++it)
        for (const QJsonValue &link : it.value().toObject().value(QStringLiteral("rels")).toArray())
            if (!link.toObject().value(QStringLiteral("external")).toBool() && !parts.contains(link.toObject().value(QStringLiteral("target")).toString())) return false;
    return true;
}

// The names the kept parts are written under: free ones, in `folder` for any that had none of their own kind of place.
QHash<QString, QString> Exporter::keptNames(const QJsonObject &parts, const QString &folder, bool keepMedia)
{
    QHash<QString, QString> names;
    for (const QString &key : parts.keys()) names.insert(key, keptPartName(key, folder, keepMedia));
    return names;
}

// The kept parts, written under their new names and linked as they were: a part's links in the order of their ids so that they keep them (rId1,
// rId2...); any that don't are changed in the part. `rewrite` may change a part's bytes before it is written.
void Exporter::writeKeptParts(const QJsonObject &parts, const QHash<QString, QString> &names, const std::function<QByteArray(const QString &, const QByteArray &)> &rewrite, const QString &asRead)
{
    for (const QString &key : parts.keys()) {
        const QJsonObject part = parts.value(key).toObject();
        const QString name = names.value(key);
        QByteArray data = QByteArray::fromBase64(part.value(QStringLiteral("data")).toString().toLatin1());
        QVector<QJsonObject> links;
        for (const QJsonValue &link : part.value(QStringLiteral("rels")).toArray()) links << link.toObject();
        auto number = [](const QJsonObject &l) { return l.value(QStringLiteral("id")).toString().mid(3).toLongLong(); };
        std::stable_sort(links.begin(), links.end(), [&](const QJsonObject &a, const QJsonObject &b) { return number(a) < number(b); });
        QHash<QString, QString> renumbered;
        for (const QJsonObject &link : std::as_const(links)) {
            const bool external = link.value(QStringLiteral("external")).toBool();
            const QString target = link.value(QStringLiteral("target")).toString();
            const QString id = m_pkg.addRelationship(name, link.value(QStringLiteral("type")).toString(), external ? target : names.value(target), external);
            const QString was = link.value(QStringLiteral("id")).toString();
            if (id != was) renumbered.insert(was, id);
        }
        if (!renumbered.isEmpty()) data = withRenumberedLinks(QString::fromUtf8(data), renumbered).toUtf8();
        if (rewrite) data = rewrite(key, data);
        const QString type = part.value(QStringLiteral("type")).toString();
        if (type.endsWith(QLatin1String("xml"))) {
            if (key != asRead) data = withStandardFontNames(data);   // (a part that was not changed is written as it was read)
            scanFonts(name, data);
        }
        m_pkg.addPart(name, data, type.isEmpty() ? QStringLiteral("application/octet-stream") : type);
    }
}

// The frame for a picture that stands for a chart, and the chart's parts in the package: what the reader kept, under names that are free, linked
// as they were, with the frame placed where the picture is. Empty, with a note, if what was kept can't be written.
QString Exporter::chartFrame(const QJsonObject &kept, const Item &item, const QString &slidePart, const QString &where)
{
    const QJsonObject parts = kept.value(QStringLiteral("parts")).toObject();
    const QString main = kept.value(QStringLiteral("main")).toString();
    QString frame = kept.value(QStringLiteral("frameXml")).toString();
    QDomDocument dom;
    const bool sound = parts.contains(main) && parsesAlone(frame, &dom) && office::isChartFrame(dom.documentElement()) && keptPartsAreWhole(parts);
    if (!sound) {
        note(QCoreApplication::translate("PptxExport", "%1: a chart's parts can't be written, so it was left out.").arg(where));
        return QString();
    }
    const QHash<QString, QString> names = keptNames(parts, QStringLiteral("/ppt/charts/"));
    writeKeptParts(parts, names, nullptr, kept.value(QStringLiteral("unchanged")).toBool() ? main : QString());
    // The slide's link to the chart, which the frame names, and the frame where the picture is.
    const QString id = m_pkg.addRelationship(slidePart, kRelBase + QStringLiteral("chart"), names.value(main));
    static const QRegularExpression chartTag(QStringLiteral("(<(\\w+):chart\\b[^>]*?\\b\\w+:id=\")[^\"]*(\")"));
    frame.replace(chartTag, QStringLiteral("\\1") + id + QStringLiteral("\\3"));
    frame = withFramePlace(frame, item.rect);
    frame = withFrameTurn(frame, item.rotation);
    return withFrameNames(frame, item.name, item.altText, item.hidden);
}

// The frame for a picture that stands for an embedded object, and the object's package in the package: its bytes exactly as the reader kept them (or as they were put on the page),
// under their own name where it is free, linked from the slide as the kept entry says; the picture is the one the item shows. The frame has the number the object's frame had, when it is
// free. Empty, with a note, if the package can't be written.
QString Exporter::objectFrame(const QJsonObject &kept, const Item &item, const QString &slidePart, office::DrawingWriteContext &ctx, const QString &where, QSet<qint64> *frameIds)
{
    const QByteArray data = QByteArray::fromBase64(kept.value(QStringLiteral("data")).toString().toLatin1());
    const QString image = item.type() == ItemType::Picture ? office::dmlwrite::relatePicture(ctx, static_cast<const PictureItem &>(item).imageId) : QString();
    if (data.isEmpty() || image.isEmpty()) {
        note(QCoreApplication::translate("PptxExport", "%1: an embedded object's package can't be written, so it was left out.").arg(where));
        return QString();
    }
    auto fresh = [&ctx]() {
        int next = std::max(ctx.nextShapeId, 2);
        while (next < INT_MAX && (ctx.usedIds.contains(next) || ctx.reservedIds.contains(next))) ++next;
        ctx.usedIds.insert(next);
        ctx.nextShapeId = next + 1;
        return next;
    };
    int id = 0;
    if (item.sourceId >= 2 && item.sourceId <= INT_MAX && !ctx.usedIds.contains(int(item.sourceId))) {
        id = int(item.sourceId);
        ctx.usedIds.insert(id);
    } else {
        id = fresh();
    }
    const int pictureId = fresh();
    if (frameIds) frameIds->insert(id);
    const QString progId = kept.value(QStringLiteral("progId")).toString();
    const QString original = kept.value(QStringLiteral("part")).toString();
    const QString name = keptPartName(original.isEmpty() ? QStringLiteral("/ppt/embeddings/object1.") + kept.value(QStringLiteral("ext")).toString() : original, QStringLiteral("/ppt/embeddings/"));
    const QString type = kept.value(QStringLiteral("type")).toString();
    m_pkg.addPart(name, data, type.isEmpty() ? QStringLiteral("application/octet-stream") : type);
    const QString link = m_pkg.addRelationship(slidePart, kept.value(QStringLiteral("relType")).toString(), name);
    const double w = std::max(0.0, item.rect.width()), h = std::max(0.0, item.rect.height());
    QString turn;
    if (const qint64 t = std::llround(item.rotation * 60000); t != 0) turn += QStringLiteral(" rot=\"%1\"").arg(t);
    if (item.flipH) turn += QStringLiteral(" flipH=\"1\"");
    if (item.flipV) turn += QStringLiteral(" flipV=\"1\"");
    const QString descr = item.altText.isEmpty() ? QString() : QStringLiteral(" descr=\"%1\"").arg(esc(item.altText));
    const QString hidden = item.hidden ? QStringLiteral(" hidden=\"1\"") : QString();
    const QString title = esc(kept.value(QStringLiteral("name")).toString().isEmpty() ? item.name : kept.value(QStringLiteral("name")).toString());
    const QString place = QStringLiteral("<a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/>").arg(emu(item.rect.x()), emu(item.rect.y()), emu(w), emu(h));
    return QStringLiteral("<p:graphicFrame xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"%2\"%3%4/><p:cNvGraphicFramePr><a:graphicFrameLocks noChangeAspect=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                          "<p:xfrm%5>%6</p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/presentationml/2006/ole\"><p:oleObj name=\"%7\" r:id=\"%8\" imgW=\"%9\" imgH=\"%10\" progId=\"%11\"><p:embed/>"
                          "<p:pic><p:nvPicPr><p:cNvPr id=\"%12\" name=\"\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill><a:blip r:embed=\"%13\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill>"
                          "<p:spPr><a:xfrm>%6</a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic></p:oleObj></a:graphicData></a:graphic></p:graphicFrame>")
        .arg(id).arg(esc(item.name), descr, hidden, turn, place, title, link, emu(w), emu(h), esc(progId)).arg(pictureId).arg(image);
}

// The frame for a picture that stands for a diagram, and the diagram's parts in the package: what the reader kept, under their own names where they
// are free, linked as they were, the slide's links to them numbered as the file numbered them where that can be. What names a link, the frame
// (r:dm, r:lo, r:qs, r:cs) and the data part (the drawing's), says the number it has now. The frame is as it was but for its place, when the
// picture was moved or resized, and its name, when that was changed. The hyperlinks in it are the one thing taken out. Empty, with a note, if what
// was kept can't be written.
QString Exporter::diagramFrame(const QJsonObject &kept, const Item &item, const QString &slidePart, const QString &where)
{
    const QJsonObject parts = kept.value(QStringLiteral("parts")).toObject();
    QString frame = withoutLinks(kept.value(QStringLiteral("frameXml")).toString());
    QVector<QJsonObject> links;
    for (const QJsonValue &v : kept.value(QStringLiteral("links")).toArray()) links << v.toObject();
    QDomDocument dom;
    bool sound = !links.isEmpty() && parsesAlone(frame, &dom) && isDiagramFrame(dom.documentElement()) && keptPartsAreWhole(parts);
    QString dataPart;
    for (const QJsonObject &l : std::as_const(links)) {
        sound = sound && parts.contains(l.value(QStringLiteral("target")).toString());
        if (l.value(QStringLiteral("type")).toString().endsWith(QLatin1String("/diagramData"))) dataPart = l.value(QStringLiteral("target")).toString();
    }
    if (!sound || dataPart.isEmpty()) {
        note(QCoreApplication::translate("PptxExport", "%1: a diagram's parts can't be written, so it was left out.").arg(where));
        return QString();
    }
    const QHash<QString, QString> names = keptNames(parts, QStringLiteral("/ppt/diagrams/"), true);
    auto number = [](const QJsonObject &l) { return l.value(QStringLiteral("id")).toString().mid(3).toLongLong(); };
    std::stable_sort(links.begin(), links.end(), [&](const QJsonObject &a, const QJsonObject &b) { return number(a) < number(b); });
    QHash<QString, QString> renumbered;
    QString drawingWas, drawingNow;
    for (const QJsonObject &l : std::as_const(links)) {
        const QString was = l.value(QStringLiteral("id")).toString();
        const QString type = l.value(QStringLiteral("type")).toString();
        const QString id = m_pkg.addRelationship(slidePart, type, names.value(l.value(QStringLiteral("target")).toString()));
        if (id != was) renumbered.insert(was, id);
        if (type.endsWith(QLatin1String("/diagramDrawing"))) {
            drawingWas = was;
            drawingNow = id;
        }
    }
    writeKeptParts(parts, names, [&](const QString &key, const QByteArray &data) {
        // The data part names the drawing's link, which may be numbered otherwise now.
        if (key != dataPart || drawingWas == drawingNow || diagramDrawingLinkId(data) != drawingWas) return data;
        return withDiagramDrawingLinkId(data, drawingNow);
    });
    frame = withRenumberedLinks(frame, renumbered);
    // As it was, unless it was moved or resized, or named otherwise.
    auto same = [](double a, double b) { return std::abs(a - b) < 1e-6; };
    const QRectF was(kept.value(QStringLiteral("x")).toDouble(), kept.value(QStringLiteral("y")).toDouble(), kept.value(QStringLiteral("w")).toDouble(), kept.value(QStringLiteral("h")).toDouble());
    if (!same(was.x(), item.rect.x()) || !same(was.y(), item.rect.y()) || !same(was.width(), item.rect.width()) || !same(was.height(), item.rect.height())) frame = withFramePlace(frame, item.rect);
    if (item.name != kept.value(QStringLiteral("name")).toString() || item.altText != kept.value(QStringLiteral("descr")).toString() || item.hidden) frame = withFrameNames(frame, item.name, item.altText, item.hidden);
    return frame;
}

// An xfrm of the first shape of `xml` (its a:xfrm, in the shape's properties), set as `item` is turned and placed. As the frame's, but with a turn.
QString withShapePlace(const QString &xml, const Item &item)
{
    static const QRegularExpression place(QStringLiteral("<(\\w+):xfrm\\b[^>]*>.*?</\\1:xfrm>"), QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch m = place.match(xml);
    if (!m.hasMatch()) return xml;
    const QString p = m.captured(1);
    QString attrs;
    if (const qint64 turn = std::llround(item.rotation * 60000); turn != 0) attrs += QStringLiteral(" rot=\"%1\"").arg(turn);
    if (item.flipH) attrs += QStringLiteral(" flipH=\"1\"");
    if (item.flipV) attrs += QStringLiteral(" flipV=\"1\"");
    const QString xfrm = QStringLiteral("<%1:xfrm%2><%1:off x=\"%3\" y=\"%4\"/><%1:ext cx=\"%5\" cy=\"%6\"/></%1:xfrm>")
                             .arg(p, attrs, emu(item.rect.x()), emu(item.rect.y()), emu(std::max(0.0, item.rect.width())), emu(std::max(0.0, item.rect.height())));
    return xml.left(m.capturedStart()) + xfrm + xml.mid(m.capturedEnd());
}

// The pair of alternatives of an equation made here, or read from a file as one: the shape whose text is the math (a14:m, for the programs that know it), and a
// picture of it for the others, drawn as JeffDeck draws the equation, with the frame's own proportions (twice as many pixels as points across, and at most 4,000).
// Both are placed alike and have the equation's name; the shape has its number, the picture the next free one. Empty, with a note, if no picture can be made.
QString Exporter::equationPair(const EquationItem &shown, office::DrawingWriteContext &ctx, const QString &where)
{
    // (A file has no Linear form: the math is saved built up, in a frame of the built-up layout's proportions.)
    const auto builtUp = builtUpEquation(shown);
    const EquationItem &eq = *builtUp;
    const double w = eq.rect.width(), h = eq.rect.height();
    const QColor color = eq.fill.isNone() ? QColor(Qt::black) : eq.fill.color.resolve(m_doc.colors);
    // The picture is three pixels to the point, at most 4,000 a side; the pictures of a whole file come to at most kMostEquationPixels, and past that each is drawn smaller
    // (never to nothing). One with no size (a frame with no width or no height) has a picture of one clear pixel. Equal math, color, and size share one picture.
    const double px = std::min({3.0, 4000.0 / std::max(1.0, w), 4000.0 / std::max(1.0, h)});
    int pw = 1, ph = 1;
    if (w > 0 && h > 0) {
        pw = std::max(1, int(std::lround(w * px)));
        ph = std::max(1, int(std::lround(h * px)));
        const qint64 room = std::max<qint64>(kMostEquationPixels - m_equationPixels, 4096);
        if (qint64(pw) * ph > room) {
            const double k = std::sqrt(double(room) / (double(pw) * ph));
            pw = std::max(1, int(pw * k));
            ph = std::max(1, int(ph * k));
        }
    }
    const QString key = QStringLiteral("equation:") + QString::fromLatin1(QCryptographicHash::hash(eq.markup + QByteArray::number(color.rgba()) + ':' + QByteArray::number(pw) + 'x' + QByteArray::number(ph), QCryptographicHash::Sha1).toHex());
    if (!m_equationPictures.contains(key)) {
        QByteArray png;
        if (w > 0 && h > 0) {
            png = office::wml::equationPng(eq.markup, pw, ph, color);
            m_equationPixels += qint64(pw) * ph;
        } else {
            QImage clear(1, 1, QImage::Format_ARGB32);
            clear.fill(Qt::transparent);
            QBuffer buffer(&png);
            buffer.open(QIODevice::WriteOnly);
            clear.save(&buffer, "PNG");
        }
        if (png.isEmpty()) {
            note(QCoreApplication::translate("PptxExport", "%1: an equation that can't be drawn was left out.").arg(where));
            return QString();
        }
        m_equationPictures.insert(key, png);
    }
    const QString link = office::dmlwrite::relatePicture(ctx, key);
    if (link.isEmpty()) {
        note(QCoreApplication::translate("PptxExport", "%1: an equation's picture can't be written, so it was left out.").arg(where));
        return QString();
    }
    const int id = office::shapeNumber(ctx, eq);
    int next = std::max(ctx.nextShapeId, 2);
    while (next < INT_MAX && (ctx.usedIds.contains(next) || ctx.reservedIds.contains(next))) ++next;
    ctx.usedIds.insert(next);
    ctx.nextShapeId = next + 1;
    // (What a file may not hold is taken out of the text: a control character in a name or the math would make the whole slide unreadable.)
    auto legal = [](const QString &text) {
        QString out;
        out.reserve(text.size());
        for (const QChar c : text) {
            const ushort u = c.unicode();
            if ((u < 0x20 && u != 9 && u != 10 && u != 13) || u == 0xFFFE || u == 0xFFFF) continue;
            out += c;
        }
        return out;
    };
    auto attribute = [&legal](const QString &text) { return legal(text).toHtmlEscaped().replace(QLatin1Char('"'), QStringLiteral("&quot;")); };
    const QString name = attribute(eq.name.isEmpty() ? QCoreApplication::translate("PptxExport", "Equation") : eq.name);
    // (The math's own markup, as a paragraph of its own: a made one is an m:oMath, put in a paragraph of math; one read from a file is the paragraph it was.)
    QString math = legal(QString::fromUtf8(eq.markup));
    const int open = int(math.indexOf(QLatin1Char('<')));
    if (!math.mid(open, 14).startsWith(QStringLiteral("<m:oMathPara")))
        math = QStringLiteral("<m:oMathPara xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:oMathParaPr><m:jc m:val=\"centerGroup\"/></m:oMathParaPr>") + math + QStringLiteral("</m:oMathPara>");
    // The letters' height as they are drawn: the layout's, at the scale of the frame.
    const auto laid = office::wml::equationShapeCached(eq.markup, eq.size);
    const double scale = laid && laid->valid() ? std::min(w / laid->box.width(), h / laid->box.height()) : 1.0;
    const int size = std::clamp(int(std::lround(eq.size * scale * 100)), 100, 400000);
    QString turn;
    if (const qint64 t = std::llround(eq.rotation * 60000); t != 0) turn += QStringLiteral(" rot=\"%1\"").arg(t);
    if (eq.flipH) turn += QStringLiteral(" flipH=\"1\"");
    if (eq.flipV) turn += QStringLiteral(" flipV=\"1\"");
    const QString place = QStringLiteral("<a:xfrm%1><a:off x=\"%2\" y=\"%3\"/><a:ext cx=\"%4\" cy=\"%5\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>")
                              .arg(turn, emu(eq.rect.x()), emu(eq.rect.y()), emu(std::max(0.0, w)), emu(std::max(0.0, h)));
    // The color of the letters as the equation has it: a theme color (with no lightening or darkening) stays the theme's; the picture is drawn in what it resolves to.
    static const char *const themeNames[SlotCount] = {"tx1", "accent1", "accent2", "accent3", "accent4", "accent5", "hlink", "folHlink"};
    QString paint;
    if (eq.fill.type == Fill::Solid) {
        if (eq.fill.color.kind() == ColorRef::Scheme && eq.fill.color.lighten() == 0 && eq.fill.color.darken() == 0 && eq.fill.color.slot() >= 0 && eq.fill.color.slot() < SlotCount)
            paint = QStringLiteral("<a:solidFill><a:schemeClr val=\"%1\"/></a:solidFill>").arg(QLatin1String(themeNames[eq.fill.color.slot()]));
        else
            paint = QString::fromUtf8(fragment([&](QXmlStreamWriter &out) { office::writeFill(out, eq.fill, ctx); }));
    }
    const QString descr = eq.altText.isEmpty() ? QString() : QStringLiteral(" descr=\"%1\"").arg(attribute(eq.altText));
    const QString picDescr = QStringLiteral(" descr=\"%1\"").arg(attribute(eq.altText.isEmpty() ? eq.linear : eq.altText));
    const QString hidden = eq.hidden ? QStringLiteral(" hidden=\"1\"") : QString();
    // (Joined, not filled in with arg(): the math is a file's text, and a %1 in it must stay as it is.)
    const QString ids = QStringLiteral(" id=\"%1\" name=\"").arg(id);
    return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
                          "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                          "<mc:Choice xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\" Requires=\"a14\"><p:sp><p:nvSpPr><p:cNvPr")
           + ids + name + QStringLiteral("\"") + descr + hidden + QStringLiteral("/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + place
           + QStringLiteral("<a:noFill/></p:spPr><p:txBody><a:bodyPr wrap=\"none\" rtlCol=\"0\" anchor=\"ctr\"/><a:lstStyle/><a:p><a14:m>") + math
           + QStringLiteral("</a14:m><a:endParaRPr lang=\"en-US\" sz=\"%1\">").arg(size) + paint
           + QStringLiteral("</a:endParaRPr></a:p></p:txBody></p:sp></mc:Choice><mc:Fallback><p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"").arg(next) + name + QStringLiteral("\"") + picDescr + hidden
           + QStringLiteral("/><p:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></p:cNvPicPr><p:nvPr/></p:nvPicPr><p:blipFill><a:blip r:embed=\"") + link
           + QStringLiteral("\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>") + place + QStringLiteral("</p:spPr></p:pic></mc:Fallback></mc:AlternateContent>");
}

// The pair of alternatives of an equation, for a picture that stands for it: as the reader kept it, with the picture's link (its image written
// into the package as pictures are, and the link to it numbered as the slide has it), and the place of both shapes set to the picture's if
// it was moved, resized, or turned. The hyperlinks in it are the one thing taken out. Empty, with a note, if what was kept can't be written.
QString Exporter::equationBlock(const QJsonObject &kept, const Item &item, office::DrawingWriteContext &ctx, const QString &where)
{
    QString xml = withoutLinks(kept.value(QStringLiteral("xml")).toString());
    QDomDocument dom;
    // What the pair links to: the picture, by one id wherever it is used.
    QSet<QString> used;
    QVector<QDomElement> stack;
    if (parsesAlone(xml, &dom)) stack << dom.documentElement();
    while (!stack.isEmpty()) {
        const QDomElement e = stack.takeLast();
        const QDomNamedNodeMap attrs = e.attributes();
        for (int i = 0; i < attrs.length(); ++i)
            if (office::sameNamespace(attrs.item(i).namespaceURI(), office::ns::R)) used << attrs.item(i).nodeValue();
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
    }
    const QString image = item.type() == ItemType::Picture ? office::dmlwrite::relatePicture(ctx, static_cast<const PictureItem &>(item).imageId) : QString();
    if (dom.documentElement().isNull() || used.size() > 1 || (used.size() == 1 && image.isEmpty())) {
        note(QCoreApplication::translate("PptxExport", "%1: an equation's picture can't be written, so it was left out.").arg(where));
        return QString();
    }
    if (used.size() == 1) xml = withRenumberedLinks(xml, {{*used.begin(), image}});
    auto same = [](double a, double b) { return std::abs(a - b) < 1e-6; };
    const QRectF was(kept.value(QStringLiteral("x")).toDouble(), kept.value(QStringLiteral("y")).toDouble(), kept.value(QStringLiteral("w")).toDouble(), kept.value(QStringLiteral("h")).toDouble());
    const bool placed = same(was.x(), item.rect.x()) && same(was.y(), item.rect.y()) && same(was.width(), item.rect.width()) && same(was.height(), item.rect.height())
                        && same(kept.value(QStringLiteral("rotation")).toDouble(), item.rotation) && kept.value(QStringLiteral("flipH")).toBool() == item.flipH
                        && kept.value(QStringLiteral("flipV")).toBool() == item.flipV;
    if (placed) return xml;
    // Both alternatives are placed: the second starts at the fallback.
    static const QRegularExpression fallback(QStringLiteral("<\\w+:Fallback\\b"));
    const QRegularExpressionMatch f = fallback.match(xml);
    if (!f.hasMatch()) return withShapePlace(xml, item);
    return withShapePlace(xml.left(f.capturedStart()), item) + withShapePlace(xml.mid(f.capturedStart()), item);
}

// A frame (p:graphicFrame) for a chart made here, to be placed and named as a frame the reader kept is; `number` is its shape number.
QString newChartFrame(qint64 number)
{
    return QStringLiteral("<p:graphicFrame xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                          "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                          "<p:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                          "<c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"rId1\"/></a:graphicData></a:graphic></p:graphicFrame>").arg(number);
}

// The kept chart for a picture that holds a chart (PictureItem::chartXml) as the page's "charts" keep one: what the reader kept with the chart part the picture now holds in place of
// its own; or, for a chart made here, a part of its own and a frame numbered `number`.
QJsonObject chartEntryOf(const PictureItem &pic, const QJsonObject &was, qint64 number)
{
    QJsonObject kept = was;
    // A chart with no kept parts of its own (a copy pasted from another slide, say) does not name the workbook and shapes of the chart it came from: nothing would be there.
    QByteArray chartXml = pic.chartXml;
    if (kept.isEmpty()) {
        static const QRegularExpression external(QStringLiteral("<(\\w+:)?externalData\\b[^>]*?(/>|>.*?</(\\w+:)?externalData>)"), QRegularExpression::DotMatchesEverythingOption);
        static const QRegularExpression shapes(QStringLiteral("<(\\w+:)?userShapes\\b[^>]*?(/>|>.*?</(\\w+:)?userShapes>)"), QRegularExpression::DotMatchesEverythingOption);
        QString text = QString::fromUtf8(chartXml);
        text.remove(external);
        text.remove(shapes);
        chartXml = text.toUtf8();
    }
    const QString type = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml");
    if (kept.isEmpty()) {
        const QString main = QStringLiteral("/ppt/charts/chart1.xml");
        kept = QJsonObject{{"item", pic.id}, {"main", main}, {"frameXml", newChartFrame(number)}, {"parts", QJsonObject{{main, QJsonObject{{"type", type}, {"rels", QJsonArray()}}}}}};
    }
    QJsonObject parts = kept.value(QStringLiteral("parts")).toObject();
    const QString main = kept.value(QStringLiteral("main")).toString();
    QJsonObject part = parts.value(main).toObject();
    const QString now = QString::fromLatin1(chartXml.toBase64());
    // (A chart the person did not change is written as it was read.)
    if (part.value(QStringLiteral("data")).toString() != now) {
        part[QStringLiteral("data")] = now;
        parts[main] = part;
        kept[QStringLiteral("parts")] = parts;
    } else {
        kept[QStringLiteral("unchanged")] = true;
    }
    return kept;
}

QString Exporter::tree(const PageBase &page, const QString &part, const QString &where, QSet<qint64> *frameIds, QSet<qint64> *mediaIds)
{
    // The frames to write back first: their shape numbers are theirs, and the shapes are numbered above them.
    QStringList frames;
    QSet<qint64> frameNumbers;
    qint64 highest = 1;
    static const QRegularExpression numbers(QStringLiteral("<\\w+:cNvPr\\b[^>]*?\\bid=\"(\\d+)\""));
    auto take = [&](const QString &xml) {
        for (auto it = numbers.globalMatch(xml); it.hasNext();) {
            const qint64 n = it.next().captured(1).toLongLong();
            highest = std::max(highest, n);
            frameNumbers.insert(n);
            if (frameIds) frameIds->insert(n);
        }
    };
    for (const QJsonValue &v : page.props.value(QStringLiteral("unread")).toArray()) {
        const QString xml = keptFrame(v.toObject().value(QStringLiteral("xml")).toString(), where);
        if (xml.isEmpty()) continue;
        frames << xml;
        take(xml);
    }
    // The charts, diagrams, and equations: each picture the reader made of one that is still on the page is written as its frame (or the pair of
    // alternatives), in its place among the shapes.
    office::DrawingWriteContext ctx = context(part);
    QHash<QString, QJsonObject> charts;
    for (const QJsonValue &v : page.props.value(QStringLiteral("charts")).toArray()) charts.insert(v.toObject().value(QStringLiteral("item")).toString(), v.toObject());
    // A picture that holds a chart (one made here, or one the reader read as a chart), at any depth, is written as a chart with the numbers it holds now.
    QSet<QString> topLevel;
    for (const ItemPtr &it : page.items) topLevel.insert(it->id);
    {
        qint64 number = highest;
        walkItems(page.items, [&](const ItemPtr &it) { number = std::max<qint64>(number, it->sourceId); });
        walkItems(page.items, [&](const ItemPtr &it) {
            if (it->type() != ItemType::Picture) return;
            const auto &pic = static_cast<const PictureItem &>(*it);
            if (pic.chartXml.isEmpty()) return;
            const QJsonObject was = charts.value(it->id);
            if (!was.isEmpty() && !was.value(QStringLiteral("kind")).toString().isEmpty()) return;
            charts.insert(it->id, chartEntryOf(pic, was, ++number));
        });
    }
    QHash<QString, QString> chartFrames;
    walkItems(page.items, [&](const ItemPtr &it) {
        if (it->type() != ItemType::Picture || !charts.contains(it->id)) return;
        const QJsonObject kept = charts.value(it->id);
        const QString kind = kept.value(QStringLiteral("kind")).toString();
        // (A diagram or an equation inside a group is written as the picture it shows; only a chart is a frame at any depth.)
        if (!topLevel.contains(it->id) && !kind.isEmpty()) return;
        // (An object's frame is written after the shapes are numbered, as it takes a number of its own and one for its picture.)
        if (kind == QLatin1String("object")) {
            if (it->type() == ItemType::Picture && topLevel.contains(it->id) && !kept.value(QStringLiteral("data")).toString().isEmpty() && !office::dmlwrite::relatePicture(ctx, static_cast<const PictureItem &>(*it).imageId).isEmpty())
                chartFrames.insert(it->id, QString());
            return;
        }
        const QString xml = kind == QLatin1String("diagram") ? diagramFrame(kept, *it, part, where)
                          : kind == QLatin1String("equation") ? equationBlock(kept, *it, ctx, where)
                                                              : chartFrame(kept, *it, part, where);
        if (xml.isEmpty()) return;
        chartFrames.insert(it->id, xml);
        take(xml);
    });
    // (The frames of the charts inside groups are written where the group writes the picture.)
    ctx.rawFrame = [&](const Item &item) {
        if (item.type() == ItemType::Equation) return equationPair(static_cast<const EquationItem &>(item), ctx, where);
        return topLevel.contains(item.id) ? QString() : chartFrames.value(item.id);
    };
    ctx.emptyParagraphsInherit = frameIds != nullptr;   // a slide's (asked for the numbers of its frames), not a layout's or a master's
    ctx.nextShapeId = int(std::min<qint64>(highest + 1, 2000000000));
    // The tree's own number and the frames' are taken; the shapes read from a file keep theirs, which new ones leave alone.
    ctx.usedIds.insert(1);
    for (const qint64 n : std::as_const(frameNumbers))
        if (n <= INT_MAX) ctx.usedIds.insert(int(n));
    walkItems(page.items, [&](const ItemPtr &it) {
        if (it->sourceId >= 2 && (!chartFrames.contains(it->id) || charts.value(it->id).value(QStringLiteral("kind")).toString() == QLatin1String("object"))) ctx.reservedIds.insert(it->sourceId);
    });
    // A picture that stands for a chart is written as the chart's frame, with the frame's number.
    ItemList drawn;
    for (const ItemPtr &it : page.items)
        if (!chartFrames.contains(it->id)) drawn.push_back(it);
    office::assignShapeIds(drawn, ctx);
    walkItems(page.items, [&](const ItemPtr &it) {
        if (chartFrames.contains(it->id) && chartFrames.value(it->id).isEmpty() && charts.value(it->id).value(QStringLiteral("kind")).toString() == QLatin1String("object"))
            chartFrames[it->id] = objectFrame(charts.value(it->id), *it, part, ctx, where, frameIds);
    });
    QStringList left;
    auto write = [&](const ItemList &list, size_t from, size_t to) {
        return QString::fromUtf8(fragment([&](QXmlStreamWriter &w) {
            for (size_t i = from; i < to; ++i) {
                const ItemPtr &it = list[i];
                // A table keeps the style it names (writeTable); the drawing writer's own form has none.
                if (it->type() == ItemType::Table) office::writeTable(w, static_cast<const TableItem &>(*it), ctx);
                else office::writeShape(w, *it, ctx, &left);
            }
        }));
    };
    QString shapes;
    size_t from = 0;
    for (size_t i = 0; i < page.items.size(); ++i) {
        if (!chartFrames.contains(page.items[i]->id)) continue;
        shapes += write(page.items, from, i) + chartFrames.value(page.items[i]->id);
        from = i + 1;
    }
    shapes += write(page.items, from, page.items.size());
    for (const QString &text : std::as_const(left)) note(QStringLiteral("%1: %2").arg(where, text));
    // A shape written with the number it came with is still what an animation aims at.
    if (frameIds)
        walkItems(drawn, [&](const ItemPtr &it) {
            if (it->sourceId >= 2 && ctx.shapeIds.value(it->id) == it->sourceId) frameIds->insert(it->sourceId);
        });
    // The pictures written with the movie or sound they play.
    if (mediaIds)
        walkItems(drawn, [&](const ItemPtr &it) {
            if (it->type() != ItemType::Picture || !ctx.shapeIds.contains(it->id)) return;
            const auto &p = static_cast<const PictureItem &>(*it);
            if (!p.imageId.isEmpty() && !p.mediaKind.isEmpty() && (m_doc.media.contains(p.mediaId) || !p.mediaLink.isEmpty()))
                mediaIds->insert(ctx.shapeIds.value(it->id));
        });
    return QStringLiteral("<p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                          "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>")
           + shapes + frames.join(QString()) + QStringLiteral("</p:spTree>");
}

// The background of a page: its fill, or (a master has to have one) the theme's first.
QString Exporter::background(const PageBase &page, const QString &part, bool required, const QString &where)
{
    const QString themed = required ? QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>") : QString();
    // A background nobody changed is written as the file had it (its theme colors and references to the theme's styles stay what they were; a page that had none has none).
    // One that names a part of the file (a picture) is written again, as it holds relationship numbers of the old file.
    const QJsonObject kept = page.props.value(QStringLiteral("bgKept")).toObject();
    if (kept.value(QStringLiteral("none")).toBool() && page.background.isNone()) return QString();
    if (const QString xml = kept.value(QStringLiteral("xml")).toString();
        !xml.isEmpty() && xml.startsWith(QLatin1String("<p:bg")) && !xml.contains(QLatin1String("r:embed")) && !xml.contains(QLatin1String("r:link")) && !xml.contains(QLatin1String("r:id")) &&
        page.background.toJson() == kept.value(QStringLiteral("fill")).toObject())
        return xml;
    if (page.background.isNone()) return themed;
    office::DrawingWriteContext ctx = context(part);
    QByteArray fill = fragment([&](QXmlStreamWriter &w) { office::writeFill(w, page.background, ctx); });
    if (fill.isEmpty() && page.background.type == Fill::Solid) {
        // The drawing writer has no form for it yet: a plain color is still written.
        const QColor c = page.background.color.kind() == ColorRef::Rgb ? page.background.color.rgbValue() : page.background.color.resolve(m_doc.colors);
        if (c.isValid()) fill = QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex(c)).toUtf8();
    }
    if (fill.isEmpty()) {
        note(QCoreApplication::translate("PptxExport", "%1: the background can't be saved yet.").arg(where));
        return themed;
    }
    return QStringLiteral("<p:bg><p:bgPr>") + QString::fromUtf8(fill) + QStringLiteral("<a:effectLst/></p:bgPr></p:bg>");
}

// A master's text styles, from what its title and text placeholders look like.
QString Exporter::textStyles(const MasterOut &master)
{
    const QTextDocument *titleDoc = nullptr, *bodyDoc = nullptr;
    for (const ItemPtr &it : master.page->items) {
        if (it->type() != ItemType::Text) continue;
        const QString type = placeholderType(*it);
        const QTextDocument *sd = m_doc.storyDoc(static_cast<const TextItem &>(*it).storyId);
        if (!titleDoc && (type == QLatin1String("title") || type == QLatin1String("ctrTitle"))) titleDoc = sd;
        else if (!bodyDoc && (type == QLatin1String("body") || type == QLatin1String("obj"))) bodyDoc = sd;
    }
    const QVector<Level> title = levelsOf(titleDoc, true, master.theme), body = levelsOf(bodyDoc, false, master.theme);
    static const double bodySizes[9] = {28, 24, 20, 18, 18, 18, 18, 18, 18};
    const bool dark = isDarkBackground(master.page->background);
    QString out = QStringLiteral("<p:txStyles><p:titleStyle>") + levelXml(0, title[0], master.theme, dark, true, false, 44) + QStringLiteral("</p:titleStyle><p:bodyStyle>");
    for (int i = 0; i < 9; ++i) out += levelXml(i, body[i], master.theme, dark, false, true, bodySizes[i]);
    return out + QStringLiteral("</p:bodyStyle>") + plainLevels("p:otherStyle", 18) + QStringLiteral("</p:txStyles>");
}

QString Exporter::titleOf(const Page &slide, int number) const
{
    for (const ItemPtr &it : slide.items) {
        const QString type = placeholderType(*it);
        if ((type != QLatin1String("title") && type != QLatin1String("ctrTitle")) || it->type() != ItemType::Text) continue;
        const QTextDocument *sd = m_doc.storyDoc(static_cast<const TextItem &>(*it).storyId);
        const QString text = sd ? sd->toPlainText().section(QChar(QChar::LineSeparator), 0, 0).section(QLatin1Char('\n'), 0, 0).trimmed() : QString();
        if (!text.isEmpty()) return text;
    }
    return slide.title.isEmpty() ? QStringLiteral("Slide %1").arg(number) : slide.title;
}

QByteArray Exporter::thumbnail() const { return renderThumbnail(m_doc); }

void Exporter::add(const QString &part, const QString &type, const QString &xml)
{
    const QByteArray bytes = (kDeclaration + withStandardFontNames(xml)).toUtf8();
    scanFonts(part, bytes);
    m_pkg.addPart(part, bytes, type);
}

// What a part's text is set in, for the fonts the file will carry: only when something is to be carried.
void Exporter::scanFonts(const QString &part, const QByteArray &xml)
{
    if (!m_embedFonts && m_keptFonts.isEmpty()) return;
    const auto fonts = m_partFonts.value(part, qMakePair(m_theme.majorLatin, m_theme.minorLatin));
    m_fontUse.scan(xml, fonts.first, fonts.second);
}

void Exporter::writeMaster(int i)
{
    MasterOut &master = m_masters[i];
    const MasterPage &page = *master.page;
    const QString where = QCoreApplication::translate("PptxExport", "Slide master %1").arg(i + 1);
    // Its links come first: the layouts it lists, and its theme.
    QString layoutIds;
    for (int l : std::as_const(master.layouts)) {
        const QString rel = m_pkg.addRelationship(master.part, kRelBase + QStringLiteral("slideLayout"), m_layouts[l].part);
        layoutIds += QStringLiteral("<p:sldLayoutId id=\"%1\" r:id=\"%2\"/>").arg(m_layouts[l].id).arg(rel);
    }
    m_pkg.addRelationship(master.part, kRelBase + QStringLiteral("theme"), master.themePart);
    m_pkg.addPart(master.themePart, withStandardFontNames(master.themeXml), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));

    QString xml = QStringLiteral("<p:sldMaster%1 preserve=\"%2\"><p:cSld%3>").arg(kNamespaces, isPreserved(page) ? QStringLiteral("1") : QStringLiteral("0"), page.name.isEmpty() ? QString() : QStringLiteral(" name=\"%1\"").arg(esc(page.name)));
    const QString fill = background(page, master.part, true, where);
    xml += fill + tree(page, master.part, where, nullptr) + QStringLiteral("</p:cSld>");
    xml += colorMapXml(isDarkBackground(page.background));
    xml += QStringLiteral("<p:sldLayoutIdLst>") + layoutIds + QStringLiteral("</p:sldLayoutIdLst>");
    xml += textStyles(master) + QStringLiteral("</p:sldMaster>");
    add(master.part, kTypeBase + QStringLiteral("slideMaster+xml"), xml);
}

// A layout's text placeholders say how each level of their text looks (the list style of the shape), so the slides on the layout take what was set for a level here
// where the layout's text styles differ from the master's: a level the placeholder has a paragraph for is written, from that paragraph. Shapes are matched to the layout's
// placeholders in order; if their numbers differ, the file is left as it is.
QString Exporter::withLevelStyles(const QString &xml, const MasterPage &page, const MasterOut &master)
{
    QVector<const TextItem *> bodies;
    for (const ItemPtr &it : page.items) {
        const QString type = placeholderType(*it);
        if (it->type() == ItemType::Text && (type == QLatin1String("body") || type == QLatin1String("obj"))) bodies << static_cast<const TextItem *>(it.get());
    }
    if (bodies.isEmpty()) return xml;
    // (The text is changed where it stands, not read and written again: the rest of the layout stays as it was written.)
    struct Spot {
        qsizetype at;
        QString idx;
    };
    QVector<Spot> lists;
    for (qsizetype from = 0;;) {
        const qsizetype open = xml.indexOf(QLatin1String("<p:sp>"), from);
        if (open < 0) break;
        const qsizetype close = xml.indexOf(QLatin1String("</p:sp>"), open);
        if (close < 0) return xml;
        from = close;
        const qsizetype ph = xml.indexOf(QLatin1String("<p:ph"), open);
        if (ph < 0 || ph > close) continue;
        const qsizetype phEnd = xml.indexOf(QLatin1Char('>'), ph);
        const QString tag = xml.mid(ph, phEnd - ph);
        static const QRegularExpression typeAttr(QStringLiteral("\\btype=\"([^\"]*)\""));
        const QRegularExpressionMatch m = typeAttr.match(tag);
        const QString type = m.hasMatch() ? m.captured(1) : QString();
        if (!type.isEmpty() && type != QLatin1String("body") && type != QLatin1String("obj")) continue;
        const qsizetype lst = xml.indexOf(QLatin1String("<a:lstStyle/>"), ph);
        if (lst < 0 || lst > close) return xml;
        static const QRegularExpression idxAttr(QStringLiteral("\\bidx=\"([^\"]*)\""));
        const QRegularExpressionMatch im = idxAttr.match(tag);
        lists << Spot{lst, im.hasMatch() ? im.captured(1) : QStringLiteral("0")};
    }
    const bool dark = isDarkBackground(page.background);
    static const double bodySizes[9] = {28, 24, 20, 18, 18, 18, 18, 18, 18};
    QString out = xml;
    for (int k = int(lists.size()) - 1; k >= 0; --k) {   // (from the last, so that the places found stay where they are)
        // The shape's text is the layout's placeholder with the same number (idx), whatever else the layout has.
        const TextItem *body = nullptr;
        for (const TextItem *b : std::as_const(bodies)) {
            const QStringList parts = b->role.split(QLatin1Char(':'));
            if ((parts.size() > 2 && !parts[2].isEmpty() ? parts[2] : QStringLiteral("0")) == lists[k].idx) body = b;
        }
        if (!body) continue;
        QVector<bool> said;
        const QVector<Level> levels = levelsOf(m_doc.storyDoc(body->storyId), false, master.theme, &said);
        QString inner;
        for (int i = 0; i < 9; ++i)
            if (said[i] && (levels[i].bullet || levels[i].marL >= 0)) inner += levelXml(i, levels[i], master.theme, dark, false, true, bodySizes[i], true);
        if (!inner.isEmpty()) out.replace(lists[k].at, int(QLatin1String("<a:lstStyle/>").size()), QStringLiteral("<a:lstStyle>") + inner + QStringLiteral("</a:lstStyle>"));
    }
    return out;
}

void Exporter::writeLayout(int i)
{
    const LayoutOut &layout = m_layouts[i];
    const MasterPage &page = *layout.page;
    const QString where = QCoreApplication::translate("PptxExport", "Layout \"%1\"").arg(page.name);
    m_pkg.addRelationship(layout.part, kRelBase + QStringLiteral("slideMaster"), m_masters[layout.master].part);
    const QString type = page.props.value(QStringLiteral("type")).toString();
    const bool custom = !isLayoutType(type);
    QString xml = QStringLiteral("<p:sldLayout%1").arg(kNamespaces);
    if (!custom) xml += QStringLiteral(" type=\"%1\"").arg(type);
    if (!page.showMasterItems) xml += QStringLiteral(" showMasterSp=\"0\"");
    xml += isPreserved(page) ? QStringLiteral(" preserve=\"1\"") : QStringLiteral(" preserve=\"0\"");
    if (custom) xml += QStringLiteral(" userDrawn=\"1\"");
    xml += QStringLiteral("><p:cSld name=\"%1\">").arg(esc(page.name));
    const QString fill = background(page, layout.part, false, where);
    xml += fill + tree(page, layout.part, where, nullptr) + QStringLiteral("</p:cSld>");
    xml += QStringLiteral("<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>");
    xml = withLevelStyles(xml, page, m_masters[layout.master]);
    add(layout.part, kTypeBase + QStringLiteral("slideLayout+xml"), xml);
}

void Exporter::writeSlide(int i)
{
    const Page &page = *m_doc.pages[i];
    const QString part = QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1);
    const QString where = QCoreApplication::translate("PptxExport", "Slide %1").arg(i + 1);
    m_pkg.addRelationship(part, kRelBase + QStringLiteral("slideLayout"), m_layouts[m_slideLayout[i]].part);
    if (m_notesNumber[i]) m_pkg.addRelationship(part, kRelBase + QStringLiteral("notesSlide"), QStringLiteral("/ppt/notesSlides/notesSlide%1.xml").arg(m_notesNumber[i]));

    QString xml = QStringLiteral("<p:sld%1").arg(kNamespaces);
    if (!page.showMasterItems) xml += QStringLiteral(" showMasterSp=\"0\"");
    if (page.hidden) xml += QStringLiteral(" show=\"0\"");
    xml += QStringLiteral("><p:cSld>");
    QSet<qint64> frameIds, mediaIds;
    const QString fill = background(page, part, false, where);
    xml += fill + tree(page, part, where, &frameIds, &mediaIds) + QStringLiteral("</p:cSld>");
    xml += QStringLiteral("<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>");
    // The transition as read when it is still that one (the XML keeps what the show has no form for, a sound
    // included); else as it is now, made from the show's settings.
    const deck::Transition transition = deck::transitionOf(page);
    const QString readXml = page.props.value(QStringLiteral("transitionXml")).toString();
    QString transitionOut = deck::transitionXml(transition);
    if (!readXml.isEmpty() && deck::transitionFromXml(readXml) == transition) {
        QDomDocument dom;
        if (parsesAlone(readXml, &dom) && refersToParts(dom.documentElement())) {
            note(QCoreApplication::translate("PptxExport", "%1: the transition's sound isn't kept; the transition is saved without it.").arg(where));
        } else if (const QString kept = keptSlideXml(page, "transitionXml", QCoreApplication::translate("PptxExport", "transition"), where, frameIds, mediaIds); !kept.isEmpty()) {
            transitionOut = kept;
        }
    }
    xml += transitionOut;
    // The animation, but for the effects of shapes that are gone: the ones on the other shapes stay, as the animation pane writes them.
    int dropped = 0;
    const QString timing = deck::timingForSave(m_doc, page, [&](int shape) { return frameIds.contains(shape); }, &dropped);
    if (dropped > 0) note(QCoreApplication::translate("PptxExport", "%1: animation effects of shapes that are gone were left out.").arg(where));
    xml += keptSlideXml(page, "timingXml", QCoreApplication::translate("PptxExport", "animation"), where, frameIds, mediaIds, &timing);
    xml += QStringLiteral("</p:sld>");
    add(part, kTypeBase + QStringLiteral("slide+xml"), xml);
    writeComments(i, part);
}

void Exporter::writeNotesSlide(int slide, int number)
{
    const Page &page = *m_doc.pages[slide];
    const QString part = QStringLiteral("/ppt/notesSlides/notesSlide%1.xml").arg(number);
    m_pkg.addRelationship(part, kRelBase + QStringLiteral("notesMaster"), kNotesMasterPart);
    m_pkg.addRelationship(part, kRelBase + QStringLiteral("slide"), QStringLiteral("/ppt/slides/slide%1.xml").arg(slide + 1));
    // The notes are a text body; one with no paragraph at all is not valid, so an empty one is written.
    office::DrawingWriteContext ctx = context(part);
    QByteArray paragraphs = fragment([&](QXmlStreamWriter &w) { office::writeParagraphs(w, m_doc.storyDoc(page.notesStoryId), ctx); });
    if (paragraphs.isEmpty()) paragraphs = "<a:p><a:endParaRPr lang=\"en-US\"/></a:p>";
    QString xml = QStringLiteral("<p:notes%1><p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                                 "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>"
                                 "<p:sp><p:nvSpPr><p:cNvPr id=\"2\" name=\"Slide Image Placeholder 1\"/><p:cNvSpPr><a:spLocks noGrp=\"1\" noRot=\"1\" noChangeAspect=\"1\"/></p:cNvSpPr>"
                                 "<p:nvPr><p:ph type=\"sldImg\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp>"
                                 "<p:sp><p:nvSpPr><p:cNvPr id=\"3\" name=\"Notes Placeholder 2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr>"
                                 "<p:nvPr><p:ph type=\"body\" idx=\"1\"/></p:nvPr></p:nvSpPr><p:spPr/><p:txBody><a:bodyPr/><a:lstStyle/>")
                      .arg(kNamespaces);
    xml += QString::fromUtf8(paragraphs);
    xml += QStringLiteral("</p:txBody></p:sp></p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:notes>");
    add(part, kTypeBase + QStringLiteral("notesSlide+xml"), xml);
}

// The page's background: its own color, or the theme's.
static QString backgroundXml(const QColor &color)
{
    if (!color.isValid()) return QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg>");
    return QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>").arg(color.name().mid(1).toUpper());
}

// The namespace and extension of the handout master's own settings: whether the handouts lie across, and where their slides go (the standard has no place for either).
static const QString kDeckNs = QStringLiteral("http://jeffoffice.com/ns/deck"), kHandoutExtUri = QStringLiteral("{7B0F4F1E-5E7A-4C55-9D3B-2F6E1A9C0D11}");

static QString handoutExtensionXml(const HandoutMaster &m, const HandoutMaster &standard)
{
    if (!m.landscape && m.area == standard.area) return QString();
    return QStringLiteral("<p:extLst><p:ext uri=\"%1\"><jo:handout xmlns:jo=\"%2\" landscape=\"%3\" area=\"%4 %5 %6 %7\"/></p:ext></p:extLst>")
        .arg(kHandoutExtUri, kDeckNs).arg(m.landscape ? 1 : 0).arg(m.area.x(), 0, 'f', 6).arg(m.area.y(), 0, 'f', 6).arg(m.area.width(), 0, 'f', 6).arg(m.area.height(), 0, 'f', 6);
}

// A handout master the file had, with its places and background changed to the layout the presentation has now: each of the header, date, footer, and number places whose place changed, the
// background where it changed, and the extension that holds the rest. All else stays as it was.
static QString editedHandoutLayout(const QByteArray &xml, const HandoutMaster &now, const HandoutMaster &was, const QSizeF &page, const HandoutMaster &standard)
{
    QDomDocument dom;
    if (!dom.setContent(xml, true)) return QString::fromUtf8(xml);
    const QString pns = QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"), ans = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main");
    QDomElement root = dom.documentElement();
    auto setPlace = [&](const char *type, const QRectF &place) {
        const QDomNodeList shapes = dom.elementsByTagNameNS(pns, QStringLiteral("sp"));
        for (int i = 0; i < shapes.size(); ++i) {
            const QDomElement sp = shapes.at(i).toElement();
            const QDomNodeList phs = sp.elementsByTagNameNS(pns, QStringLiteral("ph"));
            if (phs.isEmpty() || phs.at(0).toElement().attribute(QStringLiteral("type")) != QLatin1String(type)) continue;
            QDomElement shapeProps;
            for (QDomElement c = sp.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                if (c.localName() == QLatin1String("spPr")) shapeProps = c;
            if (shapeProps.isNull()) return;
            QDomElement xfrm;
            for (QDomElement c = shapeProps.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                if (c.localName() == QLatin1String("xfrm")) xfrm = c;
            if (xfrm.isNull()) {
                xfrm = dom.createElementNS(ans, QStringLiteral("a:xfrm"));
                shapeProps.insertBefore(xfrm, shapeProps.firstChild());
            }
            while (!xfrm.firstChild().isNull()) xfrm.removeChild(xfrm.firstChild());
            QDomElement off = dom.createElementNS(ans, QStringLiteral("a:off")), ext = dom.createElementNS(ans, QStringLiteral("a:ext"));
            off.setAttribute(QStringLiteral("x"), QString::number(office::pointsToEmu(place.x() * page.width())));
            off.setAttribute(QStringLiteral("y"), QString::number(office::pointsToEmu(place.y() * page.height())));
            ext.setAttribute(QStringLiteral("cx"), QString::number(office::pointsToEmu(place.width() * page.width())));
            ext.setAttribute(QStringLiteral("cy"), QString::number(office::pointsToEmu(place.height() * page.height())));
            xfrm.appendChild(off);
            xfrm.appendChild(ext);
            return;
        }
    };
    if (now.header != was.header) setPlace("hdr", now.header);
    if (now.date != was.date) setPlace("dt", now.date);
    if (now.footer != was.footer) setPlace("ftr", now.footer);
    if (now.number != was.number) setPlace("sldNum", now.number);
    if (now.background.isValid() != was.background.isValid() || (now.background.isValid() && now.background.rgb() != was.background.rgb())) {
        QDomDocument piece;
        piece.setContent(QStringLiteral("<r xmlns:p=\"%1\" xmlns:a=\"%2\">%3</r>").arg(pns, ans, backgroundXml(now.background)), true);
        for (QDomElement c = root.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            if (c.localName() != QLatin1String("cSld")) continue;
            QDomElement bg;
            for (QDomElement k = c.firstChildElement(); !k.isNull(); k = k.nextSiblingElement())
                if (k.localName() == QLatin1String("bg")) bg = k;
            const QDomNode made = dom.importNode(piece.documentElement().firstChildElement(), true);
            if (bg.isNull()) c.insertBefore(made, c.firstChild());
            else c.replaceChild(made, bg);
            break;
        }
    }
    if (now.landscape != was.landscape || now.area != was.area) {
        // The extension with the settings of our own: replaced where there, else made (the extension list is the last child).
        QDomElement list;
        for (QDomElement c = root.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
            if (c.localName() == QLatin1String("extLst")) list = c;
        if (list.isNull()) {
            list = dom.createElementNS(pns, QStringLiteral("p:extLst"));
            root.appendChild(list);
        }
        for (QDomElement c = list.firstChildElement(); !c.isNull();) {
            const QDomElement next = c.nextSiblingElement();
            if (c.attribute(QStringLiteral("uri")) == kHandoutExtUri) list.removeChild(c);
            c = next;
        }
        const QString extXml = handoutExtensionXml(now, standard);
        if (!extXml.isEmpty()) {
            QDomDocument piece;
            piece.setContent(QStringLiteral("<r xmlns:p=\"%1\">%2</r>").arg(pns, extXml), true);
            const QDomElement ext = piece.documentElement().firstChildElement().firstChildElement();
            list.appendChild(dom.importNode(ext, true));
        }
        if (list.firstChildElement().isNull()) root.removeChild(list);
    }
    return QString::fromUtf8(dom.toByteArray());
}

// The hf element of a notes master or handout master: which of its header, footer, date, and number are on.
static QString hfElement(const PrintHeaderFooter &hf)
{
    return QStringLiteral("<p:hf hdr=\"%1\" ftr=\"%2\" dt=\"%3\" sldNum=\"%4\"/>").arg(hf.header ? 1 : 0).arg(hf.footer ? 1 : 0).arg(hf.date ? 1 : 0).arg(hf.pageNumber ? 1 : 0);
}

void Exporter::writeNotesMaster()
{
    m_pkg.addRelationship(kNotesMasterPart, kRelBase + QStringLiteral("theme"), m_notesThemePart);
    m_pkg.addPart(m_notesThemePart, withStandardFontNames(m_themeXml), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    // The places on the notes page, in the standard's proportions for the page's size.
    const qint64 nw = office::pointsToEmu(m_notesSize.width()), nh = office::pointsToEmu(m_notesSize.height());
    auto x = [&](qint64 v) { return v * nw / kNotesWidth; };
    auto y = [&](qint64 v) { return v * nh / kNotesHeight; };
    const QSizeF slide = m_doc.pageSize();
    const qint64 imageW = x(5486400), imageH = slide.width() > 0 ? qint64(imageW * slide.height() / slide.width()) : y(3086100);
    auto place = [&](qint64 px, qint64 py, qint64 cx, qint64 cy) {
        return QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>").arg(px).arg(py).arg(cx).arg(cy);
    };
    // The places: the standard's proportions, or the layout the presentation has (the master's fractions of the page).
    struct Box {
        qint64 x, y, cx, cy;
    };
    Box stdImage{x(685800), y(1143000), imageW, imageH}, stdBody{x(685800), y(4400550), x(5486400), y(3600450)}, stdHeader{0, 0, x(2971800), y(458788)}, stdDate{x(3884613), 0, x(2971800), y(458788)},
        stdFooter{0, y(8685213), x(2971800), y(458788)}, stdNumber{x(3884613), y(8685213), x(2971800), y(458788)};
    const bool own = hasNotesMaster(m_doc);
    const NotesMaster layout = notesMasterOf(m_doc, m_notesSize);
    const NotesPageGeometry geo = notesPageGeometry(layout, m_notesSize, slide);
    auto boxOf = [&](const QRectF &r, const Box &standard) { return own ? Box{office::pointsToEmu(r.x()), office::pointsToEmu(r.y()), office::pointsToEmu(r.width()), office::pointsToEmu(r.height())} : standard; };
    const Box boxImage = boxOf(geo.slideImage, stdImage), boxBody = boxOf(geo.body, stdBody), boxHeader = boxOf(geo.header, stdHeader), boxDate = boxOf(geo.date, stdDate),
              boxFooter = boxOf(geo.footer, stdFooter), boxNumber = boxOf(geo.number, stdNumber);
    auto small = [&](int id, const QString &name, const QString &ph, const Box &b, const QString &align, const QString &anchor, const QString &text) {
        return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph %3/></p:nvPr></p:nvSpPr><p:spPr>%4</p:spPr>"
                              "<p:txBody><a:bodyPr vert=\"horz\" lIns=\"91440\" tIns=\"45720\" rIns=\"91440\" bIns=\"45720\" rtlCol=\"0\"%5/><a:lstStyle><a:lvl1pPr algn=\"%6\"><a:defRPr sz=\"1200\"/></a:lvl1pPr></a:lstStyle>"
                              "%7</p:txBody></p:sp>")
            .arg(id).arg(name, ph, place(b.x, b.y, b.cx, b.cy), anchor.isEmpty() ? QString() : QStringLiteral(" anchor=\"%1\"").arg(anchor), align, text);
    };
    // The header and footer the presentation chose for its notes pages: their texts sit in the master's own places, and the hf element says which are on.
    const bool chose = hasPrintHeaderFooter(m_doc, PrintedPages::Notes);
    const PrintHeaderFooter hf = printHeaderFooter(m_doc, PrintedPages::Notes);
    auto typed = [](const QString &text) {
        return text.isEmpty() ? QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>") : QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(esc(text));
    };
    const QString empty = typed(chose ? hf.headerText : QString());
    const QString footerParagraph = typed(chose ? hf.footerText : QString());
    const QString number = QStringLiteral("<a:p><a:fld id=\"{9A1B2C3D-0000-4000-8000-000000000001}\" type=\"slidenum\"><a:rPr lang=\"en-US\"/><a:t>\u2039#\u203A</a:t></a:fld><a:endParaRPr lang=\"en-US\"/></a:p>");
    const QString date = QStringLiteral("<a:p><a:fld id=\"{9A1B2C3D-0000-4000-8000-000000000002}\" type=\"datetimeFigureOut\"><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:fld><a:endParaRPr lang=\"en-US\"/></a:p>")
                             .arg(esc(m_doc.props.modified.date().toString(Qt::ISODate)));
    const QString dateParagraph = chose && !hf.dateText.isEmpty() ? typed(hf.dateText) : date;
    static const char *const samples[5] = {"Notes text: change its look here", "Level 2 text", "Level 3 text", "Level 4 text", "Level 5 text"};
    QString texts;
    for (int level = 0; level < 5; ++level)
        texts += QStringLiteral("<a:p><a:pPr lvl=\"%1\"/><a:r><a:rPr lang=\"en-US\"/><a:t>%2</a:t></a:r></a:p>").arg(level).arg(QLatin1String(samples[level]));

    QString xml = QStringLiteral("<p:notesMaster%1><p:cSld>").arg(kNamespaces) + backgroundXml(own ? layout.background : QColor()) + QStringLiteral("<p:spTree>"
                                 "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                                 "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>");
    xml += small(2, QStringLiteral("Header Placeholder 1"), QStringLiteral("type=\"hdr\" sz=\"quarter\""), boxHeader, QStringLiteral("l"), QString(), empty);
    xml += small(3, QStringLiteral("Date Placeholder 2"), QStringLiteral("type=\"dt\" idx=\"1\""), boxDate, QStringLiteral("r"), QString(), dateParagraph);
    xml += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"4\" name=\"Slide Image Placeholder 3\"/><p:cNvSpPr><a:spLocks noGrp=\"1\" noRot=\"1\" noChangeAspect=\"1\"/></p:cNvSpPr>"
                          "<p:nvPr><p:ph type=\"sldImg\" idx=\"2\"/></p:nvPr></p:nvSpPr><p:spPr>%1<a:noFill/><a:ln w=\"12700\"><a:solidFill><a:prstClr val=\"black\"/></a:solidFill></a:ln></p:spPr></p:sp>")
               .arg(place(boxImage.x, boxImage.y, boxImage.cx, boxImage.cy));
    xml += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"Notes Placeholder 4\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph type=\"body\" sz=\"quarter\" idx=\"3\"/></p:nvPr></p:nvSpPr>"
                          "<p:spPr>%1</p:spPr><p:txBody><a:bodyPr vert=\"horz\" lIns=\"91440\" tIns=\"45720\" rIns=\"91440\" bIns=\"45720\" rtlCol=\"0\"/><a:lstStyle/>%2</p:txBody></p:sp>")
               .arg(place(boxBody.x, boxBody.y, boxBody.cx, boxBody.cy), texts);
    xml += small(6, QStringLiteral("Footer Placeholder 5"), QStringLiteral("type=\"ftr\" sz=\"quarter\" idx=\"4\""), boxFooter, QStringLiteral("l"), QStringLiteral("b"), footerParagraph);
    xml += small(7, QStringLiteral("Slide Number Placeholder 6"), QStringLiteral("type=\"sldNum\" sz=\"quarter\" idx=\"5\""), boxNumber, QStringLiteral("r"), QStringLiteral("b"), number);
    xml += QStringLiteral("</p:spTree></p:cSld><p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" "
                          "accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>");
    if (chose) xml += hfElement(hf);
    xml += QStringLiteral("<p:notesStyle>");
    for (int i = 0; i < 9; ++i)
        xml += QStringLiteral("<a:lvl%1pPr marL=\"%2\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\"><a:defRPr sz=\"1200\" kern=\"1200\">"
                              "<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl%1pPr>")
                   .arg(i + 1).arg(457200 * i);
    xml += QStringLiteral("</p:notesStyle></p:notesMaster>");
    add(kNotesMasterPart, kTypeBase + QStringLiteral("notesMaster+xml"), xml);
}

// A handout master the file had, with the choice changed in it: the hf element, and the text of each place whose text the person changed. All else stays as it was.
static QString editedHandoutMaster(const QByteArray &kept, const PrintHeaderFooter &was, const PrintHeaderFooter &hf, bool hadChoice)
{
    QDomDocument dom;
    if (!dom.setContent(kept, true)) return QString::fromUtf8(kept);
    const QString pns = QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"), ans = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main");
    QDomElement root = dom.documentElement();
    // The hf element: in place, else after the color map.
    QDomElement hfElement = root.firstChildElement();
    while (!hfElement.isNull() && !(hfElement.localName() == QLatin1String("hf") && hfElement.namespaceURI() == pns)) hfElement = hfElement.nextSiblingElement();
    if (hfElement.isNull()) {
        hfElement = dom.createElementNS(pns, QStringLiteral("p:hf"));
        QDomElement after = root.firstChildElement();
        while (!after.isNull() && after.localName() != QLatin1String("clrMap")) after = after.nextSiblingElement();
        if (after.isNull()) root.appendChild(hfElement);
        else root.insertAfter(hfElement, after);
    }
    hfElement.setAttribute(QStringLiteral("hdr"), hf.header ? 1 : 0);
    hfElement.setAttribute(QStringLiteral("ftr"), hf.footer ? 1 : 0);
    hfElement.setAttribute(QStringLiteral("dt"), hf.date ? 1 : 0);
    hfElement.setAttribute(QStringLiteral("sldNum"), hf.pageNumber ? 1 : 0);
    // The texts that changed.
    auto setText = [&](const char *type, const QString &text, bool isDate) {
        const QDomNodeList shapes = dom.elementsByTagNameNS(pns, QStringLiteral("sp"));
        for (int i = 0; i < shapes.size(); ++i) {
            const QDomElement sp = shapes.at(i).toElement();
            const QDomNodeList phs = sp.elementsByTagNameNS(pns, QStringLiteral("ph"));
            if (phs.isEmpty() || phs.at(0).toElement().attribute(QStringLiteral("type")) != QLatin1String(type)) continue;
            const QDomNodeList bodies = sp.elementsByTagNameNS(pns, QStringLiteral("txBody"));
            if (bodies.isEmpty()) continue;
            QDomElement body = bodies.at(0).toElement();
            for (QDomElement p = body.firstChildElement(); !p.isNull();) {
                const QDomElement next = p.nextSiblingElement();
                if (p.localName() == QLatin1String("p") && p.namespaceURI() == ans) body.removeChild(p);
                p = next;
            }
            QDomElement para = dom.createElementNS(ans, QStringLiteral("a:p"));
            if (!text.isEmpty()) {
                QDomElement run = dom.createElementNS(ans, QStringLiteral("a:r"));
                QDomElement props = dom.createElementNS(ans, QStringLiteral("a:rPr"));
                props.setAttribute(QStringLiteral("lang"), QStringLiteral("en-US"));
                QDomElement t = dom.createElementNS(ans, QStringLiteral("a:t"));
                t.appendChild(dom.createTextNode(text));
                run.appendChild(props);
                run.appendChild(t);
                para.appendChild(run);
            } else if (isDate) {
                QDomElement field = dom.createElementNS(ans, QStringLiteral("a:fld"));
                field.setAttribute(QStringLiteral("id"), QStringLiteral("{9A1B2C3D-0000-4000-8000-000000000004}"));
                field.setAttribute(QStringLiteral("type"), QStringLiteral("datetimeFigureOut"));
                QDomElement t = dom.createElementNS(ans, QStringLiteral("a:t"));
                t.appendChild(dom.createTextNode(QDate::currentDate().toString(Qt::ISODate)));
                field.appendChild(dom.createElementNS(ans, QStringLiteral("a:rPr")));
                field.lastChildElement().setAttribute(QStringLiteral("lang"), QStringLiteral("en-US"));
                field.appendChild(t);
                para.appendChild(field);
            } else {
                para.appendChild(dom.createElementNS(ans, QStringLiteral("a:endParaRPr")));
            }
            body.appendChild(para);
            return;
        }
    };
    if (hf.headerText != was.headerText || (!hadChoice && !hf.headerText.isEmpty())) setText("hdr", hf.headerText, false);
    if (hf.footerText != was.footerText || (!hadChoice && !hf.footerText.isEmpty())) setText("ftr", hf.footerText, false);
    if (hf.dateText != was.dateText || (!hadChoice && !hf.dateText.isEmpty())) setText("dt", hf.dateText, true);
    return QString::fromUtf8(dom.toByteArray());
}

// The handout master: only where the presentation chose the header and footer of its handouts (the places are the standard's, for a page of the notes' size).
void Exporter::writeHandoutMaster()
{
    if (m_handoutMasterRel.isEmpty()) return;
    // A master the file had keeps all it had: its relationships, with the same ids, and the parts they name (its own theme, its pictures), under a free name where theirs is taken.
    const QJsonArray kept = keptHandoutRelationships(m_doc);
    bool hasTheme = false;
    QString keptXml = QString::fromUtf8(keptHandoutMaster(m_doc));
    for (const QJsonValue &v : kept) {
        const QJsonObject o = v.toObject();
        QString target = o.value(QStringLiteral("target")).toString();
        const bool external = o.value(QStringLiteral("external")).toBool();
        if (!external) {
            const QByteArray data = QByteArray::fromBase64(o.value(QStringLiteral("data")).toString().toLatin1());
            if (!m_pkg.partData(target).isEmpty()) {
                const int dot = target.lastIndexOf(QLatin1Char('.'));
                for (int n = 2; !m_pkg.partData(target).isEmpty() || n == 2; ++n) {
                    target = (dot > 0 ? target.left(dot) : target) + QStringLiteral("-h%1").arg(n) + (dot > 0 ? o.value(QStringLiteral("target")).toString().mid(dot) : QString());
                    if (m_pkg.partData(target).isEmpty()) break;
                }
            }
            m_pkg.addPart(target, data, o.value(QStringLiteral("contentType")).toString());
        }
        m_pkg.addRelationshipWithId(kHandoutMasterPart, o.value(QStringLiteral("id")).toString(), o.value(QStringLiteral("type")).toString(), target, external);
        hasTheme |= o.value(QStringLiteral("type")).toString().endsWith(QLatin1String("/theme"));
    }
    if (!hasTheme) {
        m_pkg.addRelationship(kHandoutMasterPart, kRelBase + QStringLiteral("theme"), m_handoutThemePart);
        m_pkg.addPart(m_handoutThemePart, withStandardFontNames(m_themeXml), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    }
    Q_UNUSED(keptXml)
    const PrintHeaderFooter hf = printHeaderFooter(m_doc, PrintedPages::Handouts);
    if (const QByteArray kept = keptHandoutMaster(m_doc); !kept.isEmpty()) {
        QString xml = handoutMasterEdited(m_doc) ? editedHandoutMaster(kept, handoutMasterRead(m_doc), hf, m_doc.appData.contains(QStringLiteral("handoutMasterRead"))) : QString::fromUtf8(kept);
        // (the places and the background, only where the person changed them)
        if (hasHandoutMaster(m_doc)) {
            const HandoutMaster now = handoutMasterOf(m_doc, m_notesSize);
            HandoutMaster was = handoutMasterFromJson(m_doc.appData.value(QStringLiteral("handoutMasterReadLayout")).toObject(), defaultHandoutMaster(m_notesSize));
            was.perPage = now.perPage;   // (how many slides a page holds is in the print settings, not in the master)
            if (now != was) xml = editedHandoutLayout(xml.toUtf8(), now, was, m_notesSize, defaultHandoutMaster(m_notesSize));
        }
        // (the part is written with a declaration of its own)
        if (xml.startsWith(QLatin1String("<?xml"))) xml = xml.mid(xml.indexOf(QLatin1String("?>")) + 2).trimmed();
        add(kHandoutMasterPart, kTypeBase + QStringLiteral("handoutMaster+xml"), xml);
        return;
    }
    const bool own = hasHandoutMaster(m_doc);
    const HandoutMaster layout = handoutMasterOf(m_doc, m_notesSize);
    auto typed = [](const QString &text) {
        return text.isEmpty() ? QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>") : QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(esc(text));
    };
    const QString number = QStringLiteral("<a:p><a:fld id=\"{9A1B2C3D-0000-4000-8000-000000000003}\" type=\"slidenum\"><a:rPr lang=\"en-US\"/><a:t>\u2039#\u203A</a:t></a:fld><a:endParaRPr lang=\"en-US\"/></a:p>");
    const QString date = QStringLiteral("<a:p><a:fld id=\"{9A1B2C3D-0000-4000-8000-000000000004}\" type=\"datetimeFigureOut\"><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:fld><a:endParaRPr lang=\"en-US\"/></a:p>")
                             .arg(esc(m_doc.props.modified.date().toString(Qt::ISODate)));
    auto place = [&](int id, const QString &name, const QString &ph, const QRectF &box, const QString &align, const QString &anchor, const QString &text) {
        return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph %3/></p:nvPr></p:nvSpPr><p:spPr><a:xfrm><a:off x=\"%4\" y=\"%5\"/>"
                              "<a:ext cx=\"%6\" cy=\"%7\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr><p:txBody><a:bodyPr vert=\"horz\" lIns=\"91440\" tIns=\"45720\" rIns=\"91440\" bIns=\"45720\" rtlCol=\"0\"%8/>"
                              "<a:lstStyle><a:lvl1pPr algn=\"%9\"><a:defRPr sz=\"1200\"/></a:lvl1pPr></a:lstStyle>%10</p:txBody></p:sp>")
            .arg(id).arg(name, ph).arg(office::pointsToEmu(box.x())).arg(office::pointsToEmu(box.y())).arg(office::pointsToEmu(box.width())).arg(office::pointsToEmu(box.height())).arg(anchor.isEmpty() ? QString() : QStringLiteral(" anchor=\"%1\"").arg(anchor), align, text);
    };
    QString xml = QStringLiteral("<p:handoutMaster%1><p:cSld>").arg(kNamespaces) + backgroundXml(own ? layout.background : QColor()) + QStringLiteral("<p:spTree>"
                                 "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                                 "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>");
    // (the standard's places, or the layout the presentation has)
    const QSizeF notes = m_notesSize;
    const double sw = notes.width() / kNotesWidth * 12700, sh = notes.height() / kNotesHeight * 12700;
    auto standard = [&](qint64 x, qint64 y) { return QRectF(x * sw / 12700 * 1.0, y * sh / 12700 * 1.0, 2971800 * sw / 12700, 458788 * sh / 12700); };
    auto rect = [&](const QRectF &fraction, const QRectF &standardPlace) { return own ? QRectF(fraction.x() * notes.width(), fraction.y() * notes.height(), fraction.width() * notes.width(), fraction.height() * notes.height()) : standardPlace; };
    xml += place(2, QStringLiteral("Header Placeholder 1"), QStringLiteral("type=\"hdr\" sz=\"quarter\""), rect(layout.header, standard(0, 0)), QStringLiteral("l"), QString(), typed(hf.headerText));
    xml += place(3, QStringLiteral("Date Placeholder 2"), QStringLiteral("type=\"dt\" sz=\"quarter\" idx=\"1\""), rect(layout.date, standard(3884613, 0)), QStringLiteral("r"), QString(), hf.dateText.isEmpty() ? date : typed(hf.dateText));
    xml += place(4, QStringLiteral("Footer Placeholder 3"), QStringLiteral("type=\"ftr\" sz=\"quarter\" idx=\"2\""), rect(layout.footer, standard(0, 8685213)), QStringLiteral("l"), QStringLiteral("b"), typed(hf.footerText));
    xml += place(5, QStringLiteral("Slide Number Placeholder 4"), QStringLiteral("type=\"sldNum\" sz=\"quarter\" idx=\"3\""), rect(layout.number, standard(3884613, 8685213)), QStringLiteral("r"), QStringLiteral("b"), number);
    xml += QStringLiteral("</p:spTree></p:cSld><p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" "
                          "accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>");
    xml += hfElement(hf);
    if (own) xml += handoutExtensionXml(layout, defaultHandoutMaster(notes));
    xml += QStringLiteral("</p:handoutMaster>");
    add(kHandoutMasterPart, kTypeBase + QStringLiteral("handoutMaster+xml"), xml);
}

void Exporter::writeProperties()
{
    const QString presProps = QStringLiteral("/ppt/presProps.xml"), viewProps = QStringLiteral("/ppt/viewProps.xml"), tableStyles = QStringLiteral("/ppt/tableStyles.xml");
    add(presProps, kTypeBase + QStringLiteral("presProps+xml"), deck::presentationPropertiesXml(m_doc, kNamespaces));
    add(viewProps, kTypeBase + QStringLiteral("viewProps+xml"), deck::viewPropertiesXml(m_doc, kNamespaces));
    // The styles the file defined are written back as they were (a table that names one is drawn and edited with them).
    add(tableStyles, kTypeBase + QStringLiteral("tableStyles+xml"), deck::tableStylesPartXml(m_doc));

    const DocProps &props = m_doc.props;
    auto element = [](const char *name, const QString &text) { return text.isEmpty() ? QString() : QStringLiteral("<%1>%2</%1>").arg(QLatin1String(name), esc(text)); };
    auto date = [](const char *name, const QDateTime &t) {
        return QStringLiteral("<%1 xsi:type=\"dcterms:W3CDTF\">%2</%1>").arg(QLatin1String(name), t.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'")));
    };
    QString core = QStringLiteral("<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                                  "xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">");
    core += element("dc:title", props.title) + element("dc:subject", props.subject) + element("dc:creator", props.author) + element("cp:keywords", props.keywords)
            + element("dc:description", props.comments) + element("cp:lastModifiedBy", props.lastModifiedBy.isEmpty() ? props.author : props.lastModifiedBy) + QStringLiteral("<cp:revision>1</cp:revision>")
            + date("dcterms:created", props.created.isValid() ? props.created : QDateTime::currentDateTime())
            + date("dcterms:modified", props.modified.isValid() ? props.modified : QDateTime::currentDateTime()) + element("cp:category", props.category)
            + (props.printed.isValid() ? date("cp:lastPrinted", props.printed) : QString()) + QStringLiteral("</cp:coreProperties>");
    add(QStringLiteral("/docProps/core.xml"), QStringLiteral("application/vnd.openxmlformats-package.core-properties+xml"), core);

    // The program's own properties: what the presentation holds, counted and listed.
    QStringList fonts, themes, titles;
    for (const MasterOut &m : m_masters) {
        for (const QString &f : {interchangeFontName(m.theme.minorLatin), interchangeFontName(m.theme.majorLatin)})
            if (!f.isEmpty() && !fonts.contains(f)) fonts << f;
        themes << (m.theme.name.isEmpty() ? QStringLiteral("Theme") : m.theme.name);
    }
    int hidden = 0, notes = 0;
    for (int i = 0; i < int(m_doc.pages.size()); ++i) {
        titles << titleOf(*m_doc.pages[i], i + 1);
        hidden += m_doc.pages[i]->hidden;
        notes += m_notesNumber[i] > 0;
    }
    const QString names = m_doc.setup.sizeName == QLatin1String("Standard") ? QStringLiteral("On-screen Show (4:3)") : m_doc.setup.sizeName;
    auto pair = [](const QString &label, int count) { return QStringLiteral("<vt:variant><vt:lpstr>%1</vt:lpstr></vt:variant><vt:variant><vt:i4>%2</vt:i4></vt:variant>").arg(esc(label)).arg(count); };
    QString heading, parts;
    int vectors = 0;
    auto group = [&](const QString &label, const QStringList &list) {
        if (list.isEmpty()) return;
        heading += pair(label, int(list.size()));
        for (const QString &s : list) parts += QStringLiteral("<vt:lpstr>%1</vt:lpstr>").arg(esc(s));
        vectors += int(list.size());
    };
    group(QStringLiteral("Fonts Used"), fonts);
    group(QStringLiteral("Theme"), themes);
    group(QStringLiteral("Slide Titles"), titles);
    int groups = int(!fonts.isEmpty()) + int(!themes.isEmpty()) + int(!titles.isEmpty());
    QString app = QStringLiteral("<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\" xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\">");
    app += QStringLiteral("<TotalTime>0</TotalTime><Application>JeffOffice</Application><PresentationFormat>%1</PresentationFormat>").arg(esc(names));
    app += QStringLiteral("<Slides>%1</Slides><Notes>%2</Notes><HiddenSlides>%3</HiddenSlides><MMClips>0</MMClips><ScaleCrop>false</ScaleCrop>").arg(m_doc.pages.size()).arg(notes).arg(hidden);
    app += QStringLiteral("<HeadingPairs><vt:vector size=\"%1\" baseType=\"variant\">%2</vt:vector></HeadingPairs><TitlesOfParts><vt:vector size=\"%3\" baseType=\"lpstr\">%4</vt:vector></TitlesOfParts>")
               .arg(groups * 2).arg(heading).arg(vectors).arg(parts);
    app += element("Manager", props.manager) + element("Company", props.company);
    app += QStringLiteral("<LinksUpToDate>false</LinksUpToDate><SharedDoc>false</SharedDoc><HyperlinksChanged>false</HyperlinksChanged></Properties>");
    add(QStringLiteral("/docProps/app.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.extended-properties+xml"), app);

    // The properties of a person's own (the mark of a file that is final is one).
    if (!props.custom.isEmpty()) m_pkg.addPart(QStringLiteral("/docProps/custom.xml"), office::writeCustomProps(QByteArray(), props.toPackageProps()), QLatin1String(office::kCustomPartType));
    if (!m_thumbnail.isEmpty()) m_pkg.addPart(QStringLiteral("/docProps/thumbnail.jpeg"), m_thumbnail, QStringLiteral("image/jpeg"));
}

// The sections, as the extension that lists each one's slides by their numbers in the slide list; nothing for a presentation without sections.
QString sectionListXml(const Document &doc)
{
    const QVector<Section> sections = sectionsOf(doc);
    if (sections.isEmpty()) return QString();
    static const QRegularExpression guid(QStringLiteral("^\\{[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\\}$"));
    QString xml = QStringLiteral("<p:extLst><p:ext uri=\"{521415D9-36F7-43E2-AB2F-B90AF26B5E84}\"><p14:sectionLst xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\">");
    int slide = 0;
    for (const Section &s : sections) {
        xml += QStringLiteral("<p14:section name=\"%1\" id=\"%2\">").arg(esc(s.name), guid.match(s.id).hasMatch() ? s.id : newSectionId());
        if (s.count == 0) xml += QStringLiteral("<p14:sldIdLst/>");
        else {
            xml += QStringLiteral("<p14:sldIdLst>");
            for (int i = 0; i < s.count; ++i) xml += QStringLiteral("<p14:sldId id=\"%1\"/>").arg(kFirstSlideId + slide++);
            xml += QStringLiteral("</p14:sldIdLst>");
        }
        xml += QStringLiteral("</p14:section>");
    }
    return xml + QStringLiteral("</p14:sectionLst></p:ext></p:extLst>");
}

void Exporter::writePresentation()
{
    const QSizeF size = m_doc.pageSize();
    const FooterOptions footers = footerOptions(m_doc);
    QString xml = QStringLiteral("<p:presentation%1").arg(kNamespaces);
    if (footers.skipTitleSlide) xml += QStringLiteral(" showSpecialPlsOnTitleSld=\"0\"");
    if (m_doc.setup.firstPageNumber != 1) xml += QStringLiteral(" firstSlideNum=\"%1\"").arg(std::clamp(m_doc.setup.firstPageNumber, 0, 9999));
    // Without this the presentation program does not look at the fonts the file carries.
    if (!m_fonts.listXml.isEmpty()) xml += QStringLiteral(" embedTrueTypeFonts=\"1\"");
    xml += QStringLiteral("><p:sldMasterIdLst>");
    for (const MasterOut &m : m_masters) xml += QStringLiteral("<p:sldMasterId id=\"%1\" r:id=\"%2\"/>").arg(m.id).arg(m.rel);
    xml += QStringLiteral("</p:sldMasterIdLst><p:notesMasterIdLst><p:notesMasterId r:id=\"%1\"/></p:notesMasterIdLst>").arg(m_notesMasterRel);
    if (!m_handoutMasterRel.isEmpty()) xml += QStringLiteral("<p:handoutMasterIdLst><p:handoutMasterId r:id=\"%1\"/></p:handoutMasterIdLst>").arg(m_handoutMasterRel);
    if (!m_slideRel.isEmpty()) {
        xml += QStringLiteral("<p:sldIdLst>");
        for (int i = 0; i < m_slideRel.size(); ++i) xml += QStringLiteral("<p:sldId id=\"%1\" r:id=\"%2\"/>").arg(kFirstSlideId + i).arg(m_slideRel[i]);
        xml += QStringLiteral("</p:sldIdLst>");
    }
    const QString type = sizeType(m_doc.setup.sizeName);
    xml += QStringLiteral("<p:sldSz cx=\"%1\" cy=\"%2\"%3/>").arg(emu(size.width()), emu(size.height()), type.isEmpty() ? QString() : QStringLiteral(" type=\"%1\"").arg(type));
    xml += QStringLiteral("<p:notesSz cx=\"%1\" cy=\"%2\"/>").arg(emu(m_notesSize.width()), emu(m_notesSize.height()));
    xml += m_fonts.listXml;
    xml += deck::customShowListXml(m_doc, m_slideRel);
    xml += QStringLiteral("<p:defaultTextStyle><a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr>");
    for (int i = 0; i < 9; ++i)
        xml += QStringLiteral("<a:lvl%1pPr marL=\"%2\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\"><a:defRPr sz=\"1800\" kern=\"1200\">"
                              "<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl%1pPr>")
                   .arg(i + 1).arg(457200 * i);
    xml += QStringLiteral("</p:defaultTextStyle>") + sectionListXml(m_doc) + QStringLiteral("</p:presentation>");
    add(kPresentationPart, office::macroMainType(office::MacroProgram::Presentation, m_kind == PresentationKind::Template ? office::MacroKind::Template : m_kind == PresentationKind::Show ? office::MacroKind::Show : office::MacroKind::Plain, m_macros), xml);
}

// The comments are written in the older form, the one every version of the
// program opens (ECMA-376 Part 1, 19.4): the authors in one part, which the
// presentation relates to (".../commentAuthors"), and a part of comments for
// each slide that has some (".../comments"). A comment has the author's id and
// an index that counts up for that author across the presentation, then its
// place (p:pos, in 576ths of an inch), then its text. A reply is a comment of
// its own, in the place of the comment it answers, and says which that is in
// the threading extension (p15:parentCm). The time is the author's own, with
// the offset from UTC in minutes the extension gives (timeZoneBias: UTC is the
// time plus the bias). The standard has no mark for a comment that is done, so
// it goes in an extension of JeffDeck's own, which other programs pass over.
void Exporter::planComments()
{
    m_slideComments.resize(m_doc.pages.size());
    for (int i = 0; i < int(m_doc.pages.size()); ++i) {
        const QVector<comments::Comment> list = comments::commentsOf(*m_doc.pages[i]);
        auto author = [&](const QString &name, const QString &initials) {
            const QPair<QString, QString> who(name, initials);
            int id = int(m_commentAuthors.indexOf(who));
            if (id < 0) {
                id = int(m_commentAuthors.size());
                m_commentAuthors << who;
                m_commentLastIdx << 0;
            }
            return id;
        };
        for (const comments::Comment &c : list) {
            CommentOut top;
            top.author = author(c.author, c.initials);
            top.idx = ++m_commentLastIdx[top.author];
            top.text = c.text;
            top.date = c.date;
            top.at = c.at;
            top.done = c.done;
            m_slideComments[i] << top;
            for (const comments::CommentReply &r : c.replies) {
                CommentOut reply;
                reply.author = author(r.author, r.initials);
                reply.idx = ++m_commentLastIdx[reply.author];
                reply.parentAuthor = top.author;
                reply.parentIdx = top.idx;
                reply.text = r.text;
                reply.date = r.date;
                reply.at = c.at;
                m_slideComments[i] << reply;
            }
        }
    }
}

void Exporter::writeCommentAuthors()
{
    if (m_commentAuthors.isEmpty()) return;
    const QString part = QStringLiteral("/ppt/commentAuthors.xml");
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("commentAuthors"), part);
    QString xml = QStringLiteral("<p:cmAuthorLst%1>").arg(kNamespaces);
    for (int i = 0; i < int(m_commentAuthors.size()); ++i)
        xml += QStringLiteral("<p:cmAuthor id=\"%1\" name=\"%2\" initials=\"%3\" lastIdx=\"%4\" clrIdx=\"%1\"/>")
                   .arg(i).arg(esc(m_commentAuthors[i].first), esc(m_commentAuthors[i].second)).arg(m_commentLastIdx[i]);
    xml += QStringLiteral("</p:cmAuthorLst>");
    add(part, kTypeBase + QStringLiteral("commentAuthors+xml"), xml);
}

void Exporter::writeComments(int slide, const QString &slidePart)
{
    if (slide >= m_slideComments.size() || m_slideComments[slide].isEmpty()) return;
    const QString part = QStringLiteral("/ppt/comments/comment%1.xml").arg(++m_commentParts);
    m_pkg.addRelationship(slidePart, kRelBase + QStringLiteral("comments"), part);
    QString xml = QStringLiteral("<p:cmLst%1>").arg(kNamespaces);
    for (const CommentOut &c : std::as_const(m_slideComments[slide])) {
        xml += QStringLiteral("<p:cm authorId=\"%1\"").arg(c.author);
        int bias = 0;
        if (c.date.isValid()) {
            const QDateTime own = c.date.toLocalTime();
            bias = -own.offsetFromUtc() / 60;
            xml += QStringLiteral(" dt=\"%1\"").arg(own.toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss.zzz")));
        }
        xml += QStringLiteral(" idx=\"%1\"><p:pos x=\"%2\" y=\"%3\"/><p:text>%4</p:text>").arg(c.idx).arg(std::lround(c.at.x() * 8)).arg(std::lround(c.at.y() * 8)).arg(esc(c.text));
        xml += QStringLiteral("<p:extLst><p:ext uri=\"{C676402C-5697-4E1C-873F-D02D1690AC5C}\"><p15:threadingInfo xmlns:p15=\"http://schemas.microsoft.com/office/powerpoint/2012/main\" timeZoneBias=\"%1\"")
                   .arg(bias);
        if (c.parentAuthor >= 0) xml += QStringLiteral("><p15:parentCm authorId=\"%1\" idx=\"%2\"/></p15:threadingInfo>").arg(c.parentAuthor).arg(c.parentIdx);
        else xml += QStringLiteral("/>");
        xml += QStringLiteral("</p:ext>");
        if (c.done) xml += QStringLiteral("<p:ext uri=\"{7C0E3E5B-5A0B-4B52-9D6E-2F1A8E4C9B21}\"><jo:done xmlns:jo=\"http://jeffoffice.com/ns/deck/comments\" val=\"1\"/></p:ext>");
        xml += QStringLiteral("</p:extLst></p:cm>");
    }
    xml += QStringLiteral("</p:cmLst>");
    add(part, kTypeBase + QStringLiteral("comments+xml"), xml);
}

QByteArray Exporter::run(QString *error)
{
    plan();
    planComments();
    prepareThemes();
    if (!m_thumbnailGiven) m_thumbnail = thumbnail();

    // The presentation part leads the file; its text is written last, when the links it names are known.
    m_pkg.addPart(kPresentationPart, QByteArray(), kTypeBase + QStringLiteral("presentation.main+xml"));
    m_pkg.addDefault(QStringLiteral("jpeg"), QStringLiteral("image/jpeg"));
    for (const auto &image : m_doc.images) {
        const QString format = image.format.toLower();
        if (format == QLatin1String("png")) m_pkg.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        else if (format == QLatin1String("jpg") || format == QLatin1String("jpeg")) m_pkg.addDefault(QStringLiteral("jpg"), QStringLiteral("image/jpeg"));
        else if (format == QLatin1String("gif")) m_pkg.addDefault(QStringLiteral("gif"), QStringLiteral("image/gif"));
        else if (format == QLatin1String("bmp")) m_pkg.addDefault(QStringLiteral("bmp"), QStringLiteral("image/bmp"));
        else if (format == QLatin1String("tif") || format == QLatin1String("tiff")) m_pkg.addDefault(QStringLiteral("tiff"), QStringLiteral("image/tiff"));
        else if (format == QLatin1String("emf")) m_pkg.addDefault(QStringLiteral("emf"), QStringLiteral("image/x-emf"));
        else if (format == QLatin1String("wmf")) m_pkg.addDefault(QStringLiteral("wmf"), QStringLiteral("image/x-wmf"));
        else if (format == QLatin1String("svg")) m_pkg.addDefault(QStringLiteral("svg"), QStringLiteral("image/svg+xml"));
    }
    m_pkg.addRelationship(QStringLiteral("/"), kRelBase + QStringLiteral("officeDocument"), kPresentationPart);
    m_pkg.addRelationship(QStringLiteral("/"), kPackageRelBase + QStringLiteral("metadata/core-properties"), QStringLiteral("/docProps/core.xml"));
    m_pkg.addRelationship(QStringLiteral("/"), kRelBase + QStringLiteral("extended-properties"), QStringLiteral("/docProps/app.xml"));
    if (!m_doc.props.custom.isEmpty()) m_pkg.addRelationship(QStringLiteral("/"), QLatin1String(office::kCustomRelationship), QStringLiteral("/docProps/custom.xml"));
    if (!m_thumbnail.isEmpty()) m_pkg.addRelationship(QStringLiteral("/"), kPackageRelBase + QStringLiteral("metadata/thumbnail"), QStringLiteral("/docProps/thumbnail.jpeg"));
    // The changes Compare found that nobody has decided yet (a part other programs do not know and leave alone).
    if (const QByteArray review = reviewPartOf(m_doc); !review.isEmpty()) {
        m_pkg.addPart(reviewPartName(), review, QStringLiteral("application/json"));
        m_pkg.addRelationship(QStringLiteral("/"), QStringLiteral("http://jeffoffice.com/relationships/review"), reviewPartName());
    }

    for (MasterOut &m : m_masters) m.rel = m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("slideMaster"), m.part);
    for (int i = 0; i < int(m_doc.pages.size()); ++i)
        m_slideRel << m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("slide"), QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1));
    m_notesMasterRel = m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("notesMaster"), kNotesMasterPart);
    // (a handout master the file had is written back; one is made only when the file had none and the person chose a header and footer)
    if (hasPrintHeaderFooter(m_doc, PrintedPages::Handouts) || hasHandoutMaster(m_doc) || !keptHandoutMaster(m_doc).isEmpty()) m_handoutMasterRel = m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("handoutMaster"), kHandoutMasterPart);
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("presProps"), QStringLiteral("/ppt/presProps.xml"));
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("viewProps"), QStringLiteral("/ppt/viewProps.xml"));
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("theme"), m_masters.front().themePart);
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("tableStyles"), QStringLiteral("/ppt/tableStyles.xml"));
    writeCommentAuthors();

    if (m_masters.size() + m_layouts.size() + 1 > size_t(kLastMasterId - kFirstMasterId)) {
        if (error) *error = QCoreApplication::translate("PptxExport", "The presentation has more slide masters and layouts than a file can number.");
        return QByteArray();
    }
    for (int i = 0; i < int(m_masters.size()); ++i) writeMaster(i);
    for (int i = 0; i < int(m_layouts.size()); ++i) writeLayout(i);
    for (int i = 0; i < int(m_doc.pages.size()); ++i) {
        writeSlide(i);
        if (m_notesNumber[i]) writeNotesSlide(i, m_notesNumber[i]);
    }
    writeNotesMaster();
    writeHandoutMaster();
    writeProperties();
    if (m_embedFonts || !m_keptFonts.isEmpty()) {
        m_fonts = writeEmbeddedFonts(m_pkg, kPresentationPart, m_fontUse, m_keptFonts, m_embedFonts);
    }
    writePresentation();
    // The macro project is kept only in a macro-enabled kind, as it was and never run; in any other kind it is left out.
    if (m_macros) office::writeMacroProject(&m_pkg, kPresentationPart, presentationMacros(m_doc), QStringLiteral("/ppt/"));
    if (m_droppedLinks > 0)
        note(QCoreApplication::translate("PptxExport", "Links to a slide that is no longer in the presentation were left out (%1).").arg(m_droppedLinks));

    const QByteArray bytes = m_pkg.finish();
    if (bytes.isEmpty()) {
        if (error) *error = QCoreApplication::translate("PptxExport", "The presentation file could not be put together.");
        return QByteArray();
    }
    if (m_callerSkipped) *m_callerSkipped += m_skipped;
    return bytes;
}

} // namespace

PresentationKind kindOfFileName(const QString &path)
{
    if (path.endsWith(QLatin1String(".potx"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".pot"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".potm"), Qt::CaseInsensitive)) return PresentationKind::Template;
    if (path.endsWith(QLatin1String(".ppsx"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".pps"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".ppsm"), Qt::CaseInsensitive)) return PresentationKind::Show;
    return PresentationKind::Presentation;
}

bool isMacroEnabledFileName(const QString &path)
{
    for (const char *ending : {".pptm", ".ppsm", ".potm"})
        if (path.endsWith(QLatin1String(ending), Qt::CaseInsensitive)) return true;
    return false;
}

office::MacroProject presentationMacros(const Document &doc) { return office::macroProjectFromJson(doc.appData.value(QStringLiteral("macros")).toObject()); }

// Text that shrinks on overflow is saved at the fit it has now (fontScale and lnSpcReduction), so that the file opens the same in other programs;
// text a file shrank keeps what the file said until it is edited.
static bool shrinksLive(const Item &it)
{
    if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).shrinks && !static_cast<const ShapeItem &>(it).fitAsStored;
    return it.type() == ItemType::Text && static_cast<const TextItem &>(it).autofit == TextItem::ShrinkOnOverflow && !static_cast<const TextItem &>(it).fitAsStored;
}

static void settleFits(Document &doc)
{
    LayoutCache cache;
    for (int i = 0; i < doc.pages.size(); ++i)
        for (const auto &it : doc.pages[i]->items)
            if (shrinksLive(*it) && it->type() == ItemType::Shape) {
                auto &sh = static_cast<ShapeItem &>(*it);
                PaintContext ctx;
                ctx.doc = &doc;
                ctx.cache = &cache;
                ctx.pageNumber = i + 1;
                ctx.pageCount = int(doc.pages.size());
                Renderer::shapeTextLayout(ctx, sh, nullptr, &sh.fitScale, &sh.fitLineCut);
            } else if (shrinksLive(*it)) {
                auto &t = static_cast<TextItem &>(*it);
                const auto fl = cache.textFrame(doc, t, i + 1, RenderOptions());
                t.fitScale = fl.fitScale;
                t.fitLineCut = fl.fitCut;
            }
}

QByteArray exportPresentation(const Document &doc, QString *error, QStringList *skipped, PresentationKind kind, bool embedFonts, bool macros)
{
    bool live = false;
    for (const auto &page : doc.pages)
        for (const auto &it : page->items) live = live || shrinksLive(*it);
    if (live && doc.slideText) {
        const std::unique_ptr<Document> fitted = Document::fromSnapshot(doc.snapshot());
        settleFits(*fitted);
        return Exporter(*fitted, kind, skipped, embedFonts, macros).run(error);
    }
    return Exporter(doc, kind, skipped, embedFonts, macros).run(error);
}

ExportSnapshot snapshotForExport(const Document &doc)
{
    ExportSnapshot s;
    s.doc = doc.snapshot();
    s.thumbnail = renderThumbnail(doc);
    s.keptFonts = EmbeddedFonts::facesOf(&doc);
    return s;
}

QByteArray exportSnapshot(const ExportSnapshot &snapshot, QString *error, PresentationKind kind, bool embedFonts, bool macros)
{
    const std::unique_ptr<Document> doc = Document::fromSnapshot(snapshot.doc);
    if (doc->slideText) settleFits(*doc);
    return Exporter(*doc, kind, embedFonts, snapshot.keptFonts, snapshot.thumbnail, macros).run(error);
}

bool savePresentationFile(const Document &doc, const QString &path, QString *error, QStringList *skipped, bool embedFonts, const QString &password, bool removePersonal, QStringList *personalProblems)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    QByteArray bytes = exportPresentation(doc, error, skipped, kindOfFileName(path), embedFonts, isMacroEnabledFileName(path));
    if (bytes.isEmpty()) return false;
    if (removePersonal) bytes = office::removePersonalFromPackage(bytes, nullptr, personalProblems);
    if (!password.isEmpty()) {
        bytes = office::encryptPackage(bytes, password);
        if (bytes.isEmpty()) return fail(QCoreApplication::translate("PptxExport", "The password couldn't be put on the file."));
    }
    if (path.isEmpty()) return fail(QCoreApplication::translate("PptxExport", "There's no file name to save to."));
    // The old file stays whole until the new one is complete.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return fail(file.errorString());
    if (file.write(bytes) != bytes.size()) {
        const QString why = file.errorString();
        file.cancelWriting();
        return fail(why);
    }
    if (!file.commit()) return fail(file.errorString());
    return true;
}

namespace {

// The entry of the page's "charts" for an object, by the picture's id: its place in the list, or -1.
int objectEntryOf(const PageBase &page, const QString &itemId)
{
    const QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
    for (int i = 0; i < charts.size(); ++i) {
        const QJsonObject e = charts[i].toObject();
        if (e.value(QStringLiteral("item")).toString() == itemId && e.value(QStringLiteral("kind")).toString() == QLatin1String("object")) return i;
    }
    return -1;
}

ItemPtr itemOfPage(const PageBase &page, const QString &itemId)
{
    ItemPtr found;
    walkItems(page.items, [&](const ItemPtr &it) {
        if (!found && it->id == itemId) found = it;
    });
    return found;
}

// What the objects of every slide come to, taken in as a budget; `leave` is the picture whose object a new one takes the place of. False if they are over it already.
bool objectBudgetOf(const Document &doc, const QString &leave, office::ole::ObjectBudget *budget)
{
    for (const auto &page : doc.pages)
        for (const QJsonValue &v : page->props.value(QStringLiteral("charts")).toArray()) {
            const QJsonObject e = v.toObject();
            if (e.value(QStringLiteral("kind")).toString() != QLatin1String("object") || e.value(QStringLiteral("item")).toString() == leave) continue;
            if (!budget->admit(e.value(QStringLiteral("data")).toString().size() / 4 * 3)) return false;
        }
    return true;
}

} // namespace

ItemPtr addPageObject(Document &doc, PageBase &page, const office::ole::EmbeddedObject &object, const QRectF &rect)
{
    if (!object.isOurs || object.previewPng.isEmpty() || object.partBytes.isEmpty() || rect.isEmpty()) return nullptr;
    office::ole::ObjectBudget budget;
    if (!objectBudgetOf(doc, QString(), &budget) || !budget.admit(object.partBytes.size(), object.previewPng.size())) return nullptr;
    const QString image = doc.addImage(object.previewPng, QStringLiteral("png"));
    if (image.isEmpty()) return nullptr;
    auto pic = std::make_shared<PictureItem>();
    pic->rect = rect;
    pic->imgRect = QRectF(QPointF(0, 0), rect.size());
    pic->wrap.mode = Wrap::None;
    pic->imageId = image;
    page.items.push_back(pic);
    QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
    const QString type = object.kind() == office::ole::Kind::Sheet ? QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet") : QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    charts.append(QJsonObject{{"item", pic->id}, {"kind", "object"}, {"progId", object.progId}, {"ext", office::ole::extensionOf(object.kind())}, {"type", type},
                              {"relType", kRelBase + QStringLiteral("package")}, {"data", QString::fromLatin1(object.partBytes.toBase64())}});
    page.props[QStringLiteral("charts")] = charts;
    return pic;
}

std::optional<office::ole::EmbeddedObject> pageObjectOf(const Document &doc, const PageBase &page, const QString &itemId)
{
    const int at = objectEntryOf(page, itemId);
    const ItemPtr item = itemOfPage(page, itemId);
    if (at < 0 || !item || item->type() != ItemType::Picture) return std::nullopt;
    const QJsonObject e = page.props.value(QStringLiteral("charts")).toArray()[at].toObject();
    const auto image = doc.images.constFind(static_cast<const PictureItem &>(*item).imageId);
    office::ole::ObjectBudget budget;
    std::optional<office::ole::EmbeddedObject> object = office::ole::keepObject(e.value(QStringLiteral("progId")).toString(), QByteArray::fromBase64(e.value(QStringLiteral("data")).toString().toLatin1()),
                                                                                image != doc.images.cend() && image->format == QLatin1String("png") ? image->bytes : QByteArray(),
                                                                                std::llround(item->rect.width() * office::ole::kEmuPerPoint), std::llround(item->rect.height() * office::ole::kEmuPerPoint), budget,
                                                                                e.value(QStringLiteral("part")).toString());
    return object;
}

bool setPageObject(Document &doc, PageBase &page, const QString &itemId, const office::ole::EmbeddedObject &object)
{
    const int at = objectEntryOf(page, itemId);
    const ItemPtr item = itemOfPage(page, itemId);
    if (at < 0 || !item || item->type() != ItemType::Picture || !object.isOurs || object.previewPng.isEmpty() || object.partBytes.isEmpty()) return false;
    office::ole::ObjectBudget budget;
    if (!objectBudgetOf(doc, itemId, &budget) || !budget.admit(object.partBytes.size(), object.previewPng.size())) return false;
    const QString image = doc.addImage(object.previewPng, QStringLiteral("png"));
    if (image.isEmpty()) return false;
    static_cast<PictureItem &>(*item).imageId = image;
    QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
    QJsonObject e = charts[at].toObject();
    e[QStringLiteral("data")] = QString::fromLatin1(object.partBytes.toBase64());
    charts[at] = e;
    page.props[QStringLiteral("charts")] = charts;
    return true;
}

} // namespace jp::deck
