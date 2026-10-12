#pragma once
// What the parts of the open document presentation reader share
// (odpimport.h): the elements and attributes by the names the standard gives
// them, the styles and what they come to once their parents are taken in,
// and the reader itself, whose members are in odpimport*.cpp.

#include "core/deck.h"
#include "core/document.h"
#include "office/drawingml.h"
#include "office/drawingml_geometry.h"
#include "office/package.h"
#include "office/xml.h"

#include <QColor>
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QJsonArray>
#include <QMap>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QVector>
#include <deque>
#include <functional>
#include <memory>

namespace jp::deck::odp {

// ---- the elements and attributes

// The prefix the standard uses for a namespace, for any prefix a file chose:
// every name below is written the usual way ("draw:frame", "svg:width").
QString canonicalPrefix(const QString &uri);
// "draw:frame" for an element, "svg:width" for an attribute.
QString tagOf(const QDomElement &e);

// An element's attributes by the usual names; one pass over them at the start,
// so the many lookups a shape needs cost little.
class Attrs {
public:
    Attrs() = default;
    explicit Attrs(const QDomElement &e);
    QString get(const char *name, const QString &fallback = QString()) const { return m_values.value(QLatin1String(name), fallback); }
    bool has(const char *name) const { return m_values.contains(QLatin1String(name)); }
    const QHash<QString, QString> &all() const { return m_values; }

private:
    QHash<QString, QString> m_values;
};

// The first child element with that usual name; null if none.
QDomElement kid(const QDomElement &e, const char *name);
// The children with that usual name, in order.
QVector<QDomElement> kids(const QDomElement &e, const char *name);
// The attribute of an element, by its usual name.
QString at(const QDomElement &e, const char *name, const QString &fallback = QString());

// ---- values

// A length in points; "1.5cm", "12mm", "0.2in", "18pt", "3pc", "16px", "1m". False for
// anything else, a percentage included.
bool parseLength(const QString &s, double *points);
double lengthOr(const QString &s, double fallback);
// "45%" as 0.45; false for a value without a percent sign.
bool parsePercent(const QString &s, double *fraction);
// "#rrggbb" or a color name; invalid for "transparent" or anything unreadable.
QColor parseColor(const QString &s);
// "1.5" as a number; false for anything else, and for what isn't finite.
bool parseNumber(const QString &s, double *value);
// ISO 8601 durations such as "PT00H00M05S" or "PT5S" in seconds; false for anything else.
bool parseDuration(const QString &s, double *seconds);

// ---- styles

enum Kind { Text, Para, Graphic, Cell, Row, Column, Table, KindCount };

// One style:style, as written: the properties of each kind by their usual names.
struct StyleDef {
    QString name, family, parent, listStyle, displayName;
    QHash<QString, QString> values[KindCount];
    QDomElement tabStops;           // style:tab-stops of its paragraph properties
    QDomElement graphicListStyle;   // text:list-style inside its graphic properties (the bullets of a presentation style)
};

// The properties that come of a style and the styles it is based on.
struct Bag {
    QHash<QString, QString> values[KindCount];
    QDomElement tabStops, graphicListStyle;
    QString listStyle;
    QString get(Kind k, const char *name, const QString &fallback = QString()) const { return values[k].value(QLatin1String(name), fallback); }
    bool has(Kind k, const char *name) const { return values[k].contains(QLatin1String(name)); }
    // The properties of another bag on top of these.
    void overlay(const Bag &top);
};

// A level of a list style: the label a paragraph at that level has.
struct ListLevel {
    enum Type { None, Bullet, Number, Image } type = None;
    QString bulletChar, numFormat, prefix, suffix;
    int startValue = 1;
    double spaceBefore = 0, minLabelWidth = 0;
    bool labelAlignment = false;
    double marginLeft = 0, textIndent = 0;
    QHash<QString, QString> text;   // the label's text properties (font, color, size)
    double relativeSize = 0;        // bullet-relative-size as a fraction, 0 for none
};

// The styles of one part, and those they are based on in the common styles.
class StyleSheet {
public:
    struct Common {
        QHash<QString, StyleDef> styles;               // by family and name
        QHash<QString, StyleDef> defaults;             // by family
        QHash<QString, QDomElement> lists, gradients, hatches, images, dashes, markers, opacities, pageLayouts;
        QHash<QString, QString> fonts;                 // font-face name -> family
    };
    explicit StyleSheet(std::shared_ptr<Common> common = std::make_shared<Common>()) : m_common(std::move(common)) {}

    // office:styles (common) or office:automatic-styles; and the font-face-decls.
    void read(const QDomElement &styles, bool automatic);
    void readFonts(const QDomElement &decls);

