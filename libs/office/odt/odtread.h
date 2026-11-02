#pragma once
// The reader of an OpenDocument text: turns the package's parts into the parts of a .docx package (see odt.h). Internal to libs/office/odt.

#include "odt/odt.h"
#include "odt/odtcommon.h"
#include "office/package.h"
#include "wml/xmltree.h"

#include <QHash>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QVector>
#include <memory>

namespace jp::office::odt {

// The properties of a style, by the kind of properties they came from and the attribute's name in the standard's own prefixes ("para:fo:margin-left", "text:fo:font-weight",
// "cell:fo:border-top", "tbl:style:width", "col:style:column-width", "row:style:row-height", "sect:fo:column-count", "page:fo:page-width", "hdr:fo:min-height").
using Props = QHash<QString, QString>;

struct StyleDef {
    QString family, name, parent, display, next, listStyle, masterPage;
    Props props;
    QByteArray tabsXml;        // the style's tab stops, as <w:tabs>
    bool automatic = false;
    bool noList = false;       // style:list-style-name="" : no numbering, whatever the parent says
    int outlineLevel = 0;      // style:default-outline-level
};

struct ListLevel {
    bool defined = false, bullet = false;
    QString numFmt;            // the standard's: "1", "a", "A", "i", "I", or another
    QString prefix, suffix, bulletChar, textStyle;
    int display = 1, start = 1;
    double left = 0, hanging = 0;      // twips: where the text and the label begin
    bool hasIndent = false;
};

struct ListStyleDef {
    QString name;
    ListLevel level[10];
};

struct PageLayoutDef {
    Props props;               // page:, hdr:, and ftr: keys
    QByteArray colsXml;
    bool header = false, footer = false;
};

struct MasterPageDef {
    QString name, layout, next;
    int header = -1, headerLeft = -1, headerFirst = -1, footer = -1, footerLeft = -1, footerFirst = -1;   // nodes of the styles tree
    bool headerFirstOff = false;
};

// What a section of the body is (a master page, and columns).
struct SectionState {
    QString master;            // the master page the section is set on
    int columns = 1;
    qint64 columnGap = 720;
    bool separator = false;
    bool continuous = false;   // starts on the same page as the one before
    QByteArray extra;          // the header and footer references and the page's own properties, as <w:sectPr> children
};

// The names the readers share (odtread.cpp).
const char *docxNamespaces();
extern const char kXmlHeader[];
extern const char kRelBase[];
extern const char kWml[];
// Whether a style of the family with this name as the person sees it is one the .docx format names (and that name).
bool builtInStyleName(const QString &family, const QString &display, QString *name);

constexpr qint64 kMaxSpacesPerParagraph = 1 << 20;
constexpr qint64 kMaxSpacesPerDocument = qint64(32) << 20;
// The most bytes of output the copies of repeated cells and rows may come to, together.
constexpr qint64 kMaxRepeatedBytes = qint64(256) << 20;
bool linkIsSafe(const QString &href);

class Reader {
public:
    Reader() = default;
    // The package parts; false, with a message, when the file isn't a text document that can be read.
    bool read(const QByteArray &file, QString *error, QStringList *notes);
    QByteArray docx() const { return m_docx; }

private:
    // ---- the parts and what the styles say
    bool openParts(const QByteArray &file, QString *error);
    void collectFonts(const Odf &o, int root);
    void collectStyles(const Odf &o, int root, bool automatic);
    void collectDefaults(const Odf &o, int styleNode);
    void collectStyle(const Odf &o, int node, bool automatic);
    void collectListStyle(const Odf &o, int node);
    void collectOutline(const Odf &o, int node);
    void collectPageLayout(const Odf &o, int node);
    void collectMasterPage(const Odf &o, int node);
    void collectProps(const Odf &o, int propsNode, const QString &kind, Props *into);
    QByteArray tabStopsXml(const Odf &o, int tabsNode) const;
    const StyleDef *style(const QString &family, const QString &name) const;
    // The properties a named style has of its own and from the automatic styles it follows, down to the first style that is named; `named` gets that one's name.
    const Props &ownProps(const QString &family, const QString &name, QString *named, QByteArray *tabs = nullptr, QString *listStyle = nullptr, QString *masterPage = nullptr) const;
    const Props &resolved(const QString &family, const QString &name) const;   // all of it, parents under
    Props resolvedUncached(const QString &family, const QString &name, int depth) const;
    struct Own {
        Props props;
        QString named, listStyle, masterPage;
        QByteArray tabs;
    };
    mutable QHash<QString, std::shared_ptr<Own>> m_ownCache;         // (styles are all read before the body: nothing here is ever out of date)
    mutable QHash<QString, std::shared_ptr<Props>> m_resolvedCache;
    QHash<QByteArray, QByteArray> m_rPrCache;
    // ---- the parts of the package that are made
    QString styleId(const QString &family, const QString &name);
    QString displayOf(const QString &family, const QString &name) const;
    QByteArray stylesXml();
    QByteArray rPrXml(const Props &p, const QString &rStyle, bool inStyle = false) const;
    QByteArray pPrXml(const Props &p, const QByteArray &tabs, const QByteArray &pStyle, const QByteArray &numPr, int outline, bool pageBreakBefore) const;
    QString fontsXmlValue(const Props &p, const QString &kind, const char *which) const;
    QByteArray tabsOfDefault() const;
    const MasterPageDef *masterNamed(const QString &name) const;
    QByteArray bordersXml(const Props &p, const QString &kind, const char *tag, bool cell) const;
    QByteArray shadingXml(const QString &color) const;
    QByteArray fontsXml(const Props &p, const QString &kind) const;
    QString fontFamily(const QString &name) const;
    // lists
    int numberOf(const QString &listStyle, bool fresh);
    QByteArray numberingXml() const;
    // pages
    QByteArray sectPrXml(const SectionState &s);
    QString headerPart(int node, bool header);
    // ---- the body
    struct Block {
        bool para = false;
        QByteArray pPr, content;   // a paragraph: its properties (without sectPr) and the runs
        QByteArray xml;            // anything else, whole
        QByteArray sect;           // for a paragraph that ends a section: its <w:sectPr>
    };
    struct RunFmt {
        QString rStyle;
        Props direct;
        QByteArray key() const;
    };
    struct ParaCtx;
    void convertBody(const Odf &o, int body);
    void convertBlocks(const Odf &o, int parent, QVector<Block> *out, int depth, int listLevel = -1, const QString &listStyle = QString(), int listNum = 0);
    void convertBlock(const Odf &o, int n, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum);
    void convertParagraph(const Odf &o, int n, QVector<Block> *out, int listLevel, const QString &listStyle, int listNum, bool numbered, int depth);
    void convertList(const Odf &o, int n, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum);
    void convertTable(const Odf &o, int n, QVector<Block> *out, int depth);
    void convertIndex(const Odf &o, int n, QVector<Block> *out, int depth);
    void convertSection(const Odf &o, int n, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum);
    void inlineContent(const Odf &o, int parent, ParaCtx *pc, const RunFmt &fmt, int depth);
    void inlineElement(const Odf &o, int n, ParaCtx *pc, const RunFmt &fmt, int depth);
    void addText(ParaCtx *pc, const QString &text, const RunFmt &fmt);
    void addSpaces(ParaCtx *pc, const QString &raw, const RunFmt &fmt);
    void addRun(ParaCtx *pc, const QByteArray &content, const RunFmt &fmt);
    void addField(ParaCtx *pc, const QString &instr, const QString &result, const RunFmt &fmt);
    QByteArray drawing(const Odf &o, int frame, ParaCtx *pc, bool *inlineOk);
    QByteArray pictureDrawing(const Odf &o, int frame, int image, int docPrId, bool anchored, const QString &styleName);
    QByteArray textBoxDrawing(const Odf &o, int frame, int box, ParaCtx *pc, bool anchored, const QString &styleName);
    QString imageRelationship(const Odf &o, int image);
    QByteArray anchorXml(const Odf &o, int frame, const QString &styleName, qint64 cx, qint64 cy, const QByteArray &graphic, int docPrId);
    QByteArray noteReference(const Odf &o, int note, ParaCtx *pc, const RunFmt &fmt);
    QByteArray commentXml(const Odf &o, int annotation, ParaCtx *pc);
    void collectChanges(const Odf &o);
    void closeInsertion(ParaCtx *pc);
    QByteArray deletionXml(const Odf &o, const QString &id, ParaCtx *pc);
    QByteArray bodyBlocksXml(const QVector<Block> &blocks, bool lastMustBeParagraph) const;
    QByteArray blockXml(const Block &b) const;
    void startSection(const QString &master);
    void endSection(QVector<Block> *out, const SectionState &next);
    int nextDocPrId() { return ++m_docPrId; }
    void left(const QString &what);
    int m_lastSpan = 1, m_lastRowSpan = 1;

