#include "io/odpexport.h"

#include "io/odpexport_p.h"

#include "core/comments.h"
#include "core/fonts.h"
#include "core/sections.h"
#include "io/zip.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDateTime>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::deck {
namespace odpw {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("OdpExport", text); }

constexpr const char *kPresentationType = "application/vnd.oasis.opendocument.presentation";
constexpr const char *kTemplateType = "application/vnd.oasis.opendocument.presentation-template";

// The namespaces of the parts of a document, as the other office suite declares them.
const QString kNamespaces = QStringLiteral(
    " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
    " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
    " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\""
    " xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\""
    " xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\""
    " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
    " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
    " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
    " xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
    " xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\""
    " xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\""
    " xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\""
    " xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\""
    " xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\""
    " xmlns:math=\"http://www.w3.org/1998/Math/MathML\""
    " xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\""
    " xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\""
    " xmlns:ooo=\"http://openoffice.org/2004/office\""
    " xmlns:ooow=\"http://openoffice.org/2004/writer\""
    " xmlns:oooc=\"http://openoffice.org/2004/calc\""
    " xmlns:dom=\"http://www.w3.org/2001/xml-events\""
    " xmlns:smil=\"urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0\""
    " xmlns:anim=\"urn:oasis:names:tc:opendocument:xmlns:animation:1.0\""
    " xmlns:officeooo=\"http://openoffice.org/2009/office\""
    " xmlns:drawooo=\"http://openoffice.org/2010/draw\""
    " xmlns:loext=\"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0\""
    " office:version=\"1.3\"");

const QString kDeclaration = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

QString mediaTypeOf(const QString &format)
{
    const QString f = format.toLower();
    if (f == QLatin1String("png")) return QStringLiteral("image/png");
    if (f == QLatin1String("jpg") || f == QLatin1String("jpeg")) return QStringLiteral("image/jpeg");
    if (f == QLatin1String("gif")) return QStringLiteral("image/gif");
    if (f == QLatin1String("bmp")) return QStringLiteral("image/bmp");
    if (f == QLatin1String("tif") || f == QLatin1String("tiff")) return QStringLiteral("image/tiff");
    if (f == QLatin1String("svg")) return QStringLiteral("image/svg+xml");
    if (f == QLatin1String("emf")) return QStringLiteral("image/x-emf");
    if (f == QLatin1String("wmf")) return QStringLiteral("image/x-wmf");
    if (f == QLatin1String("webp")) return QStringLiteral("image/webp");
    return QStringLiteral("application/octet-stream");
}

// A date in the form the files of this suite write them: local time, no zone.
QString dateText(const QDateTime &t) { return (t.isValid() ? t : QDateTime::currentDateTime()).toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss")); }

} // namespace

// ---- numbers and text

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

QString number(double v, int decimals)
{
    if (!std::isfinite(v)) v = 0;
    QString s = QString::number(v, 'f', decimals);
    if (s.contains(QLatin1Char('.'))) {
        while (s.endsWith(QLatin1Char('0'))) s.chop(1);
        if (s.endsWith(QLatin1Char('.'))) s.chop(1);
    }
    return s == QLatin1String("-0") ? QStringLiteral("0") : s;
}

QString cm(double points) { return number(points * 2.54 / 72.0, 4) + QLatin1String("cm"); }

QString percent(double fraction) { return number(fraction * 100, 2) + QLatin1Char('%'); }

QString hexOf(const QColor &c) { return c.name(QColor::HexRgb); }

QString styleNameOf(const QString &display)
{
    QString out;
    for (const QChar c : display) {
        if (c.isLetterOrNumber() || c == QLatin1Char('-') || c == QLatin1Char('.') || (c == QLatin1Char('_') && false)) out += c;
        else out += QStringLiteral("_%1_").arg(c.unicode(), 2, 16, QLatin1Char('0'));
    }
    if (out.isEmpty()) out = QStringLiteral("Style");
    if (out[0].isDigit() || out[0] == QLatin1Char('-') || out[0] == QLatin1Char('.')) out.prepend(QLatin1Char('_'));
    return out;
}

// ---- the XML writer

void X::endTag()
{
    if (!m_inTag) return;
    m_out += QLatin1Char('>');
    m_inTag = false;
}

X &X::open(const char *name)
{
    endTag();
    m_out += QLatin1Char('<');
    m_out += QLatin1String(name);
    m_stack.push_back(QString::fromLatin1(name));
    m_inTag = true;
    return *this;
}

X &X::attr(const char *name, const QString &value)
{
    m_out += QLatin1Char(' ');
    m_out += QLatin1String(name);
    m_out += QLatin1String("=\"");
    m_out += esc(value);
    m_out += QLatin1Char('"');
    return *this;
}

X &X::attrs(const QString &raw)
{
    m_out += raw;
    return *this;
}

X &X::close()
{
    if (m_stack.isEmpty()) return *this;
    const QString name = m_stack.takeLast();
    if (m_inTag) {
        m_out += QLatin1String("/>");
        m_inTag = false;
    } else {
        m_out += QLatin1String("</") + name + QLatin1Char('>');
    }
    return *this;
}

X &X::text(const QString &t)
{
    endTag();
    m_out += esc(t);
    return *this;
}

X &X::raw(const QString &xml)
{
    endTag();
    m_out += xml;
    return *this;
}

// ---- named things

QString Registry::add(const QString &prefix, const QString &before, const QString &after)
{
    const QString key = before + QLatin1Char('\x01') + after;
    if (const auto it = m_byXml.constFind(key); it != m_byXml.cend()) return m_entries[it.value()].name;
    const QString name = prefix + QString::number(++m_counts[prefix]);
    m_byXml.insert(key, int(m_entries.size()));
    m_entries.push_back({name, before, after});
    return name;
}

QString Registry::addNamed(const QString &name, const QString &before, const QString &after)
{
    const QString key = before + name + QLatin1Char('\x02') + after;
    if (m_byXml.contains(key)) return name;
    m_byXml.insert(key, int(m_entries.size()));
    m_entries.push_back({name, before, after});
    return name;
}