    const StyleDef *find(const QString &family, const QString &name) const;
    // The properties of a style of that family with the defaults beneath and its parents: merged onto `bag`.
    void apply(Bag &bag, const QString &family, const QString &name) const;
    void applyDefaults(Bag &bag, const QString &family) const;
    Bag resolve(const QString &family, const QString &name) const;

    QDomElement list(const QString &name) const;
    QDomElement gradient(const QString &name) const { return lookup(&Common::gradients, m_gradients, name); }
    QDomElement hatch(const QString &name) const { return lookup(&Common::hatches, m_hatches, name); }
    QDomElement fillImage(const QString &name) const { return lookup(&Common::images, m_images, name); }
    QDomElement dash(const QString &name) const { return lookup(&Common::dashes, m_dashes, name); }
    QDomElement marker(const QString &name) const { return lookup(&Common::markers, m_markers, name); }
    QDomElement opacity(const QString &name) const { return lookup(&Common::opacities, m_opacities, name); }
    QDomElement pageLayout(const QString &name) const { return lookup(&Common::pageLayouts, m_pageLayouts, name); }
    QString fontFamily(const QString &faceName) const;
    const std::shared_ptr<Common> &common() const { return m_common; }

private:
    using Map = QHash<QString, QDomElement>;
    QDomElement lookup(Map Common::*member, const Map &own, const QString &name) const
    {
        const auto it = own.constFind(name);
        if (it != own.cend()) return it.value();
        return ((*m_common).*member).value(name);
    }
    StyleDef makeStyle(const QDomElement &e) const;
    void merge(Bag &bag, const StyleDef &s) const;
    void applyFrom(Bag &bag, const QString &family, const QString &name, int depth) const;

    std::shared_ptr<Common> m_common;
    QHash<QString, StyleDef> m_styles;                 // the automatic ones, by family and name
    Map m_lists, m_gradients, m_hatches, m_images, m_dashes, m_markers, m_opacities, m_pageLayouts;
};

// ---- the shapes of a drawing that are not the library's

// The paths of a draw:enhanced-geometry (its path commands and equations), in
// points within a frame of `size`; empty if it holds none that can be drawn.
office::Geometry enhancedGeometry(const QDomElement &geometry, const QSizeF &size);
// The preset (the standard's name, "rightArrow") that an enhanced geometry's
// type stands for, with its adjustments by name when `modifiers` give them:
// the ones named "ooxml-*", and the usual shapes named as the standard does
// ("rectangle", "right-arrow"). Empty for a type with no preset.
QString presetOfType(const QString &type);

// Whether a link in a file is one to follow: to a page on the web or a mail address, not to a program or a file.
bool isWebLink(const QString &href);
// What a master's empty placeholder of that class says on the master itself.
QString masterPrompt(const QString &cls);

// ---- the reader

class Reader {
public:
    Reader(const office::Package &package, Document &doc, QStringList *notes);
    bool run(QString *error);

private:
    friend struct TextEnv;
    // One style:master-page and what it became.
    struct MasterInfo {
        QDomElement element;
        QString name, displayName, layoutName;
        std::shared_ptr<MasterPage> page;
        QMap<QString, std::shared_ptr<MasterPage>> layouts;   // by key, in the order made
        QStringList layoutOrder;
        QColor backgroundColor = Qt::white;
    };
    // The page being read: where its objects go and which styles name them.
    struct Scope {
        enum Kind { Slide, Master, Layout, Notes } kind = Slide;
        const StyleSheet *styles = nullptr;
        PageBase *page = nullptr;
        MasterInfo *master = nullptr;
        int shapes = 0;
        int depth = 0;
        int contentIndex = 0;          // the content places met so far
        QColor background = Qt::white; // what the page is, for the color of text that names none
        // A connector and the objects it was drawn between, by the numbers the file gives them.
        struct Pending {
            std::shared_ptr<LineItem> line;
            QString startId, endId;
            double startSite = -1, endSite = -1;
        };
        QVector<Pending> pending;
        QHash<QString, ItemPtr> byId, byName;
    };
    // The text of an object: the properties its paragraphs start from.
    struct TextEnv {
        const StyleSheet *styles = nullptr;
        Bag base;
        QString cls;                // the presentation class of the frame, "" for none
        QString presentationStyle;  // the frame's presentation style, whose outline levels are named from it
        QString textStyle;
        bool outline = false;       // the text of a placeholder's outline: lists without a style of their own take its levels' bullets
        QColor autoColor;           // the color of text that names none: black on light, white on dark
        bool vertical = false;
    };

    // parts and notes
    void buildLinks();
    bool parse(const QString &part, QDomDocument *dom, QString *why);
    void note(const QString &text);
    office::DrawingContext drawingContext();

