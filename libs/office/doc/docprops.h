#pragma once
// The property sets of the old binary format and the sprms that change them: characters (Chp), paragraphs (Pap), table rows (Tap), and
// sections (Sep). A set holds only what its sprms state; what a style or the document gives the rest is the XML reader's to resolve.
// Each set can be written as the XML of a .docx. Internal to libs/office/doc.

#include "doc/docformat.h"

#include <QByteArray>
#include <QString>
#include <QVector>
#include <optional>

namespace jp::office::doc {

// A border of the old format: the line's width in eighths of a point, its kind, its color, and its distance from the text.
struct Brc {
    bool stated = false;
    int width = 0, type = 0, space = 0;
    qint32 color = -1;                // 0xRRGGBB, or -1 for automatic
    bool shadow = false, frame = false;
    bool nil = false;                 // stated as "no border" (all bits set)
    bool operator==(const Brc &o) const { return stated == o.stated && width == o.width && type == o.type && space == o.space && color == o.color && shadow == o.shadow && nil == o.nil; }
};
// Shading: a pattern's number, with the colors of its dots and its ground (-1: automatic).
struct Shd {
    bool stated = false;
    qint32 fore = -1, back = -1;
    int pattern = 0;
};

// The toggles of a character, each stated as 0 (off), 1 (on), 0x80 (as the style has it), or 0x81 (the opposite of the style's).
enum Toggle { TogBold, TogItalic, TogStrike, TogDStrike, TogOutline, TogShadow, TogEmboss, TogImprint, TogSmallCaps, TogCaps, TogVanish,
              TogBoldBi, TogItalicBi, TogCount };

// A tracked change of some text: who made it (a place in the file's list of people) and when (the date and time as the file packs them, 0 for none).
struct Revision {
    bool made = false;
    int author = -1;
    quint32 date = 0;
    bool operator==(const Revision &o) const { return made == o.made && (!made || (author == o.author && date == o.date)); }
    bool operator!=(const Revision &o) const { return !(*this == o); }
};

struct Chp {
    std::optional<quint8> tog[TogCount];
    std::optional<int> istd;                      // the character style
    std::optional<int> ftcAscii, ftcFE, ftcOther, ftcBi;   // fonts, as numbers in the font table
    std::optional<int> hps, hpsBi;                // size in half-points
    std::optional<int> hpsPos, dxaSpace, hpsKern, scale;
    std::optional<int> kul, iss, highlight;
    std::optional<qint32> color;                  // 0xRRGGBB, or -1 for automatic
    std::optional<qint32> underlineColor;
    Shd shd;
    Brc brc;
    std::optional<bool> noProof, bidi, complexScripts, webHidden;
    bool special = false, data = false, object = false, ole2 = false;
    Revision insertion, deletion;                 // tracked changes: the text was inserted, deleted, or (when it was inserted and then deleted) both
    qint32 picture = -1;                          // where its picture is in the Data stream (-1: none)
    int symbolFont = -1, symbolChar = 0;          // a symbol from a font, drawn in place of the character
    std::optional<int> lid;

    void apply(const Span &sprms);
};

struct TabStop {
    int pos = 0, jc = 0, leader = 0;
};
struct Frame {
    bool stated = false;
    int anchors = 0;                              // sprmPPc
    int x = 0, y = 0, w = 0, h = 0, wrap = 0;
    int distX = 0, distY = 0;
    bool xStated = false, yStated = false, hStated = false;
};

struct Tc {
    bool firstMerged = false, merged = false, vertMerge = false, vertRestart = false;
    int vAlign = 0, textFlow = 0;
    bool noWrap = false, fitText = false, hideMark = false;
    int width = 0, widthType = 0;                 // the preferred width, in twips or fiftieths of a percent (type 1 automatic, 2 percent, 3 twips; 0 none)
    Brc top, left, bottom, right, tl2br, tr2bl;
    Shd shd;
    int padTop = -1, padLeft = -1, padBottom = -1, padRight = -1;   // -1: the table's
};

// The row of a table: what its end-of-row paragraph states.
struct Tap {
    bool stated = false;
    int jc = 0;
    QVector<int> center;                          // the cell edges, in twips from the margin (cells + 1 of them)
    QVector<Tc> cells;
    int gapHalf = -1;                             // half the space left on each side of a cell's text
    int rowHeight = 0;                            // positive: at least; negative: exactly
    bool cantSplit = false, header = false;
    Brc top, left, bottom, right, insideH, insideV;
    int padTop = -1, padLeft = -1, padBottom = -1, padRight = -1;
    int widthBefore = 0, widthAfter = 0;
    int tableWidth = 0, tableWidthType = 0;       // the preferred width of the table (type as a cell's)
    bool autofit = false;
    std::optional<int> indent;                    // the indent the row states, in twips
    int spacing = 0;                              // space between cells
    std::optional<int> istd;                      // the table style
    bool bidi = false;
    // A table set in the text (its place): the anchors, the distances, and the space left around it.
    bool floating = false;
    int anchors = 0x60, xAbs = 0, yAbs = 0, fromLeft = 0, fromTop = 0, fromRight = 0, fromBottom = 0;
    bool noOverlap = false;
    void apply(const Span &sprms);
    // Whether `o` is a row of the same table: what a table says of itself (its width, borders, style, and place) is the same; the space in the cells of a row is the row's own.
    bool sameTable(const Tap &o) const;
};

struct Pap {
    std::optional<int> istd;
    std::optional<int> jc;
    std::optional<bool> keep, keepNext, pageBreakBefore, widow, noLineNumber, noAutoHyph, bidi, contextual, mirrorIndents, beforeAuto, afterAuto;
    std::optional<int> dxaLeft, dxaLeft1, dxaRight, dyaBefore, dyaAfter;
    std::optional<int> dyaLine;
    std::optional<bool> multiLine;
    std::optional<int> ilfo, ilvl, outline;
    int itap = 0;
    bool inTable = false, ttp = false, innerCell = false, innerTtp = false;
    Brc brcTop, brcLeft, brcBottom, brcRight, brcBetween, brcBar;
    Shd shd;
    bool tabsStated = false;
    QVector<int> tabsCleared;
    QVector<TabStop> tabsAdded;
    Frame frame;
    Tap tap;

