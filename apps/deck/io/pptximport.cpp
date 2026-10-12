#include "io/pptximport.h"

#include "core/appinfo.h"
#include "core/deck.h"
#include "office/drawingml.h"
#include "office/drawingml_table.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "text/textengine.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QTextDocument>
#include <QTextStream>
#include <climits>
#include <deque>
#include <memory>

namespace jp::deck {

namespace {

const QLatin1String kMc("http://schemas.openxmlformats.org/markup-compatibility/2006");

// What a file may hold: more is left unread, with a note.
constexpr int kMaxSlides = 10000, kMaxLayouts = 1000, kMaxMasters = 1000, kMaxShapes = 100000;
// The notes said of a file: more than this are counted, not listed.
constexpr int kMaxNotes = 20;
// A slide may be 56 inches across at most (the standard's limit), and is a point at least.
constexpr qint64 kMinSlideEmu = 12700, kMaxSlideEmu = 51206400;

QString say(const char *text) { return QCoreApplication::translate("PptxImport", text); }

QDomElement pChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::P, QLatin1String(name)); }
QDomElement aChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::A, QLatin1String(name)); }

bool isOneOf(const QString &name, std::initializer_list<const char *> names)
{
    for (const char *n : names)
        if (name == QLatin1String(n)) return true;
    return false;
}

// The name of a part for a note: its file name.
QString shortName(const QString &part) { return part.mid(part.lastIndexOf(QLatin1Char('/')) + 1); }

// What a placeholder says about itself (p:ph).
struct PhRef {
    QDomElement shape;
    QString type = QStringLiteral("obj");
    qint64 idx = 0;
};

bool placeholderOf(const QDomElement &shape, PhRef *out)
{
    for (QDomElement nv = shape.firstChildElement(); !nv.isNull(); nv = nv.nextSiblingElement()) {
        if (!nv.localName().startsWith(QLatin1String("nv"))) continue;
        const QDomElement ph = pChild(pChild(nv, "nvPr"), "ph");
        if (ph.isNull()) return false;
        out->shape = shape;
        out->type = office::attr(ph, QStringLiteral("type"), QStringLiteral("obj"));
        out->idx = std::max<qint64>(0, office::parseInt(office::attr(ph, QStringLiteral("idx")), 0));
        return true;
    }
    return false;
}

// Placeholder types that take each other's places. `wide`: every kind of
// content takes the body's, as a slide master has no other place for it.
QString family(const QString &type, bool wide)
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return QStringLiteral("title");
    if (isOneOf(type, {"body", "subTitle", "obj"})) return QStringLiteral("body");
    if (wide && isOneOf(type, {"pic", "chart", "tbl", "clipArt", "dgm", "media"})) return QStringLiteral("body");
    return type;
}

// The placeholder in `list` that `want` stands on: on the same index and of
// the same kind, else of the same type, else of the same kind. A slide's
// placeholder goes by index to its layout's (`byIdx`); a layout's has only a
// type to go by to its master's.
const PhRef *findPlaceholder(const QVector<PhRef> &list, const PhRef &want, bool byIdx, bool wide)
{
    const QString kind = family(want.type, wide);
    if (byIdx)
        for (const PhRef &p : list)
            if (p.idx == want.idx && family(p.type, wide) == kind) return &p;
    for (const PhRef &p : list)
        if (p.type == want.type) return &p;
    for (const PhRef &p : list)
        if (family(p.type, wide) == kind) return &p;
    return nullptr;
}

QString roleOf(const PhRef &ph)
{
    return ph.idx > INT_MAX ? QStringLiteral("ph:%1:%2").arg(ph.type).arg(ph.idx) : placeholderRole(ph.type, int(ph.idx));
}

// The kinds of placeholder that hold text, and so are text boxes.
bool holdsText(const QString &type) { return !isOneOf(type, {"sldImg", "tbl", "chart", "dgm", "media", "clipArt"}); }

// The picture a graphic frame holds for programs that can't show what it
// really is: an embedded object's preview.
QDomElement previewPicture(const QDomElement &e, int depth = 0)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (office::isElement(c, office::ns::P, QStringLiteral("pic"))) return c;
        if (depth < 8)
            if (const QDomElement found = previewPicture(c, depth + 1); !found.isNull()) return found;
    }
    return QDomElement();
}

QDomElement listStyleOf(const QDomElement &shape) { return aChild(pChild(shape, "txBody"), "lstStyle"); }

QString escaped(const QString &text, bool attribute)
{
    QString out;
    out.reserve(text.size());
    for (const QChar c : text) {
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (attribute && c == QLatin1Char('"')) out += QLatin1String("&quot;");
        else if (attribute && c == QLatin1Char('\n')) out += QLatin1String("&#10;");
        else if (attribute && c == QLatin1Char('\t')) out += QLatin1String("&#9;");
        else out += c;
    }
    return out;
}

QString qualified(const QString &prefix, const QString &local) { return prefix.isEmpty() ? local : prefix + QLatin1Char(':') + local; }

// Every prefix the element and what is in it use, with its namespace.
void collectPrefixes(const QDomElement &e, QHash<QString, QString> *known, int depth = 0)
{
    if (depth > 256) return;
    if (!e.prefix().isEmpty()) known->insert(e.prefix(), e.namespaceURI());
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (!a.prefix().isEmpty()) known->insert(a.prefix(), a.namespaceURI());
    }
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) collectPrefixes(c, known, depth + 1);
}

