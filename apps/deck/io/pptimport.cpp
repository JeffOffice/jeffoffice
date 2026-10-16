#include "io/pptimport.h"

#include "core/appinfo.h"
#include "core/blipeffects.h"
#include "core/color.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "core/texteffects.h"
#include "core/transition.h"
#include "io/cfb.h"
#include "io/presentationfile.h"
#include "office/drawingml.h"
#include "office/drawingml_style.h"
#include "office/odraw.h"
#include "office/package.h"
#include "office/theme.h"
#include "office/xml.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QHash>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QPainter>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <functional>
#include <memory>
#include <vector>

namespace jp::deck {

namespace {

// What a file may hold: more is left unread, with a note.
constexpr int kMaxSlides = 10000, kMaxMasters = 1000;
constexpr qint64 kMaxRecords = 1000000;   // read from one stream
constexpr int kMaxEdits = 100000;
// The shapes of every slide and master together (each is an item of a page, which takes about a kilobyte): a file that names one drawing
// from thousands of slides, or has a large drawing for each, can't make memory grow with its slides. The real presentations here hold
// 16 to 500 (91 slides at the most, 35 files), so this is six hundred times the most of them.
constexpr qint64 kMaxTotalShapes = 300000;
// What one text may hold, and all the texts together: more is left out.
constexpr qint64 kMaxTextChars = 1 << 20, kMaxTotalChars = 20 << 20;
constexpr int kMaxRuns = 20000, kMaxParagraphs = 10000;
constexpr qint64 kMaxTotalParagraphs = 200000, kMaxTotalPieces = 500000;
// The notes said of a file: more than this are counted, not listed.
constexpr int kMaxNotes = 20;
constexpr int kMaxFooterChars = 1000;   // of a footer or a date text
constexpr int kMaxPp9Entries = 20000;   // the runs of one text that its extension says more of
// The XML a writer kept for a shape or a theme: the most of one that is read, and of all of them.
// The most pixels of a picture that has its brightness or contrast applied to them, or its tiles moved.
constexpr qint64 kMaxAdjustedPixels = 16'000'000;
// A pattern is a bitmap of a few pixels (8 by 8); one this large is not taken for one.
constexpr qint64 kMaxPatternPixels = 65'536;
// The pieces that splitting a paragraph's text in symbol fonts may add to it: real text has a few (a symbol among words), and a paragraph
// that would have more keeps the rest of its run as it is.
constexpr int kMaxSymbolPieces = 256;
// A shape's is a few kilobytes, packed and unpacked; a theme's package may
// hold the theme's pictures as well. Each is held to its size unpacked too
// (a few packed bytes can unpack to gigabytes), and all of them together.
constexpr qsizetype kMaxWritersXml = 1 << 20;
constexpr qint64 kMaxWritersShapeUnpacked = 1 << 20, kMaxWritersThemeUnpacked = 16 << 20;
constexpr qint64 kMaxTotalWritersXml = 64 << 20;
// The format's unit for places and sizes is 1/576 inch.
constexpr double kPoints = 72.0 / 576.0;
// A slide may be 56 inches across at most (the standard's limit), and is a point at least.
constexpr double kMinSlide = 1, kMaxSlide = 56 * 72;

QString say(const char *text) { return QCoreApplication::translate("PptImport", text); }

// ---- Reading the bytes ----

// Little-endian numbers from a byte array that never reads past an end: a
// read that doesn't fit gives 0 and ends everything after it (ok() is false).
class Cursor {
public:
    Cursor(const QByteArray &d, qint64 pos, qint64 end) : m_d(d), m_pos(pos), m_end(std::min<qint64>(end, d.size()))
    {
        if (pos < 0 || pos > m_end) m_ok = false;
    }
    bool ok() const { return m_ok; }
    qint64 left() const { return m_ok ? m_end - m_pos : 0; }
    qint64 pos() const { return m_pos; }
    quint8 u8() { return quint8(take(1)); }
    quint16 u16() { return quint16(take(2)); }
    qint16 i16() { return qint16(take(2)); }
    quint32 u32() { return quint32(take(4)); }
    qint32 i32() { return qint32(take(4)); }
    void skip(qint64 n)
    {
        if (n < 0 || n > left()) m_ok = false;
        else m_pos += n;
    }

private:
    quint64 take(int n)
    {
        if (!m_ok || m_end - m_pos < n) {
            m_ok = false;
            return 0;
        }
        quint64 v = 0;
        for (int i = 0; i < n; ++i) v |= quint64(quint8(m_d.at(m_pos + i))) << (8 * i);
        m_pos += n;
        return v;
    }
    const QByteArray &m_d;
    qint64 m_pos, m_end;
    bool m_ok = true;
};

// One record's header ([MS-PPT] 2.3.1 RecordHeader) and where its data is.
struct Rec {
    quint16 type = 0, inst = 0;
    quint8 ver = 0;
    qint64 off = 0, len = 0;
    qint64 end() const { return off + len; }
};

// The record whose header is at `pos`, if it and its data lie before `limit`.
bool recAt(const QByteArray &d, qint64 pos, qint64 limit, Rec *r)
{
    Cursor c(d, pos, std::min<qint64>(limit, d.size()));
    const quint16 verInst = c.u16(), type = c.u16();
    const quint32 len = c.u32();
    if (!c.ok() || qint64(len) > c.left()) return false;
    r->ver = verInst & 0xF;
    r->inst = verInst >> 4;
    r->type = type;
    r->off = pos + 8;
    r->len = len;
    return true;
}

// A stream of records, which counts what it hands out: a stream can't be made
// to take forever by filling it with records of nothing.
class Records {
public:
    explicit Records(QByteArray data = QByteArray()) : m_d(std::move(data)) {}
    const QByteArray &data() const { return m_d; }
    QByteArray slice(qint64 off, qint64 len) const { return off >= 0 && len >= 0 && off + len <= m_d.size() ? m_d.mid(off, len) : QByteArray(); }
    // The records one level down, from `start` to `end`; a record that doesn't fit ends the list.
    QVector<Rec> in(qint64 start, qint64 end)
    {
        QVector<Rec> out;
        end = std::min<qint64>(end, m_d.size());
        while (start >= 0 && start + 8 <= end && m_left > 0) {
            Rec r;
            if (!recAt(m_d, start, end, &r)) break;
            out << r;
            --m_left;
            start = r.end();
        }
        if (m_left <= 0) m_exhausted = true;
        return out;
    }
    QVector<Rec> in(const Rec &box) { return in(box.off, box.end()); }
    bool exhausted() const { return m_exhausted; }

private:
    QByteArray m_d;
    qint64 m_left = kMaxRecords;
    bool m_exhausted = false;
};

const Rec *find(const QVector<Rec> &list, quint16 type, int inst = -1)
{
    for (const Rec &r : list)
        if (r.type == type && (inst < 0 || r.inst == inst)) return &r;
    return nullptr;
}

// Record types ([MS-PPT] 2.13.24 RecordType).
enum : quint16 {
    RT_Document = 1000, RT_DocumentAtom = 1001, RT_Slide = 1006, RT_SlideAtom = 1007, RT_Notes = 1008, RT_NotesAtom = 1009, RT_Environment = 1010,
    RT_SlidePersistAtom = 1011, RT_MainMaster = 1016, RT_SlideShowSlideInfoAtom = 1017, RT_ExObjList = 1033, RT_DrawingGroup = 1035, RT_PPDrawing = 1036,
    RT_RoundTripThemeAtom = 1038, RT_RoundTripColorMappingAtom = 1039,
    RT_List = 2000, RT_FontCollection = 2005, RT_ColorSchemeAtom = 2032, RT_PlaceholderAtom = 3011, RT_OutlineTextRefAtom = 3998, RT_TextHeaderAtom = 3999,
    RT_TextCharsAtom = 4000, RT_StyleTextPropAtom = 4001, RT_MasterTextPropAtom = 4002, RT_TextMasterStyleAtom = 4003, RT_TextCFExceptionAtom = 4004,
    RT_TextPFExceptionAtom = 4005, RT_TextRulerAtom = 4006, RT_TextBytesAtom = 4008, RT_TextSpecialInfoAtom = 4010, RT_FontEntityAtom = 4023,
    RT_CString = 4026, RT_ExHyperlinkAtom = 4051, RT_ExHyperlink = 4055, RT_SlideNumberMCAtom = 4056, RT_HeadersFooters = 4057, RT_HeadersFootersAtom = 4058,
    RT_StyleTextProp9Atom = 4012, RT_OutlineTextProps9 = 4014, RT_OutlineTextPropsHeader9Atom = 4015, RT_ProgTags = 5000, RT_ProgBinaryTag = 5002, RT_BinaryTagData = 5003,
    RT_TextInteractiveInfoAtom = 4063, RT_SlideListWithText = 4080, RT_InteractiveInfo = 4082, RT_InteractiveInfoAtom = 4083, RT_UserEditAtom = 4085,
    RT_CurrentUserAtom = 4086, RT_DateTimeMCAtom = 4087, RT_GenericDateMCAtom = 4088, RT_HeaderMCAtom = 4089, RT_FooterMCAtom = 4090,
    RT_PersistDirectoryAtom = 6002,
    // The drawing records ([MS-ODRAW] 2.2).
    RT_DgContainer = 0xF002, RT_SpgrContainer = 0xF003, RT_SpContainer = 0xF004, RT_ClientTextbox = 0xF00D, RT_ClientData = 0xF011,
};

// The text types (TextTypeEnum).
enum { TxTitle = 0, TxBody = 1, TxNotes = 2, TxOther = 4, TxCenterBody = 5, TxCenterTitle = 6, TxHalfBody = 7, TxQuarterBody = 8, TxTypes = 9 };

// ---- Text styles: what a paragraph and a character can say ----

// A color as the format stores it (ColorIndexStruct): its own RGB (index 0xFE), a place in the
// color scheme (0 to 7), or nothing (0xFF).
struct ColorIndex {
    quint8 r = 0, g = 0, b = 0, index = 0xFF;
};

ColorIndex readColor(Cursor &c)
{
    ColorIndex ci;
    ci.r = c.u8();
    ci.g = c.u8();
    ci.b = c.u8();
    ci.index = c.u8();
    return ci;
}

struct Tab {
    qint16 position = 0, type = 0;
};

// The bits of the masks that say which fields a TextPFException or TextCFException holds.
enum : quint32 {
    PfBulletFlags = 0xF, PfBulletFont = 0x10, PfBulletColor = 0x20, PfBulletSize = 0x40, PfBulletChar = 0x80, PfLeft = 0x100, PfIndent = 0x400, PfAlign = 0x800,
    PfLine = 0x1000, PfBefore = 0x2000, PfAfter = 0x4000, PfDefaultTab = 0x8000, PfFontAlign = 0x10000, PfWrap = 0xE0000, PfTabs = 0x100000, PfDirection = 0x200000,
    CfStyle = 0x3EB7, CfFont = 0x10000, CfSize = 0x20000, CfColor = 0x40000, CfPosition = 0x80000, CfEastAsian = 0x200000, CfAnsi = 0x400000, CfSymbol = 0x800000,
};
// The bits of a paragraph's bullet flags and of a character's font style.
enum : quint16 { BulletOn = 1, BulletOwnFont = 2, BulletOwnColor = 4, BulletOwnSize = 8 };
enum : quint16 { StyleBold = 1, StyleItalic = 2, StyleUnderline = 4, StyleShadow = 0x10, StyleEmboss = 0x200 };

// TextPFException: a paragraph's bullet, alignment, spacing, and indents.
struct Pf {
    quint32 has = 0;            // the mask bits of the fields that are set
    quint16 flags = 0;          // BulletOn...
    quint16 bulletChar = 0, bulletFont = 0, align = 0, left = 0, indent = 0, defaultTab = 0;
    qint16 bulletSize = 100, line = 100, before = 0, after = 0;
    ColorIndex bulletColor;
    QVector<Tab> tabs;
};

// TextCFException: a run of characters' style, font, size, color, and height.
struct Cf {
    quint32 has = 0;
    quint16 style = 0;
    quint16 font = 0, ansiFont = 0, symbolFont = 0, size = 0;
    qint16 position = 0;
    ColorIndex color;
};

bool readPf(Cursor &c, Pf *pf)
{
    const quint32 m = c.u32();
    pf->has = m;
    if (m & PfBulletFlags) pf->flags = c.u16();
    if (m & PfBulletChar) pf->bulletChar = c.u16();
    if (m & PfBulletFont) pf->bulletFont = c.u16();
    if (m & PfBulletSize) pf->bulletSize = c.i16();
    if (m & PfBulletColor) pf->bulletColor = readColor(c);
    if (m & PfAlign) pf->align = c.u16();
    if (m & PfLine) pf->line = c.i16();
    if (m & PfBefore) pf->before = c.i16();
    if (m & PfAfter) pf->after = c.i16();
    if (m & PfLeft) pf->left = c.u16();
    if (m & PfIndent) pf->indent = c.u16();
    if (m & PfDefaultTab) pf->defaultTab = c.u16();
    if (m & PfTabs) {
        const int n = c.u16();
        for (int i = 0; i < n && c.ok(); ++i) {
            Tab t;
            t.position = c.i16();
            t.type = c.i16();
            if (pf->tabs.size() < 64) pf->tabs << t;
        }
    }
    if (m & PfFontAlign) c.u16();
    if (m & PfWrap) c.u16();
    if (m & PfDirection) c.u16();
    return c.ok();
}

bool readCf(Cursor &c, Cf *cf)
{
    const quint32 m = c.u32();
    cf->has = m;
    if (m & CfStyle) cf->style = c.u16();
    if (m & CfFont) cf->font = c.u16();
    if (m & CfEastAsian) c.u16();
    if (m & CfAnsi) cf->ansiFont = c.u16();
    if (m & CfSymbol) cf->symbolFont = c.u16();
    if (m & CfSize) cf->size = c.u16();
    if (m & CfColor) cf->color = readColor(c);
    if (m & CfPosition) cf->position = c.i16();
    return c.ok();
}

// What `over` sets, set in `base`.
void mergePf(Pf &base, const Pf &over)
{
    const quint32 m = over.has;
    const quint16 flagBits = quint16(m & PfBulletFlags);
    base.flags = quint16((base.flags & ~flagBits) | (over.flags & flagBits));
    if (m & PfBulletChar) base.bulletChar = over.bulletChar;
    if (m & PfBulletFont) base.bulletFont = over.bulletFont;
    if (m & PfBulletSize) base.bulletSize = over.bulletSize;
    if (m & PfBulletColor) base.bulletColor = over.bulletColor;
    if (m & PfAlign) base.align = over.align;
    if (m & PfLine) base.line = over.line;
    if (m & PfBefore) base.before = over.before;
    if (m & PfAfter) base.after = over.after;
    if (m & PfLeft) base.left = over.left;
    if (m & PfIndent) base.indent = over.indent;
    if (m & PfDefaultTab) base.defaultTab = over.defaultTab;
    if (m & PfTabs) base.tabs = over.tabs;
    base.has |= m;
}

void mergeCf(Cf &base, const Cf &over)
{
    const quint32 m = over.has;
    const quint16 styleBits = quint16(m & CfStyle);
    base.style = quint16((base.style & ~styleBits) | (over.style & styleBits));
    if (m & CfFont) base.font = over.font;
    if (m & CfAnsi) base.ansiFont = over.ansiFont;
    if (m & CfSymbol) base.symbolFont = over.symbolFont;
    if (m & CfSize) base.size = over.size;
    if (m & CfColor) base.color = over.color;
    if (m & CfPosition) base.position = over.position;
    base.has |= m;
}

struct StyleLevel {
    Pf pf;
    Cf cf;
};
// One text type's style on a master (TextMasterStyleAtom): up to five levels,
// each saying only what differs from the level before it.
struct TypeStyle {
    bool present = false;
    int levels = 0;
    StyleLevel level[5];
};
using StyleSet = std::array<TypeStyle, TxTypes>;

TypeStyle readTextMasterStyle(const QByteArray &d, const Rec &r)
{
    TypeStyle t;
    Cursor c(d, r.off, r.end());
    const int n = c.u16();
    for (int i = 0; i < n && i < 5 && c.ok(); ++i) {
        // The styles of the half and quarter body text and the like start each level with its number.
        if (r.inst >= TxCenterBody) c.u16();
        StyleLevel s;
        if (!readPf(c, &s.pf) || !readCf(c, &s.cf)) break;
        t.level[i] = s;
        t.levels = i + 1;
    }
    t.present = t.levels > 0;
    return t;
}

// Where a text type takes what its own style leaves unsaid.
int styleBase(int type)
{
    switch (type) {
    case TxCenterTitle: return TxTitle;
    case TxCenterBody: case TxHalfBody: case TxQuarterBody: return TxBody;
    default: return -1;
    }
}

// ---- Text: what a text box holds ----

struct PfRun {
    qint64 count = 0;
    int level = 0;
    Pf pf;
};
struct CfRun {
    qint64 count = 0;
    Cf cf;
};
struct LangRun {
    qint64 count = 0;
    quint16 lang = 0;
};
// A character of the text that stands for a field: the slide number, a date, the footer.
struct FieldMark {
    qint64 position = 0;
    enum Kind { SlideNumber, Date, DateFormat, Header, Footer } kind = SlideNumber;
    int format = 0;
};
struct LinkMark {
    qint64 start = 0, end = 0;
    QString url;
};
// A text ruler: the indents and tabs a text box sets for itself.
struct Ruler {
    quint32 has = 0;
    quint16 left[5] = {}, indent[5] = {};
    QVector<Tab> tabs;
};

struct LevelRun {
    qint64 count = 0;
    int level = 0;
};

// What the text extension (StyleTextProp9Atom) says of a run of paragraphs: that they are numbered, in which scheme and from which number.
struct Pp9 {
    bool autoNumber = false;
    int scheme = 3;   // TextAutoNumberScheme: 3 is "1."
    int start = 1;
};

// The records of one text: its type, characters, and the runs that style them.
struct TextGroup {
    int type = -1;              // TextTypeEnum; -1 when the group has no header
    bool hasText = false;
    QString text;
    QVector<PfRun> pf;
    QVector<CfRun> cf;
    QVector<LevelRun> levels;   // the paragraph levels a master's text names (MasterTextPropAtom)
    Ruler ruler;
    QVector<LangRun> lang;
    QVector<FieldMark> fields;
    QVector<LinkMark> links;
    QVector<Pp9> pp9;           // by run of paragraphs, what its extension says (from the document's outline properties)
    qint64 outline = -1;        // the text of the slide this shape shows (OutlineTextRefAtom)
    bool bytes = false;         // the text is 8-bit (a TextBytesAtom), its codes 0x80 to 0x9F read as Windows-1252's characters
};

// Windows-1252's characters from 0x80 to 0x9F; the 8-bit text of the format has been written by programs that used them.
const quint16 kWin1252[32] = {0x20AC, 0x81, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x8D, 0x017D, 0x8F,
                              0x90, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x9D, 0x017E, 0x0178};

// The byte of 8-bit text that read as `u`: Windows-1252's characters at 0x80 to 0x9F go back to their codes, the others are their own.
ushort rawByteOf(ushort u)
{
    for (int i = 0; i < 32; ++i)
        if (kWin1252[i] == u) return ushort(0x80 + i);
    return u;
}

QString readChars(const QByteArray &d, const Rec &r, bool wide)
{
    QString s;
    if (wide) {
        const qint64 len = std::min<qint64>(r.len, 2 * kMaxTextChars);
        s.reserve(int(len / 2));
        for (qint64 i = 0; i + 2 <= len; i += 2) s += QChar(quint16(quint8(d.at(r.off + i)) | (quint8(d.at(r.off + i + 1)) << 8)));
    } else {
        const qint64 len = std::min<qint64>(r.len, kMaxTextChars);
        s.reserve(int(len));
        for (qint64 i = 0; i < len; ++i) {
            const quint8 b = quint8(d.at(r.off + i));
            s += QChar(b >= 0x80 && b < 0xA0 ? kWin1252[b - 0x80] : b);
        }
    }
    return s;
}

void readStyleRuns(const QByteArray &d, const Rec &r, qint64 textLength, TextGroup *g)
{
    // The runs cover the text and the paragraph end after it.
    const qint64 need = textLength + 1;
    Cursor c(d, r.off, r.end());
    qint64 total = 0;
    while (total < need && c.left() >= 10 && g->pf.size() < kMaxRuns) {
        PfRun run;
        run.count = c.u32();
        run.level = std::min<int>(c.u16(), 8);
        if (!readPf(c, &run.pf) || run.count <= 0) break;
        g->pf << run;
        total += run.count;
    }
    total = 0;
    while (total < need && c.left() >= 8 && g->cf.size() < kMaxRuns) {
        CfRun run;
        run.count = c.u32();
        if (!readCf(c, &run.cf) || run.count <= 0) break;
        g->cf << run;
        total += run.count;
    }
}

void readRuler(const QByteArray &d, const Rec &r, Ruler *ru)
{
    Cursor c(d, r.off, r.end());
    const quint32 m = c.u32();
    if (m & 1) c.u16();   // the default tab
    if (m & 2) c.u16();   // the number of levels
    if (m & 4) {
        const int n = c.u16();
        for (int i = 0; i < n && c.ok(); ++i) {
            Tab t;
            t.position = c.i16();
            t.type = c.i16();
            if (ru->tabs.size() < 64) ru->tabs << t;
        }
    }
    // Each level's left margin, then its indent.
    for (int k = 0; k < 5; ++k) {
        if (m & (8u << k)) ru->left[k] = c.u16();
        if (m & (0x100u << k)) ru->indent[k] = c.u16();
    }
    if (c.ok()) ru->has = m;
}

// The languages the text runs are in (TextSpecialInfoAtom), when they can be read.
void readLanguages(const QByteArray &d, const Rec &r, TextGroup *g)
{
    Cursor c(d, r.off, r.end());
    while (c.left() >= 8 && g->lang.size() < kMaxRuns) {
        LangRun run;
        run.count = c.u32();
        const quint32 m = c.u32();
        // The fields are the spelling state, the language, an alternate language, and the direction.
        if (m & ~0x47u) break;
        if (m & 1) c.u16();
        if (m & 2) run.lang = c.u16();
        if (m & 4) c.u16();
        if (m & 0x40) c.u16();
        if (!c.ok() || run.count <= 0) break;
        g->lang << run;
    }
}

// A language as the system that made the text numbered it, as the name the text engine's spelling uses.
QString languageName(quint16 lcid)
{
    struct Lang { quint16 id; const char *name; };
    static const Lang names[] = {{0x0409, "en-US"}, {0x0809, "en-GB"}, {0x0C09, "en-AU"}, {0x1009, "en-CA"}, {0x0407, "de-DE"}, {0x0807, "de-CH"}, {0x0C07, "de-AT"},
                                 {0x040C, "fr-FR"}, {0x0C0C, "fr-CA"}, {0x080C, "fr-BE"}, {0x0410, "it-IT"}, {0x0C0A, "es-ES"}, {0x040A, "es-ES"}, {0x080A, "es-MX"},
                                 {0x0816, "pt-PT"}, {0x0416, "pt-BR"}, {0x0413, "nl-NL"}, {0x0415, "pl-PL"}, {0x0419, "ru-RU"}, {0x0411, "ja-JP"}, {0x0412, "ko-KR"},
                                 {0x0804, "zh-CN"}, {0x0404, "zh-TW"}, {0x041D, "sv-SE"}, {0x0414, "nb-NO"}, {0x0406, "da-DK"}, {0x040B, "fi-FI"}, {0x040E, "hu-HU"},
                                 {0x0405, "cs-CZ"}, {0x0408, "el-GR"}, {0x041F, "tr-TR"}, {0x040D, "he-IL"}, {0x0401, "ar-SA"}};
    for (const Lang &l : names)
        if (l.id == lcid) return QString::fromLatin1(l.name);
    return QString();
}

// The date and time formats a field can show (the format numbers of the date field), as the text engine's fields take them.
QString dateFormat(int index)
{
    static const char *formats[] = {"M/d/yyyy", "dddd, MMMM d, yyyy", "d MMMM yyyy", "MMMM d, yyyy", "d-MMM-yy", "MMMM yy", "MMM-yy", "M/d/yyyy h:mm AP",
                                    "M/d/yyyy h:mm:ss AP", "h:mm", "h:mm:ss", "h:mm AP", "h:mm:ss AP"};
    return QString::fromLatin1(formats[std::clamp(index, 0, 12)]);
}

// ---- The presentation's own structure ----

// The slides, masters, or notes a SlideListWithTextContainer lists, each with the persist id of its
// container and the texts the list keeps for it.
struct SlideEntry {
    quint32 persist = 0, slideId = 0;
    QVector<TextGroup> groups;
};

// The theme a master's writer kept with it (the old format's round-trip records hold the 2007 program's), the shape XML names colors by,
// with its list of styles and the master's color mapping.
struct WritersTheme {
    office::Theme theme = office::defaultTheme();
    QDomDocument dom;         // the theme part's XML, which `formats` points into
    QDomElement formats;
    office::ColorMap colors;
};

// Where a master's placeholders are among its items: the first item of each type and of each kind, and the first place of each type that has
// one. Looking for them through all the items, for each placeholder of each slide, takes as long as the items and the placeholders are
// many, multiplied.
struct PlaceholderIndex {
    QHash<QString, const Item *> byType, byKind;
    QHash<QString, QRectF> rectByType;
};

// A slide master: a main master, or a title master, which the title slides of a main master stand on.
struct MasterInfo {
    quint32 persist = 0, slideId = 0;
    int geom = 1;
    int flags = 3;              // follows the master's objects, scheme, background
    QByteArray placeholders;
    bool title = false;
    QColor scheme[8];
    StyleSet styles;
    QString name;
    QByteArray drawing;         // its drawing container (0xF002), header and all
    QMap<QString, QByteArray> shapeProps;   // the property records of its placeholder shapes, by type: what slides' placeholders take after
    std::shared_ptr<WritersTheme> theme;    // the theme its writer kept with it, if it did
    ItemList items;
    mutable std::shared_ptr<const PlaceholderIndex> placeholderIndex;   // made when the masters are all read and a lookup first needs it
    Fill background;
    std::shared_ptr<MasterPage> page;               // the slide master, of a main master
    MasterInfo *main = nullptr;                     // itself, or the main master a title master is part of
    MasterInfo *titleMaster = nullptr;              // of a main master, its title master
    QMap<int, std::shared_ptr<MasterPage>> layouts; // of a main master, by layout geometry
};

// What text needs to know of where it stands: the masters whose styles it takes, and the colors of its scheme.
struct TextCtx {
    const MasterInfo *main = nullptr, *title = nullptr;
    const QColor *scheme = nullptr;
    // Whether the masters' styles apply: text in a text box that is no placeholder takes the presentation's own
    // default text style, not the masters' style for other text (which is that of the footers).
    bool masters = true;
};

// The page being read, for the functions the drawing reader calls.
struct PageState {
    const QColor *scheme = nullptr;
    TextCtx text;
    const MasterInfo *source = nullptr;
    const QVector<TextGroup> *groups = nullptr;   // the texts the slide's list keeps
    QVector<bool> used;
    int contents = 0;                             // content placeholders met so far
    QStringList types;                            // the placeholders met, by type
    bool master = false;
    Fill background;                              // what the page stands on, for shapes filled with the background
};

// The type a placeholder is called by ([MS-PPT] 2.13.21 PlaceholderEnum), as presentation files name it.
QString placeholderName(int id)
{
    switch (id) {
    case 0x01: case 0x0D: case 0x11: return QStringLiteral("title");
    case 0x02: case 0x0E: case 0x06: case 0x0C: case 0x12: return QStringLiteral("body");
    case 0x03: case 0x0F: return QStringLiteral("ctrTitle");
    case 0x04: case 0x10: return QStringLiteral("subTitle");
    case 0x05: case 0x0B: return QStringLiteral("sldImg");
    case 0x07: return QStringLiteral("dt");
    case 0x08: return QStringLiteral("sldNum");
    case 0x09: return QStringLiteral("ftr");
    case 0x0A: return QStringLiteral("hdr");
    case 0x13: case 0x19: return QStringLiteral("obj");
    case 0x14: return QStringLiteral("chart");
    case 0x15: return QStringLiteral("tbl");
    case 0x16: return QStringLiteral("clipArt");
    case 0x17: return QStringLiteral("dgm");
    case 0x18: return QStringLiteral("media");
    case 0x1A: return QStringLiteral("pic");
    default: return QString();
    }
}

bool isTitleType(const QString &t) { return t == QLatin1String("title") || t == QLatin1String("ctrTitle"); }
bool isFooterType(const QString &t) { return t == QLatin1String("dt") || t == QLatin1String("ftr") || t == QLatin1String("sldNum") || t == QLatin1String("hdr"); }
// A placeholder that takes the text and the objects of the slide: it is numbered by its place among them.
bool isContentType(const QString &t) { return !t.isEmpty() && !isTitleType(t) && !isFooterType(t) && t != QLatin1String("sldImg"); }

// The kind of shape a placeholder takes after on a master: a title, a footer of its own kind, or the body.
QString placeholderKind(const QString &t) { return isTitleType(t) ? QStringLiteral("title") : isFooterType(t) ? t : QStringLiteral("body"); }

// The items an item is: itself, and for a group what is in it.
qint64 itemCount(const Item &it, int depth = 0)
{
    qint64 n = 1;
    if (it.type() == ItemType::Group && depth < 64)
        for (const ItemPtr &c : static_cast<const GroupItem &>(it).children) n += itemCount(*c, depth + 1);
    return n;
}

// The type of the placeholder a shape is, from the records of its client data (the contents of its client data record); none when it is no
// placeholder.
QString placeholderOf(const QByteArray &clientData)
{
    QString type;
    Records rs(clientData);
    QVector<Rec> kids = rs.in(0, clientData.size());
    if (kids.size() == 1 && kids.first().type == RT_ClientData) kids = rs.in(kids.first());
    for (const Rec &r : kids) {
        if (r.type != RT_PlaceholderAtom || r.len < 5) continue;
        Cursor c(clientData, r.off, r.end());
        c.u32();
        type = placeholderName(c.u8());
    }
    return type;
}

// The property records of the placeholder shapes of a master's drawing (0xF002), by the placeholder's type, the first of each: what the
// placeholders of the slides on the master take after.
void readShapeProps(const QByteArray &drawing, QMap<QString, QByteArray> *out)
{
    std::function<void(const office::OdrawRecord &, int)> walk = [&](const office::OdrawRecord &box, int depth) {
        if (depth > 8) return;
        for (const office::OdrawRecord &r : office::odrawRecords(drawing, box.offset, box.offset + qsizetype(box.length))) {
            if (r.type == RT_DgContainer || r.type == RT_SpgrContainer) {
                walk(r, depth + 1);
                continue;
            }
            if (r.type != RT_SpContainer) continue;
            QByteArray props, clientData;
            for (const office::OdrawRecord &c : office::odrawRecords(drawing, r.offset, r.offset + qsizetype(r.length))) {
                if (c.type == 0xF00B || c.type == 0xF121 || c.type == 0xF122) props += drawing.mid(c.offset - 8, qsizetype(c.length) + 8);
                else if (c.type == RT_ClientData) clientData = drawing.mid(c.offset, qsizetype(c.length));
            }
            const QString type = placeholderOf(clientData);
            if (!type.isEmpty() && !out->contains(type)) out->insert(type, props);
        }
    };
    for (const office::OdrawRecord &r : office::odrawRecords(drawing)) walk(r, 0);
}

// The text type a placeholder's text has, when its text names none.
int textTypeOf(const QString &placeholder)
{
    if (placeholder == QLatin1String("title")) return TxTitle;
    if (placeholder == QLatin1String("ctrTitle")) return TxCenterTitle;
    if (placeholder == QLatin1String("subTitle")) return TxCenterBody;
    if (isFooterType(placeholder) || placeholder.isEmpty()) return TxOther;
    return TxBody;
}

// A shape's place: the client anchor of a slide, a rectangle of top, left, right, and bottom in master units.
QRectF anchorRect(const QByteArray &a)
{
    Cursor c(a, 0, a.size());
    qint32 top, left, right, bottom;
    if (a.size() >= 16) {
        top = c.i32();
        left = c.i32();
        right = c.i32();
        bottom = c.i32();
    } else if (a.size() >= 8) {
        top = c.i16();
        left = c.i16();
        right = c.i16();
        bottom = c.i16();
    } else {
        return QRectF();
    }
    return QRectF(QPointF(left, top) * kPoints, QPointF(right, bottom) * kPoints).normalized();
}

// The places the slide layouts give their placeholders, as shares of the slide.
QRectF scaled(const QRectF &r, const QSizeF &slide) { return QRectF(r.x() / kSlideWidth * slide.width(), r.y() / kSlideHeight * slide.height(), r.width() / kSlideWidth * slide.width(), r.height() / kSlideHeight * slide.height()); }

QRectF defaultRect(const QString &type, const QSizeF &slide)
{
    if (type == QLatin1String("ctrTitle")) return scaled(QRectF(120, 88.4, 720, 188), slide);
    if (type == QLatin1String("subTitle")) return scaled(QRectF(120, 283.6, 720, 130.4), slide);
    if (isTitleType(type)) return scaled(QRectF(66, 28.75, 828, 104.4), slide);
    if (type == QLatin1String("dt")) return scaled(QRectF(66, 500.5, 216, 28.75), slide);
    if (type == QLatin1String("ftr")) return scaled(QRectF(318, 500.5, 324, 28.75), slide);
    if (type == QLatin1String("sldNum")) return scaled(QRectF(678, 500.5, 216, 28.75), slide);
    return scaled(QRectF(66, 143.75, 828, 342.6), slide);
}

// A layout geometry (SlideLayoutType): what the layout is called in presentation files and in JeffDeck, and its placeholders.
struct LayoutDef {
    QString type, name;
    // The placeholders: type, and which of the places the geometry has.
    struct Place {
        QString type;
        int slot;
    };
    QVector<Place> places;
    int slots = 0;   // how many content places the geometry cuts the body into
};

enum { SlTitleSlide = 0, SlTitleBody = 1, SlTitleOnly = 7, SlTwoColumns = 8, SlTwoRows = 9, SlColumnTwoRows = 10, SlTwoRowsColumn = 11, SlRowTwoColumns = 12,
       SlTwoColumnsRow = 13, SlFourObjects = 14, SlBigObject = 15, SlBlank = 16 };

LayoutDef layoutDef(int geom)
{
    const QString title = QStringLiteral("title"), obj = QStringLiteral("obj");
    auto objects = [&](int n) {
        QVector<LayoutDef::Place> p{{title, 0}};
        for (int i = 0; i < n; ++i) p.append({obj, i + 1});
        return p;
    };
    switch (geom) {
    case SlTitleSlide: return {QStringLiteral("title"), say("Title Slide"), {{QStringLiteral("ctrTitle"), 0}, {QStringLiteral("subTitle"), 1}}, 1};
    case SlTitleBody: return {QStringLiteral("obj"), say("Title and Content"), objects(1), 1};
    case SlTitleOnly: return {QStringLiteral("titleOnly"), say("Title Only"), {{title, 0}}, 0};
    case SlTwoColumns: return {QStringLiteral("twoObj"), say("Two Content"), objects(2), 2};
    case SlTwoRows: return {QStringLiteral("cust"), say("Content Over Content"), objects(2), 2};
    case SlColumnTwoRows: return {QStringLiteral("cust"), say("Content and Two Rows"), objects(3), 3};
    case SlTwoRowsColumn: return {QStringLiteral("cust"), say("Two Rows and Content"), objects(3), 3};
    case SlRowTwoColumns: return {QStringLiteral("cust"), say("Content Over Two Columns"), objects(3), 3};
    case SlTwoColumnsRow: return {QStringLiteral("cust"), say("Two Columns Over Content"), objects(3), 3};
    case SlFourObjects: return {QStringLiteral("cust"), say("Four Content"), objects(4), 4};
    case SlBigObject: return {QStringLiteral("objOnly"), say("Content"), objects(1), 1};
    case SlBlank: return {QStringLiteral("blank"), say("Blank"), {}, 0};
    default: return {QStringLiteral("cust"), say("Custom Layout"), objects(1), 1};
    }
}

// The place of content slot `i` of `n` in a geometry, within the body's place.
QRectF contentSlot(int geom, int i, const QRectF &body)
{
    const double gap = 12, w = (body.width() - gap) / 2, h = (body.height() - gap) / 2;
    const QRectF left(body.x(), body.y(), w, body.height()), right(body.x() + w + gap, body.y(), w, body.height());
    const QRectF top(body.x(), body.y(), body.width(), h), bottom(body.x(), body.y() + h + gap, body.width(), h);
    auto quarter = [&](const QRectF &r, int k) { return QRectF(r.x(), r.y() + (k ? h + gap : 0), r.width(), h); };
    switch (geom) {
    case SlTwoColumns: return i == 0 ? left : right;
    case SlTwoRows: return i == 0 ? top : bottom;
    case SlColumnTwoRows: return i == 0 ? left : quarter(right, i - 1);
    case SlTwoRowsColumn: return i == 2 ? right : quarter(left, i);
    case SlRowTwoColumns: return i == 0 ? top : QRectF(bottom.x() + (i == 2 ? w + gap : 0), bottom.y(), w, bottom.height());
    case SlTwoColumnsRow: return i == 2 ? bottom : QRectF(top.x() + (i == 1 ? w + gap : 0), top.y(), w, top.height());
    case SlFourObjects: return QRectF(body.x() + (i % 2 ? w + gap : 0), body.y() + (i / 2 ? h + gap : 0), w, h);
    default: return body;
    }
}

bool holdsText(const Item &it) { return it.type() == ItemType::Text || it.type() == ItemType::Shape; }

QString &storyIdOf(Item &it)
{
    return it.type() == ItemType::Text ? static_cast<TextItem &>(it).storyId : static_cast<ShapeItem &>(it).storyId;
}

QString sizeName(int type, const QSizeF &size)
{
    switch (type) {
    case 1: return QStringLiteral("Letter Paper");
    case 2: return QStringLiteral("A4 Paper");
    case 3: return QStringLiteral("35mm Slides");
    case 4: return QStringLiteral("Overhead");
    case 5: return QStringLiteral("Banner");
    case 0: {
        const double ratio = size.width() / size.height();
        if (std::abs(ratio - 4.0 / 3) < 0.01) return QStringLiteral("On-screen Show (4:3)");
        if (std::abs(ratio - 16.0 / 9) < 0.01) return QStringLiteral("On-screen Show (16:9)");
        if (std::abs(ratio - 16.0 / 10) < 0.01) return QStringLiteral("On-screen Show (16:10)");
        return QStringLiteral("Custom");
    }
    default: return QStringLiteral("Custom");
    }
}

// A slide's transition from the numbers of its slide show info: the effect (0 none, 4 cover, 5 dissolve, 6 fade, 7 uncover,
// 10 wipe, 11 zoom, 13 split; any other is a fade), its direction (left, up, right, down; for a split or a zoom, 0 and 1 go
// out and in), its speed (slow, medium, fast), when the slide moves on by itself, and whether a click moves it on.
Transition transitionOf(int effect, int direction, int speed, int flags, qint32 timeMs)
{
    Transition t;
    switch (effect) {
    case 0: t.type = QStringLiteral("none"); break;
    case 4: t.type = QStringLiteral("cover"); break;
    case 7: t.type = QStringLiteral("uncover"); break;
    case 10: t.type = QStringLiteral("wipe"); break;
    case 11: t.type = QStringLiteral("zoom"); break;
    case 13: t.type = QStringLiteral("split"); break;
    default: t.type = QStringLiteral("fade"); break;
    }
    if (t.hasDirection()) {
        static const char *const dirs[] = {"l", "u", "r", "d"};
        t.dir = QString::fromLatin1(dirs[direction & 3]);
    } else if (t.hasInOut()) {
        t.dir = direction & 1 ? QStringLiteral("in") : QStringLiteral("out");
    }
    if (effect != 0) t.duration = speed == 0 ? 1.0 : speed == 1 ? 0.7 : 0.5;
    if (flags & 0x100) t.advanceAfter = std::max(0.0, timeMs / 1000.0);
    t.advanceOnClick = (flags & 1) || !(flags & 0x100);
    return t;
}

// The colors a scheme has when a master names none.
const QRgb kDefaultScheme[8] = {0xFFFFFF, 0x000000, 0x808080, 0x000000, 0x99CCFF, 0x333399, 0x0000FF, 0x800080};

// Where a run of `count`s starting at 0 holds a position.
template <class Run>
class RunIndex {
public:
    explicit RunIndex(const QVector<Run> &runs) : m_runs(runs)
    {
        qint64 at = 0;
        for (const Run &r : runs) {
            m_start << at;
            at += r.count;
        }
        m_start << at;
    }
    // The run holding `pos`, or -1.
    int at(qint64 pos) const
    {
        if (m_runs.isEmpty() || pos < 0 || pos >= m_start.last()) return -1;
        return int(std::upper_bound(m_start.cbegin(), m_start.cend(), pos) - m_start.cbegin()) - 1;
    }
    qint64 endOf(int i) const { return m_start[i + 1]; }

private:
    const QVector<Run> &m_runs;
    QVector<qint64> m_start;
};

// What a headers and footers record says (HeadersFootersAtom's flags, and the texts of its strings).
struct FooterSpec {
    bool set = false;
    quint16 formatId = 0, flags = 0;   // the format of an automatic date (an index into the program's list of them)
    QString date, footer;
};
enum : quint16 { FootHasDate = 0x1, FootTodayDate = 0x2, FootUserDate = 0x4, FootSlideNumber = 0x8, FootHasFooter = 0x20 };

class Importer {
public:
    Importer(const QByteArray &file, Document &doc) : m_file(file), m_doc(doc) {}
    bool run(QString *error);

private:
    // The streams and the edits.
    bool openStreams(QString *why);
    bool readEdits(QString *why);
    bool readDirectory(qint64 offset);
    bool containerAt(quint32 persistId, quint16 type, Rec *out);
    QByteArray view(const Rec &r, qint64 headerBytes = 8) const;

