#pragma once
// What the parts of the open document presentation writer share
// (odpexport.h): the XML it is written with, the styles that are made as
// they are asked for, and the writer itself, whose members are in
// odpexport*.cpp.

#include "core/animation.h"
#include "core/deck.h"
#include "core/document.h"
#include "core/showsettings.h"
#include "core/transition.h"

#include <QColor>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QVector>
#include <memory>
#include <vector>

namespace jp::deck::odpw {

// ---- numbers and text for XML

// Text or an attribute's value with markup escaped, and what XML 1.0 can't hold (control characters, lone surrogates) left out.
QString esc(const QString &text);
// A length in points as the file writes it: "2.54cm".
QString cm(double points);
// "45%" for 0.45.
QString percent(double fraction);
// "#rrggbb".
QString hexOf(const QColor &c);
// The presentation class (title, subtitle, outline, date-time, footer, page-number, graphic...) a placeholder of that type ("title", "ctrTitle",
// "body", "dt", ...) has; empty for a type with none.
QString holderClassOf(const QString &type);
// A name for a style from the name people see (a space becomes "_20_", as the other office suite does).
QString styleNameOf(const QString &display);
// A number with no more digits than it needs.
QString number(double v, int decimals = 4);

// The XML of a part, written as it goes: an element's attributes are added until something is put in it.
class X {
public:
    X &open(const char *name);
    X &attr(const char *name, const QString &value);
    X &attr(const char *name, const char *value) { return attr(name, QString::fromUtf8(value)); }
    X &attr(const char *name, int value) { return attr(name, QString::number(value)); }
    // A length (given in points) in centimeters.
    X &len(const char *name, double points) { return attr(name, cm(points)); }
    // Attributes as ready-made text, ' name="value"' each.
    X &attrs(const QString &raw);
    X &close();
    X &text(const QString &t);
    // Ready-made XML.
    X &raw(const QString &xml);
    // An element with nothing in it.
    X &leaf(const char *name) { return open(name).close(); }
    const QString &str() const { return m_out; }

private:
    void endTag();
    QString m_out;
    QVector<QString> m_stack;
    bool m_inTag = false;
};

// Styles, and the other named things that are made as they are first asked for (the same again gets the same name): `before` is the
// XML up to the name, `after` the rest from the end of the name's quotes.
class Registry {
public:
    // The name of the entry with that XML, a new one counted up from the prefix if there is none.
    QString add(const QString &prefix, const QString &before, const QString &after);
    // The same under a name that is given (a marker's name tells what it is).
    QString addNamed(const QString &name, const QString &before, const QString &after);
    // A style of the family: `props` are its property elements, `extra` more attributes of the style element.
    QString style(const QString &family, const QString &prefix, const QString &parent, const QString &props, const QString &extra = QString());
    void write(X &x) const;
    bool isEmpty() const { return m_entries.isEmpty(); }

private:
    struct Entry {
        QString name, before, after;
    };
    QVector<Entry> m_entries;
    QHash<QString, int> m_byXml;
    QHash<QString, int> m_counts;
};

// ---- the writer

// What the objects being written know of where they stand.
struct Scope {
    Registry *styles = nullptr;     // where their automatic styles go
    bool master = false;            // on a master page (else a slide)
    QString layer;                  // "layout" for a slide's objects, "backgroundobjects" for a master's
    QString prefix;                 // what the names of the master page's text styles begin with ("Default-")
    QColor background = Qt::white;  // what is behind the text, for the color of text that names none
    QString where;                  // "Slide 3", for the notes on what is left out
    int depth = 0;
};

class Exporter {
public:
    Exporter(const Document &doc, bool asTemplate, QStringList *skipped);
    QByteArray run(QString *error);

private:
    // One style:master-page: the slide master with, when the layout has graphics of its own, that layout.
    struct MasterOut {
        const MasterPage *master = nullptr;
        const MasterPage *layout = nullptr;   // null when the page is the slide master's alone
        QString name, display, prefix;
    };
    // What the text of a master's placeholder looks like at one level.
    struct Level {
        QTextCharFormat chr;
        QTextBlockFormat blk;
        bool listed = false;
        QTextListFormat list;
        bool seen = false;
    };
    struct Footer {
        bool date = false, footer = false, number = false;
        QString footerText, dateText, dateCode;   // dateText is the fixed date; empty with a dateCode for the current one
        int dateDecl = -1, footerDecl = -1;
    };
    struct PlacedItems {
        ItemList decor, holders;
    };