// Writes the element and what is in it, declaring each namespace where it is
// first used (`scope`: the prefixes declared so far, "" the default). The
// prefixes markup compatibility names in attribute values (mc:Choice's
// Requires, mc:Ignorable) are declared on that element too, from `known`:
// readers look them up there, and a file whose Requires names an undeclared
// prefix is refused whole.
void writeElement(const QDomElement &e, QString *out, QHash<QString, QString> *scope, const QHash<QString, QString> &known)
{
    QVector<QPair<QString, QString>> declared;
    auto need = [&](const QString &prefix, const QString &uri) {
        const auto it = scope->constFind(prefix);
        const QString current = it == scope->cend() ? QString() : it.value();
        if (current == uri) return;
        for (const auto &d : declared)
            if (d.first == prefix) return;
        declared << qMakePair(prefix, uri);
    };
    need(e.prefix(), e.namespaceURI());
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (!a.prefix().isEmpty()) need(a.prefix(), a.namespaceURI());
        const QString name = a.localName();
        if (name == QLatin1String("Requires") || name == QLatin1String("Ignorable") || name == QLatin1String("ProcessContent") || name == QLatin1String("MustUnderstand"))
            for (const QString &listed : a.value().split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                const QString prefix = listed.section(QLatin1Char(':'), 0, 0);   // ProcessContent names prefix:element
                if (known.contains(prefix)) need(prefix, known.value(prefix));
            }
    }
    *out += QLatin1Char('<') + qualified(e.prefix(), e.localName());
    struct Saved {
        QString prefix, uri;
        bool had;
    };
    QVector<Saved> saved;
    for (const auto &d : declared) {
        saved << Saved{d.first, scope->value(d.first), scope->contains(d.first)};
        *out += d.first.isEmpty() ? QStringLiteral(" xmlns=\"%1\"").arg(escaped(d.second, true)) : QStringLiteral(" xmlns:%1=\"%2\"").arg(d.first, escaped(d.second, true));
        scope->insert(d.first, d.second);
    }
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        *out += QLatin1Char(' ') + qualified(a.prefix(), a.localName()) + QLatin1String("=\"") + escaped(a.value(), true) + QLatin1Char('"');
    }
    if (!e.hasChildNodes()) {
        *out += QLatin1String("/>");
    } else {
        *out += QLatin1Char('>');
        for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.isElement()) writeElement(n.toElement(), out, scope, known);
            else if (n.isCharacterData() && !n.isComment()) *out += escaped(n.toCharacterData().data(), false);
        }
        *out += QLatin1String("</") + qualified(e.prefix(), e.localName()) + QLatin1Char('>');
    }
    // The namespaces go back to what they were outside the element.
    for (const Saved &v : saved) {
        if (v.had) scope->insert(v.prefix, v.uri);
        else scope->remove(v.prefix);
    }
}

// The element as XML that can be read by itself: every namespace it uses is
// declared in it (the DOM does not keep the declarations of the elements around it).
QString standaloneXml(const QDomElement &e)
{
    QString out;
    QHash<QString, QString> scope, known;
    collectPrefixes(e, &known);
    writeElement(e, &out, &scope, known);
    return out;
}

// The name the presentation program gives a slide size.
QString sizeNameOf(const QString &type, qint64 cx, qint64 cy)
{
    struct Named { const char *type, *name; };
    static const Named names[] = {
        {"screen4x3", "On-screen Show (4:3)"}, {"screen16x9", "On-screen Show (16:9)"}, {"screen16x10", "On-screen Show (16:10)"},
        {"letter", "Letter Paper"}, {"A4", "A4 Paper"}, {"35mm", "35mm Slides"}, {"overhead", "Overhead"}, {"banner", "Banner"},
        {"ledger", "Ledger Paper"}, {"A3", "A3 Paper"}, {"B4ISO", "B4 (ISO) Paper"}, {"B5ISO", "B5 (ISO) Paper"},
        {"B4JIS", "B4 (JIS) Paper"}, {"B5JIS", "B5 (JIS) Paper"}, {"hagakiCard", "Hagaki Card"}};
    for (const Named &n : names)
        if (type == QLatin1String(n.type)) return QString::fromLatin1(n.name);
    // Widescreen is what files written without a type say for 13.333 by 7.5 inches.
    return type.isEmpty() && cx == 12192000 && cy == 6858000 ? QStringLiteral("Widescreen") : QStringLiteral("Custom");
}

class Importer {
public:
    Importer(const office::Package &pkg, Document &doc) : m_pkg(pkg), m_doc(doc) {}
    bool run(const QString &mainPart, QString *error);

private:
    enum class Kind { Master, Layout, Slide };
    struct MasterCtx {
        std::shared_ptr<MasterPage> page;
        QString part;
        QString themePart;    // its pictures' relationships are the theme part's
        office::Theme theme;
        QDomElement formats;
        office::ColorMap colors;
        QDomElement titleStyle, bodyStyle, otherStyle;
        QVector<PhRef> placeholders;
    };
    struct LayoutCtx {
        std::shared_ptr<MasterPage> page;
        MasterCtx *master = nullptr;
        office::ColorMap colors;
        QVector<PhRef> placeholders;
    };
    // The page being read: where its items go and what it stands on.
    struct Scope {
        Kind kind = Kind::Slide;
        PageBase *page = nullptr;
        office::DrawingContext ctx;
        const MasterCtx *master = nullptr;
        const LayoutCtx *layout = nullptr;
        QJsonArray unread;
        int shapes = 0;
    };

