// OpenDocument text to .docx: the package, the styles, and what they say (see odt.h).

#include "odt/odtread.h"

#include "io/zipindex.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

namespace jp::office::odt {

const char kXmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
const char kRelBase[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";
const char kWml[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.";

const char *docxNamespaces()
{
    return "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
           "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
           "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" "
           "xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
           "xmlns:w10=\"urn:schemas-microsoft-com:office:word\"";
}

bool looksLikeFlatOdt(const QByteArray &head)
{
    QByteArray h = head.left(16384);
    if (h.startsWith("\xEF\xBB\xBF")) h.remove(0, 3);
    const qsizetype root = h.indexOf("<office:document");
    if (root < 0 || h.indexOf("<office:document-content") == root || h.indexOf("<office:document-styles") == root) return false;
    return h.indexOf("opendocument.text", root) >= 0;
}

bool looksLikeOdt(const QByteArray &bytes)
{
    if (bytes.startsWith("PK\x03\x04")) {
        // The usual file: the mimetype is the first entry, stored, so that it is at a fixed place.
        if (bytes.size() > 38 && bytes.mid(30, 8) == "mimetype") {
            const quint16 method = quint8(bytes[8]) | (quint16(quint8(bytes[9])) << 8);
            const qsizetype nameLen = quint8(bytes[26]) | (qsizetype(quint8(bytes[27])) << 8), extraLen = quint8(bytes[28]) | (qsizetype(quint8(bytes[29])) << 8);
            if (method == 0) return bytes.mid(30 + nameLen + extraLen, 39).startsWith("application/vnd.oasis.opendocument.text");
        }
        // Another order or a compressed mimetype: the directory says what it is.
        QVector<jp::ZipEntry> entries;
        if (!jp::readZipDirectory(bytes, &entries)) return false;
        bool mimetype = false, content = false;
        for (const jp::ZipEntry &e : std::as_const(entries)) {
            if (e.name == QLatin1String("[Content_Types].xml")) return false;
            mimetype = mimetype || e.name == QLatin1String("mimetype");
            content = content || e.name == QLatin1String("content.xml");
        }
        if (!mimetype || !content) return false;
        Package p;
        if (!p.readLazyOpenDocument(bytes, nullptr, qint64(1) << 24)) return false;
        return p.part(QStringLiteral("/mimetype")).startsWith("application/vnd.oasis.opendocument.text");
    }
    return looksLikeFlatOdt(bytes);
}

void Reader::left(const QString &what)
{
    if (!m_notes || m_leftKeys.contains(what)) return;
    m_leftKeys.insert(what);
    m_notes->push_back(what);
}

bool Reader::openParts(const QByteArray &file, QString *error)
{
    auto fail = [&](const QString &m) {
        if (error) *error = m;
        return false;
    };
    QString why;
    QByteArray head = file.left(64);
    if (!file.startsWith("PK")) {
        // The flat form: one XML document.
        m_flat = true;
        if (!m_content.parse(file, &why)) return fail(why);
        m_oc = std::make_unique<Odf>(m_content);
        m_om = std::make_unique<Odf>(m_content);
        if (!m_oc->is(m_content.root(), "office:document")) return fail(QCoreApplication::translate("odt", "This is not an OpenDocument file."));
        const QString mime = m_oc->at(m_content.root(), "office:mimetype");
        if (!mime.startsWith(QLatin1String("application/vnd.oasis.opendocument.text"))) return fail(QCoreApplication::translate("odt", "This OpenDocument file is not a text document."));
        return true;
    }
    if (!m_pkg.readLazyOpenDocument(file, &why, qint64(1) << 30)) return fail(why);
    const QByteArray mime = m_pkg.part(QStringLiteral("/mimetype")).trimmed();
    if (!mime.isEmpty() && !mime.startsWith("application/vnd.oasis.opendocument.text")) {
        if (mime.startsWith("application/vnd.oasis.opendocument.")) return fail(QCoreApplication::translate("odt", "This OpenDocument file is not a text document."));
        return fail(QCoreApplication::translate("odt", "This is not an OpenDocument text file."));
    }
    if (!m_pkg.hasPart(QStringLiteral("/content.xml"))) return fail(QCoreApplication::translate("odt", "The file has no content.xml."));
    if (m_pkg.hasPart(QStringLiteral("/META-INF/manifest.xml"))) {
        // Encrypted parts are not readable.
        const QByteArray manifest = m_pkg.part(QStringLiteral("/META-INF/manifest.xml"));
        if (manifest.contains("encryption-data")) return fail(QCoreApplication::translate("odt", "This document is protected with a password. Open it in the program that made it and save a copy without the password."));
    }
    m_contentXml = m_pkg.part(QStringLiteral("/content.xml"));
    if (!m_content.parse(m_contentXml, &why)) return fail(QCoreApplication::translate("odt", "content.xml can't be read: %1").arg(why));
    m_oc = std::make_unique<Odf>(m_content);
    if (!m_oc->is(m_content.root(), "office:document-content")) return fail(QCoreApplication::translate("odt", "content.xml is not an OpenDocument text."));
    if (m_pkg.hasPart(QStringLiteral("/styles.xml"))) {
        m_stylesXml = m_pkg.part(QStringLiteral("/styles.xml"));
        if (!m_styles.parse(m_stylesXml, &why)) return fail(QCoreApplication::translate("odt", "styles.xml can't be read: %1").arg(why));
        m_os = std::make_unique<Odf>(m_styles);
    }
    if (m_pkg.hasPart(QStringLiteral("/meta.xml"))) {
        m_metaXml = m_pkg.part(QStringLiteral("/meta.xml"));
        if (m_meta.parse(m_metaXml, &why)) m_om = std::make_unique<Odf>(m_meta);
    }
    return true;
}

// ---- styles

void Reader::collectFonts(const Odf &o, int root)
{
    const int decls = o.child(root, "office:font-face-decls");
    if (decls < 0) return;
    const wml::XmlTree &t = o.tree();
    for (int c = t.firstChild(decls); c >= 0; c = t.next(c))
        if (o.is(c, "style:font-face")) {
            QString family = o.at(c, "svg:font-family");
            family.remove(QLatin1Char('\''));
            family.remove(QLatin1Char('"'));
            if (family.isEmpty()) family = o.at(c, "style:name");
            m_fonts.insert(o.at(c, "style:name"), family.trimmed());
        }
}

void Reader::collectProps(const Odf &o, int node, const QString &kind, Props *into)
{
    const wml::XmlTree &t = o.tree();
    for (const auto &a : t.attributes(node)) into->insert(kind + o.canonical(a.ns, a.local), a.value);
}

QByteArray Reader::tabStopsXml(const Odf &o, int tabs) const
{
    const wml::XmlTree &t = o.tree();
    QByteArray x;
    int count = 0;
    for (int c = t.firstChild(tabs); c >= 0 && count < 200; c = t.next(c)) {
        if (!o.is(c, "style:tab-stop")) continue;
        bool ok = false;
        const qint64 pos = lengthToTwips(o.at(c, "style:position"), &ok);
        if (!ok) continue;
        const QString type = o.at(c, "style:type");
        const char *val = type == QLatin1String("center") ? "center" : type == QLatin1String("right") ? "right" : type == QLatin1String("char") ? "decimal" : "left";
        const QString leader = o.at(c, "style:leader-style"), leaderText = o.at(c, "style:leader-text");
        const char *lead = nullptr;
        if (!leader.isEmpty() && leader != QLatin1String("none")) {
            if (leaderText == QLatin1String("_")) lead = "underscore";
            else if (leaderText == QLatin1String("-")) lead = "hyphen";
            else if (leaderText == QLatin1String(".") || leader == QLatin1String("dotted")) lead = "dot";
            else if (leader == QLatin1String("dash") || leader == QLatin1String("long-dash")) lead = "hyphen";
            else if (leader == QLatin1String("solid")) lead = "underscore";
            else lead = "dot";
        }
        x += QByteArray("<w:tab w:val=\"") + val + "\"" + (lead ? QByteArray(" w:leader=\"") + lead + "\"" : QByteArray()) + " w:pos=\"" + QByteArray::number(pos) + "\"/>";
        ++count;
    }
    return x.isEmpty() ? x : "<w:tabs>" + x + "</w:tabs>";
}

void Reader::collectStyle(const Odf &o, int node, bool automatic)
{
    const wml::XmlTree &t = o.tree();
    StyleDef d;
    d.family = o.at(node, "style:family");
    d.name = o.at(node, "style:name");
    if (d.name.isEmpty()) return;
    d.parent = o.at(node, "style:parent-style-name");
    d.display = o.at(node, "style:display-name");
    d.next = o.at(node, "style:next-style-name");
    d.listStyle = o.at(node, "style:list-style-name");
    d.masterPage = o.at(node, "style:master-page-name");
    d.outlineLevel = o.at(node, "style:default-outline-level").toInt();
    d.automatic = automatic;
    for (int c = t.firstChild(node); c >= 0; c = t.next(c)) {
        if (o.is(c, "style:paragraph-properties")) {
            collectProps(o, c, QStringLiteral("para:"), &d.props);
            const int tabs = o.child(c, "style:tab-stops");
            if (tabs >= 0) d.tabsXml = tabStopsXml(o, tabs);
        } else if (o.is(c, "style:text-properties")) collectProps(o, c, QStringLiteral("text:"), &d.props);
        else if (o.is(c, "style:table-cell-properties")) collectProps(o, c, QStringLiteral("cell:"), &d.props);
        else if (o.is(c, "style:table-properties")) collectProps(o, c, QStringLiteral("tbl:"), &d.props);
        else if (o.is(c, "style:table-column-properties")) collectProps(o, c, QStringLiteral("col:"), &d.props);
        else if (o.is(c, "style:table-row-properties")) collectProps(o, c, QStringLiteral("row:"), &d.props);
        else if (o.is(c, "style:graphic-properties")) collectProps(o, c, QStringLiteral("gr:"), &d.props);
        else if (o.is(c, "style:section-properties")) {
            collectProps(o, c, QStringLiteral("sect:"), &d.props);
            const int cols = o.child(c, "style:columns");
            if (cols >= 0) {
                d.props.insert(QStringLiteral("sect:count"), o.at(cols, "fo:column-count"));
                d.props.insert(QStringLiteral("sect:gap"), o.at(cols, "fo:column-gap"));
                if (o.child(cols, "style:column-sep") >= 0) d.props.insert(QStringLiteral("sect:sep"), QStringLiteral("1"));
            }
        }
    }
    m_styleDefs.insert(d.family + QLatin1Char('\n') + d.name, d);
    if (!automatic) m_styleOrder.push_back(d.family + QLatin1Char('\n') + d.name);
}

void Reader::collectDefaults(const Odf &o, int node)
{
    const wml::XmlTree &t = o.tree();
    const QString family = o.at(node, "style:family");
    Props p = m_defaults.value(family);
    for (int c = t.firstChild(node); c >= 0; c = t.next(c)) {
        if (o.is(c, "style:paragraph-properties")) collectProps(o, c, QStringLiteral("para:"), &p);
        else if (o.is(c, "style:text-properties")) collectProps(o, c, QStringLiteral("text:"), &p);
    }
    m_defaults.insert(family, p);
}

void Reader::collectListStyle(const Odf &o, int node)
{
    const wml::XmlTree &t = o.tree();
    ListStyleDef def;
    def.name = o.at(node, "style:name");
    if (def.name.isEmpty()) return;
    for (int c = t.firstChild(node); c >= 0; c = t.next(c)) {
        const bool bullet = o.is(c, "text:list-level-style-bullet"), number = o.is(c, "text:list-level-style-number"), image = o.is(c, "text:list-level-style-image");
        if (!bullet && !number && !image) continue;
        const int level = o.at(c, "text:level").toInt();
        if (level < 1 || level > 10) continue;
        ListLevel &l = def.level[level - 1];
        l.defined = true;
        l.bullet = bullet || image;
        l.bulletChar = bullet ? o.at(c, "text:bullet-char") : QStringLiteral("•");
        l.numFmt = o.at(c, "style:num-format");
        l.prefix = o.at(c, "style:num-prefix");
        l.suffix = o.at(c, "style:num-suffix");
        l.display = std::max(1, std::min(10, o.at(c, "text:display-levels").isEmpty() ? 1 : o.at(c, "text:display-levels").toInt()));
        l.start = o.at(c, "text:start-value").isEmpty() ? 1 : o.at(c, "text:start-value").toInt();
        l.textStyle = o.at(c, "text:style-name");
        const int lp = o.child(c, "style:list-level-properties");
        if (lp >= 0) {
            const int al = o.child(lp, "style:list-level-label-alignment");
            if (al >= 0) {
                bool okL = false, okI = false;
                const qint64 ml = lengthToTwips(o.at(al, "fo:margin-left"), &okL), ti = lengthToTwips(o.at(al, "fo:text-indent"), &okI);
                l.left = okL ? ml : 0;
                l.hanging = okI ? -ti : 0;
                l.hasIndent = true;
            } else {
                bool okS = false, okM = false;
                const qint64 sb = lengthToTwips(o.at(lp, "text:space-before"), &okS), ml = lengthToTwips(o.at(lp, "text:min-label-width"), &okM);
                l.left = (okS ? sb : 0) + (okM ? ml : 0);
                l.hanging = okM ? ml : 0;
                l.hasIndent = okS || okM;
            }
        }
    }
    m_listStyles.insert(def.name, def);
}

void Reader::collectOutline(const Odf &o, int node)
{
    // The outline's levels are written as a list style of their own.
    const wml::XmlTree &t = o.tree();
    ListStyleDef def;
    def.name = QStringLiteral("Outline");
    for (int c = t.firstChild(node); c >= 0; c = t.next(c)) {
        if (!o.is(c, "text:outline-level-style")) continue;
        const int level = o.at(c, "text:level").toInt();
        if (level < 1 || level > 10) continue;
        ListLevel &l = def.level[level - 1];
        l.defined = true;
        l.numFmt = o.at(c, "style:num-format");
        l.prefix = o.at(c, "style:num-prefix");
        l.suffix = o.at(c, "style:num-suffix");
        l.display = std::max(1, std::min(10, o.at(c, "text:display-levels").isEmpty() ? 1 : o.at(c, "text:display-levels").toInt()));
        l.start = o.at(c, "text:start-value").isEmpty() ? 1 : o.at(c, "text:start-value").toInt();
        const int lp = o.child(c, "style:list-level-properties");
        if (lp >= 0) {
            const int al = o.child(lp, "style:list-level-label-alignment");
            if (al >= 0) {
                bool okL = false, okI = false;
                const qint64 ml = lengthToTwips(o.at(al, "fo:margin-left"), &okL), ti = lengthToTwips(o.at(al, "fo:text-indent"), &okI);
                l.left = okL ? ml : 0;
                l.hanging = okI ? -ti : 0;
                l.hasIndent = true;
            } else {
                bool okS = false, okM = false;
                const qint64 sb = lengthToTwips(o.at(lp, "text:space-before"), &okS), ml = lengthToTwips(o.at(lp, "text:min-label-width"), &okM);
                l.left = (okS ? sb : 0) + (okM ? ml : 0);
                l.hanging = okM ? ml : 0;
                l.hasIndent = okS || okM;
            }
        }
    }
    m_outline = def;
    m_hasOutline = true;
    m_listStyles.insert(def.name, def);
}

void Reader::collectPageLayout(const Odf &o, int node)
{
    const wml::XmlTree &t = o.tree();
    PageLayoutDef d;
    const QString name = o.at(node, "style:name");
    for (int c = t.firstChild(node); c >= 0; c = t.next(c)) {
        if (o.is(c, "style:page-layout-properties")) {
            collectProps(o, c, QStringLiteral("page:"), &d.props);
            const int cols = o.child(c, "style:columns");
            if (cols >= 0) {
                d.props.insert(QStringLiteral("page:count"), o.at(cols, "fo:column-count"));
                d.props.insert(QStringLiteral("page:gap"), o.at(cols, "fo:column-gap"));
                if (o.child(cols, "style:column-sep") >= 0) d.props.insert(QStringLiteral("page:sep"), QStringLiteral("1"));
            }
        } else if (o.is(c, "style:header-style") || o.is(c, "style:footer-style")) {
            const bool header = o.is(c, "style:header-style");
            const int hp = o.child(c, "style:header-footer-properties");
            if (hp >= 0) collectProps(o, hp, header ? QStringLiteral("hdr:") : QStringLiteral("ftr:"), &d.props);
        }
    }
    m_layouts.insert(name, d);
}

void Reader::collectMasterPage(const Odf &o, int node)
{
    const wml::XmlTree &t = o.tree();
    MasterPageDef m;
    m.name = o.at(node, "style:name");
    m.layout = o.at(node, "style:page-layout-name");
    m.next = o.at(node, "style:next-style-name");
    for (int c = t.firstChild(node); c >= 0; c = t.next(c)) {
        const bool off = o.at(c, "style:display") == QLatin1String("false");
        if (o.is(c, "style:header")) m.header = off ? -1 : c;
        else if (o.is(c, "style:header-left")) m.headerLeft = off ? -1 : c;
        else if (o.is(c, "style:header-first")) m.headerFirst = off ? -1 : c;
        else if (o.is(c, "style:footer")) m.footer = off ? -1 : c;
        else if (o.is(c, "style:footer-left")) m.footerLeft = off ? -1 : c;
        else if (o.is(c, "style:footer-first")) m.footerFirst = off ? -1 : c;
    }
    m_masters.push_back(m);
}

void Reader::collectStyles(const Odf &o, int root, bool automaticOnly)
{
    const wml::XmlTree &t = o.tree();
    for (int part = t.firstChild(root); part >= 0; part = t.next(part)) {
        const bool common = o.is(part, "office:styles"), automatic = o.is(part, "office:automatic-styles"), master = o.is(part, "office:master-styles");
        if (automaticOnly && !automatic) continue;
        if (!common && !automatic && !master) continue;
        for (int c = t.firstChild(part); c >= 0; c = t.next(c)) {
            if (o.is(c, "style:style")) collectStyle(o, c, automatic);
            else if (o.is(c, "style:default-style")) collectDefaults(o, c);
            else if (o.is(c, "text:list-style")) collectListStyle(o, c);
            else if (o.is(c, "text:outline-style")) collectOutline(o, c);
            else if (o.is(c, "style:page-layout")) collectPageLayout(o, c);
            else if (o.is(c, "style:master-page")) collectMasterPage(o, c);
        }
    }
}

const StyleDef *Reader::style(const QString &family, const QString &name) const
{
    const auto it = m_styleDefs.constFind(family + QLatin1Char('\n') + name);
    return it == m_styleDefs.constEnd() ? nullptr : &it.value();
}

Props Reader::ownProps(const QString &family, const QString &name, QString *named, QByteArray *tabs, QString *listStyle, QString *masterPage) const
{
    // The automatic styles on the way up to the first named one, the nearest last.
    QVector<const StyleDef *> chain;
    QString current = name;
    named->clear();
    for (int depth = 0; depth < kMaxStyleChain && !current.isEmpty(); ++depth) {
        const StyleDef *d = style(family, current);
        if (!d) break;
        if (!d->automatic) {
            *named = current;
            break;
        }
        chain.push_back(d);
        current = d->parent;
    }
    Props out;
    for (int i = int(chain.size()) - 1; i >= 0; --i) {
        for (auto it = chain[i]->props.constBegin(); it != chain[i]->props.constEnd(); ++it) out.insert(it.key(), it.value());
        if (tabs && !chain[i]->tabsXml.isEmpty()) *tabs = chain[i]->tabsXml;
        if (listStyle && !chain[i]->listStyle.isEmpty()) *listStyle = chain[i]->listStyle;
        if (masterPage && !chain[i]->masterPage.isEmpty()) *masterPage = chain[i]->masterPage;
    }
    return out;
}

Props Reader::resolved(const QString &family, const QString &name, int depth) const
{
    Props out;
    if (depth >= kMaxStyleChain) return out;
    const StyleDef *d = style(family, name);
    if (!d) return m_defaults.value(family);
    out = d->parent.isEmpty() ? m_defaults.value(family) : resolved(family, d->parent, depth + 1);
    for (auto it = d->props.constBegin(); it != d->props.constEnd(); ++it) out.insert(it.key(), it.value());
    return out;
}

// ---- the ids and names of the styles in the .docx

namespace {

struct BuiltIn {
    const char *odf, *id, *name;
};

const BuiltIn kParagraphBuiltIns[] = {
    {"Standard", "Normal", "Normal"},
    {"Title", "Title", "Title"},
    {"Subtitle", "Subtitle", "Subtitle"},
    {"Text body", "BodyText", "Body Text"},
    {"Body Text", "BodyText", "Body Text"},
    {"List Paragraph", "ListParagraph", "List Paragraph"},
    {"Header", "Header", "header"},
    {"Footer", "Footer", "footer"},
    {"Footnote", "FootnoteText", "footnote text"},
    {"Endnote", "EndnoteText", "endnote text"},
    {"Caption", "Caption", "caption"},
    {"Contents Heading", "TOCHeading", "TOC Heading"},
    {"Index Heading", "IndexHeading", "index heading"},
    {"Contents 1", "TOC1", "toc 1"},
    {"Contents 2", "TOC2", "toc 2"},
    {"Contents 3", "TOC3", "toc 3"},
    {"Contents 4", "TOC4", "toc 4"},
    {"Contents 5", "TOC5", "toc 5"},
    {"Contents 6", "TOC6", "toc 6"},
    {"Contents 7", "TOC7", "toc 7"},
    {"Contents 8", "TOC8", "toc 8"},
    {"Contents 9", "TOC9", "toc 9"},
    {"Quotations", "Quote", "Quote"},
    {"Heading 1", "Heading1", "heading 1"},
    {"Heading 2", "Heading2", "heading 2"},
    {"Heading 3", "Heading3", "heading 3"},
    {"Heading 4", "Heading4", "heading 4"},
    {"Heading 5", "Heading5", "heading 5"},
    {"Heading 6", "Heading6", "heading 6"},
    {"Heading 7", "Heading7", "heading 7"},
    {"Heading 8", "Heading8", "heading 8"},
    {"Heading 9", "Heading9", "heading 9"},
};
const BuiltIn kCharacterBuiltIns[] = {
    {"Internet link", "Hyperlink", "Hyperlink"},
    {"Hyperlink", "Hyperlink", "Hyperlink"},
    {"Visited Internet Link", "FollowedHyperlink", "FollowedHyperlink"},
    {"Footnote Characters", "FootnoteReference", "footnote reference"},
    {"Footnote Symbol", "FootnoteReference", "footnote reference"},
    {"Endnote Characters", "EndnoteReference", "endnote reference"},
    {"Endnote Symbol", "EndnoteReference", "endnote reference"},
    {"Strong Emphasis", "Strong", "Strong"},
    {"Emphasis", "Emphasis", "Emphasis"},
};

const BuiltIn *builtInFor(const QString &family, const QString &display)
{
    if (family == QLatin1String("paragraph")) {
        for (const BuiltIn &b : kParagraphBuiltIns)
            if (display == QLatin1String(b.odf)) return &b;
    } else if (family == QLatin1String("text")) {
        for (const BuiltIn &b : kCharacterBuiltIns)
            if (display == QLatin1String(b.odf)) return &b;
    }
    return nullptr;
}

}

bool builtInStyleName(const QString &family, const QString &display, QString *name)
{
    const BuiltIn *b = builtInFor(family, display);
    if (b && name) *name = QString::fromLatin1(b->name);
    return b != nullptr;
}

QString Reader::displayOf(const QString &family, const QString &name) const
{
    const StyleDef *d = style(family, name);
    return d && !d->display.isEmpty() ? d->display : decodeStyleName(name);
}

QString Reader::styleId(const QString &family, const QString &name)
{
    const QString key = family + QLatin1Char('\n') + name;
    const auto it = m_idOfStyle.constFind(key);
    if (it != m_idOfStyle.constEnd()) return it.value();
    const QString display = displayOf(family, name);
    QString id;
    if (const BuiltIn *b = builtInFor(family, display)) id = QString::fromLatin1(b->id);
    else {
        for (const QChar c : display)
            if (c.isLetterOrNumber() && c.unicode() < 128) id += c;
        if (id.isEmpty() || id[0].isDigit()) id.prepend(QLatin1Char('S'));
    }
    const QString base = id;
    // (One id may be a built-in's for two ODF names: the first has it.)
    for (int n = 2; m_idsTaken.contains(id); ++n) id = base + QString::number(n);
    m_idsTaken.insert(id);
    m_idOfStyle.insert(key, id);
    return id;
}

QString Reader::fontFamily(const QString &name) const
{
    return m_fonts.value(name, name);
}

} // namespace jp::office::odt

namespace jp::office::odt {

namespace {

QByteArray num(qint64 v)
{
    return QByteArray::number(v);
}

} // namespace

// ---- pages

const MasterPageDef *Reader::masterNamed(const QString &name) const
{
    for (const MasterPageDef &m : m_masters)
        if (m.name == name) return &m;
    return nullptr;
}

QString Reader::headerPart(int node, bool header)
{
    // A part for each header or footer (an empty one for none, as a section whose first page has none needs one).
    const QString key = QString::number(node) + (header ? QLatin1Char('h') : QLatin1Char('f'));
    const auto it = m_headerParts.constFind(key);
    if (it != m_headerParts.constEnd()) return it.value();
    if (m_headerCount >= kMaxMadeParts) return QString();
    const QString name = QStringLiteral("/word/%1%2.xml").arg(header ? "header" : "footer").arg(++m_headerCount);
    const QString oldOwner = m_owner;
    m_owner = name;
    QVector<Block> blocks;
    if (node >= 0 && m_styleO) convertBlocks(*m_styleO, node, &blocks, 1);
    m_owner = oldOwner;
    const QByteArray tag = header ? "w:hdr" : "w:ftr";
    m_out.addPart(name, QByteArray(kXmlHeader) + "<" + tag + " " + docxNamespaces() + ">" + bodyBlocksXml(blocks, true) + "</" + tag + ">",
                  QString::fromLatin1(kWml) + (header ? QStringLiteral("header+xml") : QStringLiteral("footer+xml")));
    const QString rid = m_out.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRelBase) + (header ? QStringLiteral("header") : QStringLiteral("footer")), name);
    m_headerParts.insert(key, rid);
    return rid;
}

QByteArray Reader::sectPrXml(const SectionState &s)
{
    const MasterPageDef *m = masterNamed(s.master);
    if (!m && !m_masters.isEmpty()) m = &m_masters.first();
    MasterPageDef use;
    QString layoutName;
    bool titlePg = false;
    if (m) {
        use = *m;
        layoutName = m->layout;
        const MasterPageDef *next = !m->next.isEmpty() && m->next != m->name ? masterNamed(m->next) : nullptr;
        if (next) {
            // A page set apart that the pages after it do not follow: it is the first page of the section, and the others follow the master it names.
            use = *next;
            use.headerFirst = m->headerFirst >= 0 ? m->headerFirst : m->header;
            use.footerFirst = m->footerFirst >= 0 ? m->footerFirst : m->footer;
            titlePg = true;
        }
    }
    QByteArray x;
    const PageLayoutDef layout = m_layouts.value(layoutName);
    if (m) {
        auto ref = [&](const char *type, int node, bool header) {
            if (node < 0) return;
            const QString rid = headerPart(node, header);
            if (!rid.isEmpty()) x += QByteArray(header ? "<w:headerReference" : "<w:footerReference") + " w:type=\"" + type + "\" r:id=\"" + rid.toLatin1() + "\"/>";
        };
        ref("default", use.header, true);
        ref("even", use.headerLeft, true);
        ref("first", use.headerFirst, true);
        ref("default", use.footer, false);
        ref("even", use.footerLeft, false);
        ref("first", use.footerFirst, false);
        if (use.headerFirst >= 0 || use.footerFirst >= 0) titlePg = true;
        if (use.headerLeft >= 0 || use.footerLeft >= 0) m_evenOdd = true;
        // A first page that has a header but no footer (or the reverse) has the other empty.
        if (titlePg) {
            if (use.headerFirst < 0 && use.header >= 0) x += "<w:headerReference w:type=\"first\" r:id=\"" + headerPart(-1, true).toLatin1() + "\"/>";
            if (use.footerFirst < 0 && use.footer >= 0) x += "<w:footerReference w:type=\"first\" r:id=\"" + headerPart(-1, false).toLatin1() + "\"/>";
        }
    }
    if (s.continuous) x += "<w:type w:val=\"continuous\"/>";
    auto lp = [&](const char *k) { return layout.props.value(QStringLiteral("page:") + QLatin1String(k)); };
    auto len = [&](const QString &v, qint64 fallback) {
        bool ok = false;
        const qint64 tw = lengthToTwips(v, &ok);
        return ok ? tw : fallback;
    };
    qint64 w = len(lp("fo:page-width"), 12240), h = len(lp("fo:page-height"), 15840);
    auto margin = [&](const char *side, qint64 fallback) {
        QString v = lp((QStringLiteral("fo:margin-") + QLatin1String(side)).toLatin1().constData());
        if (v.isEmpty()) v = lp("fo:margin");
        return len(v, fallback);
    };
    qint64 top = margin("top", 1440), bottom = margin("bottom", 1440), left = margin("left", 1440), right = margin("right", 1440);
    const bool hasHeader = use.header >= 0 || use.headerLeft >= 0 || use.headerFirst >= 0, hasFooter = use.footer >= 0 || use.footerLeft >= 0 || use.footerFirst >= 0;
    const qint64 headerDistance = top, footerDistance = bottom;
    if (hasHeader) top += std::max<qint64>(0, len(layout.props.value(QStringLiteral("hdr:fo:min-height")), 0)) + std::max<qint64>(0, len(layout.props.value(QStringLiteral("hdr:fo:margin-bottom")), 0));
    if (hasFooter) bottom += std::max<qint64>(0, len(layout.props.value(QStringLiteral("ftr:fo:min-height")), 0)) + std::max<qint64>(0, len(layout.props.value(QStringLiteral("ftr:fo:margin-top")), 0));
    w = std::max<qint64>(720, std::min<qint64>(w, 31680));
    h = std::max<qint64>(720, std::min<qint64>(h, 31680));
    x += "<w:pgSz w:w=\"" + num(w) + "\" w:h=\"" + num(h) + "\"" + (lp("style:print-orientation") == QLatin1String("landscape") ? " w:orient=\"landscape\"" : "") + "/>";
    x += "<w:pgMar w:top=\"" + num(top) + "\" w:right=\"" + num(right) + "\" w:bottom=\"" + num(bottom) + "\" w:left=\"" + num(left) + "\" w:header=\"" + num(headerDistance) + "\" w:footer=\"" +
         num(footerDistance) + "\" w:gutter=\"0\"/>";
    {
        const QString first = lp("style:first-page-number");
        if (!first.isEmpty() && first != QLatin1String("continue") && first.toInt() > 0 && !s.continuous) x += "<w:pgNumType w:start=\"" + num(first.toInt()) + "\"/>";
    }
    // The columns: a section's own, or the page's.
    int cols = s.columns;
    qint64 gap = s.columnGap;
    bool sep = s.separator;
    if (cols <= 1 && layout.props.contains(QStringLiteral("page:count"))) {
        cols = layout.props.value(QStringLiteral("page:count")).toInt();
        bool ok = false;
        const qint64 g = lengthToTwips(layout.props.value(QStringLiteral("page:gap")), &ok);
        gap = ok ? g : 720;
        sep = layout.props.contains(QStringLiteral("page:sep"));
    }
    if (cols > 1 && cols <= 45) x += "<w:cols w:num=\"" + num(cols) + "\" w:space=\"" + num(gap) + "\"" + (sep ? " w:sep=\"1\"" : "") + "/>";
    if (titlePg) x += "<w:titlePg/>";
    return x;
}

// ---- the whole

void Reader::convertBody(const Odf &o, int body)
{
    const wml::XmlTree &t = o.tree();
    int text = -1;
    for (int c = t.firstChild(body); c >= 0; c = t.next(c))
        if (o.is(c, "office:text")) text = c;
    // The comments that have an end in the text.
    for (int i = 0; i < t.count(); ++i)
        if (o.is(i, "office:annotation-end")) m_annotationEnds.insert(o.at(i, "office:name"));
    m_sect = SectionState();
    {
        const MasterPageDef *first = masterNamed(QStringLiteral("Standard"));
        if (!first && !m_masters.isEmpty()) first = &m_masters.first();
        if (first) {
            m_sect.master = first->name;
            bool ok = false;
            const PageLayoutDef layout = m_layouts.value(first->layout);
            const qint64 w = lengthToTwips(layout.props.value(QStringLiteral("page:fo:page-width")), &ok);
            if (ok) {
                auto m = [&](const char *side) {
                    QString v = layout.props.value(QStringLiteral("page:fo:margin-") + QLatin1String(side));
                    if (v.isEmpty()) v = layout.props.value(QStringLiteral("page:fo:margin"));
                    return lengthToTwips(v);
                };
                m_pageTextWidth = double(std::max<qint64>(720, w - m("left") - m("right")));
            }
        }
    }
    QVector<Block> blocks;
    m_sectionsOn = true;
    if (text >= 0) convertBlocks(o, text, &blocks, 0);
    m_sectionsOn = false;
    m_body = bodyBlocksXml(blocks, false);
    m_finalSect = "<w:sectPr>" + sectPrXml(m_sect) + "</w:sectPr>";
}

bool Reader::read(const QByteArray &file, QString *error, QStringList *notes)
{
    m_notes = notes;
    if (!openParts(file, error)) return false;
    const int contentRoot = m_content.root();
    m_styleO = m_os ? m_os.get() : m_oc.get();
    if (m_os) {
        collectFonts(*m_os, m_styles.root());
        collectStyles(*m_os, m_styles.root(), false);
    }
    collectFonts(*m_oc, contentRoot);
    collectStyles(*m_oc, contentRoot, !m_flat);
    int body = m_oc->child(contentRoot, "office:body");
    if (body < 0 || m_oc->child(body, "office:text") < 0) {
        if (error) *error = QCoreApplication::translate("odt", "The file has no text in it.");
        return false;
    }
    m_owner = QStringLiteral("/word/document.xml");
    convertBody(*m_oc, body);

    const QString rel = QString::fromLatin1(kRelBase), wml = QString::fromLatin1(kWml);
    const QString mainName = QStringLiteral("/word/document.xml");
    m_out.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), mainName);
    // The parts the document reads from.
    m_out.addPart(QStringLiteral("/word/styles.xml"), stylesXml(), wml + QStringLiteral("styles+xml"));
    m_out.addRelationship(mainName, rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    if (!m_nums.isEmpty()) {
        m_out.addPart(QStringLiteral("/word/numbering.xml"), numberingXml(), wml + QStringLiteral("numbering+xml"));
        m_out.addRelationship(mainName, rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
    {
        QByteArray s = QByteArray(kXmlHeader) + "<w:settings " + docxNamespaces() + ">";
        if (m_evenOdd) s += "<w:evenAndOddHeaders/>";
        bool ok = false;
        const qint64 tab = lengthToTwips(m_defaults.value(QStringLiteral("paragraph")).value(QStringLiteral("para:style:tab-stop-distance")), &ok);
        s += "<w:defaultTabStop w:val=\"" + num(ok && tab > 0 ? tab : 720) + "\"/></w:settings>";
        m_out.addPart(QStringLiteral("/word/settings.xml"), s, wml + QStringLiteral("settings+xml"));
        m_out.addRelationship(mainName, rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    for (int pass = 0; pass < 2; ++pass) {
        const bool endnotes = pass == 1;
        const QByteArray &bodyXml = endnotes ? m_endnotesBody : m_footnotesBody;
        if (bodyXml.isEmpty()) continue;
        const QByteArray tag = endnotes ? "w:endnote" : "w:footnote";
        const QString name = endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
        QByteArray x = QByteArray(kXmlHeader) + "<" + tag + "s " + docxNamespaces() + ">";
        x += "<" + tag + " w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></" + tag + ">";
        x += "<" + tag + " w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></" + tag + ">";
        x += bodyXml + "</" + tag + "s>";
        m_out.addPart(name, x, wml + (endnotes ? QStringLiteral("endnotes+xml") : QStringLiteral("footnotes+xml")));
        m_out.addRelationship(mainName, rel + (endnotes ? QStringLiteral("endnotes") : QStringLiteral("footnotes")), name);
    }
    if (!m_commentsBody.isEmpty()) {
        m_out.addPart(QStringLiteral("/word/comments.xml"), QByteArray(kXmlHeader) + "<w:comments " + docxNamespaces() + ">" + m_commentsBody + "</w:comments>", wml + QStringLiteral("comments+xml"));
        m_out.addRelationship(mainName, rel + QStringLiteral("comments"), QStringLiteral("/word/comments.xml"));
    }
    m_out.addPart(mainName, QByteArray(kXmlHeader) + "<w:document " + docxNamespaces() + "><w:body>" + m_body + m_finalSect + "</w:body></w:document>", wml + QStringLiteral("document.main+xml"));
    // The properties the file names.
    if (m_om) {
        const wml::XmlTree &t = m_flat ? m_content : m_meta;
        const int root = t.root();
        const int meta = m_om->child(root, "office:meta");
        QString title, subject, creator, last, keywords, description, created, modified;
        if (meta >= 0)
            for (int c = t.firstChild(meta); c >= 0; c = t.next(c)) {
                if (m_om->is(c, "dc:title")) title = t.text(c);
                else if (m_om->is(c, "dc:subject")) subject = t.text(c);
                else if (m_om->is(c, "meta:initial-creator")) creator = t.text(c);
                else if (m_om->is(c, "dc:creator")) last = t.text(c);
                else if (m_om->is(c, "meta:keyword")) keywords += (keywords.isEmpty() ? QString() : QStringLiteral(", ")) + t.text(c);
                else if (m_om->is(c, "dc:description")) description = t.text(c);
                else if (m_om->is(c, "meta:creation-date")) created = t.text(c);
                else if (m_om->is(c, "dc:date")) modified = t.text(c);
            }
        auto date = [](const QString &d) {
            const QDateTime dt = QDateTime::fromString(d.left(19), Qt::ISODate);
            return dt.isValid() ? dt.toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ")) : QString();
        };
        QByteArray core = QByteArray(kXmlHeader) +
                          "<cp:coreProperties xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                          "xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">";
        if (!title.isEmpty()) core += "<dc:title>" + esc(title, false) + "</dc:title>";
        if (!subject.isEmpty()) core += "<dc:subject>" + esc(subject, false) + "</dc:subject>";
        if (!(creator.isEmpty() ? last : creator).isEmpty()) core += "<dc:creator>" + esc(creator.isEmpty() ? last : creator, false) + "</dc:creator>";
        if (!keywords.isEmpty()) core += "<cp:keywords>" + esc(keywords, false) + "</cp:keywords>";
        if (!description.isEmpty()) core += "<dc:description>" + esc(description, false) + "</dc:description>";
        if (!last.isEmpty()) core += "<cp:lastModifiedBy>" + esc(last, false) + "</cp:lastModifiedBy>";
        if (!date(created).isEmpty()) core += "<dcterms:created xsi:type=\"dcterms:W3CDTF\">" + date(created).toLatin1() + "</dcterms:created>";
        if (!date(modified).isEmpty()) core += "<dcterms:modified xsi:type=\"dcterms:W3CDTF\">" + date(modified).toLatin1() + "</dcterms:modified>";
        core += "</cp:coreProperties>";
        m_out.addPart(QStringLiteral("/docProps/core.xml"), core, QStringLiteral("application/vnd.openxmlformats-package.core-properties+xml"));
        m_out.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"), QStringLiteral("/docProps/core.xml"));
    }
    m_docx = m_out.finish();
    return true;
}

QByteArray toDocxPackage(const QByteArray &file, QString *error, QStringList *notes)
{
    Reader r;
    if (!r.read(file, error, notes)) return QByteArray();
    return r.docx();
}

} // namespace jp::office::odt
