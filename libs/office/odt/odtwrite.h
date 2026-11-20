#pragma once
// The writer of an OpenDocument text: turns the parts of a .docx package into the parts of an .odt (see odt.h). Internal to libs/office/odt.

#include "odt/odt.h"
#include "odt/odtcommon.h"
#include "office/package.h"
#include "ole/odfobject.h"
#include "wml/xmltree.h"

#include <QHash>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QVector>
#include <memory>

namespace jp::office::odt {

// The automatic styles of one part of the .odt (content.xml, or styles.xml for what the headers and footers use), named as the standard's tools name them.
struct StyleSink {
    QByteArray xml;
    QString prefix;
    QHash<QByteArray, QString> byKey;
    QHash<QString, int> counts;
    // The name of the automatic style with this body in the family (made if it is new): `base` begins its name; `attrs` are attributes of style:style besides its name and family.
    QString make(const char *base, const char *family, const QByteArray &attrs, const QByteArray &inner);
};

struct SectionInfo {
    int sectPr = -1;
    qint64 pageW = 12240, pageH = 15840, top = 1440, bottom = 1440, left = 1440, right = 1440, header = 720, footer = 720;
    bool landscape = false, continuous = false, titlePg = false, inlineSection = false;
    int columns = 1;
    qint64 columnGap = 720;
    bool separator = false;
    QString headerRef[3], footerRef[3];   // default, even, first: relationship Ids (empty: none)
    int pageStart = 0;
    QString pageFormat;   // pgNumType fmt (empty: decimal)
    QString master;
};

class Writer {
public:
    bool run(const QByteArray &docx, QByteArray *out, QString *error, QStringList *notes);

private:
    using T = wml::XmlTree;
    struct DStyle {
        QString id, name, type, basedOn, next, odfName;
        int node = -1;
    };
    struct Part {
        std::unique_ptr<T> tree;
        QString name;
    };
    // the parts
    bool loadParts(QString *error);
    T *parse(const QString &partName);
    QString partOf(const QString &rid, const QString &type = QString()) const;
    // styles
    void collectStyles();
    QString odfStyleName(const DStyle &s) const;
    QByteArray stylesPart();
    QByteArray defaultStyles();
    QByteArray paragraphAttrs(const T &t, int pPr, bool inStyle, bool *breakBefore, QByteArray *tabs, int *outline);
    QByteArray textAttrs(const T &t, int rPr);
    QByteArray bordersAttrs(const T &t, int parent, bool paragraph);
    QString borderValue(const T &t, int side, int *space, QString *lineWidth) const;
    QString fontNameOf(const QString &family, QString *generic = nullptr);
    QString themeFont(const QString &theme) const;
    QString themeColor(const QString &name) const;
    mutable QString m_themeXml;      // the theme part, read once for the save
    mutable bool m_themeRead = false;
    QString shadeColor(const T &t, int shd) const;
    // numbering
    void collectNumbering();
    QByteArray listStyleXml(int numId);
    // The name a list is given in the text: the named list style when the numbering id is a use of one that keeps its look (no restart that the style does not give), else its own.
    QByteArray listNameFor(int numId, bool *styled = nullptr);
    int numLevelOf(const T &t, int pPr, int *numId, QString pStyle) const;
    // the body
    struct ParaCtx;
    struct ListCtx {
        int numId = 0, depth = 0;
        QByteArray *out = nullptr;
    };
    void emitBlocks(const T &t, int parent, QByteArray *out, int depth, bool body);
    void emitBlock(const T &t, int n, QByteArray *out, ListCtx *lc, int depth, bool body);
    void emitParagraph(const T &t, int p, QByteArray *out, ListCtx *lc, int depth, bool body);
    void closeLists(ListCtx *lc);
    void emitTable(const T &t, int tbl, QByteArray *out, int depth, bool body);
    void emitInline(const T &t, int parent, ParaCtx *pc, int depth);
    void emitRun(const T &t, int run, ParaCtx *pc, int depth);
    void emitText(ParaCtx *pc, const QString &text, const QString &span);
    QString spanFor(const T &t, int rPr, ParaCtx *pc);
    void emitSpan(ParaCtx *pc, const QString &span, const QByteArray &content);
    void fieldEnd(ParaCtx *pc);
    QByteArray fieldXml(const QString &instr, const QString &resultText, const QByteArray &resultXml, ParaCtx *pc, bool *known);
    QByteArray drawingXml(const T &t, int drawing, ParaCtx *pc, int depth);
    // The drawn shapes of a w:pict (the older drawing form): each as the standard's rectangle, ellipse, line, or text box in a frame; empty when there is none to write.
    QByteArray shapesXml(const T &t, int pict, ParaCtx *pc);
    // The address a picture is linked to (an external relationship), as the reference of an image; empty when it is not one to keep.
    QString linkedImageAddress(const QString &rid) const;
    QByteArray pictureFrame(const T &t, int anchor, int blip, bool inlineOk, int depth);
    QByteArray frameStyle(const T &t, int anchor, bool inlineFrame, QString *anchorType, QString *extra);
    QByteArray imagePart(const QString &rid, QString *href);
    // The chart part the relationship names, as the directory of an embedded object of the new file (empty when it can't be made).
    QString chartObject(const QString &rid, qint64 widthEmu, qint64 heightEmu);
    // An equation (an m:oMath or m:oMathPara) as a formula object of the new file inside a frame; empty when it can't be made (past the limits, or markup that has no MathML).
    QByteArray formulaFrame(const T &t, int math);
    // An embedded object (a w:object: its package and its picture) as an object of the new file inside a frame; empty when it can't be kept (past the caps, or not a package to keep).
    QByteArray objectFrame(const T &t, int object);
    QByteArray noteXml(const T &t, int ref, bool endnote, int depth, const QString &mark = QString());
    QByteArray commentStart(int id);
    // sections
    void collectSections();
    void sectionEnded();
    void beginSection(QByteArray *out);
    SectionInfo sectionOf(const T &t, int sectPr);
    QString masterFor(const SectionInfo &s);
    QByteArray masterPages();
    QByteArray headerBody(const QString &rid, bool header, int depth);
    int tocDepthOf(const T &t, int p, bool *startsToc, QString *instr) const;
    QByteArray contentXml();
    QByteArray fontFacesXml() const;
    QByteArray settingsXml() const;
    QByteArray metaXml();
    void left(const QString &what);
    QByteArray changeRegion(const T &t, int node, const char *kind, bool withParagraph = false);