    QDomDocument &newDom()
    {
        m_doms.emplace_back();
        return m_doms.back();
    }
    void note(const QString &text);
    bool parsePart(const QString &part, const QString &root, const char *rootNs, QDomDocument *dom, QDomElement *rootElement, QString *why);
    office::DrawingContext contextFor(const MasterCtx &master, const office::ColorMap &colors, const QString &part);
    office::ColorMap colorsOf(const QDomElement &clrMapOvr, const office::ColorMap &inherited);
    QString addImage(const QByteArray &bytes, const QString &format);

    void readSize(const QDomElement &presentation);
    MasterCtx *readMaster(const QString &part);
    LayoutCtx *readLayout(const QString &part, MasterCtx &master);
    bool readSlide(int number, const QString &part);
    void readNotes(const QString &slidePart, Page &page, const MasterCtx &master);

    QVector<PhRef> placeholdersIn(const QDomElement &tree);
    void readTree(const QDomElement &tree, Scope &sc);
    void readChild(const QDomElement &c, Scope &sc);
    void readAlternate(const QDomElement &alternate, Scope &sc);
    ItemPtr readOne(const QDomElement &shape, Scope &sc);
    bool readFramePicture(const QDomElement &frame, Scope &sc);
    bool readFrameTable(const QDomElement &frame, Scope &sc);
    QDomElement tableStyles();
    void fitRows(TableItem &t);
    void keepUnread(const QDomElement &whole, const QDomElement &frame, Scope &sc);
    QDomElement withEmptyText(const QDomElement &shape);
    QDomElement textStyleFor(const MasterCtx &m, const QString &type) const;

    const office::Package &m_pkg;
    Document &m_doc;
    std::deque<QDomDocument> m_doms;
    QStringList m_notes;
    int m_extraNotes = 0;
    bool m_layoutLimitSaid = false;
    QDomElement m_defaultStyle;
    std::vector<std::unique_ptr<MasterCtx>> m_masters;
    std::vector<std::unique_ptr<LayoutCtx>> m_layouts;
    QHash<QString, LayoutCtx *> m_layoutByPart;   // by lower-cased part name; null for one that couldn't be read
    QHash<QString, QDomElement> m_notesStyles;    // by notes master part
    int m_layoutCount = 0;
    bool m_footers = false;
    QString m_mainPart;
    bool m_tableStylesRead = false;
    QDomElement m_tableStyles;                    // the root of the table styles part, once read; null if there is none
};

void Importer::note(const QString &text)
{
    if (m_notes.size() < kMaxNotes) m_notes << text;
    else ++m_extraNotes;
}

bool Importer::parsePart(const QString &part, const QString &root, const char *rootNs, QDomDocument *dom, QDomElement *rootElement, QString *why)
{
    const QByteArray xml = m_pkg.part(part);
    if (xml.isEmpty()) {
        *why = say("the part is missing from the file");
        return false;
    }
    if (!office::parseXml(xml, dom, why)) return false;
    *rootElement = dom->documentElement();
    if (!office::isElement(*rootElement, QLatin1String(rootNs), root)) {
        *why = say("the part is not what the file says it is");
        return false;
    }
    return true;
}

office::ColorMap Importer::colorsOf(const QDomElement &clrMapOvr, const office::ColorMap &inherited)
{
    const QDomElement over = aChild(clrMapOvr, "overrideClrMapping");
    return over.isNull() ? inherited : office::readColorMap(over);
}

office::DrawingContext Importer::contextFor(const MasterCtx &master, const office::ColorMap &colors, const QString &part)
{
    office::DrawingContext ctx;
    ctx.package = &m_pkg;
    ctx.part = part;
    ctx.theme = master.theme;
    ctx.colors = colors;
    ctx.formats = master.formats;
    ctx.themePart = master.themePart;
    ctx.addImage = [this](const QByteArray &bytes, const QString &format) { return addImage(bytes, format); };
    ctx.newStory = [this] { return m_doc.createStory(); };
    ctx.storyDoc = [this](const QString &id) { return m_doc.storyDoc(id); };
    return ctx;
}

QString Importer::addImage(const QByteArray &bytes, const QString &format)
{
    return m_doc.addImage(bytes, format);
}

