#include "io/odpimport.h"

#include "io/odpimport_p.h"
#include "io/presentationfile.h"

#include "core/deck.h"
#include "core/embeddedfonts.h"
#include "core/transition.h"
#include "io/pptxfonts.h"
#include "office/drawingml_style.h"
#include "office/theme.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <cmath>

namespace jp::deck::odp {

namespace {

constexpr int kMaxSlides = 10000, kMaxMasters = 1000, kMaxLayouts = 1000, kMaxNotes = 20;
// A slide may be 56 inches across at most, and is a point at least.
constexpr double kMinSlide = 1, kMaxSlide = 4032;


const QString kTitleSample = QStringLiteral("Click to edit Master title style");
const QString kSubtitleSample = QStringLiteral("Click to edit Master subtitle style");
const QString kBodySample = QStringLiteral("Click to edit Master text styles");

QString sampleFor(const QString &type)
{
    if (type == QLatin1String("title") || type == QLatin1String("ctrTitle")) return kTitleSample;
    if (type == QLatin1String("subTitle")) return kSubtitleSample;
    if (type == QLatin1String("body") || type == QLatin1String("obj")) return kBodySample;
    return QString();
}

bool isFooter(const QString &type) { return type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum"); }
bool isContent(const QString &type) { return type == QLatin1String("obj") || type == QLatin1String("body") || type == QLatin1String("pic"); }

// What a page's background is to text on it: the color of a solid fill, the first of a gradient's, else white.
QColor colorBehind(const Fill &f)
{
    if (f.type == Fill::Solid && f.color.kind() == ColorRef::Rgb) return f.color.rgbValue();
    if (f.type == Fill::Gradient && !f.stops.isEmpty() && f.stops.first().color.kind() == ColorRef::Rgb) return f.stops.first().color.rgbValue();
    return QColor(Qt::white);
}

// A story's text replaced by a prompt, in the formatting of its first letter (and its first bullet).
void makePrompt(Document &d, TextItem &t, const QString &sample)
{
    QTextDocument *sd = d.storyDoc(t.storyId);
    if (!sd) return;
    QTextCursor first(sd);
    const QTextCharFormat cf = first.charFormat();
    const QTextBlockFormat bf = first.blockFormat();
    QTextListFormat lf;
    bool listed = false;
    if (QTextList *list = first.currentList()) {
        lf = list->format();
        listed = true;
    }
    sd->clear();
    QTextCursor c(sd);
    c.setBlockFormat(bf);
    c.setBlockCharFormat(cf);
    if (listed) c.createList(lf);
    c.insertText(sample, cf);
    t.prompt.clear();
}

} // namespace

Reader::Reader(const office::Package &package, Document &doc, QStringList *notes) : m_pkg(package), m_doc(doc), m_notes(notes) {}

void Reader::note(const QString &text)
{
    if (!m_notes || m_notes->contains(text)) return;
    if (m_notes->size() < kMaxNotes) m_notes->append(text);
}

bool Reader::parse(const QString &part, QDomDocument *dom, QString *why)
{
    const QByteArray xml = m_pkg.part(part);
    if (xml.isEmpty()) {
        *why = QCoreApplication::translate("OdpImport", "The part is missing from the file.");
        return false;
    }
    return office::parseXml(xml, dom, why);
}

office::DrawingContext Reader::drawingContext()
{
    office::DrawingContext ctx;
    ctx.package = &m_links;
    ctx.part = QStringLiteral("/links.xml");
    ctx.theme = office::defaultTheme();
    ctx.addImage = [this](const QByteArray &bytes, const QString &format) { return m_doc.addImage(bytes, format); };
    ctx.addMedia = [this](const QByteArray &bytes, const QString &contentType, const QString &extension) { return m_doc.addMedia(bytes, contentType, extension); };
    ctx.newStory = [this] { return m_doc.createStory(); };
    ctx.storyDoc = [this](const QString &id) { return m_doc.storyDoc(id); };
    return ctx;
}

// ---- the document

// The addresses text links to, as relationships of a part of ours: text is read by the shared reader, which names a link by one.
void Reader::buildLinks()
{
    office::PackageWriter w;
    w.addPart(QStringLiteral("/links.xml"), "<links/>", QStringLiteral("application/xml"));
    int count = 0;
    for (QDomDocument *d : {&m_contentDom, &m_stylesDom}) {
        const QDomNodeList list = d->elementsByTagNameNS(QStringLiteral("urn:oasis:names:tc:opendocument:xmlns:text:1.0"), QStringLiteral("a"));
        for (int i = 0; i < list.size() && count < 100000; ++i) {
            const QString href = list.item(i).toElement().attributeNS(QStringLiteral("http://www.w3.org/1999/xlink"), QStringLiteral("href"));
            if (!isWebLink(href) || m_linkIds.contains(href)) continue;
            m_linkIds.insert(href, w.addRelationship(QStringLiteral("/links.xml"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink"), href, true));
            ++count;
        }
    }
    m_links.read(w.finish());
}

void Reader::readMeta()
{
    if (m_meta.isNull()) return;
    const QDomElement meta = kid(m_meta, "office:meta");
    auto text = [&](const char *name) { return kid(meta, name).text().trimmed(); };
    DocProps &p = m_doc.props;
    p.title = text("dc:title");
    p.subject = text("dc:subject");
    p.author = !text("meta:initial-creator").isEmpty() ? text("meta:initial-creator") : text("dc:creator");
    p.comments = text("dc:description");
    QStringList keywords;
    for (const QDomElement &k : kids(meta, "meta:keyword")) keywords << k.text().trimmed();
    p.keywords = keywords.join(QStringLiteral(", "));
    const QDateTime created = QDateTime::fromString(text("meta:creation-date"), Qt::ISODate), modified = QDateTime::fromString(text("dc:date"), Qt::ISODate);
    if (created.isValid()) p.created = created;
    if (modified.isValid()) p.modified = modified;
}

Reader::MasterInfo *Reader::masterByName(const QString &name)
{
    for (auto &m : m_masters)
        if (m->name == name) return m.get();
    return m_masters.empty() ? nullptr : m_masters.front().get();
}

bool Reader::readSize()
{
    // The slide size is the first slide's: the page layout of its master page.
    MasterInfo *first = nullptr;
    const QDomElement body = kid(m_content, "office:body");
    m_presentation = kid(body, "office:presentation");
    if (const QDomElement page = kid(m_presentation, "draw:page"); !page.isNull()) first = masterByName(at(page, "draw:master-page-name"));
    if (!first && !m_masters.empty()) first = m_masters.front().get();
    QSizeF size(kSlideWidth, kSlideHeight);
    bool known = false;
    if (first) {
        const QDomElement layout = m_masterSheet->pageLayout(first->layoutName);
        const QDomElement props = kid(layout, "style:page-layout-properties");
        double w = 0, h = 0;
        if (!props.isNull() && parseLength(at(props, "fo:page-width"), &w) && parseLength(at(props, "fo:page-height"), &h) && w >= kMinSlide && h >= kMinSlide && w <= kMaxSlide && h <= kMaxSlide) {
            size = QSizeF(w, h);
            known = true;
        }
    }
    if (!known) note(QCoreApplication::translate("OdpImport", "The slide size in the file can't be used, so the slides are 13.333 by 7.5 inches."));
    m_slideSize = size;
    m_doc.setup.size = m_doc.setup.sheet = size;
    m_doc.setup.sizeName = slideSizeName(size);
    m_doc.setup.margins = QMarginsF(0, 0, 0, 0);
    // The notes pages' size: the page layout of the notes in the master page.
    if (first) {
        const QDomElement notes = kid(first->element, "presentation:notes");
        const QDomElement props = kid(m_masterSheet->pageLayout(at(notes, "style:page-layout-name")), "style:page-layout-properties");
        double w = 0, h = 0;
        if (!props.isNull() && parseLength(at(props, "fo:page-width"), &w) && parseLength(at(props, "fo:page-height"), &h) && w >= kMinSlide && h >= kMinSlide)
            m_doc.appData[QStringLiteral("notesSize")] = QJsonObject{{"w", w}, {"h", h}};
    }
    return true;
}

// ---- masters and layouts

void Reader::readMasterPage(MasterInfo &m, int number)
{
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("SM%1").arg(number);
    page->name = !m.displayName.isEmpty() ? m.displayName : !m.name.isEmpty() ? m.name : QCoreApplication::translate("OdpImport", "Slide Master");
    page->abbr = number == 1 ? QStringLiteral("SM") : QStringLiteral("SM%1").arg(number);
    page->props[QStringLiteral("kind")] = QStringLiteral("master");
    m.page = page;
    // Its background: the fill of its page style.
    const Bag bg = m_masterSheet->resolve(QStringLiteral("drawing-page"), at(m.element, "draw:style-name"));
    page->background = fillOf(bg, *m_masterSheet);
    m.backgroundColor = colorBehind(page->background);
    m_pageFill = page->background;
    Scope sc;
    sc.background = m.backgroundColor;
    sc.kind = Scope::Master;
    sc.styles = m_masterSheet.get();
    sc.page = page.get();
    sc.master = &m;
    readShapes(m.element, sc, page->items, QTransform());
    // The connectors among its objects.
    for (const Scope::Pending &p : std::as_const(sc.pending)) {
        auto glue = [&](const QString &id, double site, LineItem::Glue *end) {
            const auto it = sc.byId.constFind(id);
            if (it == sc.byId.constEnd() || site < 0 || site > 3) return;
            static const int sites[4] = {0, 3, 2, 1};   // top, right, bottom, left
            *end = LineItem::Glue{(*it)->id, sites[int(site)]};
        };
        glue(p.startId, p.startSite, &p.line->start);
        glue(p.endId, p.endSite, &p.line->end);
    }
}

void Reader::readMasters()
{
    const QDomElement masterStyles = kid(m_styles, "office:master-styles");
    int number = 0;
    for (const QDomElement &e : kids(masterStyles, "style:master-page")) {
        if (int(m_masters.size()) >= kMaxMasters) {
            note(QCoreApplication::translate("OdpImport", "Only the first 1,000 master pages are read."));
            break;
        }
        auto m = std::make_unique<MasterInfo>();
        m->element = e;
        m->name = at(e, "style:name");
        m->displayName = at(e, "style:display-name");
        m->layoutName = at(e, "style:page-layout-name");
        m_masters.push_back(std::move(m));
    }
    // The size first: it is what objects are placed on.
    readSize();
    for (auto &m : m_masters) readMasterPage(*m, ++number);
    for (auto &m : m_masters) m_doc.masters << m->page;
}

namespace {

// What a slide's placeholders come to: the layout's type, its name, and the key the layouts of a master are told apart by.
struct Shape {
    QString type, name, key;
};

Shape shapeOf(const QStringList &types)
{
    QStringList main;
    int contents = 0;
    for (const QString &t : types) {
        if (isFooter(t)) continue;
        main << t;
        if (isContent(t) && t != QLatin1String("pic")) ++contents;
    }
    Shape s;
    s.key = main.join(QLatin1Char(','));
    if (main.isEmpty()) {
        s.type = QStringLiteral("blank");
        s.name = QCoreApplication::translate("OdpImport", "Blank");
    } else if (main == QStringList{QStringLiteral("ctrTitle"), QStringLiteral("subTitle")}) {
        s.type = QStringLiteral("title");
        s.name = QCoreApplication::translate("OdpImport", "Title Slide");
    } else if (main == QStringList{QStringLiteral("title"), QStringLiteral("obj")}) {
        s.type = QStringLiteral("obj");
        s.name = QCoreApplication::translate("OdpImport", "Title and Content");
    } else if (main == QStringList{QStringLiteral("title")}) {
        s.type = QStringLiteral("titleOnly");
        s.name = QCoreApplication::translate("OdpImport", "Title Only");
    } else if (main == QStringList{QStringLiteral("title"), QStringLiteral("obj"), QStringLiteral("obj")}) {
        s.type = QStringLiteral("twoObj");
        s.name = QCoreApplication::translate("OdpImport", "Two Content");
    } else {
        s.type = QStringLiteral("cust");
        s.name = QString();
    }
    return s;
}

// The order the layouts of a master are listed in.
int layoutRank(const QString &type)
{
    static const QStringList order = {QStringLiteral("title"), QStringLiteral("obj"), QStringLiteral("twoObj"), QStringLiteral("titleOnly"), QStringLiteral("blank")};
    const int i = int(order.indexOf(type));
    return i < 0 ? int(order.size()) : i;
}

} // namespace

ItemPtr Reader::samplePlaceholder(MasterInfo &m, const QString &cls, const QRectF &rect, const QString &text)
{
    // A placeholder the master doesn't have, made of the master's own styles for that class.
    auto t = std::make_shared<TextItem>();
    t->rect = rect;
    t->insets = QMarginsF(7.2, 3.6, 7.2, 3.6);
    t->hyphenate = false;
    t->autofit = TextItem::ShrinkOnOverflow;
    t->storyId = m_doc.createStory();
    // The font and size the styles give a title or an outline.
    QString family = cls == QLatin1String("title") || cls == QLatin1String("ctrTitle") ? QStringLiteral("title") : QStringLiteral("outline1");
    QString styleName;
    for (auto it = m.element.firstChildElement(); !it.isNull(); it = it.nextSiblingElement()) {
        if (tagOf(it) != QLatin1String("draw:frame")) continue;
        const QString c = at(it, "presentation:class");
        if ((family == QLatin1String("title") && c == QLatin1String("title")) || (family != QLatin1String("title") && c == QLatin1String("outline"))) styleName = at(it, "presentation:style-name");
    }
    if (QTextDocument *sd = m_doc.storyDoc(t->storyId)) {
        const DeckTheme theme = themeOf(m_doc);
        QTextCursor c(sd);
        QTextCharFormat cf;
        const bool heading = family == QLatin1String("title");
        cf.setFontFamilies({heading ? theme.headingFont : theme.bodyFont});
        cf.setFontPointSize(std::clamp(m_slideSize.height() / 540.0, 0.5, 4.0) * (heading ? 44 : 28));
        cf.setForeground(theme.colors[0]);
        cf.setProperty(tp::ColorRefP, ColorRef::rgb(theme.colors[0]).toString());
        c.setCharFormat(cf);
        c.insertText(text, cf);
    }
    return t;
}

std::shared_ptr<MasterPage> Reader::standardLayout(MasterInfo &m, const QString &type)
{
    Shape wanted;
    wanted.type = type;
    for (const QString &key : m.layoutOrder)
        if (layoutType(*m.layouts.value(key)) == type) return m.layouts.value(key);
    if (m_layoutCount >= kMaxLayouts) return nullptr;
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("L%1").arg(++m_layoutCount);
    page->abbr = QString::number(m_layoutCount);
    page->baseId = m.page->id;
    page->props[QStringLiteral("kind")] = QStringLiteral("layout");
    page->props[QStringLiteral("type")] = type;
    const QSizeF s = m_slideSize;
    // The master's own title, body, and footers are what the layout stands on.
    const Item *title = nullptr, *body = nullptr;
    for (const ItemPtr &it : m.page->items) {
        const QString t = placeholderType(*it);
        if (t == QLatin1String("title") && !title) title = it.get();
        else if ((t == QLatin1String("body") || t == QLatin1String("obj")) && !body) body = it.get();
    }
    const QRectF titleRect = title ? title->rect : QRectF(s.width() * 0.0688, s.height() * 0.0532, s.width() * 0.8625, s.height() * 0.1933);
    const QRectF bodyRect = body ? body->rect : QRectF(s.width() * 0.0688, s.height() * 0.2662, s.width() * 0.8625, s.height() * 0.6344);
    auto add = [&](const Item *from, const QString &roleType, int index, const QRectF &rect, const QString &sample, const QString &cls) {
        ItemPtr it = from ? m_doc.cloneItem(*from) : samplePlaceholder(m, cls, rect, sample);
        it->rect = rect;
        it->role = deck::placeholderRole(roleType, index);
        it->name = promptFor(roleType).isEmpty() ? roleType : roleType;
        if (it->type() == ItemType::Text) {
            auto *t = static_cast<TextItem *>(it.get());
            makePrompt(m_doc, *t, sample);
            if (roleType == QLatin1String("ctrTitle")) t->valign = VAlign::Bottom;
        }
        page->items.push_back(it);
    };
    if (type == QLatin1String("title")) {
        page->name = QCoreApplication::translate("OdpImport", "Title Slide");
        add(title, QStringLiteral("ctrTitle"), 0, QRectF(titleRect.x(), s.height() * 0.2, titleRect.width(), s.height() * 0.3), kTitleSample, QStringLiteral("title"));
        add(body, QStringLiteral("subTitle"), 1, QRectF(titleRect.x(), s.height() * 0.53, titleRect.width(), s.height() * 0.25), kSubtitleSample, QStringLiteral("outline"));
    } else if (type == QLatin1String("obj")) {
        page->name = QCoreApplication::translate("OdpImport", "Title and Content");
        add(title, QStringLiteral("title"), 0, titleRect, kTitleSample, QStringLiteral("title"));
        add(body, QStringLiteral("obj"), 1, bodyRect, kBodySample, QStringLiteral("outline"));
    } else if (type == QLatin1String("titleOnly")) {
        page->name = QCoreApplication::translate("OdpImport", "Title Only");
        add(title, QStringLiteral("title"), 0, titleRect, kTitleSample, QStringLiteral("title"));
    } else {
        page->name = QCoreApplication::translate("OdpImport", "Blank");
    }
    for (const ItemPtr &it : m.page->items)
        if (isFooter(placeholderType(*it))) page->items.push_back(m_doc.cloneItem(*it));
    m.layouts.insert(QStringLiteral("standard:") + type, page);
    m.layoutOrder << QStringLiteral("standard:") + type;
    return page;
}

std::shared_ptr<MasterPage> Reader::layoutFor(MasterInfo &m, const ItemList &items)
{
    QStringList types;
    for (const ItemPtr &it : items) {
        const QString t = placeholderType(*it);
        if (!t.isEmpty()) types << t;
    }
    const Shape shape = shapeOf(types);
    const QString key = shape.key.isEmpty() && shape.type == QLatin1String("blank") ? QStringLiteral("standard:blank") : shape.key;
    // A layout this master has for those placeholders already (the standard ones are made as they are asked for).
    if (const auto it = m.layouts.constFind(key); it != m.layouts.cend()) return it.value();
    if (shape.type != QLatin1String("cust"))
        for (const QString &k : m.layoutOrder)
            if (k.startsWith(QLatin1String("standard:")) && layoutType(*m.layouts.value(k)) == shape.type) return m.layouts.value(k);
    if (m_layoutCount >= kMaxLayouts) {
        if (!m_said.contains(QStringLiteral("layouts"))) note(QCoreApplication::translate("OdpImport", "Only the first 1,000 layouts are made."));
        m_said.insert(QStringLiteral("layouts"));
        return m.layouts.isEmpty() ? nullptr : m.layouts.value(m.layoutOrder.first());
    }

    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("L%1").arg(++m_layoutCount);
    page->abbr = QString::number(m_layoutCount);
    page->baseId = m.page->id;
    page->name = shape.name.isEmpty() ? QCoreApplication::translate("OdpImport", "Custom Layout %1").arg(m_layoutCount) : shape.name;
    page->props[QStringLiteral("kind")] = QStringLiteral("layout");
    page->props[QStringLiteral("type")] = shape.type;
    // The placeholders as this slide has them, with prompts for their text.
    for (const ItemPtr &it : items) {
        const QString t = placeholderType(*it);
        if (t.isEmpty() || isFooter(t)) continue;
        ItemPtr copy = m_doc.cloneItem(*it);
        if (copy->type() == ItemType::Text) makePrompt(m_doc, static_cast<TextItem &>(*copy), sampleFor(t));
        copy->name.clear();
        page->items.push_back(copy);
    }
    for (const ItemPtr &it : m.page->items)
        if (isFooter(placeholderType(*it))) page->items.push_back(m_doc.cloneItem(*it));
    m.layouts.insert(key, page);
    m.layoutOrder << key;
    return page;
}

// ---- slides

void Reader::readTransition(const Bag &style, Page &page)
{
    Transition t;
    const QString type = style.get(Graphic, "smil:type"), subtype = style.get(Graphic, "smil:subtype");
    const bool reverse = style.get(Graphic, "smil:direction") == QLatin1String("reverse");
    const QString oldStyle = style.get(Graphic, "presentation:transition-style");
    auto fromSide = [](const QString &sub) {
        // The side a slide comes in from, as the way things move.
        if (sub.contains(QLatin1String("Left"))) return QStringLiteral("r");
        if (sub.contains(QLatin1String("Right"))) return QStringLiteral("l");
        if (sub.contains(QLatin1String("Top"))) return QStringLiteral("d");
        if (sub.contains(QLatin1String("Bottom"))) return QStringLiteral("u");
        return QStringLiteral("l");
    };
    auto opposite = [](const QString &d) { return d == QLatin1String("l") ? QStringLiteral("r") : d == QLatin1String("r") ? QStringLiteral("l") : d == QLatin1String("u") ? QStringLiteral("d") : QStringLiteral("u"); };
    bool matched = true;
    if (type == QLatin1String("fade") || type == QLatin1String("dissolve")) {
        t.type = QStringLiteral("fade");
    } else if (type == QLatin1String("pushWipe")) {
        t.type = QStringLiteral("push");
        t.dir = fromSide(subtype);
        if (reverse) t.dir = opposite(t.dir);
    } else if (type == QLatin1String("slideWipe")) {
        t.type = reverse ? QStringLiteral("uncover") : QStringLiteral("cover");
        t.dir = reverse ? opposite(fromSide(subtype)) : fromSide(subtype);
    } else if (type == QLatin1String("barWipe")) {
        t.type = QStringLiteral("wipe");
        t.dir = subtype == QLatin1String("topToBottom") ? QStringLiteral("d") : subtype == QLatin1String("leftToRight") ? QStringLiteral("r") : QStringLiteral("r");
        if (reverse) t.dir = opposite(t.dir);
    } else if (type == QLatin1String("barnDoorWipe")) {
        t.type = QStringLiteral("split");
        t.dir = reverse ? QStringLiteral("in") : QStringLiteral("out");
    } else if (type == QLatin1String("irisWipe") || type == QLatin1String("zoom") || type == QLatin1String("ellipseWipe")) {
        t.type = QStringLiteral("zoom");
        t.dir = reverse ? QStringLiteral("out") : QStringLiteral("in");
    } else if (type.isEmpty() && !oldStyle.isEmpty() && oldStyle != QLatin1String("none")) {
        // The older names of the effects: where they come from, and how.
        auto side = [&](const QString &s) { return s.endsWith(QLatin1String("left")) ? QStringLiteral("r") : s.endsWith(QLatin1String("right")) ? QStringLiteral("l") : s.endsWith(QLatin1String("top")) ? QStringLiteral("d") : QStringLiteral("u"); };
        if (oldStyle.startsWith(QLatin1String("fade-from-")) && !oldStyle.endsWith(QLatin1String("center"))) { t.type = QStringLiteral("wipe"); t.dir = side(oldStyle); }
        else if (oldStyle.startsWith(QLatin1String("move-from-"))) { t.type = QStringLiteral("push"); t.dir = side(oldStyle); }
        else if (oldStyle.startsWith(QLatin1String("roll-from-"))) { t.type = QStringLiteral("cover"); t.dir = side(oldStyle); }
        else if (oldStyle.startsWith(QLatin1String("uncover-to-"))) { t.type = QStringLiteral("uncover"); t.dir = opposite(side(oldStyle)); }
        else if (oldStyle == QLatin1String("dissolve")) t.type = QStringLiteral("fade");
        else if (oldStyle == QLatin1String("fade-from-center")) { t.type = QStringLiteral("zoom"); t.dir = QStringLiteral("in"); }
        else if (oldStyle == QLatin1String("fade-to-center")) { t.type = QStringLiteral("zoom"); t.dir = QStringLiteral("out"); }
        else if (oldStyle.startsWith(QLatin1String("open-"))) { t.type = QStringLiteral("split"); t.dir = QStringLiteral("out"); }
        else if (oldStyle.startsWith(QLatin1String("close-"))) { t.type = QStringLiteral("split"); t.dir = QStringLiteral("in"); }
        else matched = false;
    } else if (!type.isEmpty()) {
        matched = false;
    }
    if (!matched) ++m_transitionsLost;
    // How fast, and when it moves on.
    const QString speed = style.get(Graphic, "presentation:transition-speed");
    t.duration = speed == QLatin1String("slow") ? 1.0 : speed == QLatin1String("fast") ? 0.4 : 0.7;
    const QString mode = style.get(Graphic, "presentation:transition-type");
    double after = 0;
    if ((mode == QLatin1String("automatic") || mode == QLatin1String("semi-automatic")) && parseDuration(style.get(Graphic, "presentation:duration"), &after) && after > 0.001) {
        t.advanceAfter = after;
        t.advanceOnClick = mode == QLatin1String("semi-automatic");
    }
    setTransition(page, t);
}

void Reader::readNotes(const QDomElement &pageElement, Page &page)
{
    page.notesStoryId = m_doc.createStory();
    const QDomElement notes = kid(pageElement, "presentation:notes");
    if (notes.isNull()) return;
    for (const QDomElement &frame : kids(notes, "draw:frame")) {
        const Attrs a(frame);
        if (a.get("presentation:class") != QLatin1String("notes")) continue;
        const QDomElement box = kid(frame, "draw:text-box");
        if (box.isNull()) continue;
        Scope sc;
        sc.kind = Scope::Notes;
        sc.styles = m_contentSheet.get();
        sc.background = Qt::white;
        const Bag bag = shapeBag(a, *m_contentSheet);
        TextEnv env = textEnvFor(sc, a, bag, QStringLiteral("notes"));
        fillStory(page.notesStoryId, box, env);
        return;
    }
}

void Reader::readSlides(const QDomElement &presentation)
{
    for (const QDomElement &d : kids(presentation, "presentation:footer-decl")) m_footerDecls.insert(at(d, "presentation:name"), d.text().trimmed());
    for (const QDomElement &d : kids(presentation, "presentation:date-time-decl")) {
        m_dateDecls.insert(at(d, "presentation:name"), d.text().trimmed());
        // (A date is fixed unless it says it is the current one.)
        if (at(d, "presentation:source") != QLatin1String("current-date")) m_fixedDates.insert(at(d, "presentation:name"));
    }
    int number = 0;
    for (const QDomElement &p : kids(presentation, "draw:page")) {
        if (++number > kMaxSlides) {
            note(QCoreApplication::translate("OdpImport", "Only the first 10,000 slides are read."));
            break;
        }
        readSlide(p, number);
    }
}

bool Reader::readSlide(const QDomElement &pageElement, int number)
{
    const Attrs a(pageElement);
    MasterInfo *m = masterByName(a.get("draw:master-page-name"));
    if (!m) {
        note(QCoreApplication::translate("OdpImport", "Slide %1 was left out: the file has no master page for it.").arg(number));
        return false;
    }
    const Bag style = m_contentSheet->resolve(QStringLiteral("drawing-page"), a.get("draw:style-name"));
    auto page = std::make_shared<Page>();
    page->hidden = style.get(Graphic, "presentation:visibility") == QLatin1String("hidden");
    page->showMasterItems = style.get(Graphic, "presentation:background-objects-visible") != QLatin1String("false");

    // A background of its own.
    if (style.has(Graphic, "draw:fill") && style.get(Graphic, "draw:fill") != QLatin1String("none")) page->background = fillOf(style, *m_contentSheet);
    Scope sc;
    sc.kind = Scope::Slide;
    sc.styles = m_contentSheet.get();
    sc.page = page.get();
    sc.master = m;
    sc.background = page->background.isNone() ? m->backgroundColor : colorBehind(page->background);
    m_pageFill = page->background.isNone() ? m->page->background : page->background;
    readShapes(pageElement, sc, page->items, QTransform());
    if (!kid(pageElement, "anim:par").isNull()) ++m_animations;
    for (const Scope::Pending &p : std::as_const(sc.pending)) {
        auto glue = [&](const QString &id, double site, LineItem::Glue *end) {
            const auto it = sc.byId.constFind(id);
            if (it == sc.byId.constEnd() || site < 0 || site > 3) return;
            static const int sites[4] = {0, 3, 2, 1};
            *end = LineItem::Glue{(*it)->id, sites[int(site)]};
        };
        glue(p.startId, p.startSite, &p.line->start);
        glue(p.endId, p.endSite, &p.line->end);
    }

    // A title with a subtitle beside it is the centered title of a title slide.
    bool subtitle = false;
    for (const ItemPtr &it : page->items) subtitle |= placeholderType(*it) == QLatin1String("subTitle");
    for (const ItemPtr &it : page->items)
        if (subtitle && placeholderType(*it) == QLatin1String("title")) it->role = QStringLiteral("ph:ctrTitle");
    // Several content places are numbered in the order they are in.
    int content = 0;
    for (const ItemPtr &it : page->items) {
        const QString type = placeholderType(*it);
        if (type == QLatin1String("obj") || type == QLatin1String("pic")) it->role = deck::placeholderRole(type, ++content);
    }
    const std::shared_ptr<MasterPage> layout = layoutFor(*m, page->items);
    page->masterId = layout ? layout->id : m->page->id;
    readTransition(style, *page);
    for (const ItemPtr &it : page->items) {
        const QString type = placeholderType(*it);
        if ((type == QLatin1String("title") || type == QLatin1String("ctrTitle")) && it->type() == ItemType::Text) {
            if (const QTextDocument *story = m_doc.storyDoc(static_cast<const TextItem *>(it.get())->storyId))
                page->title = story->toPlainText().section(QChar(QChar::LineSeparator), 0, 0).section(QLatin1Char('\n'), 0, 0).trimmed();
            break;
        }
    }
    if (page->title.isEmpty()) page->title = a.get("draw:name");
    readNotes(pageElement, *page);
    readFooters(pageElement, style, *page);
    m_doc.pages << page;
    return true;
}

void Reader::readFooters(const QDomElement &pageElement, const Bag &style, Page &page)
{
    const Attrs a(pageElement);
    FooterOptions o;
    o.slideNumber = style.get(Graphic, "presentation:display-page-number") == QLatin1String("true");
    const QString footer = m_footerDecls.value(a.get("presentation:use-footer-name"));
    o.footerText = footer;
    o.footer = style.get(Graphic, "presentation:display-footer") == QLatin1String("true") && !footer.isEmpty();
    const QString dateName = a.get("presentation:use-date-time-name");
    o.date = style.get(Graphic, "presentation:display-date-time") == QLatin1String("true") && m_dateDecls.contains(dateName);
    // A fixed date is the text it is; one with no text shows nothing.
    if (o.date && m_fixedDates.contains(dateName)) {
        o.dateText = m_dateDecls.value(dateName);
        o.date = !o.dateText.isEmpty();
    }
    if (!o.any()) return;
    m_pendingFooters.append({&page, o});
    m_footers = true;
}

// ---- the whole

bool Reader::run(QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    // What kind of open document it is.
    const QString mime = QString::fromUtf8(m_pkg.part(QStringLiteral("/mimetype"))).trimmed();
    if (!mime.isEmpty() && !mime.startsWith(QLatin1String("application/vnd.oasis.opendocument.presentation"))) {
        QString kind = mime.mid(mime.lastIndexOf(QLatin1Char('.')) + 1);
        if (kind.startsWith(QLatin1String("text"))) kind = QCoreApplication::translate("OdpImport", "a text document");
        else if (kind.startsWith(QLatin1String("spreadsheet"))) kind = QCoreApplication::translate("OdpImport", "a spreadsheet");
        else if (kind.startsWith(QLatin1String("graphics"))) kind = QCoreApplication::translate("OdpImport", "a drawing");
        else kind = QCoreApplication::translate("OdpImport", "another kind of document");
        return fail(QCoreApplication::translate("OdpImport", "This file is an open document, but it is %1, not a presentation.").arg(kind));
    }
    if (m_pkg.part(QStringLiteral("/META-INF/manifest.xml")).contains("encryption-data"))
        return fail(QCoreApplication::translate("OdpImport", "This presentation is protected with a password, which JeffDeck can't open yet."));

    QString why;
    if (!parse(QStringLiteral("/content.xml"), &m_contentDom, &why)) return fail(QCoreApplication::translate("OdpImport", "The file has no readable content: %1").arg(why));
    if (!parse(QStringLiteral("/styles.xml"), &m_stylesDom, &why)) return fail(QCoreApplication::translate("OdpImport", "The file has no readable styles: %1").arg(why));
    m_content = m_contentDom.documentElement();
    m_styles = m_stylesDom.documentElement();
    if (tagOf(m_content) != QLatin1String("office:document-content") || tagOf(m_styles) != QLatin1String("office:document-styles"))
        return fail(QCoreApplication::translate("OdpImport", "The file is not an open document."));
    if (QDomDocument metaDom; m_pkg.hasPart(QStringLiteral("/meta.xml")) && parse(QStringLiteral("/meta.xml"), &metaDom, &why)) {
        m_metaDom = metaDom;
        m_meta = m_metaDom.documentElement();
    }
    const QDomElement body = kid(m_content, "office:body");
    const QDomElement presentation = kid(body, "office:presentation");
    if (presentation.isNull()) return fail(QCoreApplication::translate("OdpImport", "The file is an open document, but not a presentation."));

    // The styles, and the links text makes.
    m_common = std::make_shared<StyleSheet::Common>();
    m_contentSheet = std::make_unique<StyleSheet>(m_common);
    m_masterSheet = std::make_unique<StyleSheet>(m_common);
    for (const QDomElement &root : {m_styles, m_content}) m_contentSheet->readFonts(kid(root, "office:font-face-decls"));
    m_contentSheet->read(kid(m_styles, "office:styles"), false);
    m_contentSheet->read(kid(m_content, "office:automatic-styles"), true);
    m_masterSheet->read(kid(m_styles, "office:automatic-styles"), true);
    // Pictures and other styles are named in either part.
    m_contentSheet->read(kid(m_content, "office:styles"), false);
    buildLinks();

    readMeta();
    m_doc.masters.clear();
    readMasters();
    if (m_masters.empty()) return fail(QCoreApplication::translate("OdpImport", "The presentation has no master page that can be read."));
    readSlides(presentation);
    if (!ensureASlide(m_doc)) return fail(QCoreApplication::translate("OdpImport", "The presentation has no slides."));
    for (const QString &text : leftOutNotes(m_doc.imagesLeftOut, m_pkg.skippedCount())) note(text);

    // The theme: the colors the first master names, and the fonts of its title and outline.
    DeckTheme theme = defaultTheme();
    for (const auto &m : m_masters) {
        const QDomElement colors = kid(kid(m->element, "loext:theme"), "loext:theme-colors");
        if (colors.isNull()) continue;
        static const char *const names[12] = {"dark1", "light1", "dark2", "light2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hyperlink", "followed-hyperlink"};
        for (const QDomElement &c : kids(colors, "loext:color"))
            for (int i = 0; i < 12; ++i)
                if (at(c, "loext:name") == QLatin1String(names[i]))
                    if (const QColor v = parseColor(at(c, "loext:color")); v.isValid()) theme.colors[i] = v;
        if (const QString name = at(kid(m->element, "loext:theme"), "loext:name"); !name.isEmpty()) theme.name = name;
        break;
    }
    for (const QDomElement &frame : kids(m_masters.front()->element, "draw:frame")) {
        const Attrs a(frame);
        const QString cls = a.get("presentation:class");
        if (cls != QLatin1String("title") && cls != QLatin1String("outline")) continue;
        const Bag b = m_masterSheet->resolve(QStringLiteral("presentation"), a.get("presentation:style-name"));
        QString face = b.get(Text, "style:font-name");
        face = face.isEmpty() ? b.get(Text, "fo:font-family") : m_masterSheet->fontFamily(face);
        if (face.size() >= 2 && (face.startsWith(QLatin1Char('\'')) || face.startsWith(QLatin1Char('"')))) face = face.mid(1, face.size() - 2);
        if (!face.isEmpty()) (cls == QLatin1String("title") ? theme.headingFont : theme.bodyFont) = face;
    }
    setTheme(m_doc, theme);
    m_doc.fonts = {theme.name, theme.headingFont, theme.bodyFont};

    // The layouts: every master has the four that every presentation has, in the order they are listed.
    for (auto &m : m_masters)
        for (const QString &type : {QStringLiteral("title"), QStringLiteral("obj"), QStringLiteral("titleOnly"), QStringLiteral("blank")}) standardLayout(*m, type);
    QVector<std::shared_ptr<MasterPage>> ordered;
    for (auto &m : m_masters) {
        ordered << m->page;
        QStringList keys = m->layoutOrder;
        std::stable_sort(keys.begin(), keys.end(), [&](const QString &x, const QString &y) { return layoutRank(layoutType(*m->layouts.value(x))) < layoutRank(layoutType(*m->layouts.value(y))); });
        for (const QString &k : keys) ordered << m->layouts.value(k);
    }
    {
        // Numbered in the order they are listed, now that it is known.
        QHash<QString, QString> renamed;
        int n = 0;
        for (const auto &mp : ordered) {
            if (isSlideMaster(*mp)) continue;
            const QString id = QStringLiteral("L%1").arg(++n);
            renamed.insert(mp->id, id);
            mp->id = id;
            mp->abbr = QString::number(n);
        }
        for (const auto &page : m_doc.pages)
            if (renamed.contains(page->masterId)) page->masterId = renamed.value(page->masterId);
    }
    m_doc.masters = ordered;
    m_doc.appData[QStringLiteral("footers")] = false;
    // The footers the slides show, in the places the layouts have for them.
    for (const auto &pf : std::as_const(m_pendingFooters)) applyFooters(m_doc, pf.second, pf.first);
    if (m_footers) m_doc.appData[QStringLiteral("footers")] = true;

    if (m_animations) note(QCoreApplication::translate("OdpImport", "The animations of %1 slides were left out.").arg(m_animations));
    if (m_transitionsLost) note(QCoreApplication::translate("OdpImport", "The transitions of %1 slides have no match in JeffDeck, and were left out.").arg(m_transitionsLost));
    return true;
}

} // namespace jp::deck::odp

namespace jp::deck {

bool looksLikeOpenDocument(const QByteArray &head)
{
    // The first entry of the ZIP is "mimetype", stored, with the type for its data.
    if (head.size() < 38 || !head.startsWith("PK\x03\x04")) return false;
    const auto u16 = [&](int at) { return int(uchar(head[at])) | int(uchar(head[at + 1])) << 8; };
    const int method = u16(8), nameLength = u16(26), extraLength = u16(28);
    if (method != 0 || nameLength != 8 || head.mid(30, 8) != "mimetype") return false;
    return head.mid(30 + nameLength + extraLength, 40).startsWith("application/vnd.oasis.opendocument.");
}

std::unique_ptr<Document> importOdp(const QByteArray &zip, QString *error, QStringList *skipped)
{
    office::Package pkg;
    if (!pkg.readOpenDocument(zip, error)) return nullptr;
    auto doc = std::make_unique<Document>();
    doc->slideText = true;
    doc->slidePictures = true;
    QStringList notes;
    odp::Reader reader(pkg, *doc, &notes);
    if (!reader.run(error)) return nullptr;
    // The fonts the package carries are available while the presentation is open (core/embeddedfonts.h).
    EmbeddedFonts::use(doc.get(), readEmbeddedFontFiles(pkg, QStringLiteral("/Fonts"), &notes));
    doc->fromFile = true;   // what the file names outside itself is read only from beside it
    if (skipped) *skipped = notes;
    return doc;
}

} // namespace jp::deck