    // odpimport_style.cpp
    QString loadPicture(const QString &href, QString *format = nullptr);
    Fill fillOf(const Bag &bag, const StyleSheet &styles);
    Stroke strokeOf(const Bag &bag, const StyleSheet &styles);
    Effects effectsOf(const Bag &bag);
    Fill gradientFill(const QDomElement &gradient, double transparency, double endTransparency, bool varyTransparency);

    // odpimport_text.cpp
    QString textXml(const QDomElement &body, const TextEnv &env);
    void fillStory(const QString &storyId, const QDomElement &body, const TextEnv &env);
    TextEnv textEnvFor(const Scope &sc, const Attrs &frame, const Bag &shapeBag, const QString &cls);
    bool hasText(const QDomElement &body) const;

    // odpimport_chart.cpp: the picture (an image id) of a chart object, drawn from its numbers; empty if it is no chart that can be drawn.
    QString chartImage(const QDomElement &object, const QSizeF &size);

    // odpimport_shape.cpp
    void readShapes(const QDomElement &parent, Scope &sc, ItemList &out, const QTransform &parentTransform);
    ItemPtr readShape(const QDomElement &e, Scope &sc, const QTransform &parentTransform);
    ItemPtr readFrame(const QDomElement &e, Scope &sc, const QTransform &parentTransform);
    ItemPtr readTextBox(const QDomElement &frame, const QDomElement &box, Scope &sc, const QTransform &t);
    ItemPtr readPicture(const QDomElement &frame, const QDomElement &image, Scope &sc, const QTransform &t);
    ItemPtr readTableFrame(const QDomElement &frame, const QDomElement &table, Scope &sc, const QTransform &t);
    void fitRows(TableItem &t);
    ItemPtr readCustomShape(const QDomElement &e, Scope &sc, const QTransform &t);
    ItemPtr readBasicShape(const QDomElement &e, Scope &sc, const QTransform &t);
    ItemPtr readLine(const QDomElement &e, Scope &sc, const QTransform &t);
    ItemPtr readPathShape(const QDomElement &e, Scope &sc, const QTransform &t);
    ItemPtr readGroup(const QDomElement &e, Scope &sc, const QTransform &t);
    void decorate(Item &it, const Attrs &a, const Bag &bag, const StyleSheet &styles);
    Bag shapeBag(const Attrs &a, const StyleSheet &styles);
    QTransform transformOf(const Attrs &a, const QSizeF &size, QRectF *box, bool *plain);
    void place(Item &it, const QRectF &box, const QTransform &total);
    ItemPtr withGeometry(ItemPtr base, const office::Geometry &geometry, const Fill &fill, const Stroke &stroke);
    QString roleOf(const QString &cls, const Scope &sc, int *contentIndex);

    // odpimport.cpp
    void readMeta();
    bool readSize();
    void readMasters();
    MasterInfo *masterByName(const QString &name);
    void readMasterPage(MasterInfo &m, int number);
    std::shared_ptr<MasterPage> layoutFor(MasterInfo &m, const ItemList &items);
    std::shared_ptr<MasterPage> standardLayout(MasterInfo &m, const QString &type);
    ItemPtr promptItem(MasterInfo &m, const QString &cls, const QString &role, const QRectF &rect, const QString &text);
    void readSlides(const QDomElement &presentation);
    bool readSlide(const QDomElement &pageElement, int number);
    void readNotes(const QDomElement &pageElement, Page &page);
    void readTransition(const Bag &pageStyle, Page &page);
    void readFooters(const QDomElement &pageElement, const Bag &pageStyle, Page &page);
    ItemPtr samplePlaceholder(MasterInfo &m, const QString &cls, const QRectF &rect, const QString &text);

    const office::Package &m_pkg;
    Document &m_doc;
    QStringList *m_notes;
    std::deque<QDomDocument> m_doms;
    QDomDocument m_contentDom, m_stylesDom, m_metaDom;
    QDomElement m_content, m_styles, m_meta;
    std::shared_ptr<StyleSheet::Common> m_common;
    std::unique_ptr<StyleSheet> m_contentSheet, m_masterSheet;
    std::vector<std::unique_ptr<MasterInfo>> m_masters;
    QHash<QString, QString> m_pictures;     // part name -> image id
    QHash<QString, QString> m_linkIds;      // address -> the id of its relationship in m_links
    office::Package m_links;
    QVector<QPair<Page *, FooterOptions>> m_pendingFooters;
    QSet<QString> m_said;
    QDomElement m_presentation;
    int m_layoutCount = 0;
    bool m_footers = false;
    QHash<QString, QString> m_footerDecls, m_dateDecls;
    int m_transitionsLost = 0, m_animations = 0;
    QSizeF m_slideSize;
    Fill m_pageFill;                        // the background of the page being read, for objects filled with it
};

} // namespace jp::deck::odp