void Importer::readSize(const QDomElement &presentation)
{
    const QDomElement sz = pChild(presentation, "sldSz");
    const qint64 cx = office::parseInt(office::attr(sz, QStringLiteral("cx"))), cy = office::parseInt(office::attr(sz, QStringLiteral("cy")));
    QSizeF size(kSlideWidth, kSlideHeight);
    QString name = QStringLiteral("Widescreen");
    if (!sz.isNull()) {
        if (cx >= kMinSlideEmu && cx <= kMaxSlideEmu && cy >= kMinSlideEmu && cy <= kMaxSlideEmu) {
            size = QSizeF(office::emuToPoints(cx), office::emuToPoints(cy));
            name = sizeNameOf(office::attr(sz, QStringLiteral("type")), cx, cy);
        } else {
            note(say("The slide size in the file can't be used, so the slides are 13.333 by 7.5 inches."));
        }
    }
    m_doc.setup.size = m_doc.setup.sheet = size;
    m_doc.setup.sizeName = name;
    m_doc.setup.margins = QMarginsF(0, 0, 0, 0);
    m_doc.setup.firstPageNumber = int(std::clamp<qint64>(office::parseInt(office::attr(presentation, QStringLiteral("firstSlideNum")), 1), 0, 9999));
    const QDomElement notesSz = pChild(presentation, "notesSz");
    const qint64 nx = office::parseInt(office::attr(notesSz, QStringLiteral("cx"))), ny = office::parseInt(office::attr(notesSz, QStringLiteral("cy")));
    if (nx > 0 && ny > 0)
        m_doc.appData[QStringLiteral("notesSize")] = QJsonObject{{"w", office::emuToPoints(nx)}, {"h", office::emuToPoints(ny)}};
}

QVector<PhRef> Importer::placeholdersIn(const QDomElement &tree)
{
    QVector<PhRef> out;
    for (QDomElement c = tree.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        PhRef ph;
        if (isOneOf(c.localName(), {"sp", "pic", "graphicFrame"}) && placeholderOf(c, &ph)) out << ph;
    }
    return out;
}

QDomElement Importer::textStyleFor(const MasterCtx &m, const QString &type) const
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return m.titleStyle;
    if (family(type, true) == QLatin1String("body")) return m.bodyStyle;
    return m.otherStyle;
}

Importer::MasterCtx *Importer::readMaster(const QString &part)
{
    QDomDocument &dom = newDom();
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("sldMaster"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(say("The slide master %1 was left out: %2").arg(shortName(part), why));
        return nullptr;
    }
    auto m = std::make_unique<MasterCtx>();
    m->part = part;
    m->theme = office::defaultTheme();
    const QVector<office::Relationship> themes = m_pkg.relationshipsOfType(part, QStringLiteral("/theme"));
    QByteArray themeBytes;
    if (themes.isEmpty() || themes[0].external) {
        note(say("The slide master %1 has no theme, so JeffDeck's colors and fonts stand in.").arg(shortName(part)));
    } else {
        themeBytes = m_pkg.part(themes[0].target);
        QString themeWhy;
        QDomDocument &themeDom = newDom();
        if (!office::readTheme(themeBytes, &m->theme, &themeWhy) || !office::parseXml(themeBytes, &themeDom, &themeWhy)) {
            note(say("The theme of slide master %1 can't be read (%2), so JeffDeck's colors and fonts stand in.").arg(shortName(part), themeWhy));
            m->theme = office::defaultTheme();
            themeBytes.clear();
        } else {
            m->formats = aChild(aChild(themeDom.documentElement(), "themeElements"), "fmtScheme");
            m->themePart = themes[0].target;
        }
    }
    m->colors = office::readColorMap(pChild(root, "clrMap"));
    const QDomElement styles = pChild(root, "txStyles");
    m->titleStyle = pChild(styles, "titleStyle");
    m->bodyStyle = pChild(styles, "bodyStyle");
    m->otherStyle = pChild(styles, "otherStyle");

    const int n = int(m_masters.size()) + 1;
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("SM%1").arg(n);
    const QDomElement cSld = pChild(root, "cSld");
    page->name = office::attr(cSld, QStringLiteral("name"));
    if (page->name.isEmpty()) page->name = m->theme.name.isEmpty() ? say("Slide Master") : say("%1 Slide Master").arg(m->theme.name);
    page->abbr = n == 1 ? QStringLiteral("SM") : QStringLiteral("SM%1").arg(n);
    page->props[QStringLiteral("kind")] = QStringLiteral("master");
    if (n > 1 && !themeBytes.isEmpty()) page->props[QStringLiteral("themeXml")] = QString::fromLatin1(themeBytes.toBase64());
    m->page = page;

    Scope sc;
    sc.kind = Kind::Master;
    sc.page = page.get();
    sc.master = m.get();
    sc.ctx = contextFor(*m, m->colors, part);
    if (const QDomElement bg = pChild(cSld, "bg"); !bg.isNull()) page->background = office::readBackground(bg, sc.ctx);
    const QDomElement tree = pChild(cSld, "spTree");
    m->placeholders = placeholdersIn(tree);
    readTree(tree, sc);

    MasterCtx *raw = m.get();
    m_masters.push_back(std::move(m));
    m_doc.masters << page;
    if (m_masters.size() == 1) {
        // The first master's theme is the presentation's.
        DeckTheme t = defaultTheme();
        t.name = raw->theme.name.isEmpty() ? say("Theme") : raw->theme.name;
        for (int i = 0; i < 12; ++i) t.colors[i] = raw->theme.colors[i];
        if (!raw->theme.majorLatin.isEmpty()) t.headingFont = raw->theme.majorLatin;
        if (!raw->theme.minorLatin.isEmpty()) t.bodyFont = raw->theme.minorLatin;
        setTheme(m_doc, t);
        m_doc.fonts = {t.name, t.headingFont, t.bodyFont};
        if (!themeBytes.isEmpty()) m_doc.appData[QStringLiteral("themeXml")] = QString::fromLatin1(themeBytes.toBase64());
    }

    // Its layouts, in the order it lists them, right after it.
    for (const QDomElement &id : office::children(pChild(root, "sldLayoutIdLst"), office::ns::P, QStringLiteral("sldLayoutId"))) {
        const office::Relationship rel = m_pkg.relationship(part, office::attrNs(id, office::ns::R, QStringLiteral("id")));
        if (rel.id.isEmpty() || rel.external) continue;
        readLayout(rel.target, *raw);
    }
    return raw;
}

