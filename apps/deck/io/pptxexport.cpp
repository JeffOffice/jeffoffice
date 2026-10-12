#include "io/pptxexport.h"

#include "core/comments.h"
#include "core/deck.h"
#include "core/sections.h"
#include "io/pptxtransition.h"
#include "office/chart.h"
#include "office/drawingml_table.h"
#include "office/drawingml_write.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QBuffer>
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
#include <deque>
#include <functional>
#include <memory>

namespace jp::deck {

namespace {

QString say(const char *text) { return QCoreApplication::translate("PptxExport", text); }

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

// The theme color a color is, when it is one of the theme's.
QString schemeName(const QColor &c, const office::Theme &theme)
{
    static const char *const names[office::Theme::ColorCount] = {"tx1", "bg1", "tx2", "bg2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
    for (int i = 0; i < office::Theme::ColorCount; ++i)
        if (theme.colors[i].isValid() && theme.colors[i].rgb() == c.rgb()) return QString::fromLatin1(names[i]);
    return QString();
}

QString solidFill(const QColor &c, const office::Theme &theme)
{
    const QString scheme = schemeName(c, theme);
    return scheme.isEmpty() ? QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex(c))
                            : QStringLiteral("<a:solidFill><a:schemeClr val=\"%1\"/></a:solidFill>").arg(scheme);
}

// What the text of a level looks like in a master's text style.
struct Level {
    double size = 0;               // points; 0 for none said
    QString font;                  // a typeface, or "+mj-lt" or "+mn-lt" for the theme's
    bool bold = false, italic = false;
    QColor color;
    QString align = QStringLiteral("l");
    double marL = -1, indent = 0;  // points; marL < 0 for none said
    double before = 0, after = 0;  // space above and below, points
    double lineHeight = 0;         // percent of a line (proportional), or points when `fixedHeight`; 0 for none said
    bool fixedHeight = false;
    bool bullet = false;
    QString bulletChar = QStringLiteral("\u2022");
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
QVector<Level> levelsOf(const QTextDocument *doc, bool heading, const office::Theme &theme)
{
    QVector<Level> levels(9);
    QVector<bool> found(9, false);
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
            const QString c = list->format().stringProperty(tp::BulletChar);
            if (!c.isEmpty()) l.bulletChar = c;
        }
    }
    // A level the placeholder has none of looks like the first, smaller the deeper it goes.
    static const double standard[9] = {28, 24, 20, 18, 18, 18, 18, 18, 18};
    for (int i = 1; i < 9; ++i) {
        if (found[i]) continue;
        levels[i] = levels[0];
        if (!heading && levels[0].size > 0) levels[i].size = levels[0].size * standard[i] / standard[0];
        levels[i].marL = -1;
    }
    return levels;
}

QString levelXml(int level, const Level &l, const office::Theme &theme, bool heading, bool body, double defaultSize)
{
    const double marL = l.marL >= 0 ? l.marL : body ? 18 + 36 * level : (heading ? 0 : 36 * level);
    const double indent = l.marL >= 0 || !body ? l.indent : -18;
    QString out = QStringLiteral("<a:lvl%1pPr marL=\"%2\" indent=\"%3\" algn=\"%4\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\">")
                      .arg(level + 1).arg(emu(std::clamp(marL, 0.0, 4000.0))).arg(emu(std::clamp(indent, -4000.0, 4000.0)), l.align);
    if (l.lineHeight > 0)
        out += l.fixedHeight ? QStringLiteral("<a:lnSpc><a:spcPts val=\"%1\"/></a:lnSpc>").arg(std::clamp(int(std::lround(l.lineHeight * 100)), 0, 158400))
                             : QStringLiteral("<a:lnSpc><a:spcPct val=\"%1\"/></a:lnSpc>").arg(std::clamp(int(std::lround(l.lineHeight * 1000)), 0, 13200000));
    out += QStringLiteral("<a:spcBef><a:spcPts val=\"%1\"/></a:spcBef>").arg(std::clamp(int(std::lround(l.before * 100)), 0, 158400));
    if (l.after > 0) out += QStringLiteral("<a:spcAft><a:spcPts val=\"%1\"/></a:spcAft>").arg(std::clamp(int(std::lround(l.after * 100)), 0, 158400));
    if (body && l.bullet)
        out += QStringLiteral("<a:buFont typeface=\"Arial\" panose=\"020B0604020202020204\" pitchFamily=\"34\" charset=\"0\"/><a:buChar char=\"%1\"/>").arg(esc(l.bulletChar));
    else
        out += QStringLiteral("<a:buNone/>");
    const double size = l.size > 0 ? l.size : defaultSize;
    out += QStringLiteral("<a:defRPr sz=\"%1\"%2%3 kern=\"1200\">").arg(std::clamp(int(std::lround(size * 100)), 100, 400000)).arg(l.bold ? QStringLiteral(" b=\"1\"") : QString(),
                                                                                                                           l.italic ? QStringLiteral(" i=\"1\"") : QString());
    out += l.color.isValid() ? solidFill(l.color, theme) : QStringLiteral("<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>");
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
    *out = theme;
    return office::writeTheme(theme);
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
QString withFrameNames(const QString &frame, const QString &name, const QString &descr)
{
    static const QRegularExpression tag(QStringLiteral("<(\\w+):cNvPr\\b([^>]*?)(/?>)"));
    const QRegularExpressionMatch m = tag.match(frame);
    if (!m.hasMatch()) return frame;
    static const QRegularExpression named(QStringLiteral("\\s(?:name|descr)=\"[^\"]*\""));
    QString attrs = m.captured(2);
    attrs.remove(named);
    attrs += QStringLiteral(" name=\"%1\"").arg(esc(name));
    if (!descr.isEmpty()) attrs += QStringLiteral(" descr=\"%1\"").arg(esc(descr));
    return frame.left(m.capturedStart()) + QStringLiteral("<%1:cNvPr%2%3").arg(m.captured(1), attrs, m.captured(3)) + frame.mid(m.capturedEnd());
}

class Exporter {
public:
    Exporter(const Document &doc, PresentationKind kind, QStringList *skipped) : m_doc(doc), m_kind(kind), m_callerSkipped(skipped) {}
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
    QString keptPartName(const QString &original);
    QString keptSlideXml(const Page &slide, const char *key, const QString &what, const QString &where, const QSet<qint64> &frameIds,
                         const QSet<qint64> &mediaIds);
    QString background(const PageBase &page, const QString &part, bool required, const QString &where);
    QString textStyles(const MasterOut &master);
    QString titleOf(const Page &slide, int number) const;
    QByteArray thumbnail() const;
    void note(const QString &text) { m_skipped << text; }

    void writeMaster(int i);
    void writeLayout(int i);
    void writeSlide(int i);
    void writeNotesSlide(int slide, int number);
    void writeNotesMaster();
    void writeProperties();
    void writePresentation();
    void planComments();
    void writeCommentAuthors();
    void writeComments(int slide, const QString &slidePart);
    void add(const QString &part, const QString &type, const QString &xml);

    const Document &m_doc;
    PresentationKind m_kind;
    QStringList *m_callerSkipped;
    QStringList m_skipped;
    office::PackageWriter m_pkg;
    QHash<QString, QString> m_media;
    std::deque<MasterPage> m_synthesized;
    std::vector<MasterOut> m_masters;
    std::vector<LayoutOut> m_layouts;
    QVector<int> m_slideLayout, m_notesNumber;   // per slide: its layout; the number of its notes page (0 for none)
    QVector<QString> m_slideRel;
    QString m_notesMasterRel;
    office::Theme m_theme;                       // the presentation's, for drawings
    QByteArray m_themeXml;
    QString m_notesThemePart;
    QSet<QString> m_keptNames;                   // the names of kept chart parts written so far, lower-cased
    QByteArray m_thumbnail;
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
}

office::DrawingWriteContext Exporter::context(const QString &part)
{
    office::DrawingWriteContext ctx;
    ctx.package = &m_pkg;
    ctx.part = part;
    ctx.theme = m_theme;
    ctx.imageBytes = [this](const QString &imageId, QString *format) {
        const auto it = m_doc.images.constFind(imageId);
        if (it == m_doc.images.cend()) return QByteArray();
        if (format) *format = it->format;
        return it->bytes;
    };
    ctx.storyDoc = [this](const QString &storyId) { return m_doc.storyDoc(storyId); };
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
        note(say("%1: a table, chart, or other frame that can't be read was left out.").arg(where));
        return QString();
    }
    if (refersToParts(dom.documentElement())) {
        note(say("%1: a chart, embedded object, or other frame that refers to parts of the file that aren't kept was left out.").arg(where));
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
                               const QSet<qint64> &mediaIds)
{
    const QString xml = slide.props.value(QLatin1String(key)).toString();
    if (xml.isEmpty()) return QString();
    QDomDocument dom;
    if (!parsesAlone(xml, &dom)) {
        note(say("%1: the %2 can't be read and was left out.").arg(where, what));
        return QString();
    }
    if (refersToParts(dom.documentElement())) {
        note(say("%1: the %2 uses a sound or other part of the file that isn't kept, and was left out.").arg(where, what));
        return QString();
    }
    static const QRegularExpression targets(QStringLiteral("\\bspid=\"(\\d+)\""));
    for (auto it = targets.globalMatch(xml); it.hasNext();) {
        // Shapes are numbered afresh in the file, but a frame keeps the number it had.
        if (!frameIds.contains(it.next().captured(1).toLongLong())) {
            note(say("%1: the %2 aims at shapes by their numbers in the old file, and was left out.").arg(where, what));
            return QString();
        }
    }
    if (!playsOnlyMedia(dom.documentElement(), mediaIds)) {
        note(say("%1: the %2 plays a movie or sound that wasn't kept, and was left out.").arg(where, what));
        return QString();
    }
    return xml;
}

// A part name for a kept chart part: the one it had when that is free and in a folder only charts write to, else a free one in the charts' folder.
QString Exporter::keptPartName(const QString &original)
{
    const QString lower = original.toLower();
    const bool own = lower.startsWith(QLatin1String("/ppt/charts/")) || lower.startsWith(QLatin1String("/ppt/embeddings/")) || lower.startsWith(QLatin1String("/ppt/drawings/"))
                     || lower.startsWith(QLatin1String("/ppt/theme/themeoverride"));
    const QString file = original.mid(original.lastIndexOf(QLatin1Char('/')) + 1);
    const QString name = own ? original : QStringLiteral("/ppt/charts/") + file;
    if (!m_keptNames.contains(name.toLower())) {
        m_keptNames.insert(name.toLower());
        return name;
    }
    // Another chart has it: the same name with the next number.
    const QString dir = name.left(name.lastIndexOf(QLatin1Char('/')) + 1);
    const int dot = int(file.lastIndexOf(QLatin1Char('.')));
    QString stem = dot < 0 ? file : file.left(dot);
    const QString extension = dot < 0 ? QString() : file.mid(dot);
    while (!stem.isEmpty() && stem.back().isDigit()) stem.chop(1);
    for (int n = 1;; ++n) {
        const QString candidate = dir + stem + QString::number(n) + extension;
        if (m_keptNames.contains(candidate.toLower())) continue;
        m_keptNames.insert(candidate.toLower());
        return candidate;
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
    bool sound = parts.contains(main) && parsesAlone(frame, &dom) && office::isChartFrame(dom.documentElement());
    for (auto it = parts.begin(); sound && it != parts.end(); ++it) {
        const QJsonObject part = it.value().toObject();
        for (const QJsonValue &link : part.value(QStringLiteral("rels")).toArray())
            if (!link.toObject().value(QStringLiteral("external")).toBool() && !parts.contains(link.toObject().value(QStringLiteral("target")).toString())) sound = false;
    }
    if (!sound) {
        note(say("%1: a chart's parts can't be written, so it was left out.").arg(where));
        return QString();
    }
    QHash<QString, QString> names;
    for (const QString &key : parts.keys()) names.insert(key, keptPartName(key));
    for (const QString &key : parts.keys()) {
        const QJsonObject part = parts.value(key).toObject();
        const QString name = names.value(key);
        QByteArray data = QByteArray::fromBase64(part.value(QStringLiteral("data")).toString().toLatin1());
        // Its links, in the order of their ids so that they keep them (rId1, rId2...); any that don't are changed in the part.
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
        const QString type = part.value(QStringLiteral("type")).toString();
        m_pkg.addPart(name, data, type.isEmpty() ? QStringLiteral("application/octet-stream") : type);
    }
    // The slide's link to the chart, which the frame names, and the frame where the picture is.
    const QString id = m_pkg.addRelationship(slidePart, kRelBase + QStringLiteral("chart"), names.value(main));
    static const QRegularExpression chartTag(QStringLiteral("(<(\\w+):chart\\b[^>]*?\\b\\w+:id=\")[^\"]*(\")"));
    frame.replace(chartTag, QStringLiteral("\\1") + id + QStringLiteral("\\3"));
    frame = withFramePlace(frame, item.rect);
    return withFrameNames(frame, item.name, item.altText);
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
    // The charts: each picture the reader made of one that is still on the page is written as the chart's frame, in its place among the shapes.
    QHash<QString, QJsonObject> charts;
    for (const QJsonValue &v : page.props.value(QStringLiteral("charts")).toArray()) charts.insert(v.toObject().value(QStringLiteral("item")).toString(), v.toObject());
    QHash<QString, QString> chartFrames;
    for (const ItemPtr &it : page.items) {
        if (it->type() != ItemType::Picture || !charts.contains(it->id)) continue;
        const QString xml = chartFrame(charts.value(it->id), *it, part, where);
        if (xml.isEmpty()) continue;
        chartFrames.insert(it->id, xml);
        take(xml);
    }
    office::DrawingWriteContext ctx = context(part);
    ctx.nextShapeId = int(std::min<qint64>(highest + 1, 2000000000));
    // The tree's own number and the frames' are taken; the shapes read from a file keep theirs, which new ones leave alone.
    ctx.usedIds.insert(1);
    for (const qint64 n : std::as_const(frameNumbers))
        if (n <= INT_MAX) ctx.usedIds.insert(int(n));
    walkItems(page.items, [&](const ItemPtr &it) {
        if (it->sourceId >= 2 && !chartFrames.contains(it->id)) ctx.reservedIds.insert(it->sourceId);
    });
    // A picture that stands for a chart is written as the chart's frame, with the frame's number.
    ItemList drawn;
    for (const ItemPtr &it : page.items)
        if (!chartFrames.contains(it->id)) drawn.push_back(it);
    office::assignShapeIds(drawn, ctx);
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
    if (page.background.isNone()) return themed;
    office::DrawingWriteContext ctx = context(part);
    QByteArray fill = fragment([&](QXmlStreamWriter &w) { office::writeFill(w, page.background, ctx); });
    if (fill.isEmpty() && page.background.type == Fill::Solid) {
        // The drawing writer has no form for it yet: a plain color is still written.
        const QColor c = page.background.color.kind() == ColorRef::Rgb ? page.background.color.rgbValue() : page.background.color.resolve(m_doc.colors);
        if (c.isValid()) fill = QStringLiteral("<a:solidFill><a:srgbClr val=\"%1\"/></a:solidFill>").arg(hex(c)).toUtf8();
    }
    if (fill.isEmpty()) {
        note(say("%1: the background can't be saved yet.").arg(where));
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
    QString out = QStringLiteral("<p:txStyles><p:titleStyle>") + levelXml(0, title[0], master.theme, true, false, 44) + QStringLiteral("</p:titleStyle><p:bodyStyle>");
    for (int i = 0; i < 9; ++i) out += levelXml(i, body[i], master.theme, false, true, bodySizes[i]);
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

// The first slide, 256 pixels across, as the file's picture of itself.
QByteArray Exporter::thumbnail() const
{
    const double width = m_doc.pageSize().width();
    if (m_doc.pages.isEmpty() || width <= 0) return QByteArray();
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &m_doc;
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

void Exporter::add(const QString &part, const QString &type, const QString &xml)
{
    m_pkg.addPart(part, (kDeclaration + xml).toUtf8(), type);
}

void Exporter::writeMaster(int i)
{
    MasterOut &master = m_masters[i];
    const MasterPage &page = *master.page;
    const QString where = say("Slide master %1").arg(i + 1);
    // Its links come first: the layouts it lists, and its theme.
    QString layoutIds;
    for (int l : std::as_const(master.layouts)) {
        const QString rel = m_pkg.addRelationship(master.part, kRelBase + QStringLiteral("slideLayout"), m_layouts[l].part);
        layoutIds += QStringLiteral("<p:sldLayoutId id=\"%1\" r:id=\"%2\"/>").arg(m_layouts[l].id).arg(rel);
    }
    m_pkg.addRelationship(master.part, kRelBase + QStringLiteral("theme"), master.themePart);
    m_pkg.addPart(master.themePart, master.themeXml, QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));

    QString xml = QStringLiteral("<p:sldMaster%1 preserve=\"1\"><p:cSld%2>").arg(kNamespaces, page.name.isEmpty() ? QString() : QStringLiteral(" name=\"%1\"").arg(esc(page.name)));
    const QString fill = background(page, master.part, true, where);
    xml += fill + tree(page, master.part, where, nullptr) + QStringLiteral("</p:cSld>");
    xml += QStringLiteral("<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" "
                          "accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>");
    xml += QStringLiteral("<p:sldLayoutIdLst>") + layoutIds + QStringLiteral("</p:sldLayoutIdLst>");
    xml += textStyles(master) + QStringLiteral("</p:sldMaster>");
    add(master.part, kTypeBase + QStringLiteral("slideMaster+xml"), xml);
}

void Exporter::writeLayout(int i)
{
    const LayoutOut &layout = m_layouts[i];
    const MasterPage &page = *layout.page;
    const QString where = say("Layout \"%1\"").arg(page.name);
    m_pkg.addRelationship(layout.part, kRelBase + QStringLiteral("slideMaster"), m_masters[layout.master].part);
    const QString type = page.props.value(QStringLiteral("type")).toString();
    const bool custom = !isLayoutType(type);
    QString xml = QStringLiteral("<p:sldLayout%1").arg(kNamespaces);
    if (!custom) xml += QStringLiteral(" type=\"%1\"").arg(type);
    if (!page.showMasterItems) xml += QStringLiteral(" showMasterSp=\"0\"");
    xml += QStringLiteral(" preserve=\"1\"");
    if (custom) xml += QStringLiteral(" userDrawn=\"1\"");
    xml += QStringLiteral("><p:cSld name=\"%1\">").arg(esc(page.name));
    const QString fill = background(page, layout.part, false, where);
    xml += fill + tree(page, layout.part, where, nullptr) + QStringLiteral("</p:cSld>");
    xml += QStringLiteral("<p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>");
    add(layout.part, kTypeBase + QStringLiteral("slideLayout+xml"), xml);
}

void Exporter::writeSlide(int i)
{
    const Page &page = *m_doc.pages[i];
    const QString part = QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1);
    const QString where = say("Slide %1").arg(i + 1);
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
            note(say("%1: the transition's sound isn't kept; the transition is saved without it.").arg(where));
        } else if (const QString kept = keptSlideXml(page, "transitionXml", say("transition"), where, frameIds, mediaIds); !kept.isEmpty()) {
            transitionOut = kept;
        }
    }
    xml += transitionOut;
    xml += keptSlideXml(page, "timingXml", say("animation"), where, frameIds, mediaIds);
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

void Exporter::writeNotesMaster()
{
    m_pkg.addRelationship(kNotesMasterPart, kRelBase + QStringLiteral("theme"), m_notesThemePart);
    m_pkg.addPart(m_notesThemePart, m_themeXml, QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    // The places on the notes page, in the standard's proportions for the page's size.
    const qint64 nw = office::pointsToEmu(m_notesSize.width()), nh = office::pointsToEmu(m_notesSize.height());
    auto x = [&](qint64 v) { return v * nw / kNotesWidth; };
    auto y = [&](qint64 v) { return v * nh / kNotesHeight; };
    const QSizeF slide = m_doc.pageSize();
    const qint64 imageW = x(5486400), imageH = slide.width() > 0 ? qint64(imageW * slide.height() / slide.width()) : y(3086100);
    auto place = [&](qint64 px, qint64 py, qint64 cx, qint64 cy) {
        return QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>").arg(px).arg(py).arg(cx).arg(cy);
    };
    auto small = [&](int id, const QString &name, const QString &ph, qint64 px, qint64 py, const QString &align, const QString &anchor, const QString &text) {
        return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph %3/></p:nvPr></p:nvSpPr><p:spPr>%4</p:spPr>"
                              "<p:txBody><a:bodyPr vert=\"horz\" lIns=\"91440\" tIns=\"45720\" rIns=\"91440\" bIns=\"45720\" rtlCol=\"0\"%5/><a:lstStyle><a:lvl1pPr algn=\"%6\"><a:defRPr sz=\"1200\"/></a:lvl1pPr></a:lstStyle>"
                              "%7</p:txBody></p:sp>")
            .arg(id).arg(name, ph, place(px, py, x(2971800), y(458788)), anchor.isEmpty() ? QString() : QStringLiteral(" anchor=\"%1\"").arg(anchor), align, text);
    };
    const QString empty = QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>");
    const QString number = QStringLiteral("<a:p><a:fld id=\"{9A1B2C3D-0000-4000-8000-000000000001}\" type=\"slidenum\"><a:rPr lang=\"en-US\"/><a:t>\u2039#\u203A</a:t></a:fld><a:endParaRPr lang=\"en-US\"/></a:p>");
    const QString date = QStringLiteral("<a:p><a:fld id=\"{9A1B2C3D-0000-4000-8000-000000000002}\" type=\"datetimeFigureOut\"><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:fld><a:endParaRPr lang=\"en-US\"/></a:p>")
                             .arg(esc(m_doc.props.modified.date().toString(Qt::ISODate)));
    static const char *const samples[5] = {"Click to edit Master text styles", "Second level", "Third level", "Fourth level", "Fifth level"};
    QString texts;
    for (int level = 0; level < 5; ++level)
        texts += QStringLiteral("<a:p><a:pPr lvl=\"%1\"/><a:r><a:rPr lang=\"en-US\"/><a:t>%2</a:t></a:r></a:p>").arg(level).arg(QLatin1String(samples[level]));

    QString xml = QStringLiteral("<p:notesMaster%1><p:cSld><p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg1\"/></p:bgRef></p:bg><p:spTree>"
                                 "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr>"
                                 "<p:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"0\" cy=\"0\"/></a:xfrm></p:grpSpPr>")
                      .arg(kNamespaces);
    xml += small(2, QStringLiteral("Header Placeholder 1"), QStringLiteral("type=\"hdr\" sz=\"quarter\""), 0, 0, QStringLiteral("l"), QString(), empty);
    xml += small(3, QStringLiteral("Date Placeholder 2"), QStringLiteral("type=\"dt\" idx=\"1\""), x(3884613), 0, QStringLiteral("r"), QString(), date);
    xml += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"4\" name=\"Slide Image Placeholder 3\"/><p:cNvSpPr><a:spLocks noGrp=\"1\" noRot=\"1\" noChangeAspect=\"1\"/></p:cNvSpPr>"
                          "<p:nvPr><p:ph type=\"sldImg\" idx=\"2\"/></p:nvPr></p:nvSpPr><p:spPr>%1<a:noFill/><a:ln w=\"12700\"><a:solidFill><a:prstClr val=\"black\"/></a:solidFill></a:ln></p:spPr></p:sp>")
               .arg(place(x(685800), y(1143000), imageW, imageH));
    xml += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"Notes Placeholder 4\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph type=\"body\" sz=\"quarter\" idx=\"3\"/></p:nvPr></p:nvSpPr>"
                          "<p:spPr>%1</p:spPr><p:txBody><a:bodyPr vert=\"horz\" lIns=\"91440\" tIns=\"45720\" rIns=\"91440\" bIns=\"45720\" rtlCol=\"0\"/><a:lstStyle/>%2</p:txBody></p:sp>")
               .arg(place(x(685800), y(4400550), x(5486400), y(3600450)), texts);
    xml += small(6, QStringLiteral("Footer Placeholder 5"), QStringLiteral("type=\"ftr\" sz=\"quarter\" idx=\"4\""), 0, y(8685213), QStringLiteral("l"), QStringLiteral("b"), empty);
    xml += small(7, QStringLiteral("Slide Number Placeholder 6"), QStringLiteral("type=\"sldNum\" sz=\"quarter\" idx=\"5\""), x(3884613), y(8685213), QStringLiteral("r"), QStringLiteral("b"), number);
    xml += QStringLiteral("</p:spTree></p:cSld><p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" "
                          "accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/><p:notesStyle>");
    for (int i = 0; i < 9; ++i)
        xml += QStringLiteral("<a:lvl%1pPr marL=\"%2\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\"><a:defRPr sz=\"1200\" kern=\"1200\">"
                              "<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl%1pPr>")
                   .arg(i + 1).arg(457200 * i);
    xml += QStringLiteral("</p:notesStyle></p:notesMaster>");
    add(kNotesMasterPart, kTypeBase + QStringLiteral("notesMaster+xml"), xml);
}

void Exporter::writeProperties()
{
    const QString presProps = QStringLiteral("/ppt/presProps.xml"), viewProps = QStringLiteral("/ppt/viewProps.xml"), tableStyles = QStringLiteral("/ppt/tableStyles.xml");
    add(presProps, kTypeBase + QStringLiteral("presProps+xml"), QStringLiteral("<p:presentationPr%1/>").arg(kNamespaces));
    add(viewProps, kTypeBase + QStringLiteral("viewProps+xml"),
        QStringLiteral("<p:viewPr%1><p:normalViewPr><p:restoredLeft sz=\"15620\"/><p:restoredTop sz=\"94660\"/></p:normalViewPr><p:slideViewPr><p:cSldViewPr><p:cViewPr varScale=\"1\">"
                       "<p:scale><a:sx n=\"100\" d=\"100\"/><a:sy n=\"100\" d=\"100\"/></p:scale><p:origin x=\"0\" y=\"0\"/></p:cViewPr><p:guideLst/></p:cSldViewPr></p:slideViewPr>"
                       "<p:notesTextViewPr><p:cViewPr><p:scale><a:sx n=\"1\" d=\"1\"/><a:sy n=\"1\" d=\"1\"/></p:scale><p:origin x=\"0\" y=\"0\"/></p:cViewPr></p:notesTextViewPr>"
                       "<p:gridSpacing cx=\"76200\" cy=\"76200\"/></p:viewPr>")
            .arg(kNamespaces));
    add(tableStyles, kTypeBase + QStringLiteral("tableStyles+xml"),
        QStringLiteral("<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" def=\"{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}\"/>"));

    const DocProps &props = m_doc.props;
    auto element = [](const char *name, const QString &text) { return text.isEmpty() ? QString() : QStringLiteral("<%1>%2</%1>").arg(QLatin1String(name), esc(text)); };
    auto date = [](const char *name, const QDateTime &t) {
        return QStringLiteral("<%1 xsi:type=\"dcterms:W3CDTF\">%2</%1>").arg(QLatin1String(name), t.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss'Z'")));
    };
    QString core = QStringLiteral("<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                                  "xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">");
    core += element("dc:title", props.title) + element("dc:subject", props.subject) + element("dc:creator", props.author) + element("cp:keywords", props.keywords)
            + element("dc:description", props.comments) + element("cp:lastModifiedBy", props.author) + QStringLiteral("<cp:revision>1</cp:revision>")
            + date("dcterms:created", props.created.isValid() ? props.created : QDateTime::currentDateTime())
            + date("dcterms:modified", props.modified.isValid() ? props.modified : QDateTime::currentDateTime()) + element("cp:category", props.category)
            + QStringLiteral("</cp:coreProperties>");
    add(QStringLiteral("/docProps/core.xml"), QStringLiteral("application/vnd.openxmlformats-package.core-properties+xml"), core);

    // The program's own properties: what the presentation holds, counted and listed.
    QStringList fonts, themes, titles;
    for (const MasterOut &m : m_masters) {
        for (const QString &f : {m.theme.minorLatin, m.theme.majorLatin})
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
    xml += QStringLiteral("><p:sldMasterIdLst>");
    for (const MasterOut &m : m_masters) xml += QStringLiteral("<p:sldMasterId id=\"%1\" r:id=\"%2\"/>").arg(m.id).arg(m.rel);
    xml += QStringLiteral("</p:sldMasterIdLst><p:notesMasterIdLst><p:notesMasterId r:id=\"%1\"/></p:notesMasterIdLst>").arg(m_notesMasterRel);
    if (!m_slideRel.isEmpty()) {
        xml += QStringLiteral("<p:sldIdLst>");
        for (int i = 0; i < m_slideRel.size(); ++i) xml += QStringLiteral("<p:sldId id=\"%1\" r:id=\"%2\"/>").arg(kFirstSlideId + i).arg(m_slideRel[i]);
        xml += QStringLiteral("</p:sldIdLst>");
    }
    const QString type = sizeType(m_doc.setup.sizeName);
    xml += QStringLiteral("<p:sldSz cx=\"%1\" cy=\"%2\"%3/>").arg(emu(size.width()), emu(size.height()), type.isEmpty() ? QString() : QStringLiteral(" type=\"%1\"").arg(type));
    xml += QStringLiteral("<p:notesSz cx=\"%1\" cy=\"%2\"/>").arg(emu(m_notesSize.width()), emu(m_notesSize.height()));
    xml += QStringLiteral("<p:defaultTextStyle><a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr>");
    for (int i = 0; i < 9; ++i)
        xml += QStringLiteral("<a:lvl%1pPr marL=\"%2\" algn=\"l\" defTabSz=\"914400\" rtl=\"0\" eaLnBrk=\"1\" latinLnBrk=\"0\" hangingPunct=\"1\"><a:defRPr sz=\"1800\" kern=\"1200\">"
                              "<a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill><a:latin typeface=\"+mn-lt\"/><a:ea typeface=\"+mn-ea\"/><a:cs typeface=\"+mn-cs\"/></a:defRPr></a:lvl%1pPr>")
                   .arg(i + 1).arg(457200 * i);
    xml += QStringLiteral("</p:defaultTextStyle>") + sectionListXml(m_doc) + QStringLiteral("</p:presentation>");
    const char *type_ = m_kind == PresentationKind::Template ? "template.main+xml" : m_kind == PresentationKind::Show ? "slideshow.main+xml" : "presentation.main+xml";
    add(kPresentationPart, kTypeBase + QLatin1String(type_), xml);
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
        const QVector<deck::Comment> list = deck::commentsOf(*m_doc.pages[i]);
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
        for (const deck::Comment &c : list) {
            CommentOut top;
            top.author = author(c.author, c.initials);
            top.idx = ++m_commentLastIdx[top.author];
            top.text = c.text;
            top.date = c.date;
            top.at = c.at;
            top.done = c.done;
            m_slideComments[i] << top;
            for (const deck::CommentReply &r : c.replies) {
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
    m_thumbnail = thumbnail();

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
    if (!m_thumbnail.isEmpty()) m_pkg.addRelationship(QStringLiteral("/"), kPackageRelBase + QStringLiteral("metadata/thumbnail"), QStringLiteral("/docProps/thumbnail.jpeg"));

    for (MasterOut &m : m_masters) m.rel = m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("slideMaster"), m.part);
    for (int i = 0; i < int(m_doc.pages.size()); ++i)
        m_slideRel << m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("slide"), QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1));
    m_notesMasterRel = m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("notesMaster"), kNotesMasterPart);
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("presProps"), QStringLiteral("/ppt/presProps.xml"));
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("viewProps"), QStringLiteral("/ppt/viewProps.xml"));
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("theme"), m_masters.front().themePart);
    m_pkg.addRelationship(kPresentationPart, kRelBase + QStringLiteral("tableStyles"), QStringLiteral("/ppt/tableStyles.xml"));
    writeCommentAuthors();

    if (m_masters.size() + m_layouts.size() + 1 > size_t(kLastMasterId - kFirstMasterId)) {
        if (error) *error = say("The presentation has more slide masters and layouts than a file can number.");
        return QByteArray();
    }
    for (int i = 0; i < int(m_masters.size()); ++i) writeMaster(i);
    for (int i = 0; i < int(m_layouts.size()); ++i) writeLayout(i);
    for (int i = 0; i < int(m_doc.pages.size()); ++i) {
        writeSlide(i);
        if (m_notesNumber[i]) writeNotesSlide(i, m_notesNumber[i]);
    }
    writeNotesMaster();
    writeProperties();
    writePresentation();

    const QByteArray bytes = m_pkg.finish();
    if (bytes.isEmpty()) {
        if (error) *error = say("The presentation file could not be put together.");
        return QByteArray();
    }
    if (m_callerSkipped) *m_callerSkipped += m_skipped;
    return bytes;
}

} // namespace

PresentationKind kindOfFileName(const QString &path)
{
    if (path.endsWith(QLatin1String(".potx"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".pot"), Qt::CaseInsensitive)) return PresentationKind::Template;
    if (path.endsWith(QLatin1String(".ppsx"), Qt::CaseInsensitive) || path.endsWith(QLatin1String(".pps"), Qt::CaseInsensitive)) return PresentationKind::Show;
    return PresentationKind::Presentation;
}

QByteArray exportPresentation(const Document &doc, QString *error, QStringList *skipped, PresentationKind kind)
{
    return Exporter(doc, kind, skipped).run(error);
}

bool savePresentationFile(const Document &doc, const QString &path, QString *error, QStringList *skipped)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    const QByteArray bytes = exportPresentation(doc, error, skipped, kindOfFileName(path));
    if (bytes.isEmpty()) return false;
    if (path.isEmpty()) return fail(say("There's no file name to save to."));
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

} // namespace jp::deck
