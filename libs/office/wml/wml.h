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
    bool operator==(const Underline &o) const { return val == o.val && color == o.color; }
    bool operator!=(const Underline &o) const { return !(*this == o); }
};

struct RunProps {
    QString style;                    // rStyle
    std::optional<Fonts> fonts;
    std::optional<bool> bold, italic, caps, smallCaps, strike, dstrike, vanish, noProof;
    std::optional<Color> color;
    std::optional<int> spacing;       // twentieths of a point added after each letter
    std::optional<int> position;      // half-points raised (lowered when negative)
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

struct ParaProps {
    QString style;                    // pStyle
    std::optional<bool> keepNext, keepLines, pageBreakBefore, widowControl, contextualSpacing;
    std::optional<int> numId, ilvl;   // numPr
    std::optional<QVector<TabStop>> tabs;
    std::optional<int> before, after; // twentieths of a point
    std::optional<int> line;          // 240ths of a line, or twentieths of a point (see lineRule)
    QString lineRule;                 // auto, exact, atLeast
    std::optional<int> indLeft, indRight;   // twentieths of a point
    std::optional<int> indFirst;      // the first line's, from indLeft; negative for a hanging indent
    QString jc;                       // left, center, right, both, distribute... as written
    std::optional<int> outlineLvl;    // 0 to 8, 9 for body text
    QByteArray spacingAttrs, indAttrs;   // attributes of w:spacing and w:ind that aren't fields here, as written
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
    QVector<RawChild> extra;          // header and footer references, borders, line numbers, grid... as written
};

// What a paragraph holds, in order.
struct Inline {
    // Text is a run of text; a RunAtom is a child of a run that is kept as written (a picture, a field's code or mark, a footnote
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

// An element of the body: a paragraph, or an element that is kept as written (a table; the start or end of a content control).
struct Block {
    bool isRaw = false;
    Paragraph para;
    QByteArray raw;
    QString display;                  // a raw block: the text shown in its place
    Inline::Role role = Inline::Single;
    QString tag, key;
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
    // defines gives the defaults.
    ParaProps paragraph(const QString &id) const;
    // The same, for a style whose own numbering gives its paragraphs the indents of `level` (the properties of the list's level): the
    // level's come below those of the style that says it is numbered, so that style's own indents win over the list's. (When a
    // paragraph itself says it is numbered, the list's come over its style's: that is `paragraph(id)` and then `overlay(level)`.)
    ParaProps paragraphWithNumbering(const QString &id, const ParaProps &level) const;
    // What a run has when nothing is said directly: the defaults, the paragraph style's run properties, then the character style's.
    RunProps run(const QString &paragraphStyle, const QString &characterStyle) const;
    void changed() { m_paragraph.clear(); m_run.clear(); m_index.clear(); m_indexed = -1; }

private:
    void reindex() const;
    mutable QHash<QString, int> m_index;
    mutable qsizetype m_indexed = -1;
    mutable QHash<QString, ParaProps> m_paragraph;
    mutable QHash<QString, RunProps> m_run;
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
    int nextNumId() const;
    int nextAbstractId() const;

private:
    void index() const;
    mutable QHash<int, int> m_nums, m_abstracts;   // the first of each id, by place
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
};

// Reads a .docx (or .docm, .dotx) package. False, with a message, if it isn't a word processing document or is damaged beyond reading.
// A file is held to limits: of parts together inflated, of the XML of a part (xmltree.h), and of what the body holds, past which the rest is
// left out and a note says so. Real files, in 1,795 .docx files of public collections: 125 KB inflated in the middle, 15 MB for 99 of 100, and
// 97.5 MB at the most (of which word/document.xml 20.6 MB); at most 45,000 paragraphs and 65,000 runs (the pieces kept as XML are about as many again). Each limit
// is 10 times the most or more.
constexpr qint64 kMaxInflatedBytes = qint64(1) << 30;
constexpr int kMaxParagraphs = 500000;
constexpr int kMaxInlines = 1500000;
bool readDocx(const QByteArray &zip, Document *doc, QString *error = nullptr, ReadNotes *notes = nullptr);
// The package of a document. Empty, with a message, if it can't be made (an Id or a part name that can't be written).
QByteArray writeDocx(const Document &doc, QString *error = nullptr);
// A new document: Letter paper with one-inch margins, the styles of a new document (Normal, Heading 1 to 3, Title, ...), and a theme.
Document newDocument();

// The built-in definitions of the styles a new document has and a person may apply (Normal, Heading 1 to 9, Title, Subtitle, List
// Paragraph...); used to add one a document lacks. The id is the style's name without spaces ("Heading1"). Empty id when unknown.
Style builtinStyle(const QString &id);
QStringList builtinStyleIds();

// The place the schema gives each child of pPr, rPr, and sectPr (children are written in this order; unknown ones sort last).
int paraChildRank(const QByteArray &name);
int runChildRank(const QByteArray &name);
int sectionChildRank(const QByteArray &name);

// Property elements for writing.
QByteArray runPropsXml(const RunProps &p, bool inParagraphMark = false);
QByteArray paraPropsXml(const ParaProps &p);
QByteArray sectionPropsXml(const SectionProps &s);

} // namespace jp::office::wml