QString Registry::style(const QString &family, const QString &prefix, const QString &parent, const QString &props, const QString &extra)
{
    QString after = QStringLiteral("\" style:family=\"") + family + QLatin1Char('"');
    if (!parent.isEmpty()) after += QStringLiteral(" style:parent-style-name=\"") + esc(parent) + QLatin1Char('"');
    after += extra;
    after += props.isEmpty() ? QStringLiteral("/>") : QLatin1Char('>') + props + QStringLiteral("</style:style>");
    return add(prefix, QStringLiteral("<style:style style:name=\""), after);
}

void Registry::write(X &x) const
{
    for (const Entry &e : m_entries) x.raw(e.before + e.name + e.after);
}

// ---- the writer

Exporter::Exporter(const Document &doc, bool asTemplate, QStringList *skipped)
    : m_doc(doc), m_template(asTemplate), m_callerSkipped(skipped), m_theme(themeOf(doc)), m_colors(doc.colors), m_fonts(doc.fonts), m_size(doc.pageSize())
{
    const QJsonObject size = doc.appData.value(QStringLiteral("notesSize")).toObject();
    if (size.value(QStringLiteral("w")).toDouble() > 0 && size.value(QStringLiteral("h")).toDouble() > 0)
        m_notesSize = QSizeF(size.value(QStringLiteral("w")).toDouble(), size.value(QStringLiteral("h")).toDouble());
}

void Exporter::say(const QString &note)
{
    if (!m_skipped.contains(note)) m_skipped << note;
}

void Exporter::sayOnce(const QString &key, const QString &note)
{
    if (m_said.contains(key)) return;
    m_said.insert(key);
    say(note);
}

QString Exporter::uniquePageName(const QString &wanted)
{
    QString base = wanted.simplified();
    base.remove(QLatin1Char(','));
    if (base.isEmpty()) base = QStringLiteral("Slide");
    QString name = base;
    for (int n = 2; m_pageNames.contains(name.toLower()); ++n) name = QStringLiteral("%1 (%2)").arg(base).arg(n);
    m_pageNames.insert(name.toLower());
    return name;
}

QString Exporter::fontDecl(const QString &family)
{
    const QString name = interchangeFontName(family).trimmed();
    if (name.isEmpty()) return QString();
    if (!m_fontDecls.contains(name)) {
        m_fontDecls.insert(name, name);
        m_fontOrder << name;
    }
    return name;
}

void Exporter::writeFontDecls(X &x) const
{
    x.open("office:font-face-decls");
    for (const QString &name : m_fontOrder) {
        x.open("style:font-face").attr("style:name", name);
        x.attr("svg:font-family", name.contains(QLatin1Char(' ')) || name.contains(QLatin1Char(',')) ? QLatin1Char('\'') + name + QLatin1Char('\'') : name);
        x.close();
    }
    x.close();
}

void Exporter::addFile(const QString &path, const QByteArray &data, const QString &mediaType, bool compress)
{
    if (!m_files.contains(path)) m_order << path;
    m_files.insert(path, data);
    m_mediaTypes.insert(path, mediaType);
    m_compress.insert(path, compress);
}

QString Exporter::addPicture(const QString &imageId)
{
    if (imageId.isEmpty()) return QString();
    if (const auto it = m_pictureNames.constFind(imageId); it != m_pictureNames.cend()) return it.value();
    const auto image = m_doc.images.constFind(imageId);
    if (image == m_doc.images.cend() || image->bytes.isEmpty()) {
        m_pictureNames.insert(imageId, QString());
        return QString();
    }
    QString ext = image->format.toLower();
    if (ext == QLatin1String("jpeg")) ext = QStringLiteral("jpg");
    if (ext.isEmpty()) ext = QStringLiteral("png");
    const QString name = QStringLiteral("Pictures/image%1.%2").arg(m_pictureNames.size() + 1).arg(ext);
    addFile(name, image->bytes, mediaTypeOf(ext), ext != QLatin1String("png") && ext != QLatin1String("jpg") && ext != QLatin1String("gif"));
    m_pictureNames.insert(imageId, name);
    return name;
}

// A movie or sound kept in the presentation, as a part of the package under Media/; empty (a link to a file outside has none) if there are no bytes.
QString Exporter::addMedia(const QString &mediaId, QString *mediaType)
{
    const auto it = m_doc.media.constFind(mediaId);
    if (it == m_doc.media.cend() || it->bytes.isEmpty()) return QString();
    const QString key = QStringLiteral("media:") + mediaId;
    if (mediaType) *mediaType = it->contentType.isEmpty() ? QStringLiteral("application/octet-stream") : it->contentType;
    if (const auto known = m_pictureNames.constFind(key); known != m_pictureNames.cend()) return known.value();
    const QString name = QStringLiteral("Media/media%1.%2").arg(m_mediaCount + 1).arg(it->extension.isEmpty() ? QStringLiteral("bin") : it->extension);
    ++m_mediaCount;
    addFile(name, it->bytes, it->contentType.isEmpty() ? QStringLiteral("application/octet-stream") : it->contentType, false);
    m_pictureNames.insert(key, name);
    return name;
}

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
    if (!image.convertToFormat(QImage::Format_RGB32).save(&buffer, "PNG")) return QByteArray();
    return bytes;
}

// ---- masters and layouts

namespace {

bool isHolder(const ItemPtr &it) { return it->role.startsWith(QLatin1String("ph:")); }

// A layout that holds only placeholders, with no background of its own and the master's graphics under it, needs no page of its own: its slides' placeholders say where they stand.
bool needsOwnPage(const MasterPage &layout)
{
    if (!layout.background.isNone() || !layout.showMasterItems) return true;
    return std::any_of(layout.items.begin(), layout.items.end(), [](const ItemPtr &it) { return !isHolder(it); });
}

} // namespace