    // The document.
    void readDocumentAtom(const Rec &r);
    void readEnvironment(const Rec &r);
    void readDrawingGroup(const Rec &r);
    void readHyperlinks(const Rec &r);
    QVector<SlideEntry> readSlideList(const Rec &list);
    void readTextExtensions(const QVector<Rec> &documentRecords);
    TextGroup readTextGroup(Records &rs, const QVector<Rec> &recs);
    QString pictureId(int blipIndex);
    QString adjustedPicture(const QString &id, double brightness, double contrast);
    QString patternedPicture(const QString &id, const QColor &fillColor, const QColor &backColor);
    QString duotonedPicture(const QString &id, const QColor &dark, const QColor &light);

    // The masters.
    void readMasters();
    void buildMaster(MasterInfo &m);
    const Fill &backgroundOf(const MasterInfo &m) const;
    Fill backgroundAt(const Fill &background, const QRectF &shape);
    void makeMasterPage(MasterInfo &m, int number);
    const Item *masterPlaceholder(const MasterInfo &m, const QString &type) const;
    const PlaceholderIndex &placeholdersOf(const MasterInfo &m) const;
    QByteArray masterShapeProps(const MasterInfo &m, const QString &type) const;
    QRectF masterRect(const MasterInfo &m, const QString &type) const;
    std::shared_ptr<MasterPage> layoutFor(MasterInfo &main, const MasterInfo &source, int geom);
    void assembleMasters();