Importer::LayoutCtx *Importer::readLayout(const QString &part, MasterCtx &master)
{
    const QString key = part.toLower();
    if (m_layoutByPart.contains(key)) return m_layoutByPart.value(key);
    if (m_layoutCount >= kMaxLayouts) {
        if (!m_layoutLimitSaid) note(say("Only the first %1 layouts are read.").arg(kMaxLayouts));
        m_layoutLimitSaid = true;
        return nullptr;
    }
    m_layoutByPart.insert(key, nullptr);
    QDomDocument &dom = newDom();
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("sldLayout"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(say("The layout %1 was left out: %2").arg(shortName(part), why));
        return nullptr;
    }
    // A layout stands on the master that lists it, unless its own link names another that was read.
    MasterCtx *base = &master;
    const QVector<office::Relationship> own = m_pkg.relationshipsOfType(part, QStringLiteral("/slideMaster"));
    if (!own.isEmpty())
        for (const auto &m : m_masters)
            if (m->part.compare(own[0].target, Qt::CaseInsensitive) == 0) base = m.get();

    auto l = std::make_unique<LayoutCtx>();
    l->master = base;
    l->colors = colorsOf(pChild(root, "clrMapOvr"), base->colors);
    const int n = ++m_layoutCount;
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("L%1").arg(n);
    const QDomElement cSld = pChild(root, "cSld");
    page->name = office::attr(cSld, QStringLiteral("name"));
    if (page->name.isEmpty()) page->name = say("Layout %1").arg(n);
    page->abbr = QString::number(n);
    page->baseId = base->page->id;
    page->showMasterItems = office::parseBool(office::attr(root, QStringLiteral("showMasterSp")), true);
    page->props[QStringLiteral("kind")] = QStringLiteral("layout");
    page->props[QStringLiteral("type")] = office::attr(root, QStringLiteral("type"), QStringLiteral("cust"));
    l->page = page;

    Scope sc;
    sc.kind = Kind::Layout;
    sc.page = page.get();
    sc.master = base;
    sc.ctx = contextFor(*base, l->colors, part);
    if (const QDomElement bg = pChild(cSld, "bg"); !bg.isNull()) page->background = office::readBackground(bg, sc.ctx);
    const QDomElement tree = pChild(cSld, "spTree");
    l->placeholders = placeholdersIn(tree);
    readTree(tree, sc);

    LayoutCtx *raw = l.get();
    m_layouts.push_back(std::move(l));
    m_layoutByPart.insert(key, raw);
    m_doc.masters << page;
    return raw;
}

void Importer::readTree(const QDomElement &tree, Scope &sc)
{
    for (QDomElement c = tree.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (++sc.shapes > kMaxShapes) {
            note(say("A page holds more than %1 objects; the rest are left out.").arg(kMaxShapes));
            break;
        }
        readChild(c, sc);
    }
    if (!sc.unread.isEmpty()) sc.page->props[QStringLiteral("unread")] = sc.unread;
}

void Importer::readChild(const QDomElement &c, Scope &sc)
{
    const QString name = c.localName();
    if (office::isElement(c, kMc, QStringLiteral("AlternateContent"))) {
        readAlternate(c, sc);
    } else if (name == QLatin1String("graphicFrame") || name == QLatin1String("contentPart")) {
        if (!readFrameTable(c, sc) && !readFramePicture(c, sc)) keepUnread(c, c, sc);
    } else if (isOneOf(name, {"sp", "pic", "grpSp", "cxnSp"})) {
        if (ItemPtr it = readOne(c, sc)) sc.page->items.push_back(it);
    }
}

// Alternatives: what the reader can use of the choices, else a picture in the
// fallback, else the whole is kept as written.
void Importer::readAlternate(const QDomElement &alternate, Scope &sc)
{
    QDomElement fallback, frame;
    bool read = false;
    for (QDomElement alt = alternate.firstChildElement(); !alt.isNull() && !read; alt = alt.nextSiblingElement()) {
        if (office::isElement(alt, kMc, QStringLiteral("Fallback"))) {
            if (fallback.isNull()) fallback = alt;
            continue;
        }
        if (!office::isElement(alt, kMc, QStringLiteral("Choice"))) continue;
        for (QDomElement e = alt.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (isOneOf(e.localName(), {"graphicFrame", "contentPart"})) {
                if (readFrameTable(e, sc) || readFramePicture(e, sc)) read = true;
                else if (frame.isNull()) frame = e;
            } else if (isOneOf(e.localName(), {"sp", "pic", "grpSp", "cxnSp"})) {
                read = true;
                if (ItemPtr it = readOne(e, sc)) sc.page->items.push_back(it);
            }
        }
    }
    if (read) return;
    for (QDomElement e = fallback.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.localName() != QLatin1String("pic")) continue;
        if (ItemPtr it = readOne(e, sc)) {
            sc.page->items.push_back(it);
            read = true;
        }
    }
    if (!read) keepUnread(alternate, frame.isNull() ? alternate : frame, sc);
}

