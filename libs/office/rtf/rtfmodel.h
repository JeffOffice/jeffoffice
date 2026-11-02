#pragma once
// What reading a Rich Text Format file keeps while it goes: the properties of characters, paragraphs, rows, and sections as the file states them,
// the tables the file starts with (fonts, colors, styles, lists), and the functions that write them as the XML of a .docx. Internal to libs/office/rtf.

#include "rtf/rtf.h"

#include <QHash>
#include <QString>
#include <QVector>
#include <climits>
#include <cstring>

namespace jp::office::rtf {

constexpr int kUnset = INT_MIN;

// A border: its kind (an index of kBorderNames, kUnset for none stated), width in twips, color (index of the color table), and distance from the text in twips.
struct Border {
    int style = kUnset, width = 0, color = kUnset, space = 0;
    bool stated() const { return style != kUnset; }
};

// The properties of text as stated: each kUnset where the file says nothing (the style or the default has it); toggles 0 off, 1 on.
struct Chr {
    int font = kUnset, size = kUnset, bold = kUnset, italic = kUnset, strike = kUnset, dstrike = kUnset, caps = kUnset, scaps = kUnset, hidden = kUnset;
    int ul = kUnset, ulColor = kUnset;         // the kind of underline (an index of kUnderlineNames), its color
    int vert = kUnset;                         // 0 normal, 1 superscript, 2 subscript
    int color = kUnset, hl = kUnset, shd = kUnset;   // indexes of the color table
    int spacing = kUnset, scale = kUnset, pos = kUnset, lang = kUnset, style = kUnset;
    int outline = kUnset, shadow = kUnset, emboss = kUnset, imprint = kUnset;
    bool operator==(const Chr &o) const { return std::memcmp(this, &o, sizeof(Chr)) == 0; }
    bool operator!=(const Chr &o) const { return !(*this == o); }
};

struct Tab {
    int pos = 0, type = 0, leader = 0;         // type: 0 left, 1 center, 2 right, 3 decimal, 4 bar; leader: 0 none, 1 dot, 2 hyphen, 3 underscore, 4 thick, 5 equal
};

struct Par {
    int align = kUnset, li = kUnset, ri = kUnset, fi = kUnset, sb = kUnset, sa = kUnset, sl = kUnset, slmult = kUnset;
    int keep = kUnset, keepn = kUnset, pagebb = kUnset, widow = kUnset, outline = kUnset, style = kUnset, ls = kUnset, ilvl = kUnset;
    int contextual = kUnset, rtl = kUnset, shd = kUnset, shdPct = kUnset, noHyph = kUnset;
    int itap = 0;                              // how deep in tables (0 none)
    Border border[5];                          // top, left, bottom, right, between
    QVector<Tab> tabs;
};

struct CellDef {
    int right = 0;
    bool mergeFirst = false, mergeCont = false, vFirst = false, vCont = false;
    Border border[4];                          // top, left, bottom, right
    int shd = kUnset, valign = kUnset, shdPct = kUnset;
};

struct RowDef {
    QVector<CellDef> cells;
    int left = 0, gaph = 0, height = 0, jc = 0;
    bool header = false, cantSplit = false;
    Border border[6];                          // top, left, bottom, right, inside horizontal, inside vertical
    int shd = kUnset;
};

struct Font {
    int index = 0, charset = 0, family = 0, prq = 0, codepage = 0;
    QString name;
};

struct Section {
    int pageW = 12240, pageH = 15840, left = 1800, right = 1800, top = 1440, bottom = 1440, header = 720, footer = 720, gutter = 0;
    bool landscape = false;
    int cols = 1, colSpace = 720;
    bool lineBetween = false;
    int type = kUnset;                         // 0 continuous, 1 column, 2 page, 3 even, 4 odd
    bool titlePage = false;
    int pageStart = kUnset, pageFormat = kUnset;
    int vAlign = 0;
    QVector<int> colWidth, colGap;
};

struct StyleDef {
    int kind = 0;                              // 0 paragraph, 1 character, 2 table
    int id = 0;
    int basedOn = kUnset, next = kUnset;
    bool hidden = false;
    QString name, docxId;
    Par par;
    Chr chr;
};

struct ListLevel {
    int nfc = 0, jc = 0, start = 1, follow = 0, li = kUnset, fi = kUnset, legal = 0, noRestart = 0;
    QString lvlText;                           // the text of the level, with %1 to %9 for the numbers of the levels
    Chr chr;
    bool present = false;
};
struct ListDef {
    qint64 id = 0;
    QVector<ListLevel> levels;
    bool simple = false;
};

// Everything the tables at the start of a file say, and what the parts of the package need of them.
struct Tables {
    QVector<Font> fonts;
    QHash<int, int> fontAt;                    // the file's font number -> place in `fonts`
    QVector<qint32> colors;                    // 0xRRGGBB, or -1 for automatic
    QVector<StyleDef> styles;
    QHash<int, int> paraStyleAt, charStyleAt;  // the file's style number -> place in `styles`
    QVector<ListDef> lists;
    QHash<qint64, int> listAt;                 // the file's list id -> place in `lists`
    struct Num { int list = 0; QVector<ListLevel> overrides; };
    QVector<Num> nums;                         // numId - 1
    QHash<qint64, int> numAt;                  // the file's \ls number -> numId
    int defaultFont = 0, defaultTab = 720, defaultSize = 24, defaultLang = kUnset;
    bool facingPages = false;
};

extern const char *const kBorderNames[];
extern const char *const kUnderlineNames[];
int borderStyleOf(const char *word);

// Whether the letters are one of the control words this reader acts on (the words are matched by a hash, and the hash alone is not enough).
bool isKnownWord(const char *letters, int length);

QByteArray runPropertiesXml(const Chr &c, const Tables &t);
QByteArray paragraphPropertiesXml(const Par &p, const Tables &t, const QString &styleId, bool inStyle);
QByteArray sectionPropertiesXml(const Section &s, const QByteArray &refs);
QByteArray stylesXml(const Tables &t);
QByteArray numberingXml(const Tables &t);
QByteArray fontTableXml(const Tables &t);
QString colorHex(const Tables &t, int index);
QByteArray borderXml(const char *side, const Border &b, const Tables &t);

} // namespace jp::office::rtf