    // The slides.
    void readSlide(int number, const SlideEntry &e);
    FooterSpec readFooterSpec(const Rec &box);
    void showFooters(const FooterSpec &document);
    QString masterFooterText(const Page &slide) const;
    MasterInfo *masterByRef(quint32 ref) const;
    QString masterPlaceText(const Page &slide, const QString &type, QString *fieldCode = nullptr) const;
    void takeMasterTexts(const FooterSpec &f, const Page &slide, FooterOptions *o) const;
    void finishItems(ItemList &items, PageState &st, bool slide);
    QString readNotes(const SlideEntry &slide, quint32 notesRef, const MasterInfo &master);
    int effectiveGeometry(int geom, const QStringList &types) const;

    // Text.
    QString buildStory(const TextGroup &g, int type, const TextCtx &ctx);
    StyleLevel resolve(int type, int level, const TextCtx &ctx) const;
    QColor colorOf(const ColorIndex &c, const TextCtx &ctx, const QColor &fallback) const;
    QString fontName(const Cf &cf) const;
    QTextCharFormat charFormat(const Cf &cf, const TextCtx &ctx) const;

    // The drawing reader's functions.
    office::OdrawContext drawingContext();
    void clientShape(Item &item, const QByteArray &textbox, const QByteArray &clientData);
    ItemList readShapes(const QByteArray &drawing, PageState &st);

    std::shared_ptr<WritersTheme> readWritersTheme(const QByteArray &themePackage, const QByteArray &colorMapping) const;
    void takeLookFromWritersXml(Item &item, const QByteArray &package);
    void note(const QString &text);
    void noteShapeLimit();

    const QByteArray m_file;
    Document &m_doc;
    cfb::File m_cfb;
    Records m_ppt;
    QByteArray m_pictures;
    quint32 m_documentId = 0;
    QHash<quint32, quint32> m_persist;
    QSet<qint64> m_directories;   // the persist directories read, by offset: one named by many edits is read once
    qint64 m_persistRead = 0;
    bool m_persistFull = false;

    // What the document says.
    QStringList m_fonts;
    Pf m_defaultPf;
    Cf m_defaultCf;
    StyleSet m_styles;
    QHash<quint32, QString> m_links;
    QVector<office::OdrawStoreEntry> m_store;
    QHash<int, int> m_storeAt;   // by the number a shape gives a picture, the store's entry
    QHash<int, QString> m_pictureIds;
    qint64 m_pictureWork = office::kOdrawPictWork;   // what the Macintosh pictures may still do between them
    QHash<quint32, QString> m_pictureAtOffset;   // by where it is in the pictures stream, the picture a store entry points to
    qint64 m_writersXmlBytes = 0;   // the XML of shapes read so far
    QHash<QString, QString> m_adjusted;   // pictures with brightness and contrast applied, by picture and the two
    QVector<SlideEntry> m_slides, m_masterList, m_notesList;
    QSet<qint64> m_slidesRead, m_mastersRead;   // the containers read, by offset: one named by many entries is read once
    office::OdrawBudget m_shapes{kMaxTotalShapes, false};
    bool m_shapeLimitSaid = false;
    std::vector<std::unique_ptr<MasterInfo>> m_masters;
    bool m_mastersBuilt = false;   // their items are as they will be: lookups of their placeholders can use the index
    QHash<quint32, quint32> m_notesOfSlide;   // by slide id, the id of its notes
    QHash<quint32, int> m_notesBySlideId;   // by a notes page's slide id, its first place in m_notesList
    bool m_notesIndexed = false;