    Package m_pkg;
    QString m_main;
    std::vector<Part> m_parts;
    T *m_doc = nullptr, *m_styles = nullptr, *m_numbering = nullptr, *m_settings = nullptr, *m_footnotes = nullptr, *m_endnotes = nullptr, *m_comments = nullptr;
    QString m_source;                                  // the part being read (its relationships find the pictures and links)
    QStringList *m_notes = nullptr;
    QSet<QString> m_leftKeys;
    QHash<QString, QString> m_systemFaces;              // a face for East Asian or complex script -> its family
    QByteArray m_odfItems;                              // the layout settings an .odt had, as config items
    QHash<QString, QPair<QString, QString>> m_fontKinds;   // font name -> the family kind and pitch the .docx font table gives
    QHash<QString, DStyle> m_dstyles;                  // by id
    QStringList m_styleOrder;
    QString m_defaultParaStyle, m_defaultCharStyle;
    // the namespaces of the document
    int m_wp = -1, m_a = -1, m_pic = -1, m_wps = -1, m_mc = -1, m_v = -1, m_m = -1, m_wp14 = -1;
    // numbering
    struct LevelDef {
        bool defined = false;
        QString fmt, text, jc, suff;
        int start = 1;
        qint64 left = 0, hanging = 0;
        int pPr = -1;
        int rPr = -1;
    };
    struct AbstractDef {
        LevelDef level[9];
        QString styleLink, numStyleLink, styled;   // w:styleLink, and the numbering style whose levels these are (its own, or the one a w:numStyleLink chain reaches)
    };
    QHash<int, AbstractDef> m_abstracts;
    struct NumDef {
        int abstractId = -1;
        LevelDef overrides[9];
        int startOverride[9];
    };
    QHash<int, NumDef> m_numDefs;
    QSet<int> m_numsUsed, m_numsSeen;
    QHash<int, QByteArray> m_listIdOfNum;   // the xml:id of the last list of a numbering id that names a list style (a later one goes on from it)
    int m_listIds = 0;
    QByteArray listLevelsXml(const QByteArray &head, const AbstractDef &def, const int *startOverride);
    // output
    StyleSink m_content, m_stylesSink;
    StyleSink *m_sink = &m_content;
    QByteArray m_bodyXml;
    QHash<QString, QString> m_images;                  // relationship Id of the part -> its path in the package
    QVector<QPair<QString, QByteArray>> m_pictures;
    QVector<QPair<QString, QByteArray>> m_chartObjects;   // an embedded chart's directory -> its content.xml
    QHash<QString, QString> m_chartDone;                  // a chart part of the package -> its directory
    struct Formula {
        QString dir;
        QByteArray content, picture;
    };
    QVector<Formula> m_formulas;                          // the equations: the directory of each formula object, its MathML, and its picture
    qint64 m_formulaBytes = 0;
    ole::ObjectBudget m_objectBudget;                     // the embedded objects written so far, against the object core's caps
    QHash<QString, QPair<QString, bool>> m_objectDone;    // an object's part in the package -> its directory in the new file (empty for one that is refused) and whether it is a folder, so that a part many frames name is read once
    QVector<ole::OdfOutPart> m_objectParts;               // the parts the objects are written as
    int m_objectDirs = 0;                                 // the directories made for objects
    bool m_objectsPastLimit = false;
    int m_imageCount = 0, m_frameCount = 0, m_noteCount = 0, m_endnoteCount = 0, m_tableCount = 0, m_sectionCount = 0, m_commentCounter = 0, m_bookmarkSeq = 0, m_galleryMarks = 0;
    QSet<QString> m_sequences;
    QMap<QString, QString> m_fontFaces;                // name -> generic
    QSet<QString> m_commentsStarted;
    QHash<int, int> m_bookmarkIdToName;
    QHash<int, QString> m_bookmarkNames;
    // sections
    QVector<SectionInfo> m_sections;
    int m_sectionIndex = 0;
    QHash<int, int> m_sectionOfPara;                   // the paragraph with a sectPr -> its section
    QString m_pendingMaster;
    bool m_pendingBreak = false;
    // What the break that is pending came from (it is told on the paragraph that takes it, so that it reads back as it was): nothing the file may say, a paragraph of the body that ended with it, or one that was only it.
    enum { kBreakNone, kBreakEnded, kBreakAlone };
    int m_breakSource = kBreakNone;
    int m_breakAlone = 0;   // how many paragraphs that were only a page break are waiting to be told (one break is all the standard has for them)
    QHash<QString, QString> m_masterByKey;
    QVector<SectionInfo> m_masters;
    QHash<QString, QByteArray> m_headerCache;
    bool m_evenOdd = false;
    QByteArray m_layouts, m_masterXml;
    QByteArray m_pendingBookmarks, m_changeRegions;
    int m_changeCount = 0;
    qint64 m_cellsMade = 0;
    QByteArray m_cellText;                             // the text attributes the style of the table gives the words of the cell being written (a span's own come over them)
    int m_tocDepth = 0;
    bool m_inToc = false;
    int m_innerSections = 0;
    bool m_sectionWrapperOpen = false;
    bool m_inNote = false;
    QString m_stripMark;      // while a note whose mark was typed is written: the mark its words begin with (it is the citation's label), and then the blank after it
    bool m_stripBlank = false;
};

} // namespace jp::office::odt