    // ---- odpexport.cpp
    void say(const QString &note);
    void sayOnce(const QString &key, const QString &note);
    void plan();
    QString uniquePageName(const QString &wanted);
    const MasterOut &pageOf(const Page &slide) const;
    PlacedItems itemsOf(const MasterOut &m) const;
    Footer footerOf(const Page &slide);
    bool isFooterHolder(const Item &it, const Footer &f) const;
    void writeMeta();
    void writeStyles(const QString &masterBody, const QString &presentationStyles);
    void writeContent(const QString &body);
    void writeManifest();
    QString writeMasterPages(QString *presentationStyles);
    void writeMasterPage(X &x, const MasterOut &m, QString *presentationStyles);
    void writePresentationStyles(X &out, const MasterOut &m);
    void writeSlides(X &x);
    void writeSlide(X &x, int index);
    void writeNotes(X &x, int index);
    void writeFooterDecls(X &x);
    QString pageStyle(const Page &page, const Footer &f, bool background);
    QString masterPageStyle(const MasterOut &m);
    QString pageLayoutFor(const QSizeF &size);
    QByteArray thumbnail() const;
    QString addPicture(const QString &imageId);
    QString addMedia(const QString &mediaId, QString *mediaType);
    void addFile(const QString &path, const QByteArray &data, const QString &mediaType, bool compress);
    QString fontDecl(const QString &family);
    void writeFontDecls(X &x) const;
    void writeThemeColors(X &x) const;
    void writeDefaultStyles(X &x) const;
    QSizeF notesSize() const { return m_notesSize; }

    // ---- odpexport_style.cpp
    QColor resolve(const ColorRef &c) const;
    // Attributes of style:graphic-properties (or of the drawing page) for a fill, an outline, and effects.
    QString fillAttrs(const Fill &fill, const QSizeF &frame);
    QString strokeAttrs(const Stroke &stroke);
    QString effectAttrs(const Effects &fx);
    QString namedGradient(const Fill &fill, QString *opacityName);
    QString namedDash(const Stroke &stroke);
    QString namedMarker(Arrow arrow);
    QString namedHatch(const Fill &fill);
    QString namedBitmap(const Fill &fill);
    QString dateStyle(Registry &reg, const QString &code, bool *isTime);

    // ---- odpexport_text.cpp
    struct TextOpts {
        bool placeholder = false;   // a placeholder's text
        QColor behind;              // the color of text that names none
        int shape = 0;              // the object's number in the file, for the paragraphs an animation aims at
    };
    void writeStory(X &x, Scope &sc, const QTextDocument *doc, const TextOpts &opts);
    QString charAttrs(const QTextCharFormat &cf, bool field = false);
    QString paraAttrs(const QTextBlockFormat &bf, bool inList);
    QString paraProps(const QTextBlockFormat &bf, bool inList);
    // The text style of an object's frame, which says its lines are set as a slide sets them (1.2 times the text, whatever the font).
    QString slideLinesStyle(Scope &sc);
    QString paraStyleOf(Scope &sc, const QTextBlock &b, bool inList, const TextOpts &opts);
    QString textStyleOf(Scope &sc, const QTextCharFormat &cf, bool field = false);
    QString listStyleFor(Scope &sc, const QVector<QTextBlock> &run);
    QString listLevelXml(int level, const QTextBlockFormat &bf, const QTextListFormat &lf, double firstSize, double *marL, double *indent);
    void writeParagraph(X &x, Scope &sc, const QTextBlock &b, bool inList, const TextOpts &opts);
    QString fontName(const QTextCharFormat &cf) const;
    QColor textColor(const QTextCharFormat &cf, const QColor &behind) const;
    QVector<Level> levelsOf(const QTextDocument *doc) const;

