// The objects of an open document presentation (odpimport_p.h): frames (text
// boxes, pictures, tables, embedded objects), custom shapes, the basic
// drawing shapes, lines and connectors, polygons and paths, and groups, with
// the fill, outline, and effects their styles give them.

#include "io/odpimport_p.h"

#include "core/deck.h"
#include "core/items.h"
#include "core/svg.h"
#include "office/drawingml_style.h"
#include "render/shapes.h"
#include "text/textengine.h"

#include <QCoreApplication>
#include <QImage>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextDocument>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp::deck::odp {

namespace {

constexpr int kMaxShapes = 100000;
constexpr int kMaxDepth = 64;
constexpr int kMaxTableSide = 256, kMaxTableCells = 10000;
// The text of one table, in all its cells: the cells after it are left without text. And how much of it the rows are fitted to, in all and
// in one cell: laying out a long text in a narrow cell takes a long time (a tenth of a second for two thousand characters, a second and a
// half for twenty thousand), so the rows of cells past those limits keep the height the file gives them. The tables of the real decks
// here hold 1,107 characters at the most (a cell, 675) and 48 cells; a table of a million took minutes to draw, so a table keeps 90 times
// the most real ones hold.
constexpr qsizetype kMaxTableChars = 100000, kMaxFitChars = 50000, kMaxFitCellChars = 10000;
constexpr double kMaxPoint = 1e6;

QString say(const char *text) { return QCoreApplication::translate("OdpImport", text); }

double position(double v) { return std::isfinite(v) ? std::clamp(v, -kMaxPoint, kMaxPoint) : 0.0; }
double extent(double v) { return std::isfinite(v) ? std::clamp(v, 0.0, kMaxPoint) : 0.0; }

double turn(double degrees)
{
    if (!std::isfinite(degrees)) return 0;
    double d = std::fmod(degrees, 360.0);
    if (d < 0) d += 360;
    return d >= 360 ? 0 : d;
}

// "rotate (0.5) translate (1cm 2cm)": the operations in the order the file lists them, the first done first.
QTransform parseTransform(const QString &text)
{
    static const QRegularExpression op(QStringLiteral("([A-Za-z]+)\\s*\\(([^)]*)\\)"));
    QTransform total;
    int ops = 0;
    for (auto it = op.globalMatch(text); it.hasNext() && ops < 32; ++ops) {
        const QRegularExpressionMatch m = it.next();
        const QString name = m.captured(1);
        const QStringList args = m.captured(2).split(QRegularExpression(QStringLiteral("[\\s,]+")), Qt::SkipEmptyParts);
        auto number = [&](int i, double fallback = 0) {
            double v = fallback;
            if (i < args.size()) {
                const QString a = args[i].endsWith(QLatin1String("deg")) ? args[i] : args[i];
                parseNumber(a, &v);
            }
            return v;
        };
        auto angle = [&](int i) {
            if (i < args.size() && args[i].endsWith(QLatin1String("deg"))) {
                double v = 0;
                parseNumber(args[i].chopped(3), &v);
                return qDegreesToRadians(v);
            }
            return number(i);
        };
        auto lengthAt = [&](int i) { return i < args.size() ? lengthOr(args[i], 0) : 0.0; };
        QTransform t;
        // The file turns counter-clockwise by a positive angle.
        if (name == QLatin1String("rotate")) t = QTransform().rotateRadians(-angle(0));
        else if (name == QLatin1String("translate")) t = QTransform::fromTranslate(lengthAt(0), lengthAt(1));
        else if (name == QLatin1String("scale")) t = QTransform::fromScale(number(0, 1), args.size() > 1 ? number(1, 1) : number(0, 1));
        else if (name == QLatin1String("skewX")) t = QTransform(1, 0, std::tan(angle(0)), 1, 0, 0);
        else if (name == QLatin1String("skewY")) t = QTransform(1, std::tan(angle(0)), 0, 1, 0, 0);
        else if (name == QLatin1String("matrix")) t = QTransform(number(0, 1), number(1), number(2), number(3, 1), lengthAt(4), lengthAt(5));
        else continue;
        total = total * t;
    }
    return total;
}

// The ends of a box turned about its center: whether a shape this turned has its width along the page's other side.
bool finiteTransform(const QTransform &t)
{
    return std::isfinite(t.m11()) && std::isfinite(t.m12()) && std::isfinite(t.m21()) && std::isfinite(t.m22()) && std::isfinite(t.dx()) && std::isfinite(t.dy());
}

} // namespace

bool isWebLink(const QString &href)
{
    const QString scheme = href.section(QLatin1Char(':'), 0, 0).toLower();
    return href.contains(QLatin1Char(':')) && (scheme == QLatin1String("http") || scheme == QLatin1String("https") || scheme == QLatin1String("mailto") || scheme == QLatin1String("ftp"));
}

QString masterPrompt(const QString &cls)
{
    if (cls == QLatin1String("title")) return QCoreApplication::translate("OdpImport", "Click to edit Master title style");
    if (cls == QLatin1String("subtitle")) return QCoreApplication::translate("OdpImport", "Click to edit Master subtitle style");
    if (cls == QLatin1String("outline")) return QCoreApplication::translate("OdpImport", "Click to edit Master text styles");
    return QString();
}

// ---- the page's objects

void Reader::readShapes(const QDomElement &parent, Scope &sc, ItemList &out, const QTransform &t)
{
    for (QDomElement c = parent.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (sc.shapes >= kMaxShapes) {
            if (!m_said.contains(QStringLiteral("shapes"))) note(say("A page holds more than 100,000 objects; the rest are left out."));
            m_said.insert(QStringLiteral("shapes"));
            break;
        }
        const QString tag = tagOf(c);
        if (tag == QLatin1String("draw:a")) {
            // A link around objects: they are read as they are, each with the link.
            if (sc.depth >= kMaxDepth) continue;
            ItemList inner;
            ++sc.depth;
            readShapes(c, sc, inner, t);
            --sc.depth;
            const QString href = at(c, "xlink:href");
            for (const ItemPtr &it : inner) {
                if (isWebLink(href) && it->hyperlink.isEmpty()) it->hyperlink = href;
                out.push_back(it);
            }
            continue;
        }
        if (ItemPtr it = readShape(c, sc, t)) out.push_back(it);
    }
}

