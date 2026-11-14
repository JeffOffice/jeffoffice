#include "rtf/rtfmodel.h"

#include "doc/docprops.h"
#include "office/package.h"
#include "office/theme.h"
#include "sheet/xlscodepage.h"

#include <QCoreApplication>
#include <QSet>
#include <QSize>
#include <algorithm>
#include <array>
#include <memory>
#include <vector>

namespace jp::office::rtf {

namespace {

using doc::xmlEscape;

constexpr uint32_t H(const char *s, uint32_t h = 2166136261u) { return *s ? H(s + 1, (h ^ uint8_t(*s)) * 16777619u) : h; }
uint32_t hashOf(const char *s, int n)
{
    uint32_t h = 2166136261u;
    for (int i = 0; i < n; ++i) h = (h ^ uint8_t(s[i])) * 16777619u;
    return h;
}

const char kNamespaces[] =
    "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
    "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
    "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\"";
const char kXmlHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
const char kRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";
const char kWml[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.";

QByteArray num(qint64 v) { return QByteArray::number(v); }

const char *const kKnownWords[] = {
#include "rtf/rtfwords.inc"
};


enum class Dest {
    Normal, Skip, FontTbl, ColorTbl, StyleSheet, StyleDef, ListTable, ListDef, ListLevel, LevelText, ListOverrideTable, ListOverride, LfoLevel,
    RevTbl, OldCprops, OldPprops, FldInst, Field, BkmkStart, BkmkEnd, Pict, Header, Footnote, NestProps, Pn, PnText, ShapeProp, Shape, ShapeText, SpName, SpValue
};

// Which destinations the children of a group carry on (the rest start as plain text).
bool inherits(Dest d)
{
    switch (d) {
    case Dest::Skip: case Dest::FontTbl: case Dest::ColorTbl: case Dest::FldInst: case Dest::BkmkStart: case Dest::BkmkEnd: case Dest::Pict: case Dest::ShapeProp:
    case Dest::LevelText: case Dest::ListTable: case Dest::ListDef: case Dest::ListLevel: case Dest::ListOverrideTable: case Dest::ListOverride: case Dest::LfoLevel:
    case Dest::NestProps: case Dest::Pn: case Dest::StyleDef: case Dest::OldCprops: case Dest::OldPprops: case Dest::RevTbl:
        return true;
    default:
        return false;
    }
}

struct State {
    Chr chr;
    Par par;
    Dest dest = Dest::Normal;
    bool owns = false;
    int bsel = -1, btarget = 0;
    int tabType = 0, tabLead = 0;
    int uc = 1;
    bool shp = false, shapeText = false, shapePushed = false;
};

struct Picture {
    QByteArray data;
    int format = 0;                 // 1 png, 2 jpeg, 3 emf, 4 wmf, -1 one that can't be shown
    int w = 0, h = 0, wGoal = 0, hGoal = 0, scaleX = 100, scaleY = 100, cropL = 0, cropT = 0, cropR = 0, cropB = 0;
    bool tooBig = false;
};

struct Story {
    QString part;
    bool note = false, endnote = false;
    int slot = -1;
    QByteArray body;
    QByteArray runs;
    QString text;
    Chr textChr;
    struct Held {
        bool valid = false, sect = false;
        QByteArray ppr, runs;
    } held;
    struct RowRec {
        RowDef def;
        QVector<QByteArray> cells;
    };
    struct Level {
        QByteArray cell;
        QVector<QByteArray> cells;
        QVector<RowRec> rows;
    };
    QVector<Level> levels = QVector<Level>(1);
    RowDef def[kMaxTableDepth + 2];
    struct FieldRec { bool real = false, begun = false; };
    QVector<FieldRec> fields;
    bool hasContent() const { return !runs.isEmpty() || !text.isEmpty(); }
};

bool isDoubleByte(int cp) { return cp == 932 || cp == 936 || cp == 949 || cp == 950 || cp == 1361; }
bool isLeadByte(int cp, uchar b)
{
    switch (cp) {
    case 932: return (b >= 0x81 && b <= 0x9F) || (b >= 0xE0 && b <= 0xFC);
    case 936: case 949: case 950: return b >= 0x81 && b <= 0xFE;
    case 1361: return (b >= 0x84 && b <= 0xD3) || (b >= 0xD8 && b <= 0xDE) || (b >= 0xE0 && b <= 0xF9);
    default: return false;
    }
}

int codepageOfCharset(int cs)
{
    switch (cs) {
    case 238: return 1250;
    case 204: return 1251;
    case 161: return 1253;
    case 162: return 1254;
    case 177: return 1255;
    case 178: return 1256;
    case 186: return 1257;
    case 163: return 1258;
    case 222: return 874;
    case 128: return 932;
    case 134: return 936;
    case 129: return 949;
    case 136: return 950;
    case 130: return 1361;
    case 77: return 10000;
    case 255: return 437;
    case 2: return -2;   // symbol: the bytes stand for the characters of the private use area
    default: return 0;
    }
}

class Reader {
public:
    Reader(const QByteArray &bytes, wml::ReadNotes *notes) : m_d(bytes.constData()), m_n(bytes.size()), m_notes(notes) {}
    QByteArray run(QString *error);
    qint64 outputBytes() const { return m_outputBytes; }
    ReadWork work() const { return m_work; }

private:
    // ---- input
    const char *m_d;
    qint64 m_n, m_pos = 0;
    wml::ReadNotes *m_notes;
    QVector<State> m_stack;
    int m_overflow = 0;
    bool m_star = false;
    State m_lastClosed;
    bool m_started = false, m_done = false, m_stopped = false;
    qint64 m_chars = 0, m_outputBytes = 0;

    // ---- document
    Tables t;
    int m_docCp = 1252;
    Section m_docSec, m_sec;
    QString m_secRef[6];                   // header default, first, even; footer default, first, even
    std::vector<std::unique_ptr<Story>> m_stories;
    Story m_main;
    QSet<QString> m_leftKeys;
    PackageWriter pkg;
    int m_sections = 0, m_paragraphs = 0, m_tableCells = 0, m_headers = 0, m_pictures = 0, m_mediaCount = 0, m_drawingId = 0, m_bookmarkId = 0;
    qint64 m_pictureBytes = 0;
    QByteArray m_notesXml[2];
    int m_noteCount[2] = {0, 0};
    QHash<QByteArray, QString> m_mediaOf;
    QHash<QString, QString> m_relOfMedia;
    QHash<QString, int> m_bookmarks;
    QStringList m_authors;                 // the authors of tracked changes, in the order the file lists them
    QVector<Par> m_oldPars;                // the properties a paragraph had before a change of its format
    QVector<Chr> m_oldProps;               // the properties text had before a change of its format
    int m_revId = 0;
    QSet<QString> m_endedBookmarks;
    QByteArray m_pendingLead;
    QByteArray m_utf8;                     // the bytes so far of a character in UTF-8 (code page 65001), and how many more it needs
    int m_utf8Need = 0;
    int m_ucSkip = 0;
    int m_lastShppict = 0;
    QHash<int, std::array<QChar, 128>> m_pageTables;

    // ---- scratch for the destinations
    QString m_name;                        // the text of a name being read
    QByteArray m_instr;
    Font m_font;
    bool m_fontPending = false;
    int m_red = 0, m_green = 0, m_blue = 0;
    bool m_colorPending = false;
    StyleDef m_style;
    bool m_stylePending = false;
    ListDef m_list;
    ListLevel m_level;
    QVector<quint32> m_levelRaw;
    qint64 m_overrideList = 0;
    int m_overrideLs = 0;
    QVector<ListLevel> m_overrides;
    ListLevel m_lfo;
    struct PnScratch { bool bullet = false; int nfc = 0, start = 1, indent = 360, font = kUnset; QString before, after; bool any = false; } m_pn;
    bool m_pnAfter = false;
    int m_half = -1;
    Picture m_pict;
    int m_pictStory = 0;
    struct ShapeScratch {
        int posh = 0, posv = 0, type = 0, left = 0, top = 0, right = 0, bottom = 0, wrap = 2, bx = 2, by = 2, posrelh = kUnset, posrelv = kUnset, z = 0;
        int filled = kUnset, fillColor = 0, line = kUnset, lineColor = 0, lineWidth = kUnset, inset[4] = {kUnset, kUnset, kUnset, kUnset}, behind = 0;
    } m_shape;
    ReadWork m_work;
    bool m_shapeOpen = false, m_shapeHadText = false;
    // A shape in the text of another: the outer one's numbers are kept here while the inner is read (as deep as the text boxes go).
    struct SavedShape { ShapeScratch shape; bool open, hadText; };
    QVector<SavedShape> m_shapeStack;
    int m_shapes = 0;
    QString m_spName;
    CellDef m_pendCell;
    bool m_pendCellBlank = true;
    int m_pnStyleCounter = 0;
    QString m_pnKey;
    int m_pnLs = 0, m_pnParagraph = -10;

    Story &story() { return m_stories.empty() ? m_main : *m_stories.back(); }
    State &st() { return m_stack.back(); }

    void left(const QString &key, const QString &text)
    {
        if (m_leftKeys.contains(key)) return;
        m_leftKeys.insert(key);
        if (m_notes) {
            m_notes->notes.append(text);
            m_notes->leftOut = true;
        }
    }

    bool spend(qint64 bytes)
    {
        m_outputBytes += bytes;
        if (m_outputBytes > kMaxOutputBytes) {
            if (!m_stopped) {
                m_stopped = true;
                left(QStringLiteral("size"), QCoreApplication::translate("jp::office::rtf", "The document is larger than the limit, so the rest of it was left out."));
            }
            return false;
        }
        return true;
    }
    // One more character of text (a tab, a break, a field): counted against the limit on the text.
    bool room(int n = 1)
    {
        m_chars += n;
        if (m_chars > kMaxChars) {
            if (!m_stopped) {
                m_stopped = true;
                left(QStringLiteral("chars"), QCoreApplication::translate("jp::office::rtf", "The text is longer than the limit, so the rest of it was left out."));
            }
            return false;
        }
        return !m_stopped;
    }
    void addLevelRaw(quint32 v)
    {
        if (m_levelRaw.size() < 1000) m_levelRaw.append(v);
        m_work.maxLevelRaw = std::max<int>(m_work.maxLevelRaw, m_levelRaw.size());
    }

    // ---- the loop and the tokens
    void pushGroup();
    void popGroup();
    void control();
    void word(uint32_t h, const char *name, int len, bool has, int param);
    void onByte(uchar b);
    void onTextChars(const char *p, int n);
    void flushLead();
    QChar decodeHigh(int cp, uchar b);
    int codepageFor(int fontIndex) const;
    void routeText(const QString &s);

    // ---- paragraphs, runs, tables
    void flushText(Story &s);
    QByteArray revisionAttributes(int author, int date);
    QByteArray revisionWrapped(const Chr &c, QByteArray run);
    QByteArray markRevisionXml(const Chr &c);
    QByteArray paragraphChangeXml(const Par &p);
    void addText(Story &s, const QString &text, const Chr &c);
    void addRaw(Story &s, const QByteArray &inner, const Chr &c, bool withProperties = true);
    void addInline(Story &s, const QByteArray &xml);
    void addField(const QByteArray &instr, const QString &result);
    void endParagraph(const State &g, int level, bool force);
    QString styleIdOf(int number) const;
    void ensureLevel(Story &s, int level);
    void closeLevel(Story &s, int level);
    void closeTables(Story &s);
    void flushHeld(Story &s);
    QByteArray tableXml(const QVector<Story::RowRec> &rows);
    QByteArray finishStory(Story &s, const State &g);
    void cellEnd(const State &g, int level);
    void rowEnd(const State &g, int level);
    void sectionEnd(const State &g);
    RowDef &rowDefFor(const State &g);
    // The level a nested cell or row belongs to: the paragraph's own, between the second and the one past the deepest table open.
    int nestLevel(const State &g) { return std::clamp(g.par.itap, 2, std::max(2, int(story().levels.size()))); }
    Border *borderFor(State &g, int side);

    // ---- the things a group ends with
    void commitFont();
    void commitColor();
    void commitStyle();
    void finishListLevel(const State &g);
    void finishPict(const State &g);
    void finishNote(const State &g);
    void finishHeader(const State &g);
    void finishShapeText(const State &g);
    QByteArray shapeAttributes(const char *type, double least, bool filled = true, bool strokedByDefault = false);
    void finishShapeDrawing();
    void shapeWord(uint32_t h, int param);
    void finishPn(State &parent);
    QString addMedia(const QString &part, const QByteArray &bytes, const QString &ext);
    QByteArray drawingXml(const Picture &p, const QString &rid);
    QByteArray header(const char *tag, const QByteArray &body) { return QByteArray(kXmlHeader) + "<" + tag + " " + kNamespaces + ">" + body + "</" + tag + ">"; }
    QByteArray build(const State &g);

    void charWord(State &g, uint32_t h, bool has, int param, bool *handled);
    void parWord(State &g, uint32_t h, bool has, int param, bool *handled);
    void tableWord(State &g, uint32_t h, bool has, int param, bool *handled);
    void sectionWord(State &g, uint32_t h, bool has, int param, bool *handled);
    void destWord(State &g, uint32_t h, bool has, int param, bool *handled);
    void listWord(State &g, uint32_t h, bool has, int param);
    void pictWord(uint32_t h, int param);
};

// ---------------------------------------------------------------- text and bytes

int Reader::codepageFor(int fontIndex) const
{
    const int f = fontIndex == kUnset ? t.defaultFont : fontIndex;
    const auto it = t.fontAt.constFind(f);
    if (it == t.fontAt.constEnd()) return m_docCp;
    const int cp = t.fonts[it.value()].codepage;
    return cp == 0 ? m_docCp : cp;
}

QChar Reader::decodeHigh(int cp, uchar b)
{
    if (cp == -2) return QChar(0xF000 + b);
    auto it = m_pageTables.find(cp);
    if (it == m_pageTables.end()) {
        std::array<QChar, 128> tab;
        for (int i = 0; i < 128; ++i) {
            const char c = char(0x80 + i);
            const QString s = sheet::xlsBytesToText(cp, &c, 1);
            tab[i] = s.size() == 1 ? s[0] : QChar(0xFFFD);
        }
        it = m_pageTables.insert(cp, tab);
    }
    return it.value()[b - 0x80];
}

void Reader::flushLead()
{
    if (m_utf8Need > 0) {
        m_utf8.clear();
        m_utf8Need = 0;
        routeText(QString(QChar(0xFFFD)));
    }
    if (m_pendingLead.isEmpty()) return;
    const uchar b = uchar(m_pendingLead[0]);
    m_pendingLead.clear();
    routeText(QString(QChar(0xFFFD)));
    (void)b;
}

void Reader::routeText(const QString &s)
{
    State &g = st();
    switch (g.dest) {
    case Dest::Normal:
    case Dest::Header:
    case Dest::Footnote:
    case Dest::Field:
        if (m_stopped) return;
        m_lastShppict = 0;
        m_chars += s.size();
        if (m_chars > kMaxChars) {
            m_stopped = true;
            left(QStringLiteral("chars"), QCoreApplication::translate("jp::office::rtf", "The text is longer than the limit, so the rest of it was left out."));
            return;
        }
        addText(story(), s, g.chr);
        break;
    case Dest::FontTbl: case Dest::StyleDef: case Dest::BkmkStart: case Dest::BkmkEnd: case Dest::PnText: case Dest::SpName: case Dest::SpValue: case Dest::RevTbl:
        if (m_name.size() < 1000) m_name += s;
        break;
    case Dest::FldInst:
        if (m_instr.size() < kMaxFieldInstructionChars) m_instr += s.toUtf8();
        break;
    case Dest::LevelText:
        for (QChar c : s) addLevelRaw(c.unicode());
        break;
    default:
        break;
    }
}

void Reader::onByte(uchar b)
{
    State &g = st();
    if (m_ucSkip > 0) {
        --m_ucSkip;
        return;
    }
    if (g.dest == Dest::Skip) return;
    int cp = codepageFor(g.chr.font);
    if (g.dest == Dest::FontTbl) cp = m_font.codepage ? m_font.codepage : m_docCp;
    if (g.dest == Dest::LevelText && b < 0x20) {
        addLevelRaw(0x80000000u | b);
        return;
    }
    if (g.dest == Dest::LevelText && b >= 0x80) {
        addLevelRaw(0x80000000u | b);
        return;
    }
    if (cp == 65001) {
        // UTF-8: a character is a lead byte and the bytes that follow it.
        if (b < 0x80) {
            flushLead();
            routeText(QString(QChar(b)));
        } else if (b >= 0xC2 && b <= 0xF4) {
            flushLead();
            m_utf8 = QByteArray(1, char(b));
            m_utf8Need = b >= 0xF0 ? 3 : b >= 0xE0 ? 2 : 1;
        } else if (b < 0xC0 && m_utf8Need > 0) {
            m_utf8 += char(b);
            if (--m_utf8Need == 0) {
                const QString s = QString::fromUtf8(m_utf8);
                m_utf8.clear();
                routeText(s);
            }
        } else {
            flushLead();
            routeText(QString(QChar(0xFFFD)));
        }
        return;
    }
    if (!m_pendingLead.isEmpty()) {
        const char pair[2] = {m_pendingLead[0], char(b)};
        m_pendingLead.clear();
        routeText(sheet::xlsBytesToText(cp, pair, 2));
        return;
    }
    if (b < 0x80) {
        routeText(QString(QChar(b)));
        return;
    }
    if (isDoubleByte(cp) && isLeadByte(cp, b)) {
        m_pendingLead = QByteArray(1, char(b));
        return;
    }
    routeText(QString(decodeHigh(cp, b)));
}

void Reader::onTextChars(const char *p, int n)
{
    State &g = st();
    if (g.dest == Dest::Skip || g.dest == Dest::Pict) {
        if (g.dest == Dest::Pict && m_ucSkip == 0) {
            // hexadecimal digits
            for (int i = 0; i < n; ++i) {
                const char c = p[i];
                int v = (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
                if (v < 0) continue;
                if (m_half < 0) m_half = v;
                else {
                    if (m_pict.data.size() >= kMaxPictureBytes) m_pict.tooBig = true;
                    else m_pict.data.append(char(m_half * 16 + v));
                    m_half = -1;
                }
            }
        }
        return;
    }
    const bool plain = g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote;
    if (plain && g.dest != Dest::Skip) {
        // (A run of ASCII goes in as one piece.)
        int i = 0;
        while (i < n) {
            if (m_ucSkip > 0) {
                --m_ucSkip;
                ++i;
                continue;
            }
            int j = i;
            while (j < n && uchar(p[j]) < 0x80 && m_pendingLead.isEmpty() && m_utf8Need == 0) ++j;
            if (j > i) {
                routeText(QString::fromLatin1(p + i, j - i));
                i = j;
            } else {
                onByte(uchar(p[i]));
                ++i;
            }
        }
        return;
    }
    for (int i = 0; i < n; ++i) {
        const uchar c = uchar(p[i]);
        if (g.dest == Dest::RevTbl && c == ';') {
            if (m_ucSkip > 0) { --m_ucSkip; continue; }
            if (m_authors.size() < 1000) m_authors.append(m_name.trimmed().left(255));
            m_name.clear();
            continue;
        }
        if (g.dest == Dest::FontTbl || g.dest == Dest::StyleDef || g.dest == Dest::ColorTbl) {
            if (c == ';') {
                if (m_ucSkip > 0) { --m_ucSkip; continue; }
                if (g.dest == Dest::FontTbl) commitFont();
                else if (g.dest == Dest::ColorTbl) commitColor();
                else commitStyle();
                continue;
            }
        }
        if (c < 0x80 && m_ucSkip == 0) {
            if (g.dest == Dest::ColorTbl) continue;
            routeText(QString(QChar(c)));
        } else {
            onByte(c);
        }
    }
}

// ---------------------------------------------------------------- stories, paragraphs

void Reader::flushText(Story &s)
{
    if (s.text.isEmpty()) return;
    QByteArray rpr = runPropertiesXml(s.textChr, t);
    if (s.textChr.crAuth != kUnset && s.textChr.oldProps != kUnset && s.textChr.oldProps < m_oldProps.size())
        rpr += "<w:rPrChange" + revisionAttributes(s.textChr.crAuth, s.textChr.crDate) + "><w:rPr>" + runPropertiesXml(m_oldProps[s.textChr.oldProps], t) + "</w:rPr></w:rPrChange>";
    QByteArray x = "<w:r>";
    if (!rpr.isEmpty()) x += "<w:rPr>" + rpr + "</w:rPr>";
    x += "<w:t xml:space=\"preserve\">" + xmlEscape(s.text) + "</w:t></w:r>";
    x = revisionWrapped(s.textChr, x);
    s.text.clear();
    if (spend(x.size())) s.runs += x;
}

// The attributes of a tracked change: its number, its author (the file's table of them), and its time (the file's packed date: minutes in bits 0 to 5, hour 6 to 10, day 11 to 15, month 16 to 19, years since 1900 20 to 28).
QByteArray Reader::revisionAttributes(int author, int date)
{
    const QString name = author != kUnset && author >= 0 && author < m_authors.size() && !m_authors[author].isEmpty() ? m_authors[author] : QStringLiteral("Unknown");
    QByteArray x = " w:id=\"" + num(++m_revId) + "\" w:author=\"" + xmlEscape(name) + "\"";
    if (date != kUnset && date > 0) {
        const int minute = date & 0x3F, hour = (date >> 6) & 0x1F, day = (date >> 11) & 0x1F, month = (date >> 16) & 0xF, year = 1900 + ((date >> 20) & 0x1FF);
        if (month >= 1 && month <= 12 && day >= 1 && day <= 31 && hour < 24 && minute < 60)
            x += QString(" w:date=\"%1-%2-%3T%4:%5:00Z\"").arg(year, 4, 10, QLatin1Char('0')).arg(month, 2, 10, QLatin1Char('0')).arg(day, 2, 10, QLatin1Char('0')).arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0')).toLatin1();
    }
    return x;
}

// A run in the tracked changes its properties say it is in: deleted text is deleted text (w:delText), inserted text is inside w:ins.
QByteArray Reader::revisionWrapped(const Chr &c, QByteArray run)
{
    if (c.del == 1) {
        run.replace("<w:t ", "<w:delText ").replace("</w:t>", "</w:delText>").replace("<w:instrText", "<w:delInstrText").replace("</w:instrText>", "</w:delInstrText>");
        run = "<w:del" + revisionAttributes(c.delAuth, c.delDate) + ">" + run + "</w:del>";
    }
    if (c.ins == 1) run = "<w:ins" + revisionAttributes(c.insAuth, c.insDate) + ">" + run + "</w:ins>";
    return run;
}

// A change of the paragraph's format, with the format it had (w:pPrChange: the last thing in the paragraph's properties).
QByteArray Reader::paragraphChangeXml(const Par &p)
{
    if (p.prAuth == kUnset || p.oldPar == kUnset || p.oldPar >= m_oldPars.size()) return QByteArray();
    return "<w:pPrChange" + revisionAttributes(p.prAuth, p.prDate) + "><w:pPr>" + paragraphPropertiesXml(m_oldPars[p.oldPar], t, styleIdOf(m_oldPars[p.oldPar].style), false) + "</w:pPr></w:pPrChange>";
}

// The paragraph mark's own mark of a tracked change, when the mark is inserted or deleted (the properties at \par say so).
QByteArray Reader::markRevisionXml(const Chr &c)
{
    if (c.ins != 1 && c.del != 1) return QByteArray();
    QByteArray x = "<w:rPr>";
    if (c.ins == 1) x += "<w:ins" + revisionAttributes(c.insAuth, c.insDate) + "/>";
    if (c.del == 1) x += "<w:del" + revisionAttributes(c.delAuth, c.delDate) + "/>";
    return x + "</w:rPr>";
}

void Reader::addText(Story &s, const QString &text, const Chr &c)
{
    if (s.text.isEmpty()) s.textChr = c;
    else if (s.textChr != c || s.text.size() > 60000) {
        flushText(s);
        s.textChr = c;
    }
    s.text += text;
}

void Reader::addRaw(Story &s, const QByteArray &inner, const Chr &c, bool withProperties)
{
    flushText(s);
    const QByteArray rpr = withProperties ? runPropertiesXml(c, t) : QByteArray();
    const QByteArray x = revisionWrapped(c, "<w:r>" + (rpr.isEmpty() ? QByteArray() : "<w:rPr>" + rpr + "</w:rPr>") + inner + "</w:r>");
    if (spend(x.size())) s.runs += x;
}

void Reader::addInline(Story &s, const QByteArray &xml)
{
    flushText(s);
    if (spend(xml.size())) s.runs += xml;
}

void Reader::addField(const QByteArray &instr, const QString &result)
{
    Story &s = story();
    const Chr &c = st().chr;
    addRaw(s, "<w:fldChar w:fldCharType=\"begin\"/>", c);
    addRaw(s, "<w:instrText xml:space=\"preserve\"> " + instr + " </w:instrText>", c);
    addRaw(s, "<w:fldChar w:fldCharType=\"separate\"/>", c);
    addText(s, result, c);
    flushText(s);
    addRaw(s, "<w:fldChar w:fldCharType=\"end\"/>", c);
}

QString Reader::styleIdOf(int number) const
{
    if (number == kUnset || number == 0) return QString();
    const auto it = t.paraStyleAt.constFind(number);
    return it == t.paraStyleAt.constEnd() ? QString() : t.styles[it.value()].docxId;
}

void Reader::flushHeld(Story &s)
{
    if (!s.held.valid) return;
    QByteArray x = "<w:p>";
    if (!s.held.ppr.isEmpty() || s.held.sect) x += "<w:pPr>" + s.held.ppr + "</w:pPr>";
    x += s.held.runs + "</w:p>";
    s.body += x;
    s.held = Story::Held();
}

void Reader::ensureLevel(Story &s, int level)
{
    level = std::clamp(level, 0, kMaxTableDepth);
    while (s.levels.size() - 1 > level) closeLevel(s, s.levels.size() - 1);
    while (s.levels.size() - 1 < level) s.levels.append(Story::Level());
}

void Reader::closeTables(Story &s)
{
    while (s.levels.size() > 1) closeLevel(s, s.levels.size() - 1);
}

void Reader::closeLevel(Story &s, int level)
{
    if (level < 1 || level >= s.levels.size()) return;
    Story::Level lv = s.levels[level];
    s.levels.resize(level);
    if (!lv.cell.isEmpty() || !lv.cells.isEmpty()) {
        // (A row the file did not end: what it has is a row.)
        if (!lv.cell.isEmpty()) lv.cells.append(lv.cell);
        if (lv.rows.size() < kMaxTableRows) {
            Story::RowRec r;
            r.def = s.def[std::min(level, kMaxTableDepth + 1)];
            r.cells = lv.cells;
            lv.rows.append(r);
        }
    }
    if (lv.rows.isEmpty()) return;
    const QByteArray tbl = tableXml(lv.rows);
    if (level == 1) {
        flushHeld(s);
        s.body += tbl;
    } else {
        s.levels[level - 1].cell += tbl;
    }
}

void Reader::endParagraph(const State &g, int level, bool force)
{
    Story &s = story();
    if (m_stopped) return;
    flushText(s);
    if (!force && s.runs.isEmpty()) return;
    m_lastShppict = 0;
    if (++m_paragraphs > kMaxParagraphs) {
        m_stopped = true;
        left(QStringLiteral("paragraphs"), QCoreApplication::translate("jp::office::rtf", "The document has more paragraphs than the limit, so the rest of it was left out."));
        return;
    }
    const QByteArray ppr = paragraphPropertiesXml(g.par, t, styleIdOf(g.par.style), false) + markRevisionXml(g.chr) + paragraphChangeXml(g.par);
    spend(ppr.size() + 16);
    if (level <= 0) {
        closeTables(s);
        flushHeld(s);
        s.held.valid = true;
        s.held.ppr = ppr;
        s.held.runs = s.runs;
    } else {
        ensureLevel(s, level);
        s.levels[level].cell += "<w:p>" + (ppr.isEmpty() ? QByteArray() : "<w:pPr>" + ppr + "</w:pPr>") + s.runs + "</w:p>";
    }
    s.runs.clear();
}

QByteArray Reader::finishStory(Story &s, const State &g)
{
    flushText(s);
    if (s.hasContent()) endParagraph(g, 0, true);
    closeTables(s);
    flushHeld(s);
    return s.body.isEmpty() ? QByteArray("<w:p/>") : s.body;
}

void Reader::cellEnd(const State &g, int level)
{
    Story &s = story();
    if (m_stopped) return;
    level = std::clamp(level, 1, kMaxTableDepth);
    flushText(s);
    if (s.hasContent()) endParagraph(g, level, true);
    ensureLevel(s, level);
    Story::Level &lv = s.levels[level];
    if (lv.cell.isEmpty()) {
        const QByteArray ppr = paragraphPropertiesXml(g.par, t, styleIdOf(g.par.style), false);
        lv.cell = "<w:p>" + (ppr.isEmpty() ? QByteArray() : "<w:pPr>" + ppr + "</w:pPr>") + "</w:p>";
    }
    if (++m_tableCells > kMaxTableCellsAll || lv.cells.size() >= kMaxRowCells) {
        left(QStringLiteral("cells"), QCoreApplication::translate("jp::office::rtf", "A table is larger than the limit, so part of it was left out."));
        lv.cell.clear();
        return;
    }
    lv.cells.append(lv.cell);
    lv.cell.clear();
}

void Reader::rowEnd(const State &g, int level)
{
    Story &s = story();
    if (m_stopped) return;
    level = std::clamp(level, 1, kMaxTableDepth);
    flushText(s);
    if (s.hasContent()) cellEnd(g, level);
    ensureLevel(s, level);
    Story::Level &lv = s.levels[level];
    if (!lv.cell.isEmpty()) {
        lv.cells.append(lv.cell);
        lv.cell.clear();
    }
    if (lv.rows.size() >= kMaxTableRows) {
        left(QStringLiteral("rows"), QCoreApplication::translate("jp::office::rtf", "A table is larger than the limit, so part of it was left out."));
        lv.cells.clear();
        return;
    }
    Story::RowRec r;
    r.def = s.def[std::min(level, kMaxTableDepth + 1)];
    r.cells = lv.cells;
    lv.rows.append(r);
    lv.cells.clear();
}

RowDef &Reader::rowDefFor(const State &g)
{
    Story &s = story();
    const int level = g.dest == Dest::NestProps ? std::clamp(std::max(g.par.itap, 2), 2, kMaxTableDepth) : 1;
    return s.def[level];
}

Border *Reader::borderFor(State &g, int side)
{
    if (side < 0) return nullptr;
    switch (g.btarget) {
    case 0: return side < 5 ? &g.par.border[side] : nullptr;
    case 1: return side < 4 ? &m_pendCell.border[side] : nullptr;
    case 2: return side < 6 ? &rowDefFor(g).border[side] : nullptr;
    default: return nullptr;
    }
}

QByteArray Reader::tableXml(const QVector<Story::RowRec> &rows)
{
    if (rows.isEmpty()) return QByteArray();
    // The columns: every edge any row has, as far as the columns the .docx reader takes; a cell takes the columns between its edges.
    QVector<int> edges;
    for (const Story::RowRec &r : rows) {
        edges.append(r.def.left);
        int prev = r.def.left;
        for (const CellDef &c : r.def.cells) {
            if (c.right <= prev) continue;
            edges.append(c.right);
            prev = c.right;
        }
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    if (edges.size() > wml::kMaxTableColumns) {
        // (Too many: those of the widest row, and the others take the nearest.)
        int widest = 0;
        for (int i = 1; i < rows.size(); ++i)
            if (rows[i].def.cells.size() > rows[widest].def.cells.size()) widest = i;
        edges.clear();
        edges.append(rows[widest].def.left);
        for (const CellDef &c : rows[widest].def.cells)
            if (c.right > edges.last()) edges.append(c.right);
    }
    auto colOf = [&](int x) {
        const auto it = std::lower_bound(edges.begin(), edges.end(), x);
        if (it == edges.end()) return int(edges.size()) - 1;
        if (it == edges.begin()) return 0;
        return (x - *(it - 1) < *it - x) ? int(it - edges.begin()) - 1 : int(it - edges.begin());
    };
    const RowDef &first = rows[0].def;
    QByteArray x = "<w:tbl><w:tblPr>";
    const int total = edges.isEmpty() ? 0 : edges.last() - edges.first();
    x += "<w:tblW w:w=\"" + num(std::clamp(total, 0, 31680 * 4)) + "\" w:type=\"dxa\"/>";
    if (first.jc == 1) x += "<w:jc w:val=\"center\"/>";
    else if (first.jc == 2) x += "<w:jc w:val=\"right\"/>";
    x += "<w:tblInd w:w=\"" + num(std::clamp(first.left + first.gaph, -31680, 31680)) + "\" w:type=\"dxa\"/>";
    {
        static const char *const sides[] = {"top", "left", "bottom", "right", "insideH", "insideV"};
        QByteArray b;
        for (int i = 0; i < 6; ++i) b += borderXml(sides[i], first.border[i], t);
        if (!b.isEmpty()) x += "<w:tblBorders>" + b + "</w:tblBorders>";
    }
    x += "<w:tblLayout w:type=\"fixed\"/>";
    x += "<w:tblCellMar><w:left w:w=\"" + num(std::clamp(first.gaph, 0, 31680)) + "\" w:type=\"dxa\"/><w:right w:w=\"" + num(std::clamp(first.gaph, 0, 31680)) + "\" w:type=\"dxa\"/></w:tblCellMar>";
    x += "</w:tblPr><w:tblGrid>";
    // The columns can go on past the edges, for a row that has more cells than edges.
    int gridCols = std::max<int>(1, edges.size() - 1);
    for (const Story::RowRec &r : rows) {
        int col = r.def.cells.isEmpty() ? 0 : colOf(r.def.left);
        for (int i = 0; i < r.cells.size(); ++i) col += 1 + 0 * i;
        gridCols = std::max(gridCols, std::min<int>(col, wml::kMaxTableColumns));
    }
    gridCols = std::min(gridCols, wml::kMaxTableColumns);
    for (int i = 0; i < gridCols; ++i) {
        const int w = i + 1 < edges.size() ? edges[i + 1] - edges[i] : 720;
        x += "<w:gridCol w:w=\"" + num(std::max(w, 1)) + "\"/>";
    }
    x += "</w:tblGrid>";
    const int minEdge = edges.isEmpty() ? 0 : edges.first();
    for (const Story::RowRec &r : rows) {
        const RowDef &d = r.def;
        x += "<w:tr>";
        QByteArray trpr;
        if (d.cantSplit) trpr += "<w:cantSplit/>";
        if (d.height != 0) trpr += "<w:trHeight w:val=\"" + num(std::min(std::abs(d.height), 31680)) + "\" w:hRule=\"" + (d.height < 0 ? "exact" : "atLeast") + "\"/>";
        if (d.header) trpr += "<w:tblHeader/>";
        int startCol = 0;
        if (d.left > minEdge && !d.cells.isEmpty()) {
            startCol = std::min(colOf(d.left), gridCols - 1);
            if (startCol > 0) trpr += "<w:gridBefore w:val=\"" + num(startCol) + "\"/><w:wBefore w:w=\"" + num(d.left - minEdge) + "\" w:type=\"dxa\"/>";
        }
        if (!trpr.isEmpty()) x += "<w:trPr>" + trpr + "</w:trPr>";
        // The cells: one for each piece of content, with the edges the row's definition gives (and those after its last, one inch wide each).
        int prevRight = d.left, col = startCol;
        struct Out { int span, width; QByteArray tcpr, content; bool cont; };
        QVector<Out> outs;
        for (int i = 0; i < r.cells.size(); ++i) {
            CellDef c;
            if (i < d.cells.size()) c = d.cells[i];
            else c.right = prevRight + 1440;
            if (c.right <= prevRight) c.right = prevRight + 1;
            const int endCol = std::max(col + 1, i < d.cells.size() ? colOf(c.right) : col + 1);
            if (c.mergeCont && !outs.isEmpty() && !outs.last().cont) {
                outs.last().span += endCol - col;
                outs.last().width += c.right - prevRight;
                if (r.cells[i].contains("<w:r>")) outs.last().content += r.cells[i];   // (what the joined cell holds is kept)
                prevRight = c.right;
                col = endCol;
                continue;
            }
            Out o;
            o.span = endCol - col;
            o.width = c.right - prevRight;
            o.cont = false;
            QByteArray tcpr;
            if (c.vFirst) tcpr += "<w:vMerge w:val=\"restart\"/>";
            else if (c.vCont) tcpr += "<w:vMerge/>";
            QByteArray b;
            static const char *const sides[] = {"top", "left", "bottom", "right"};
            for (int k = 0; k < 4; ++k) b += borderXml(sides[k], c.border[k], t);
            if (!b.isEmpty()) tcpr += "<w:tcBorders>" + b + "</w:tcBorders>";
            const int fill = c.shd != kUnset ? c.shd : d.shd;
            if (fill != kUnset && fill > 0) tcpr += "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + colorHex(t, fill).toLatin1() + "\"/>";
            if (c.valign != kUnset) tcpr += c.valign == 1 ? "<w:vAlign w:val=\"center\"/>" : c.valign == 2 ? "<w:vAlign w:val=\"bottom\"/>" : "";
            o.tcpr = tcpr;
            o.content = r.cells[i];
            outs.append(o);
            prevRight = c.right;
            col = endCol;
        }
        for (const Out &o : outs) {
            x += "<w:tc><w:tcPr><w:tcW w:w=\"" + num(std::max(o.width, 0)) + "\" w:type=\"dxa\"/>";
            if (o.span > 1) x += "<w:gridSpan w:val=\"" + num(o.span) + "\"/>";
            x += o.tcpr + "</w:tcPr>" + o.content;
            if (o.content.isEmpty() || o.content.endsWith("</w:tbl>")) x += "<w:p/>";
            x += "</w:tc>";
        }
        if (outs.isEmpty()) x += "<w:tc><w:tcPr><w:tcW w:w=\"720\" w:type=\"dxa\"/></w:tcPr><w:p/></w:tc>";
        x += "</w:tr>";
    }
    x += "</w:tbl>";
    qint64 inner = 0;
    for (const Story::RowRec &r : rows)
        for (const QByteArray &c : r.cells) inner += c.size();
    spend(std::max<qint64>(0, x.size() - inner));
    return x;
}

// ---------------------------------------------------------------- sections

void Reader::sectionEnd(const State &g)
{
    Story &s = story();
    if (m_stopped) return;
    flushText(s);
    if (s.hasContent()) endParagraph(g, 0, true);
    if (s.levels.size() > 1) closeTables(s);
    static const char *const kinds[] = {"header", "header", "header", "footer", "footer", "footer"};
    static const char *const types[] = {"default", "first", "even", "default", "first", "even"};
    QByteArray refs;
    for (int i = 0; i < 6; ++i)
        if (!m_secRef[i].isEmpty()) refs += QByteArray("<w:") + kinds[i] + "Reference w:type=\"" + types[i] + "\" r:id=\"" + m_secRef[i].toUtf8() + "\"/>";
    const QByteArray sect = "<w:sectPr>" + sectionPropertiesXml(m_sec, refs) + "</w:sectPr>";
    if (++m_sections > kMaxSections) {
        left(QStringLiteral("sections"), QCoreApplication::translate("jp::office::rtf", "The document has more sections than the limit, so the rest were joined."));
        return;
    }
    if (s.held.valid && !s.held.sect) {
        if (const qsizetype at = s.held.ppr.indexOf("<w:pPrChange"); at >= 0) s.held.ppr.insert(at, sect);   // (the change of format is last)
        else s.held.ppr += sect;
        s.held.sect = true;
    } else {
        flushHeld(s);
        s.body += "<w:p><w:pPr>" + sect + "</w:pPr></w:p>";
    }
}

// ---------------------------------------------------------------- groups

void Reader::pushGroup()
{
    flushLead();
    m_ucSkip = 0;
    if (m_stack.size() > kMaxGroupDepth) {
        if (m_overflow++ == 0) left(QStringLiteral("depth"), QCoreApplication::translate("jp::office::rtf", "Some text is nested more deeply than the limit, and was left out."));
        return;
    }
    State child = st();
    child.owns = false;
    child.shapeText = false;
    child.shapePushed = false;
    if (child.dest == Dest::StyleSheet) {
        child.dest = Dest::StyleDef;
        child.owns = true;
        child.chr = Chr();
        child.par = Par();
        m_style = StyleDef();
        m_name.clear();
        m_stylePending = true;
    } else if (!inherits(child.dest)) {
        // (A group inside a note, header, field, or the like goes on as text.)
        if (child.dest != Dest::Normal) child.dest = Dest::Normal;
    }
    if (m_star) m_star = false;
    m_stack.append(child);
}

void Reader::popGroup()
{
    flushLead();
    m_ucSkip = 0;
    if (m_overflow > 0) {
        --m_overflow;
        return;
    }
    if (m_stack.size() <= 1) return;
    State g = m_stack.back();
    if (g.shapeText) finishShapeText(g);
    if (g.owns) {
        switch (g.dest) {
        case Dest::FontTbl: if (m_fontPending) commitFont(); break;
        case Dest::ColorTbl: break;
        case Dest::StyleDef: if (m_stylePending) commitStyle(); break;
        case Dest::ListLevel: finishListLevel(g); break;
        case Dest::LevelText: break;
        case Dest::ListDef:
            if (t.lists.size() < kMaxLists) {
                m_list.simple = m_list.levels.size() <= 1;
                t.listAt.insert(m_list.id, t.lists.size());
                t.lists.append(m_list);
            }
            m_list = ListDef();
            break;
        case Dest::ListOverride:
            if (t.listAt.contains(m_overrideList) && t.nums.size() < kMaxListInstances) {
                Tables::Num n;
                n.list = t.listAt.value(m_overrideList);
                n.overrides = m_overrides;
                t.nums.append(n);
                t.numAt.insert(m_overrideLs, t.nums.size());
            }
            m_overrides.clear();
            break;
        case Dest::LfoLevel:
            m_lfo.present = m_lfo.present;
            if (m_overrides.size() < 9) m_overrides.append(m_lfo);
            m_work.maxOverrides = std::max<int>(m_work.maxOverrides, m_overrides.size());
            m_lfo = ListLevel();
            break;
        case Dest::FldInst: {
            Story &s = story();
            if (!s.fields.isEmpty() && s.fields.last().real && !s.fields.last().begun && !m_instr.isEmpty()) {
                const Chr &c = m_stack[m_stack.size() - 2].chr;
                addRaw(s, "<w:fldChar w:fldCharType=\"begin\"/>", c);
                addRaw(s, "<w:instrText xml:space=\"preserve\">" + xmlEscape(QString::fromUtf8(m_instr)) + "</w:instrText>", c);
                addRaw(s, "<w:fldChar w:fldCharType=\"separate\"/>", c);
                s.fields.last().begun = true;
            }
            m_instr.clear();
            break;
        }
        case Dest::Field: {
            Story &s = story();
            if (!s.fields.isEmpty()) {
                if (s.fields.last().begun) addRaw(s, "<w:fldChar w:fldCharType=\"end\"/>", g.chr);
                s.fields.removeLast();
            }
            break;
        }
        case Dest::BkmkStart: {
            const QString name = m_name.trimmed().left(40);
            if (!name.isEmpty() && !m_bookmarks.contains(name) && m_bookmarks.size() < kMaxBookmarks && !m_stopped) {
                const int id = ++m_bookmarkId;
                m_bookmarks.insert(name, id);
                addInline(story(), "<w:bookmarkStart w:id=\"" + num(id) + "\" w:name=\"" + xmlEscape(name) + "\"/>");
            }
            m_name.clear();
            break;
        }
        case Dest::BkmkEnd: {
            const QString name = m_name.trimmed().left(40);
            const auto it = m_bookmarks.constFind(name);
            if (it != m_bookmarks.constEnd() && !m_stopped && !m_endedBookmarks.contains(name)) {
                addInline(story(), "<w:bookmarkEnd w:id=\"" + num(it.value()) + "\"/>");
                m_bookmarks.insert(name, it.value());
                m_endedBookmarks.insert(name);
            }
            m_name.clear();
            break;
        }
        case Dest::PnText:
            (m_pnAfter ? m_pn.after : m_pn.before) = m_name;
            m_name.clear();
            break;
        case Dest::Pict: finishPict(g); break;
        case Dest::Header: finishHeader(g); break;
        case Dest::Shape:
            m_shapeOpen = false;
            if (!m_shapeHadText) finishShapeDrawing();
            if (g.shapePushed && !m_shapeStack.isEmpty()) {
                const SavedShape outer = m_shapeStack.takeLast();
                m_shape = outer.shape;
                m_shapeOpen = outer.open;
                m_shapeHadText = outer.hadText;
            }
            break;
        case Dest::SpName: m_spName = m_name.trimmed(); m_name.clear(); break;
        case Dest::SpValue: {
            const int v = m_name.trimmed().toInt();
            const QString &n = m_spName;
            if (n == QLatin1String("shapeType")) m_shape.type = v;
            else if (n == QLatin1String("fFilled")) m_shape.filled = v;
            else if (n == QLatin1String("fillColor")) m_shape.fillColor = v;
            else if (n == QLatin1String("fLine")) m_shape.line = v;
            else if (n == QLatin1String("lineColor")) m_shape.lineColor = v;
            else if (n == QLatin1String("lineWidth")) m_shape.lineWidth = v;
            else if (n == QLatin1String("posrelh")) m_shape.posrelh = v;
            else if (n == QLatin1String("posrelv")) m_shape.posrelv = v;
            else if (n == QLatin1String("posh")) m_shape.posh = v;
            else if (n == QLatin1String("posv")) m_shape.posv = v;
            else if (n == QLatin1String("fBehindDocument")) m_shape.behind = v;
            else if (n == QLatin1String("dxTextLeft")) m_shape.inset[0] = v;
            else if (n == QLatin1String("dyTextTop")) m_shape.inset[1] = v;
            else if (n == QLatin1String("dxTextRight")) m_shape.inset[2] = v;
            else if (n == QLatin1String("dyTextBottom")) m_shape.inset[3] = v;
            m_name.clear();
            break;
        }
        case Dest::Footnote: finishNote(g); break;
        case Dest::OldPprops:
            if (m_stack.size() >= 2 && m_oldPars.size() < 100000) {
                m_oldPars.append(g.par);
                m_stack[m_stack.size() - 2].par.oldPar = int(m_oldPars.size()) - 1;
            }
            break;
        case Dest::OldCprops:
            if (m_stack.size() >= 2 && m_oldProps.size() < 100000) {
                m_oldProps.append(g.chr);
                m_stack[m_stack.size() - 2].chr.oldProps = int(m_oldProps.size()) - 1;
            }
            break;
        default: break;
        }
    }
    const bool isPn = g.owns && g.dest == Dest::Pn;
    m_stack.removeLast();
    if (isPn) finishPn(st());
    if (m_stack.size() == 1) {
        m_lastClosed = g;
        m_done = true;
    }
}

void Reader::finishPn(State &parent)
{
    if (!m_pn.any || t.lists.size() >= kMaxLists || t.nums.size() >= kMaxListInstances) {
        m_pn = PnScratch();
        return;
    }
    const QString key = QStringLiteral("%1|%2|%3|%4|%5|%6|%7").arg(m_pn.bullet).arg(m_pn.nfc).arg(m_pn.start).arg(m_pn.indent).arg(m_pn.font).arg(m_pn.before, m_pn.after);
    // (The paragraph right after one with the same settings goes on counting in its list.)
    if (key == m_pnKey && m_paragraphs == m_pnParagraph + 1) {
        parent.par.ls = m_pnLs;
        parent.par.ilvl = 0;
        m_pnParagraph = m_paragraphs;
        m_pn = PnScratch();
        return;
    }
    ListDef l;
    l.simple = true;
    l.id = -1 - m_pnStyleCounter;
    ListLevel lv;
    lv.present = true;
    lv.nfc = m_pn.bullet ? 23 : m_pn.nfc;
    lv.start = m_pn.start;
    lv.li = m_pn.indent;
    lv.fi = -m_pn.indent;
    lv.lvlText = m_pn.bullet ? (m_pn.before.isEmpty() ? QString(QChar(0x2022)) : m_pn.before) : m_pn.before + QStringLiteral("%1") + m_pn.after;
    l.levels.append(lv);
    t.listAt.insert(l.id, t.lists.size());
    t.lists.append(l);
    Tables::Num n;
    n.list = t.lists.size() - 1;
    t.nums.append(n);
    const int ls = -1 - m_pnStyleCounter++;
    t.numAt.insert(ls, t.nums.size());
    m_pnKey = key;
    m_pnLs = ls;
    m_pnParagraph = m_paragraphs;
    parent.par.ls = ls;
    parent.par.ilvl = 0;
    m_pn = PnScratch();
}

void Reader::commitFont()
{
    m_fontPending = false;
    Font f = m_font;
    f.name = m_name.trimmed();
    m_font = Font();
    m_name.clear();
    if (f.name.isEmpty() || t.fonts.size() >= kMaxFonts) return;
    if (f.codepage == 0) f.codepage = codepageOfCharset(f.charset);
    t.fontAt.insert(f.index, t.fonts.size());
    t.fonts.append(f);
}

void Reader::commitColor()
{
    if (t.colors.size() >= kMaxColors) return;
    t.colors.append(m_colorPending ? qint32((std::clamp(m_red, 0, 255) << 16) | (std::clamp(m_green, 0, 255) << 8) | std::clamp(m_blue, 0, 255)) : -1);
    m_colorPending = false;
    m_red = m_green = m_blue = 0;
}

void Reader::commitStyle()
{
    m_stylePending = false;
    State &g = st();
    StyleDef d = m_style;
    d.name = m_name.trimmed();
    d.par = g.par;
    d.chr = g.chr;
    m_name.clear();
    m_style = StyleDef();
    if (t.styles.size() >= kMaxStyles) return;
    auto &index = d.kind == 0 ? t.paraStyleAt : t.charStyleAt;
    if (d.kind == 2 || index.contains(d.id)) {
        if (d.kind != 2) return;
    }
    // The id the package gives it: the name without its spaces, made unique.
    QString id;
    bool up = true;
    for (QChar c : d.name) {
        if (c.isLetterOrNumber()) {
            id += up ? c.toUpper() : c;
            up = false;
        } else {
            up = true;
        }
    }
    if (d.kind == 0 && d.id == 0) id = QStringLiteral("Normal");
    if (id.isEmpty() || !id[0].isLetter()) id = QStringLiteral("S") + QString::number(d.id) + id;
    for (const StyleDef &o : t.styles)
        if (o.docxId.compare(id, Qt::CaseInsensitive) == 0) {
            id += QStringLiteral("_") + QString::number(d.kind) + QString::number(d.id);
            break;
        }
    d.docxId = id;
    if (d.kind != 2) index.insert(d.id, t.styles.size());
    t.styles.append(d);
}

void Reader::finishListLevel(const State &g)
{
    ListLevel lv = m_level;
    m_level = ListLevel();
    lv.present = true;
    lv.chr = g.chr;
    lv.li = g.par.li;
    lv.fi = g.par.fi;
    QString text;
    const int cp = codepageFor(g.chr.font);
    for (int i = 1; i < m_levelRaw.size(); ++i) {   // (the first is the count)
        const quint32 v = m_levelRaw[i];
        const bool raw = v & 0x80000000u;
        const uint c = v & 0xFFFFu;
        if (raw ? (c <= 8) : (c < 9)) text += QStringLiteral("%") + QString::number(c + 1);
        else if (raw) {
            if (c < 0x20) continue;
            text += c < 0x80 ? QString(QChar(c)) : QString(decodeHigh(cp, uchar(c)));
        } else {
            text += QChar(c);
        }
    }
    m_levelRaw.clear();
    if (text.endsWith(QLatin1Char(';'))) text.chop(1);   // (the semicolon that ends the text)
    if (lv.nfc == 23 && text.isEmpty()) text = QString(QChar(0x2022));
    lv.lvlText = text;
    if (m_list.levels.size() < 9) m_list.levels.append(lv);
}

void Reader::finishHeader(const State &g)
{
    if (m_stories.empty()) return;
    // (Finished while it is still the story text goes to: what its last paragraph has, with no mark to end it, is in it.)
    const QByteArray body = finishStory(*m_stories.back(), g);
    std::unique_ptr<Story> s = std::move(m_stories.back());
    m_stories.pop_back();
    if (m_stopped || s->slot < 0) return;
    const bool isHeader = s->slot < 3;
    pkg.addPart(s->part, header(isHeader ? "w:hdr" : "w:ftr", body), QString::fromLatin1(kWml) + (isHeader ? QStringLiteral("header+xml") : QStringLiteral("footer+xml")));
    m_secRef[s->slot] = pkg.addRelationship(QStringLiteral("/word/document.xml"), QString::fromLatin1(kRel) + (isHeader ? QStringLiteral("header") : QStringLiteral("footer")), s->part);
}

void Reader::shapeWord(uint32_t h, int param)
{
    ShapeScratch &s = m_shape;
    switch (h) {
    case H("shpleft"): s.left = param; break;
    case H("shptop"): s.top = param; break;
    case H("shpright"): s.right = param; break;
    case H("shpbottom"): s.bottom = param; break;
    case H("shpwr"): s.wrap = param; break;
    case H("shpz"): s.z = param; break;
    case H("shpfblwtxt"): s.behind = param; break;
    case H("shpbxmargin"): s.bx = 0; break;
    case H("shpbxpage"): s.bx = 1; break;
    case H("shpbxcolumn"): s.bx = 2; break;
    case H("shpbymargin"): s.by = 0; break;
    case H("shpbypage"): s.by = 1; break;
    case H("shpbypara"): s.by = 2; break;
    default: break;
    }
}

QByteArray Reader::shapeAttributes(const char *type, double least, bool filled, bool strokedByDefault)
{
    const ShapeScratch &s = m_shape;
    auto pt = [](double v) { return QByteArray::number(v, 'f', 1); };
    const double w = std::clamp((double(s.right) - s.left) / 20.0, least, 1584.0), h = std::clamp((double(s.bottom) - s.top) / 20.0, least, 1584.0);
    const int hrel = s.posrelh != kUnset ? std::clamp(s.posrelh, 0, 2) : s.bx, vrel = s.posrelv != kUnset ? std::clamp(s.posrelv, 0, 2) : s.by;
    const int id = ++m_drawingId;
    QByteArray style = "position:absolute;margin-left:" + pt(std::clamp(double(s.left), -31680.0, 31680.0 * 4) / 20.0) + "pt;margin-top:" + pt(std::clamp(double(s.top), -31680.0, 31680.0 * 4) / 20.0) + "pt;width:" +
                       pt(w) + "pt;height:" + pt(h) + "pt;z-index:" + num(s.behind ? -251658240 + 1024 * id : 251658240 + 1024 * id);
    // (Placed by an alignment, not by the offsets: left, center, right, inside, or outside.)
    static const char *const kAligns[] = {nullptr, "left", "center", "right", "inside", "outside"};
    if (s.posh >= 1 && s.posh <= 5) style += QByteArray(";mso-position-horizontal:") + (s.posh == 1 ? "left" : kAligns[s.posh]);
    if (s.posv >= 1 && s.posv <= 5) style += QByteArray(";mso-position-vertical:") + (s.posv == 1 ? "top" : s.posv == 3 ? "bottom" : kAligns[s.posv]);
    if (hrel != 2) style += QByteArray(";mso-position-horizontal-relative:") + (hrel == 0 ? "margin" : "page");
    if (vrel != 2) style += QByteArray(";mso-position-vertical-relative:") + (vrel == 0 ? "margin" : "page");
    auto rgb = [](int v) { return QByteArray("#") + QByteArray::number(((v & 0xFF) << 16) | (v & 0xFF00) | ((v >> 16) & 0xFF), 16).rightJustified(6, '0'); };
    QByteArray attrs = QByteArray(" type=\"") + type + "\" id=\"_x0000_s" + num(id) + "\" style=\"" + style + "\"";
    attrs += filled ? (s.filled == 1 ? " filled=\"t\" fillcolor=\"" + rgb(s.fillColor) + "\"" : QByteArray(" filled=\"f\"")) : QByteArray(" filled=\"f\"");
    if (s.line == 1 || (s.line == kUnset && strokedByDefault)) attrs += " stroked=\"t\" strokecolor=\"" + rgb(s.lineColor) + "\" strokeweight=\"" + pt(s.lineWidth != kUnset ? std::clamp(s.lineWidth, 0, 3175000) / 12700.0 : 0.75) + "pt\"";
    else attrs += " stroked=\"f\"";
    return attrs;
}

// A shape with no text of its own that is a line, a rectangle, or an oval: it is drawn where the file puts it.
void Reader::finishShapeDrawing()
{
    const ShapeScratch &s = m_shape;
    if (m_stopped || m_shapes >= kMaxShapes || (s.type != 20 && s.type != 1 && s.type != 2 && s.type != 3)) return;
    ++m_shapes;
    auto pt = [](double v) { return QByteArray::number(v, 'f', 1); };
    const QByteArray wrapNone = "<w10:wrap type=\"none\"/>";
    QByteArray xml;
    if (s.type == 20) {
        const double w = std::clamp((double(s.right) - s.left) / 20.0, 0.0, 1584.0), h = std::clamp((double(s.bottom) - s.top) / 20.0, 0.0, 1584.0);
        const QByteArray attrs = shapeAttributes("", 0.0, false, true);
        xml = "<w:pict><v:line" + attrs.mid(attrs.indexOf(" id=")) + " from=\"0pt,0pt\" to=\"" + pt(w) + "pt," + pt(h) + "pt\">" + wrapNone + "</v:line></w:pict>";
    } else {
        const QByteArray attrs = shapeAttributes("", 1.0, true, true);
        const char *tag = s.type == 3 ? "v:oval" : "v:rect";
        xml = QByteArray("<w:pict><") + tag + attrs.mid(attrs.indexOf(" id=")) + ">" + wrapNone + "</" + tag + "></w:pict>";
    }
    addRaw(story(), xml, st().chr, false);
}

void Reader::finishShapeText(const State &g)
{
    if (m_stories.empty()) return;
    const QByteArray body = finishStory(*m_stories.back(), g);
    m_stories.pop_back();
    if (m_stopped) return;
    const ShapeScratch &s = m_shape;
    auto pt = [](double v) { return QByteArray::number(v, 'f', 1); };
    const QByteArray attrs = shapeAttributes("#_x0000_t202", 1.0);
    m_shapeHadText = true;
    QByteArray textbox = "<v:textbox";
    if (s.inset[0] != kUnset || s.inset[1] != kUnset || s.inset[2] != kUnset || s.inset[3] != kUnset) {
        auto in = [&](int v, double d) { return pt(v == kUnset ? d : std::clamp(v, 0, 3175000) / 12700.0); };
        textbox += " inset=\"" + in(s.inset[0], 7.2) + "pt," + in(s.inset[1], 3.6) + "pt," + in(s.inset[2], 7.2) + "pt," + in(s.inset[3], 3.6) + "pt\"";
    }
    QByteArray wrap;
    if (s.wrap == 1) wrap = "<w10:wrap type=\"topAndBottom\"/>";
    else if (s.wrap == 4) wrap = "<w10:wrap type=\"tight\"/>";
    else if (s.wrap == 5) wrap = "<w10:wrap type=\"through\"/>";
    else if (s.wrap != 3) wrap = "<w10:wrap type=\"square\"/>";
    const QByteArray xml = "<w:pict><v:shape" + attrs + ">" + textbox + "><w:txbxContent>" + body + "</w:txbxContent></v:textbox>" + wrap + "</v:shape></w:pict>";
    addRaw(story(), xml, m_stack[m_stack.size() - 2].chr, false);
}

void Reader::finishNote(const State &g)
{
    if (m_stories.empty()) return;
    // (Finished while it is still the story text goes to: what its last paragraph has, with no mark to end it, is in it.)
    const QByteArray body = finishStory(*m_stories.back(), g);
    std::unique_ptr<Story> s = std::move(m_stories.back());
    m_stories.pop_back();
    if (m_stopped) return;
    const int kind = s->endnote ? 1 : 0;
    const int id = ++m_noteCount[kind];
    const char *tag = kind ? "w:endnote" : "w:footnote";
    m_notesXml[kind] += QByteArray("<") + tag + " w:id=\"" + num(id) + "\">" + body + "</" + tag + ">";
    Chr c = m_stack[m_stack.size() - 2].chr;
    c.vert = 1;
    addRaw(story(), QByteArray(kind ? "<w:endnoteReference w:id=\"" : "<w:footnoteReference w:id=\"") + num(id) + "\"/>", c);
}

QString Reader::addMedia(const QString &part, const QByteArray &bytes, const QString &ext)
{
    QString media = m_mediaOf.value(bytes);
    if (media.isEmpty()) {
        media = QStringLiteral("/word/media/image%1.%2").arg(++m_mediaCount).arg(ext);
        static const QHash<QString, QString> types = {{"png", "image/png"}, {"jpeg", "image/jpeg"}, {"emf", "image/x-emf"}, {"wmf", "image/x-wmf"}};
        pkg.addPart(media, bytes, types.value(ext, QStringLiteral("application/octet-stream")));
        m_mediaOf.insert(bytes, media);
    }
    const QString relKey = part + QLatin1Char('|') + media;
    QString rid = m_relOfMedia.value(relKey);
    if (rid.isEmpty()) {
        rid = pkg.addRelationship(part, QString::fromLatin1(kRel) + QStringLiteral("image"), media);
        m_relOfMedia.insert(relKey, rid);
    }
    return rid;
}

QByteArray Reader::drawingXml(const Picture &p, const QString &rid)
{
    double w = 0, h = 0;
    if (p.wGoal > 0 && p.hGoal > 0) {
        w = double(p.wGoal) * (p.scaleX > 0 ? p.scaleX : 100) / 100.0 / 20.0;
        h = double(p.hGoal) * (p.scaleY > 0 ? p.scaleY : 100) / 100.0 / 20.0;
    } else if (p.w > 0 && p.h > 0) {
        w = double(p.w) * 72.0 / 2540.0 * (p.scaleX > 0 ? p.scaleX : 100) / 100.0;
        h = p.h * 72.0 / 2540.0 * (p.scaleY > 0 ? p.scaleY : 100) / 100.0;
    }
    if (w < 1 || h < 1) {
        QSize size;
        const QByteArray &d = p.data;
        auto be16 = [&](qsizetype o) { return o + 2 <= d.size() ? (int(uchar(d[o])) << 8) | uchar(d[o + 1]) : 0; };
        if (p.format == 1 && d.size() >= 24) {
            size = QSize(int((quint32(be16(16)) << 16) | quint32(be16(18))), int((quint32(be16(20)) << 16) | quint32(be16(22))));
        } else if (p.format == 2) {
            // (The size is in the first frame header: read from the markers, never by decoding the picture.)
            qsizetype o = 2;
            for (int steps = 0; steps < 4096 && o + 4 <= d.size(); ++steps) {
                if (uchar(d[o]) != 0xFF) {
                    ++o;
                    continue;
                }
                const int marker = uchar(d[o + 1]);
                if (marker == 0xFF) {
                    ++o;
                    continue;
                }
                if ((marker >= 0xC0 && marker <= 0xCF) && marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
                    size = QSize(be16(o + 7), be16(o + 5));
                    break;
                }
                if (marker == 0xD8 || (marker >= 0xD0 && marker <= 0xD7) || marker == 0x01) o += 2;
                else o += 2 + be16(o + 2);
            }
        }
        if (size.isValid() && size.width() > 0 && size.height() > 0) {
            w = size.width() * 0.75;
            h = size.height() * 0.75;
        } else {
            w = h = 72;
        }
    }
    w = std::clamp(w, 1.0, 1584.0);
    h = std::clamp(h, 1.0, 1584.0);
    const qint64 cx = qint64(w * 12700), cy = qint64(h * 12700);
    const QByteArray id = num(++m_drawingId);
    QByteArray x = "<w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"" + num(cx) + "\" cy=\"" + num(cy) + "\"/><wp:docPr id=\"" + id + "\" name=\"Picture " + id +
                   "\"/><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"" + id + "\" name=\"Picture " + id +
                   "\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"" + rid.toUtf8() + "\"/>";
    if ((p.cropL || p.cropT || p.cropR || p.cropB) && p.wGoal > 0 && p.hGoal > 0) {
        const double fw = double(p.wGoal) + p.cropL + p.cropR, fh = double(p.hGoal) + p.cropT + p.cropB;
        auto pct = [](double v) { return num(qint64(std::clamp(v, 0.0, 0.95) * 100000)); };
        x += "<a:srcRect l=\"" + pct(p.cropL / fw) + "\" t=\"" + pct(p.cropT / fh) + "\" r=\"" + pct(p.cropR / fw) + "\" b=\"" + pct(p.cropB / fh) + "\"/>";
    }
    x += "<a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + num(cx) + "\" cy=\"" + num(cy) +
         "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>";
    return x;
}

void Reader::finishPict(const State &g)
{
    Picture p = std::move(m_pict);
    m_pict = Picture();
    m_half = -1;
    if (m_stopped) return;
    const auto u16 = [&](int o) { return p.data.size() >= o + 2 ? int(uchar(p.data[o])) | (int(uchar(p.data[o + 1])) << 8) : -1; };
    const auto u32 = [&](int o) { return p.data.size() >= o + 4 ? quint32(uchar(p.data[o])) | (quint32(uchar(p.data[o + 1])) << 8) | (quint32(uchar(p.data[o + 2])) << 16) | (quint32(uchar(p.data[o + 3])) << 24) : 0u; };
    QString ext;
    const QByteArray &d = p.data;
    // (What the bytes are decides, not the word before them: some programs write a JPEG after \pngblip.)
    if (p.format != -1) {
        if (d.startsWith("\x89PNG")) ext = QStringLiteral("png");
        else if (d.startsWith("\xFF\xD8")) ext = QStringLiteral("jpeg");
        else if (u32(0) == 1 && u32(40) == 0x464D4520u) ext = QStringLiteral("emf");
        else if (u32(0) == 0x9AC6CDD7u || ((u16(0) == 1 || u16(0) == 2) && u16(2) == 9)) ext = QStringLiteral("wmf");
    }
    Picture shown = p;
    shown.format = ext == QLatin1String("png") ? 1 : ext == QLatin1String("jpeg") ? 2 : 0;
    if (ext.isEmpty() || p.tooBig || m_pictures >= kMaxPictures || m_pictureBytes + d.size() > kMaxPictureTotal) {
        left(QStringLiteral("pictures"), QCoreApplication::translate("jp::office::rtf", "Some pictures could not be shown, and were left out."));
        return;
    }
    ++m_pictures;
    m_pictureBytes += d.size();
    Story &s = story();
    const QString rid = addMedia(s.part, d, ext);
    addRaw(s, drawingXml(shown, rid), Chr(), false);
    m_lastShppict = g.shp ? 1 : 0;
}

// ---------------------------------------------------------------- the words

void Reader::pictWord(uint32_t h, int param)
{
    switch (h) {
    case H("pngblip"): m_pict.format = 1; break;
    case H("jpegblip"): m_pict.format = 2; break;
    case H("emfblip"): m_pict.format = 3; break;
    case H("wmetafile"): m_pict.format = 4; break;
    case H("dibitmap"): case H("wbitmap"): case H("pmmetafile"): case H("macpict"): m_pict.format = -1; break;
    case H("picw"): m_pict.w = param; break;
    case H("pich"): m_pict.h = param; break;
    case H("picwgoal"): m_pict.wGoal = param; break;
    case H("pichgoal"): m_pict.hGoal = param; break;
    case H("picscalex"): m_pict.scaleX = param; break;
    case H("picscaley"): m_pict.scaleY = param; break;
    case H("piccropl"): m_pict.cropL = std::max(param, 0); break;
    case H("piccropt"): m_pict.cropT = std::max(param, 0); break;
    case H("piccropr"): m_pict.cropR = std::max(param, 0); break;
    case H("piccropb"): m_pict.cropB = std::max(param, 0); break;
    default: break;
    }
}

void Reader::listWord(State &g, uint32_t h, bool has, int param)
{
    switch (g.dest) {
    case Dest::ListDef:
        if (h == H("listid")) m_list.id = param;
        else if (h == H("listsimple")) m_list.simple = param != 0;
        break;
    case Dest::ListLevel:
        switch (h) {
        case H("levelnfc"): case H("levelnfcn"): m_level.nfc = param; break;
        case H("leveljc"): case H("leveljcn"): m_level.jc = param; break;
        case H("levelstartat"): m_level.start = param; break;
        case H("levelfollow"): m_level.follow = param; break;
        case H("levellegal"): m_level.legal = !has || param ? 1 : 0; break;
        case H("levelnorestart"): m_level.noRestart = !has || param ? 1 : 0; break;
        default: break;
        }
        break;
    case Dest::ListOverride:
        if (h == H("listid")) m_overrideList = param;
        else if (h == H("ls")) m_overrideLs = param;
        break;
    case Dest::LfoLevel:
        if (h == H("levelstartat")) {
            m_lfo.start = param;
            m_lfo.present = true;
        }
        break;
    default:
        break;
    }
}

void Reader::destWord(State &g, uint32_t h, bool has, int param, bool *handled)
{
    *handled = true;
    // (A group makes one thing: a second word that would make another in the same group is no more than a word.)
    if (g.owns) {
        switch (h) {
        case H("field"): case H("fldinst"): case H("footnote"): case H("header"): case H("headerr"): case H("headerl"): case H("headerf"): case H("footer"): case H("footerr"):
        case H("footerl"): case H("footerf"): case H("pict"): case H("bkmkstart"): case H("bkmkend"): case H("pn"): case H("nesttableprops"):
            return;
        default: break;
        }
    }
    auto enter = [&](Dest d) {
        g.dest = d;
        g.owns = true;
    };
    Story &s = story();
    switch (h) {
    case H("fonttbl"): enter(Dest::FontTbl); m_name.clear(); m_font = Font(); m_fontPending = true; break;
    case H("colortbl"): enter(Dest::ColorTbl); m_colorPending = false; m_red = m_green = m_blue = 0; break;
    case H("stylesheet"): enter(Dest::StyleSheet); break;
    case H("revtbl"): enter(Dest::RevTbl); m_name.clear(); break;
    case H("oldcprops"): enter(Dest::OldCprops); g.chr = Chr(); break;
    case H("oldpprops"): enter(Dest::OldPprops); g.par = Par(); break;
    case H("info"): case H("filetbl"): case H("listtext"): case H("pntext"): case H("pnseclvl"): case H("listname"): case H("upr"): case H("annotation"):
    case H("xe"): case H("tc"): case H("tcn"): case H("rxe"): case H("txe"): case H("pgdsctbl"): case H("template"): case H("fchars"): case H("lchars"): case H("mmath"):
    case H("shprslt"): case H("nonesttables"): case H("levelnumbers"): case H("background"): case H("stylerestrictions"): case H("themedata"): case H("falt"): case H("panose"):
        g.dest = Dest::Skip;
        g.owns = true;
        break;
    case H("nonshppict"):
        if (m_lastShppict) {
            g.dest = Dest::Skip;
            g.owns = true;
        }
        break;
    case H("shppict"): g.shp = true; break;
    case H("sp"): enter(Dest::ShapeProp); break;
    case H("sn"): enter(Dest::SpName); m_name.clear(); break;
    case H("sv"): enter(Dest::SpValue); m_name.clear(); break;
    case H("shpinst"):
        enter(Dest::Shape);
        if (m_shapeStack.size() < 4) {
            m_shapeStack.push_back({m_shape, m_shapeOpen, m_shapeHadText});
            g.shapePushed = true;
        }
        m_shape = ShapeScratch();
        m_shapeOpen = true;
        m_shapeHadText = false;
        break;
    case H("shptxt"):
        if (m_shapeOpen && m_shapes < kMaxShapes && m_stories.size() < 3u && !m_stopped) {
            ++m_shapes;
            g.shapeText = true;
            g.owns = true;
            auto ns = std::make_unique<Story>();
            ns->part = story().part;
            m_stories.push_back(std::move(ns));
            g.chr = Chr();
            g.par = Par();
        }
        break;
    case H("field"): {
        enter(Dest::Field);
        Story::FieldRec f;
        f.real = s.fields.size() < kMaxFieldDepth;
        s.fields.append(f);
        break;
    }
    case H("fldinst"):
        if (s.fields.isEmpty()) {
            g.dest = Dest::Skip;
            g.owns = true;
        } else {
            enter(Dest::FldInst);
            m_instr.clear();
        }
        break;
    case H("bkmkstart"): enter(Dest::BkmkStart); m_name.clear(); break;
    case H("bkmkend"): enter(Dest::BkmkEnd); m_name.clear(); break;
    case H("pict"): enter(Dest::Pict); m_pict = Picture(); m_half = -1; break;
    case H("nesttableprops"): enter(Dest::NestProps); break;
    case H("pn"): enter(Dest::Pn); m_pn = PnScratch(); m_pn.any = true; break;
    case H("listtable"): enter(Dest::ListTable); break;
    case H("listoverridetable"): enter(Dest::ListOverrideTable); break;
    case H("list"):
        if (g.dest == Dest::ListTable || g.dest == Dest::ListDef) {
            enter(Dest::ListDef);
            m_list = ListDef();
        } else *handled = false;
        break;
    case H("listlevel"):
        if (g.dest == Dest::ListDef) {
            enter(Dest::ListLevel);
            m_level = ListLevel();
            m_levelRaw.clear();
            g.chr = Chr();
            g.par = Par();
        } else *handled = false;
        break;
    case H("leveltext"):
        if (g.dest == Dest::ListLevel) {
            enter(Dest::LevelText);
            m_levelRaw.clear();
        } else *handled = false;
        break;
    case H("listoverride"):
        if (g.dest == Dest::ListOverrideTable || g.dest == Dest::ListOverride) {
            enter(Dest::ListOverride);
            m_overrideList = 0;
            m_overrideLs = 0;
            m_overrides.clear();
        } else *handled = false;
        break;
    case H("lfolevel"):
        if (g.dest == Dest::ListOverride) {
            enter(Dest::LfoLevel);
            m_lfo = ListLevel();
        } else *handled = false;
        break;
    case H("header"): case H("headerr"): case H("headerl"): case H("headerf"): case H("footer"): case H("footerr"): case H("footerl"): case H("footerf"): {
        const bool isHeader = h == H("header") || h == H("headerr") || h == H("headerl") || h == H("headerf");
        const int kind = (h == H("headerf") || h == H("footerf")) ? 1 : (h == H("headerl") || h == H("footerl")) ? 2 : 0;
        if (m_headers >= kMaxHeaderStories || m_stories.size() >= 3u || m_stopped) {
            g.dest = Dest::Skip;
            g.owns = true;
            left(QStringLiteral("headers"), QCoreApplication::translate("jp::office::rtf", "Some headers or footers were left out."));
            break;
        }
        enter(Dest::Header);
        auto ns = std::make_unique<Story>();
        ns->slot = (isHeader ? 0 : 3) + kind;
        ns->part = QStringLiteral("/word/%1%2.xml").arg(isHeader ? "header" : "footer").arg(++m_headers);
        m_stories.push_back(std::move(ns));
        g.chr = Chr();
        g.par = Par();
        break;
    }
    case H("footnote"):
        if (m_noteCount[0] + m_noteCount[1] >= kMaxNotes || m_stories.size() >= 3u || m_stopped) {
            g.dest = Dest::Skip;
            g.owns = true;
            left(QStringLiteral("notes"), QCoreApplication::translate("jp::office::rtf", "Some notes were left out."));
            break;
        }
        enter(Dest::Footnote);
        {
            auto ns = std::make_unique<Story>();
            ns->note = true;
            ns->part = QStringLiteral("/word/footnotes.xml");
            m_stories.push_back(std::move(ns));
        }
        g.chr = Chr();
        g.par = Par();
        break;
    default:
        *handled = false;
        break;
    }
}

void Reader::charWord(State &g, uint32_t h, bool has, int param, bool *handled)
{
    *handled = true;
    Chr &c = g.chr;
    const int on = (!has || param != 0) ? 1 : 0;
    switch (h) {
    case H("plain"): {
        // (Plain text is plain in its look; the changes it is in go on.)
        const Chr was = c;
        c = Chr();
        c.ins = was.ins; c.insAuth = was.insAuth; c.insDate = was.insDate;
        c.del = was.del; c.delAuth = was.delAuth; c.delDate = was.delDate;
        break;
    }
    case H("revised"): c.ins = on; break;
    case H("revauth"): if (has) c.insAuth = param; break;
    case H("revdttm"): if (has) c.insDate = param; break;
    case H("deleted"): c.del = on; break;
    case H("revauthdel"): if (has) c.delAuth = param; break;
    case H("revdttmdel"): if (has) c.delDate = param; break;
    case H("crauth"): if (has) c.crAuth = param; break;
    case H("crdate"): if (has) c.crDate = param; break;
    case H("pprauth"): if (has) g.par.prAuth = param; break;
    case H("pprdate"): if (has) g.par.prDate = param; break;
    case H("b"): c.bold = on; break;
    case H("i"): c.italic = on; break;
    case H("strike"): c.strike = on; break;
    case H("striked"): c.dstrike = on; break;
    case H("caps"): c.caps = on; break;
    case H("scaps"): c.scaps = on; break;
    case H("v"): c.hidden = on; break;
    case H("outl"): c.outline = on; break;
    case H("shad"): c.shadow = on; break;
    case H("embo"): c.emboss = on; break;
    case H("impr"): c.imprint = on; break;
    case H("ul"): c.ul = on ? 1 : 0; break;
    case H("ulnone"): c.ul = 0; break;
    case H("uld"): c.ul = 3; break;
    case H("uldb"): c.ul = 2; break;
    case H("ulw"): c.ul = 4; break;
    case H("ulth"): c.ul = 5; break;
    case H("ulwave"): c.ul = 6; break;
    case H("uldash"): c.ul = 7; break;
    case H("uldashd"): c.ul = 8; break;
    case H("uldashdd"): c.ul = 9; break;
    case H("ulthd"): c.ul = 10; break;
    case H("ulthdash"): c.ul = 11; break;
    case H("ululdbwave"): c.ul = 12; break;
    case H("ulldash"): c.ul = 13; break;
    case H("ulc"): c.ulColor = param; break;
    case H("super"): c.vert = on ? 1 : 0; break;
    case H("sub"): c.vert = on ? 2 : 0; break;
    case H("nosupersub"): c.vert = 0; break;
    case H("up"): c.pos = has ? param : 6; break;
    case H("dn"): c.pos = has ? -param : -6; break;
    case H("fs"): if (has && param > 0) c.size = param; break;
    case H("f"): if (has) c.font = param; break;
    case H("cf"): if (has) c.color = param; break;
    case H("highlight"): if (has) c.hl = param; break;
    case H("cb"): case H("chcbpat"): if (has) c.shd = param; break;
    case H("expnd"): if (has) c.spacing = int(std::clamp<qint64>(qint64(param) * 5 / 4, -31680, 31680)); break;
    case H("expndtw"): if (has) c.spacing = param; break;
    case H("charscalex"): if (has) c.scale = param; break;
    case H("lang"): if (has) c.lang = param; break;
    case H("cs"): if (has) c.style = param; break;
    default: *handled = false; break;
    }
}

void Reader::parWord(State &g, uint32_t h, bool has, int param, bool *handled)
{
    *handled = true;
    Par &p = g.par;
    const int on = (!has || param != 0) ? 1 : 0;
    auto border = [&](int side) {
        g.bsel = side;
        g.btarget = 0;
    };
    auto setBorderStyle = [&](int style) {
        auto apply = [&](Border *b) {
            if (!b) return;
            b->style = style;
            if (b->width == 0) b->width = 15;
        };
        if (g.bsel == 9) {
            for (int i = 0; i < 4; ++i) apply(borderFor(g, i));
        } else {
            apply(borderFor(g, g.bsel));
        }
    };
    auto eachBorder = [&](auto &&f) {
        if (g.bsel == 9) {
            for (int i = 0; i < 4; ++i)
                if (Border *b = borderFor(g, i)) f(*b);
        } else if (Border *b = borderFor(g, g.bsel)) {
            f(*b);
        }
    };
    switch (h) {
    case H("pard"):
        p = Par();
        g.tabType = g.tabLead = 0;
        g.bsel = -1;
        break;
    case H("s"): if (has) p.style = param; break;
    case H("ql"): p.align = 0; break;
    case H("qc"): p.align = 1; break;
    case H("qr"): p.align = 2; break;
    case H("qj"): case H("qd"): case H("qt"): p.align = 3; break;
    case H("li"): case H("lin"): if (has) p.li = param; break;
    case H("ri"): case H("rin"): if (has) p.ri = param; break;
    case H("fi"): if (has) p.fi = param; break;
    case H("sb"): if (has) p.sb = std::max(param, 0); break;
    case H("sa"): if (has) p.sa = std::max(param, 0); break;
    case H("sl"): if (has) p.sl = param; break;
    case H("slmult"): if (has) p.slmult = param; break;
    case H("keep"): p.keep = on; break;
    case H("keepn"): p.keepn = on; break;
    case H("pagebb"): p.pagebb = on; break;
    case H("widctlpar"): p.widow = 1; break;
    case H("nowidctlpar"): p.widow = 0; break;
    case H("hyphpar"): p.noHyph = on ? 0 : 1; break;
    case H("contextualspace"): p.contextual = on; break;
    case H("outlinelevel"): if (has) p.outline = param; break;
    case H("intbl"): p.itap = std::max(p.itap, 1); break;
    case H("itap"): if (has) p.itap = std::clamp(param, 0, kMaxTableDepth); break;
    case H("ls"):
        if (has) p.ls = param;
        break;
    case H("ilvl"): if (has) p.ilvl = param; break;
    case H("tqr"): g.tabType = 2; break;
    case H("tqc"): g.tabType = 1; break;
    case H("tqdec"): g.tabType = 3; break;
    case H("tldot"): g.tabLead = 1; break;
    case H("tlmdot"): g.tabLead = 5; break;
    case H("tlhyph"): g.tabLead = 2; break;
    case H("tlul"): g.tabLead = 3; break;
    case H("tlth"): g.tabLead = 4; break;
    case H("tleq"): g.tabLead = 3; break;
    case H("tx"): case H("tb"):
        if (has && p.tabs.size() < kMaxTabs) {
            Tab t;
            t.pos = std::clamp(param, 0, 31680);
            t.type = h == H("tb") ? 4 : g.tabType;
            t.leader = g.tabLead;
            p.tabs.append(t);
        }
        g.tabType = g.tabLead = 0;
        break;
    case H("cbpat"): if (has) p.shd = param; break;
    case H("brdrt"): border(0); break;
    case H("brdrl"): border(1); break;
    case H("brdrb"): border(2); break;
    case H("brdrr"): border(3); break;
    case H("brdrbtw"): border(4); break;
    case H("box"): g.bsel = 9; g.btarget = 0; break;
    case H("brdrw"): eachBorder([&](Border &b) { b.width = std::clamp(param, 0, 255); }); break;
    case H("brdrcf"): eachBorder([&](Border &b) { b.color = param; }); break;
    case H("brsp"): eachBorder([&](Border &b) { b.space = std::clamp(param, 0, 620); }); break;
    default: {
        // The kinds of line a border can be (an index of kBorderNames; those with two or three lines are double).
        static const struct { uint32_t h; int style; } kinds[] = {
            {H("brdrs"), 1}, {H("brdrhair"), 1}, {H("brdrsh"), 1}, {H("brdrth"), 2}, {H("brdrdb"), 3}, {H("brdrdot"), 4}, {H("brdrdash"), 5}, {H("brdrdashsm"), 5},
            {H("brdrwavy"), 6}, {H("brdrwavydb"), 6}, {H("brdrtriple"), 7}, {H("brdrdashd"), 8}, {H("brdrdashdd"), 9}, {H("brdrinset"), 10}, {H("brdroutset"), 11},
            {H("brdrnone"), 0}, {H("brdrnil"), 0}, {H("brdrthtnsg"), 3}, {H("brdrthtnmg"), 3}, {H("brdrthtnlg"), 3}, {H("brdrtnthsg"), 3}, {H("brdrtnthmg"), 3},
            {H("brdrtnthlg"), 3}, {H("brdrthtnthsg"), 7}, {H("brdrthtnthmg"), 7}, {H("brdrthtnthlg"), 7}, {H("brdrengrave"), 10}, {H("brdremboss"), 11},
            {H("brdrdashdotstr"), 5}, {H("brdrframe"), 1}};
        *handled = false;
        for (const auto &k : kinds)
            if (k.h == h) {
                setBorderStyle(k.style);
                *handled = true;
                break;
            }
        break;
    }
    }
}

void Reader::tableWord(State &g, uint32_t h, bool has, int param, bool *handled)
{
    *handled = true;
    Story &s = story();
    RowDef &d = rowDefFor(g);
    auto cellBorder = [&](int side) {
        g.bsel = side;
        g.btarget = 1;
    };
    auto rowBorder = [&](int side) {
        g.bsel = side;
        g.btarget = 2;
    };
    switch (h) {
    case H("trowd"):
        d = RowDef();
        m_pendCell = CellDef();
        g.bsel = -1;
        break;
    case H("cellx"):
        if (has && d.cells.size() < kMaxRowCells) {
            m_pendCell.right = std::clamp(param, -1000000, 1000000);
            d.cells.append(m_pendCell);
        }
        m_pendCell = CellDef();
        break;
    case H("trgaph"): d.gaph = std::clamp(param, 0, 31680); break;
    case H("trleft"): d.left = std::clamp(param, -31680, 31680); break;
    case H("trrh"): d.height = std::clamp(param, -31680, 31680); break;
    case H("trhdr"): d.header = true; break;
    case H("trkeep"): d.cantSplit = true; break;
    case H("trqc"): d.jc = 1; break;
    case H("trqr"): d.jc = 2; break;
    case H("trql"): d.jc = 0; break;
    case H("trcbpat"): d.shd = param; break;
    case H("clbrdrt"): cellBorder(0); break;
    case H("clbrdrl"): cellBorder(1); break;
    case H("clbrdrb"): cellBorder(2); break;
    case H("clbrdrr"): cellBorder(3); break;
    case H("trbrdrt"): rowBorder(0); break;
    case H("trbrdrl"): rowBorder(1); break;
    case H("trbrdrb"): rowBorder(2); break;
    case H("trbrdrr"): rowBorder(3); break;
    case H("trbrdrh"): rowBorder(4); break;
    case H("trbrdrv"): rowBorder(5); break;
    case H("clcbpat"): m_pendCell.shd = param; break;
    case H("clmgf"): m_pendCell.mergeFirst = true; break;
    case H("clmrg"): m_pendCell.mergeCont = true; break;
    case H("clvmgf"): m_pendCell.vFirst = true; break;
    case H("clvmrg"): m_pendCell.vCont = true; break;
    case H("clvertalt"): m_pendCell.valign = 0; break;
    case H("clvertalc"): m_pendCell.valign = 1; break;
    case H("clvertalb"): m_pendCell.valign = 2; break;
    case H("cell"): cellEnd(g, 1); break;
    case H("nestcell"): cellEnd(g, nestLevel(g)); break;
    case H("row"): rowEnd(g, 1); break;
    case H("nestrow"): rowEnd(g, nestLevel(g)); break;
    default: *handled = false; break;
    }
    (void)s;
}

void Reader::sectionWord(State &g, uint32_t h, bool has, int param, bool *handled)
{
    *handled = true;
    auto both = [&](int Section::*field, int v) {
        m_docSec.*field = v;
        m_sec.*field = v;
    };
    switch (h) {
    case H("sectd"): m_sec = m_docSec; break;
    case H("sect"): if (m_stories.empty() && g.dest == Dest::Normal) sectionEnd(g); break;
    case H("paperw"): if (has) both(&Section::pageW, param); break;
    case H("paperh"): if (has) both(&Section::pageH, param); break;
    case H("margl"): if (has) both(&Section::left, param); break;
    case H("margr"): if (has) both(&Section::right, param); break;
    case H("margt"): if (has) both(&Section::top, param); break;
    case H("margb"): if (has) both(&Section::bottom, param); break;
    case H("gutter"): if (has) both(&Section::gutter, param); break;
    case H("pgwsxn"): if (has) m_sec.pageW = param; break;
    case H("pghsxn"): if (has) m_sec.pageH = param; break;
    case H("marglsxn"): if (has) m_sec.left = param; break;
    case H("margrsxn"): if (has) m_sec.right = param; break;
    case H("margtsxn"): if (has) m_sec.top = param; break;
    case H("margbsxn"): if (has) m_sec.bottom = param; break;
    case H("guttersxn"): if (has) m_sec.gutter = param; break;
    case H("headery"): if (has) m_sec.header = param; break;
    case H("footery"): if (has) m_sec.footer = param; break;
    case H("landscape"): m_docSec.landscape = true; m_sec.landscape = true; break;
    case H("lndscpsxn"): m_sec.landscape = true; break;
    case H("cols"): if (has) m_sec.cols = std::clamp(param, 1, 45); break;
    case H("colsx"): if (has) m_sec.colSpace = param; break;
    case H("linebetcol"): m_sec.lineBetween = true; break;
    case H("sbknone"): m_sec.type = 0; break;
    case H("sbkcol"): m_sec.type = 1; break;
    case H("sbkpage"): m_sec.type = 2; break;
    case H("sbkeven"): m_sec.type = 3; break;
    case H("sbkodd"): m_sec.type = 4; break;
    case H("titlepg"): m_sec.titlePage = true; break;
    case H("pgnstart"): if (has) m_sec.pageStart = param; break;
    case H("pgnrestart"): if (m_sec.pageStart == kUnset) m_sec.pageStart = 1; break;
    case H("pgndec"): m_sec.pageFormat = 0; break;
    case H("pgnucrm"): m_sec.pageFormat = 1; break;
    case H("pgnlcrm"): m_sec.pageFormat = 2; break;
    case H("pgnucltr"): m_sec.pageFormat = 3; break;
    case H("pgnlcltr"): m_sec.pageFormat = 4; break;
    case H("vertalt"): m_sec.vAlign = 0; break;
    case H("vertalc"): m_sec.vAlign = 1; break;
    case H("vertalb"): m_sec.vAlign = 2; break;
    case H("vertalj"): m_sec.vAlign = 3; break;
    case H("facingp"): t.facingPages = true; break;
    case H("deftab"): if (has && param > 0) t.defaultTab = param; break;
    case H("deff"): if (has) t.defaultFont = param; break;
    case H("deflang"): if (has) t.defaultLang = param; break;
    case H("ansicpg"): if (has && param > 0) m_docCp = param; break;
    case H("mac"): m_docCp = 10000; break;
    case H("pc"): m_docCp = 437; break;
    case H("pca"): m_docCp = 850; break;
    case H("ansi"): m_docCp = 1252; break;
    default: *handled = false; break;
    }
}

void Reader::word(uint32_t h, const char *name, int len, bool has, int param)
{
    (void)name;
    (void)len;
    State &g = st();
    if (h != H("u")) {
        flushLead();
        m_ucSkip = 0;
    }
    if (m_star) {
        m_star = false;
        switch (h) {
        case H("revtbl"): case H("oldcprops"): case H("oldpprops"): case H("fldinst"): case H("bkmkstart"): case H("bkmkend"): case H("listtable"): case H("listoverridetable"): case H("pn"): case H("nesttableprops"): case H("shpinst"):
        case H("shppict"): case H("cs"): case H("ts"): case H("pntxtb"): case H("pntxta"): case H("txbxcontent"): case H("footnote"):
            break;
        default:
            if (g.dest != Dest::Skip) {
                g.dest = Dest::Skip;
                g.owns = true;
            }
            return;
        }
    }
    if (g.dest == Dest::Skip) return;
    if (h == H("u")) {
        if (!has) return;
        int v = param;
        if (v < 0) v += 65536;
        const int uc = g.uc;
        if (m_ucSkip > 0) m_ucSkip = 0;
        flushLead();
        if (g.dest != Dest::Pict && g.dest != Dest::ColorTbl) {
            if (g.dest == Dest::LevelText) addLevelRaw(v & 0xFFFF);
            else routeText(QString(QChar(ushort(v & 0xFFFF))));
        }
        m_ucSkip = uc;
        return;
    }
    if (h == H("uc")) {
        if (has) g.uc = std::clamp(param, 0, 8);
        return;
    }
    bool handled = false;
    switch (g.dest) {
    case Dest::FontTbl:
        switch (h) {
        case H("f"): if (has) m_font.index = param; m_fontPending = true; return;
        case H("fnil"): m_font.family = 0; return;
        case H("froman"): m_font.family = 1; return;
        case H("fswiss"): m_font.family = 2; return;
        case H("fmodern"): m_font.family = 3; return;
        case H("fscript"): m_font.family = 4; return;
        case H("fdecor"): m_font.family = 5; return;
        case H("fcharset"): m_font.charset = param; if (!m_font.codepage) m_font.codepage = codepageOfCharset(param); return;
        case H("fprq"): m_font.prq = param; return;
        case H("cpg"): if (has) m_font.codepage = param; return;
        case H("u"): break;
        default: break;
        }
        if (h != H("u")) {
            bool d = false;
            destWord(g, h, has, param, &d);
            return;
        }
        break;
    case Dest::ColorTbl:
        switch (h) {
        case H("red"): m_red = param; m_colorPending = true; return;
        case H("green"): m_green = param; m_colorPending = true; return;
        case H("blue"): m_blue = param; m_colorPending = true; return;
        default: return;
        }
    case Dest::Pict:
        if (h == H("bin")) break;
        pictWord(h, param);
        return;
    case Dest::StyleSheet:
        return;
    case Dest::StyleDef:
        switch (h) {
        case H("s"): m_style.kind = 0; m_style.id = param; return;
        case H("cs"): m_style.kind = 1; m_style.id = param; return;
        case H("ts"): m_style.kind = 2; m_style.id = param; return;
        case H("sbasedon"): m_style.basedOn = param == 222 ? kUnset : param; return;
        case H("snext"): m_style.next = param; return;
        case H("shidden"): case H("ssemihidden"): m_style.hidden = true; return;
        case H("additive"): case H("sautoupd"): case H("slink"): case H("slocked"): case H("sqformat"): case H("spriority"): case H("sunhideused"): case H("styrsid"): return;
        default: break;
        }
        break;
    case Dest::ListTable: case Dest::ListOverrideTable: case Dest::ListDef: case Dest::ListOverride: case Dest::LfoLevel: case Dest::LevelText:
        {
            bool d = false;
            destWord(g, h, has, param, &d);
            if (d) return;
            listWord(g, h, has, param);
        }
        return;
    case Dest::ListLevel: {
        bool d = false;
        destWord(g, h, has, param, &d);
        if (d) return;
        listWord(g, h, has, param);
        break;
    }
    case Dest::Pn:
        switch (h) {
        case H("pnlvlblt"): m_pn.bullet = true; return;
        case H("pnlvlbody"): case H("pnlvlcont"): return;
        case H("pndec"): m_pn.nfc = 0; return;
        case H("pnucrm"): m_pn.nfc = 1; return;
        case H("pnlcrm"): m_pn.nfc = 2; return;
        case H("pnucltr"): m_pn.nfc = 3; return;
        case H("pnlcltr"): m_pn.nfc = 4; return;
        case H("pnord"): m_pn.nfc = 5; return;
        case H("pnordt"): m_pn.nfc = 7; return;
        case H("pnstart"): m_pn.start = param; return;
        case H("pnindent"): m_pn.indent = std::clamp(param, 0, 31680); return;
        case H("pnf"): m_pn.font = param; g.chr.font = param; return;
        case H("pntxtb"): g.dest = Dest::PnText; g.owns = true; m_pnAfter = false; m_name.clear(); return;
        case H("pntxta"): g.dest = Dest::PnText; g.owns = true; m_pnAfter = true; m_name.clear(); return;
        default: return;
        }
    case Dest::PnText: case Dest::ShapeProp: case Dest::SpName: case Dest::SpValue:
        destWord(g, h, has, param, &handled);
        return;
    case Dest::Shape:
        shapeWord(h, param);
        return;
    default:
        break;
    }
    destWord(g, h, has, param, &handled);
    if (handled) return;
    charWord(g, h, has, param, &handled);
    if (handled) return;
    parWord(g, h, has, param, &handled);
    if (handled) return;
    tableWord(g, h, has, param, &handled);
    if (handled) return;
    sectionWord(g, h, has, param, &handled);
    if (handled) return;
    // The characters and breaks.
    switch (h) {
    case H("par"):
        if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote || g.dest == Dest::Field) {
            if (!m_stopped) endParagraph(g, std::clamp(g.par.itap, 0, kMaxTableDepth), true);
        }
        break;
    case H("tab"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote || g.dest == Dest::Field) { if (room()) addRaw(story(), "<w:tab/>", g.chr); } break;
    case H("line"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote || g.dest == Dest::Field) { if (room()) addRaw(story(), "<w:br/>", g.chr); } break;
    case H("page"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote || g.dest == Dest::Field) { if (room()) addRaw(story(), "<w:br w:type=\"page\"/>", g.chr); } break;
    case H("column"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote || g.dest == Dest::Field) { if (room()) addRaw(story(), "<w:br w:type=\"column\"/>", g.chr); } break;
    case H("emdash"): routeText(QString(QChar(0x2014))); break;
    case H("endash"): routeText(QString(QChar(0x2013))); break;
    case H("emspace"): routeText(QString(QChar(0x2003))); break;
    case H("enspace"): routeText(QString(QChar(0x2002))); break;
    case H("qmspace"): routeText(QString(QChar(0x2005))); break;
    case H("bullet"): routeText(QString(QChar(0x2022))); break;
    case H("lquote"): routeText(QString(QChar(0x2018))); break;
    case H("rquote"): routeText(QString(QChar(0x2019))); break;
    case H("ldblquote"): routeText(QString(QChar(0x201C))); break;
    case H("rdblquote"): routeText(QString(QChar(0x201D))); break;
    case H("zwj"): routeText(QString(QChar(0x200D))); break;
    case H("zwnj"): routeText(QString(QChar(0x200C))); break;
    case H("ltrmark"): routeText(QString(QChar(0x200E))); break;
    case H("rtlmark"): routeText(QString(QChar(0x200F))); break;
    case H("chpgn"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote) { if (room(4)) addField("PAGE", QStringLiteral("1")); } break;
    case H("chdate"): case H("chdpl"): case H("chdpa"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote) { if (room(4)) addField("DATE", QString()); } break;
    case H("chtime"): if (g.dest == Dest::Normal || g.dest == Dest::Header || g.dest == Dest::Footnote) { if (room(4)) addField("TIME", QString()); } break;
    case H("chftn"):
        if (g.dest == Dest::Footnote && !m_stopped) {
            Chr c = g.chr;
            c.vert = 1;
            addRaw(story(), story().endnote ? "<w:endnoteRef/>" : "<w:footnoteRef/>", c);
        }
        break;
    case H("ftnalt"):
        if (g.dest == Dest::Footnote && !m_stories.empty()) {
            m_stories.back()->endnote = true;
            m_stories.back()->part = QStringLiteral("/word/endnotes.xml");
        }
        break;
    default:
        break;
    }
}

void Reader::control()
{
    ++m_pos;
    if (m_pos >= m_n) return;
    const unsigned char c = uchar(m_d[m_pos]);
    const bool inGroup = m_stack.size() > 1 && m_overflow == 0;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        const qint64 start = m_pos;
        while (m_pos < m_n && ((uchar(m_d[m_pos]) | 0x20) >= 'a' && (uchar(m_d[m_pos]) | 0x20) <= 'z')) ++m_pos;
        const int len = int(std::min<qint64>(m_pos - start, 32));
        bool has = false;
        qint64 param = 0;
        bool neg = false;
        if (m_pos + 1 < m_n && m_d[m_pos] == '-' && m_d[m_pos + 1] >= '0' && m_d[m_pos + 1] <= '9') {
            neg = true;
            ++m_pos;
        }
        while (m_pos < m_n && m_d[m_pos] >= '0' && m_d[m_pos] <= '9') {
            has = true;
            if (param < 100000000000LL) param = param * 10 + (m_d[m_pos] - '0');
            ++m_pos;
        }
        if (m_pos < m_n && m_d[m_pos] == ' ') ++m_pos;
        const int p = int(std::clamp<qint64>(neg ? -param : param, -2000000000LL, 2000000000LL));
        // (A word is acted on only if its letters are one this reader knows, not just its hash: another word with the same hash is no word of the format's.)
        const uint32_t h = isKnownWord(m_d + start, len) ? hashOf(m_d + start, len) : 0u;
        if (h == H("bin") && has && p > 0) {
            // Binary data follows: that many bytes, whatever they are.
            const qint64 take = std::min<qint64>(p, m_n - m_pos);
            if (inGroup && st().dest == Dest::Pict && !m_stopped) {
                if (m_pict.data.size() + take > kMaxPictureBytes) m_pict.tooBig = true;
                else m_pict.data.append(m_d + m_pos, int(take));
            }
            m_pos += take;
            return;
        }
        if (inGroup) word(h, m_d + start, len, has, p);
        return;
    }
    ++m_pos;
    switch (c) {
    case '\'': {
        auto hex = [](char ch) { return ch >= '0' && ch <= '9' ? ch - '0' : (ch | 0x20) >= 'a' && (ch | 0x20) <= 'f' ? (ch | 0x20) - 'a' + 10 : -1; };
        if (m_pos + 1 < m_n && hex(m_d[m_pos]) >= 0 && hex(m_d[m_pos + 1]) >= 0) {
            const uchar b = uchar(hex(m_d[m_pos]) * 16 + hex(m_d[m_pos + 1]));
            m_pos += 2;
            if (inGroup) {
                if (st().dest == Dest::Pict) return;
                if (st().dest == Dest::ColorTbl) return;
                onByte(b);
            }
        }
        break;
    }
    case '\\': case '{': case '}':
        if (inGroup) {
            if (m_ucSkip > 0) {
                --m_ucSkip;
                break;
            }
            if (st().dest == Dest::FontTbl || st().dest == Dest::StyleDef || st().dest == Dest::Normal || st().dest == Dest::Header || st().dest == Dest::Footnote || st().dest == Dest::Field ||
                st().dest == Dest::FldInst || st().dest == Dest::BkmkStart || st().dest == Dest::BkmkEnd || st().dest == Dest::LevelText || st().dest == Dest::PnText)
                routeText(QString(QChar(c)));
        }
        break;
    case '~': if (inGroup) { flushLead(); routeText(QString(QChar(0xA0))); } break;
    case '-': if (inGroup) { flushLead(); routeText(QString(QChar(0xAD))); } break;
    case '_': if (inGroup) { flushLead(); routeText(QString(QChar(0x2011))); } break;
    case '*': if (inGroup) m_star = true; break;
    case '\r': case '\n':
        if (inGroup) word(H("par"), "par", 3, false, 0);
        break;
    default: break;
    }
}

QByteArray Reader::build(const State &g)
{
    // The body's last paragraph, and the last section.
    Story &s = m_main;
    flushText(s);
    if (s.hasContent()) endParagraph(g, g.par.itap > 0 ? std::min(g.par.itap, kMaxTableDepth) : 0, true);
    closeTables(s);
    flushHeld(s);
    QByteArray refs;
    {
        static const char *const kinds[] = {"header", "header", "header", "footer", "footer", "footer"};
        static const char *const types[] = {"default", "first", "even", "default", "first", "even"};
        for (int i = 0; i < 6; ++i)
            if (!m_secRef[i].isEmpty()) refs += QByteArray("<w:") + kinds[i] + "Reference w:type=\"" + types[i] + "\" r:id=\"" + m_secRef[i].toUtf8() + "\"/>";
    }
    const QString mainName = QStringLiteral("/word/document.xml");
    const QString rel = QString::fromLatin1(kRel), wml = QString::fromLatin1(kWml);
    pkg.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), mainName);
    const QByteArray styles = stylesXml(t);
    spend(styles.size());
    pkg.addPart(QStringLiteral("/word/styles.xml"), styles, wml + QStringLiteral("styles+xml"));
    pkg.addRelationship(mainName, rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    const QByteArray numbering = numberingXml(t);
    if (!numbering.isEmpty() && spend(numbering.size())) {
        pkg.addPart(QStringLiteral("/word/numbering.xml"), numbering, wml + QStringLiteral("numbering+xml"));
        pkg.addRelationship(mainName, rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
    Theme theme = defaultTheme();
    theme.colors[Theme::Dark1] = QColor(0, 0, 0);
    theme.colors[Theme::Light1] = QColor(255, 255, 255);
    theme.colors[Theme::Hyperlink] = QColor(0, 0, 255);
    theme.colors[Theme::FollowedHyperlink] = QColor(128, 0, 128);
    pkg.addPart(QStringLiteral("/word/theme/theme1.xml"), writeTheme(theme), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    pkg.addRelationship(mainName, rel + QStringLiteral("theme"), QStringLiteral("/word/theme/theme1.xml"));
    {
        QByteArray x = QByteArray(kXmlHeader) + "<w:settings " + kNamespaces + ">";
        if (t.facingPages) x += "<w:evenAndOddHeaders/>";
        x += "<w:defaultTabStop w:val=\"" + num(std::clamp(t.defaultTab, 1, 31680)) + "\"/>";
        x += "<w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"11\"/></w:compat></w:settings>";
        pkg.addPart(QStringLiteral("/word/settings.xml"), x, wml + QStringLiteral("settings+xml"));
        pkg.addRelationship(mainName, rel + QStringLiteral("settings"), QStringLiteral("/word/settings.xml"));
    }
    if (!t.fonts.isEmpty()) {
        pkg.addPart(QStringLiteral("/word/fontTable.xml"), fontTableXml(t), wml + QStringLiteral("fontTable+xml"));
        pkg.addRelationship(mainName, rel + QStringLiteral("fontTable"), QStringLiteral("/word/fontTable.xml"));
    }
    for (int kind = 0; kind < 2; ++kind) {
        if (m_noteCount[kind] == 0) continue;
        const char *tag = kind ? "w:endnote" : "w:footnote";
        const QString name = kind ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
        QByteArray x = QByteArray(kXmlHeader) + "<" + tag + "s " + kNamespaces + ">";
        x += QByteArray("<") + tag + " w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></" + tag + ">";
        x += QByteArray("<") + tag + " w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></" + tag + ">";
        x += m_notesXml[kind] + "</" + tag + "s>";
        pkg.addPart(name, x, wml + (kind ? QStringLiteral("endnotes+xml") : QStringLiteral("footnotes+xml")));
        pkg.addRelationship(mainName, rel + (kind ? QStringLiteral("endnotes") : QStringLiteral("footnotes")), name);
    }
    const QByteArray sect = "<w:sectPr>" + sectionPropertiesXml(m_sec, refs) + "</w:sectPr>";
    pkg.addPart(mainName, QByteArray(kXmlHeader) + "<w:document " + kNamespaces + "><w:body>" + (s.body.isEmpty() ? QByteArray("<w:p/>") : s.body) + sect + "</w:body></w:document>",
                wml + QStringLiteral("document.main+xml"));
    return pkg.finish();
}

QByteArray Reader::run(QString *error)
{
    m_stack.append(State());
    m_main.part = QStringLiteral("/word/document.xml");
    // (The styles a paragraph has when it says none.)
    while (m_pos < m_n && !m_done) {
        const char c = m_d[m_pos];
        if (c == '{') {
            ++m_pos;
            m_started = true;
            pushGroup();
        } else if (c == '}') {
            ++m_pos;
            popGroup();
        } else if (c == '\\') {
            control();
        } else if (c == '\r' || c == '\n') {
            ++m_pos;
        } else {
            const qint64 start = m_pos;
            while (m_pos < m_n && m_d[m_pos] != '{' && m_d[m_pos] != '}' && m_d[m_pos] != '\\' && m_d[m_pos] != '\r' && m_d[m_pos] != '\n') ++m_pos;
            if (m_stack.size() > 1 && m_overflow == 0) {
                // (Text a destination reads as it comes; a long stretch is cut so that a name or an instruction can not grow without bound.)
                qint64 p = start;
                while (p < m_pos) {
                    const int n = int(std::min<qint64>(m_pos - p, 1 << 16));
                    onTextChars(m_d + p, n);
                    p += n;
                }
            }
        }
        if (m_stopped && m_stack.size() > 1) {
            // The limit: nothing more is made, but the groups still close.
            m_pos = m_n;
        }
    }
    if (!m_started) {
        if (error) *error = QCoreApplication::translate("jp::office::rtf", "The file is not a Rich Text Format document.");
        return QByteArray();
    }
    State last = m_stack.size() > 1 ? m_stack.back() : m_lastClosed;
    // The groups a damaged file leaves open.
    while (m_stack.size() > 1) {
        last = m_stack.back();
        popGroup();
    }
    if (m_done) last = m_lastClosed;
    // Stories still open (a header or note group the file did not close) are dropped.
    return build(last);
}

} // namespace

thread_local qint64 g_lastOutputBytes = 0;
thread_local ReadWork g_lastWork;

ReadWork lastWorkForTest() { return g_lastWork; }

qint64 lastOutputBytesForTest() { return g_lastOutputBytes; }

bool isKnownWord(const char *letters, int length)
{
    static const QSet<QByteArray> words = [] {
        QSet<QByteArray> w;
        for (const char *k : kKnownWords) w.insert(QByteArray(k));
        return w;
    }();
    return length > 0 && length <= 32 && words.contains(QByteArray(letters, length));
}

bool looksLikeRtf(const QByteArray &bytes)
{
    qsizetype i = 0;
    if (bytes.startsWith("\xEF\xBB\xBF")) i = 3;
    while (i < bytes.size() && i < 64 && (bytes[i] == ' ' || bytes[i] == '\r' || bytes[i] == '\n' || bytes[i] == '\t')) ++i;
    return bytes.size() - i >= 5 && bytes.mid(i, 5) == "{\\rtf";
}

QByteArray toDocxPackage(const QByteArray &bytes, QString *error, wml::ReadNotes *notes)
{
    if (bytes.size() > kMaxFileBytes) {
        if (error) *error = QCoreApplication::translate("jp::office::rtf", "The file is too large.");
        return QByteArray();
    }
    if (!looksLikeRtf(bytes)) {
        if (error) *error = QCoreApplication::translate("jp::office::rtf", "The file is not a Rich Text Format document.");
        return QByteArray();
    }
    Reader r(bytes, notes);
    const QByteArray out = r.run(error);
    g_lastOutputBytes = r.outputBytes();
    g_lastWork = r.work();
    return out;
}

bool readRtf(const QByteArray &bytes, wml::Document *doc, QString *error, wml::ReadNotes *notes)
{
    wml::ReadNotes local;
    wml::ReadNotes *use = notes ? notes : &local;
    const QByteArray zip = toDocxPackage(bytes, error, use);
    if (zip.isEmpty()) return false;
    return wml::readDocx(zip, doc, error, use);
}

} // namespace jp::office::rtf