void Exporter::plan()
{
    QVector<const MasterPage *> masters;
    for (const auto &m : m_doc.masters)
        if (isSlideMaster(*m)) masters << m.get();
    QSet<QString> names;
    auto uniqueName = [&](const QString &display) {
        const QString base = styleNameOf(display);
        QString name = base;
        for (int n = 2; names.contains(name.toLower()); ++n) name = QStringLiteral("%1_%2").arg(base).arg(n);
        names.insert(name.toLower());
        return name;
    };
    if (masters.isEmpty()) {
        MasterOut m;
        m.display = tr("Slide Master");
        m.name = uniqueName(m.display);
        m.prefix = m.name + QLatin1Char('-');
        m_masters.push_back(m);
    }
    QHash<QString, int> byId;
    for (const MasterPage *master : std::as_const(masters)) {
        MasterOut m;
        m.master = master;
        m.display = master->name.isEmpty() ? tr("Slide Master") : master->name;
        m.name = uniqueName(m.display);
        m.prefix = m.name + QLatin1Char('-');
        byId.insert(master->id, int(m_masters.size()));
        m_pageOfMaster.insert(master, int(m_masters.size()));
        m_masters.push_back(m);
    }
    for (const auto &l : m_doc.masters) {
        if (!isLayout(*l)) continue;
        int under = byId.value(l->baseId, 0);
        if (!needsOwnPage(*l) || m_masters.empty()) {
            m_pageOfMaster.insert(l.get(), under);
            continue;
        }
        MasterOut m;
        m.master = m_masters[under].master;
        m.layout = l.get();
        const QString layoutName = l->name.isEmpty() ? tr("Layout") : l->name;
        m.display = masters.size() > 1 ? m_masters[under].display + QStringLiteral(" - ") + layoutName : layoutName;
        m.name = uniqueName(m.display);
        m.prefix = m.name + QLatin1Char('-');
        m_pageOfMaster.insert(l.get(), int(m_masters.size()));
        m_masters.push_back(m);
    }
}

const Exporter::MasterOut &Exporter::pageOf(const Page &slide) const
{
    const MasterPage *m = m_doc.masterFor(slide);
    const auto it = m_pageOfMaster.constFind(m);
    return m_masters[it == m_pageOfMaster.cend() ? 0 : it.value()];
}

Exporter::PlacedItems Exporter::itemsOf(const MasterOut &m) const
{
    PlacedItems out;
    if (m.master && (!m.layout || m.layout->showMasterItems))
        for (const ItemPtr &it : m.master->items)
            if (!isHolder(it) || !m.layout) (isHolder(it) ? out.holders : out.decor).push_back(it);
    if (m.layout)
        for (const ItemPtr &it : m.layout->items) (isHolder(it) ? out.holders : out.decor).push_back(it);
    return out;
}

// ---- footers

namespace {

QString plainText(const QTextDocument *doc)
{
    if (!doc) return QString();
    QString text = doc->toPlainText();
    text.remove(QChar(QChar::ObjectReplacementCharacter));
    return text.replace(QChar(QChar::LineSeparator), QLatin1Char(' ')).replace(QLatin1Char('\n'), QLatin1Char(' ')).simplified();
}

// The field a story holds, "page" or "date:M/d/yyyy"; empty if it has none.
QString fieldCode(const QTextDocument *doc)
{
    if (!doc) return QString();
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().isValid() && !it.fragment().charFormat().stringProperty(tp::Field).isEmpty()) return it.fragment().charFormat().stringProperty(tp::Field);
    return QString();
}

} // namespace

Exporter::Footer Exporter::footerOf(const Page &slide)
{
    Footer f;
    for (const ItemPtr &it : slide.items) {
        if (it->type() != ItemType::Text) continue;
        const QString type = placeholderType(*it);
        const QTextDocument *story = m_doc.storyDoc(static_cast<const TextItem &>(*it).storyId);
        if (type == QLatin1String("sldNum")) {
            f.number = true;
        } else if (type == QLatin1String("ftr")) {
            f.footer = true;
            f.footerText = plainText(story);
        } else if (type == QLatin1String("dt")) {
            f.date = true;
            const QString code = fieldCode(story), kind = code.section(QLatin1Char(':'), 0, 0);
            if (kind == QLatin1String("date") || kind == QLatin1String("time") || kind == QLatin1String("datetime")) f.dateCode = code;
            else f.dateText = plainText(story);
        }
    }
    return f;
}

bool Exporter::isFooterHolder(const Item &it, const Footer &) const
{
    if (it.type() != ItemType::Text) return false;
    const QString type = placeholderType(it);
    return type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum");
}

void Exporter::writeFooterDecls(X &x)
{
    // The footer texts and the dates the slides name, each once.
    for (int i = 0; i < m_footerDecls.size(); ++i)
        x.open("presentation:footer-decl").attr("presentation:name", QStringLiteral("ftr%1").arg(i + 1)).text(m_footerDecls[i]).close();
    int n = 0;
    for (const QString &d : std::as_const(m_dateDecls)) {
        ++n;
        x.open("presentation:date-time-decl").attr("presentation:name", QStringLiteral("dtd%1").arg(n));
        if (d.startsWith(QLatin1String("fixed:"))) {
            x.attr("presentation:source", "fixed").text(d.mid(6));
        } else {
            bool isTime = false;
            const QString style = dateStyle(m_contentStyles, d.mid(8), &isTime);
            x.attr("presentation:source", "current-date");
            if (!style.isEmpty()) x.attr("style:data-style-name", style);
        }
        x.close();
    }
}

// ---- pages

QString Exporter::pageLayoutFor(const QSizeF &size)
{
    const QString key = QStringLiteral("%1,%2").arg(size.width(), 0, 'f', 3).arg(size.height(), 0, 'f', 3);
    if (const auto it = m_pageLayouts.constFind(key); it != m_pageLayouts.cend()) return it.value();
    const QString name = QStringLiteral("PM%1").arg(m_pageLayouts.size());
    m_pageLayouts.insert(key, name);
    m_pageLayoutXml << QStringLiteral("<style:page-layout style:name=\"%1\"><style:page-layout-properties fo:margin-top=\"0cm\" fo:margin-bottom=\"0cm\" fo:margin-left=\"0cm\" "
                                      "fo:margin-right=\"0cm\" fo:page-width=\"%2\" fo:page-height=\"%3\" style:print-orientation=\"%4\"/></style:page-layout>")
                           .arg(name, cm(size.width()), cm(size.height()), size.width() >= size.height() ? QStringLiteral("landscape") : QStringLiteral("portrait"));
    return name;
}