// The table styles part (a:tblStyleLst), read once; null if the presentation has none.
QDomElement Importer::tableStyles()
{
    if (m_tableStylesRead) return m_tableStyles;
    m_tableStylesRead = true;
    const QVector<office::Relationship> rels = m_pkg.relationshipsOfType(m_mainPart, QStringLiteral("/tableStyles"));
    if (rels.isEmpty() || rels[0].external) return m_tableStyles;
    QDomDocument &dom = newDom();
    QString why;
    if (!parsePart(rels[0].target, QStringLiteral("tblStyleLst"), "http://schemas.openxmlformats.org/drawingml/2006/main", &dom, &m_tableStyles, &why)) {
        m_tableStyles = QDomElement();
        note(say("The table styles of the presentation can't be read (%1), so its tables take the usual style.").arg(why));
    }
    return m_tableStyles;
}

// A frame that holds a table: the table, in the place of the frame.
bool Importer::readFrameTable(const QDomElement &frame, Scope &sc)
{
    if (!office::isTableFrame(frame)) return false;
    office::TextStyleStack styles;
    if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
    ItemPtr it = office::readTable(frame, sc.ctx, styles, tableStyles());
    if (!it) return false;
    fitRows(static_cast<TableItem &>(*it));
    PhRef ph;
    if (placeholderOf(frame, &ph)) it->role = roleOf(ph);
    sc.page->items.push_back(it);
    return true;
}

// The height a row has in the file is the least it can be: it grows to hold the
// text of its cells (as Editor::fitTableRows does as text is typed), and the
// table with it. Cells that span rows, or turn their text, don't make it grow.
void Importer::fitRows(TableItem &t)
{
    if (!t.growToFit) return;
    LayoutEnv env;
    env.colors = m_doc.colors;
    env.fonts = m_doc.fonts;
    env.defaultTab = m_doc.defaultTabStop;
    bool grew = false;
    for (int r = 0; r < t.rows; ++r) {
        double need = 0;
        for (int c = 0; c < t.cols; ++c) {
            const TableCell &cell = t.cell(r, c);
            const QTextDocument *story = cell.covered || cell.rowSpan > 1 || cell.vertical || cell.divided() ? nullptr : m_doc.storyDoc(cell.storyId);
            if (!story) continue;
            FrameSpec spec;
            spec.size = QSizeF(t.cellRect(r, c).width(), 1e5);
            spec.insets = cell.margins;
            spec.hyphenate = cell.hyphenate;
            spec.hyphenZone = cell.hyphenZone;
            StoryLayout layout;
            layout.build(story, {spec}, env);
            need = std::max(need, layout.usedHeight(0));
        }
        if (need > t.rowH[r]) {
            t.rowH[r] = need;
            grew = true;
        }
    }
    if (grew) t.syncRect();
}

// What a frame shows for itself, when it holds a picture of it.
bool Importer::readFramePicture(const QDomElement &frame, Scope &sc)
{
    const QDomElement pic = previewPicture(aChild(frame, "graphic"));
    if (pic.isNull()) return false;
    ItemPtr it = readOne(pic, sc);
    if (!it) return false;
    sc.page->items.push_back(it);
    return true;
}

void Importer::keepUnread(const QDomElement &whole, const QDomElement &frame, Scope &sc)
{
    QDomElement xfrm = pChild(frame, "xfrm");
    if (xfrm.isNull()) xfrm = aChild(frame, "xfrm");
    const QDomElement off = aChild(xfrm, "off"), ext = aChild(xfrm, "ext");
    auto points = [](const QDomElement &e, const char *name) { return office::emuToPoints(office::parseInt(office::attr(e, QLatin1String(name)))); };
    sc.unread.append(QJsonObject{{"xml", standaloneXml(whole)},
                                 {"x", points(off, "x")},
                                 {"y", points(off, "y")},
                                 {"w", points(ext, "cx")},
                                 {"h", points(ext, "cy")}});
}

// A placeholder that has no text body yet, with one that is empty, so it can
// be typed in.
QDomElement Importer::withEmptyText(const QDomElement &shape)
{
    QDomDocument dom = shape.ownerDocument();
    QDomElement copy = shape.cloneNode(true).toElement();
    const bool strict = shape.namespaceURI() == office::ns::PStrict;
    const QString p = strict ? office::ns::PStrict : office::ns::P, a = strict ? office::ns::AStrict : office::ns::A;
    QDomElement body = dom.createElementNS(p, QStringLiteral("p:txBody"));
    body.appendChild(dom.createElementNS(a, QStringLiteral("a:bodyPr")));
    body.appendChild(dom.createElementNS(a, QStringLiteral("a:lstStyle")));
    body.appendChild(dom.createElementNS(a, QStringLiteral("a:p")));
    copy.appendChild(body);
    return copy;
}