    PageState *m_state = nullptr;
    qint64 m_chars = 0, m_paragraphs = 0, m_pieces = 0;
    bool m_charLimitSaid = false, m_paragraphLimitSaid = false;
    int m_layoutCount = 0;
    bool m_footers = false;
    QHash<const Page *, FooterSpec> m_slideFooters;
    QHash<quint64, QVector<Pp9>> m_outlineProps9;   // by slide and kind of text (slideId << 8 | kind)   // the headers and footers a slide sets for itself
    QStringList m_notes;
    int m_extraNotes = 0;

public:
    QString notesText() const
    {
        QStringList lines = m_notes;
        if (m_extraNotes) lines << say("And %1 more.").arg(m_extraNotes);
        return lines.join(QLatin1Char('\n'));
    }
};

void Importer::note(const QString &text)
{
    if (m_notes.size() < kMaxNotes) m_notes << text;
    else ++m_extraNotes;
}

// Said once, when a drawing had more shapes than were left to read (its own limit, or the budget of all of them).
void Importer::noteShapeLimit()
{
    if (!m_shapes.refused || m_shapeLimitSaid) return;
    m_shapeLimitSaid = true;
    note(say("The presentation holds more shapes than can be read; the rest are left out."));
}

// ---- The streams and the edits ----

bool Importer::openStreams(QString *why)
{
    QString cfbWhy;
    if (!cfb::read(m_file, &m_cfb, &cfbWhy)) {
        *why = say("This is not a presentation: %1").arg(cfbWhy);
        return false;
    }
    // The presentation's stream is named for the program that wrote it, ending " Document".
    QByteArray stream;
    bool found = false;
    for (const QString &path : m_cfb.streamPaths()) {
        if (path.contains(QLatin1Char('/')) || !path.endsWith(QLatin1String(" Document"))) continue;
        stream = m_cfb.stream(path);
        found = true;
        break;
    }
    if (!found) {
        if (m_cfb.find(QStringLiteral("EncryptedPackage")) >= 0)
            *why = say("The presentation is protected with a password, which %1 can't open yet.").arg(appInfo().name);
        else
            *why = say("This file holds no presentation.");
        return false;
    }
    m_ppt = Records(stream);
    m_pictures = m_cfb.stream(QStringLiteral("Pictures"));
    return true;
}

bool Importer::readEdits(QString *why)
{
    const QByteArray user = m_cfb.stream(QStringLiteral("Current User"));
    Rec cu;
    if (user.isEmpty() || !recAt(user, 0, user.size(), &cu) || cu.type != RT_CurrentUserAtom || cu.len < 16) {
        *why = say("The presentation doesn't say which edit of it is the last, so it can't be read.");
        return false;
    }
    Cursor c(user, cu.off, cu.end());
    c.u32();   // the size of the fixed part
    const quint32 token = c.u32();
    quint32 offset = c.u32();
    const QString password = say("The presentation is protected with a password, which %1 can't open yet.").arg(appInfo().name);
    if (token == 0xF3D1C4DF) {
        *why = password;
        return false;
    }
    const QByteArray &d = m_ppt.data();
    QSet<quint32> seen;
    bool first = true;
    for (;;) {
        if (seen.contains(offset)) {
            *why = say("The edits of the presentation go round in a circle, so it can't be read.");
            return false;
        }
        seen.insert(offset);
        Rec e;
        if (seen.size() > kMaxEdits || !recAt(d, offset, d.size(), &e) || e.type != RT_UserEditAtom || e.len < 28) {
            *why = say("The presentation is cut short or damaged: a record of one of its edits can't be found.");
            return false;
        }
        Cursor ec(d, e.off, e.end());
        ec.u32();   // the last slide id
        ec.u32();   // the version
        const quint32 lastEdit = ec.u32(), directory = ec.u32(), document = ec.u32();
        ec.u32();   // the seed for new persist ids
        ec.u32();   // the last view and an unused value
        if (e.len >= 32 && ec.u32() != 0) {
            *why = password;
            return false;
        }
        if (first) m_documentId = document;
        first = false;
        if (!readDirectory(directory)) {
            *why = say("The presentation is cut short or damaged: the list of where its parts are can't be read.");
            return false;
        }
        if (m_persistFull) {
            note(say("The presentation lists more of its parts than can be read, so its older edits are left out."));
            break;
        }
        if (lastEdit == 0) break;
        offset = lastEdit;
    }
    return true;
}

// The persist directory at `offset`: where each persist id's record is. The newest edit is read first, and
// the older keep their entries only where it has none. A directory read before has nothing more to give; and the
// directories read all together have no more ids than the stream has bytes (each id takes four, and directories
// that lie inside one another are what make more), after which m_persistFull says no more are read.
bool Importer::readDirectory(qint64 offset)
{
    const QByteArray &d = m_ppt.data();
    Rec r;
    if (!recAt(d, offset, d.size(), &r) || r.type != RT_PersistDirectoryAtom) return false;
    if (m_directories.contains(offset)) return true;
    m_directories.insert(offset);
    Cursor c(d, r.off, r.end());
    while (c.left() >= 4) {
        const quint32 v = c.u32();
        const quint32 first = v & 0xFFFFF, count = v >> 20;
        for (quint32 i = 0; i < count && c.left() >= 4; ++i) {
            const quint32 at = c.u32();
            if (++m_persistRead > d.size()) {
                m_persistFull = true;
                return true;
            }
            if (!m_persist.contains(first + i)) m_persist.insert(first + i, at);
        }
    }
    return true;
}

bool Importer::containerAt(quint32 persistId, quint16 type, Rec *out)
{
    const auto it = m_persist.constFind(persistId);
    if (it == m_persist.constEnd()) return false;
    return recAt(m_ppt.data(), it.value(), m_ppt.data().size(), out) && out->type == type;
}

QByteArray Importer::view(const Rec &r, qint64 headerBytes) const
{
    const qint64 from = r.off - headerBytes;
    return from >= 0 && r.end() <= m_ppt.data().size() ? QByteArray::fromRawData(m_ppt.data().constData() + from, qsizetype(r.len + headerBytes)) : QByteArray();
}

// ---- The document ----

void Importer::readDocumentAtom(const Rec &r)
{
    Cursor c(m_ppt.data(), r.off, r.end());
    const qint32 sx = c.i32(), sy = c.i32(), nx = c.i32(), ny = c.i32();
    c.skip(8);   // the zoom
    c.skip(8);   // the notes and handout masters
    const int firstNumber = c.u16(), type = c.u16();
    QSizeF size(kSlideWidth, kSlideHeight);
    QString name = QStringLiteral("Widescreen");
    if (c.ok()) {
        const QSizeF s(sx * kPoints, sy * kPoints);
        if (s.width() >= kMinSlide && s.width() <= kMaxSlide && s.height() >= kMinSlide && s.height() <= kMaxSlide) {
            size = s;
            name = sizeName(type, s);
        } else {
            note(say("The slide size in the file can't be used, so the slides are 13.333 by 7.5 inches."));
        }
        if (nx > 0 && ny > 0) m_doc.appData[QStringLiteral("notesSize")] = QJsonObject{{"w", nx * kPoints}, {"h", ny * kPoints}};
        m_doc.setup.firstPageNumber = std::clamp(firstNumber, 0, 9999);
    }
    m_doc.setup.size = m_doc.setup.sheet = size;
    m_doc.setup.sizeName = name;
    m_doc.setup.margins = QMarginsF(0, 0, 0, 0);
}

void Importer::readEnvironment(const Rec &r)
{
    const QByteArray &d = m_ppt.data();
    for (const Rec &k : m_ppt.in(r)) {
        if (k.type == RT_FontCollection) {
            for (const Rec &f : m_ppt.in(k)) {
                if (f.type != RT_FontEntityAtom || m_fonts.size() >= 4096) continue;
                // The face name: 32 characters of UTF-16, ended by a zero.
                QString name;
                for (qint64 i = 0; i < 64 && i + 2 <= f.len; i += 2) {
                    const QChar ch(quint16(quint8(d.at(f.off + i)) | (quint8(d.at(f.off + i + 1)) << 8)));
                    if (ch.isNull()) break;
                    name += ch;
                }
                m_fonts << name;
            }
        } else if (k.type == RT_TextCFExceptionAtom) {
            Cursor c(d, k.off, k.end());
            Cf cf;
            if (readCf(c, &cf)) m_defaultCf = cf;
        } else if (k.type == RT_TextPFExceptionAtom) {
            Cursor c(d, k.off, k.end());
            c.u16();
            Pf pf;
            if (readPf(c, &pf)) m_defaultPf = pf;
        } else if (k.type == RT_TextMasterStyleAtom && k.inst < TxTypes) {
            m_styles[k.inst] = readTextMasterStyle(d, k);
        }
    }
}

void Importer::readDrawingGroup(const Rec &r)
{
    for (const Rec &dgg : m_ppt.in(r)) {
        if (dgg.type != 0xF000) continue;
        for (const Rec &b : m_ppt.in(dgg)) {
            // Its default property tables (0xF00B, 0xF121, 0xF122) are what the program that wrote the file starts a new shape from; the
            // program that draws it takes a shape's looks from the shape alone (and from its master's), so they are left unread: text
            // sits at the top and wraps, and a filled shape that names no color is white, whatever the tables say.
            if (b.type != 0xF001) continue;
            m_store = office::odrawStore(view(b));
            // A store given as the records inside the container.
            if (m_store.isEmpty()) m_store = office::odrawStore(view(b, 0));
        }
    }
    // The pictures are numbered from 1 in a shape's property, and the store counts from its own first entry.
    m_storeAt.clear();
    int first = INT_MAX;
    for (const office::OdrawStoreEntry &e : std::as_const(m_store)) first = std::min(first, e.index);
    for (int i = 0; i < m_store.size(); ++i) {
        const qint64 number = qint64(m_store.at(i).index) - first + 1;
        if (number <= INT_MAX && !m_storeAt.contains(int(number))) m_storeAt.insert(int(number), i);
    }
}

void Importer::readHyperlinks(const Rec &list)
{
    const QByteArray &d = m_ppt.data();
    for (const Rec &h : m_ppt.in(list)) {
        if (h.type != RT_ExHyperlink || m_links.size() >= 100000) continue;
        const QVector<Rec> kids = m_ppt.in(h);
        const Rec *atom = find(kids, RT_ExHyperlinkAtom);
        if (!atom || atom->len < 4) continue;
        Cursor ac(d, atom->off, atom->end());
        const quint32 id = ac.u32();
        QString target, location;
        for (const Rec &k : kids) {
            if (k.type != RT_CString) continue;
            if (k.inst == 1) target = readChars(d, k, true);
            else if (k.inst == 3) location = readChars(d, k, true);
        }
        const int zero = target.indexOf(QChar(0));
        if (zero >= 0) target.truncate(zero);
        if (target.isEmpty()) continue;
        if (!location.isEmpty() && !location.contains(QChar(0))) target += QLatin1Char('#') + location;
        m_links.insert(id, target);
    }
}

// The extension of the document's text (the outline properties its ___PPT9 tag keeps: for each slide's text of a kind, what its runs of paragraphs
// say beyond the usual ones). Each run's entry is its paragraph format (a bullet picture, numbering), character format, and special info, each
// with the fields its mask names; only the numbering is kept. A text keeps at most kMaxPp9Entries of them.
void Importer::readTextExtensions(const QVector<Rec> &documentRecords)
{
    const QByteArray &d = m_ppt.data();
    // (The tags are among the records of the document's text information, a list.)
    QVector<Rec> candidates = documentRecords;
    for (const Rec &r : documentRecords)
        if (r.type == RT_List) candidates += m_ppt.in(r);
    for (const Rec &tags : candidates) {
        if (tags.type != RT_ProgTags) continue;
        for (const Rec &tag : m_ppt.in(tags)) {
            if (tag.type != RT_ProgBinaryTag) continue;
            const QVector<Rec> parts = m_ppt.in(tag);
            const Rec *name = find(parts, RT_CString), *data = find(parts, RT_BinaryTagData);
            if (!name || !data || readChars(d, *name, true).section(QChar(0), 0, 0) != QLatin1String("___PPT9")) continue;
            for (const Rec &box : m_ppt.in(*data)) {
                if (box.type != RT_OutlineTextProps9) continue;
                const QVector<Rec> items = m_ppt.in(box);
                for (int i = 0; i + 1 < items.size(); ++i) {
                    if (items[i].type != RT_OutlineTextPropsHeader9Atom || items[i].len < 8 || items[i + 1].type != RT_StyleTextProp9Atom) continue;
                    Cursor h(d, items[i].off, items[i].end());
                    const quint32 slideId = h.u32(), kind = h.u32();
                    QVector<Pp9> entries;
                    Cursor c(d, items[i + 1].off, items[i + 1].end());
                    while (c.ok() && c.left() >= 12 && entries.size() < kMaxPp9Entries) {
                        Pp9 e;
                        const quint32 paragraph = c.u32();
                        if (paragraph & 0x00800000) c.u16();   // the bullet's picture
                        if (paragraph & 0x02000000) e.autoNumber = c.u16() != 0;
                        if (paragraph & 0x01000000) {
                            e.scheme = c.u16();
                            e.start = c.u16();
                        }
                        const quint32 character = c.u32();
                        if (character & 0x00100000) c.u32();
                        const quint32 special = c.u32();
                        if (special & 0x02) c.u16();
                        if (special & 0x04) c.u16();
                        if (special & 0x40) c.u16();
                        if (special & 0x20) c.u32();
                        if (!c.ok()) break;
                        entries << e;
                    }
                    if (h.ok() && kind < 256 && !entries.isEmpty() && m_outlineProps9.size() < kMaxSlides * 4) m_outlineProps9.insert((quint64(slideId) << 8) | kind, entries);
                    ++i;
                }
            }
        }
    }
}

// The slides (or masters, or notes) a list names, in its order, each with the texts it keeps for them.
QVector<SlideEntry> Importer::readSlideList(const Rec &list)
{
    const QByteArray &d = m_ppt.data();
    QVector<SlideEntry> out;
    const QVector<Rec> kids = m_ppt.in(list);
    QSet<quint64> extended;
    int i = 0;
    while (i < kids.size()) {
        const Rec &r = kids[i];
        if (r.type == RT_SlidePersistAtom && r.len >= 16) {
            Cursor c(d, r.off, r.end());
            SlideEntry e;
            e.persist = c.u32();
            c.u32();   // the flags
            c.u32();   // the number of texts
            e.slideId = c.u32();
            if (out.size() < kMaxSlides) out << e;
            ++i;
        } else if (r.type == RT_TextHeaderAtom) {
            // A text runs to the next header or the next slide.
            int j = i + 1;
            while (j < kids.size() && kids[j].type != RT_TextHeaderAtom && kids[j].type != RT_SlidePersistAtom) ++j;
            if (!out.isEmpty() && out.last().groups.size() < 1000) {
                TextGroup g = readTextGroup(m_ppt, kids.mid(i, j - i));
                // The extension's entries for a kind of a slide's text go to the first text of that kind.
                if (const quint64 key = (quint64(out.last().slideId) << 8) | quint64(g.type & 0xFF); g.type >= 0 && !extended.contains(key)) {
                    extended.insert(key);
                    g.pp9 = m_outlineProps9.value(key);
                }
                out.last().groups << g;
            }
            i = j;
        } else {
            ++i;
        }
    }
    return out;
}

// The text in a run of records (the contents of a shape's text box, or a text of a slide list).
TextGroup Importer::readTextGroup(Records &rs, const QVector<Rec> &recs)
{
    const QByteArray &d = rs.data();
    TextGroup g;
    const Rec *style = nullptr;
    QString pendingLink;
    for (const Rec &r : recs) {
        switch (r.type) {
        case RT_TextHeaderAtom: {
            Cursor c(d, r.off, r.end());
            g.type = int(std::min<quint32>(c.u32(), 100));
            break;
        }
        case RT_TextCharsAtom: g.text = readChars(d, r, true); g.hasText = true; g.bytes = false; break;
        case RT_TextBytesAtom: g.text = readChars(d, r, false); g.hasText = true; g.bytes = true; break;
        case RT_StyleTextPropAtom: style = &r; break;
        case RT_MasterTextPropAtom: {
            Cursor c(d, r.off, r.end());
            while (c.left() >= 6 && g.levels.size() < kMaxRuns) {
                const qint64 count = c.u32();
                g.levels << LevelRun{count, std::min<int>(c.u16(), 4)};
            }
            break;
        }
        case RT_TextRulerAtom: readRuler(d, r, &g.ruler); break;
        case RT_TextSpecialInfoAtom: readLanguages(d, r, &g); break;
        case RT_OutlineTextRefAtom: {
            Cursor c(d, r.off, r.end());
            g.outline = c.u32();
            break;
        }
        case RT_SlideNumberMCAtom: case RT_GenericDateMCAtom: case RT_DateTimeMCAtom: case RT_HeaderMCAtom: case RT_FooterMCAtom: {
            Cursor c(d, r.off, r.end());
            FieldMark f;
            f.position = c.u32();
            f.kind = r.type == RT_SlideNumberMCAtom ? FieldMark::SlideNumber : r.type == RT_GenericDateMCAtom ? FieldMark::Date
                     : r.type == RT_DateTimeMCAtom ? FieldMark::DateFormat : r.type == RT_HeaderMCAtom ? FieldMark::Header : FieldMark::Footer;
            if (r.type == RT_DateTimeMCAtom) f.format = c.u8();
            if (c.ok() && g.fields.size() < 10000) g.fields << f;
            break;
        }
        case RT_InteractiveInfo: {
            // A click on this run follows a link; the next TextInteractiveInfoAtom says which run.
            pendingLink.clear();
            if (r.inst != 0) break;
            for (const Rec &k : rs.in(r)) {
                if (k.type != RT_InteractiveInfoAtom || k.len < 9) continue;
                Cursor c(d, k.off, k.end());
                c.u32();
                const quint32 id = c.u32();
                const quint8 action = c.u8();
                if (action == 4) pendingLink = m_links.value(id);
            }
            break;
        }
        case RT_TextInteractiveInfoAtom: {
            Cursor c(d, r.off, r.end());
            LinkMark l;
            l.start = c.u32();
            l.end = c.u32();
            if (c.ok() && r.inst == 0 && !pendingLink.isEmpty() && l.end > l.start && g.links.size() < 1000) {
                l.url = pendingLink;
                g.links << l;
            }
            pendingLink.clear();
            break;
        }
        default: break;
        }
    }
    if (style) readStyleRuns(d, *style, g.text.size(), &g);
    return g;
}

QString Importer::pictureId(int blipIndex)
{
    if (const auto done = m_pictureIds.constFind(blipIndex); done != m_pictureIds.constEnd()) return done.value();
    // A number the store has no picture for costs no more than the look, and isn't kept.
    const auto at = m_storeAt.constFind(blipIndex);
    if (at == m_storeAt.constEnd()) return QString();
    const office::OdrawStoreEntry &e = m_store.at(at.value());
    QString id;
    // The picture itself is in the pictures stream, where the store says, when it isn't in the store: entries that point to one place
    // in it are one picture, made once.
    const bool inStream = e.embedded.isEmpty();
    if (const auto known = inStream ? m_pictureAtOffset.constFind(e.delayOffset) : m_pictureAtOffset.constEnd(); known != m_pictureAtOffset.constEnd()) {
        id = known.value();
    } else {
        QByteArray blip = e.embedded;
        if (inStream) {
            Rec r;
            if (recAt(m_pictures, e.delayOffset, m_pictures.size(), &r) && r.type >= 0xF018 && r.type <= 0xF117) blip = m_pictures.mid(e.delayOffset, qsizetype(r.len + 8));
        }
        QString format;
        // Macintosh pictures are drawn from what all of them in the file may do together; those after it is spent are left out, with the note
        // of pictures left out.
        const bool pict = blip.size() >= 8 && qFromLittleEndian<quint16>(blip.constData() + 2) == 0xF01C;
        const bool spent = pict && m_pictureWork <= 0;
        if (spent) ++m_doc.imagesLeftOut;
        const QByteArray bytes = blip.isEmpty() || spent ? QByteArray() : office::odrawBlip(blip, &format, &m_pictureWork);
        if (!bytes.isEmpty() && m_pictureIds.size() < 20000) id = m_doc.addImage(bytes, format);
        if (inStream) m_pictureAtOffset.insert(e.delayOffset, id);
    }
    m_pictureIds.insert(blipIndex, id);
    return id;
}

// A picture with a brightness and a contrast (a percent each way) in its pixels, as the presentation program draws it (luminanceTable: fitted to
// what the program makes of pictures and of washed-out textures, and of a:lum's). The picture itself when it can't be read.
QString Importer::adjustedPicture(const QString &id, double brightness, double contrast)
{
    const QString key = QStringLiteral("%1|%2|%3").arg(id).arg(brightness).arg(contrast);
    if (const auto it = m_adjusted.constFind(key); it != m_adjusted.constEnd()) return it.value();
    QString result = id;
    QImage img = m_doc.image(id);
    if (!img.isNull() && qint64(img.width()) * img.height() <= kMaxAdjustedPixels && m_adjusted.size() < 200) {
        img = img.convertToFormat(QImage::Format_ARGB32);
        const QVector<uchar> level = luminanceTable(brightness / 100.0, (100.0 + contrast) / 100.0);
        for (int y = 0; y < img.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x) line[x] = qRgba(level[qRed(line[x])], level[qGreen(line[x])], level[qBlue(line[x])], qAlpha(line[x]));
        }
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        if (img.save(&buffer, "PNG")) result = m_doc.addImage(png, QStringLiteral("png"), QString(), img);
    }
    m_adjusted.insert(key, result);
    return result;
}

// A pattern's bitmap (black and white) drawn in the shape's colors, as the presentation program draws a pattern fill: its white bits in the
// fill color and its black ones in the back color (checked against the program's pictures of a master's striped background, whose bitmap has
// three rows of black to one of white and whose pictures show three rows of the back color to one of the fill color). The picture itself
// when it isn't a bitmap of black and white.
QString Importer::patternedPicture(const QString &id, const QColor &fillColor, const QColor &backColor)
{
    const QString key = QStringLiteral("pattern|%1|%2|%3").arg(id, fillColor.name(QColor::HexArgb), backColor.name(QColor::HexArgb));
    if (const auto it = m_adjusted.constFind(key); it != m_adjusted.constEnd()) return it.value();
    // Past the limit on the pictures made, the bitmap stays as it is.
    if (m_adjusted.size() >= 200) return id;
    QString result = id;
    // (The size is asked first: a big picture is not decoded for each pair of colors.)
    const QSize size = m_doc.imageSize(id);
    const QImage img = size.isValid() && qint64(size.width()) * size.height() <= kMaxPatternPixels ? m_doc.image(id) : QImage();
    if (!img.isNull()) {
        QImage tile = img.convertToFormat(QImage::Format_ARGB32);
        bool twoTone = true;
        for (int y = 0; y < tile.height() && twoTone; ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(tile.scanLine(y));
            for (int x = 0; x < tile.width(); ++x) {
                const int v = qGray(line[x]);
                if (qAlpha(line[x]) < 255 || (v > 8 && v < 247)) {
                    twoTone = false;
                    break;
                }
                line[x] = (v <= 8 ? backColor : fillColor).rgba();
            }
        }
        if (twoTone) {
            QByteArray png;
            QBuffer buffer(&png);
            buffer.open(QIODevice::WriteOnly);
            if (tile.save(&buffer, "PNG")) result = m_doc.addImage(png, QStringLiteral("png"), QString(), tile);
        }
    }
    m_adjusted.insert(key, result);
    return result;
}

// A picture in two colors, as the presentation program draws a picture fill that is recolored as a picture: each pixel as light as the picture's
// gray (by the weights of the sRGB colors of a screen) from the dark color, for black, to the light one, for white. The picture itself when it
// is too big (past kMaxAdjustedPixels) or when too many have been made.
QString Importer::duotonedPicture(const QString &id, const QColor &dark, const QColor &light)
{
    const QString key = QStringLiteral("duotone|%1|%2|%3").arg(id, dark.name(), light.name());
    if (const auto it = m_adjusted.constFind(key); it != m_adjusted.constEnd()) return it.value();
    QString result = id;
    const QSize size = m_doc.imageSize(id);
    QImage img = size.isValid() && qint64(size.width()) * size.height() <= kMaxAdjustedPixels && m_adjusted.size() < 200 ? m_doc.image(id) : QImage();
    if (!img.isNull()) {
        img = img.convertToFormat(QImage::Format_ARGB32);
        QRgb ramp[256];
        for (int i = 0; i < 256; ++i)
            ramp[i] = qRgb(dark.red() + (light.red() - dark.red()) * i / 255, dark.green() + (light.green() - dark.green()) * i / 255, dark.blue() + (light.blue() - dark.blue()) * i / 255);
        for (int y = 0; y < img.height(); ++y) {
            QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x) {
                const int gray = int(std::lround(0.2126 * qRed(line[x]) + 0.7152 * qGreen(line[x]) + 0.0722 * qBlue(line[x])));
                line[x] = qRgba(qRed(ramp[gray]), qGreen(ramp[gray]), qBlue(ramp[gray]), qAlpha(line[x]));
            }
        }
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        if (img.save(&buffer, "PNG")) result = m_doc.addImage(png, QStringLiteral("png"), QString(), img);
    }
    m_adjusted.insert(key, result);
    return result;
}