QString Exporter::masterPageStyle(const MasterOut &m)
{
    Fill bg = m.master ? m.master->background : Fill();
    if (m.layout && !m.layout->background.isNone()) bg = m.layout->background;
    if (bg.isNone()) bg = Fill::solid(ColorRef::rgb(Qt::white));
    const QString props = QStringLiteral("<style:drawing-page-properties draw:background-size=\"border\"%1/>").arg(fillAttrs(bg, m_size));
    return m_masterStyles.style(QStringLiteral("drawing-page"), QStringLiteral("Mdp"), QString(), props);
}

QString Exporter::pageStyle(const Page &page, const Footer &f, bool)
{
    QString attrs = QStringLiteral(" presentation:background-visible=\"true\" presentation:background-objects-visible=\"%1\"").arg(page.showMasterItems ? QStringLiteral("true") : QStringLiteral("false"));
    attrs += QStringLiteral(" presentation:display-footer=\"%1\" presentation:display-page-number=\"%2\" presentation:display-date-time=\"%3\"")
                 .arg(f.footer ? QStringLiteral("true") : QStringLiteral("false"), f.number ? QStringLiteral("true") : QStringLiteral("false"), f.date ? QStringLiteral("true") : QStringLiteral("false"));
    if (page.hidden) attrs += QStringLiteral(" presentation:visibility=\"hidden\"");
    if (!page.background.isNone()) attrs += fillAttrs(page.background, m_size);
    attrs += transitionAttrs(page);
    return m_contentStyles.style(QStringLiteral("drawing-page"), QStringLiteral("dp"), QString(), QStringLiteral("<style:drawing-page-properties%1/>").arg(attrs));
}

// ---- the master pages

void Exporter::writeThemeColors(X &x) const
{
    static const char *const names[12] = {"dark1", "light1", "dark2", "light2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hyperlink", "followed-hyperlink"};
    x.open("loext:theme").attr("loext:name", m_theme.name.isEmpty() ? tr("Theme") : m_theme.name);
    x.open("loext:theme-colors").attr("loext:name", m_theme.name.isEmpty() ? tr("Theme") : m_theme.name);
    for (int i = 0; i < 12; ++i) x.open("loext:color").attr("loext:name", names[i]).attr("loext:color", hexOf(m_theme.colors[i].isValid() ? m_theme.colors[i] : QColor(Qt::black))).close();
    x.close().close();
}

// The class a placeholder has on a master page, and a slide.
QString holderClassOf(const QString &type)
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return QStringLiteral("title");
    if (type == QLatin1String("subTitle")) return QStringLiteral("subtitle");
    if (type == QLatin1String("body") || type == QLatin1String("obj")) return QStringLiteral("outline");
    if (type == QLatin1String("dt")) return QStringLiteral("date-time");
    if (type == QLatin1String("ftr")) return QStringLiteral("footer");
    if (type == QLatin1String("sldNum")) return QStringLiteral("page-number");
    if (type == QLatin1String("hdr")) return QStringLiteral("header");
    if (type == QLatin1String("pic")) return QStringLiteral("graphic");
    if (type == QLatin1String("chart")) return QStringLiteral("chart");
    if (type == QLatin1String("tbl")) return QStringLiteral("table");
    if (type == QLatin1String("dgm") || type == QLatin1String("media") || type == QLatin1String("clipArt")) return QStringLiteral("object");
    return QString();
}

void Exporter::writeMasterPage(X &x, const MasterOut &m, QString *presentationStyles)
{
    m_currentMaster = &m;
    Scope sc;
    sc.styles = &m_masterStyles;
    sc.master = true;
    sc.layer = QStringLiteral("backgroundobjects");
    sc.prefix = m.prefix;
    sc.where = tr("Master page \"%1\"").arg(m.display);
    Fill bg = m.master ? m.master->background : Fill();
    if (m.layout && !m.layout->background.isNone()) bg = m.layout->background;
    sc.background = bg.type == Fill::Solid && bg.color.kind() == ColorRef::Rgb ? bg.color.rgbValue() : (bg.isNone() ? QColor(Qt::white) : resolve(bg.color));
    if (!sc.background.isValid()) sc.background = Qt::white;
    if (bg.type == Fill::Gradient && !bg.stops.isEmpty()) sc.background = resolve(bg.stops.first().color);

    x.open("style:master-page").attr("style:name", m.name).attr("style:display-name", m.display);
    x.attr("style:page-layout-name", pageLayoutFor(m_size)).attr("draw:style-name", masterPageStyle(m));
    writeThemeColors(x);

    const PlacedItems placed = itemsOf(m);
    writeItems(x, sc, placed.decor);
    // One placeholder of each class.
    QSet<QString> classes;
    ItemList holders;
    for (const ItemPtr &it : placed.holders) {
        const QString cls = holderClassOf(placeholderType(*it));
        if (cls.isEmpty() || classes.contains(cls)) continue;
        classes.insert(cls);
        holders.push_back(it);
    }
    writeItems(x, sc, holders);

    // The notes page of this master.
    x.open("presentation:notes").attr("style:page-layout-name", pageLayoutFor(m_notesSize));
    const double sx = m_notesSize.width() / 540.0, sy = m_notesSize.height() / 720.0;
    const double imageW = 432 * sx, imageH = m_size.width() > 0 ? imageW * m_size.height() / m_size.width() : 243 * sy;
    x.open("draw:page-thumbnail").attr("presentation:style-name", m.prefix + QStringLiteral("title")).attr("draw:layer", "backgroundobjects");
    x.len("svg:width", imageW).len("svg:height", imageH).len("svg:x", 54 * sx).len("svg:y", 90 * sy).attr("presentation:class", "page").close();
    x.open("draw:frame").attr("presentation:style-name", m.prefix + QStringLiteral("notes")).attr("draw:layer", "backgroundobjects");
    x.len("svg:width", 432 * sx).len("svg:height", 283.5 * sy).len("svg:x", 54 * sx).len("svg:y", 346.5 * sy);
    x.attr("presentation:class", "notes").attr("presentation:placeholder", "true");
    x.leaf("draw:text-box");
    x.close();
    x.close();
    x.close();

    X styles;
    writePresentationStyles(styles, m);
    *presentationStyles += styles.str();
    m_currentMaster = nullptr;
}

