#include "sheet/xls.h"

#include "sheet/cellref.h"
#include "sheet/xlsbytes.h"
#include "sheet/xlscodepage.h"
#include "sheet/xlsfunctions.h"

#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>

namespace jp::office::sheet {

namespace {

QString translate(const char *text) { return QCoreApplication::translate("Sheet", text); }

// The records the readers look for.
enum : quint16 {
    kBof = 0x0809,
    kEof = 0x000A,
    kContinue = 0x003C,
    kCodepage = 0x0042,
    kDateMode = 0x0022,
    kCalcMode = 0x000D,
    kIteration = 0x0011,
    kFilePass = 0x002F,
    kFont = 0x0031,
    kFormat = 0x041E,
    kXf = 0x00E0,
    kPalette = 0x0092,
    kBoundSheet = 0x0085,
    kSst = 0x00FC,
    kSupBook = 0x01AE,
    kExternCount = 0x0016,
    kExternSheet = 0x0017,
    kExternName = 0x0023,
    kName = 0x0018,
    kWindow1 = 0x003D,
    kMsoDrawingGroup = 0x00EB,
    kMsoDrawing = 0x00EC,
    kObj = 0x005D,
    kNote = 0x001C,
    kSxView = 0x00B0,
    kDimensions = 0x0200,
    kNumber = 0x0203,
    kRk = 0x027E,
    kMulRk = 0x00BD,
    kLabelSst = 0x00FD,
    kLabel = 0x0204,
    kRString = 0x00D6,
    kBlank = 0x0201,
    kMulBlank = 0x00BE,
    kBoolErr = 0x0205,
    kFormula = 0x0006,
    kString = 0x0207,
    kShrFmla = 0x04BC,
    kArray = 0x0221,
    kTable = 0x0236,
    kRow = 0x0208,
    kColInfo = 0x007D,
    kDefColWidth = 0x0055,
    kStandardWidth = 0x0099,
    kDefaultRowHeight = 0x0225,
    kMergedCells = 0x00E5,
    kWindow2 = 0x023E,
    kPane = 0x0041,
    kScl = 0x00A0,
    kSelection = 0x001D,
};

// The most nested substreams (charts inside a sheet) a sheet may hold: real files have one deep.
constexpr int kMaxSubstreamDepth = 64;

struct Rec {
    quint16 id = 0;
    qsizetype pos = 0;   // where the bytes of the record begin
    qsizetype len = 0;
};

// The records of a stream, one after another. A record cut short by the end of the stream is given with the bytes there are.
class Scanner {
public:
    Scanner(const QByteArray &d, qsizetype at) : m_d(d), m_o(at) {}
    bool next(Rec *r)
    {
        if (m_o < 0 || m_o + 4 > m_d.size()) return false;
        Bytes b(m_d, m_o, m_o + 4);
        r->id = b.u16();
        const qsizetype len = b.u16();
        r->pos = m_o + 4;
        r->len = std::min<qsizetype>(len, m_d.size() - r->pos);
        m_o = r->pos + r->len;
        return true;
    }
    // The id of the next record, or false at the end.
    bool peek(Rec *r) const
    {
        Scanner copy = *this;
        return copy.next(r);
    }
    qsizetype offset() const { return m_o; }

private:
    const QByteArray &m_d;
    qsizetype m_o;
};

// A record with the records that continue it joined, and where each piece ends.
struct Logical {
    QByteArray data;
    QVector<qsizetype> ends;
};

Logical joined(const QByteArray &d, Scanner &sc, const Rec &first, bool joinContinues)
{
    Logical lg;
    lg.data = d.mid(first.pos, first.len);
    lg.ends << lg.data.size();
    Rec c;
    while (joinContinues && sc.peek(&c) && c.id == kContinue) {
        sc.next(&c);
        lg.data += d.mid(c.pos, c.len);
        lg.ends << lg.data.size();
    }
    return lg;
}

// Characters of a string that may be continued in the next piece: the continuation of a string begins with a byte that says whether its characters take one byte
// or two.
class Chars {
public:
    explicit Chars(const Logical &lg, qsizetype at = 0) : m_lg(lg), m_o(at) { settleSegment(); }
    qsizetype at() const { return m_o; }
    void skip(qint64 n) { m_o = qsizetype(std::min<qint64>(qint64(m_o) + n, m_lg.data.size())); settleSegment(); }
    qsizetype left() const { return m_lg.data.size() - m_o; }
    const QByteArray &data() const { return m_lg.data; }
    bool ok() const { return m_ok; }

    quint8 u8()
    {
        if (m_o + 1 > m_lg.data.size()) return quint8(bad());
        const quint8 v = quint8(m_lg.data[m_o++]);
        settleSegment();
        return v;
    }
    quint16 u16() { const quint8 a = u8(); return quint16(a | (quint16(u8()) << 8)); }
    quint32 u32() { const quint32 a = u16(); return a | (quint32(u16()) << 16); }

