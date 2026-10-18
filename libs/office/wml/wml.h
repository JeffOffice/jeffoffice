#pragma once
// WordprocessingML (ECMA-376 part 1, chapter 17): the word processor's document, as the package of a .docx holds it. The reader turns
// the package into the Document below; the writer turns a Document back into a package.
//
// The Document holds what a word processor edits (paragraphs, their runs of text, styles, lists, and sections) as plain data,
// and keeps everything else as it was read, to be written back unchanged:
//  - a part it doesn't edit (headers, footers, footnotes, comments, pictures, the theme, settings, custom XML...) is kept whole,
//    with its relationships and their Ids, as bytes;
//  - an element inside a part it edits that it doesn't understand (a table, a picture, a field's code, a bookmark, a hyperlink's
//    tags, a property of a paragraph or run it has no field for) is kept as the slice of XML it was, and written back in the place
//    the schema gives it.
// Properties hold only what the paragraph, run, or style states (std::optional is "not stated"); the style that applies and the
// document's defaults give the rest (StyleSheet::paragraph, StyleSheet::run).

#include "office/package.h"
#include "office/theme.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QHash>
#include <QMap>
#include <QString>
#include <QVector>
#include <memory>
#include <optional>

namespace jp::office::wml {

// An element kept as written, with the place the schema gives it among its siblings (written in this order).
struct RawChild {
    int rank = 0;
    QByteArray xml;
    bool operator==(const RawChild &o) const { return rank == o.rank && xml == o.xml; }
};

// The typefaces of a run, a script each; a theme name (minorHAnsi) in place of a face when the file gave one.
struct Fonts {
    QString ascii, hAnsi, eastAsia, cs;
    QString asciiTheme, hAnsiTheme, eastAsiaTheme, csTheme;
    QString hint;                     // which script's font the characters that could be either take ("eastAsia", "cs", or "default")
    // The fonts of the scripts the text engine's formats have no place for (everything but the Latin ones, and the hint).
    bool hasOtherScripts() const { return !eastAsia.isEmpty() || !cs.isEmpty() || !eastAsiaTheme.isEmpty() || !csTheme.isEmpty() || !hint.isEmpty(); }
    bool sameOtherScripts(const Fonts &o) const { return eastAsia == o.eastAsia && cs == o.cs && eastAsiaTheme == o.eastAsiaTheme && csTheme == o.csTheme && hint == o.hint; }
    bool operator==(const Fonts &o) const;
    bool operator!=(const Fonts &o) const { return !(*this == o); }
    bool isEmpty() const;
    void overlay(const Fonts &top);   // each script `top` states replaces this one's
};

struct Color {
    QString val;                      // "RRGGBB" or "auto"
    QString theme, tint, shade;       // themeColor, themeTint, themeShade as written
    bool operator==(const Color &o) const { return val == o.val && theme == o.theme && tint == o.tint && shade == o.shade; }
    bool operator!=(const Color &o) const { return !(*this == o); }
};

struct Underline {
    QString val;                      // single, double, thick, dotted, dash, wave... or none
    QString color;                    // "RRGGBB", "auto", or empty
    QByteArray attrs;                 // the other attributes of w:u (themeColor, themeTint, themeShade...), as written
    bool operator==(const Underline &o) const { return val == o.val && color == o.color && attrs == o.attrs; }
    bool operator!=(const Underline &o) const { return !(*this == o); }
};

struct RunProps {
    QString style;                    // rStyle
    std::optional<Fonts> fonts;
    std::optional<bool> bold, italic, caps, smallCaps, strike, dstrike, vanish, noProof;
    std::optional<Color> color;
    std::optional<int> spacing;       // twentieths of a point added after each letter
    std::optional<int> position;      // half-points raised (lowered when negative)
    std::optional<int> kern;          // half-points: the letters are kerned from this size up (0: not kerned)
    std::optional<int> size;          // half-points
    QString highlight;                // yellow, green...; "none"; empty when not stated
    std::optional<Underline> underline;
    QString vertAlign;                // baseline, superscript, subscript; empty when not stated
    QVector<RawChild> extra;          // the rest of rPr (kept as written)