ItemPtr Reader::readShape(const QDomElement &e, Scope &sc, const QTransform &t)
{
    const QString tag = tagOf(e);
    if (!tag.startsWith(QLatin1String("draw:")) && !tag.startsWith(QLatin1String("dr3d:"))) return nullptr;
    if (tag == QLatin1String("draw:glue-point") || tag == QLatin1String("draw:layer") || tag == QLatin1String("draw:layer-set") || tag == QLatin1String("draw:page-thumbnail")
        || tag == QLatin1String("draw:event-listeners")) return nullptr;
    const Attrs a(e);
    // Hidden on the screen, and the layer for controls.
    const QString display = a.get("drawooo:display", a.get("draw:display"));
    if (display == QLatin1String("none") || display == QLatin1String("printer")) return nullptr;
    if (a.get("draw:layer") == QLatin1String("controls") || a.get("draw:layer") == QLatin1String("measurelines")) return nullptr;
    ++sc.shapes;
    ItemPtr it;
    if (tag == QLatin1String("draw:frame")) it = readFrame(e, sc, t);
    else if (tag == QLatin1String("draw:custom-shape")) it = readCustomShape(e, sc, t);
    else if (tag == QLatin1String("draw:rect") || tag == QLatin1String("draw:ellipse") || tag == QLatin1String("draw:circle") || tag == QLatin1String("draw:caption")) it = readBasicShape(e, sc, t);
    else if (tag == QLatin1String("draw:line") || tag == QLatin1String("draw:connector")) it = readLine(e, sc, t);
    else if (tag == QLatin1String("draw:polyline") || tag == QLatin1String("draw:polygon") || tag == QLatin1String("draw:path")) it = readPathShape(e, sc, t);
    else if (tag == QLatin1String("draw:g")) it = readGroup(e, sc, t);
    else if (tag == QLatin1String("draw:control") || tag.startsWith(QLatin1String("dr3d:")) || tag == QLatin1String("draw:measure")) {
        if (!m_said.contains(tag)) note(say("Some objects of a kind JeffDeck can't show were left out."));
        m_said.insert(tag);
    }
    if (it) {
        const QString id = a.get("draw:id", a.get("xml:id"));
        const QString name = a.get("draw:name");
        if (it->name.isEmpty()) it->name = name;
        // The description the object has for those who can't see it, and the address a click on it goes to.
        if (it->altText.isEmpty()) it->altText = kid(e, "svg:desc").text().trimmed().isEmpty() ? kid(e, "svg:title").text().trimmed() : kid(e, "svg:desc").text().trimmed();
        if (it->hyperlink.isEmpty())
            for (const QDomElement &l : kids(kid(e, "office:event-listeners"), "presentation:event-listener")) {
                const QString href = at(l, "xlink:href");
                if (isWebLink(href)) {
                    it->hyperlink = href;
                    break;
                }
            }
        if (!id.isEmpty()) sc.byId.insert(id, it);
        if (!name.isEmpty()) sc.byName.insert(name, it);
    }
    return it;
}

Bag Reader::shapeBag(const Attrs &a, const StyleSheet &styles)
{
    Bag bag;
    styles.applyDefaults(bag, QStringLiteral("graphic"));
    if (a.has("presentation:style-name")) styles.apply(bag, QStringLiteral("presentation"), a.get("presentation:style-name"));
    if (a.has("draw:style-name")) styles.apply(bag, QStringLiteral("graphic"), a.get("draw:style-name"));
    return bag;
}

void Reader::place(Item &it, const QRectF &box, const QTransform &total)
{
    if (!finiteTransform(total)) {
        it.rect = QRectF(position(box.x()), position(box.y()), extent(box.width()), extent(box.height()));
        return;
    }
    const QPointF center = total.map(box.center());
    const double sx = std::hypot(total.m11(), total.m12()), sy = std::hypot(total.m21(), total.m22());
    const double det = total.m11() * total.m22() - total.m12() * total.m21();
    const double w = extent(box.width() * sx), h = extent(box.height() * sy);
    it.rect = QRectF(position(center.x() - w / 2), position(center.y() - h / 2), w, h);
    it.rotation = turn(qRadiansToDegrees(std::atan2(total.m12(), total.m11())));
    if (det < 0) it.flipV = !it.flipV;
}

QTransform Reader::transformOf(const Attrs &a, const QSizeF &size, QRectF *box, bool *plain)
{
    *box = QRectF(lengthOr(a.get("svg:x"), 0), lengthOr(a.get("svg:y"), 0), size.width(), size.height());
    const QString text = a.get("draw:transform");
    if (plain) *plain = text.isEmpty();
    return text.isEmpty() ? QTransform() : parseTransform(text);
}

void Reader::decorate(Item &it, const Attrs &a, const Bag &bag, const StyleSheet &styles)
{
    it.name = a.get("draw:name");
    it.wrap.mode = Wrap::None;
    it.fill = fillOf(bag, styles);
    // An object filled with what is behind it.
    if (bag.get(Graphic, "loext:fill-use-slide-background") == QLatin1String("true") && (bag.get(Graphic, "draw:fill") == QLatin1String("none") || bag.get(Graphic, "draw:fill").isEmpty()))
        it.fill = m_pageFill;
    it.stroke = strokeOf(bag, styles);
    it.fx = effectsOf(bag);
}

// ---- placeholders

QString Reader::roleOf(const QString &cls, const Scope &sc, int *contentIndex)
{
    if (cls.isEmpty()) return QString();
    if (cls == QLatin1String("title")) return QStringLiteral("ph:title");
    if (cls == QLatin1String("subtitle")) return QStringLiteral("ph:subTitle:1");
    if (cls == QLatin1String("date-time")) return QStringLiteral("ph:dt");
    if (cls == QLatin1String("footer")) return QStringLiteral("ph:ftr");
    if (cls == QLatin1String("page-number")) return QStringLiteral("ph:sldNum");
    if (cls == QLatin1String("outline") || cls == QLatin1String("graphic") || cls == QLatin1String("object") || cls == QLatin1String("chart") || cls == QLatin1String("table")
        || cls == QLatin1String("orgchart")) {
        const int n = ++*contentIndex;
        if (cls == QLatin1String("graphic")) return QStringLiteral("ph:pic:%1").arg(n);
        if (sc.kind == Scope::Master && cls == QLatin1String("outline")) return QStringLiteral("ph:body:%1").arg(n);
        return QStringLiteral("ph:obj:%1").arg(n);
    }
    return QString();
}

// ---- frames

ItemPtr Reader::readFrame(const QDomElement &e, Scope &sc, const QTransform &parentTransform)
{
    // What the frame holds: text, a table, or a picture (which an embedded object has for its replacement).
    QDomElement box, image, table, object, plugin;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString t = tagOf(c);
        if (t == QLatin1String("draw:text-box") && box.isNull()) box = c;
        else if (t == QLatin1String("draw:image") && image.isNull()) image = c;
        else if (t == QLatin1String("table:table") && table.isNull()) table = c;
        else if ((t == QLatin1String("draw:object") || t == QLatin1String("draw:object-ole")) && object.isNull()) object = c;
        else if ((t == QLatin1String("draw:plugin") || t == QLatin1String("draw:applet") || t == QLatin1String("draw:floating-frame")) && plugin.isNull()) plugin = c;
    }
    if (!box.isNull()) return readTextBox(e, box, sc, parentTransform);
    if (!table.isNull()) return readTableFrame(e, table, sc, parentTransform);
    if (!image.isNull()) return readPicture(e, image, sc, parentTransform);
    if (!plugin.isNull()) {
        if (!m_said.contains(QStringLiteral("plugin"))) note(say("Movies, sounds, and other plug-in objects were left out."));
        m_said.insert(QStringLiteral("plugin"));
        return nullptr;
    }
    if (!object.isNull()) {
        if (!m_said.contains(QStringLiteral("object"))) note(say("Embedded objects that have no picture in the file were left out."));
        m_said.insert(QStringLiteral("object"));
        return nullptr;
    }
    // A placeholder frame with nothing in it yet (a picture or content to be added).
    const Attrs a(e);
    const QString cls = a.get("presentation:class");
    if (!cls.isEmpty() && a.get("presentation:placeholder") == QLatin1String("true") && cls != QLatin1String("page")) return readTextBox(e, QDomElement(), sc, parentTransform);
    return nullptr;
}