    // `count` characters, in one byte or two each. Latin-1 for one byte unless a converter is given.
    QString chars(qint64 count, bool high, const std::function<QString(const char *, int)> &single = nullptr)
    {
        QString out;
        while (count > 0) {
            if (m_o >= m_lg.data.size()) {
                bad();
                return out;
            }
            qsizetype avail = m_segEnd - m_o;
            if (avail <= 0) {
                // the characters go on in the next piece, which begins with the byte that says how they are written
                if (m_seg + 1 >= m_lg.ends.size()) {
                    bad();
                    return out;
                }
                ++m_seg;
                m_segEnd = m_lg.ends[m_seg];
                const quint8 flags = quint8(m_lg.data[m_o]);
                m_o += 1;
                high = flags & 1;
                continue;
            }
            const qint64 per = high ? 2 : 1;
            qint64 n = std::min<qint64>(count, avail / per);
            if (n <= 0) {
                // (a half character at the end of a piece: damaged)
                bad();
                return out;
            }
            const char *p = m_lg.data.constData() + m_o;
            if (high) out += QString::fromUtf16(reinterpret_cast<const char16_t *>(p), int(n));   // (little endian machines only; the format is little endian)
            else out += single ? single(p, int(n)) : QString::fromLatin1(p, int(n));
            m_o += qsizetype(n * per);
            count -= n;
        }
        return out;
    }

private:
    int bad()
    {
        m_ok = false;
        return 0;
    }
    // Which piece the position is in.
    void settleSegment()
    {
        while (m_seg < m_lg.ends.size() && m_o > m_lg.ends[m_seg] && m_seg + 1 < m_lg.ends.size()) ++m_seg;
        m_segEnd = m_seg < m_lg.ends.size() ? m_lg.ends[m_seg] : m_lg.data.size();
    }
    const Logical &m_lg;
    qsizetype m_o;
    int m_seg = 0;
    qsizetype m_segEnd = 0;
    bool m_ok = true;
};

Logical single(const QByteArray &d, const Rec &r)
{
    Logical lg;
    lg.data = d.mid(r.pos, r.len);
    lg.ends << lg.data.size();
    return lg;
}

// What the format spells out in a few bits.
CellError cellErrorFromCode(int code)
{
    switch (code) {
    case 0x00: return CellError::Null;
    case 0x07: return CellError::Div0;
    case 0x0F: return CellError::Value;
    case 0x17: return CellError::Ref;
    case 0x1D: return CellError::Name;
    case 0x24: return CellError::Num;
    case 0x2A: return CellError::NA;
    case 0x2B: return CellError::GettingData;
    }
    return CellError::Unknown;
}

double rkValue(quint32 rk)
{
    double v;
    if (rk & 2) {
        v = double(qint32(rk) >> 2);
    } else {
        const quint64 bits = quint64(rk & 0xFFFFFFFC) << 32;
        std::memcpy(&v, &bits, 8);
    }
    return (rk & 1) ? v / 100.0 : v;
}

// The cell format that is the default look is the 16th in the file (the first 15 are the formats of styles); here it is the first, as in the XML form, so that a
// cell with no look of its own has style 0, and the first format of the file takes its place.
quint32 mapXf(quint32 xf, qsizetype count)
{
    if (count <= 15) return xf;
    if (xf == 15) return 0;
    if (xf == 0) return 15;
    return xf;
}

struct RawXf {
    quint16 font = 0, format = 0;
    bool style = false;
    quint8 halign = 0, valign = 2;
    bool wrap = false, shrink = false;
    quint8 rotation = 0, indent = 0;
    quint8 left = 0, right = 0, top = 0, bottom = 0, diagonal = 0;
    quint16 colorLeft = 64, colorRight = 64, colorTop = 64, colorBottom = 64, colorDiagonal = 64;
    bool diagUp = false, diagDown = false;
    quint8 pattern = 0;
    quint16 fore = 64, back = 65;
};

ColorSpec indexedColor(int icv)
{
    ColorSpec c;
    if (icv == 0x7FFF) {
        c.kind = ColorSpec::Kind::Auto;
    } else {
        c.kind = ColorSpec::Kind::Indexed;
        c.index = icv;
    }
    return c;
}

struct RawName {
    QString text;
    int localSheet = -1;
    bool hidden = false;
    QByteArray rgce, extra;
};

const char *const kBuiltinNames[] = {"Consolidate_Area", "Auto_Open", "Auto_Close", "Extract", "Database", "Criteria", "Print_Area", "Print_Titles",
                                     "Cell_Contents", "Recorder", "Data_Form", "Auto_Activate", "Auto_Deactivate", "Sheet_Title", "_FilterDatabase"};

} // namespace

bool startsCompoundFile(const QByteArray &head) { return head.size() >= 8 && head.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1")); }

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the globals

class XlsGlobals {
public:
    XlsGlobals(const QByteArray &stream, const Limits &limits) : m_d(stream), m_lim(limits) {}

    std::shared_ptr<const XlsBook> run(bool hasMacros, WorkbookInfo *info, SharedStrings *shared, Styles *styles, QString *error)
    {
        auto fail = [&](const QString &why) -> std::shared_ptr<const XlsBook> {
            if (error) *error = why;
            return nullptr;
        };
        Scanner sc(m_d, 0);
        Rec r;
        if (!sc.next(&r)) return fail(translate("The file has no workbook in it."));
        if (r.id != kBof) {
            if (r.id == 0x0009 || r.id == 0x0209 || r.id == 0x0409) return fail(translate("This workbook is in a file format older than this program reads."));
            return fail(translate("The file has no workbook in it."));
        }
        {
            Bytes b(m_d, r.pos, r.pos + r.len);
            const quint16 vers = b.u16();
            const quint16 type = b.u16();
            if (vers >= 0x0600) m_biff = 8;
            else if (vers >= 0x0500) m_biff = 5;
            else return fail(translate("This workbook is in a file format older than this program reads."));
            if (type != 0x0005) return fail(translate("The file has no workbook in it."));
        }
        m_cx.biff = m_biff;
        m_cx.bytesToText = [this](const char *p, int n) { return text(p, n); };
        bool done = false;
        qint64 records = 0;
        while (!done && sc.next(&r)) {
            if (++records > 50000000) return fail(translate("The workbook has more records than any real file has."));
            switch (r.id) {
            case kEof: done = true; break;
            case kFilePass: return fail(translate("The workbook is encrypted, and this program can't open an encrypted workbook yet."));
            case kCodepage: {
                Bytes b(m_d, r.pos, r.pos + r.len);
                m_codepage = b.u16();
                break;
            }
            case kDateMode: {
                Bytes b(m_d, r.pos, r.pos + r.len);
                m_date1904 = b.u16() == 1;
                break;
            }
            case kCalcMode: {
                Bytes b(m_d, r.pos, r.pos + r.len);
                const quint16 m = b.u16();
                m_calcMode = m == 0 ? CalcMode::Manual : m == 0xFFFF ? CalcMode::AutoNoTable : CalcMode::Auto;
                break;
            }
            case kIteration: {
                Bytes b(m_d, r.pos, r.pos + r.len);
                m_iterate = b.u16() == 1;
                break;
            }
            case kFont:
                if (!readFont(r)) return fail(translate("The workbook has more fonts than any real file has."));
                break;
            case kFormat:
                if (!readFormat(r)) return fail(translate("The workbook has more number formats than any real file has."));
                break;
            case kXf:
                if (!readXf(r)) return fail(translate("The workbook has more cell formats than any real file has."));
                break;
            case kPalette: readPalette(r); break;
            case kBoundSheet:
                if (!readBoundSheet(r)) return fail(translate("The workbook has more sheets than any real file has."));
                break;
            case kSst: {
                const Logical lg = joined(m_d, sc, r, true);
                if (!readSst(lg)) return fail(translate("The workbook has more strings than any real file has."));
                break;
            }
            case kSupBook: readSupBook(r); break;
            case kExternSheet: readExternSheet(r); break;
            case kExternName: readExternName(r); break;
            case kName: {
                const Logical lg = joined(m_d, sc, r, true);
                if (!readName(lg)) return fail(translate("The workbook has more names than any real file has."));
                break;
            }
            case kWindow1: {
                if (m_haveWindow) break;
                m_haveWindow = true;
                Bytes b(m_d, r.pos, r.pos + r.len);
                b.skip(10);
                m_activeTab = b.u16();
                m_firstTab = b.u16();
                break;
            }
            case kMsoDrawingGroup: m_drawings = true; break;
            default: break;
            }
        }
        if (m_sheets.isEmpty()) return fail(translate("The workbook has no sheets."));
        // each sheet's data: where it begins, and where it ends (so that no two share any)
        if (!placeSheets()) return fail(m_placeProblem);
        return finish(hasMacros, info, shared, styles);
    }

private:
    const QByteArray &m_d;
    const Limits &m_lim;
    int m_biff = 8;
    int m_codepage = 1252;
    bool m_date1904 = false;
    CalcMode m_calcMode = CalcMode::Auto;
    bool m_iterate = false;
    bool m_haveWindow = false;
    int m_activeTab = 0, m_firstTab = 0;
    bool m_drawings = false;
    XlsContext m_cx;
    QVector<FontSpec> m_fonts;
    QHash<int, QString> m_formats;
    qint64 m_formatChars = 0;
    QVector<RawXf> m_xfs;
    QVector<quint32> m_palette;   // from place 8
    SharedStrings m_sst;
    QVector<SheetEntry> m_sheets;
    QVector<XlsBook::Place> m_places;
    QVector<RawName> m_rawNames;
    qint64 m_nameChars = 0;
    int m_externals = 0;
    QString m_placeProblem;
    bool m_charts = false, m_pivots = false, m_comments = false;
    bool m_drawingsInSheets = false;

    QString text(const char *p, int n) const { return xlsBytesToText(m_codepage, p, n); }

    // A string of BIFF8 (a count of one or two bytes, then a byte that says one byte or two for each character) or of BIFF5 (a count and the bytes).
    QString readString(Bytes &b, int countBytes)
    {
        const int n = countBytes == 1 ? b.u8() : b.u16();
        if (m_biff >= 8) {
            const quint8 flags = b.u8();
            const bool high = flags & 1;
            if (flags & 8) b.skip(2);    // runs of rich text
            if (flags & 4) b.skip(4);    // phonetic data
            const char *p = b.take(qint64(n) * (high ? 2 : 1));
            if (!p) return QString();
            QString s = high ? QString::fromUtf16(reinterpret_cast<const char16_t *>(p), n) : QString::fromLatin1(p, n);
            return s;
        }
        const char *p = b.take(n);
        return p ? text(p, n) : QString();
    }

    bool readFont(const Rec &r)
    {
        if (m_fonts.size() >= m_lim.fonts) return false;
        Bytes b(m_d, r.pos, r.pos + r.len);
        FontSpec f;
        const quint16 height = b.u16();
        const quint16 grbit = b.u16();
        const quint16 icv = b.u16();
        const quint16 weight = b.u16();
        const quint16 escapement = b.u16();
        const quint8 underline = b.u8();
        b.skip(3);   // family, character set, reserved
        const int n = b.u8();
        if (b.bad()) {
            m_fonts.append(f);
            return true;
        }
        QString name;
        if (m_biff >= 8) {
            const quint8 flags = b.u8();
            const bool high = flags & 1;
            const char *p = b.take(qint64(n) * (high ? 2 : 1));
            if (p) name = high ? QString::fromUtf16(reinterpret_cast<const char16_t *>(p), n) : QString::fromLatin1(p, n);
        } else {
            const char *p = b.take(n);
            if (p) name = text(p, n);
        }
        f.name = name.left(kMaxFontNameChars);
        f.size = height / 20.0;
        f.bold = weight >= 700;
        f.italic = grbit & 0x02;
        f.strike = grbit & 0x08;
        f.underline = underline == 1 ? Underline::Single : underline == 2 ? Underline::Double : underline == 0x21 ? Underline::SingleAccounting
            : underline == 0x22 ? Underline::DoubleAccounting : Underline::None;
        f.vertAlign = escapement == 1 ? 1 : escapement == 2 ? 2 : 0;
        f.color = indexedColor(icv);
        m_fonts.append(f);
        return true;
    }

    bool readFormat(const Rec &r)
    {
        if (m_formats.size() >= m_lim.numFmts) return false;
        Bytes b(m_d, r.pos, r.pos + r.len);
        const int id = b.u16();
        QString code = readString(b, m_biff >= 8 ? 2 : 1);
        if (b.bad() && code.isEmpty()) return true;
        if (code.size() > 4096) code.truncate(4096);
        m_formatChars += code.size();
        if (m_formatChars > m_lim.numFmtChars) return false;
        m_formats.insert(id, code);
        return true;
    }

    bool readXf(const Rec &r)
    {
        if (m_xfs.size() >= m_lim.cellXfs) return false;
        Bytes b(m_d, r.pos, r.pos + r.len);
        RawXf x;
        x.font = b.u16();
        x.format = b.u16();
        const quint16 type = b.u16();
        x.style = type & 4;
        if (m_biff >= 8) {
            const quint8 align = b.u8();
            x.rotation = b.u8();
            const quint8 indent = b.u8();
            b.skip(1);   // which attributes differ from the style's
            const quint16 borders = b.u16();
            const quint16 colors = b.u16();
            const quint32 more = b.u32();
            const quint16 fill = b.u16();
            x.halign = align & 7;
            x.wrap = align & 8;
            x.valign = (align >> 4) & 7;
            x.indent = indent & 15;
            x.shrink = indent & 16;
            x.left = borders & 15;
            x.right = (borders >> 4) & 15;
            x.top = (borders >> 8) & 15;
            x.bottom = (borders >> 12) & 15;
            x.colorLeft = colors & 0x7F;
            x.colorRight = (colors >> 7) & 0x7F;
            x.diagDown = colors & 0x4000;
            x.diagUp = colors & 0x8000;
            x.colorTop = more & 0x7F;
            x.colorBottom = (more >> 7) & 0x7F;
            x.colorDiagonal = (more >> 14) & 0x7F;
            x.diagonal = (more >> 21) & 15;
            x.pattern = (more >> 26) & 0x3F;
            x.fore = fill & 0x7F;
            x.back = (fill >> 7) & 0x7F;
        } else {
            const quint8 align = b.u8();
            const quint8 orient = b.u8();
            const quint32 area = b.u32();
            const quint32 border = b.u32();
            x.halign = align & 7;
            x.wrap = align & 8;
            x.valign = (align >> 4) & 7;
            const int o = orient & 3;
            x.rotation = o == 1 ? 255 : o == 2 ? 90 : o == 3 ? 180 : 0;
            // (the colors are 7 bits, as in BIFF8; the bottom border is with the fill)
            x.fore = area & 0x7F;
            x.back = (area >> 7) & 0x7F;
            x.pattern = (area >> 16) & 0x3F;
            x.bottom = (area >> 22) & 7;
            x.colorBottom = (area >> 25) & 0x7F;
            x.top = border & 7;
            x.left = (border >> 3) & 7;
            x.right = (border >> 6) & 7;
            x.colorTop = (border >> 9) & 0x7F;
            x.colorLeft = (border >> 16) & 0x7F;
            x.colorRight = (border >> 23) & 0x7F;
        }
        m_xfs.append(x);
        return true;
    }

    void readPalette(const Rec &r)
    {
        Bytes b(m_d, r.pos, r.pos + r.len);
        const int n = b.u16();
        m_palette.clear();
        for (int i = 0; i < n && i < 56 && b.left() >= 4; ++i) {
            const quint8 red = b.u8(), green = b.u8(), blue = b.u8();
            b.skip(1);
            m_palette.append((quint32(red) << 16) | (quint32(green) << 8) | blue);
        }
    }

    bool readBoundSheet(const Rec &r)
    {
        if (m_sheets.size() >= m_lim.sheets) return false;
        Bytes b(m_d, r.pos, r.pos + r.len);
        const quint32 offset = b.u32();
        SheetEntry e;
        int type;
        if (m_biff >= 8) {
            const quint8 state = b.u8();
            type = b.u8();
            e.state = state == 1 ? SheetState::Hidden : state == 2 ? SheetState::VeryHidden : SheetState::Visible;
            e.name = readString(b, 1);
        } else {
            const quint16 grbit = b.u16();
            type = grbit >> 8;
            const quint8 state = grbit & 0xFF;
            e.state = state == 1 ? SheetState::Hidden : state == 2 ? SheetState::VeryHidden : SheetState::Visible;
            e.name = readString(b, 1);
        }
        e.name = e.name.left(kMaxSheetNameChars);
        e.sheetId = int(m_sheets.size()) + 1;
        e.relId = QStringLiteral("xls");
        e.kind = type == 2 ? SheetKind::Chartsheet : type == 1 ? SheetKind::Macrosheet : type == 6 ? SheetKind::Macrosheet : SheetKind::Worksheet;
        if (type == 6) e.state = SheetState::VeryHidden;   // a module of macros is listed with the sheets and is not one
        m_sheets.append(e);
        XlsBook::Place p;
        p.offset = qsizetype(offset);
        p.type = type == 0 ? 0x10 : type == 1 ? 0x40 : type == 2 ? 0x20 : 0x06;
        m_places.append(p);
        m_cx.sheetNames << e.name;
        return true;
    }

    bool readSst(const Logical &lg)
    {
        Chars c(lg, 0);
        c.u32();   // the total of references
        const quint32 unique = c.u32();
        while (c.left() >= 3 && m_sst.strings.size() < qsizetype(unique)) {
            if (m_sst.strings.size() >= m_lim.sharedStrings) return false;
            const int n = c.u16();
            const quint8 flags = c.u8();
            const bool high = flags & 1;
            const int runs = (flags & 8) ? c.u16() : 0;
            const quint32 ext = (flags & 4) ? c.u32() : 0;
            QString s = c.chars(n, high);
            if (!c.ok()) {
                // (damaged: what came before stays)
                break;
            }
            c.skip(qint64(runs) * 4 + ext);
            m_sst.chars += s.size();
            if (m_sst.chars > m_lim.sharedStringChars) return false;
            m_sst.strings.append(s);
        }
        return true;
    }

    void readSupBook(const Rec &r)
    {
        Bytes b(m_d, r.pos, r.pos + r.len);
        XlsSupBook sb;
        if (m_biff < 8) return;
        const int sheets = b.u16();
        const int cch = b.u16();
        if (cch == 0x0401) {
            sb.kind = XlsSupBook::Kind::Internal;
        } else if (cch == 0x3A01) {
            sb.kind = XlsSupBook::Kind::AddIn;
        } else {
            sb.kind = XlsSupBook::Kind::External;
            const quint8 flags = b.u8();
            const bool high = flags & 1;
            const char *p = b.take(qint64(cch) * (high ? 2 : 1));
            if (p) sb.url = high ? QString::fromUtf16(reinterpret_cast<const char16_t *>(p), cch) : QString::fromLatin1(p, cch);
            for (int i = 0; i < sheets && !b.bad(); ++i) {
                const int n = b.u16();
                const quint8 f = b.u8();
                const bool h = f & 1;
                const char *q = b.take(qint64(n) * (h ? 2 : 1));
                if (!q) break;
                sb.sheets << (h ? QString::fromUtf16(reinterpret_cast<const char16_t *>(q), n) : QString::fromLatin1(q, n));
            }
            sb.ordinal = ++m_externals;
        }
        m_cx.supbooks.append(sb);
    }

    void readExternSheet(const Rec &r)
    {
        Bytes b(m_d, r.pos, r.pos + r.len);
        if (m_biff >= 8) {
            const int n = b.u16();
            for (int i = 0; i < n && b.left() >= 6; ++i) {
                XlsXti x;
                x.supbook = b.u16();
                const quint16 first = b.u16(), last = b.u16();
                x.first = first == 0xFFFF ? -1 : first == 0xFFFE ? -2 : first;
                x.last = last == 0xFFFF ? -1 : last == 0xFFFE ? -2 : last;
                m_cx.xti.append(x);
            }
        } else {
            const int n = b.u8();
            const char *p = b.take(n);
            m_cx.externSheet5.append(p ? QByteArray(p, n) : QByteArray());
        }
    }

    void readExternName(const Rec &r)
    {
        Bytes b(m_d, r.pos, r.pos + r.len);
        b.skip(6);
        const QString name = readString(b, 1);
        if (m_biff < 8) {
            m_cx.externNames5 << name;
            return;
        }
        if (m_cx.supbooks.isEmpty()) return;
        m_cx.supbooks.last().names << name;
    }

    bool readName(const Logical &lg)
    {
        if (m_rawNames.size() >= m_lim.definedNames) return false;
        Bytes b(lg.data, 0, lg.data.size());
        const quint16 grbit = b.u16();
        b.skip(1);   // keyboard shortcut
        const int cch = b.u8();
        const int cce = b.u16();
        b.skip(2);   // the sheet of an external name
        const int itab = b.u16();
        b.skip(4);   // lengths of the menu text, description, help topic, and status text
        RawName n;
        QString name;
        if (m_biff >= 8) {
            const quint8 flags = b.u8();
            const bool high = flags & 1;
            const char *p = b.take(qint64(cch) * (high ? 2 : 1));
            if (p) name = high ? QString::fromUtf16(reinterpret_cast<const char16_t *>(p), cch) : QString::fromLatin1(p, cch);
        } else {
            const char *p = b.take(cch);
            if (p) name = text(p, cch);
        }
        if (grbit & 0x20) {
            const int code = name.isEmpty() ? -1 : name[0].unicode();
            name = QStringLiteral("_xlnm.") + (code >= 0 && code < int(sizeof(kBuiltinNames) / sizeof(kBuiltinNames[0])) ? QString::fromLatin1(kBuiltinNames[code]) : QStringLiteral("Name") + QString::number(code));
        }
        n.text = name;
        n.localSheet = itab > 0 ? itab - 1 : -1;
        n.hidden = grbit & 1;
        if (!b.bad() && cce >= 0 && b.left() >= cce) {
            n.rgce = lg.data.mid(b.at(), cce);
            n.extra = lg.data.mid(b.at() + cce);
        }
        m_nameChars += name.size() + 100;
        if (m_nameChars > m_lim.definedNameChars) return false;
        m_rawNames.append(n);
        return true;
    }

    // Where each sheet's data ends, and the features in it that are not read; no two sheets may share a record.
    bool placeSheets()
    {
        struct Span {
            qsizetype begin, end;
        };
        QVector<Span> spans;
        qint64 budget = qint64(m_d.size()) * 2 + (1 << 20);
        for (int i = 0; i < m_places.size(); ++i) {
            const XlsBook::Place &p = m_places[i];
            if (p.offset < 0 || p.offset + 4 > m_d.size()) {
                continue;
            }
            Scanner sc(m_d, p.offset);
            Rec r;
            if (!sc.next(&r) || r.id != kBof) continue;
            int depth = 0;
            bool nestedChart = false;
            bool ended = false;
            qsizetype last = sc.offset();
            while (sc.next(&r)) {
                budget -= 4 + r.len;
                if (budget < 0) {
                    m_placeProblem = translate("The workbook's sheets share data, which no real file's do.");
                    return false;
                }
                last = sc.offset();
                if (r.id == kBof) {
                    if (++depth > kMaxSubstreamDepth) {
                        m_placeProblem = translate("A sheet is nested more deeply than any real file's.");
                        return false;
                    }
                    Bytes b(m_d, r.pos + 2, r.pos + r.len);
                    if (b.u16() == 0x0020) nestedChart = true;
                    continue;
                }
                if (r.id == kEof) {
                    if (depth == 0) {
                        ended = true;
                        break;
                    }
                    --depth;
                    continue;
                }
                switch (r.id) {
                case kObj:
                case kMsoDrawing: m_drawingsInSheets = true; break;
                case kNote: m_comments = true; break;
                case kSxView: m_pivots = true; break;
                default: break;
                }
            }
            if (nestedChart) m_charts = true;
            if (p.type == 0x20) m_charts = true;
            (void)ended;
            if (p.type != 0x06) spans.append({p.offset, last});
        }
        std::sort(spans.begin(), spans.end(), [](const Span &a, const Span &b) { return a.begin < b.begin; });
        for (int i = 1; i < spans.size(); ++i)
            if (spans[i].begin < spans[i - 1].end) {
                m_placeProblem = translate("Two sheets of the workbook share data, which no real file's do.");
                return false;
            }
        return true;
    }

    // Everything read becomes the workbook's model.
    std::shared_ptr<const XlsBook> finish(bool hasMacros, WorkbookInfo *info, SharedStrings *shared, Styles *styles)
    {
        // the names, now that the sheets, books, and names they refer to are known
        for (const RawName &n : std::as_const(m_rawNames)) {
            XlsNameInfo ni;
            ni.text = n.text;
            ni.localSheet = n.localSheet;
            m_cx.names.append(ni);
        }
        info->sheets = m_sheets;
        info->dateSystem = m_date1904 ? DateSystem::Date1904 : DateSystem::Date1900;
        info->calcMode = m_calcMode;
        info->iterate = m_iterate;
        info->activeTab = m_activeTab < m_sheets.size() ? m_activeTab : 0;
        info->firstSheet = m_firstTab < m_sheets.size() ? m_firstTab : 0;
        info->definedNames.clear();
        qint64 refChars = 0;   // (what the names stand for, all together, is bounded as the XML form's are)
        for (const RawName &n : std::as_const(m_rawNames)) {
            DefinedName d;
            d.name = n.text;
            d.localSheet = n.localSheet < m_sheets.size() ? n.localSheet : -1;
            d.hidden = n.hidden;
            XlsFormulaOptions opts;
            opts.relative = true;
            opts.name = true;
            opts.sheet = d.localSheet;
            opts.maxChars = m_lim.formulaChars;
            opts.maxChars = std::min<qint64>(opts.maxChars, std::max<qint64>(0, m_lim.definedNameChars - refChars));
            const XlsFormulaResult res = n.rgce.isEmpty() ? XlsFormulaResult() : xlsFormulaText(m_cx, n.rgce, n.extra, opts);
            d.ref = n.rgce.isEmpty() ? QString() : res.ok() ? res.text : QStringLiteral("#REF!");
            refChars += d.ref.size();
            info->definedNames.append(d);
        }
        *shared = m_sst;
        buildStyles(styles);

        auto book = std::shared_ptr<XlsBook>(new XlsBook());
        book->m_data = m_d;
        book->m_biff = m_biff;
        book->m_codepage = m_codepage;
        book->m_places = m_places;
        book->m_cx = m_cx;
        book->m_sharedStringCount = m_sst.strings.size();
        book->m_xfCount = m_xfs.size();
        if (m_charts) book->m_skipped << translate("charts");
        if (m_drawings || m_drawingsInSheets) book->m_skipped << translate("pictures and other drawing objects");
        if (m_pivots) book->m_skipped << translate("pivot tables");
        if (m_comments) book->m_skipped << translate("comments");
        if (hasMacros) book->m_skipped << translate("macros");
        for (const SheetEntry &e : std::as_const(m_sheets))
            if (e.kind == SheetKind::Macrosheet && e.state != SheetState::VeryHidden) {
                book->m_skipped << translate("macro sheets");
                break;
            }
        return book;
    }

    void buildStyles(Styles *st)
    {
        *st = Styles();
        st->numFmts = m_formats;
        st->chars = m_formatChars;
        // (the font at place 4 is never in a file: the places above it are one higher than the fonts that were read)
        st->fonts = m_fonts;
        if (st->fonts.size() > 4) st->fonts.insert(4, st->fonts[3]);
        QHash<QByteArray, int> fillIds, borderIds;
        st->fills.append(FillSpec());
        st->borders.append(BorderSpec());
        const qsizetype count = m_xfs.size();
        st->cellXfs.fill(0, int(count));
        st->formats.fill(CellFormat(), int(count));
        for (int i = 0; i < count; ++i) {
            const RawXf &x = m_xfs[i];
            CellFormat f;
            f.numFmtId = x.format;
            f.font = x.font;
            // alignment: the vertical places are numbered from the top here, and from the bottom in the model
            static const VAlign vmap[] = {VAlign::Top, VAlign::Center, VAlign::Bottom, VAlign::Justify, VAlign::Distributed};
            f.vertical = x.valign < 5 ? vmap[x.valign] : VAlign::Bottom;
            f.horizontal = HAlign(std::min<int>(x.halign, 7));
            f.wrap = x.wrap;
            f.shrink = x.shrink;
            f.indent = x.indent;
            f.rotation = x.rotation;
            // fill
            FillSpec fill;
            fill.pattern = PatternKind(std::min<int>(x.pattern, int(PatternKind::Gray0625)));
            if (fill.pattern != PatternKind::None) {
                fill.fg = indexedColor(x.fore);
                fill.bg = indexedColor(x.back);
            }
            const QByteArray fkey = QByteArray::number(int(fill.pattern)) + ',' + QByteArray::number(x.fore) + ',' + QByteArray::number(x.back);
            if (fill.pattern == PatternKind::None) {
                f.fill = 0;
            } else {
                auto it = fillIds.constFind(fkey);
                if (it == fillIds.cend()) {
                    st->fills.append(fill);
                    it = fillIds.insert(fkey, int(st->fills.size() - 1));
                }
                f.fill = quint32(it.value());
            }
            // border
            BorderSpec bd;
            auto side = [&](BorderSide &s, quint8 style, quint16 color) {
                s.style = BorderStyle(std::min<int>(style, int(BorderStyle::SlantDashDot)));
                if (s.style != BorderStyle::None) s.color = indexedColor(color);
            };
            side(bd.left, x.left, x.colorLeft);
            side(bd.right, x.right, x.colorRight);
            side(bd.top, x.top, x.colorTop);
            side(bd.bottom, x.bottom, x.colorBottom);
            side(bd.diagonal, x.diagonal, x.colorDiagonal);
            bd.diagonalUp = x.diagUp;
            bd.diagonalDown = x.diagDown;
            const QByteArray bkey = QByteArray::number(x.left) + ',' + QByteArray::number(x.right) + ',' + QByteArray::number(x.top) + ',' + QByteArray::number(x.bottom) + ','
                + QByteArray::number(x.diagonal) + ',' + QByteArray::number(x.colorLeft) + ',' + QByteArray::number(x.colorRight) + ',' + QByteArray::number(x.colorTop) + ','
                + QByteArray::number(x.colorBottom) + ',' + QByteArray::number(x.colorDiagonal) + ',' + QByteArray::number(int(x.diagUp)) + QByteArray::number(int(x.diagDown));
            if (x.left == 0 && x.right == 0 && x.top == 0 && x.bottom == 0 && x.diagonal == 0) {
                f.border = 0;
            } else {
                auto it = borderIds.constFind(bkey);
                if (it == borderIds.cend()) {
                    st->borders.append(bd);
                    it = borderIds.insert(bkey, int(st->borders.size() - 1));
                }
                f.border = quint32(it.value());
            }
            const quint32 at = mapXf(quint32(i), count);
            st->formats[int(at)] = f;
            st->cellXfs[int(at)] = f.numFmtId;
        }
        if (!m_palette.isEmpty()) {
            st->indexed.clear();
            for (int i = 0; i < 64; ++i) st->indexed.append(i >= 8 && i - 8 < m_palette.size() ? m_palette[i - 8] : Styles::defaultIndexed(i));
        }
    }
};

std::shared_ptr<const XlsBook> XlsBook::open(const QByteArray &stream, bool hasMacros, const Limits &limits, WorkbookInfo *info, SharedStrings *shared, Styles *styles,
                                             QString *error)
{
    XlsGlobals g(stream, limits);
    return g.run(hasMacros, info, shared, styles, error);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// a sheet

class XlsSheetReader {
public:
    XlsSheetReader(const XlsBook &book, int index, const Limits &limits, const std::function<bool(qint64)> &progress, Sheet *out)
        : m_book(book), m_cx(&book.m_cx), m_d(book.m_data), m_index(index), m_lim(limits), m_progress(progress), m_s(*out), m_b8(book.m_biff >= 8)
    {
    }

    bool run(const XlsBook::Place &place)
    {
        bool ok = true;
        if (place.offset < 0 || place.type == 0x20 || place.type == 0x06) {
            m_s.cells.finish();
            return true;
        }
        Scanner sc(m_d, place.offset);
        Rec r;
        if (!sc.next(&r) || r.id != kBof) {
            m_s.error = translate("The sheet's data is not where the workbook says it is.");
            m_s.cells.finish();
            return false;
        }
        m_format.defaultRowHeight = 12.75;
        int depth = 0;
        while (ok && sc.next(&r)) {
            if (depth > 0) {
                if (r.id == kBof) {
                    if (++depth > kMaxSubstreamDepth) ok = fail(translate("The sheet is nested more deeply than any real file's."));
                } else if (r.id == kEof) {
                    --depth;
                }
                continue;
            }
            if (m_holding && r.id != kString && r.id != kShrFmla && r.id != kArray && r.id != kTable && r.id != kContinue) ok = flushHeld();
            if (!ok) break;
            if (r.id == kBof) {
                depth = 1;
                continue;
            }
            if (r.id == kEof) break;
            ok = handle(sc, r);
        }
        if (ok && m_holding) ok = flushHeld();
        m_s.cells.finish();
        if (ok) ok = resolveExpressions();
        if (m_haveWidth) m_format.defaultColWidth = m_standardWidth;
        m_s.format = m_format;
        // (a split that is not frozen is not kept)
        if (m_view.frozen && m_pane.present && (m_pane.x > 0 || m_pane.y > 0)) {
            m_view.frozenCols = m_pane.x;
            m_view.frozenRows = m_pane.y;
            m_view.paneRow = m_pane.top;
            m_view.paneCol = m_pane.left;
        } else {
            m_view.frozen = false;
        }
        m_view.zoom = m_scl ? m_scl : m_wScale ? m_wScale : 100;
        if (m_haveSelection) {
            const Selection &sel = m_selections.contains(m_pane.active) ? m_selections[m_pane.active] : m_selections.cbegin().value();
            m_view.activeRow = sel.row;
            m_view.activeCol = sel.col;
        }
        m_s.view = m_view;
        m_s.stats.heldBytes = m_held;
        return ok && m_s.error.isEmpty();
    }

private:
    struct SharedDef {
        CellRange range;
        qint32 text = -1;
        bool expand = false;
        QByteArray rgce, extra;
    };
    struct ArrayDef {
        CellRange range;
        qint32 text = -1;
        qint32 rangeIndex = -1;
    };
    struct TableDef {
        CellRange range;
        qint32 text = -1;
        qint32 rangeIndex = -1;
    };
    struct Pending {
        qint32 formula;
        quint32 row;
        quint32 col;
        quint32 expRow, expCol;
        bool table;
    };
    struct Selection {
        quint32 row = 0, col = 0;
    };
    struct PaneInfo {
        bool present = false;
        quint32 x = 0, y = 0, top = 0, left = 0;
        int active = 3;
    };

    const XlsBook &m_book;
    const XlsContext *m_cx;                  // the workbook's, or (BIFF5 keeps the list of external sheets and names in each sheet) the sheet's own
    std::unique_ptr<XlsContext> m_ownCx;
    const QByteArray &m_d;
    int m_index;
    const Limits &m_lim;
    const std::function<bool(qint64)> &m_progress;
    Sheet &m_s;
    bool m_b8;
    qint64 m_cells = 0;
    qint64 m_held = 0;
    qint64 m_outside = 0;
    SheetFormat m_format;
    SheetView m_view;
    PaneInfo m_pane;
    QMap<int, Selection> m_selections;
    bool m_haveSelection = false;
    bool m_holding = false;
    Cell m_heldCell;
    QHash<quint32, SharedDef> m_shared;
    QHash<quint32, ArrayDef> m_arrays;
    QHash<quint32, TableDef> m_tables;
    std::vector<Pending> m_pending;
    bool m_haveWidth = false;
    quint32 m_lastFormulaRow = 0, m_lastFormulaCol = 0;   // the cell of the formula record before the one being read: the cell a shared formula, array, or data table is the record of
    bool m_haveLastFormula = false;
    double m_standardWidth = 0;
    int m_wScale = 0, m_scl = 0;

    bool fail(const QString &why)
    {
        if (m_s.error.isEmpty()) m_s.error = why;
        return false;
    }

    QString text(const char *p, int n) const { return xlsBytesToText(m_book.m_codepage, p, n); }
    quint32 style(quint32 xf) const { return mapXf(xf, m_book.m_xfCount); }
    static quint32 key(quint32 row, quint32 col) { return (row << 16) | col; }

    bool addCell(Cell c, qint64 extraBytes = 0)
    {
        if (c.col >= 256) {
            ++m_outside;
            return true;
        }
        if (c.type == CellType::Empty && !c.hasFormula() && c.style == 0) return true;
        if (++m_cells > m_lim.cellsPerSheet) return fail(translate("The sheet has more cells than any real file has."));
        m_held += 32 + extraBytes;
        if (m_held > m_lim.sheetBytes) return fail(translate("The sheet is larger than any real file's."));
        m_s.cells.add(c);
        m_s.extentRows = std::max(m_s.extentRows, c.row + 1);
        m_s.extentCols = std::max(m_s.extentCols, quint32(c.col) + 1);
        if (m_progress && m_cells % 4096 == 0 && !m_progress(m_cells)) return fail(translate("The read was stopped."));
        return true;
    }

    Cell makeCell(quint32 row, quint32 col, quint32 xf)
    {
        Cell c;
        c.row = row;
        c.col = quint16(std::min<quint32>(col, 0xFFFF));
        c.style = style(xf);
        return c;
    }

    bool addText(Cell c, const QString &s, CellType type)
    {
        c.type = type;
        c.number = double(m_s.texts.size());
        m_s.texts.append(s);
        m_s.stats.longestString = std::max<qint64>(m_s.stats.longestString, s.size());
        return addCell(c, s.size() * 2 + 32);
    }

    bool flushHeld()
    {
        m_holding = false;
        return addCell(m_heldCell);
    }

    bool handle(Scanner &sc, const Rec &r)
    {
        Bytes b(m_d, r.pos, r.pos + r.len);
        switch (r.id) {
        case kDimensions: {
            if (m_b8) {
                const quint32 r1 = b.u32(), r2 = b.u32();
                const quint16 c1 = b.u16(), c2 = b.u16();
                if (!b.bad() && r2 > r1 && c2 > c1) {
                    m_s.dimension.row1 = r1;
                    m_s.dimension.row2 = r2 - 1;
                    m_s.dimension.col1 = c1;
                    m_s.dimension.col2 = c2 - 1;
                    m_s.dimension.valid = true;
                }
            } else {
                const quint16 r1 = b.u16(), r2 = b.u16(), c1 = b.u16(), c2 = b.u16();
                if (!b.bad() && r2 > r1 && c2 > c1) {
                    m_s.dimension.row1 = r1;
                    m_s.dimension.row2 = r2 - 1u;
                    m_s.dimension.col1 = c1;
                    m_s.dimension.col2 = c2 - 1u;
                    m_s.dimension.valid = true;
                }
            }
            return true;
        }
        case kNumber: {
            const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
            const double v = b.f64();
            if (b.bad()) return true;
            Cell c = makeCell(row, col, xf);
            c.type = CellType::Number;
            c.number = v;
            return addCell(c);
        }
        case kRk: {
            const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
            const quint32 rk = b.u32();
            if (b.bad()) return true;
            Cell c = makeCell(row, col, xf);
            c.type = CellType::Number;
            c.number = rkValue(rk);
            return addCell(c);
        }
        case kMulRk: {
            const quint16 row = b.u16(), first = b.u16();
            const qint64 n = (qint64(r.len) - 6) / 6;
            for (qint64 i = 0; i < n; ++i) {
                const quint16 xf = b.u16();
                const quint32 rk = b.u32();
                Cell c = makeCell(row, quint32(first) + quint32(i), xf);
                c.type = CellType::Number;
                c.number = rkValue(rk);
                if (!addCell(c)) return false;
            }
            return true;
        }
        case kLabelSst: {
            const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
            const quint32 index = b.u32();
            if (b.bad()) return true;
            Cell c = makeCell(row, col, xf);
            c.type = CellType::SharedString;
            c.number = double(index);
            return addCell(c);
        }
        case kLabel:
        case kRString: {
            const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
            if (b.bad()) return true;
            QString s;
            if (m_b8 && r.id == kLabel) {
                Logical lg = single(m_d, r);
                Chars ch(lg, 6);
                const int n = ch.u16();
                const quint8 flags = ch.u8();
                s = ch.chars(n, flags & 1);
            } else {
                const int n = b.u16();
                const char *p = b.take(n);
                if (p) s = text(p, n);
            }
            Cell c = makeCell(row, col, xf);
            return addText(c, s, CellType::InlineString);
        }
        case kBlank: {
            const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
            if (b.bad()) return true;
            return addCell(makeCell(row, col, xf));
        }
        case kMulBlank: {
            const quint16 row = b.u16(), first = b.u16();
            const qint64 n = (qint64(r.len) - 6) / 2;
            for (qint64 i = 0; i < n; ++i) {
                const quint16 xf = b.u16();
                if (!addCell(makeCell(row, quint32(first) + quint32(i), xf))) return false;
            }
            return true;
        }
        case kBoolErr: {
            const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
            const quint8 v = b.u8(), isError = b.u8();
            if (b.bad()) return true;
            Cell c = makeCell(row, col, xf);
            if (isError) {
                c.type = CellType::Error;
                c.number = double(int(cellErrorFromCode(v)));
            } else {
                c.type = CellType::Bool;
                c.number = v ? 1 : 0;
            }
            return addCell(c);
        }
        case kExternCount:
            if (!m_b8) {
                m_ownCx = std::make_unique<XlsContext>(m_book.m_cx);
                m_ownCx->externSheet5.clear();
                m_ownCx->externNames5.clear();
                m_cx = m_ownCx.get();
            }
            return true;
        case kExternSheet:
            if (!m_b8 && m_ownCx) {
                const int n = b.u8();
                const char *p = b.take(n);
                m_ownCx->externSheet5.append(p ? QByteArray(p, n) : QByteArray());
            }
            return true;
        case kExternName:
            if (!m_b8 && m_ownCx) {
                b.skip(6);
                const int n = b.u8();
                const char *p = b.take(n);
                m_ownCx->externNames5.append(p ? text(p, n) : QString());
            }
            return true;
        case kFormula: return handleFormula(sc, r);
        case kString: {
            if (!m_holding) return true;
            const Logical lg = joined(m_d, sc, r, true);
            QString s;
            if (m_b8) {
                Chars ch(lg, 0);
                const int n = ch.u16();
                const quint8 flags = ch.u8();
                s = ch.chars(n, flags & 1);
            } else {
                Bytes sb(lg.data, 0, lg.data.size());
                const int n = sb.u16();
                const char *p = sb.take(n);
                if (p) s = text(p, n);
            }
            m_heldCell.type = CellType::FormulaString;
            m_heldCell.number = double(m_s.texts.size());
            m_s.texts.append(s);
            m_s.stats.longestString = std::max<qint64>(m_s.stats.longestString, s.size());
            m_held += s.size() * 2 + 32;
            ++m_s.storedResults;
            return flushHeld();
        }
        case kShrFmla: return handleShared(sc, r);
        case kArray: return handleArray(sc, r);
        case kTable: return handleTable(r);
        case kRow: {
            const quint16 row = b.u16();
            b.skip(4);
            const quint16 height = b.u16();
            b.skip(4);
            const quint16 grbit = b.u16();
            const quint16 xfField = b.u16();
            if (b.bad()) return true;
            RowSpec spec;
            spec.row = row;
            spec.height = (height & 0x7FFF) / 20.0;
            spec.hidden = grbit & 0x20;
            spec.customHeight = grbit & 0x40;
            spec.customFormat = grbit & 0x80;
            spec.style = spec.customFormat ? style(xfField & 0x0FFF) : 0;
            const bool defaultHeight = (height & 0x8000) || std::abs(spec.height - m_format.defaultRowHeight) < 0.01;
            if (spec.hidden || spec.customFormat || spec.customHeight || !defaultHeight) {
                if (m_s.rowSpecs.size() >= m_lim.rowSpecs) return fail(translate("The sheet has more rows with their own size than any real file has."));
                if (defaultHeight) spec.height = -1;
                m_s.rowSpecs.append(spec);
                ++m_s.stats.rowSpecs;
            }
            return true;
        }
        case kColInfo: {
            const quint16 first = b.u16(), last = b.u16(), width = b.u16(), xf = b.u16(), grbit = b.u16();
            if (b.bad()) return true;
            if (m_s.columns.size() >= m_lim.columnSpans) return fail(translate("The sheet has more column ranges than any real file has."));
            ColumnSpan c;
            c.first = first;
            c.last = std::min<quint32>(last, kMaxColumns - 1);
            c.width = width / 256.0;
            c.hidden = grbit & 1;
            c.customWidth = grbit & 2;
            c.style = xf ? style(xf) : 0;
            m_s.columns.append(c);
            ++m_s.stats.columnSpans;
            return true;
        }
        case kDefColWidth: {
            const quint16 w = b.u16();
            if (!b.bad()) m_format.baseColWidth = w;
            return true;
        }
        case kStandardWidth: {
            const quint16 w = b.u16();
            if (!b.bad() && w > 0) {
                m_standardWidth = w / 256.0;
                m_haveWidth = true;
            }
            return true;
        }
        case kDefaultRowHeight: {
            const quint16 grbit = b.u16(), height = b.u16();
            if (b.bad()) return true;
            if (height > 0) m_format.defaultRowHeight = (height & 0x7FFF) / 20.0;
            m_format.zeroHeight = grbit & 2;
            m_format.customHeight = grbit & 1;
            return true;
        }
        case kMergedCells: {
            const int n = b.u16();
            for (int i = 0; i < n && b.left() >= 8; ++i) {
                const quint16 r1 = b.u16(), r2 = b.u16(), c1 = b.u16(), c2 = b.u16();
                if (m_s.mergeCount >= m_lim.merges) return fail(translate("The sheet has more merged ranges than any real file has."));
                ++m_s.mergeCount;
                CellRange range;
                range.row1 = std::min(r1, r2);
                range.row2 = std::max(r1, r2);
                range.col1 = std::min(c1, c2);
                range.col2 = std::max(c1, c2);
                range.valid = true;
                m_s.merges.append(range);
            }
            return true;
        }
        case kWindow2: {
            const quint16 grbit = b.u16();
            const quint16 top = b.u16(), left = b.u16();
            if (b.bad()) return true;
            m_view.showGridLines = grbit & 0x02;
            m_view.showHeadings = grbit & 0x04;
            m_view.frozen = grbit & 0x08;
            m_view.showZeros = grbit & 0x10;
            m_view.rightToLeft = grbit & 0x40;
            m_view.tabSelected = grbit & 0x200;
            m_view.topRow = top;
            m_view.leftCol = left;
            if (m_b8 && r.len >= 14) {
                b.skip(6);
                const quint16 scale = b.u16();
                if (!b.bad() && scale >= 10 && scale <= 400) m_wScale = scale;
            }
            return true;
        }
        case kPane: {
            const quint16 x = b.u16(), y = b.u16(), top = b.u16(), left = b.u16();
            const quint8 active = b.u8();
            if (b.bad()) return true;
            m_pane.present = true;
            m_pane.x = x;
            m_pane.y = y;
            m_pane.top = top;
            m_pane.left = left;
            m_pane.active = active;
            return true;
        }
        case kScl: {
            const quint16 num = b.u16(), denom = b.u16();
            if (b.bad() || denom == 0) return true;
            const qint64 z = qint64(num) * 100 / denom;
            if (z >= 10 && z <= 400) m_scl = int(z);
            return true;
        }
        case kSelection: {
            const quint8 pane = b.u8();
            const quint16 row = b.u16(), col = b.u16();
            if (b.bad()) return true;
            Selection sel;
            sel.row = row;
            sel.col = col;
            m_selections[pane] = sel;
            m_haveSelection = true;
            return true;
        }
        default: return true;
        }
    }

    bool handleFormula(Scanner &sc, const Rec &r)
    {
        const Logical lg = joined(m_d, sc, r, true);
        Bytes b(lg.data, 0, lg.data.size());
        const quint16 row = b.u16(), col = b.u16(), xf = b.u16();
        const char *res = b.take(8);
        b.skip(6);   // flags and the chain
        const int cce = b.u16();
        if (b.bad() || !res) return true;
        Cell c = makeCell(row, col, xf);
        c.flags |= HasFormula;
        m_lastFormulaRow = row;
        m_lastFormulaCol = col;
        m_haveLastFormula = true;
        // the result
        bool wantString = false;
        quint8 rb[8];
        std::memcpy(rb, res, 8);
        if (rb[6] == 0xFF && rb[7] == 0xFF) {
            switch (rb[0]) {
            case 0: wantString = true; break;
            case 1:
                c.type = CellType::Bool;
                c.number = rb[2] ? 1 : 0;
                break;
            case 2:
                c.type = CellType::Error;
                c.number = double(int(cellErrorFromCode(rb[2])));
                break;
            case 3:
                c.type = CellType::FormulaString;
                c.number = double(m_s.texts.size());
                m_s.texts.append(QString());
                break;
            default: break;
            }
        } else {
            double v;
            std::memcpy(&v, rb, 8);
            c.type = CellType::Number;
            c.number = v;
        }
        ++m_s.formulaCells;
        if (c.type != CellType::Empty) ++m_s.storedResults;
        const QByteArray rgce = lg.data.mid(b.at(), cce);
        const QByteArray extra = b.at() + cce <= lg.data.size() ? lg.data.mid(b.at() + cce) : QByteArray();
        Formula f;
        if (rgce.size() == 5 && (quint8(rgce[0]) == 0x01 || quint8(rgce[0]) == 0x02)) {
            // one that other cells share, or part of an array or a data table: its text comes with the record that holds it
            Bytes eb(rgce);
            const bool table = quint8(rgce[0]) == 0x02;
            eb.skip(1);
            const quint32 er = eb.u16();
            const quint32 ec = eb.u16();
            m_s.formulas.append(f);
            c.formula = qint32(m_s.formulas.size() - 1);
            m_held += 48;
            m_pending.push_back({c.formula, row, col, er, ec, table});
        } else {
            XlsFormulaOptions opts;
            opts.baseRow = row;
            opts.baseCol = col;
            opts.sheet = m_index;
            opts.maxChars = m_lim.formulaChars;
            const XlsFormulaResult conv = xlsFormulaText(*m_cx, rgce, extra, opts);
            if (!conv.ok()) {
                // (the result the file stored stays; the formula is not kept)
                c.flags = 0;
                ++m_s.stats.unreadFormulas;
                --m_s.formulaCells;
                if (c.type != CellType::Empty) --m_s.storedResults;
            } else {
                m_s.stats.longestFormula = std::max<qint64>(m_s.stats.longestFormula, conv.text.size());
                m_s.stats.expandedFormulaChars += conv.text.size();
                if (m_s.stats.expandedFormulaChars > m_lim.expandedFormulaChars) return fail(translate("The formulas, all together, are longer than any real file has."));
                m_s.formulaTexts.append(conv.text);
                f.text = qint32(m_s.formulaTexts.size() - 1);
                m_s.formulas.append(f);
                c.formula = qint32(m_s.formulas.size() - 1);
                m_held += conv.text.size() * 2 + 48;
            }
        }
        if (wantString) {
            m_heldCell = c;
            m_holding = true;
            return true;
        }
        return addCell(c);
    }

    CellRange readRefU(Bytes &b) const
    {
        CellRange rg;
        const quint16 r1 = b.u16(), r2 = b.u16();
        const quint8 c1 = b.u8(), c2 = b.u8();
        rg.row1 = std::min(r1, r2);
        rg.row2 = std::max(r1, r2);
        rg.col1 = std::min(c1, c2);
        rg.col2 = std::max(c1, c2);
        rg.valid = !b.bad();
        return rg;
    }

    // The cells that share a formula name the cell whose record it follows (not always the first of the range).
    void registerShared(const SharedDef &def)
    {
        if (m_haveLastFormula && def.range.contains(m_lastFormulaRow, m_lastFormulaCol)) m_shared.insert(key(m_lastFormulaRow, m_lastFormulaCol), def);
        if (!m_shared.contains(key(def.range.row1, def.range.col1))) m_shared.insert(key(def.range.row1, def.range.col1), def);
    }

    bool handleShared(Scanner &sc, const Rec &r)
    {
        const Logical lg = joined(m_d, sc, r, true);
        Bytes b(lg.data, 0, lg.data.size());
        SharedDef def;
        def.range = readRefU(b);
        b.skip(2);
        const int cce = b.u16();
        if (b.bad() || !def.range.valid || b.left() < cce) return true;
        if (m_shared.size() >= m_lim.sharedFormulas) return fail(translate("The sheet has more shared formulas than any real file has."));
        if (def.range.cellCount() > m_lim.sharedRangeCells) return fail(translate("A shared formula covers a range larger than any real file has."));
        const QByteArray rgce = lg.data.mid(b.at(), cce);
        const QByteArray extra = lg.data.mid(b.at() + cce);
        XlsFormulaOptions opts;
        opts.relative = true;
        opts.sheet = m_index;
        opts.maxChars = m_lim.formulaChars;
        opts.baseRow = def.range.row1;
        opts.baseCol = def.range.col1;
        const XlsFormulaResult first = xlsFormulaText(*m_cx, rgce, extra, opts);
        if (!first.ok()) {
            // none of its cells can have it
            registerShared(def);
            return true;
        }
        opts.baseRow = def.range.row2;
        opts.baseCol = def.range.col2;
        const XlsFormulaResult last = xlsFormulaText(*m_cx, rgce, extra, opts);
        def.expand = first.outside || !last.ok() || last.outside;
        if (def.expand) {
            def.rgce = rgce;
            def.extra = extra;
        } else {
            m_s.stats.longestFormula = std::max<qint64>(m_s.stats.longestFormula, first.text.size());
            m_s.formulaTexts.append(first.text);
            def.text = qint32(m_s.formulaTexts.size() - 1);
            m_held += first.text.size() * 2 + 48;
        }
        ++m_s.stats.sharedFormulas;
        m_s.stats.largestSharedRange = std::max(m_s.stats.largestSharedRange, def.range.cellCount());
        registerShared(def);
        return true;
    }

    bool handleArray(Scanner &sc, const Rec &r)
    {
        const Logical lg = joined(m_d, sc, r, true);
        Bytes b(lg.data, 0, lg.data.size());
        ArrayDef def;
        def.range = readRefU(b);
        b.skip(6);
        const int cce = b.u16();
        if (b.bad() || !def.range.valid || b.left() < cce) return true;
        const QByteArray rgce = lg.data.mid(b.at(), cce);
        const QByteArray extra = lg.data.mid(b.at() + cce);
        XlsFormulaOptions opts;
        opts.baseRow = def.range.row1;
        opts.baseCol = def.range.col1;
        opts.sheet = m_index;
        opts.maxChars = m_lim.formulaChars;
        const XlsFormulaResult conv = xlsFormulaText(*m_cx, rgce, extra, opts);
        if (conv.ok()) {
            m_s.stats.longestFormula = std::max<qint64>(m_s.stats.longestFormula, conv.text.size());
            m_s.stats.expandedFormulaChars += conv.text.size();
            m_s.formulaTexts.append(conv.text);
            def.text = qint32(m_s.formulaTexts.size() - 1);
            m_s.ranges.append(def.range);
            def.rangeIndex = qint32(m_s.ranges.size() - 1);
            m_held += conv.text.size() * 2 + 64;
        }
        if (m_haveLastFormula && def.range.contains(m_lastFormulaRow, m_lastFormulaCol)) m_arrays.insert(key(m_lastFormulaRow, m_lastFormulaCol), def);
        if (!m_arrays.contains(key(def.range.row1, def.range.col1))) m_arrays.insert(key(def.range.row1, def.range.col1), def);
        return true;
    }

    bool handleTable(const Rec &r)
    {
        Bytes b(m_d, r.pos, r.pos + r.len);
        TableDef def;
        def.range = readRefU(b);
        const quint16 grbit = b.u16();
        const quint16 r1 = b.u16(), c1 = b.u16(), r2 = b.u16(), c2 = b.u16();
        if (b.bad() || !def.range.valid) return true;
        const bool twoInputs = grbit & 0x08;
        QString text = QStringLiteral("TABLE(") + cellName(r1, c1);
        if (twoInputs) text += QLatin1Char(',') + cellName(r2, c2);
        text += QLatin1Char(')');
        m_s.formulaTexts.append(text);
        def.text = qint32(m_s.formulaTexts.size() - 1);
        m_s.ranges.append(def.range);
        def.rangeIndex = qint32(m_s.ranges.size() - 1);
        m_held += 160;
        if (m_haveLastFormula && def.range.contains(m_lastFormulaRow, m_lastFormulaCol)) m_tables.insert(key(m_lastFormulaRow, m_lastFormulaCol), def);
        if (!m_tables.contains(key(def.range.row1, def.range.col1))) m_tables.insert(key(def.range.row1, def.range.col1), def);
        return true;
    }

    // The cells that held only a pointer to a shared formula, an array, or a data table get what they point to.
    bool resolveExpressions()
    {
        for (const Pending &p : m_pending) {
            Cell *c = m_s.cells.findMutable(p.row, p.col);
            if (!c || c->formula != p.formula) continue;
            auto plain = [&]() {
                c->flags &= quint8(~(HasFormula | SharedFormula | ArrayFormula | DataTable));
                c->formula = -1;
                --m_s.formulaCells;
                if (c->type != CellType::Empty) --m_s.storedResults;
            };
            Formula &f = m_s.formulas[p.formula];
            const quint32 k = key(p.expRow, p.expCol);
            if (p.table) {
                const auto it = m_tables.constFind(k);
                if (it != m_tables.cend() && p.row == p.expRow && p.col == p.expCol) {
                    c->flags |= DataTable;
                    f.text = it->text;
                    f.range = it->rangeIndex;
                    m_s.stats.expandedFormulaChars += m_s.formulaTexts[it->text].size();
                } else {
                    plain();
                }
                continue;
            }
            const auto sh = m_shared.constFind(k);
            if (sh != m_shared.cend() && sh->range.contains(p.row, p.col)) {
                if (!sh->expand && sh->text >= 0) {
                    c->flags |= SharedFormula;
                    f.text = sh->text;
                    f.dRow = qint32(qint64(p.row) - sh->range.row1);
                    f.dCol = qint32(qint64(p.col) - sh->range.col1);
                    m_s.stats.expandedFormulaChars += m_s.formulaTexts[sh->text].size();
                    if (m_s.stats.expandedFormulaChars > m_lim.expandedFormulaChars) return fail(translate("The formulas, all together, are longer than any real file has."));
                } else if (sh->expand) {
                    XlsFormulaOptions opts;
                    opts.relative = true;
                    opts.sheet = m_index;
                    opts.maxChars = m_lim.formulaChars;
                    opts.baseRow = p.row;
                    opts.baseCol = p.col;
                    const XlsFormulaResult conv = xlsFormulaText(*m_cx, sh->rgce, sh->extra, opts);
                    if (conv.ok()) {
                        m_s.formulaTexts.append(conv.text);
                        f.text = qint32(m_s.formulaTexts.size() - 1);
                        m_s.stats.expandedFormulaChars += conv.text.size();
                        if (m_s.stats.expandedFormulaChars > m_lim.expandedFormulaChars) return fail(translate("The formulas, all together, are longer than any real file has."));
                    } else {
                        plain();
                        ++m_s.stats.unreadFormulas;
                    }
                } else {
                    plain();
                    ++m_s.stats.unreadFormulas;
                }
                continue;
            }
            const auto ar = m_arrays.constFind(k);
            if (ar != m_arrays.cend() && ar->range.contains(p.row, p.col)) {
                if (p.row == p.expRow && p.col == p.expCol && ar->text >= 0) {
                    c->flags |= ArrayFormula;
                    f.text = ar->text;
                    f.range = ar->rangeIndex;
                } else {
                    plain();
                    if (ar->text < 0 && p.row == p.expRow && p.col == p.expCol) ++m_s.stats.unreadFormulas;
                }
                continue;
            }
            plain();
            ++m_s.stats.unreadFormulas;
        }
        m_pending.clear();
        return true;
    }
};

bool XlsBook::readSheet(int index, const Limits &limits, const std::function<bool(qint64)> &progress, Sheet *out) const
{
    *out = Sheet();
    if (index < 0 || index >= m_places.size()) {
        out->error = translate("There is no such sheet.");
        return false;
    }
    XlsSheetReader reader(*this, index, limits, progress, out);
    return reader.run(m_places[index]);
}

} // namespace jp::office::sheet