    // Every field `top` states takes the place of this one's; style and extra are left alone.
    void overlay(const RunProps &top);
    // The same properties with every one that isn't stated given the value it has when nothing says (not bold, automatic color...), to
    // compare what two sets of properties come to.
    RunProps normalized() const;
    // The fields where this differs from `base` in effect, the rest left unstated; style and extra are kept.
    RunProps differences(const RunProps &base) const;
    bool operator==(const RunProps &o) const;
};

struct TabStop {
    int pos = 0;                      // twentieths of a point from the margin
    QString val;                      // left, center, right, decimal, bar, clear
    QString leader;                   // none, dot, hyphen, underscore...
    bool operator==(const TabStop &o) const { return pos == o.pos && val == o.val && leader == o.leader; }
    bool operator!=(const TabStop &o) const { return !(*this == o); }
};

struct SectionProps;

// One side of a paragraph's borders (w:pBdr), as the paragraph or its style states it.
struct Border {
    QString val;                      // single, double, dashed, dotted, thick, nil or none (no border)...
    int size = 0;                     // eighths of a point
    int space = 0;                    // points between the border and the text
    QString color;                    // "RRGGBB", or auto
    bool operator==(const Border &o) const { return val == o.val && size == o.size && space == o.space && color == o.color; }
    bool visible() const { return !val.isEmpty() && val != QLatin1String("nil") && val != QLatin1String("none") && size > 0; }
};
struct ParaBorders {
    std::optional<Border> top, left, bottom, right, between;
    bool operator==(const ParaBorders &o) const { return top == o.top && left == o.left && bottom == o.bottom && right == o.right && between == o.between; }
};

// A paragraph's shading (w:shd).
struct Shading {
    QString val;                      // clear, solid, pct10...
    QString color, fill;              // "RRGGBB" or auto
    bool operator==(const Shading &o) const { return val == o.val && color == o.color && fill == o.fill; }
};

struct ParaProps {
    QString style;                    // pStyle
    std::optional<bool> keepNext, keepLines, pageBreakBefore, widowControl, contextualSpacing;
    std::optional<int> numId, ilvl;   // numPr
    std::optional<QVector<TabStop>> tabs;
    std::optional<int> before, after; // twentieths of a point
    // The space before or after is "automatic" (w:beforeAutospacing, w:afterAutospacing): the word processor's own for a paragraph that came from a web
    // page, which is not what `before` and `after` say. Stated with them: a paragraph that gives its space overrides its style's automatic one.
    std::optional<bool> beforeAuto, afterAuto;
    std::optional<int> line;          // 240ths of a line, or twentieths of a point (see lineRule)
    QString lineRule;                 // auto, exact, atLeast
    std::optional<int> indLeft, indRight;   // twentieths of a point
    std::optional<int> indFirst;      // the first line's, from indLeft; negative for a hanging indent
    QString jc;                       // left, center, right, both, distribute... as written
    std::optional<int> outlineLvl;    // 0 to 8, 9 for body text
    QByteArray spacingAttrs, indAttrs;   // attributes of w:spacing and w:ind that aren't fields here, as written
    // The borders and shading the paragraph states, read from w:pBdr and w:shd (which are kept as written in `extra` too, and are what is written): a
    // side a style states and the paragraph doesn't is the style's.
    std::optional<ParaBorders> borders;
    std::optional<Shading> shading;
    // Whether punctuation may extend past the margin (w:overflowPunct): read for the layout, and kept as written in `extra` too, which is what is written.
    std::optional<bool> overflowPunct;
    std::optional<RunProps> markRun;  // the paragraph mark's own run properties
    std::shared_ptr<SectionProps> section;   // sectPr: this paragraph ends a section
    QVector<RawChild> extra;