ItemPtr Reader::readTextBox(const QDomElement &frame, const QDomElement &box, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(frame);
    const QString cls = a.get("presentation:class");
    if (cls == QLatin1String("page") || cls == QLatin1String("notes") || cls == QLatin1String("handout") || cls == QLatin1String("header")) return nullptr;
    const StyleSheet &styles = *sc.styles;
    const Bag bag = shapeBag(a, styles);
    const double minHeight = bag.get(Graphic, "draw:auto-grow-height") == QLatin1String("true") ? lengthOr(bag.get(Graphic, "fo:min-height"), 0) : 0;
    const QSizeF size(extent(lengthOr(a.get("svg:width"), 0)), extent(std::max(lengthOr(a.get("svg:height"), 0), minHeight)));
    QRectF boxRect;
    const QTransform total = transformOf(a, size, &boxRect, nullptr) * parentTransform;

    auto t = std::make_shared<TextItem>();
    decorate(*t, a, bag, styles);
    place(*t, boxRect, total);
    int content = sc.contentIndex;
    t->role = roleOf(cls, sc, &content);
    sc.contentIndex = content;
    t->hyphenate = false;
    // The padding is the text's distance from the edge.
    const double pad = lengthOr(bag.get(Graphic, "fo:padding"), 0);
    t->insets = QMarginsF(lengthOr(bag.get(Graphic, "fo:padding-left"), pad), lengthOr(bag.get(Graphic, "fo:padding-top"), pad),
                          lengthOr(bag.get(Graphic, "fo:padding-right"), pad), lengthOr(bag.get(Graphic, "fo:padding-bottom"), pad));
    const QString anchor = bag.get(Graphic, "draw:textarea-vertical-align");
    t->valign = anchor == QLatin1String("middle") ? VAlign::Middle : anchor == QLatin1String("bottom") ? VAlign::Bottom : VAlign::Top;
    if (bag.get(Graphic, "style:shrink-to-fit") == QLatin1String("true") || bag.get(Graphic, "draw:fit-to-size") == QLatin1String("shrink")) t->autofit = TextItem::ShrinkOnOverflow;
    else if (bag.get(Graphic, "draw:auto-grow-height") == QLatin1String("true")) t->autofit = TextItem::GrowBox;
    const QString mode = bag.get(Para, "style:writing-mode");
    t->vertical = mode.startsWith(QLatin1String("tb"));
    // Text turned a quarter by the file's own setting.
    if (double angle = 0; parseNumber(bag.get(Text, "style:rotation-angle"), &angle) && std::fmod(std::fabs(angle), 360) == 90) t->vertical = true;

    t->storyId = m_doc.createStory();
    TextEnv env = textEnvFor(sc, a, bag, cls);
    env.vertical = t->vertical;
    fillStory(t->storyId, box, env);
    if (!cls.isEmpty()) {
        QTextDocument *story = m_doc.storyDoc(t->storyId);
        if (story && story->isEmpty()) {
            if (sc.kind == Scope::Slide) {
                t->prompt = promptFor(placeholderType(*t));
            } else if (sc.kind == Scope::Master) {
                // A master's placeholder with nothing in it says what it is for, in the formatting its styles give.
                QTextCursor c(story);
                c.insertText(masterPrompt(cls), c.charFormat());
            }
        }
    }
    return t;
}

ItemPtr Reader::readPicture(const QDomElement &frame, const QDomElement &imageElement, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(frame);
    const QString cls = a.get("presentation:class");
    const StyleSheet &styles = *sc.styles;
    const Bag bag = shapeBag(a, styles);
    const QSizeF size(extent(lengthOr(a.get("svg:width"), 0)), extent(lengthOr(a.get("svg:height"), 0)));
    QRectF boxRect;
    const QTransform total = transformOf(a, size, &boxRect, nullptr) * parentTransform;

    QString id = loadPicture(at(imageElement, "xlink:href"));
    // The picture the file keeps inside the image element itself.
    if (id.isEmpty()) {
        const QDomElement data = kid(imageElement, "office:binary-data");
        if (!data.isNull()) {
            const QByteArray bytes = QByteArray::fromBase64(data.text().toLatin1());
            const QString kind = office::imageFormat(bytes);
            if (!kind.isEmpty()) id = m_doc.addImage(bytes, kind);
        }
    }
    // An embedded object that is a chart, drawn from its numbers, when its own picture can't be shown.
    if (id.isEmpty())
        if (const QDomElement object = kid(frame, "draw:object"); !object.isNull()) id = chartImage(object, size);
    const bool placeholder = a.get("presentation:placeholder") == QLatin1String("true");
    if (id.isEmpty() && !(placeholder && cls == QLatin1String("graphic"))) {
        // An embedded object (a chart) is shown by its picture; the file may keep none.
        const bool object = !kid(frame, "draw:object").isNull() || !kid(frame, "draw:object-ole").isNull();
        const QString key = object ? QStringLiteral("objectpicture") : QStringLiteral("picture");
        if (!m_said.contains(key))
            note(object ? say("Embedded objects that are not charts were left out: the file keeps no picture of them that JeffDeck can show.")
                        : say("Pictures that are not in the file, or that JeffDeck can't read, were left out."));
        m_said.insert(key);
        return nullptr;
    }
    auto p = std::make_shared<PictureItem>();
    decorate(*p, a, bag, styles);
    // A picture's hairline outline (the line of no width a standard style has) isn't drawn.
    if (lengthOr(bag.get(Graphic, "svg:stroke-width"), 0) <= 0) p->stroke = Stroke::none();
    place(*p, boxRect, total);
    p->imageId = id;
    int content = sc.contentIndex;
    p->role = roleOf(cls, sc, &content);
    sc.contentIndex = content;
    p->imgRect = QRectF(0, 0, p->rect.width(), p->rect.height());
    if (!id.isEmpty()) {
        // The crop: lengths cut from the picture as it is, at the size its own resolution gives.
        const QString clip = bag.get(Graphic, "fo:clip");
        static const QRegularExpression rect(QStringLiteral("^rect\\(\\s*([^,\\s]+)[,\\s]+([^,\\s]+)[,\\s]+([^,\\s]+)[,\\s]+([^,\\s)]+)\\s*\\)$"));
        const QRegularExpressionMatch m = rect.match(clip);
        const QSize px = m_doc.imageSize(id);
        if (m.hasMatch() && px.width() > 0 && px.height() > 0) {
            const QImage image = m_doc.image(id);
            const double dpiX = image.dotsPerMeterX() > 0 ? image.dotsPerMeterX() * 0.0254 : 96, dpiY = image.dotsPerMeterY() > 0 ? image.dotsPerMeterY() * 0.0254 : 96;
            const double naturalW = px.width() / dpiX * 72, naturalH = px.height() / dpiY * 72;
            const double top = lengthOr(m.captured(1), 0), right = lengthOr(m.captured(2), 0), bottom = lengthOr(m.captured(3), 0), left = lengthOr(m.captured(4), 0);
            double cl = left / naturalW, ct = top / naturalH, cr = right / naturalW, cb = bottom / naturalH;
            if (1 - cl - cr <= 0.001 || 1 - ct - cb <= 0.001) cl = ct = cr = cb = 0;
            const double w = p->rect.width() / (1 - cl - cr), h = p->rect.height() / (1 - ct - cb);
            p->imgRect = QRectF(position(-cl * w), position(-ct * h), extent(w), extent(h));
        }
        const QString mirror = bag.get(Graphic, "style:mirror");
        if (mirror.contains(QLatin1String("horizontal"))) p->flipH = !p->flipH;
        if (mirror.contains(QLatin1String("vertical"))) p->flipV = !p->flipV;
        const QString mode = bag.get(Graphic, "draw:color-mode");
        if (mode == QLatin1String("greyscale")) p->recolor = PictureItem::Grayscale;
        else if (mode == QLatin1String("mono")) p->recolor = PictureItem::BlackWhite;
        else if (mode == QLatin1String("watermark")) p->recolor = PictureItem::Washout;
        double v = 0;
        if (parsePercent(bag.get(Graphic, "draw:luminance"), &v)) p->brightness = std::clamp(v * 100, -100.0, 100.0);
        if (parsePercent(bag.get(Graphic, "draw:contrast"), &v)) p->contrast = std::clamp(v * 100, -100.0, 100.0);
        if (parsePercent(bag.get(Graphic, "draw:image-opacity"), &v)) p->transparency = std::clamp(1 - v, 0.0, 1.0);
    }
    return p;
}