    // The parts
    QByteArray m_docx;
    QByteArray m_contentXml, m_stylesXml, m_metaXml;
    wml::XmlTree m_content, m_styles, m_meta;
    bool m_flat = false, m_sameTree = false;
    std::unique_ptr<Odf> m_oc, m_os, m_om;
    Package m_pkg;
    QStringList *m_notes = nullptr;
    QSet<QString> m_leftKeys;

    QHash<QString, QString> m_fonts;
    QHash<QString, StyleDef> m_styleDefs;
    QHash<QString, Props> m_defaults;                 // by family
    QHash<QString, QString> m_idOfStyle;              // family\nname -> the style's id in the .docx
    QSet<QString> m_idsTaken;
    QHash<QString, ListStyleDef> m_listStyles;
    ListStyleDef m_outline;
    bool m_hasOutline = false;
    QHash<QString, PageLayoutDef> m_layouts;
    QVector<MasterPageDef> m_masters;
    QString m_defaultTabs;
    // the numbering made
    struct NumUse {
        QString listStyle;
        int abstractId = 0;
        bool restart = false;     // the count begins again at every level
        int level = -1, start = 1;   // or at one level, at a number
    };
    QVector<NumUse> m_nums;
    QHash<QString, int> m_firstNumOf;                 // list style -> the number of its first use
    QHash<QString, int> m_lastNumOf;
    QVector<QString> m_abstractOrder;                 // list styles that have an abstractNum, by id
    QHash<QString, int> m_abstractOf;
    // package builder state
    PackageWriter m_out;
    QHash<QString, QString> m_imageRel;               // the picture's path in the package -> its relationship Id
    QHash<QString, QString> m_linkRel;                // address -> relationship Id
    int m_imageCount = 0, m_docPrId = 0, m_bookmarkId = 0, m_footnoteCount = 0, m_endnoteCount = 0, m_commentCount = 0;
    QByteArray m_footnotesBody, m_endnotesBody, m_commentsBody;
    QHash<QString, QString> m_headerParts;            // "tree:node" -> relationship Id
    int m_headerCount = 0;
    bool m_evenOdd = false;
    QHash<QString, int> m_bookmarkIds;
    QSet<QString> m_commentNames;
    // the section being made
    SectionState m_sect;
    bool m_pendingPageBreak = false;
    QString m_firstMaster;
    const Odf *m_styleO = nullptr;
    QString m_owner;                                  // the part whose relationships a picture or link made now belongs to
    double m_pageTextWidth = 9360;
    QSet<QString> m_bookmarkOpen, m_bookmarkDone, m_annotationEnds;
    QHash<QString, int> m_commentOpen;
    struct Change {
        bool insertion = false, deletion = false;
        QString author, date;
        int node = -1;          // the changed region
    };
    QHash<QString, Change> m_changes;
    QString m_openInsertion;                          // the id of an insertion that goes on past the end of a paragraph
    int m_revisionId = 0;
    qint64 m_spaces = 0, m_repeatedBytes = 0;
    QHash<QString, int> m_listById;                   // a list's xml:id -> its number
    bool m_inNote = false, m_sectionsOn = false, m_sectionStarted = false;
    QByteArray m_body, m_finalSect;
    QStringList m_styleOrder;
};

} // namespace jp::office::odt