    void overlay(const ParaProps &top);
    ParaProps normalized() const;
    ParaProps differences(const ParaProps &base) const;
};

struct ColumnDef {
    int width = 0, space = 0;
};

struct SectionProps {
    QString type;                     // nextPage (when empty), continuous, evenPage, oddPage, nextColumn
    int pageW = 12240, pageH = 15840; // twentieths of a point (Letter)
    QString orient;                   // portrait or landscape
    int top = 1440, right = 1440, bottom = 1440, left = 1440, header = 720, footer = 720, gutter = 0;
    int cols = 1, colSpace = 720;
    bool colSep = false, colEqual = true;
    QVector<ColumnDef> colDefs;
    std::optional<int> pageStart;     // pgNumType start (empty: not stated; 0 is a number a section can start at)
    QString pageFormat;               // pgNumType fmt (decimal when empty)
    bool titlePg = false;
    bool hasPgSz = false, hasPgMar = false, hasCols = false;   // stated in the file
    QByteArray attrs;                 // the attributes of w:sectPr (revision marks), as written
    // The attributes of w:pgSz (code...), w:pgMar, w:pgNumType (chapStyle, chapSep...), and w:cols that have no field here, as written.
    QByteArray pgSzAttrs, pgMarAttrs, pgNumAttrs, colsAttrs;
    QVector<RawChild> extra;          // header and footer references, borders, line numbers, grid... as written
};

// What a paragraph holds, in order.
struct Inline {
    // Text is a run of text (a tab is U+0009, a line break U+2028, and a page break U+000C: <w:br w:type="page"/> reads as that character and
    // is written from it); a RunAtom is a child of a run that is kept as written (a picture, a field's code or mark, a footnote
    // reference...); a ParaAtom is an element between runs that is kept as written (a bookmark, the start or end tag of a hyperlink).
    enum Kind : quint8 { Text, RunAtom, ParaAtom };
    // A tag or a field mark that has a partner (the end of what the Open one began) is written only with it.
    enum Role : quint8 { Single, Open, Close };
    Kind kind = Text;
    Role role = Single;
    RunProps props;                   // the run's (Text and RunAtom)
    QString text;                     // Text: a tab is '\t', a line break U+2028, a no-break hyphen U+2011, a soft hyphen U+00AD
    QByteArray raw;                   // atoms: the XML (a RunAtom's is the child of the run, a ParaAtom's is the element, or the tag)
    QString display;                  // atoms: the text shown in the atom's place, none for most
    QString tag, key;                 // atoms: the element's local name, and what pairs it with its partner (a bookmark's id, a field mark's type)
};

struct Paragraph {
    ParaProps props;
    QByteArray attrs;                 // the attributes of w:p (revision marks, paragraph ids), as written
    QVector<Inline> content;
};

// A width a table says of itself, of a column, or of a cell: dxa is twentieths of a point, pct fiftieths of a percent, auto and nil have none.
struct TableWidth {
    QString type;                     // dxa, pct, auto, nil; empty when not stated
    int w = 0;
    bool stated() const { return !type.isEmpty(); }
    bool isDxa() const { return type == QLatin1String("dxa") || type.isEmpty(); }
    bool operator==(const TableWidth &o) const { return type == o.type && w == o.w; }
};

// A border of a table or a cell as w:top, w:left... state it.
struct TableBorder {
    QString val;                      // single, double, dotted, dashed, thick, nil, none... (empty when not stated)
    int size = 0;                     // eighths of a point
    int space = 0;                    // points
    QString color;                    // "RRGGBB", "auto", or empty
    bool stated() const { return !val.isEmpty(); }
    bool operator==(const TableBorder &o) const { return val == o.val && size == o.size && space == o.space && color == o.color; }
};

// What a w:tbl, w:tr, or w:tc says, read for the program that lays tables out (the element is written from the raw bytes of its block, which
// are what the file had, so what is not read here is kept).
struct TableMark {
    enum Kind : quint8 { Table, Row, Cell };
    Kind kind = Table;
    // ---- a table (tblPr and tblGrid)
    QVector<int> grid;                // the columns' widths, twentieths of a point
    qsizetype gridBegin = -1, gridEnd = -1;   // where w:tblGrid is in the raw bytes of the block
    QString style;                    // tblStyle
    TableWidth width, indent, spacing;        // tblW, tblInd, tblCellSpacing
    QString jc;                       // left, center, right, start, end
    QString layout;                   // fixed or autofit (empty when not stated)
    std::optional<TableWidth> marTop, marLeft, marBottom, marRight;   // tblCellMar
    TableBorder top, left, bottom, right, insideH, insideV;           // tblBorders (also tcBorders for a cell)
    QString shdFill;                  // shd: "RRGGBB", "auto", or empty (also a cell's)
    QString shdVal, shdColor;         // its pattern (clear, solid, pct25...) and the color of the pattern
    int look = 0x04A0;                // tblLook as the bit field (0x20 first row, 0x40 last row, 0x80 first column, 0x100 last column, 0x200 no row bands, 0x400 no column bands)
    int rowBand = 1, colBand = 1;     // tblStyleRowBandSize and tblStyleColBandSize
    bool floating = false;            // tblpPr
    bool bidi = false;
    // ---- a row (trPr)
    int gridBefore = 0, gridAfter = 0;
    TableWidth widthBefore, widthAfter;
    int height = 0;                   // trHeight, twentieths of a point (0: not stated)
    QString heightRule;               // atLeast (when empty), exact, auto
    bool cantSplit = false, header = false;
    bool hidden = false;              // hidden: the row is not shown
    // ---- a cell (tcPr)
    TableWidth cellWidth;             // tcW
    int span = 1;                     // gridSpan
    QString vMerge;                   // "", restart, or continue
    TableBorder diagDown, diagUp;     // tl2br, tr2bl
    std::optional<TableWidth> cMarTop, cMarLeft, cMarBottom, cMarRight;   // tcMar
    QString vAlign;                   // top, center, bottom, both
    bool noWrap = false;
    QString textDirection;
    // ---- a table, for writing it as it was
    // The whole element as it was read, and a fingerprint of the blocks it was read into as the writer would write them: when the table's blocks come to
    // the same fingerprint as they are written (nothing in the table changed), the table is written as these bytes, not from its blocks.
    QByteArray original, fingerprint;
};

// An element of the body: a paragraph, or an element that is kept as written (a table's parts that are not read, the start or end of a
// content control). A table is a run of blocks: an Open block (tag "tbl", with the table's start tag, properties, and grid as raw bytes), the
// rows in it (an Open block with tag "tr", a block of tag "tc" for each cell with the cell's content after it, and a Close block of each),
// and a Close block. Anything else in a table that is not understood (a bookmark between rows) is a raw block between them.
struct Block {
    bool isRaw = false;
    Paragraph para;
    QByteArray raw;
    QString display;                  // a raw block: the text shown in its place
    Inline::Role role = Inline::Single;
    QString tag, key;
    std::shared_ptr<const TableMark> table;   // the Open block of a table, a row, or a cell: what it says
};

// What a table style says (or one of its parts: the first row, banded rows...) of the table, its cells, and the paragraphs in them.
struct TableStylePart {
    TableMark tbl, cell;              // its tblPr and tcPr, as a table and a cell say theirs
    std::optional<ParaProps> pPr;
    std::optional<RunProps> rPr;
};

// A table style, with the styles it is based on: the whole table's part, and the parts of rows, columns, bands, and corners ("firstRow", "lastRow",
// "firstCol", "lastCol", "band1Vert", "band2Vert", "band1Horz", "band2Horz", "neCell", "nwCell", "seCell", "swCell"), each over the styles it is based on.
struct TableStyleDef {
    TableStylePart base;
    QMap<QString, TableStylePart> parts;
    std::optional<int> rowBand, colBand;   // how many rows or columns a band is
};

// The paragraph and run properties the style of a table gives the paragraphs in a cell (with the parts of it that apply to the cell): they come over the
// document's defaults and under the paragraph's own style.
struct TableLayer {
    ParaProps p;
    RunProps r;
    QString key;                      // what tells it from another (for the caches)
};

struct Style {
    QString id, name, type;           // type: paragraph, character, table, numbering
    QString basedOn, next, link;
    bool isDefault = false, customStyle = false, qFormat = false, semiHidden = false, unhideWhenUsed = false;
    std::optional<int> uiPriority;
    ParaProps pPr;
    RunProps rPr;
    QByteArray tail;                  // what follows rPr (tblPr, trPr, tcPr, tblStylePr), as written
    QByteArray raw;                   // the element as written, while it is unchanged
};

class StyleSheet {
public:
    QByteArray rootStart;             // the start tag of w:styles as written (empty: write the usual one)
    QByteArray docDefaults, latentStyles;   // the elements as written
    RunProps defaultRun;
    ParaProps defaultPara;
    QVector<Style> styles;
    QByteArray original;              // the part as read; written back as it was unless something changed
    bool modified = false;