// A page's background as a shape at `shape` shows it: tiles lie where the page's do, so the picture is moved by the shape's place (a
// tile is a picture point at 96 per inch, scaled).
Fill Importer::backgroundAt(const Fill &background, const QRectF &shape)
{
    if (background.type != Fill::Texture && !(background.type == Fill::Picture && background.tile)) return background;
    if (shape.isNull() || (shape.left() == 0 && shape.top() == 0)) return background;
    const double pointsPerPixel = 0.75 * background.tileScale;
    const QString key = QStringLiteral("%1@%2,%3").arg(background.imageId).arg(shape.left() / pointsPerPixel, 0, 'f', 3).arg(shape.top() / pointsPerPixel, 0, 'f', 3);
    Fill f = background;
    if (const auto it = m_adjusted.constFind(key); it != m_adjusted.constEnd()) {
        f.imageId = it.value();
        return f;
    }
    const QImage tile = m_doc.image(background.imageId);
    if (tile.isNull() || qint64(tile.width()) * tile.height() > kMaxAdjustedPixels || m_adjusted.size() >= 200) return background;
    // The tile seen from a point further along, as the brush of the shape starts at its corner.
    QImage moved(tile.size(), QImage::Format_ARGB32);
    moved.fill(Qt::transparent);
    {
        QPainter p(&moved);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        const double dx = std::fmod(shape.left() / pointsPerPixel, tile.width()), dy = std::fmod(shape.top() / pointsPerPixel, tile.height());
        for (int i = -1; i <= 1; ++i)
            for (int j = -1; j <= 1; ++j) p.drawImage(QPointF(-dx + i * tile.width(), -dy + j * tile.height()), tile);
    }
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    QString id = background.imageId;
    if (moved.save(&buffer, "PNG")) id = m_doc.addImage(png, QStringLiteral("png"), QString(), moved);
    m_adjusted.insert(key, id);
    f.imageId = id;
    return f;
}

// The theme a master's round-trip records hold: a package with the theme part, and the XML of the color mapping. None if they can't be read.
std::shared_ptr<WritersTheme> Importer::readWritersTheme(const QByteArray &themePackage, const QByteArray &colorMapping) const
{
    office::Package package;
    if (themePackage.size() > kMaxWritersXml || !package.read(themePackage, nullptr, kMaxWritersThemeUnpacked)) return nullptr;
    QByteArray xml;
    for (const QString &name : package.partNames())
        if (QRegularExpression(QStringLiteral("/theme\\d*\\.xml$"), QRegularExpression::CaseInsensitiveOption).match(name).hasMatch()) {
            xml = package.part(name);
            break;
        }
    auto theme = std::make_shared<WritersTheme>();
    if (xml.isEmpty() || !office::readTheme(xml, &theme->theme) || !office::parseXml(xml, &theme->dom)) return nullptr;
    theme->formats = office::childNamed(office::childNamed(theme->dom.documentElement(), "themeElements"), "fmtScheme");
    QDomDocument map;
    if (!colorMapping.isEmpty() && office::parseXml(colorMapping, &map)) theme->colors = office::readColorMap(map.documentElement());
    return theme;
}

// The look of a shape from the XML its writer kept for it: the fill, and the line, as far as it states them; what it doesn't state the
// old properties have said.
void Importer::takeLookFromWritersXml(Item &item, const QByteArray &packageBytes)
{
    office::Package package;
    QDomDocument dom;
    if (packageBytes.size() > kMaxWritersXml || m_writersXmlBytes > kMaxTotalWritersXml || !package.read(packageBytes, nullptr, kMaxWritersShapeUnpacked)) return;
    const QByteArray xml = package.part(QStringLiteral("/drs/shapexml.xml"));
    if ((m_writersXmlBytes += xml.size()) > kMaxTotalWritersXml || !office::parseXml(xml, &dom)) return;
    const QDomElement properties = office::childNamed(dom.documentElement(), "spPr"), style = office::childNamed(dom.documentElement(), "style");
    if (properties.isNull()) return;
    const MasterInfo *source = m_state ? m_state->source : nullptr;
    const std::shared_ptr<WritersTheme> theme = source && source->theme ? source->theme : source && source->main ? source->main->theme : nullptr;
    office::DrawingContext ctx;
    ctx.package = &package;
    ctx.part = QStringLiteral("/drs/shapexml.xml");
    if (theme) {
        ctx.theme = theme->theme;
        ctx.colors = theme->colors;
        ctx.formats = theme->formats;
    } else {
        ctx.theme = office::defaultTheme();
    }
    ctx.addImage = [this](const QByteArray &bytes, const QString &format) { return m_doc.addImage(bytes, format); };
    ctx.newStory = [this] { return m_doc.createStory(); };
    ctx.storyDoc = [this](const QString &id) { return m_doc.storyDoc(id); };
    if (office::hasOwnFill(properties)) item.fill = office::readFill(properties, style, ctx);
    // A line that says what it is drawn with (and not just how wide).
    const QDomElement line = office::childNamed(properties, "ln");
    for (const char *kind : {"noFill", "solidFill", "gradFill", "pattFill"})
        if (!office::childNamed(line, kind).isNull()) {
            item.stroke = office::readLine(properties, style, ctx);
            break;
        }
}

// ---- The masters ----

void Importer::readMasters()
{
    const QByteArray &d = m_ppt.data();
    for (const SlideEntry &e : std::as_const(m_masterList)) {
        if (int(m_masters.size()) >= kMaxMasters) {
            note(say("Only the first %1 slide masters are read.").arg(kMaxMasters));
            break;
        }
        Rec c;
        // A main master is kept in a master record, and a title master in a slide record.
        if (!containerAt(e.persist, RT_MainMaster, &c) && !containerAt(e.persist, RT_Slide, &c)) {
            note(say("A slide master was left out: it can't be found in the file."));
            continue;
        }
        if (m_mastersRead.contains(c.off)) {
            note(say("A slide master was left out: it holds the same contents as an earlier one."));
            continue;
        }
        m_mastersRead.insert(c.off);
        auto m = std::make_unique<MasterInfo>();
        m->persist = e.persist;
        m->slideId = e.slideId;
        for (int i = 0; i < 8; ++i) m->scheme[i] = QColor(kDefaultScheme[i]);
        const QVector<Rec> kids = m_ppt.in(c);
        if (const Rec *a = find(kids, RT_SlideAtom); a && a->len >= 24) {
            Cursor sc(d, a->off, a->end());
            m->geom = int(sc.u32());
            for (int i = 0; i < 8; ++i) m->placeholders.append(char(sc.u8()));
            sc.skip(8);
            m->flags = sc.u16();
        }
        // The master's own scheme (instance 1; the others are alternatives it offers).
        const Rec *scheme = find(kids, RT_ColorSchemeAtom, 1);
        if (!scheme) scheme = find(kids, RT_ColorSchemeAtom);
        if (scheme && scheme->len >= 32) {
            Cursor sc(d, scheme->off, scheme->end());
            for (int i = 0; i < 8; ++i) {
                const ColorIndex ci = readColor(sc);
                m->scheme[i] = QColor(ci.r, ci.g, ci.b);
            }
        }
        for (const Rec &r : kids) {
            if (r.type == RT_TextMasterStyleAtom && r.inst < TxTypes) m->styles[r.inst] = readTextMasterStyle(d, r);
            else if (r.type == RT_CString && r.inst == 2) m->name = readChars(d, r, true).section(QChar(0), 0, 0);
        }
        if (const Rec *pp = find(kids, RT_PPDrawing)) {
            for (const Rec &k : m_ppt.in(*pp))
                if (k.type == RT_DgContainer) {
                    m->drawing = view(k);
                    break;
                }
        }
        if (const Rec *t = find(kids, RT_RoundTripThemeAtom)) {
            const Rec *map = find(kids, RT_RoundTripColorMappingAtom);
            m->theme = readWritersTheme(view(*t, 0), map ? view(*map, 0) : QByteArray());
        }
        // A title master has the centered title and the subtitle, and a main master the title and the body; a master
        // that lists no placeholders is known by its layout.
        const bool main = m->placeholders.contains(char(1)) || m->placeholders.contains(char(2));
        const bool centered = m->placeholders.contains(char(3)) || m->placeholders.contains(char(4));
        m->title = main ? false : centered || m->geom == 2 || m->geom == 3;
        m_masters.push_back(std::move(m));
    }
    // A title master belongs to the main master before it, else the first after it; with no main master at all it is one.
    MasterInfo *last = nullptr;
    for (auto &m : m_masters) {
        if (!m->title) last = m->main = m.get();
        else m->main = last;
    }
    for (auto &m : m_masters)
        if (m->title && !m->main)
            for (auto &o : m_masters)
                if (!o->title) {
                    m->main = o.get();
                    break;
                }
    for (auto &m : m_masters)
        if (m->title && !m->main) {
            m->title = false;
            m->main = m.get();
        }
    for (auto &m : m_masters)
        if (m->title && !m->main->titleMaster) m->main->titleMaster = m.get();
    // The backgrounds first: a shape of a master or of a slide may be filled with one.
    for (auto &m : m_masters) {
        PageState st;
        st.scheme = m->scheme;
        m_state = &st;
        m->background = office::odrawBackground(m->drawing, drawingContext());
        m_state = nullptr;
        noteShapeLimit();
        if (m->background.isNone()) m->background = Fill::solid(ColorRef::rgb(m->scheme[0]));
    }
    for (auto &m : m_masters) buildMaster(*m);
    int number = 0;
    for (auto &m : m_masters)
        if (!m->title) makeMasterPage(*m, ++number);
    m_mastersBuilt = true;
}

void Importer::buildMaster(MasterInfo &m)
{
    PageState st;
    st.scheme = m.scheme;
    st.text = TextCtx{m.main, m.title ? &m : nullptr, m.scheme};
    st.source = &m;
    st.master = true;
    st.background = backgroundOf(m);
    readShapeProps(m.drawing, &m.shapeProps);
    m.items = readShapes(m.drawing, st);
    finishItems(m.items, st, false);
}

// What a page on the master stands on: the master's background, or for a title master that follows the main master's, that.
const Fill &Importer::backgroundOf(const MasterInfo &m) const
{
    return m.title && (m.flags & 4) && m.main ? m.main->background : m.background;
}

void Importer::makeMasterPage(MasterInfo &m, int number)
{
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("SM%1").arg(number);
    page->name = !m.name.isEmpty() ? m.name : number == 1 ? say("Slide Master") : say("Slide Master %1").arg(number);
    page->abbr = number == 1 ? QStringLiteral("SM") : QStringLiteral("SM%1").arg(number);
    page->props[QStringLiteral("kind")] = QStringLiteral("master");
    page->background = m.background;
    page->items = m.items;
    m.page = page;
}

const PlaceholderIndex &Importer::placeholdersOf(const MasterInfo &m) const
{
    if (!m.placeholderIndex) {
        auto index = std::make_shared<PlaceholderIndex>();
        for (const auto &it : m.items) {
            const QString type = placeholderType(*it);
            if (!index->byType.contains(type)) index->byType.insert(type, it.get());
            if (!it->rect.isEmpty() && !index->rectByType.contains(type)) index->rectByType.insert(type, it->rect);
            if (type.isEmpty()) continue;
            const QString kind = placeholderKind(type);
            if (!index->byKind.contains(kind)) index->byKind.insert(kind, it.get());
        }
        m.placeholderIndex = index;
    }
    return *m.placeholderIndex;
}

// The master's placeholder that holds what a layout's placeholder of `type` shows: the same type, else
// the same kind (a title for a centered title, the body for the rest). The first, in the order of the items.
const Item *Importer::masterPlaceholder(const MasterInfo &m, const QString &type) const
{
    if (!m_mastersBuilt) {
        // The masters' items are still being made (a placeholder with no place of its own takes its master's, which may be one already made).
        for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main}) {
            if (!x) continue;
            for (const auto &it : x->items)
                if (placeholderType(*it) == type) return it.get();
        }
        for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main}) {
            if (!x) continue;
            for (const auto &it : x->items) {
                const QString t = placeholderType(*it);
                if (!t.isEmpty() && placeholderKind(t) == placeholderKind(type)) return it.get();
            }
        }
        return nullptr;
    }
    for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main}) {
        if (!x) continue;
        const PlaceholderIndex &found = placeholdersOf(*x);
        if (const auto it = found.byType.constFind(type); it != found.byType.constEnd()) return it.value();
    }
    const QString kind = placeholderKind(type);
    for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main}) {
        if (!x) continue;
        const PlaceholderIndex &found = placeholdersOf(*x);
        if (const auto it = found.byKind.constFind(kind); it != found.byKind.constEnd()) return it.value();
    }
    return nullptr;
}

// The property records of the master's placeholder shape that a slide's placeholder of this type takes after, like masterPlaceholder.
QByteArray Importer::masterShapeProps(const MasterInfo &m, const QString &type) const
{
    for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main})
        if (x && x->shapeProps.contains(type)) return x->shapeProps.value(type);
    for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main}) {
        if (!x) continue;
        for (auto it = x->shapeProps.cbegin(); it != x->shapeProps.cend(); ++it)
            if (placeholderKind(it.key()) == placeholderKind(type)) return it.value();
    }
    return QByteArray();
}

QRectF Importer::masterRect(const MasterInfo &m, const QString &type) const
{
    for (const MasterInfo *x : std::initializer_list<const MasterInfo *>{&m, m.main}) {
        if (!x) continue;
        if (m_mastersBuilt) {
            const PlaceholderIndex &found = placeholdersOf(*x);
            if (const auto it = found.rectByType.constFind(type); it != found.rectByType.constEnd()) return it.value();
            continue;
        }
        for (const auto &it : x->items)
            if (placeholderType(*it) == type && !it->rect.isEmpty()) return it->rect;
    }
    // The centered title and the subtitle have places of their own on a title slide.
    if (type == QLatin1String("ctrTitle") || type == QLatin1String("subTitle")) return defaultRect(type, m_doc.setup.size);
    if (const Item *t = masterPlaceholder(m, type); t && !t->rect.isEmpty()) return t->rect;
    return defaultRect(type, m_doc.setup.size);
}

// The layout of a geometry on a main master: its placeholders at the master's places (the body cut into
// as many places as the geometry has), with a title master's own objects when the layout stands on one.
std::shared_ptr<MasterPage> Importer::layoutFor(MasterInfo &main, const MasterInfo &source, int geom)
{
    if (const auto it = main.layouts.constFind(geom); it != main.layouts.cend()) return it.value();
    const LayoutDef def = layoutDef(geom);
    auto page = std::make_shared<MasterPage>();
    page->id = QStringLiteral("L%1").arg(++m_layoutCount);
    page->name = def.name;
    page->abbr = QString::number(m_layoutCount);
    page->baseId = main.page->id;
    page->props[QStringLiteral("kind")] = QStringLiteral("layout");
    page->props[QStringLiteral("type")] = def.type;
    if (source.title) {
        page->showMasterItems = source.flags & 1;
        if (!(source.flags & 4)) page->background = source.background;
        // Each layout has its own copy of them: the shapes made so are among those the budget allows.
        for (const auto &it : source.items) {
            if (!it->role.isEmpty()) continue;
            const qint64 count = itemCount(*it);
            if (m_shapes.left < count) {
                m_shapes.refused = true;
                break;
            }
            m_shapes.left -= count;
            page->items.push_back(m_doc.cloneItem(*it));
        }
        noteShapeLimit();
    }
    const QRectF body = masterRect(source, QStringLiteral("body"));
    auto placeholder = [&](const QString &type, int index, const QRectF &rect) {
        ItemPtr it;
        if (const Item *tpl = masterPlaceholder(source, type)) {
            it = m_doc.cloneItem(*tpl);
        } else {
            auto t = std::make_shared<TextItem>();
            t->insets = QMarginsF(7.2, 3.6, 7.2, 3.6);
            t->hyphenate = false;
            t->storyId = m_doc.createStory();
            it = t;
        }
        it->role = placeholderRole(type, index);
        it->name = say("%1 Placeholder").arg(type);
        it->rect = rect;
        page->items.push_back(it);
    };
    for (const LayoutDef::Place &pl : def.places) {
        if (isTitleType(pl.type) || pl.type == QLatin1String("subTitle")) placeholder(pl.type, pl.type == QLatin1String("subTitle") ? 1 : 0, masterRect(source, pl.type));
        else placeholder(pl.type, pl.slot, def.slots > 1 ? contentSlot(geom, pl.slot - 1, body) : body);
    }
    for (const QString &type : {QStringLiteral("dt"), QStringLiteral("ftr"), QStringLiteral("sldNum")}) {
        const Item *footer = masterPlaceholder(source, type);
        if (footer && placeholderType(*footer) == type) placeholder(type, 0, masterRect(source, type));
    }
    main.layouts.insert(geom, page);
    return page;
}