// The text styles of a master page: its title, subtitle, and the nine levels of its outline, as the text of its placeholders looks, and
// what the other objects of the page start from.
void Exporter::writePresentationStyles(X &out, const MasterOut &m)
{
    PlacedItems placed = itemsOf(m);
    ItemList holders = placed.holders;
    if (m.master)
        for (const ItemPtr &it : m.master->items)
            if (isHolder(it)) holders.push_back(it);
    const TextItem *title = nullptr, *subtitle = nullptr, *body = nullptr;
    for (const ItemPtr &it : holders) {
        if (it->type() != ItemType::Text) continue;
        const QString type = placeholderType(*it);
        const auto *t = static_cast<const TextItem *>(it.get());
        if (!title && (type == QLatin1String("title") || type == QLatin1String("ctrTitle"))) title = t;
        else if (!subtitle && type == QLatin1String("subTitle")) subtitle = t;
        else if (!body && (type == QLatin1String("body") || type == QLatin1String("obj"))) body = t;
    }
    // Levels the placeholder has no text for look like the first, smaller the deeper they go.
    auto plain = [&](bool heading, double size) {
        Level l;
        l.chr.setFontFamilies({heading ? m_theme.headingFont : m_theme.bodyFont});
        l.chr.setFontPointSize(size);
        l.chr.setProperty(tp::ColorRefP, ColorRef::rgb(m_theme.colors[0]).toString());
        l.seen = true;
        return l;
    };
    auto levelsFor = [&](const TextItem *t, bool heading) {
        QVector<Level> levels = levelsOf(t ? m_doc.storyDoc(t->storyId) : nullptr);
        static const double sizes[9] = {28, 24, 20, 18, 18, 18, 18, 18, 18};
        if (!levels[0].seen) levels[0] = plain(heading, heading ? 44 : 28);
        for (int i = 1; i < 9; ++i) {
            if (levels[i].seen) continue;
            levels[i] = levels[0];
            if (!heading && levels[0].chr.hasProperty(QTextFormat::FontPointSize)) levels[i].chr.setFontPointSize(levels[0].chr.fontPointSize() * sizes[i] / sizes[0]);
            levels[i].blk.setLeftMargin(36.0 * i + (levels[0].listed ? 18 : 0));
            levels[i].blk.setTextIndent(levels[0].listed ? -18 : 0);
        }
        return levels;
    };
    auto frameAttrs = [&](const TextItem *t) {
        const QMarginsF in = t ? t->insets : QMarginsF(7.2, 3.6, 7.2, 3.6);
        const VAlign v = t ? t->valign : VAlign::Top;
        QString a = QStringLiteral(" draw:stroke=\"none\" draw:fill=\"none\" draw:textarea-vertical-align=\"%1\" draw:textarea-horizontal-align=\"justify\" draw:auto-grow-height=\"false\" draw:fit-to-size=\"false\" "
                                   "style:shrink-to-fit=\"false\" fo:wrap-option=\"wrap\"")
                        .arg(v == VAlign::Middle ? QStringLiteral("middle") : v == VAlign::Bottom ? QStringLiteral("bottom") : QStringLiteral("top"));
        a += QStringLiteral(" fo:padding-top=\"%1\" fo:padding-bottom=\"%2\" fo:padding-left=\"%3\" fo:padding-right=\"%4\"").arg(cm(in.top()), cm(in.bottom()), cm(in.left()), cm(in.right()));
        return a;
    };
    auto emit = [&](const QString &name, const QString &graphic, const QString &children, const Level &l) {
        out.open("style:style").attr("style:name", m.prefix + name).attr("style:family", "presentation");
        out.open("style:graphic-properties").attrs(graphic);
        if (!children.isEmpty()) out.raw(children);
        out.close();
        out.raw(paraProps(l.blk, false));
        out.raw(QStringLiteral("<style:text-properties%1/>").arg(charAttrs(l.chr)));
        out.close();
    };
    const QVector<Level> titleLevels = levelsFor(title, true);
    emit(QStringLiteral("title"), frameAttrs(title), QString(), titleLevels[0]);
    const QVector<Level> subtitleLevels = subtitle ? levelsFor(subtitle, false) : levelsFor(body, false);
    emit(QStringLiteral("subtitle"), frameAttrs(subtitle ? subtitle : body), QString(), subtitleLevels[0]);
    const QVector<Level> bodyLevels = levelsFor(body, false);
    for (int i = 0; i < 9; ++i) {
        const Level &l = bodyLevels[i];
        QString children;
        if (l.listed) {
            double marL = 0, indent = 0;
            const QString level = listLevelXml(i + 1, l.blk, l.list, l.chr.hasProperty(QTextFormat::FontPointSize) ? l.chr.fontPointSize() : 18.0, &marL, &indent);
            children = QStringLiteral("<text:list-style style:name=\"%1\">%2</text:list-style>").arg(esc(m.prefix + QStringLiteral("outline%1").arg(i + 1)), level);
        }
        emit(QStringLiteral("outline%1").arg(i + 1), frameAttrs(body), children, l);
    }
    Level notes = plain(false, 12);
    emit(QStringLiteral("notes"), QStringLiteral(" draw:stroke=\"none\" draw:fill=\"none\" draw:auto-grow-height=\"false\""), QString(), notes);
    // What the footers and the other objects of a master page start from. (No style for the background: the other suite takes a
    // style of that name as the look of the master page's background, over the page's own.)
    out.open("style:style").attr("style:name", m.prefix + QStringLiteral("backgroundobjects")).attr("style:family", "presentation")
        .open("style:graphic-properties").attr("draw:stroke", "none").attr("draw:fill", "none").close().close();
}

QString Exporter::writeMasterPages(QString *presentationStyles)
{
    X x;
    x.open("draw:layer-set");
    for (const char *layer : {"layout", "background", "backgroundobjects", "controls", "measurelines"}) x.open("draw:layer").attr("draw:name", layer).close();
    x.close();
    for (const MasterOut &m : m_masters) writeMasterPage(x, m, presentationStyles);
    return x.str();
}