    const Style *find(const QString &id) const;
    // The id of the style that is the default of its type (Normal for paragraphs), or empty.
    QString defaultId(const QString &type) const;
    // Adds a style (replacing one of the id); false for an empty id.
    bool add(const Style &style);
    // What a paragraph of style `id` has: the document's defaults, then the styles it is based on, then the style. A style no file
    // defines gives the defaults. In a table's cell, the table style's properties for it go after the defaults.
    ParaProps paragraph(const QString &id, const TableLayer *table = nullptr) const;
    // The table style `id` makes of tables (the styles it is based on under it); an empty one for no such style.
    const TableStyleDef &tableStyle(const QString &id) const;
    // The same, for a style whose own numbering gives its paragraphs the indents of `level` (the properties of the list's level): the
    // level's come below those of the style that says it is numbered, so that style's own indents win over the list's. (When a
    // paragraph itself says it is numbered, the list's come over its style's: that is `paragraph(id)` and then `overlay(level)`.)
    ParaProps paragraphWithNumbering(const QString &id, const ParaProps &level, const TableLayer *table = nullptr) const;
    // What a run has when nothing is said directly: the defaults, the paragraph style's run properties, then the character style's.
    RunProps run(const QString &paragraphStyle, const QString &characterStyle, const TableLayer *table = nullptr) const;
    void changed() { m_paragraph.clear(); m_run.clear(); m_index.clear(); m_indexed = -1; m_tableStyles.clear(); }

private:
    void reindex() const;
    mutable QHash<QString, int> m_index;
    mutable qsizetype m_indexed = -1;
    mutable QHash<QString, ParaProps> m_paragraph;
    mutable QHash<QString, RunProps> m_run;
    mutable QHash<QString, TableStyleDef> m_tableStyles;
};

// A level of a list.
struct Level {
    int start = 1;
    QString numFmt = QStringLiteral("decimal");   // decimal, lowerLetter, upperRoman, bullet...
    QString text;                     // lvlText: "%1." or a bullet character
    QString jc = QStringLiteral("left");
    QString suff = QStringLiteral("tab");
    QString pStyle;                   // the paragraph style this level is linked to
    bool legal = false;
    bool legacy = false;              // w:legacy: a list the older programs made (its text begins at the indent)
    std::optional<int> restart;       // lvlRestart
    ParaProps pPr;                    // indents (and tabs) of the paragraphs in it
    RunProps rPr;                     // the number's own formatting (a bullet's font)
    bool present = false;
};

struct AbstractNum {
    int id = 0;
    QString multiLevelType;
    Level levels[9];
    QByteArray head, tail;            // what comes before the levels (nsid, tmpl, name, styleLink) and after, as written
    QByteArray raw;                   // the element as written, while it is unchanged
};

struct Num {
    int id = 0;
    int abstractId = 0;
    QMap<int, int> startOverrides;    // level -> startOverride
    QByteArray raw;
};

struct Numbering {
    QByteArray rootStart, prefix;     // w:numbering's start tag, and the numPicBullet elements, as written
    QVector<AbstractNum> abstracts;
    QVector<Num> nums;
    QByteArray suffix;                // numIdMacAtCleanup
    QByteArray original;              // the part as read; written back as it was unless something changed
    bool modified = false;
    const AbstractNum *abstractOf(int numId) const;
    const Num *num(int numId) const;
    // The list a numbering id counts with: its own, when it restarts a level (startOverride), else the first id with no restart of the same definition, which every such id
    // goes on counting with.
    int listOf(int numId) const;
    // Whether another numbering id counts with it (so that the number an item has is not its own list's count).
    bool listIsShared(int numId) const;
    int nextNumId() const;
    int nextAbstractId() const;

private:
    void index() const;
    mutable QHash<int, int> m_nums, m_abstracts;   // the first of each id, by place
    mutable QHash<int, int> m_plain;               // the first numbering id of each definition (by its id) that restarts no level
    mutable QHash<int, int> m_plainCount;          // how many such ids each definition has
    mutable qsizetype m_numCount = -1, m_abstractCount = -1;
};

// A part kept whole.
struct KeptPart {
    QString name;                     // absolute ("/word/header1.xml")
    QString contentType;
    QByteArray data;
};

struct Document {
    // The main part.
    QByteArray rootStart;             // w:document's start tag as written (empty: write the usual one)
    QByteArray beforeBody, afterBody; // what stands beside w:body (w:background...), as written
    QVector<Block> body;
    SectionProps lastSection;         // the section the body ends with
    bool lastSectionStated = false;
    // The styles, lists, theme, and settings the document uses.
    StyleSheet styles;
    Numbering numbering;
    Theme theme;
    bool hasTheme = false;
    int defaultTabStop = 720;         // settings.xml's, twentieths of a point
    bool evenAndOddHeaders = false;
    int compatMode = 12;              // compatibilityMode of settings.xml (11 for the 2003 programs' files, 12 for 2007's, 15 for 2013's and after); 12 when it says nothing