// ---- tables

namespace {

// A cell border "0.05pt solid #cccccc".
Stroke borderOf(const QString &value)
{
    const QString v = value.simplified();
    if (v.isEmpty() || v == QLatin1String("none") || v == QLatin1String("hidden")) return Stroke::none();
    const QStringList parts = v.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    Stroke s;
    double width = 0;
    QColor color;
    bool dashed = false, none = false;
    for (const QString &p : parts) {
        if (p == QLatin1String("none") || p == QLatin1String("hidden")) none = true;
        else if (p == QLatin1String("dashed") || p == QLatin1String("dotted")) dashed = true;
        else if (p.startsWith(QLatin1Char('#'))) color = parseColor(p);
        else if (double w = 0; parseLength(p, &w)) width = w;
    }
    if (none) return Stroke::none();
    s.width = width > 0 ? width : 0.5;
    s.color = ColorRef::rgb(color.isValid() ? color : QColor(Qt::black));
    if (dashed) s.dash = Stroke::DashLine;
    return s;
}

} // namespace

ItemPtr Reader::readTableFrame(const QDomElement &frame, const QDomElement &tableEl, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(frame);
    const StyleSheet &styles = *sc.styles;
    const Bag frameBag = shapeBag(a, styles);

    struct ColumnInfo { double width = 0; QString cellStyle; };
    struct RowInfo { double height = 0; QString cellStyle; QDomElement element; };
    QVector<ColumnInfo> columns;
    QVector<RowInfo> rows;
    bool tooBig = false;
    // Columns and rows, in the groups that hold them (header rows, row groups).
    struct Collect {
        static void run(const QDomElement &parent, const StyleSheet &styles, QVector<ColumnInfo> *columns, QVector<RowInfo> *rows, bool *tooBig, int depth)
        {
            if (depth > 8) return;
            for (QDomElement c = parent.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
                const QString t = tagOf(c);
                const Attrs ca(c);
                if (t == QLatin1String("table:table-column")) {
                    const double repeat = std::clamp(lengthOr(ca.get("table:number-columns-repeated", QStringLiteral("1")), 1), 1.0, 10000.0);
                    ColumnInfo info;
                    const Bag cb = styles.resolve(QStringLiteral("table-column"), ca.get("table:style-name"));
                    info.width = lengthOr(cb.get(Column, "style:column-width"), 0);
                    info.cellStyle = ca.get("table:default-cell-style-name");
                    for (int i = 0; i < int(repeat); ++i) {
                        if (columns->size() >= kMaxTableSide) {
                            *tooBig = true;
                            break;
                        }
                        columns->append(info);
                    }
                } else if (t == QLatin1String("table:table-row")) {
                    const double repeat = std::clamp(lengthOr(ca.get("table:number-rows-repeated", QStringLiteral("1")), 1), 1.0, 10000.0);
                    RowInfo info;
                    const Bag rb = styles.resolve(QStringLiteral("table-row"), ca.get("table:style-name"));
                    info.height = std::max(lengthOr(rb.get(Row, "style:row-height"), 0), lengthOr(rb.get(Row, "style:min-row-height"), 0));
                    info.cellStyle = ca.get("table:default-cell-style-name");
                    info.element = c;
                    for (int i = 0; i < int(repeat); ++i) {
                        if (rows->size() >= kMaxTableSide) {
                            *tooBig = true;
                            break;
                        }
                        rows->append(info);
                    }
                } else if (t == QLatin1String("table:table-columns") || t == QLatin1String("table:table-header-columns") || t == QLatin1String("table:table-rows")
                           || t == QLatin1String("table:table-header-rows") || t == QLatin1String("table:table-row-group")) {
                    run(c, styles, columns, rows, tooBig, depth + 1);
                }
            }
        }
    };
    Collect::run(tableEl, styles, &columns, &rows, &tooBig, 0);
    if (columns.isEmpty() || rows.isEmpty()) {
        if (!m_said.contains(QStringLiteral("table"))) note(say("A table with no rows or no columns was left out."));
        m_said.insert(QStringLiteral("table"));
        return nullptr;
    }
    // A table of more cells than that is read as far as its rows go.
    if (columns.size() * rows.size() > kMaxTableCells) {
        rows.resize(std::max<qsizetype>(1, kMaxTableCells / columns.size()));
        if (!m_said.contains(QStringLiteral("tablecells"))) note(say("Tables are read up to 10,000 cells."));
        m_said.insert(QStringLiteral("tablecells"));
    }
    if (tooBig && !m_said.contains(QStringLiteral("tablebig"))) {
        note(say("Tables are read up to 256 rows and columns."));
        m_said.insert(QStringLiteral("tablebig"));
    }

    const int nr = int(rows.size()), nc = int(columns.size());
    auto t = std::make_shared<TableItem>();
    t->rows = nr;
    t->cols = nc;
    t->wrap.mode = Wrap::None;
    t->header = false;
    t->banded = false;
    t->name = a.get("draw:name");
    const double frameW = extent(lengthOr(a.get("svg:width"), 0)), frameH = extent(lengthOr(a.get("svg:height"), 0));
    double columnsWidth = 0, rowsHeight = 0, unsizedColumns = 0, unsizedRows = 0;
    for (const ColumnInfo &c : columns) {
        columnsWidth += c.width;
        unsizedColumns += c.width <= 0;
    }
    for (const RowInfo &r : rows) {
        rowsHeight += r.height;
        unsizedRows += r.height <= 0;
    }
    for (const ColumnInfo &c : columns) t->colW.append(c.width > 0 ? c.width : (frameW > columnsWidth && unsizedColumns > 0 ? (frameW - columnsWidth) / unsizedColumns : 40.0));
    for (const RowInfo &r : rows) t->rowH.append(r.height > 0 ? r.height : (frameH > rowsHeight && unsizedRows > 0 ? (frameH - rowsHeight) / unsizedRows : 20.0));
    t->cells.resize(nr * nc);

    // Each cell, with the merged cells it covers. A cell that the table repeats (across a row, or down the table when rows repeat, which
    // share one element) is made once, and the ones that repeat it are copies, each with a story of its own.
    struct Made {
        bool valid = false;
        QDomElement element;
        QString styleName;
        bool covered = false;
        TableCell cell;
        qsizetype chars = 0;   // of its text
    };
    QVector<Made> above(nc), here(nc);
    qsizetype tableChars = 0;
    bool textCut = false;
    int cellCount = 0;
    for (int r = 0; r < nr; ++r) {
        here.fill(Made());
        int c = 0;
        for (QDomElement cell = rows[r].element.firstChildElement(); !cell.isNull() && c < nc; cell = cell.nextSiblingElement()) {
            const QString tag = tagOf(cell);
            const bool covered = tag == QLatin1String("table:covered-table-cell");
            if (tag != QLatin1String("table:table-cell") && !covered) continue;
            const Attrs ca(cell);
            const int repeat = int(std::clamp(lengthOr(ca.get("table:number-columns-repeated", QStringLiteral("1")), 1), 1.0, 10000.0));
            for (int k = 0; k < repeat && c < nc; ++k, ++c) {
                if (++cellCount > kMaxTableCells) break;
                TableCell &out = t->cell(r, c);
                // The cell's style: its own, else its row's, else its column's.
                QString styleName = ca.get("table:style-name");
                if (styleName.isEmpty()) styleName = rows[r].cellStyle;
                if (styleName.isEmpty()) styleName = columns[c].cellStyle;
                const Made *twin = nullptr;
                for (const Made *m : {c > 0 ? &here[c - 1] : nullptr, &above[c]})
                    if (m && m->valid && m->element == cell && m->styleName == styleName && m->covered == covered) {
                        twin = m;
                        break;
                    }
                if (twin) out = twin->cell;
                out.covered = covered;
                out.hyphenate = false;
                out.colSpan = out.rowSpan = 1;
                if (!covered) {
                    out.colSpan = int(std::clamp(lengthOr(ca.get("table:number-columns-spanned", QStringLiteral("1")), 1), 1.0, double(nc - c)));
                    out.rowSpan = int(std::clamp(lengthOr(ca.get("table:number-rows-spanned", QStringLiteral("1")), 1), 1.0, double(nr - r)));
                }
                if (twin) {
                    const bool room = tableChars + twin->chars <= kMaxTableChars;
                    textCut = textCut || !room;
                    out.storyId = room && !covered ? m_doc.copyStory(twin->cell.storyId) : m_doc.createStory();
                    if (room) tableChars += twin->chars;
                    here[c] = *twin;
                    continue;
                }
                Bag cellBag = styles.resolve(QStringLiteral("table-cell"), styleName);
                const Bag fillBag = [&] {
                    // The fill is said in the cell properties of a file from the standard's own, and as a shape's in the one from others.
                    Bag b;
                    for (const char *key : {"draw:fill", "draw:fill-color", "draw:opacity", "draw:fill-gradient-name", "draw:fill-image-name", "draw:fill-hatch-name", "style:repeat"})
                        if (cellBag.has(Cell, key)) b.values[Graphic].insert(QLatin1String(key), cellBag.get(Cell, key));
                    // A color with no word on whether it fills is a fill.
                    if (!b.has(Graphic, "draw:fill") && b.has(Graphic, "draw:fill-color")) b.values[Graphic].insert(QStringLiteral("draw:fill"), QStringLiteral("solid"));
                    if (!b.has(Graphic, "draw:fill") && cellBag.has(Cell, "fo:background-color")) {
                        const QString color = cellBag.get(Cell, "fo:background-color");
                        b.values[Graphic].insert(QStringLiteral("draw:fill"), color == QLatin1String("transparent") ? QStringLiteral("none") : QStringLiteral("solid"));
                        b.values[Graphic].insert(QStringLiteral("draw:fill-color"), color);
                    }
                    return b;
                }();
                out.fill = covered ? Fill() : fillOf(fillBag, styles);
                const double pad = lengthOr(cellBag.get(Cell, "fo:padding"), 0);
                out.margins = QMarginsF(lengthOr(cellBag.get(Cell, "fo:padding-left"), pad), lengthOr(cellBag.get(Cell, "fo:padding-top"), pad),
                                        lengthOr(cellBag.get(Cell, "fo:padding-right"), pad), lengthOr(cellBag.get(Cell, "fo:padding-bottom"), pad));
                const QString align = cellBag.has(Cell, "style:vertical-align") ? cellBag.get(Cell, "style:vertical-align") : cellBag.get(Cell, "draw:textarea-vertical-align");
                out.valign = align == QLatin1String("middle") ? VAlign::Middle : align == QLatin1String("bottom") ? VAlign::Bottom : VAlign::Top;
                // Borders are said on the cell, or (in a file from the standard's reader of tables) in its paragraph properties.
                auto side = [&](const char *which) {
                    const QString specific = QStringLiteral("fo:border-") + QLatin1String(which), whole = QStringLiteral("fo:border");
                    for (Kind k : {Cell, Para}) {
                        if (cellBag.values[k].contains(specific)) return borderOf(cellBag.values[k].value(specific));
                        if (cellBag.values[k].contains(whole)) return borderOf(cellBag.values[k].value(whole));
                    }
                    return Stroke::none();
                };
                out.border.left = side("left");
                out.border.right = side("right");
                out.border.top = side("top");
                out.border.bottom = side("bottom");
                // The text, as far as the table's budget goes.
                out.storyId = m_doc.createStory();
                qsizetype chars = 0;
                if (!covered) {
                    chars = cell.text().size();
                    if (tableChars + chars > kMaxTableChars) {
                        textCut = true;
                        chars = 0;
                    } else {
                        Bag textBag;
                        styles.applyDefaults(textBag, QStringLiteral("graphic"));
                        textBag.values[Text].insert(cellBag.values[Text]);
                        textBag.values[Para].insert(cellBag.values[Para]);
                        TextEnv env;
                        env.styles = &styles;
                        env.base = textBag;
                        QColor behind = sc.background;
                        if (out.fill.type == Fill::Solid && out.fill.transparency < 0.5 && out.fill.color.kind() == ColorRef::Rgb) behind = out.fill.color.rgbValue();
                        env.autoColor = behind.lightness() < 128 ? QColor(Qt::white) : QColor(Qt::black);
                        fillStory(out.storyId, cell, env);
                        tableChars += chars;
                    }
                }
                here[c] = Made{true, cell, styleName, covered, out, chars};
            }
        }
        std::swap(above, here);
    }
    if (textCut && !m_said.contains(QStringLiteral("tabletext"))) {
        note(say("A table holds more text than can be read; the rest is left out."));
        m_said.insert(QStringLiteral("tabletext"));
    }
    // A merged cell covers the ones its spans reach.
    for (int r = 0; r < nr; ++r)
        for (int c = 0; c < nc; ++c) {
            const TableCell origin = t->cell(r, c);
            if (origin.covered) continue;
            for (int rr = r; rr < r + origin.rowSpan; ++rr)
                for (int cc = c; cc < c + origin.colSpan; ++cc) {
                    if (rr == r && cc == c) continue;
                    TableCell &under = t->cell(rr, cc);
                    under.covered = true;
                    under.rowSpan = under.colSpan = 1;
                }
        }

    const QRectF tableAt(lengthOr(a.get("svg:x"), 0), lengthOr(a.get("svg:y"), 0), 0, 0);
    const QTransform total = (a.has("draw:transform") ? parseTransform(a.get("draw:transform")) : QTransform()) * parentTransform;
    const QPointF origin = total.map(tableAt.topLeft());
    t->rect = QRectF(position(origin.x()), position(origin.y()), 0, 0);
    t->syncRect();
    t->fx = effectsOf(frameBag);
    fitRows(*t);
    int content = sc.contentIndex;
    t->role = roleOf(a.get("presentation:class"), sc, &content);
    sc.contentIndex = content;
    return t;
}