// ---- the slides

void Exporter::writeSlides(X &x)
{
    // What the slides show at their feet, declared once.
    m_footers.clear();
    for (const auto &page : m_doc.pages) {
        Footer f = footerOf(*page);
        if (f.footer) {
            int at = int(m_footerDecls.indexOf(f.footerText));
            if (at < 0) {
                at = int(m_footerDecls.size());
                m_footerDecls << f.footerText;
            }
            f.footerDecl = at;
        }
        if (f.date) {
            const QString key = f.dateCode.isEmpty() ? QStringLiteral("fixed:") + f.dateText : QStringLiteral("current:") + f.dateCode;
            int at = int(m_dateDecls.indexOf(key));
            if (at < 0) {
                at = int(m_dateDecls.size());
                m_dateDecls << key;
            }
            f.dateDecl = at;
        }
        m_footers << f;
    }
    x.open("office:presentation");
    writeFooterDecls(x);
    for (int i = 0; i < m_doc.pages.size(); ++i) writeSlide(x, i);
    writeShowSettings(x);
    x.close();
}

void Exporter::writeSlide(X &x, int index)
{
    const Page &page = *m_doc.pages[index];
    const MasterOut &master = pageOf(page);
    const Footer &f = m_footers[index];
    const QString name = uniquePageName(page.title.isEmpty() ? tr("Slide %1").arg(index + 1) : page.title);
    Scope sc;
    sc.styles = &m_contentStyles;
    sc.layer = QStringLiteral("layout");
    sc.prefix = master.prefix;
    sc.where = tr("Slide %1").arg(index + 1);
    sc.background = !page.background.isNone() ? (page.background.type == Fill::Gradient && !page.background.stops.isEmpty() ? resolve(page.background.stops.first().color) : resolve(page.background.color)) : QColor();
    if (!sc.background.isValid()) {
        const MasterOut &mo = master;
        Fill bg = mo.master ? mo.master->background : Fill();
        if (mo.layout && !mo.layout->background.isNone()) bg = mo.layout->background;
        sc.background = bg.type == Fill::Gradient && !bg.stops.isEmpty() ? resolve(bg.stops.first().color) : bg.isNone() ? QColor(Qt::white) : resolve(bg.color);
    }
    if (!sc.background.isValid()) sc.background = Qt::white;

    m_currentMaster = &master;
    m_page = &page;
    m_where = sc.where;
    // The objects something else names: connectors' ends, and what animations aim at.
    m_ids.clear();
    m_idOfShape.clear();
    m_paragraphIds.clear();
    m_needIds.clear();
    int &counter = m_idCounter;
    walkItems(page.items, [&](const ItemPtr &it) {
        if (it->type() == ItemType::Line) {
            const auto &l = static_cast<const LineItem &>(*it);
            if (!l.start.id.isEmpty()) m_needIds.insert(l.start.id);
            if (!l.end.id.isEmpty()) m_needIds.insert(l.end.id);
        }
    });
    const Timeline timeline = timelineOf(page);
    QSet<int> aimed;
    QSet<QPair<int, int>> paragraphs;
    for (const Step &step : timeline.steps)
        for (const Effect &e : step.effects)
            for (const Behavior &b : e.behaviors) {
                if (b.target.shape <= 0) continue;
                aimed.insert(b.target.shape);
                if (b.target.paragraphs())
                    for (int p = b.target.firstParagraph; p <= b.target.lastParagraph && p < b.target.firstParagraph + 1000; ++p) paragraphs.insert(qMakePair(b.target.shape, p));
            }
    // (A click on a movie or sound is written, so the object it is on and the one it plays have ids too.)
    for (const Trigger &trigger : timeline.triggers) {
        if (trigger.shape > 0) aimed.insert(trigger.shape);
        for (const Effect &e : trigger.effects)
            for (const Behavior &b : e.behaviors)
                if (b.kind == Behavior::Kind::Command && b.target.shape > 0) aimed.insert(b.target.shape);
    }
    walkItems(page.items, [&](const ItemPtr &it) {
        if (m_needIds.contains(it->id) || (it->sourceId > 0 && aimed.contains(it->sourceId))) {
            const QString id = QStringLiteral("id%1").arg(++counter);
            m_ids.insert(it->id, id);
            if (it->sourceId > 0) m_idOfShape.insert(it->sourceId, id);
        }
        // The paragraphs of its text that an animation aims at.
        if (it->sourceId <= 0) return;
        QString story;
        if (it->type() == ItemType::Text) story = static_cast<const TextItem &>(*it).storyId;
        else if (it->type() == ItemType::Shape) story = static_cast<const ShapeItem &>(*it).storyId;
        const QTextDocument *doc = story.isEmpty() ? nullptr : m_doc.storyDoc(story);
        if (!doc) return;
        for (int p = 0; p < doc->blockCount(); ++p)
            if (paragraphs.contains(qMakePair(it->sourceId, p))) m_paragraphIds.insert(qMakePair(it->sourceId, p), QStringLiteral("id%1").arg(++counter));
    });
    m_slideNames << name;
    // What this kind of file holds as a picture only.
    for (const QJsonValue &v : page.props.value(QStringLiteral("charts")).toArray()) {
        const QString kind = v.toObject().value(QStringLiteral("kind")).toString();
        if (kind == QLatin1String("diagram")) sayOnce(QStringLiteral("diagram"), tr("SmartArt diagrams are saved as pictures: their parts can't be edited in this kind of file."));
        else if (kind == QLatin1String("equation")) sayOnce(QStringLiteral("equation"), tr("Equations are saved as pictures."));
        else sayOnce(QStringLiteral("chart"), tr("Charts are saved as pictures: their data can't be edited in this kind of file."));
    }
    if (!page.props.value(QStringLiteral("unread")).toArray().isEmpty()) say(tr("%1: a table, chart, or other frame that can't be read was left out.").arg(sc.where));

    x.open("draw:page").attr("draw:name", name).attr("draw:style-name", pageStyle(page, f, true)).attr("draw:master-page-name", master.name);
    if (f.footer) x.attr("presentation:use-footer-name", QStringLiteral("ftr%1").arg(f.footerDecl + 1));
    if (f.date) x.attr("presentation:use-date-time-name", QStringLiteral("dtd%1").arg(f.dateDecl + 1));
    writeComments(x, page);
    ItemList shown;
    for (const ItemPtr &it : page.items)
        if (!isFooterHolder(*it, f)) shown.push_back(it);
    writeItems(x, sc, shown);
    writeAnimations(x, page);
    writeNotes(x, index);
    x.close();
    m_currentMaster = nullptr;
}