// The masters and their layouts in the order the presentation lists them, with the layouts every
// presentation has (a title slide, a title and content, a title only, and a blank one) among them.
void Importer::assembleMasters()
{
    for (auto &m : m_masters) {
        if (m->title) continue;
        for (int geom : {int(SlTitleSlide), int(SlTitleBody), int(SlTitleOnly), int(SlBlank)}) {
            const MasterInfo &source = geom == SlTitleSlide && m->titleMaster ? *m->titleMaster : *m;
            layoutFor(*m, source, geom);
        }
    }
    m_doc.masters.clear();
    static const int order[] = {SlTitleSlide, SlTitleBody, SlTitleOnly, SlTwoColumns, SlTwoRows, SlColumnTwoRows, SlTwoRowsColumn, SlRowTwoColumns, SlTwoColumnsRow, SlFourObjects, SlBigObject, SlBlank};
    for (auto &m : m_masters) {
        if (m->title) continue;
        m_doc.masters << m->page;
        QSet<int> done;
        for (int geom : order)
            if (m->layouts.contains(geom)) {
                m_doc.masters << m->layouts.value(geom);
                done.insert(geom);
            }
        for (auto it = m->layouts.cbegin(); it != m->layouts.cend(); ++it)
            if (!done.contains(it.key())) m_doc.masters << it.value();
    }
}

// ---- The slides ----

MasterInfo *Importer::masterByRef(quint32 ref) const
{
    for (const auto &m : m_masters)
        if (m->slideId == ref) return m.get();
    for (const auto &m : m_masters)
        if (m->persist == ref) return m.get();
    for (const auto &m : m_masters)
        if (!m->title) return m.get();
    return m_masters.empty() ? nullptr : m_masters.front().get();
}

// The layout geometry a slide stands on: the one its atom names, except that a slide called blank
// with placeholders on it, as newer programs write them, stands on the layout those placeholders make.
int Importer::effectiveGeometry(int geom, const QStringList &types) const
{
    const bool known = geom == SlTitleSlide || geom == SlTitleBody || (geom >= SlTitleOnly && geom <= SlBlank);
    if (known && geom != SlBlank) return geom;
    int contents = 0;
    for (const QString &t : types) contents += isContentType(t) && t != QLatin1String("subTitle");
    if (types.contains(QStringLiteral("ctrTitle"))) return SlTitleSlide;
    if (types.contains(QStringLiteral("title"))) {
        if (contents == 0) return SlTitleOnly;
        if (contents == 2) return SlTwoColumns;
        if (contents >= 4) return SlFourObjects;
        return SlTitleBody;
    }
    return known ? geom : SlBlank;
}

// A text box in place of a rectangle that holds a placeholder's text.
ItemPtr textBoxFor(const ShapeItem &s)
{
    auto t = std::make_shared<TextItem>();
    t->noWrap = s.noWrap;
    t->centerBlock = s.centerBlock;
    t->name = s.name;
    t->role = s.role;
    t->altText = s.altText;
    t->hyperlink = s.hyperlink;
    t->rect = s.rect;
    t->rotation = s.rotation;
    t->flipH = s.flipH;
    t->flipV = s.flipV;
    t->locked = s.locked;
    t->fill = s.fill;
    t->stroke = s.stroke;
    t->fx = s.fx;
    t->wrap = s.wrap;
    t->storyId = s.storyId;
    t->insets = s.insets;
    t->valign = s.valign;
    return t;
}

// The placeholders of a page made ready: text boxes, in their master's place when the drawing gave none, and
// (on a slide) the texts the presentation's list keeps for the slide that the drawing didn't show.
void Importer::finishItems(ItemList &items, PageState &st, bool slide)
{
    const MasterInfo &source = *st.source;
    for (auto &it : items) {
        const QString type = placeholderType(*it);
        if (type.isEmpty()) continue;
        if (it->type() == ItemType::Shape) {
            const auto &s = static_cast<const ShapeItem &>(*it);
            if (s.shape == QLatin1String("rect") && s.customPath.isEmpty()) it = textBoxFor(s);
        }
        // A placeholder with no place of its own stands where the master has its kind.
        if (it->rect.isEmpty() || it->rect == QRectF(72, 72, 144, 72)) it->rect = masterRect(source, type);
        if (it->name.isEmpty()) it->name = say("%1 Placeholder").arg(type);
        if (it->type() != ItemType::Text) continue;
        auto *t = static_cast<TextItem *>(it.get());
        t->hyphenate = false;
        // A slide's placeholder sits in its text where its master's does, unless it says otherwise.
        if (slide && t->valign == VAlign::Top)
            if (const Item *tpl = masterPlaceholder(source, type); tpl && tpl->type() == ItemType::Text) t->valign = static_cast<const TextItem *>(tpl)->valign;
        if (t->storyId.isEmpty()) t->storyId = m_doc.createStory();
        const QTextDocument *story = m_doc.storyDoc(t->storyId);
        if (slide && story && story->isEmpty()) t->prompt = promptFor(type);
        if (slide && isFooterType(type)) m_footers = true;
    }
    if (!slide || !st.groups) return;
    for (int i = 0; i < st.groups->size(); ++i) {
        if (st.used.value(i)) continue;
        const TextGroup &g = st.groups->at(i);
        QString type;
        switch (g.type) {
        case TxTitle: type = QStringLiteral("title"); break;
        case TxCenterTitle: type = QStringLiteral("ctrTitle"); break;
        case TxBody: case TxHalfBody: case TxQuarterBody: type = QStringLiteral("body"); break;
        case TxCenterBody: type = QStringLiteral("subTitle"); break;
        default: break;
        }
        if (type.isEmpty() || !g.hasText) continue;
        auto t = std::make_shared<TextItem>();
        t->role = placeholderRole(type, isTitleType(type) ? 0 : ++st.contents);
        t->name = say("%1 Placeholder").arg(type);
        t->rect = masterRect(source, type);
        t->insets = QMarginsF(7.2, 3.6, 7.2, 3.6);
        t->valign = isTitleType(type) ? VAlign::Middle : VAlign::Top;
        t->hyphenate = false;
        t->storyId = buildStory(g, g.type, st.text);
        st.types << type;
        items.push_back(t);
    }
}

void Importer::readSlide(int number, const SlideEntry &e)
{
    Rec c;
    if (!containerAt(e.persist, RT_Slide, &c)) {
        note(say("Slide %1 was left out: it can't be found in the file.").arg(number));
        return;
    }
    // Entries of the list that name one slide would each have it read, and its drawing made, again.
    if (m_slidesRead.contains(c.off)) {
        note(say("Slide %1 was left out: it holds the same contents as an earlier slide.").arg(number));
        return;
    }
    m_slidesRead.insert(c.off);
    const QByteArray &d = m_ppt.data();
    const QVector<Rec> kids = m_ppt.in(c);
    int geom = SlBlank, flags = 7;
    quint32 masterRef = 0, notesRef = 0;
    if (const Rec *a = find(kids, RT_SlideAtom); a && a->len >= 24) {
        Cursor sc(d, a->off, a->end());
        geom = int(sc.u32());
        sc.skip(8);
        masterRef = sc.u32();
        notesRef = sc.u32();
        flags = sc.u16();
    }
    MasterInfo *source = masterByRef(masterRef);
    if (!source) {
        note(say("Slide %1 was left out: the presentation has no slide master.").arg(number));
        return;
    }

    auto page = std::make_shared<Page>();
    page->showMasterItems = flags & 1;
    // The slide's own colors, unless it follows its master's.
    QColor own[8];
    bool hasOwn = false;
    if (const Rec *s = find(kids, RT_ColorSchemeAtom, 1); s && s->len >= 32) {
        Cursor sc(d, s->off, s->end());
        for (int i = 0; i < 8; ++i) {
            const ColorIndex ci = readColor(sc);
            own[i] = QColor(ci.r, ci.g, ci.b);
        }
        hasOwn = true;
    }
    const bool ownColors = hasOwn && !(flags & 2);
    // How the slide ends and moves on: the flags hide it, and the effect, its direction and speed, and when the
    // slide moves on are the slide's transition. What the file says is also kept as it is, when there is an effect.
    if (const Rec *info = find(kids, RT_SlideShowSlideInfoAtom); info && info->len >= 16) {
        Cursor ic(d, info->off, info->end());
        const qint32 time = ic.i32();
        ic.u32();
        const int direction = ic.u8(), effect = ic.u8(), infoFlags = ic.u16(), speed = ic.u8();
        page->hidden = infoFlags & 0x4;
        if (effect != 0)
            page->props[QStringLiteral("binaryTransition")] = QJsonObject{{"effect", effect}, {"direction", direction}, {"speed", speed},
                                                                          {"advanceOnTime", bool(infoFlags & 0x100)}, {"advanceMs", time}, {"advanceOnClick", bool(infoFlags & 1)}};
        setTransition(*page, transitionOf(effect, direction, speed, infoFlags, time));
    }

    PageState st;
    st.scheme = ownColors ? own : source->scheme;
    st.text = TextCtx{source->main, source->title ? source : nullptr, st.scheme};
    st.source = source;
    st.groups = &e.groups;
    st.used.fill(false, e.groups.size());
    QByteArray drawing;
    if (const Rec *pp = find(kids, RT_PPDrawing)) {
        for (const Rec &k : m_ppt.in(*pp))
            if (k.type == RT_DgContainer) {
                drawing = view(k);
                break;
            }
    }
    // The slide's own background, unless it follows its master's.
    Fill bg;
    if (!(flags & 4)) {
        m_state = &st;
        bg = office::odrawBackground(drawing, drawingContext());
        m_state = nullptr;
        noteShapeLimit();
        if (bg.isNone() && ownColors) bg = Fill::solid(ColorRef::rgb(own[0]));
        page->background = bg;
    }
    st.background = bg.isNone() ? backgroundOf(*source) : bg;
    ItemList items = readShapes(drawing, st);
    finishItems(items, st, true);
    page->items = std::move(items);
    if (const Rec *hf = find(kids, RT_HeadersFooters)) {
        const FooterSpec own = readFooterSpec(*hf);
        if (own.set) m_slideFooters.insert(page.get(), own);
    }
    for (const auto &it : page->items) {
        const QString type = placeholderType(*it);
        if (isTitleType(type) && it->type() == ItemType::Text) {
            if (const QTextDocument *story = m_doc.storyDoc(static_cast<const TextItem *>(it.get())->storyId))
                page->title = story->toPlainText().section(QLatin1Char('\n'), 0, 0).section(QChar(QChar::LineSeparator), 0, 0).trimmed();
            break;
        }
    }
    const std::shared_ptr<MasterPage> layout = layoutFor(*source->main, *source, effectiveGeometry(geom, st.types));
    page->masterId = layout->id;
    page->notesStoryId = readNotes(e, notesRef, *source);
    m_doc.pages << page;
}

// The flags and strings of a headers and footers record: the user's date (instance 0) and the footer (2), each cut short past a line's worth.
FooterSpec Importer::readFooterSpec(const Rec &box)
{
    FooterSpec spec;
    const QByteArray &d = m_ppt.data();
    for (const Rec &k : m_ppt.in(box)) {
        if (k.type == RT_HeadersFootersAtom && k.len >= 4) {
            Cursor c(d, k.off, k.end());
            spec.formatId = c.u16();
            spec.flags = c.u16();
            spec.set = c.ok();
        } else if (k.type == RT_CString && (k.inst == 0 || k.inst == 2)) {
            const QString text = readChars(d, k, true).section(QChar(0), 0, 0).left(kMaxFooterChars);
            (k.inst == 0 ? spec.date : spec.footer) = text;
        }
    }
    return spec;
}