ItemPtr Importer::readOne(const QDomElement &shape, Scope &sc)
{
    office::TextStyleStack styles;
    if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
    PhRef ph;
    if (!placeholderOf(shape, &ph)) return office::readShape(shape, sc.ctx, styles);

    // Where it gets its place and its text formatting from: its layout's
    // placeholder, and that one's (else its own) on the master.
    const PhRef *layoutPh = nullptr, *masterPh = nullptr;
    if (sc.kind == Kind::Slide && sc.layout) layoutPh = findPlaceholder(sc.layout->placeholders, ph, true, false);
    if (sc.kind != Kind::Master && sc.master) masterPh = findPlaceholder(sc.master->placeholders, layoutPh ? *layoutPh : ph, false, true);
    QVector<QDomElement> inherits;
    if (layoutPh) inherits << layoutPh->shape;
    if (masterPh) inherits << masterPh->shape;
    if (sc.master) {
        if (const QDomElement base = textStyleFor(*sc.master, ph.type); !base.isNull()) styles << base;
    }
    for (const PhRef *from : {masterPh, layoutPh})
        if (from)
            if (const QDomElement lst = listStyleOf(from->shape); !lst.isNull()) styles << lst;
    if (const QDomElement lst = listStyleOf(shape); !lst.isNull()) styles << lst;

    const bool text = shape.localName() == QLatin1String("sp") && holdsText(ph.type) && pChild(shape, "txBody").isNull();
    ItemPtr it = office::readShape(text ? withEmptyText(shape) : shape, sc.ctx, styles, inherits);
    if (!it) return nullptr;
    it->role = roleOf(ph);
    if (sc.kind == Kind::Slide) {
        if (isOneOf(ph.type, {"dt", "ftr", "sldNum"})) m_footers = true;
        if (it->type() == ItemType::Text) {
            auto *t = static_cast<TextItem *>(it.get());
            const QTextDocument *story = m_doc.storyDoc(t->storyId);
            if (story && story->isEmpty()) t->prompt = promptFor(ph.type);
        }
    }
    return it;
}

bool Importer::readSlide(int number, const QString &part)
{
    QDomDocument dom;
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("sld"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(say("Slide %1 was left out: %2").arg(number).arg(why));
        return false;
    }
    // The layout it stands on, and through it the slide master. One that no
    // master lists is read now, on the master its own link names.
    LayoutCtx *layout = nullptr;
    const QVector<office::Relationship> layoutRels = m_pkg.relationshipsOfType(part, QStringLiteral("/slideLayout"));
    if (!layoutRels.isEmpty() && !layoutRels[0].external) {
        const QString target = layoutRels[0].target;
        layout = m_layoutByPart.value(target.toLower());
        if (!layout && !m_layoutByPart.contains(target.toLower())) {
            MasterCtx *base = m_masters.front().get();
            const QVector<office::Relationship> masterRels = m_pkg.relationshipsOfType(target, QStringLiteral("/slideMaster"));
            if (!masterRels.isEmpty())
                for (const auto &m : m_masters)
                    if (m->part.compare(masterRels[0].target, Qt::CaseInsensitive) == 0) base = m.get();
            layout = readLayout(target, *base);
        }
    }
    const MasterCtx &master = layout ? *layout->master : *m_masters.front();

    auto page = std::make_shared<Page>();
    page->masterId = layout ? layout->page->id : master.page->id;
    page->hidden = !office::parseBool(office::attr(root, QStringLiteral("show")), true);
    page->showMasterItems = office::parseBool(office::attr(root, QStringLiteral("showMasterSp")), true);

    Scope sc;
    sc.kind = Kind::Slide;
    sc.page = page.get();
    sc.master = &master;
    sc.layout = layout;
    sc.ctx = contextFor(master, colorsOf(pChild(root, "clrMapOvr"), layout ? layout->colors : master.colors), part);
    const QDomElement cSld = pChild(root, "cSld");
    if (const QDomElement bg = pChild(cSld, "bg"); !bg.isNull()) page->background = office::readBackground(bg, sc.ctx);
    readTree(pChild(cSld, "spTree"), sc);

    // How the slide changes to the next, and what moves on it, as written.
    for (const QDomElement &c : office::children(root)) {
        if (office::isElement(c, office::ns::P, QStringLiteral("transition"))) {
            page->props[QStringLiteral("transitionXml")] = standaloneXml(c);
        } else if (office::isElement(c, office::ns::P, QStringLiteral("timing"))) {
            page->props[QStringLiteral("timingXml")] = standaloneXml(c);
        } else if (office::isElement(c, kMc, QStringLiteral("AlternateContent"))) {
            // A transition with another form for programs that don't know the first.
            for (const QDomElement &alt : office::children(c))
                if (!pChild(alt, "transition").isNull()) page->props[QStringLiteral("transitionXml")] = standaloneXml(c);
        }
    }
    for (const auto &it : page->items) {
        const QString type = placeholderType(*it);
        if ((type == QLatin1String("title") || type == QLatin1String("ctrTitle")) && it->type() == ItemType::Text) {
            const QTextDocument *story = m_doc.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
            if (story) page->title = story->toPlainText().section(QChar(QChar::LineSeparator), 0, 0).section(QLatin1Char('\n'), 0, 0).trimmed();
            break;
        }
    }
    readNotes(part, *page, master);
    m_doc.pages << page;
    return true;
}