void Exporter::writeNotes(X &x, int index)
{
    const Page &page = *m_doc.pages[index];
    const MasterOut &master = pageOf(page);
    const QTextDocument *story = m_doc.storyDoc(page.notesStoryId);
    const bool empty = !story || story->isEmpty() || story->toPlainText().trimmed().isEmpty();
    const double sx = m_notesSize.width() / 540.0, sy = m_notesSize.height() / 720.0;
    const double imageW = 432 * sx, imageH = m_size.width() > 0 ? imageW * m_size.height() / m_size.width() : 243 * sy;
    const QString pageProps = QStringLiteral("<style:drawing-page-properties presentation:display-header=\"true\" presentation:display-footer=\"true\" "
                                             "presentation:display-page-number=\"false\" presentation:display-date-time=\"true\"/>");
    const QString pageStyleName = m_contentStyles.style(QStringLiteral("drawing-page"), QStringLiteral("dp"), QString(), pageProps);
    x.open("presentation:notes").attr("draw:style-name", pageStyleName);
    x.open("draw:page-thumbnail").attr("draw:style-name", m_contentStyles.style(QStringLiteral("graphic"), QStringLiteral("gr"), QString(), QStringLiteral("<style:graphic-properties style:protect=\"size\"/>")));
    x.attr("draw:layer", "layout").len("svg:width", imageW).len("svg:height", imageH).len("svg:x", 54 * sx).len("svg:y", 90 * sy);
    x.attr("draw:page-number", index + 1).attr("presentation:class", "page").close();
    Scope sc;
    sc.styles = &m_contentStyles;
    sc.layer = QStringLiteral("layout");
    sc.prefix = master.prefix;
    sc.background = Qt::white;
    const QString props = QStringLiteral("<style:graphic-properties draw:fill=\"none\" draw:stroke=\"none\" draw:auto-grow-height=\"false\" fo:min-height=\"%1\"/>").arg(cm(283.5 * sy));
    const QString frameStyle = m_contentStyles.style(QStringLiteral("presentation"), QStringLiteral("pr"), master.prefix + QStringLiteral("notes"), props);
    x.open("draw:frame").attr("presentation:style-name", frameStyle).attr("draw:layer", "layout");
    if (!empty) x.attr("draw:text-style-name", slideLinesStyle(sc));
    x.len("svg:width", 432 * sx).len("svg:height", 283.5 * sy).len("svg:x", 54 * sx).len("svg:y", 346.5 * sy).attr("presentation:class", "notes");
    if (empty) x.attr("presentation:placeholder", "true");
    x.open("draw:text-box");
    if (!empty) {
        TextOpts opts;
        opts.behind = Qt::white;
        writeStory(x, sc, story, opts);
    }
    x.close();
    x.close();
    x.close();
}

// ---- the parts

void Exporter::writeMeta()
{
    const DocProps &p = m_doc.props;
    X x;
    x.open("office:document-meta").attr("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0").attr("xmlns:xlink", "http://www.w3.org/1999/xlink");
    x.attr("xmlns:dc", "http://purl.org/dc/elements/1.1/").attr("xmlns:meta", "urn:oasis:names:tc:opendocument:xmlns:meta:1.0").attr("xmlns:ooo", "http://openoffice.org/2004/office");
    x.attr("office:version", "1.3");
    x.open("office:meta");
    x.open("meta:generator").text(QStringLiteral("JeffDeck/") + QStringLiteral(JP_VERSION)).close();
    if (!p.title.isEmpty()) x.open("dc:title").text(p.title).close();
    if (!p.comments.isEmpty()) x.open("dc:description").text(p.comments).close();
    if (!p.subject.isEmpty()) x.open("dc:subject").text(p.subject).close();
    for (const QString &k : p.keywords.split(QLatin1Char(','), Qt::SkipEmptyParts))
        if (!k.trimmed().isEmpty()) x.open("meta:keyword").text(k.trimmed()).close();
    if (!p.author.isEmpty()) x.open("meta:initial-creator").text(p.author).close();
    x.open("meta:creation-date").text(dateText(p.created)).close();
    if (!p.author.isEmpty()) x.open("dc:creator").text(p.author).close();
    x.open("dc:date").text(dateText(p.modified)).close();
    x.open("meta:editing-cycles").text(QStringLiteral("1")).close();
    for (const auto &kv : {qMakePair(QStringLiteral("Manager"), p.manager), qMakePair(QStringLiteral("Company"), p.company), qMakePair(QStringLiteral("Category"), p.category)})
        if (!kv.second.isEmpty()) x.open("meta:user-defined").attr("meta:name", kv.first).text(kv.second).close();
    x.open("meta:document-statistic").attr("meta:page-count", int(m_doc.pages.size())).close();
    x.close().close();
    m_metaXml = (kDeclaration + x.str()).toUtf8();
}

void Exporter::writeManifest()
{
    X x;
    x.open("manifest:manifest").attr("xmlns:manifest", "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0").attr("manifest:version", "1.3");
    x.attr("xmlns:loext", "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0");
    x.open("manifest:file-entry").attr("manifest:full-path", "/").attr("manifest:version", "1.3").attr("manifest:media-type", m_template ? kTemplateType : kPresentationType).close();
    for (const QString &path : m_order) x.open("manifest:file-entry").attr("manifest:full-path", path).attr("manifest:media-type", m_mediaTypes.value(path)).close();
    x.close();
    m_manifestXml = (kDeclaration + x.str()).toUtf8();
}