    // The package.
    QString mainName = QStringLiteral("/word/document.xml");
    QString mainContentType;
    QString stylesName, numberingName, themeName, settingsName;   // the parts the document has of those kinds (empty: none)
    QVector<Relationship> mainRelationships;      // every relationship of the main part, Ids as read
    QVector<Relationship> packageRelationships;   // those of the package (the main part's, the properties')
    QMap<QString, QVector<Relationship>> partRelationships;   // of the kept parts, by part name
    QVector<KeptPart> parts;                      // every part but the main one, the styles, and the numbering, in package order
};

// What reading may leave out, one note a kind (for the reader's caller to tell the person); empty when nothing was.
struct ReadNotes {
    QStringList notes;
    // The file has macros (they are not read, and are left out when the document is written as the plain kind).
    bool hadMacros = false;
    // Something of the file is not in the document: the end of a body past the limits, text deeper than the nesting limit made plain, a part or a
    // link that could not be read. Written over the original, it would lose that.
    bool leftOut = false;
};

// The relationship Ids a section's headers and footers have, empty where it states none: index 0 is the ordinary (default) page's, 1 the first
// page's (used when the section has titlePg), and 2 the even pages' (used when the document has evenAndOddHeaders). A section that names
// none of a kind takes the section before's.
struct HeaderRefs {
    QString header[3], footer[3];
    bool any() const;
};
HeaderRefs headerRefsOf(const SectionProps &section);

// The paragraphs of a header, footer, or note part, and the elements kept as written (a table, a content control's ends), read as the body
// is. False, with nothing read, when the part isn't XML.
bool readBlocks(const QByteArray &xml, QVector<Block> *blocks);

// A drawn object of a run: a picture, a rule, a box, or a text box, read from the VML of a w:pict (or an object) or the DrawingML of a w:drawing. Sizes are
// points. An object is `floating` where its style says it is set at a place (anchored to a paragraph, the margin, or the page), else it is in the line.
struct Shape {
    enum Kind { Picture, Rect, Oval, Line, TextBox };
    // What a position is measured from; the margin areas are the strips of the page between its edge and the margins (inside and outside: the strip at the fold's side and the far side).
    enum Rel { Margin, Page, Column, Paragraph, LineOf, Character, LeftMargin, RightMargin, TopMargin, BottomMargin, InsideMargin, OutsideMargin };
    enum Align { NoAlign, Start, Center, End };
    enum Wrap { WrapNone, Square, Tight, TopBottom };
    // Where the text goes beside a square or tight wrap.
    enum Side { SideBoth, SideLeft, SideRight, SideLargest };
    Kind kind = Rect;
    bool floating = false;
    double width = 0, height = 0;
    double left = 0, top = 0;             // from the reference, when the object is not aligned
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;   // a line's ends from the anchor
    Rel hRel = Column, vRel = Paragraph;
    Align hAlign = NoAlign, vAlign = NoAlign;
    bool behind = false;                  // set behind the text
    int z = 0;
    Wrap wrap = WrapNone;
    Side side = SideBoth;
    double gapLeft = 9, gapRight = 9, gapTop = 0, gapBottom = 0;   // room the wrapping text leaves
    bool filled = false, stroked = false;
    QString fillColor, strokeColor;       // "RRGGBB", or empty
    double strokeWeight = 0.75;
    double rotation = 0;
    bool flipH = false, flipV = false;
    bool arrowEnd = false, arrowStart = false;
    QString embed;                        // a picture's relationship Id
    QString pictEmbed;                    // the Id of the picture's other form (VML's r:pict: a compressed picture the program keeps beside it)
    QString alt;                          // its description
    QByteArray textBox;                   // the w:txbxContent element of a text box, as written (insets: below)
    double inset[4] = {7.2, 3.6, 7.2, 3.6};   // left, top, right, bottom
    // A picture's cut edges, each a share of the whole picture (a negative one is room left round it), the color of it that is clear ("RRGGBB"),
    // and the border it is drawn with (its weight in points, 0 for none, and its color).
    double cropL = 0, cropT = 0, cropR = 0, cropB = 0;
    QString clearColor;
    double borderWeight = 0;              // the heaviest of its sides'
    QString borderColor;
    // The weights of the sides of a picture's border, in points (top, left, bottom, right): the sides the file sets, 0 for the others.
    double borderSides[4] = {0, 0, 0, 0};
    // The shape's own numbers (VML's adj, a list of numbers some of which it leaves empty: those are not a number), as written.
    QVector<double> adjust;
    // The number of the drawing format's shape the object is (VML's o:spt, from the shape's type), 0 when it says none: 75 is a picture, 202 a text
    // box, 19 an arc, 136 text on a path.
    int shapeType = 0;
    // Text on a path (an object that is its text, set to fill its frame): the text, its font, and whether it is bold and italic.
    QString pathText, pathFont;
    bool pathBold = false, pathItalic = false;
    // A shadow of the object (VML's v:shadow): its color and how far across and down it is, in points.
    bool shadow = false;
    QString shadowColor;
    double shadowX = 0, shadowY = 0;
    // A horizontal rule (VML's o:hr): as wide as this many percent of the text's width (0: as wide as the frame is) and as high as the frame, set as
    // its horizontal alignment says.
    bool rule = false;
    double rulePercent = 0;
};
// The objects of a w:pict, w:drawing, or mc:AlternateContent element kept as written (a run's atom): in order, nested ones (a group's members) after it.
// A VML shape that is only a template (a shape type) is none.
QVector<Shape> readShapes(const QByteArray &xml);

// A footnote or endnote of footnotes.xml or endnotes.xml: its Id, its type (empty for an ordinary note; "separator", "continuationSeparator", and
// "continuationNotice" for the paragraphs the program sets between the text and the notes), and what it holds.
struct NotePart {
    int id = 0;
    QString type;
    QVector<Block> blocks;
};
// The most notes of one part that are read (a file with more has the rest left out).
constexpr int kMaxNotes = 20000;
// Reads the notes of a footnotes or endnotes part. False, with nothing read, when the part isn't XML.
bool readNotes(const QByteArray &xml, QVector<NotePart> *notes);

// A font of a font table: its name, the alternate it names (a family to set it in where it is missing), its kind (roman, swiss, modern, script,
// decorative, or auto), its pitch (fixed, variable, or default), its PANOSE classification (ten two-digit numbers), and its character set (two hex digits,
// "02" for a symbol font), each empty when it says none.
struct FontEntry {
    QString name, alternate, family, pitch, panose, charset;
};
// The most fonts of a table that are read.
constexpr int kMaxFonts = 4000;
// Reads the fonts of a font table part. False, with nothing read, when the part isn't XML.
bool readFonts(const QByteArray &xml, QVector<FontEntry> *fonts);

// How the numbers of the notes begin again, from the settings part's w:footnotePr or w:endnotePr (`group`): the value of its w:numRestart (continuous, eachSect, or
// eachPage), empty if it says none.
QString noteNumberRestart(const QByteArray &settings, const char *group);

// Whether a compatibility option of the settings part (an element of w:compat) is on: stated, and not turned off (w:val 0, false, or off).
bool compatOption(const QByteArray &settings, const char *name);
// The compatibility options the layout reads, from one reading of the settings part (compatOption reads the whole part for each one, and a part may be very large).
struct CompatOptions {
    bool useFELayout = false, doNotWrapTextWithPunct = false, doNotUseHTMLParagraphAutoSpacing = false, noTabHangInd = false;
    bool doNotUseIndentAsNumberingTabStop = false, splitPgBreakAndParaMark = false, suppressTopSpacingWP = false, noLeading = false, suppressSpBfAfterPgBrk = false;
};
CompatOptions readCompatOptions(const QByteArray &settings);

// Reads a .docx (or .docm, .dotx) package. False, with a message, if it isn't a word processing document or is damaged beyond reading.
// A file is held to limits: of parts together inflated, of the XML of a part (xmltree.h), and of what the body holds, past which the rest is
// left out and a note says so. Real files, in 1,795 .docx files of public collections: 125 KB inflated in the middle, 15 MB for 99 of 100, and
// 97.5 MB at the most (of which word/document.xml 20.6 MB); at most 45,000 paragraphs and 65,000 runs (the pieces kept as XML are about as many again). Each limit
// is 10 times the most or more.
constexpr qint64 kMaxInflatedBytes = qint64(1) << 30;
constexpr int kMaxParagraphs = 500000;
constexpr int kMaxInlines = 1500000;
// A table is read as one of at most this many rows and columns (a word processor's own largest has 63 columns), of at most this many cells, nested at
// most this deep, and all the tables of a document hold at most the last number of cells (the most that are laid out in a document: 50,000 short cells open and
// are paged in a few seconds); a table that goes past any of them is kept whole as XML and shown as a stand-in, as every table was before, and the file's
// reader is told to say so (ReadNotes). (The dev half of the document collection has a table of 598 rows, which the suite's 256 would have left out.)
constexpr int kMaxTableRows = 20000;
constexpr int kMaxTableColumns = 256;
constexpr int kMaxTableCellsEach = 50000;
constexpr int kMaxTableDepth = 8;
constexpr int kMaxTableCells = 50000;
bool readDocx(const QByteArray &zip, Document *doc, QString *error = nullptr, ReadNotes *notes = nullptr);
// The package of a document. Empty, with a message, if it can't be made (an Id or a part name that can't be written).
QByteArray writeDocx(const Document &doc, QString *error = nullptr);
// A new document: Letter paper with one-inch margins, the styles of a new document (Normal, Heading 1 to 3, Title, ...), and a theme.
Document newDocument();

// The built-in definitions of the styles a new document has and a person may apply (Normal, Heading 1 to 9, Title, Subtitle, List
// Paragraph...); used to add one a document lacks. The id is the style's name without spaces ("Heading1"). Empty id when unknown.
Style builtinStyle(const QString &id);
QStringList builtinStyleIds();

// A fingerprint of blocks [from, to) of a body as they are written (every piece kept, as nothing says otherwise), for telling a table that has not changed.
QByteArray blocksFingerprint(const QVector<Block> &body, qsizetype from, qsizetype to);
// The fingerprint of blocks that come one at a time: what blocksFingerprint finds of them together, without holding them.
class BlocksFingerprint {
public:
    BlocksFingerprint();
    void add(const Block &block);
    QByteArray finish();

private:
    QCryptographicHash m_hash;
    QByteArray m_piece;
};

// What one table style says of itself (not the styles it is based on).
TableStyleDef readTableStyle(const Style &style);
// What the properties of a table, a row, or a cell say when those of `src` are put over those of `dst`: its borders, margins, shading, and the other properties
// a style gives (not the ones a table or cell says only of itself: its width, its span, and its merge).
void overlayTableMark(TableMark *dst, const TableMark &src);

// The place the schema gives each child of pPr, rPr, and sectPr (children are written in this order; unknown ones sort last).
int paraChildRank(const QByteArray &name);
int runChildRank(const QByteArray &name);
int sectionChildRank(const QByteArray &name);

// Property elements for writing.
QByteArray runPropsXml(const RunProps &p, bool inParagraphMark = false);
QByteArray paraPropsXml(const ParaProps &p);
QByteArray sectionPropsXml(const SectionProps &s);

} // namespace jp::office::wml

// (A block holds strings, vectors, and pointers to what is shared, none of which points into the block: a list of them grows by moving the bytes.)
Q_DECLARE_TYPEINFO(jp::office::wml::Inline, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(jp::office::wml::Paragraph, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(jp::office::wml::Block, Q_RELOCATABLE_TYPE);