    // ---- odpexport_shape.cpp
    void writeItems(X &x, Scope &sc, const ItemList &items);
    void writeItem(X &x, Scope &sc, const Item &it);
    void writeTextBox(X &x, Scope &sc, const TextItem &t);
    void writeShape(X &x, Scope &sc, const ShapeItem &s);
    void writePicture(X &x, Scope &sc, const PictureItem &p);
    void writeLine(X &x, Scope &sc, const LineItem &l);
    void writeGroup(X &x, Scope &sc, const GroupItem &g);
    void writeTable(X &x, Scope &sc, const TableItem &t);
    void writeAsPicture(X &x, Scope &sc, const Item &it);
    void writePlace(X &x, const QRectF &rect, double rotation);
    void writeCommon(X &x, const Item &it);
    void writeInside(X &x, const Item &it);
    QString idAttr(const Item &it) const;
    QString presentationClass(const Item &it, bool master) const;
    QString frameStyle(Scope &sc, const Item &it, const QString &cls, const QString &extra, const QString &children, const QString &paragraph, bool filled = true);

    // ---- odpexport_motion.cpp
    QString transitionAttrs(const Page &page);
    void writeAnimations(X &x, const Page &page);
    void writeComments(X &x, const Page &page);
    void writeShowSettings(X &x);

    const Document &m_doc;
    bool m_template;
    QStringList *m_callerSkipped;
    QStringList m_skipped;
    QSet<QString> m_said;
    DeckTheme m_theme;
    ColorScheme m_colors;
    FontScheme m_fonts;
    QSizeF m_size;
    QSizeF m_notesSize{540, 720};

    Registry m_contentStyles, m_masterStyles, m_common;
    QStringList m_fontOrder;
    QHash<QString, QString> m_fontDecls;
    std::vector<MasterOut> m_masters;
    QHash<const MasterPage *, int> m_pageOfMaster;    // a layout or slide master -> its page in m_masters
    QHash<QString, QString> m_pageLayouts;            // "w,h" -> the style:page-layout's name
    QStringList m_pageLayoutXml;
    QStringList m_order;                              // the part names in the order they are written
    QHash<QString, QByteArray> m_files;
    QHash<QString, QString> m_mediaTypes;
    QHash<QString, bool> m_compress;
    QHash<QString, QString> m_pictureNames;           // image id -> part name
    QSet<QString> m_pageNames;

    QByteArray m_contentXml, m_stylesXml, m_metaXml, m_manifestXml;

    // The page being written: the ids of the objects something else names (a connector, an animation).
    QHash<QString, QString> m_ids;                    // Item::id -> draw:id
    QHash<int, QString> m_idOfShape;                  // Item::sourceId -> draw:id
    QHash<QPair<int, int>, QString> m_paragraphIds;   // (Item::sourceId, paragraph) -> the id of the paragraph an animation aims at
    QString m_where;                                  // "Slide 3", for the notes on what is left out
    QVector<QString> m_slideNames;
    QSet<QString> m_needIds;
    const Page *m_page = nullptr;                     // the slide being written
    int m_mediaCount = 0;
    int m_idCounter = 0;                              // the ids of the objects count up through the whole presentation
    QVector<Footer> m_footers;
    QStringList m_footerDecls, m_dateDecls;
    QSet<QString> m_flat;                             // item ids written as plain pictures
    const MasterOut *m_currentMaster = nullptr;
    // A slide's text the presentation program shrank is written at the size it was shrunk to (the file can't hold the scale).
    double m_fitScale = 1.0, m_fitCut = 0.0;
};

} // namespace jp::deck::odpw