void Exporter::writeDefaultStyles(X &x) const
{
    // What an object without a style of its own looks like: no fill, no line, and the text of a slide.
    x.open("style:default-style").attr("style:family", "graphic");
    x.open("style:graphic-properties").attr("draw:fill", "none").attr("draw:stroke", "none").attr("draw:shadow", "hidden").attr("fo:wrap-option", "wrap").close();
    x.open("style:paragraph-properties").attr("style:writing-mode", "lr-tb").close();
    x.open("style:text-properties").attr("fo:font-size", "18pt").attr("fo:language", "en").attr("fo:country", "US").close();
    x.close();
    // The standard style the others are based on.
    x.open("style:style").attr("style:name", "standard").attr("style:family", "graphic");
    x.open("style:graphic-properties").attr("draw:fill", "none").attr("draw:stroke", "none").close();
    x.close();
}

void Exporter::writeStyles(const QString &masterBody, const QString &presentationStyles)
{
    X x;
    x.open("office:document-styles").attrs(kNamespaces);
    writeFontDecls(x);
    x.open("office:styles");
    writeDefaultStyles(x);
    m_common.write(x);
    x.raw(presentationStyles);
    // The look of a link.
    {
        const QString link = hexOf(m_theme.colors[10].isValid() ? m_theme.colors[10] : QColor(5, 99, 193));
        x.raw(QStringLiteral("<style:style style:name=\"Internet_20_link\" style:display-name=\"Internet link\" style:family=\"text\"><style:text-properties fo:color=\"%1\" "
                             "style:text-underline-style=\"solid\" style:text-underline-width=\"auto\" style:text-underline-color=\"font-color\"/></style:style>")
                  .arg(link));
        const QString followed = hexOf(m_theme.colors[11].isValid() ? m_theme.colors[11] : QColor(149, 79, 114));
        x.raw(QStringLiteral("<style:style style:name=\"Visited_20_Internet_20_Link\" style:display-name=\"Visited Internet Link\" style:family=\"text\"><style:text-properties fo:color=\"%1\" "
                             "style:text-underline-style=\"solid\" style:text-underline-width=\"auto\" style:text-underline-color=\"font-color\"/></style:style>")
                  .arg(followed));
    }
    x.close();
    x.open("office:automatic-styles");
    for (const QString &layout : std::as_const(m_pageLayoutXml)) x.raw(layout);
    m_masterStyles.write(x);
    x.close();
    x.open("office:master-styles").raw(masterBody).close();
    x.close();
    m_stylesXml = (kDeclaration + x.str()).toUtf8();
}

void Exporter::writeContent(const QString &body)
{
    X x;
    x.open("office:document-content").attrs(kNamespaces);
    x.leaf("office:scripts");
    writeFontDecls(x);
    x.open("office:automatic-styles");
    m_contentStyles.write(x);
    x.close();
    x.open("office:body").raw(body).close();
    x.close();
    m_contentXml = (kDeclaration + x.str()).toUtf8();
}

QByteArray Exporter::run(QString *error)
{
    if (m_doc.pages.isEmpty()) {
        if (error) *error = tr("The presentation has no slides.");
        return QByteArray();
    }
    plan();
    // The master pages first: the styles and names the slides use come out of them.
    QString presentationStyles;
    const QString masterBody = writeMasterPages(&presentationStyles);
    X body;
    writeSlides(body);
    if (!m_doc.pages.isEmpty()) {
        const QByteArray thumb = thumbnail();
        if (!thumb.isEmpty()) addFile(QStringLiteral("Thumbnails/thumbnail.png"), thumb, QStringLiteral("image/png"), false);
    }
    writeContent(body.str());
    writeStyles(masterBody, presentationStyles);
    writeMeta();
    // The manifest lists the parts in the order they are written; the standard parts first.
    const QStringList pictures = m_order;
    m_order = {QStringLiteral("Thumbnails/thumbnail.png"), QStringLiteral("styles.xml"), QStringLiteral("content.xml"), QStringLiteral("meta.xml")};
    for (const QString &p : pictures)
        if (!m_order.contains(p)) m_order << p;
    m_mediaTypes.insert(QStringLiteral("styles.xml"), QStringLiteral("text/xml"));
    m_mediaTypes.insert(QStringLiteral("content.xml"), QStringLiteral("text/xml"));
    m_mediaTypes.insert(QStringLiteral("meta.xml"), QStringLiteral("text/xml"));
    if (!m_files.contains(QStringLiteral("Thumbnails/thumbnail.png"))) m_order.removeAll(QStringLiteral("Thumbnails/thumbnail.png"));
    writeManifest();

    ZipWriter zip;
    zip.add(QStringLiteral("mimetype"), QByteArray(m_template ? kTemplateType : kPresentationType), false);
    zip.add(QStringLiteral("META-INF/manifest.xml"), m_manifestXml, true);
    zip.add(QStringLiteral("content.xml"), m_contentXml, true);
    zip.add(QStringLiteral("styles.xml"), m_stylesXml, true);
    zip.add(QStringLiteral("meta.xml"), m_metaXml, true);
    for (const QString &path : m_order) {
        if (path == QLatin1String("styles.xml") || path == QLatin1String("content.xml") || path == QLatin1String("meta.xml")) continue;
        zip.add(path, m_files.value(path), m_compress.value(path, false));
    }
    const QByteArray bytes = zip.finish();
    if (bytes.isEmpty()) {
        if (error) *error = tr("The presentation file could not be put together.");
        return QByteArray();
    }
    if (m_callerSkipped) *m_callerSkipped += m_skipped;
    return bytes;
}

} // namespace odpw

QByteArray exportOdp(const Document &doc, bool asTemplate, QString *error, QStringList *skipped)
{
    return odpw::Exporter(doc, asTemplate, skipped).run(error);
}

bool saveOdpFile(const Document &doc, const QString &path, QString *error, QStringList *skipped)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    if (path.isEmpty()) return fail(QCoreApplication::translate("OdpExport", "There's no file name to save to."));
    const bool asTemplate = path.endsWith(QLatin1String(".otp"), Qt::CaseInsensitive);
    const QByteArray bytes = exportOdp(doc, asTemplate, error, skipped);
    if (bytes.isEmpty()) return false;
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