// The generic date a story holds (the field the date place of a slide has, which takes the format of the headers and footers record) made a field
// of the code given. (The fields are found first, then changed: a change splits the story's fragments.)
static void retargetGenericDate(QTextDocument *story, const QString &code)
{
    constexpr int kMostFields = 100;
    QVector<QPair<int, int>> found;
    for (QTextBlock b = story->begin(); b.isValid() && found.size() < kMostFields; b = b.next())
        for (auto it = b.begin(); !it.atEnd() && found.size() < kMostFields; ++it)
            if (it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("date")) found << qMakePair(it.fragment().position(), it.fragment().length());
    for (const auto &f : std::as_const(found)) {
        QTextCursor c(story);
        c.setPosition(f.first);
        c.setPosition(f.first + f.second, QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setProperty(tp::Field, code);
        c.mergeCharFormat(format);
    }
}

// What a record says is shown: the date (the user's own as the text it is, or today's in the format the record names), the slide number, and
// the footer (the text the record holds, else the one the master's footer place holds, which a slide shows when it shows its footer).
static FooterOptions footerOptionsOf(const FooterSpec &f)
{
    FooterOptions o;
    const bool today = f.flags & FootTodayDate;
    o.date = (f.flags & FootHasDate) && (today || ((f.flags & FootUserDate) && !f.date.isEmpty()));
    if (o.date) {
        if (today) o.dateCode = QStringLiteral("datetime:") + dateFormat(f.formatId > 12 ? 0 : f.formatId);
        else o.dateText = f.date;
    }
    o.slideNumber = f.flags & FootSlideNumber;
    o.footer = f.flags & FootHasFooter;
    o.footerText = f.footer;
    return o;
}

// The text a slide's master holds in its footer place ("*" stands for the field there, not for text), cut at a footer's most (each slide shows it).
QString Importer::masterFooterText(const Page &slide) const
{
    const QString held = layoutFooterText(m_doc, slide);
    return held == QLatin1String("*") ? QString() : held.left(kMaxFooterChars);
}

// The date, slide number, and footer the document (or a slide) says its slides show, each on the slides that lack it, where their layout has the
// place for it: a slide keeps the places it has of its own.
void Importer::showFooters(const FooterSpec &document)
{
    if (document.set) {
        FooterOptions all = footerOptionsOf(document);
        // (What new slides will show: the footer's text, if the record has none, is the first slide's master's.)
        if (all.footer && all.footerText.isEmpty() && !m_doc.pages.isEmpty()) all.footerText = masterFooterText(*m_doc.pages.first());
        if (all.footer && all.footerText.isEmpty()) all.footer = false;
        m_doc.appData[QStringLiteral("footerOptions")] = all.toJson();
        if (all.any()) m_footers = true;
    }
    for (const auto &page : m_doc.pages) {
        const auto set = m_slideFooters.constFind(page.get());
        const FooterSpec &spec = set != m_slideFooters.cend() ? set.value() : document;
        if (!spec.set) continue;
        FooterOptions o = footerOptionsOf(spec);
        // A footer whose text the record leaves out is the master's footer place's.
        if (o.footer && o.footerText.isEmpty()) o.footerText = masterFooterText(*page);
        if (o.footer && o.footerText.isEmpty()) o.footer = false;
        // A date place the slide has of its own, holding the generic date field, shows it in the format the record names.
        if (!o.dateCode.isEmpty())
            for (const auto &it : page->items)
                if (placeholderType(*it) == QLatin1String("dt") && it->type() == ItemType::Text)
                    if (QTextDocument *story = m_doc.storyDoc(static_cast<TextItem &>(*it).storyId)) retargetGenericDate(story, o.dateCode);
        if (!o.any()) continue;
        addMissingFooters(m_doc, o, *page);
    }
// The text typed in the place of that type ("dt" or "sldNum") of a slide's master (a field in it is U+FFFC), cut at a footer's most; empty when the
// place holds only fields, which is the program's own date or number, not text.
QString Importer::masterPlaceText(const Page &slide, const QString &type, QString *fieldCode) const
{
    const QString held = layoutPlaceText(m_doc, slide, type, fieldCode);
    if (QString(held).remove(QChar(QChar::ObjectReplacementCharacter)).trimmed().isEmpty()) return QString();
    return held.left(kMaxFooterChars);
}

// What the master's places for the date and the slide number say a slide shows, where the record leaves it to them: a date the record shows
// without saying which (not today's, and none of its own) is the text typed in the master's date place, and the number takes the text the master's
// number place holds around its field.
void Importer::takeMasterTexts(const FooterSpec &f, const Page &slide, FooterOptions *o) const
{
    if ((f.flags & FootHasDate) && !o->date) {
        QString code;
        if (const QString text = masterPlaceText(slide, QStringLiteral("dt"), &code); !text.isEmpty()) {
            o->date = true;
            o->dateText = text;
            o->dateCode = code;
        }
    }
    if (o->slideNumber) o->numberText = masterPlaceText(slide, QStringLiteral("sldNum"));
}

}

// The text box of a placeholder in a drawing, looking into its groups too.
bool findPlaceholderText(Records &rs, const Rec &box, const QSet<int> &ids, int depth, Rec *textbox)
{
    const QByteArray &d = rs.data();
        if (!m_doc.pages.isEmpty()) takeMasterTexts(document, *m_doc.pages.first(), &all);
    for (const Rec &r : rs.in(box)) {
        if (r.type == RT_SpContainer) {
            const QVector<Rec> kids = rs.in(r);
            const Rec *data = find(kids, RT_ClientData), *text = find(kids, RT_ClientTextbox);
            if (!data || !text) continue;
            for (const Rec &a : rs.in(*data)) {
                if (a.type != RT_PlaceholderAtom || a.len < 5) continue;
                Cursor c(d, a.off, a.end());
                c.u32();
                if (ids.contains(c.u8())) {
                    *textbox = *text;
                    return true;
                }
            }
        } else if ((r.type == RT_SpgrContainer || r.type == RT_DgContainer) && depth < 8) {
            if (findPlaceholderText(rs, r, ids, depth + 1, textbox)) return true;
        }
    }
    return false;
        takeMasterTexts(spec, *page, &o);
}

// The story of a slide's speaker notes: the text of the notes page's body placeholder, or the notes
// the presentation's list keeps for it.
QString Importer::readNotes(const SlideEntry &slide, quint32 notesRef, const MasterInfo &master)
{
    // The notes of a slide are named by the slide, or name it.
    const SlideEntry *entry = nullptr;
    if (notesRef)
        if (const auto it = m_notesBySlideId.constFind(notesRef); it != m_notesBySlideId.constEnd()) entry = &m_notesList.at(it.value());
    if (!entry) {
        if (!m_notesIndexed) {
            m_notesIndexed = true;
            for (const SlideEntry &n : std::as_const(m_notesList)) {
                Rec nc;
                if (!containerAt(n.persist, RT_Notes, &nc)) continue;
                const QVector<Rec> parts = m_ppt.in(nc);
                if (const Rec *a = find(parts, RT_NotesAtom); a && a->len >= 4) {
                    Cursor c(m_ppt.data(), a->off, a->end());
                    m_notesOfSlide.insert(c.u32(), n.slideId);
                }
            }
        }
        const quint32 id = m_notesOfSlide.value(slide.slideId);
        if (id)
            if (const auto it = m_notesBySlideId.constFind(id); it != m_notesBySlideId.constEnd()) entry = &m_notesList.at(it.value());
    }
    if (!entry) return m_doc.createStory();

    TextCtx ctx{master.main, nullptr, master.main->scheme};
    TextGroup g;
    Rec nc, textbox;
    if (containerAt(entry->persist, RT_Notes, &nc)) {
        const QVector<Rec> parts = m_ppt.in(nc);
        if (const Rec *pp = find(parts, RT_PPDrawing); pp && findPlaceholderText(m_ppt, *pp, {0x06, 0x0C}, 0, &textbox)) g = readTextGroup(m_ppt, m_ppt.in(textbox));
    }
    if (g.outline >= 0 && g.outline < entry->groups.size()) g = entry->groups.at(int(g.outline));
    if (!g.hasText)
        for (const TextGroup &t : entry->groups)
            if (t.type == TxNotes && t.hasText) {
                g = t;
                break;
            }
    return g.hasText ? buildStory(g, TxNotes, ctx) : m_doc.createStory();
}

// ---- Text ----

QString Importer::fontName(const Cf &cf) const
{
    if (cf.has & CfFont)
        if (cf.font < m_fonts.size() && !m_fonts.at(cf.font).isEmpty()) return m_fonts.at(cf.font);
    if (cf.has & CfAnsi)
        if (cf.ansiFont < m_fonts.size()) return m_fonts.at(cf.ansiFont);
    return QString();
}

QColor Importer::colorOf(const ColorIndex &c, const TextCtx &ctx, const QColor &fallback) const
{
    if (c.index == 0xFE) return QColor(c.r, c.g, c.b);
    if (c.index < 8 && ctx.scheme) return ctx.scheme[c.index];
    return fallback;
}

// What a paragraph of a text of this kind and level says, with the defaults of the presentation and its
// masters beneath: the presentation's own, then the main master's, then a title master's, each from the style
// the kind builds on up to its own, and each level taking what the levels before it say.
StyleLevel Importer::resolve(int type, int level, const TextCtx &ctx) const
{
    if (type < 0 || type >= TxTypes) type = TxOther;
    StyleLevel out;
    out.pf = m_defaultPf;
    out.cf = m_defaultCf;
    int chain[4];
    int n = 0;
    for (int t = type; t >= 0 && n < 4; t = styleBase(t)) chain[n++] = t;
    const StyleSet *sets[3] = {&m_styles, ctx.masters && ctx.main ? &ctx.main->styles : nullptr, ctx.masters && ctx.title ? &ctx.title->styles : nullptr};
    for (const StyleSet *set : sets) {
        if (!set) continue;
        for (int i = n - 1; i >= 0; --i) {
            const TypeStyle &ts = (*set)[chain[i]];
            for (int k = 0; k <= level && k < ts.levels; ++k) {
                mergePf(out.pf, ts.level[k].pf);
                mergeCf(out.cf, ts.level[k].cf);
            }
        }
    }
    return out;
}

QTextCharFormat Importer::charFormat(const Cf &cf, const TextCtx &ctx) const
{
    QTextCharFormat f;
    const QString font = fontName(cf);
    if (!font.isEmpty()) f.setFontFamilies(QStringList{font});
    f.setFontPointSize(cf.size ? cf.size : 18);
    if (cf.style & StyleBold) f.setFontWeight(QFont::Bold);
    if (cf.style & StyleItalic) f.setFontItalic(true);
    if (cf.style & StyleUnderline) f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    if (cf.style & StyleShadow) {
        // The shadow of an old file's words, as the presentation program draws it (fitted to its pictures of the old presentations among
        // the reference files): lighter and nearer than the one it gives the button.
        static const QString kShadow = [] {
            TextEffects fx;
            fx.outer.on = true;
            fx.outer.color = QColor(0, 0, 0, 76);   // 30 percent
            fx.outer.blur = 2;
            fx.outer.distance = 1.25;
            fx.outer.angle = 45;
            fx.outer.align = QStringLiteral("tl");
            return fx.toJson();
        }();
        f.setProperty(tp::TextFx, kShadow);
    }
    if (cf.style & StyleEmboss) f.setProperty(tp::Emboss, true);
    if (cf.position > 0) f.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    else if (cf.position < 0) f.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    const QColor color = colorOf(cf.color, ctx, ctx.scheme ? ctx.scheme[1] : QColor(Qt::black));
    f.setForeground(color);
    f.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
    return f;
}

namespace {

// A numbering scheme (TextAutoNumberScheme): the list's counting, the marks around the number, and tp::NumberFormat (0 when it is none of the
// seven that name; a scheme this reading doesn't know is "1.").
struct NumberScheme {
    QTextListFormat::Style style = QTextListFormat::ListDecimal;
    QString prefix, suffix;
    int format = 1;
};
NumberScheme numberSchemeOf(int scheme)
{
    struct Row { QTextListFormat::Style style; const char *prefix, *suffix; int format; };
    static const Row rows[] = {
        {QTextListFormat::ListLowerAlpha, "", ".", 2}, {QTextListFormat::ListUpperAlpha, "", ".", 3}, {QTextListFormat::ListDecimal, "", ")", 6},
        {QTextListFormat::ListDecimal, "", ".", 1}, {QTextListFormat::ListLowerRoman, "(", ")", 0}, {QTextListFormat::ListLowerRoman, "", ")", 0},
        {QTextListFormat::ListLowerRoman, "", ".", 4}, {QTextListFormat::ListUpperRoman, "", ".", 5}, {QTextListFormat::ListLowerAlpha, "(", ")", 0},
        {QTextListFormat::ListLowerAlpha, "", ")", 0}, {QTextListFormat::ListUpperAlpha, "(", ")", 0}, {QTextListFormat::ListUpperAlpha, "", ")", 0},
        {QTextListFormat::ListDecimal, "(", ")", 7}, {QTextListFormat::ListDecimal, "", "", 0}, {QTextListFormat::ListUpperRoman, "(", ")", 0},
        {QTextListFormat::ListUpperRoman, "", ")", 0},
    };
    NumberScheme s;
    if (scheme < 0 || scheme >= int(sizeof(rows) / sizeof(rows[0]))) {
        s.suffix = QStringLiteral(".");
        return s;
    }
    const Row &r = rows[scheme];
    s.style = r.style;
    s.prefix = QString::fromLatin1(r.prefix);
    s.suffix = QString::fromLatin1(r.suffix);
    s.format = r.format;
    return s;
}

// The characters of a run as they show: a vertical tab or a line feed breaks the line, and the other control characters are not shown.
QString displayText(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar ch : s) {
        const ushort u = ch.unicode();
        if (u == 0x0B || u == 0x0A || u == 0x2029) out += QChar(QChar::LineSeparator);
        else if (u == 0x09 || (u >= 0x20 && u != 0xFFFC)) out += ch;
    }
    return out;
}

} // namespace

// The story of a text: its paragraphs with their levels, bullets, indents, and spacing, and its runs with their
// fonts, sizes, and colors, the way the text engine reads them (lists with their level and mark, colors as
// color references, fields as the characters they stand on).
QString Importer::buildStory(const TextGroup &g, int type, const TextCtx &ctx)
{
    const QString id = m_doc.createStory();
    QTextDocument *doc = m_doc.storyDoc(id);
    if (!doc) return id;
    // All the text of a presentation is read up to a limit, so no file can be made to take forever.
    QString text = g.text;
    if (m_chars + text.size() > kMaxTotalChars) {
        text.truncate(qsizetype(std::max<qint64>(0, kMaxTotalChars - m_chars)));
        if (!m_charLimitSaid) note(say("The presentation holds more text than can be read; the rest is left out."));
        m_charLimitSaid = true;
    }
    m_chars += text.size();
    const qint64 n = text.size();
    // The text is put in at once, with no history kept of it.
    const bool undo = doc->isUndoRedoEnabled();
    doc->setUndoRedoEnabled(false);
    const RunIndex<PfRun> pfIndex(g.pf);
    const RunIndex<CfRun> cfIndex(g.cf);
    const RunIndex<LangRun> langIndex(g.lang);
    const RunIndex<LevelRun> levelIndex(g.levels);
    QHash<qint64, const FieldMark *> fields;
    QVector<qint64> fieldPositions;
    for (const FieldMark &f : g.fields) {
        fields.insert(f.position, &f);
        fieldPositions << f.position;
    }
    std::sort(fieldPositions.begin(), fieldPositions.end());
    QHash<int, StyleLevel> resolved;
    QVector<LinkMark> links = g.links;
    std::sort(links.begin(), links.end(), [](const LinkMark &a, const LinkMark &b) { return a.start < b.start; });
    int nextLink = 0;

    QTextCursor cur(doc);
    cur.beginEditBlock();
    bool firstBlock = true;
    QTextList *list = nullptr;
    QString listKey;
    int inList = 0, counted = 0;   // the paragraphs in the list now, and the numbered ones counted in the run of lists it goes on from
    qint64 start = 0;
    int paragraphs = 0;
    for (;;) {
        if (++paragraphs > kMaxParagraphs || ++m_paragraphs > kMaxTotalParagraphs) {
            if (!m_paragraphLimitSaid) note(say("A text has more paragraphs than can be read; the rest is left out."));
            m_paragraphLimitSaid = true;
            break;
        }
        qint64 end = text.indexOf(QLatin1Char('\r'), qsizetype(start));
        const bool hasEnd = end >= 0;
        if (!hasEnd) end = n;

        // The paragraph's style: its kind and level on the masters, the text box's ruler, its own settings.
        int level = 0;
        const PfRun *pfRun = nullptr;
        int pfRunNumber = -1;
        if (const int i = pfIndex.at(start); i >= 0) {
            pfRun = &g.pf.at(i);
            pfRunNumber = i;
            level = pfRun->level;
        } else if (const int i = levelIndex.at(start); i >= 0) {
            level = g.levels.at(i).level;
        }
        level = std::clamp(level, 0, 4);
        if (!resolved.contains(level)) resolved.insert(level, resolve(type, level, ctx));
        const StyleLevel &base = resolved[level];
        Pf pf = base.pf;
        if (g.ruler.has & (8u << level)) {
            pf.left = g.ruler.left[level];
            pf.has |= PfLeft;
        }
        if (g.ruler.has & (0x100u << level)) {
            pf.indent = g.ruler.indent[level];
            pf.has |= PfIndent;
        }
        if (g.ruler.has & 4) {
            pf.tabs = g.ruler.tabs;
            pf.has |= PfTabs;
        }
        if (pfRun) mergePf(pf, pfRun->pf);

        // Its runs.
        Cf first = base.cf;
        if (const int i = cfIndex.at(start); i >= 0) mergeCf(first, g.cf.at(i).cf);
        struct Piece {
            QString text;
            QTextCharFormat format;
        };
        QVector<Piece> pieces;
        int symbolPieces = 0;   // the pieces the split of text in symbol fonts has added to this paragraph
        qint64 pos = start;
        while (pos < end) {
            qint64 next = end;
            // Past the limit on pieces, what is left of the paragraph is one piece in the paragraph's own style.
            const bool plain = m_pieces >= kMaxTotalPieces;
            ++m_pieces;
            const int ci = plain ? -1 : cfIndex.at(pos), li = plain ? -1 : langIndex.at(pos);
            if (ci >= 0) next = std::min(next, cfIndex.endOf(ci));
            if (li >= 0) next = std::min(next, langIndex.endOf(li));
            const FieldMark *field = plain ? nullptr : fields.value(pos, nullptr);
            if (field) {
                next = pos + 1;
            } else if (const auto it = std::upper_bound(fieldPositions.cbegin(), fieldPositions.cend(), pos); !plain && it != fieldPositions.cend()) {
                next = std::min(next, *it);
            }
            const LinkMark *link = nullptr;
            while (!plain && nextLink < links.size() && links[nextLink].end <= pos) ++nextLink;
            if (!plain && nextLink < links.size()) {
                const LinkMark &l = links[nextLink];
                if (l.start <= pos) {
                    link = &l;
                    next = std::min(next, l.end);
                } else {
                    next = std::min(next, l.start);
                }
            }
            Cf cf = base.cf;
            if (ci >= 0) mergeCf(cf, g.cf.at(ci).cf);
            QTextCharFormat fmt = charFormat(cf, ctx);
            if (li >= 0 && g.lang.at(li).lang)
                if (const QString lang = languageName(g.lang.at(li).lang); !lang.isEmpty()) fmt.setProperty(tp::Language, lang);
            QString piece;
            if (field) {
                // The slide number and the date are fields; the header and the footer are the texts set apart for them, which a slide shows as its own.
                QString code;
                if (field->kind == FieldMark::SlideNumber) code = QStringLiteral("page");
                else if (field->kind == FieldMark::Date) code = QStringLiteral("date");
                else if (field->kind == FieldMark::DateFormat) code = QStringLiteral("datetime:") + dateFormat(field->format);
                if (!code.isEmpty()) {
                    fmt.setProperty(tp::Field, code);
                    piece = QString(QChar(QChar::ObjectReplacementCharacter));
                }
            } else {
                piece = displayText(text.mid(qsizetype(pos), qsizetype(next - pos)));
            }
            if (link && !piece.isEmpty()) {
                const QColor color = ctx.scheme ? ctx.scheme[6] : QColor(Qt::blue);
                fmt.setAnchor(true);
                fmt.setAnchorHref(link->url);
                fmt.setForeground(color);
                fmt.setProperty(tp::ColorRefP, ColorRef::rgb(color).toString());
                fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            }
            // Text in a symbol font is shown as the font's own pictures: a character of the font the run names for symbols (kept at
            // 0xF020 to 0xF0FF), or of a run set in the font itself (the plain code: in 8-bit text the byte itself, not the Windows-1252
            // character it reads as), that has a look-alike in Unicode (a Greek letter, an arrow) is that, in the text's own font; a private
            // one that has none stays in a piece of the font's own, where the font is. A paragraph that would be cut into more than
            // kMaxSymbolPieces pieces keeps the rest of its text as it is.
            const QString symbolFamily = (cf.has & CfSymbol) && cf.symbolFont < m_fonts.size() ? m_fonts.at(cf.symbolFont) : QString();
            const QString runFont = fontName(cf);
            const bool setInSymbols = isSymbolFont(runFont);
            if (!piece.isEmpty() && !field && (!symbolFamily.isEmpty() || setInSymbols)) {
                QTextCharFormat texty = fmt;
                if (setInSymbols) texty.clearProperty(QTextFormat::FontFamilies);
                // The characters in a row that are shown alike make one piece: as they are, in the text's own font, or in the symbol font's.
                enum Look { Asis, Texty, Own };
                QString held, heldFamily;
                Look heldLook = Asis;
                auto flush = [&] {
                    if (held.isEmpty()) return;
                    QTextCharFormat format = heldLook == Texty ? texty : fmt;
                    if (heldLook == Own) format.setFontFamilies(QStringList{heldFamily});
                    pieces.push_back({held, format});
                    held.clear();
                    ++m_pieces;
                    ++symbolPieces;
                };
                auto put = [&](const QString &text, Look look, const QString &family = QString()) {
                    if (look != heldLook || family != heldFamily) flush();
                    held += text;
                    heldLook = look;
                    heldFamily = family;
                };
                for (const QChar ch : std::as_const(piece)) {
                    ushort u = ch.unicode();
                    const bool inPrivateUse = u >= 0xF020 && u <= 0xF0FF;
                    if (g.bytes && setInSymbols && !inPrivateUse) u = rawByteOf(u);
                    // (Past the limits on pieces, the rest is as it stands.)
                    const QString family = m_pieces >= kMaxTotalPieces || symbolPieces >= kMaxSymbolPieces ? QString()
                                           : inPrivateUse && !symbolFamily.isEmpty() ? symbolFamily
                                           : setInSymbols && (inPrivateUse || (u >= 0x20 && u <= 0xFF)) ? runFont : QString();
                    const QString mapped = family.isEmpty() ? QString() : symbolTextToUnicode(family, u);
                    if (!mapped.isEmpty()) put(mapped, Texty);
                    else if (inPrivateUse && !family.isEmpty()) put(QString(ch), Own, family);
                    else put(QString(ch), Asis);
                }
                flush();
            } else if (!piece.isEmpty()) {
                pieces.push_back({piece, fmt});
            }
            pos = next;
        }

        // The paragraph.
        const double size = first.size ? first.size : 18;
        const QTextCharFormat blockFormat = charFormat(first, ctx);
        QTextBlockFormat bf;
        switch (pf.align) {
        case 1: bf.setAlignment(Qt::AlignHCenter); break;
        case 2: bf.setAlignment(Qt::AlignRight); break;
        case 3: case 6: bf.setAlignment(Qt::AlignJustify); break;
        case 4: case 5: bf.setAlignment(Qt::AlignJustify); bf.setProperty(tp::Distribute, true); break;
        default: bf.setAlignment(Qt::AlignLeft); break;
        }
        const double leftMargin = pf.left * kPoints, indent = pf.indent * kPoints;
        bf.setLeftMargin(leftMargin);
        bf.setTextIndent(indent - leftMargin);
        // Spacing is a share of a line (of the paragraph's text size) when positive, master units when negative.
        auto spacing = [&](int v) { return v > 0 ? v / 100.0 * size * 1.2 : v < 0 ? -v * kPoints : 0.0; };
        bf.setTopMargin(spacing(pf.before));
        bf.setBottomMargin(spacing(pf.after));
        // A share of a single line (1.2 times the text size: the text engine's slide lines), or master units.
        if (pf.line > 0 && pf.line != 100) bf.setLineHeight(pf.line, QTextBlockFormat::ProportionalHeight);
        else if (pf.line < 0) bf.setLineHeight(-pf.line * kPoints, QTextBlockFormat::FixedHeight);
        if (!pf.tabs.isEmpty()) {
            QList<QTextOption::Tab> tabs;
            for (const Tab &t : std::as_const(pf.tabs)) {
                QTextOption::Tab tab;
                tab.position = t.position * kPoints;
                tab.type = t.type == 1 ? QTextOption::CenterTab : t.type == 2 ? QTextOption::RightTab : t.type == 3 ? QTextOption::DelimiterTab : QTextOption::LeftTab;
                tabs << tab;
            }
            bf.setTabPositions(tabs);
        }
        if (firstBlock) {
            cur.setBlockFormat(bf);
            cur.setBlockCharFormat(blockFormat);
        } else {
            cur.insertBlock(bf, blockFormat);
        }
        firstBlock = false;

        // A bullet: a list of its level, the mark and its own font, size, and color when it has them, the text coming at the left margin.
        if (pf.flags & BulletOn) {
            const quint16 ch = pf.bulletChar ? pf.bulletChar : 0x2022;
            const QString font = (pf.flags & BulletOwnFont) && pf.bulletFont < m_fonts.size() ? m_fonts.at(pf.bulletFont) : QString();
            double markSize = 0;
            if (pf.flags & BulletOwnSize) markSize = pf.bulletSize > 0 ? size * pf.bulletSize / 100.0 : -pf.bulletSize;
            if (std::abs(markSize - size) < 0.01) markSize = 0;
            const QString color = (pf.flags & BulletOwnColor) ? ColorRef::rgb(colorOf(pf.bulletColor, ctx, QColor(Qt::black))).toString() : QString();
            // Paragraphs the text's extension numbers count instead of showing the mark.
            const Pp9 *numbered = pfRunNumber >= 0 && pfRunNumber < g.pp9.size() && g.pp9.at(pfRunNumber).autoNumber ? &g.pp9.at(pfRunNumber) : nullptr;
            const QString key = QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8").arg(level).arg(numbered ? 0 : ch).arg(numbered ? QString() : font).arg(markSize).arg(color).arg(leftMargin)
                                    .arg(numbered ? numbered->scheme : -1).arg(numbered ? numbered->start : 0);
            // (A list takes its items one by one at a cost that grows with its length, so a long run of bullets is cut into lists of at most 400.)
            // An empty paragraph is no item of a numbered list: it has no number, and the numbering goes on after it.
            if (numbered && list && key == listKey && start == end) {
                // (left out of the list)
            } else if (list && key == listKey && inList < 400) {
                list->add(cur.block());
                ++inList;
                if (numbered) ++counted;
            } else {
                // (A run of numbered paragraphs cut for its length goes on counting where the list before stopped.)
                const bool goesOn = numbered && list && key == listKey;
                if (!goesOn) counted = 0;
                inList = 1;
                QTextListFormat lf;
                lf.setStyle(QTextListFormat::ListDisc);
                lf.setIndent(0);
                lf.setProperty(tp::ListLevel, level);
                if (numbered) {
                    const NumberScheme scheme = numberSchemeOf(numbered->scheme);
                    lf.setStyle(scheme.style);
                    lf.setNumberPrefix(scheme.prefix);
                    lf.setNumberSuffix(scheme.suffix);
                    if (const int first = numbered->start + (goesOn ? counted : 0); first > 1) lf.setStart(std::min(first, 32767));
                    ++counted;
                    if (scheme.format) lf.setProperty(tp::NumberFormat, scheme.format);
                } else {
                    if (ch != 0x2022 || !font.isEmpty()) lf.setProperty(tp::BulletChar, QString(QChar(ch)));
                    if (!font.isEmpty()) lf.setProperty(tp::BulletFont, font);
                }
                if (!color.isEmpty()) lf.setProperty(tp::BulletColor, color);
                if (markSize > 0) lf.setProperty(tp::BulletSize, markSize);
                lf.setProperty(tp::BulletTab, leftMargin);
                list = cur.createList(lf);
                listKey = key;
            }
        } else {
            list = nullptr;
        }

        for (const Piece &p : std::as_const(pieces)) cur.insertText(p.text, p.format);
        if (!hasEnd) break;
        start = end + 1;
    }
    cur.endEditBlock();
    doc->setUndoRedoEnabled(undo);
    return id;
}