    void apply(const Span &sprms, const Span &data = Span());
};

struct Sep {
    std::optional<int> type;                      // 0 continuous, 1 new column, 2 new page, 3 even page, 4 odd page
    int pageW = 12240, pageH = 15840;
    int left = 1800, right = 1800, top = 1440, bottom = 1440, header = 720, footer = 720, gutter = 0;
    bool landscape = false;
    int columns = 1, colSpace = 720;
    bool evenlySpaced = true, lineBetween = false;
    QVector<int> colWidth, colGap;
    bool titlePage = false;
    int numberFormat = 0;
    std::optional<int> pageStart;
    bool restart = false;
    int vAlign = 0;
    bool bidi = false;
    int footnoteNumbering = -1;                   // sprmSRncFtn: 0 continuous, 1 each section, 2 each page
    int endnoteNumbering = -1;
    std::optional<int> footnoteFormat, endnoteFormat, footnoteStart, endnoteStart;   // how the section numbers its notes
    int lineNumbering = 0, lineNumberStart = 1, lineNumberModulus = 1, lineNumberDistance = 0;
    bool hasLineNumbers = false;
    Brc pageBorder[4];                            // top, left, bottom, right
    int pageBorderProps = 0;
    bool stated = false;

    void apply(const Span &sprms);
};

// ---- writing them as XML
// Text for the XML: the characters it can hold, escaped (a string out of a file goes in text or an attribute only through this).
QByteArray xmlEscape(const QString &s);
// The attributes of the mark of a tracked change (w:ins or w:del): its Id, who made it, and when (the file's packed date and time; left out if it is none or isn't a date).
QByteArray revisionAttributes(int id, const QString &author, quint32 date);
// The coding of a color index (ico) and the palette of highlights.
QString colorOfIndex(int ico);
// The name a .docx gives a number format (the code of the old format: 0 decimal, 1 upper roman, 2 lower roman, 3 upper letter, 4 lower letter...).
const char *numberFormatName(int nfc);
QString hexColor(qint32 rgb);

class FontNames;
// The rPr of a character's properties. `parent` says, for each toggle, what the style gives (1 on, 0 off); the toggles stated relative to it
// are settled with it (null: all off). `charStyle` is the id of the character style, if it has one. Empty when nothing is stated.
QByteArray runPropertiesXml(const Chp &c, const FontNames &fonts, const quint8 *parent, const QString &charStyle = QString());
// A border element (`<w:top .../>`) for the side named; empty if the border isn't stated.
QByteArray borderXml(const char *side, const Brc &b);
QByteArray shadingXml(const Shd &s);
// What a paragraph's properties are written with: the id of its style (empty for none), and how many list instances the document has (a
// reference past them is left out).
struct ParaContext {
    QString styleId;
    int listInstances = 0;
};
// The children of a paragraph's pPr (without its run properties or its section), in the order the schema gives; empty if nothing is stated.
QByteArray paragraphPropertiesXml(const Pap &p, const ParaContext &ctx);
// The children of a section's sectPr other than its header and footer references (`refs`, placed first).
QByteArray sectionPropertiesXml(const Sep &s, const QByteArray &refs);

// The fonts of the font table: names by number, with the alternate each gives and its family.
class FontNames {
public:
    struct Entry { QString name, alt; int family = 0; int charset = 0; quint8 prq = 0; };
    QVector<Entry> entries;
    QString name(int ftc) const { return ftc >= 0 && ftc < entries.size() ? entries[ftc].name : QString(); }
};

// Sets the numbers of the toggles that a run's style chain gives, to settle 0x80 and 0x81.
quint8 settleToggle(quint8 stated, quint8 parent);

} // namespace jp::office::doc