// The height a row has in the file is the least it can be: it grows to hold the text of its cells, and the table with it.
// Cells that span rows, or turn their text, don't make it grow.
void Reader::fitRows(TableItem &t)
{
    if (!t.growToFit) return;
    LayoutEnv env;
    env.colors = m_doc.colors;
    env.fonts = m_doc.fonts;
    env.defaultTab = m_doc.defaultTabStop;
    bool grew = false, spent = false;
    qsizetype fitted = 0;
    for (int r = 0; r < t.rows && !spent; ++r) {
        double need = 0;
        for (int c = 0; c < t.cols; ++c) {
            const TableCell &cell = t.cell(r, c);
            const QTextDocument *story = cell.covered || cell.rowSpan > 1 || cell.vertical || cell.divided() ? nullptr : m_doc.storyDoc(cell.storyId);
            if (!story) continue;
            // Past the limits, the rows keep the heights the file gives them.
            const qsizetype chars = story->characterCount();
            if (chars > kMaxFitCellChars || (fitted += chars) > kMaxFitChars) {
                spent = fitted > kMaxFitChars;
                if (!m_said.contains(QStringLiteral("tablefit"))) note(say("The rows of a table with a great deal of text keep the heights the file gives them."));
                m_said.insert(QStringLiteral("tablefit"));
                if (spent) break;
                continue;
            }
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

// ---- shapes

ItemPtr Reader::withGeometry(ItemPtr base, const office::Geometry &geometry, const Fill &fill, const Stroke &stroke)
{
    auto &shape = static_cast<ShapeItem &>(*base);
    const bool filled = !fill.isNone(), stroked = !stroke.isNone();
    const auto fillOf = [&](const office::GeometryPath &gp) { return filled ? gp.fill : office::GeometryPath::None; };
    const auto strokeOf = [&](const office::GeometryPath &gp) { return stroked && gp.stroke; };
    QVector<office::GeometryPath> shown;
    for (const office::GeometryPath &gp : geometry.paths)
        if (fillOf(gp) != office::GeometryPath::None || strokeOf(gp)) shown.append(gp);
    if (shown.isEmpty()) shown = geometry.paths;
    std::vector<QPainterPath> faces;
    std::vector<office::GeometryPath::Fill> faceFill;
    std::vector<bool> faceStroke;
    for (const office::GeometryPath &gp : shown) {
        size_t i = 0;
        while (i < faces.size() && !(faceFill[i] == fillOf(gp) && faceStroke[i] == strokeOf(gp))) ++i;
        if (i == faces.size()) {
            faces.emplace_back();
            faceFill.push_back(fillOf(gp));
            faceStroke.push_back(strokeOf(gp));
        }
        faces[i].addPath(gp.path);
    }
    if (faces.empty()) return base;
    ItemList parts;
    for (size_t i = 0; i < faces.size(); ++i) {
        auto s = std::make_shared<ShapeItem>(shape);
        s->id = newId();
        s->customPath = faces[i];
        if (faceFill[i] == office::GeometryPath::None) s->fill = Fill();
        if (!faceStroke[i]) s->stroke = Stroke();
        if (i > 0) {
            s->fx = Effects();
            s->storyId.clear();
        }
        parts.push_back(s);
    }
    if (parts.size() == 1) return parts[0];
    auto g = std::make_shared<GroupItem>();
    g->children = parts;
    g->name = shape.name;
    g->wrap.mode = Wrap::None;
    g->syncRect();
    return g;
}

ItemPtr Reader::readCustomShape(const QDomElement &e, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(e);
    const StyleSheet &styles = *sc.styles;
    const Bag bag = shapeBag(a, styles);
    const QSizeF size(extent(lengthOr(a.get("svg:width"), 0)), extent(lengthOr(a.get("svg:height"), 0)));
    QRectF boxRect;
    const QTransform total = transformOf(a, size, &boxRect, nullptr) * parentTransform;
    const QDomElement geom = kid(e, "draw:enhanced-geometry");
    const Attrs g(geom);
    const QString type = g.get("draw:type");

    ShapeItem base;
    decorate(base, a, bag, styles);
    place(base, boxRect, total);
    const QSizeF frame = base.rect.size();
    const double pad = lengthOr(bag.get(Graphic, "fo:padding"), 0);
    base.insets = QMarginsF(lengthOr(bag.get(Graphic, "fo:padding-left"), pad), lengthOr(bag.get(Graphic, "fo:padding-top"), pad),
                            lengthOr(bag.get(Graphic, "fo:padding-right"), pad), lengthOr(bag.get(Graphic, "fo:padding-bottom"), pad));
    const QString anchor = bag.get(Graphic, "draw:textarea-vertical-align");
    base.valign = anchor == QLatin1String("middle") ? VAlign::Middle : anchor == QLatin1String("bottom") ? VAlign::Bottom : VAlign::Top;
    base.shape = QStringLiteral("rect");

    // The preset the type stands for: the standard's own names, and the shape library or its definition for the preset.
    const QString prst = presetOfType(type);
    const bool ooxml = type.startsWith(QLatin1String("ooxml-"));
    QHash<QString, double> adjust;
    {
        const QStringList mods = g.get("draw:modifiers").simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
        for (int i = 0; i < mods.size() && i < 16; ++i) {
            double v = 0;
            if (!parseNumber(mods[i], &v)) continue;
            adjust.insert(QStringLiteral("adj%1").arg(i + 1), v);
            if (i == 0) adjust.insert(QStringLiteral("adj"), v);
        }
    }
    const QString libraryShape = prst.isEmpty() ? QString() : office::shapeForPreset(prst);
    office::Geometry geometry;
    bool usePath = false;
    if (!prst.isEmpty()) {
        // A shape of the standard's own naming that has had its handles moved is drawn from its path, which says where they are.
        usePath = !ooxml && !adjust.isEmpty() && !g.get("draw:enhanced-path").isEmpty();
        if (!usePath && ((office::prefersPresetGeometry(prst) || libraryShape.isEmpty()) && office::hasPresetGeometry(prst)))
            geometry = office::presetGeometry(prst, ooxml ? adjust : QHash<QString, double>(), frame);
    } else {
        usePath = true;
    }
    if (usePath) geometry = enhancedGeometry(geom, frame);
    if (!geometry.isEmpty() && !geometry.textRect.isNull()) {
        const QRectF tr = geometry.textRect;
        base.insets = QMarginsF(std::max(0.0, base.insets.left() + tr.left()), std::max(0.0, base.insets.top() + tr.top()),
                                std::max(0.0, base.insets.right() + frame.width() - tr.right()), std::max(0.0, base.insets.bottom() + frame.height() - tr.bottom()));
    }
    if (geometry.isEmpty() && !libraryShape.isEmpty()) {
        const ShapeDef *def = shapeDef(libraryShape);
        if (def && !def->open) {
            base.shape = libraryShape;
            if (ooxml) base.adj = office::libraryAdjustments(prst, adjust, frame);
        }
    } else if (geometry.isEmpty() && !prst.isEmpty() && !libraryShape.isEmpty()) {
        base.shape = libraryShape;
    }

    // The text.
    const bool text = hasText(e);
    ItemPtr result = std::make_shared<ShapeItem>(base);
    auto &shape = static_cast<ShapeItem &>(*result);
    shape.id = newId();
    if (text) {
        shape.storyId = m_doc.createStory();
        TextEnv env = textEnvFor(sc, a, bag, QString());
        fillStory(shape.storyId, e, env);
    }
    if (!geometry.isEmpty()) result = withGeometry(result, geometry, base.fill, base.stroke);
    return result;
}

ItemPtr Reader::readBasicShape(const QDomElement &e, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(e);
    const StyleSheet &styles = *sc.styles;
    const Bag bag = shapeBag(a, styles);
    const QString tag = tagOf(e);
    QSizeF size(extent(lengthOr(a.get("svg:width"), 0)), extent(lengthOr(a.get("svg:height"), 0)));
    if (tag == QLatin1String("draw:circle") && size.isEmpty()) {
        const double r = lengthOr(a.get("svg:r"), 0);
        size = QSizeF(2 * r, 2 * r);
    }
    QRectF boxRect;
    const QTransform total = transformOf(a, size, &boxRect, nullptr) * parentTransform;
    auto s = std::make_shared<ShapeItem>();
    decorate(*s, a, bag, styles);
    place(*s, boxRect, total);
    const double pad = lengthOr(bag.get(Graphic, "fo:padding"), 0);
    s->insets = QMarginsF(lengthOr(bag.get(Graphic, "fo:padding-left"), pad), lengthOr(bag.get(Graphic, "fo:padding-top"), pad),
                          lengthOr(bag.get(Graphic, "fo:padding-right"), pad), lengthOr(bag.get(Graphic, "fo:padding-bottom"), pad));
    const QString anchor = bag.get(Graphic, "draw:textarea-vertical-align");
    s->valign = anchor == QLatin1String("middle") ? VAlign::Middle : anchor == QLatin1String("bottom") ? VAlign::Bottom : VAlign::Top;
    if (tag == QLatin1String("draw:rect") || tag == QLatin1String("draw:caption")) {
        s->shape = QStringLiteral("rect");
        const double radius = std::max(lengthOr(a.get("svg:rx"), 0), lengthOr(a.get("draw:corner-radius"), 0));
        if (radius > 0 && std::min(s->rect.width(), s->rect.height()) > 0) {
            s->shape = QStringLiteral("roundRect");
            s->adj = {std::clamp(radius / std::min(s->rect.width(), s->rect.height()), 0.0, 0.5)};
        }
    } else {
        // An ellipse, or the part of one the angles name.
        const QString kind = a.get("draw:kind", QStringLiteral("full"));
        s->shape = QStringLiteral("ellipse");
        double start = 0, end = 360;
        parseNumber(a.get("draw:start-angle"), &start);
        parseNumber(a.get("draw:end-angle"), &end);
        if (kind != QLatin1String("full") && s->rect.width() > 0 && s->rect.height() > 0) {
            const QRectF r(0, 0, s->rect.width(), s->rect.height());
            double sweep = std::fmod(end - start + 720, 360.0);
            if (sweep == 0) sweep = 360;
            QPainterPath path;
            if (kind == QLatin1String("section")) {
                path.moveTo(r.center());
                path.arcTo(r, start, sweep);
                path.closeSubpath();
            } else if (kind == QLatin1String("cut")) {
                path.arcTo(r, start, sweep);
                path.closeSubpath();
            } else {
                path.arcTo(r, start, sweep);
                if (!s->stroke.isNone()) s->fill = Fill();
            }
            s->customPath = path;
        }
    }
    if (hasText(e)) {
        s->storyId = m_doc.createStory();
        fillStory(s->storyId, e, textEnvFor(sc, a, bag, QString()));
    }
    return s;
}

ItemPtr Reader::readLine(const QDomElement &e, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(e);
    const StyleSheet &styles = *sc.styles;
    const Bag bag = shapeBag(a, styles);
    const bool connector = tagOf(e) == QLatin1String("draw:connector");
    auto l = std::make_shared<LineItem>();
    l->name = a.get("draw:name");
    l->wrap.mode = Wrap::None;
    l->stroke = strokeOf(bag, styles);
    l->fx = effectsOf(bag);
    const QTransform total = (a.has("draw:transform") ? parseTransform(a.get("draw:transform")) : QTransform()) * parentTransform;
    const QPointF p1 = total.map(QPointF(lengthOr(a.get("svg:x1"), 0), lengthOr(a.get("svg:y1"), 0)));
    const QPointF p2 = total.map(QPointF(lengthOr(a.get("svg:x2"), 0), lengthOr(a.get("svg:y2"), 0)));
    l->p1 = QPointF(position(p1.x()), position(p1.y()));
    l->p2 = QPointF(position(p2.x()), position(p2.y()));
    // A connector that the file draws with more than a straight line is the path it gives.
    if (connector && a.has("svg:d")) {
        QPainterPath path = svg::pathData(a.get("svg:d"));
        if (path.elementCount() > 2) {
            const QPointF first = path.elementAt(0);
            const QPointF start(lengthOr(a.get("svg:x1"), 0), lengthOr(a.get("svg:y1"), 0));
            constexpr double unit = 72.0 / 2540;   // the path's numbers are thousandths of a centimeter
            QTransform into;
            if (std::hypot(first.x() * unit - start.x(), first.y() * unit - start.y()) < 1.0) {
                into.scale(unit, unit);
            } else {
                // Else they are within the view box, which is stretched over the ends.
                const QStringList box = a.get("svg:viewBox").simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
                double v[4] = {0, 0, 0, 0};
                bool ok = box.size() == 4;
                for (int i = 0; ok && i < 4; ++i) ok = parseNumber(box[i], &v[i]);
                const QRectF ends = QRectF(start, QPointF(lengthOr(a.get("svg:x2"), 0), lengthOr(a.get("svg:y2"), 0))).normalized();
                if (ok && v[2] > 0 && v[3] > 0) {
                    into.translate(ends.left(), ends.top());
                    into.scale(ends.width() / v[2], ends.height() / v[3]);
                    into.translate(-v[0], -v[1]);
                } else {
                    into.scale(unit, unit);
                }
            }
            path = (into * total).map(path);
            const QRectF bounds = path.boundingRect();
            auto shape = std::make_shared<ShapeItem>();
            shape->name = l->name;
            shape->wrap.mode = Wrap::None;
            shape->fill = Fill();
            shape->stroke = l->stroke;
            shape->fx = l->fx;
            shape->rect = QRectF(position(bounds.x()), position(bounds.y()), extent(bounds.width()), extent(bounds.height()));
            shape->customPath = QTransform::fromTranslate(-bounds.x(), -bounds.y()).map(path);
            return shape;
        }
    }
    if (connector) {
        const QString type = a.get("draw:type", QStringLiteral("standard"));
        l->route = type == QLatin1String("curve") ? LineItem::Curved : type == QLatin1String("line") || type == QLatin1String("lines") ? LineItem::Straight : LineItem::Elbow;
        if (type == QLatin1String("lines")) l->route = LineItem::Elbow;
        Scope::Pending p;
        p.line = l;
        p.startId = a.get("draw:start-shape");
        p.endId = a.get("draw:end-shape");
        parseNumber(a.get("draw:start-glue-point"), &p.startSite);
        parseNumber(a.get("draw:end-glue-point"), &p.endSite);
        if (!p.startId.isEmpty() || !p.endId.isEmpty()) sc.pending.append(p);
    }
    l->syncRect();
    return l;
}

ItemPtr Reader::readPathShape(const QDomElement &e, Scope &sc, const QTransform &parentTransform)
{
    const Attrs a(e);
    const StyleSheet &styles = *sc.styles;
    const Bag bag = shapeBag(a, styles);
    const QString tag = tagOf(e);
    const QSizeF size(extent(lengthOr(a.get("svg:width"), 0)), extent(lengthOr(a.get("svg:height"), 0)));
    QRectF boxRect;
    const QTransform total = transformOf(a, size, &boxRect, nullptr) * parentTransform;

    QRectF view(0, 0, size.width(), size.height());
    {
        const QStringList box = a.get("svg:viewBox").simplified().split(QRegularExpression(QStringLiteral("[\\s,]+")), Qt::SkipEmptyParts);
        double v[4] = {0, 0, 0, 0};
        bool ok = box.size() == 4;
        for (int i = 0; ok && i < 4; ++i) ok = parseNumber(box[i], &v[i]);
        if (ok && v[2] > 0 && v[3] > 0) view = QRectF(v[0], v[1], v[2], v[3]);
    }
    QPainterPath path;
    if (tag == QLatin1String("draw:path")) {
        path = svg::pathData(a.get("svg:d"));
    } else {
        const QStringList points = a.get("draw:points", a.get("svg:points")).simplified().split(QRegularExpression(QStringLiteral("[\\s,]+")), Qt::SkipEmptyParts);
        for (int i = 0; i + 1 < points.size() && i < 20000; i += 2) {
            double x = 0, y = 0;
            if (!parseNumber(points[i], &x) || !parseNumber(points[i + 1], &y)) continue;
            if (path.elementCount() == 0) path.moveTo(x, y);
            else path.lineTo(x, y);
        }
        if (tag == QLatin1String("draw:polygon") && path.elementCount() > 0) path.closeSubpath();
    }
    if (path.isEmpty() || view.width() <= 0 || view.height() <= 0) return nullptr;
    QTransform scale;
    scale.scale(size.width() / view.width(), size.height() / view.height());
    scale.translate(-view.left(), -view.top());
    path = scale.map(path);

    auto s = std::make_shared<ShapeItem>();
    decorate(*s, a, bag, styles);
    place(*s, boxRect, total);
    s->shape = QStringLiteral("rect");
    s->customPath = path;
    // A line that doesn't close takes no fill.
    if (tag == QLatin1String("draw:polyline") && !s->stroke.isNone()) s->fill = Fill();
    if (hasText(e)) {
        s->storyId = m_doc.createStory();
        fillStory(s->storyId, e, textEnvFor(sc, a, bag, QString()));
    }
    return s;
}

ItemPtr Reader::readGroup(const QDomElement &e, Scope &sc, const QTransform &parentTransform)
{
    if (sc.depth >= kMaxDepth) return nullptr;
    const Attrs a(e);
    const QTransform total = (a.has("draw:transform") ? parseTransform(a.get("draw:transform")) : QTransform()) * parentTransform;
    auto g = std::make_shared<GroupItem>();
    ++sc.depth;
    readShapes(e, sc, g->children, total);
    --sc.depth;
    if (g->children.empty()) return nullptr;
    g->name = a.get("draw:name");
    g->wrap.mode = Wrap::None;
    g->syncRect();
    return g;
}

} // namespace jp::deck::odp