// ---- The drawing reader's functions ----

office::OdrawContext Importer::drawingContext()
{
    office::OdrawContext ctx;
    ctx.budget = &m_shapes;
    ctx.richLook = [this](Item &item, const QByteArray &package) { takeLookFromWritersXml(item, package); };
    ctx.background = [this](const QRectF &shape) { return m_state ? backgroundAt(m_state->background, shape) : Fill(); };
    ctx.inherits = [this](const QByteArray &clientData) {
        // A slide's placeholder takes after its master's shape of its kind.
        if (!m_state || m_state->master || !m_state->source) return QByteArray();
        const QString type = placeholderOf(clientData);
        return type.isEmpty() ? QByteArray() : masterShapeProps(*m_state->source, type);
    };
    ctx.presentation = true;
    ctx.clientAnchor = [](const QByteArray &anchor) { return anchorRect(anchor); };
    ctx.schemeColor = [this](int index) { return m_state && m_state->scheme && index >= 0 && index < 8 ? m_state->scheme[index] : QColor(); };
    ctx.picture = [this](int index) { return pictureId(index); };
    ctx.adjusted = [this](const QString &id, double brightness, double contrast) { return adjustedPicture(id, brightness, contrast); };
    ctx.patterned = [this](const QString &id, const QColor &fillColor, const QColor &backColor) { return patternedPicture(id, fillColor, backColor); };
    ctx.duotoned = [this](const QString &id, const QColor &dark, const QColor &light) { return duotonedPicture(id, dark, light); };
    ctx.client = [this](Item &item, const QByteArray &textbox, const QByteArray &clientData) {
        if (m_state) clientShape(item, textbox, clientData);
    };
    return ctx;
}

namespace {

// A text box that is turned over reads upright: the presentation program turns a shape's text with the shape, a half turn more when it is flipped
// top to bottom, and never mirrors it.
void uprightText(const ItemList &items)
{
    for (const ItemPtr &it : items) {
        if (it->type() == ItemType::Group) {
            uprightText(static_cast<const GroupItem &>(*it).children);
        } else if (it->type() == ItemType::Text && (it->flipH || it->flipV)) {
            // (The item flips and then turns, so the text goes with that turn as it is, whichever way the shape flips.)
            double angle = it->rotation;
            if (it->flipV) angle += 180;
            it->rotation = std::fmod(std::fmod(angle, 360.0) + 360.0, 360.0);
            it->flipH = it->flipV = false;
        }
    }
}

// Slide objects don't push text aside: the presentation program lets text and objects overlap.
void leaveTextAlone(const ItemList &items)
{
    for (const ItemPtr &it : items) {
        it->wrap.mode = Wrap::None;
        if (it->type() == ItemType::Group) leaveTextAlone(static_cast<const GroupItem &>(*it).children);
    }
}

} // namespace

ItemList Importer::readShapes(const QByteArray &drawing, PageState &st)
{
    if (drawing.isEmpty()) return {};
    m_state = &st;
    ItemList items = office::readOdrawShapes(drawing, drawingContext());
    m_state = nullptr;
    noteShapeLimit();
    leaveTextAlone(items);
    uprightText(items);
    return items;
}

// What a shape's client records say: which placeholder it is, the link it follows, and its text (its own, or
// one the slide's list keeps for it).
void Importer::clientShape(Item &item, const QByteArray &textbox, const QByteArray &clientData)
{
    PageState &st = *m_state;
    QString link;
    const QString type = placeholderOf(clientData);
    if (!clientData.isEmpty()) {
        Records rs(clientData);
        QVector<Rec> kids = rs.in(0, clientData.size());
        if (kids.size() == 1 && kids.first().type == RT_ClientData) kids = rs.in(kids.first());
        for (const Rec &r : kids) {
            if (r.type == RT_InteractiveInfo && r.inst == 0 && link.isEmpty()) {
                for (const Rec &k : rs.in(r)) {
                    if (k.type != RT_InteractiveInfoAtom || k.len < 9) continue;
                    Cursor c(clientData, k.off, k.end());
                    c.u32();
                    const quint32 id = c.u32();
                    if (c.u8() == 4) link = m_links.value(id);
                }
            }
        }
    }
    TextGroup g;
    bool hasBox = false;
    if (!textbox.isEmpty()) {
        Records rs(textbox);
        QVector<Rec> kids = rs.in(0, textbox.size());
        if (kids.size() == 1 && kids.first().type == RT_ClientTextbox) kids = rs.in(kids.first());
        g = readTextGroup(rs, kids);
        hasBox = true;
    }
    if (!type.isEmpty() && type != QLatin1String("sldImg")) {
        item.role = placeholderRole(type, isContentType(type) ? ++st.contents : 0);
        st.types << type;
    }
    if (!link.isEmpty()) item.hyperlink = link;

    const TextGroup *use = nullptr;
    TextGroup ruled;
    if (g.outline >= 0) {
        if (st.groups && g.outline < st.groups->size()) {
            use = &st.groups->at(int(g.outline));
            st.used[int(g.outline)] = true;
            // The ruler the shape keeps beside its reference to the slide's text is that text's (its margins, indents, and tabs).
            if (g.ruler.has && !use->ruler.has) {
                ruled = *use;
                ruled.ruler = g.ruler;
                use = &ruled;
            }
        }
    } else if (g.hasText) {
        use = &g;
    }
    if (!holdsText(item)) return;
    // The old format's text is not hyphenated.
    if (item.type() == ItemType::Text) static_cast<TextItem &>(item).hyphenate = false;
    QString &story = storyIdOf(item);
    if (!story.isEmpty() && (use || hasBox || !type.isEmpty())) {
        m_doc.removeStory(story);
        story.clear();
    }
    if (use && use->hasText) {
        const int textType = use->type >= 0 ? use->type : g.type >= 0 ? g.type : textTypeOf(type);
        TextCtx ctx = st.text;
        if (type.isEmpty() && textType == TxOther) ctx.masters = false;
        story = buildStory(*use, textType, ctx);
    }
    else if (hasBox || !type.isEmpty()) story = m_doc.createStory();
}

// ---- The whole ----

bool Importer::run(QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    QString why;
    if (!openStreams(&why) || !readEdits(&why)) return fail(why);
    Rec docRec;
    if (!containerAt(m_documentId, RT_Document, &docRec)) return fail(say("The presentation is cut short or damaged: its document can't be found."));

    m_doc.masters.clear();
    m_doc.setup.size = m_doc.setup.sheet = QSizeF(kSlideWidth, kSlideHeight);
    m_doc.setup.sizeName = QStringLiteral("Widescreen");
    m_doc.setup.margins = QMarginsF(0, 0, 0, 0);
    const QVector<Rec> kids = m_ppt.in(docRec);
    if (const Rec *r = find(kids, RT_DocumentAtom)) readDocumentAtom(*r);
    if (const Rec *r = find(kids, RT_Environment)) readEnvironment(*r);
    if (const Rec *r = find(kids, RT_DrawingGroup)) readDrawingGroup(*r);
    if (const Rec *r = find(kids, RT_ExObjList)) readHyperlinks(*r);
    readTextExtensions(kids);
    for (const Rec &r : kids) {
        if (r.type != RT_SlideListWithText) continue;
        QVector<SlideEntry> list = readSlideList(r);
        QVector<SlideEntry> &into = r.inst == 0 ? m_slides : r.inst == 1 ? m_masterList : m_notesList;
        if (r.inst <= 2) into += list;
    }
    // (A scan of the list for each slide would take as long as the slides and the list are long, multiplied.)
    for (int i = 0; i < m_notesList.size(); ++i)
        if (!m_notesBySlideId.contains(m_notesList.at(i).slideId)) m_notesBySlideId.insert(m_notesList.at(i).slideId, i);

    readMasters();
    if (m_masters.empty()) {
        // Slides can still be shown on a master of the plainest kind.
        note(say("The presentation has no slide master that can be read, so a plain one stands in."));
        auto m = std::make_unique<MasterInfo>();
        for (int i = 0; i < 8; ++i) m->scheme[i] = QColor(kDefaultScheme[i]);
        m->main = m.get();
        m->background = Fill::solid(ColorRef::rgb(m->scheme[0]));
        m_masters.push_back(std::move(m));
        makeMasterPage(*m_masters.front(), 1);
    }

    int number = 0;
    for (const SlideEntry &e : std::as_const(m_slides)) {
        if (++number > kMaxSlides) {
            note(say("Only the first %1 slides are read.").arg(kMaxSlides));
            break;
        }
        readSlide(number, e);
    }
    assembleMasters();
    // (The record of the document that is for slides: the notes have their own.)
    FooterSpec documentFooters;
    for (const Rec &r : kids)
        if (r.type == RT_HeadersFooters && r.inst == 3) {
            documentFooters = readFooterSpec(r);
            break;
        }
    showFooters(documentFooters);

    // The theme: the first master's colors (the scheme's text and background are the theme's dark 1 and light 1, its
    // title text and shadow dark 2 and light 2, its fills and accent the first two accents, and its two hyperlink
    // colors the theme's), and the fonts its title and body text use.
    const MasterInfo *first = nullptr;
    for (const auto &m : m_masters)
        if (!m->title) {
            first = m.get();
            break;
        }
    DeckTheme theme = defaultTheme();
    theme.name = first && !first->name.isEmpty() ? first->name : say("Theme");
    const QColor *s = first->scheme;
    const QColor white(Qt::white), black(Qt::black);
    const QColor colors[12] = {s[1], s[0], s[3], s[2], s[4], s[5], mix(s[4], white, 0.35), mix(s[5], white, 0.35), mix(s[4], black, 0.3), mix(s[5], black, 0.3), s[6], s[7]};
    for (int i = 0; i < 12; ++i) theme.colors[i] = colors[i];
    const TextCtx ctx{first, nullptr, first->scheme};
    if (const QString heading = fontName(resolve(TxTitle, 0, ctx).cf); !heading.isEmpty()) theme.headingFont = heading;
    if (const QString body = fontName(resolve(TxBody, 0, ctx).cf); !body.isEmpty()) theme.bodyFont = body;
    setTheme(m_doc, theme);
    m_doc.fonts = {theme.name, theme.headingFont, theme.bodyFont};
    // Tabs go to the next multiple of the style's default tab distance (an inch when the styles say nothing).
    const quint16 tab = resolve(TxBody, 0, ctx).pf.defaultTab;
    m_doc.defaultTabStop = tab >= 36 ? std::min(tab * kPoints, 288.0) : 72.0;
    m_doc.appData[QStringLiteral("footers")] = m_footers;
    return true;
}

} // namespace

std::unique_ptr<Document> importBinaryPresentation(const QByteArray &bytes, QString *error)
{
    auto doc = std::make_unique<Document>();
    doc->slideText = true;
    doc->slidePictures = true;
    Importer importer(bytes, *doc);
    QString why;
    bool ok = false;
    try {
        ok = importer.run(&why);
        if (ok && !ensureASlide(*doc)) {
            ok = false;
            why = QCoreApplication::translate("PptImport", "The presentation has no slides.");
        }
    } catch (const std::bad_alloc &) {
        why = QCoreApplication::translate("PptImport", "The presentation is too big to open.");
    }
    if (!ok) {
        if (error) *error = why;
        return nullptr;
    }
    doc->fromFile = true;   // what the file names outside itself is read only from beside it
    if (error) *error = (QStringList{importer.notesText()} + leftOutNotes(doc->imagesLeftOut, 0)).join(QLatin1Char('\n')).trimmed();
    return doc;
}

} // namespace jp::deck