void Importer::readNotes(const QString &slidePart, Page &page, const MasterCtx &master)
{
    page.notesStoryId = m_doc.createStory();
    const QVector<office::Relationship> rels = m_pkg.relationshipsOfType(slidePart, QStringLiteral("/notesSlide"));
    if (rels.isEmpty() || rels[0].external) return;
    const QString part = rels[0].target;
    QDomDocument dom;
    QDomElement root;
    QString why;
    if (!parsePart(part, QStringLiteral("notes"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why)) {
        note(say("The notes of %1 can't be read: %2").arg(shortName(slidePart), why));
        return;
    }
    // The text of the notes placeholder, in the notes master's style.
    office::TextStyleStack styles;
    if (!m_defaultStyle.isNull()) styles << m_defaultStyle;
    const QVector<office::Relationship> masters = m_pkg.relationshipsOfType(part, QStringLiteral("/notesMaster"));
    if (!masters.isEmpty() && !masters[0].external) {
        const QString key = masters[0].target.toLower();
        if (!m_notesStyles.contains(key)) {
            QDomDocument &masterDom = newDom();
            QDomElement masterRoot;
            QString masterWhy;
            m_notesStyles.insert(key, parsePart(masters[0].target, QStringLiteral("notesMaster"), "http://schemas.openxmlformats.org/presentationml/2006/main", &masterDom, &masterRoot, &masterWhy)
                                          ? pChild(masterRoot, "notesStyle") : QDomElement());
        }
        if (const QDomElement style = m_notesStyles.value(key); !style.isNull()) styles << style;
    }
    const QDomElement tree = pChild(pChild(root, "cSld"), "spTree");
    for (const QDomElement &sp : office::children(tree, office::ns::P, QStringLiteral("sp"))) {
        PhRef ph;
        if (!placeholderOf(sp, &ph) || ph.type != QLatin1String("body")) continue;
        const QDomElement body = pChild(sp, "txBody");
        QTextDocument *story = m_doc.storyDoc(page.notesStoryId);
        if (body.isNull() || !story) break;
        if (const QDomElement lst = listStyleOf(sp); !lst.isNull()) styles << lst;
        office::readTextBody(body, story, contextFor(master, master.colors, part), styles);
        break;
    }
}

bool Importer::run(const QString &mainPart, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    QDomDocument &dom = newDom();
    QDomElement root;
    QString why;
    if (!parsePart(mainPart, QStringLiteral("presentation"), "http://schemas.openxmlformats.org/presentationml/2006/main", &dom, &root, &why))
        return fail(say("The file has no presentation in it: %1").arg(why));
    m_doc.masters.clear();
    m_mainPart = mainPart;
    readSize(root);
    m_defaultStyle = pChild(root, "defaultTextStyle");

    for (const QDomElement &id : office::children(pChild(root, "sldMasterIdLst"), office::ns::P, QStringLiteral("sldMasterId"))) {
        if (m_masters.size() >= kMaxMasters) {
            note(say("Only the first %1 slide masters are read.").arg(kMaxMasters));
            break;
        }
        const office::Relationship rel = m_pkg.relationship(mainPart, office::attrNs(id, office::ns::R, QStringLiteral("id")));
        if (rel.id.isEmpty() || rel.external) continue;
        readMaster(rel.target);
    }
    if (m_masters.empty()) return fail(say("The presentation has no slide master that can be read.") + QLatin1Char('\n') + m_notes.join(QLatin1Char('\n')));

    int number = 0;
    for (const QDomElement &id : office::children(pChild(root, "sldIdLst"), office::ns::P, QStringLiteral("sldId"))) {
        if (++number > kMaxSlides) {
            note(say("Only the first %1 slides are read.").arg(kMaxSlides));
            break;
        }
        const office::Relationship rel = m_pkg.relationship(mainPart, office::attrNs(id, office::ns::R, QStringLiteral("id")));
        if (rel.id.isEmpty() || rel.external) {
            note(say("Slide %1 was left out: the file names no part for it.").arg(number));
            continue;
        }
        readSlide(number, rel.target);
    }
    m_doc.appData[QStringLiteral("footers")] = m_footers;
    if (error) {
        QStringList lines = m_notes;
        if (m_extraNotes) lines << say("And %1 more.").arg(m_extraNotes);
        *error = lines.join(QLatin1Char('\n'));
    }
    return true;
}

std::unique_ptr<Document> importPackage(const office::Package &pkg, QString *error)
{
    auto doc = std::make_unique<Document>();
    Importer importer(pkg, *doc);
    if (!importer.run(pkg.mainPart(), error)) return nullptr;
    doc->fromFile = true;   // what the file names outside itself is read only from beside it
    return doc;
}

// An older presentation is a compound file, as a package protected with a password is.
bool looksLikeCompoundFile(const QByteArray &bytes) { return bytes.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1")); }

QString olderFormat()
{
    return QCoreApplication::translate("PptxImport", "This file is in an older format, or is protected with a password, which %1 can't open yet.").arg(appInfo().name);
}

} // namespace

std::unique_ptr<Document> importPresentation(const QByteArray &bytes, QString *error)
{
    if (looksLikeCompoundFile(bytes)) {
        if (error) *error = olderFormat();
        return nullptr;
    }
    office::Package pkg;
    if (!pkg.read(bytes, error)) return nullptr;
    return importPackage(pkg, error);
}

std::unique_ptr<Document> importPresentationFile(const QString &path, QString *error)
{
    office::Package pkg;
    if (!pkg.readFile(path, error)) {
        // The compound file's magic is the first thing in the file.
        QFile f(path);
        if (f.open(QIODevice::ReadOnly) && looksLikeCompoundFile(f.read(8)) && error) *error = olderFormat();
        return nullptr;
    }
    return importPackage(pkg, error);
}

} // namespace jp::deck
