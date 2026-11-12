// A workbook written as the old binary form (BIFF8), read back by the reader of xls.cpp and by other programs. The workbook stream is made in two parts: each sheet's records are made
// first (they decide which strings, formats, fonts, colors, and sheets-in-formulas the file needs), then the records every sheet needs (the strings, the formats, the fonts, the
// palette, the list of sheets, the names) are made and put before them, and the whole is put in a compound file.
//
// Every count is bounded by what the format holds (rows, columns, a text of a cell, the cell formats, the fonts, the number formats, the sheets) or by the limits of oldsave.h; what is cut
// is named in the report.

#include "sheet/oldsave.h"

#include "io/cfb.h"
#include "sheet/oldsavecommon.h"
#include "sheet/xlscompile.h"
#include "sheet/xlsrecord.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <QMap>

namespace jp::office::sheet {

namespace {

using oldsave::Written;

constexpr int kFormulaHead = 22;   // the bytes of a formula record before its tokens
constexpr int kMaxRecord = 8224;   // the bytes a record may hold; more go in CONTINUE records where the format allows

class W {
public:
    W &u8(int v)
    {
        m_b.append(char(v));
        return *this;
    }
    W &u16(int v)
    {
        m_b.append(char(v & 0xFF));
        m_b.append(char((v >> 8) & 0xFF));
        return *this;
    }
    W &u32(quint32 v)
    {
        u16(int(v & 0xFFFF));
        return u16(int(v >> 16));
    }
    W &f64(double v)
    {
        quint64 bits;
        std::memcpy(&bits, &v, 8);
        for (int i = 0; i < 8; ++i) m_b.append(char((bits >> (8 * i)) & 0xFF));
        return *this;
    }
    W &raw(const QByteArray &b)
    {
        m_b.append(b);
        return *this;
    }
    // A string of BIFF8: its count (1 or 2 bytes), the byte that says one or two bytes for each character, and the characters.
    W &str8(const QString &s, int countBytes)
    {
        bool wide = false;
        for (const QChar c : s)
            if (c.unicode() > 0xFF) wide = true;
        if (countBytes == 1) u8(int(s.size()));
        else u16(int(s.size()));
        u8(wide ? 1 : 0);
        for (const QChar c : s) {
            if (wide) u16(c.unicode());
            else u8(c.unicode());
        }
        return *this;
    }
    QByteArray bytes() const { return m_b; }
    qsizetype size() const { return m_b.size(); }

private:
    QByteArray m_b;
};

void putRecord(QByteArray *out, quint16 id, const QByteArray &data) { xlsPutRecord(out, id, data); }

bool isWide(const QString &s)
{
    for (const QChar c : s)
        if (c.unicode() > 0xFF) return true;
    return false;
}

// A record whose text may run over into CONTINUE records (the table of strings, and the text of a formula's result): the records are made as the pieces are put in.
class Continued {
public:
    Continued(QByteArray *out, quint16 id) : m_out(out), m_id(id) {}
    void bytes(const QByteArray &b)
    {
        if (m_cur.size() + b.size() > kMaxRecord) flush();
        m_cur.append(b);
    }
    void begin(qsizetype need)
    {
        if (m_cur.size() + need > kMaxRecord) flush();
    }
    // A string whose count and flags are put whole, and whose characters may be cut between records (a cut inside the characters puts the byte of flags first in the next record).
    void string(const QString &s)
    {
        const bool wide = isWide(s);
        begin(3 + (wide ? 2 : 1));
        W h;
        h.u16(int(s.size())).u8(wide ? 1 : 0);
        m_cur.append(h.bytes());
        qsizetype at = 0;
        while (at < s.size()) {
            qsizetype room = (kMaxRecord - m_cur.size()) / (wide ? 2 : 1);
            if (room <= 0) {
                flush();
                m_cur.append(char(wide ? 1 : 0));
                room = (kMaxRecord - 1) / (wide ? 2 : 1);
            }
            const qsizetype n = std::min<qsizetype>(room, s.size() - at);
            for (qsizetype i = 0; i < n; ++i) {
                const ushort c = s[at + i].unicode();
                m_cur.append(char(c & 0xFF));
                if (wide) m_cur.append(char(c >> 8));
            }
            at += n;
            if (at < s.size()) flush(), m_cur.append(char(wide ? 1 : 0));
        }
    }
    void finish()
    {
        flush();
        m_first = true;
    }

private:
    void flush()
    {
        if (m_cur.isEmpty() && !m_first) return;
        W h;
        h.u16(m_first ? m_id : 0x003C).u16(int(m_cur.size()));
        m_out->append(h.bytes());
        m_out->append(m_cur);
        m_cur.clear();
        m_first = false;
    }
    QByteArray *m_out;
    quint16 m_id;
    QByteArray m_cur;
    bool m_first = true;
};

struct Rgb {
    int r, g, b;
};

class XlsWriter {
public:
    XlsWriter(const Workbook &book, CopyReport *report, const std::function<bool()> &cancelled) : m_book(book), m_report(report), m_cancelled(cancelled), m_features(book, CopyKind::Xls) {}

    bool run(QByteArray *out, QString *error)
    {
        m_written = oldsave::sheetsToWrite(m_book, kMaxXlsSheets, 31, m_report);
        for (size_t i = 0; i < m_written.size(); ++i) {
            m_placeToWritten.insert(m_written[i].place, int(i));
            m_cx.sheetNames << m_written[i].formulaName;
        }
        setUpFormats();
        setUpNames();
        // (the sheets are built one after another into a single buffer; the globals, which need what the sheets turned up, are built after, and the two put together once)
        QByteArray sheetBytes;
        QVector<qsizetype> sheetSizes;
        for (size_t i = 0; i < m_written.size(); ++i) {
            if (m_cancelled && m_cancelled()) {
                *error = QCoreApplication::translate("Sheet", "The save was stopped.");
                return false;
            }
            const qsizetype before = sheetBytes.size();
            if (!buildSheet(int(i), &sheetBytes, error)) return false;
            if (!error->isEmpty()) return false;
            sheetSizes.append(sheetBytes.size() - before);
        }
        const QByteArray names = namesRecords();
        QByteArray stream = buildGlobals(names, sheetSizes);
        stream.reserve(stream.size() + sheetBytes.size() + 4096);
        stream.append(sheetBytes);
        sheetBytes.clear();
        sheetBytes.squeeze();
        if (stream.size() > (qint64(1) << 30)) {
            *error = QCoreApplication::translate("Sheet", "The workbook is larger than the old format can hold.");
            return false;
        }
        if (stream.size() < 4096) stream.append(QByteArray(4096 - stream.size(), '\0'));   // (a stream of the smallest size lies in whole sectors, not in the container's small-stream area)
        jp::cfb::File file;
        jp::cfb::Entry root;
        root.type = jp::cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        root.children = {1};
        jp::cfb::Entry wbs;
        wbs.type = jp::cfb::Entry::Stream;
        wbs.name = QStringLiteral("Workbook");
        wbs.data = stream;
        file.entries = {root, wbs};
        *out = jp::cfb::write(file);
        if (m_report) {
            m_report->notKept = m_features.result();
            if (m_formulasAsValues > 0) m_report->notKept << QCoreApplication::translate("Sheet", "%1 formulas the format cannot hold (they keep the value they came to)").arg(m_formulasAsValues);
            m_report->formulas = m_formulas;
            m_report->formulasAsValues = m_formulasAsValues;
            m_report->cells = m_cellsWritten;
            m_report->sheets = qint64(m_written.size());
            m_report->trimmed << m_trimmed;
            m_report->formulaReasons = oldsave::topReasons(m_reasons);
            std::sort(m_report->notKept.begin(), m_report->notKept.end());
        }
        return true;
    }

private:
    const Workbook &m_book;
    CopyReport *m_report;
    std::function<bool()> m_cancelled;
    int m_sinceLook = 0;
    bool m_stopped = false;
    // Whether the save is to stop: the caller is asked once in a few thousand cells, not only between rows.
    bool stopNow()
    {
        if (!m_cancelled) return false;
        if (m_stopped) return true;
        if (++m_sinceLook < 2048) return false;
        m_sinceLook = 0;
        m_stopped = m_cancelled();
        return m_stopped;
    }
    oldsave::Features m_features;
    std::vector<Written> m_written;
    QHash<int, int> m_placeToWritten;
    XlsCompileContext m_cx;
    QStringList m_trimmed;
    QMap<QString, qint64> m_reasons;
    qint64 m_cellsWritten = 0, m_formulas = 0, m_formulasAsValues = 0, m_textTotal = 0;
    // the strings
    QVector<QString> m_sst;
    QHash<QString, int> m_sstIndex;
    qint64 m_sstRefs = 0;
    // the external sheets
    struct Xti {
        int supbook, first, last;
    };
    QVector<Xti> m_xtis;
    QHash<qint64, int> m_xtiIndex;
    QStringList m_addIns;     // the functions of the add-in set that formulas call, in the order of their names in the file
    int m_addInXti = -1;
    // the colors
    QVector<quint32> m_palette;
    QHash<quint32, int> m_paletteIndex;
    bool m_paletteFull = false;
    // the fonts, number formats, and cell formats
    QVector<QByteArray> m_fontRecords;
    QHash<QByteArray, int> m_fontIndex;
    QHash<quint32, int> m_fontOfModel;
    QVector<QPair<int, QString>> m_formatRecords;
    QHash<QString, int> m_formatIdOfCode;
    QVector<QByteArray> m_xfRecords;
    QHash<QByteArray, int> m_xfIndex;
    QHash<quint32, int> m_xfOfModel;
    bool m_fontsFull = false, m_xfsFull = false, m_formatsFull = false;
    struct NameRec {
        QString name;
        int builtin = -1;
        int scope = -1;   // written place of the sheet, -1 for the workbook
        QString ref;
        bool hidden = false;
        QByteArray rgce, extra;   // the tokens of the definition
        bool broken = false;      // the form cannot hold the definition: it stands for a reference that is gone, and a formula that uses it is written as its value
        QString why;
    };
    std::vector<NameRec> m_names;

    void trim(const QString &what) { m_trimmed << what; }

    // ---- colors

    quint32 rgbOf(const ColorSpec &c, bool *automatic) const
    {
        const quint32 r = m_book.styles().resolve(c, 0xFFFFFFFFu);
        *automatic = !c.isSet() || r == 0xFFFFFFFFu;
        return r & 0xFFFFFF;
    }
    // The place in the palette (counted from 8) of a color; the palette is the 56 colors the workbook uses first, and any color after those is the nearest of them.
    int paletteIcv(quint32 rgb)
    {
        auto it = m_paletteIndex.constFind(rgb);
        if (it != m_paletteIndex.cend()) return it.value();
        if (m_palette.size() < 56) {
            m_palette.append(rgb);
            const int icv = 8 + int(m_palette.size()) - 1;
            m_paletteIndex.insert(rgb, icv);
            return icv;
        }
        if (!m_paletteFull) {
            m_paletteFull = true;
            trim(QCoreApplication::translate("Sheet", "colors past the 56 the format holds (each is the nearest of the first 56)"));
        }
        int best = 8, bestD = std::numeric_limits<int>::max();
        for (int i = 0; i < m_palette.size(); ++i) {
            const int dr = int((rgb >> 16) & 0xFF) - int((m_palette[i] >> 16) & 0xFF), dg = int((rgb >> 8) & 0xFF) - int((m_palette[i] >> 8) & 0xFF),
                      db = int(rgb & 0xFF) - int(m_palette[i] & 0xFF);
            const int d = dr * dr + dg * dg + db * db;
            if (d < bestD) {
                bestD = d;
                best = 8 + i;
            }
        }
        m_paletteIndex.insert(rgb, best);
        return best;
    }
    int colorIcv(const ColorSpec &c, int automatic)
    {
        bool autoColor = false;
        const quint32 rgb = rgbOf(c, &autoColor);
        if (autoColor) return automatic;
        return paletteIcv(rgb);
    }

    // ---- fonts, number formats, cell formats

    void setUpFormats()
    {
        // the four fonts every file starts with are the workbook's usual font; the one at place 4 is never in a file
        const FontSpec &base = m_book.styles().font(0);
        const QByteArray rec = fontRecord(base);
        for (int i = 0; i < 4; ++i) m_fontRecords.append(rec);
        m_fontIndex.insert(rec, 0);
        m_fontOfModel.insert(0, 0);
        // the fifteen formats of styles, all the first (the usual look), and the default look of cells (the sixteenth)
        W w;
        w.u16(0).u16(0).u16(0xFFF5).u8(0x20).u8(0).u8(0).u8(0).u16(0).u16(0).u32(0).u16(0x20C0);
        for (int i = 0; i < 15; ++i) m_xfRecords.append(w.bytes());
        m_xfRecords.append(xfRecord(0));
        m_xfIndex.insert(m_xfRecords[15], 15);
        m_xfOfModel.insert(0, 15);
    }

    QByteArray fontRecord(const FontSpec &f)
    {
        W w;
        const int height = int(std::lround(std::max(1.0, std::min(f.size, 409.0)) * 20));
        w.u16(height).u16((f.italic ? 2 : 0) | (f.strike ? 8 : 0)).u16(colorIcv(f.color, 0x7FFF)).u16(f.bold ? 700 : 400).u16(f.vertAlign == 1 ? 1 : f.vertAlign == 2 ? 2 : 0);
        const int underline = f.underline == Underline::Single ? 1 : f.underline == Underline::Double ? 2 : f.underline == Underline::SingleAccounting ? 0x21 : f.underline == Underline::DoubleAccounting ? 0x22 : 0;
        QString name = m_book.styles().fontNameOf(f);
        if (name.isEmpty()) name = QStringLiteral("Arial");
        name = name.left(kMaxFontNameFileChars);
        w.u8(underline).u8(f.family >= 0 && f.family < 256 ? f.family : 0).u8(f.charset >= 0 && f.charset < 256 ? f.charset : 0).u8(0).u8(int(name.size()));
        w.u8(isWide(name) ? 1 : 0);
        const bool wide = isWide(name);
        for (const QChar c : name) {
            if (wide) w.u16(c.unicode());
            else w.u8(c.unicode());
        }
        return w.bytes();
    }

    int fontFor(quint32 modelFont)
    {
        auto it = m_fontOfModel.constFind(modelFont);
        if (it != m_fontOfModel.cend()) return it.value();
        const QByteArray rec = fontRecord(m_book.styles().font(modelFont));
        auto same = m_fontIndex.constFind(rec);
        int at;
        if (same != m_fontIndex.cend()) {
            at = same.value();
        } else if (m_fontRecords.size() >= kMaxXlsFonts) {
            if (!m_fontsFull) {
                m_fontsFull = true;
                trim(QCoreApplication::translate("Sheet", "fonts past the %1 the format holds (the cells have the first font)").arg(kMaxXlsFonts));
            }
            at = 0;
        } else {
            m_fontRecords.append(rec);
            at = int(m_fontRecords.size()) - 1;
            m_fontIndex.insert(rec, at);
        }
        m_fontOfModel.insert(modelFont, at);
        return at;
    }
    // The file's number for a font held at that place of the record list (the place 4 is never used: the fonts after it are one higher).
    static int fileFontNumber(int place) { return place >= 4 ? place + 1 : place; }

    int formatIdFor(quint32 modelXf)
    {
        const Styles &st = m_book.styles();
        const qint32 id = st.numFmtId(modelXf);
        if (id == 0) return 0;
        if (id < 164 && !st.numFmts.contains(id)) return id;
        const QString code = st.formatCode(modelXf, QLocale::c());
        if (code.isEmpty() || code == QLatin1String("General")) return 0;
        auto it = m_formatIdOfCode.constFind(code);
        if (it != m_formatIdOfCode.cend()) return it.value();
        if (m_formatRecords.size() >= kMaxXlsNumberFormats || code.size() > 255) {
            if (!m_formatsFull) {
                m_formatsFull = true;
                trim(QCoreApplication::translate("Sheet", "number formats past the %1 the format holds (the cells show General)").arg(kMaxXlsNumberFormats));
            }
            return 0;
        }
        const int newId = 164 + int(m_formatRecords.size());
        m_formatRecords.append({newId, code});
        m_formatIdOfCode.insert(code, newId);
        return newId;
    }

    QByteArray xfRecord(quint32 modelXf)
    {
        const Styles &st = m_book.styles();
        const CellFormat &f = st.format(modelXf);
        W w;
        w.u16(fileFontNumber(fontFor(f.font))).u16(formatIdFor(modelXf));
        w.u16((f.locked ? 1 : 0) | (f.formulaHidden ? 2 : 0) | (f.quotePrefix ? 8 : 0));
        const int valign = f.vertical == VAlign::Top ? 0 : f.vertical == VAlign::Center ? 1 : f.vertical == VAlign::Justify ? 3 : f.vertical == VAlign::Distributed ? 4 : 2;
        w.u8((int(f.horizontal) & 7) | (f.wrap ? 8 : 0) | (valign << 4));
        w.u8(f.rotation);
        w.u8((std::min<int>(f.indent, 15)) | (f.shrink ? 16 : 0) | ((f.readingOrder & 3) << 6));
        w.u8(0xFC);
        const BorderSpec &b = st.border(f.border);
        const FillSpec &fill = st.fill(f.fill);
        auto side = [&](const BorderSide &s) { return s.style == BorderStyle::None ? 0 : int(s.style); };
        auto sideColor = [&](const BorderSide &s) { return s.style == BorderStyle::None ? 0 : colorIcv(s.color, 64); };
        w.u16(side(b.left) | (side(b.right) << 4) | (side(b.top) << 8) | (side(b.bottom) << 12));
        w.u16(sideColor(b.left) | (sideColor(b.right) << 7) | (b.diagonalDown ? 0x4000 : 0) | (b.diagonalUp ? 0x8000 : 0));
        const int pattern = int(fill.pattern);
        w.u32(quint32(sideColor(b.top)) | (quint32(sideColor(b.bottom)) << 7) | (quint32(sideColor(b.diagonal)) << 14) | (quint32(side(b.diagonal)) << 21) | (quint32(pattern) << 26));
        int fore = 64, back = 65;
        if (fill.pattern != PatternKind::None) {
            fore = colorIcv(fill.fg, 64);
            back = fill.pattern == PatternKind::Solid ? 65 : colorIcv(fill.bg, 65);
        }
        w.u16(fore | (back << 7));
        return w.bytes();
    }

    int xfFor(quint32 modelXf)
    {
        auto it = m_xfOfModel.constFind(modelXf);
        if (it != m_xfOfModel.cend()) return it.value();
        const QByteArray rec = xfRecord(modelXf);
        int at;
        auto same = m_xfIndex.constFind(rec);
        if (same != m_xfIndex.cend()) {
            at = same.value();
        } else if (m_xfRecords.size() >= kMaxXlsFormats) {
            if (!m_xfsFull) {
                m_xfsFull = true;
                trim(QCoreApplication::translate("Sheet", "cell formats past the %1 the format holds (the cells have the usual look)").arg(kMaxXlsFormats));
            }
            at = 15;
        } else {
            m_xfRecords.append(rec);
            at = int(m_xfRecords.size()) - 1;
            m_xfIndex.insert(rec, at);
        }
        m_xfOfModel.insert(modelXf, at);
        return at;
    }

    // ---- strings and sheets in formulas

    int sstFor(const QString &s)
    {
        ++m_sstRefs;
        auto it = m_sstIndex.constFind(s);
        if (it != m_sstIndex.cend()) return it.value();
        m_sst.append(s);
        m_sstIndex.insert(s, int(m_sst.size()) - 1);
        return int(m_sst.size()) - 1;
    }

    int xtiFor(int first, int last)
    {
        const qint64 key = (qint64(first) << 20) | qint64(last);
        auto it = m_xtiIndex.constFind(key);
        if (it != m_xtiIndex.cend()) return it.value();
        if (m_xtis.size() >= 60000) return -1;
        m_xtis.append({0, first, last});
        m_xtiIndex.insert(key, int(m_xtis.size()) - 1);
        return int(m_xtis.size()) - 1;
    }

    // ---- names

    static int builtinNameCode(const QString &name)
    {
        static const char *const names[] = {"Consolidate_Area", "Auto_Open", "Auto_Close", "Extract", "Database", "Criteria", "Print_Area", "Print_Titles", "Recorder", "Data_Form",
                                            "Auto_Activate", "Auto_Deactivate", "Sheet_Title", "_FilterDatabase"};
        if (!name.startsWith(QLatin1String("_xlnm."), Qt::CaseInsensitive)) return -1;
        const QString tail = name.mid(6);
        for (int i = 0; i < int(sizeof(names) / sizeof(names[0])); ++i)
            if (tail.compare(QString::fromLatin1(names[i]), Qt::CaseInsensitive) == 0) return i;
        return -2;
    }

    void setUpNames()
    {
        const WorkbookInfo &info = m_book.info();
        qint64 dropped = 0;
        for (const DefinedName &d : info.definedNames) {
            if (int(m_names.size()) >= kMaxXlsNames) {
                ++dropped;
                continue;
            }
            NameRec n;
            n.name = d.name;
            n.builtin = builtinNameCode(d.name);
            if (n.builtin == -2) {
                ++dropped;
                continue;
            }
            n.hidden = d.hidden || n.builtin == 13;
            n.ref = d.ref;
            if (d.localSheet >= 0) {
                const auto at = m_placeToWritten.constFind(d.localSheet);
                if (at == m_placeToWritten.cend()) {
                    ++dropped;
                    continue;
                }
                n.scope = at.value();
            }
            if (n.builtin < 0 && (n.name.isEmpty() || n.name.size() > 255)) {
                ++dropped;
                continue;
            }
            m_names.push_back(n);
        }
        if (dropped > 0) trim(QCoreApplication::translate("Sheet", "%1 names the format cannot hold").arg(dropped));
        for (size_t i = 0; i < m_names.size(); ++i) {
            const NameRec &n = m_names[i];
            if (n.builtin >= 0) continue;
            const QString key = QString::number(n.scope) + QLatin1Char('|') + n.name.toLower();
            if (!m_cx.names.contains(key)) m_cx.names.insert(key, int(i) + 1);
        }
        m_cx.xti = [this](int a, int b) { return xtiFor(a, b); };
        m_cx.addIn = [this](const QString &name, int *ixti, int *index) {
            int at = int(m_addIns.indexOf(name));
            if (at < 0) {
                if (m_addIns.size() >= 255) return false;
                m_addIns.append(name);
                at = int(m_addIns.size()) - 1;
            }
            if (m_addInXti < 0) {
                if (m_xtis.size() >= 60000) return false;
                m_xtis.append({1, 0xFFFE, 0xFFFE});   // (the second book of the list is the add-in's, and no sheet of it is meant)
                m_addInXti = int(m_xtis.size()) - 1;
            }
            *ixti = m_addInXti;
            *index = at + 1;
            return true;
        };
        compileNames();
    }

    // The tokens of every name's definition, before any formula is written: a name the form cannot hold (a cell of another workbook, a definition too long for a record) is marked, and so is a
    // name defined through one (the marks are gone over until none is new).
    void compileNames()
    {
        for (int pass = 0; pass < 8; ++pass) {
            bool news = false;
            for (size_t i = 0; i < m_names.size(); ++i) {
                NameRec &n = m_names[i];
                if (n.broken) continue;
                XlsCompileContext cx = m_cx;
                cx.isName = true;
                cx.sheet = n.scope;
                QByteArray rgce, extra;
                QString why;
                const qsizetype head = 15 + (isWide(n.name) ? 2 : 1) * n.name.size();
                bool fits = xlsCompileFormula(n.ref, cx, &rgce, &extra, &why);
                if (fits && head + rgce.size() + extra.size() > kMaxRecord) {
                    fits = false;
                    why = QCoreApplication::translate("Sheet", "The formula is longer than the old format allows.");
                }
                if (fits) {
                    n.rgce = rgce;
                    n.extra = extra;
                    continue;
                }
                n.broken = true;
                n.why = why;
                m_cx.brokenNames.insert(int(i) + 1);
                news = true;
            }
            if (!news) break;
        }
        for (NameRec &n : m_names) {
            if (!n.broken) continue;
            // (a name the format cannot hold the definition of stands for a reference that is gone, as a real file has it: the token for it through the list of external sheets)
            W r;
            r.u8(0x3C).u16(std::max(0, xtiFor(0, 0))).u16(0).u16(0);
            n.rgce = r.bytes();
            n.extra.clear();
            trim(QCoreApplication::translate("Sheet", "the name %1 (%2)").arg(n.name, n.why));
        }
    }

    QByteArray namesRecords()
    {
        QByteArray out;
        for (size_t i = 0; i < m_names.size(); ++i) {
            const NameRec &n = m_names[i];
            const QByteArray &rgce = n.rgce, &extra = n.extra;
            W w;
            w.u16((n.builtin >= 0 ? 0x20 : 0) | (n.hidden ? 1 : 0)).u8(0);
            const QString text = n.builtin >= 0 ? QString(QChar(n.builtin)) : n.name;
            const bool wide = n.builtin < 0 && isWide(text);
            w.u8(int(text.size())).u16(int(rgce.size())).u16(0).u16(n.scope >= 0 ? n.scope + 1 : 0).u8(0).u8(0).u8(0).u8(0);
            w.u8(wide ? 1 : 0);
            for (const QChar c : text) {
                if (wide) w.u16(c.unicode());
                else w.u8(c.unicode());
            }
            w.raw(rgce).raw(extra);
            putRecord(&out, 0x0018, w.bytes());
        }
        return out;
    }

    // ---- the sheets

    // An array formula's or a data table's cells: the region they cover, and the record (the ARRAY or TABLE record) that follows the formula record of the first cell.
    struct Region {
        CellRange range;
        bool table = false;
        QByteArray record;
    };

    static QByteArray bof(int type)
    {
        W w;
        w.u16(0x0600).u16(type).u16(0x0DBB).u16(0x07CC).u32(0).u32(6);
        QByteArray r;
        putRecord(&r, 0x0809, w.bytes());
        return r;
    }

    void put(QByteArray *out, quint16 id, const W &w) { putRecord(out, id, w.bytes()); }

    bool buildSheet(int index, QByteArray *dst, QString *error)
    {
        const Written &w = m_written[size_t(index)];
        m_currentSheet = index;
        Sheet sheet;
        if (w.place >= 0) {
            m_book.readSheet(w.place, &sheet);
            if (!sheet.error.isEmpty() && sheet.cells.cellCount() == 0 && sheet.error.contains(QLatin1String("not in the file"))) {
                *error = oldsave::sheetNote(w.name, sheet.error);
                return false;
            }
            m_features.note(sheet);
        }
        QByteArray &out = *dst;
        out += bof(0x0010);
        const WorkbookInfo &info = m_book.info();
        // how the workbook calculates (kept with the first sheet, where the other program has it)
        {
            W m;
            m.u16(info.calcMode == CalcMode::Manual ? 0 : info.calcMode == CalcMode::AutoNoTable ? 0xFFFF : 1);
            put(&out, 0x000D, m);
            put(&out, 0x000C, W().u16(info.iterate ? std::max(1, std::min(info.iterateCount, 32767)) : 100));
            put(&out, 0x000F, W().u16(1));
            put(&out, 0x0011, W().u16(info.iterate ? 1 : 0));
            put(&out, 0x0010, W().f64(info.iterate ? info.iterateDelta : 0.001));
            put(&out, 0x005F, W().u16(1));
        }
        put(&out, 0x002A, W().u16(sheet.page.headings ? 1 : 0));
        put(&out, 0x002B, W().u16(sheet.page.gridLines ? 1 : 0));
        put(&out, 0x0082, W().u16(1));
        const int rowLevel = sheet.format.outlineLevelRow, colLevel = sheet.format.outlineLevelCol;
        put(&out, 0x0080, W().u16(rowLevel > 0 ? 17 + 12 * (rowLevel - 1) : 0).u16(colLevel > 0 ? 17 + 12 * (colLevel - 1) : 0).u16(rowLevel > 0 ? rowLevel + 1 : 0).u16(colLevel > 0 ? colLevel + 1 : 0));
        {
            const SheetFormat &f = sheet.format;
            put(&out, 0x0225, W().u16((f.customHeight ? 1 : 0) | (f.zeroHeight ? 2 : 0)).u16(int(std::lround(std::max(1.0, std::min(f.defaultRowHeight, 409.0)) * 20))));
            int ws = 0x0001 | 0x0400 | (f.summaryBelow ? 0x40 : 0) | (f.summaryRight ? 0x80 : 0) | (rowLevel > 0 || colLevel > 0 ? 0x200 : 0) | (sheet.page.fitToPage ? 0x100 : 0);
            if (sheet.transitionEvaluation) ws |= 0x4000;
            if (sheet.transitionEntry) ws |= 0x8000;
            put(&out, 0x0081, W().u16(ws));
            writePageSetup(&out, sheet.page);
            put(&out, 0x0055, W().u16(std::max(1, std::min(f.baseColWidth, 255))));
            if (f.defaultColWidth > 0) put(&out, 0x0099, W().u16(int(std::lround(std::min(f.defaultColWidth, 255.0) * 256))));
        }
        // the columns
        qint64 colsCut = 0;
        {
            QVector<ColumnSpan> cols = sheet.columns;
            std::stable_sort(cols.begin(), cols.end(), [](const ColumnSpan &a, const ColumnSpan &b) { return a.first < b.first; });
            quint32 lastEnd = 0;
            bool any = false;
            for (const ColumnSpan &c : std::as_const(cols)) {
                if (c.first >= quint32(kXlsMaxCols)) {
                    ++colsCut;
                    continue;
                }
                const quint32 first = any ? std::max(c.first, lastEnd) : c.first;
                const quint32 last = std::min<quint32>(c.last, kXlsMaxCols - 1);
                if (last < first) continue;
                const double width = c.width >= 0 ? c.width : (sheet.format.defaultColWidth > 0 ? sheet.format.defaultColWidth : 8.43);
                const int flags = (c.hidden ? 1 : 0) | (c.customWidth ? 2 : 0) | (c.bestFit ? 4 : 0) | ((std::min<int>(c.outline, 7)) << 8) | (c.collapsed ? 0x1000 : 0);
                put(&out, 0x007D, W().u16(int(first)).u16(int(last)).u16(int(std::lround(std::min(width, 255.0) * 256))).u16(c.style ? xfFor(c.style) : 15).u16(flags).u16(0));
                lastEnd = last + 1;
                any = true;
            }
        }
        // the cells and rows
        qint64 cellsPastCap = 0;
        const std::vector<PlacedCell> cells = oldsave::sortedCells(sheet, std::max<qint64>(0, kMaxCopyCells - m_cellsWritten) + 2000000, &cellsPastCap);
        QMap<quint32, RowSpec> specs;
        qint64 rowsCut = 0;
        for (const RowSpec &s : sheet.placedRowSpecs()) {
            if (s.row >= quint32(kXlsMaxRows)) {
                ++rowsCut;
                continue;
            }
            specs.insert(s.row, s);
        }
        // arrays and data tables: the cells they cover, and the record that holds each one's formula (made now, so that a region the format cannot hold is written as values)
        std::vector<Region> regions;
        std::map<quint64, int> regionOf;   // by row, then column, of every cell the regions cover
        qint64 regionCells = 0;
        for (const PlacedCell &pc : cells) {
            if (!(pc.cell.flags & (ArrayFormula | DataTable | SpillFormula))) continue;
            Region r;
            r.range = sheet.formulaRange(pc.cell);
            // (a formula that spills is an array formula over the block it filled, which the old form holds when it has the functions; a block of one cell is a formula like any other)
            if ((pc.cell.flags & SpillFormula) && r.range.cellCount() <= 1) continue;
            r.table = pc.cell.flags & DataTable;
            if (!r.range.valid || r.range.row1 != pc.row || r.range.col1 != pc.col || r.range.row2 >= quint32(kXlsMaxRows) || r.range.col2 >= quint32(kXlsMaxCols)
                || regionCells + r.range.cellCount() > 2000000) {
                ++m_formulasAsValues;
                continue;
            }
            r.record = regionRecord(r.range, r.table, sheet.formulaText(pc.cell));
            if (r.record.isEmpty()) {
                ++m_formulasAsValues;
                continue;
            }
            regionCells += r.range.cellCount();
            regions.push_back(r);
            for (quint32 rr = r.range.row1; rr <= r.range.row2; ++rr)
                for (quint32 cc = r.range.col1; cc <= r.range.col2; ++cc) regionOf[(quint64(rr) << 16) | cc] = int(regions.size()) - 1;
        }
        // (the used area is known when the rows are done: its record is put in now, and filled in then)
        const qsizetype dimensionsAt = out.size();
        put(&out, 0x0200, W().u32(0).u32(0).u16(0).u16(0).u16(0));
        qint64 cellsCut = 0, textsCut = 0, firstRow = -1, lastRow = -1, firstCol = kXlsMaxCols, lastCol = -1;
        {
            size_t i = 0;
            const size_t n = cells.size();
            auto rowCells = [&](size_t from, size_t *to) {
                size_t j = from;
                while (j < n && cells[j].row == cells[from].row) ++j;
                *to = j;
            };
            // the rows to write: those with cells, and those with a size of their own, in order
            QSet<quint32> cellRows;
            QVector<quint32> rowList;
            {
                size_t j = 0;
                while (j < n) {
                    size_t e;
                    rowCells(j, &e);
                    if (cells[j].row < quint32(kXlsMaxRows)) {
                        cellRows.insert(cells[j].row);
                        rowList.append(cells[j].row);
                    } else {
                        for (size_t k = j; k < e; ++k)
                            if (cells[k].cell.type != CellType::Empty || cells[k].cell.hasFormula()) ++cellsCut;
                    }
                    j = e;
                }
                for (auto it = specs.cbegin(); it != specs.cend(); ++it)
                    if (!cellRows.contains(it.key())) rowList.append(it.key());
                std::sort(rowList.begin(), rowList.end());
            }
            // the cells, by row
            QHash<quint32, QPair<size_t, size_t>> spanOfRow;
            while (i < n) {
                size_t e;
                rowCells(i, &e);
                spanOfRow.insert(cells[i].row, {i, e});
                i = e;
            }
            qsizetype r = 0;
            while (r < rowList.size()) {
                if (m_cancelled && (r % 4096 == 0) && m_cancelled()) {
                    *error = QCoreApplication::translate("Sheet", "The save was stopped.");
                    return false;
                }
                // a block of up to 32 rows: their ROW records, then their cells
                const quint32 blockStart = rowList[r] / 32 * 32;
                QByteArray rows, cellBytes;
                for (; r < rowList.size() && rowList[r] < blockStart + 32; ++r) {
                    const quint32 row = rowList[r];
                    QByteArray rowCellBytes;
                    int minCol = kXlsMaxCols, maxCol = -1;
                    const auto sp = spanOfRow.constFind(row);
                    if (sp != spanOfRow.cend()) {
                        writeRowCells(sheet, cells, sp->first, sp->second, regions, regionOf, &rowCellBytes, &minCol, &maxCol, &cellsCut, &textsCut);
                        if (m_stopped) {
                            *error = QCoreApplication::translate("Sheet", "The save was stopped.");
                            return false;
                        }
                    }
                    const auto spec = specs.constFind(row);
                    W rw;
                    int height = 0xFF | 0x8000;
                    int flags = 0x100;
                    int xf = 15;
                    if (spec != specs.cend()) {
                        if (spec->height >= 0) height = int(std::lround(std::min(spec->height, 409.5) * 20)) & 0x7FFF;
                        flags |= (spec->hidden ? 0x20 : 0) | (spec->customHeight ? 0x40 : 0) | (spec->customFormat ? 0x80 : 0) | std::min<int>(spec->outline, 7) | (spec->collapsed ? 0x10 : 0);
                        if (spec->customFormat) xf = xfFor(spec->style);
                    } else if (sheet.format.defaultRowHeight > 0) {
                        height = int(std::lround(sheet.format.defaultRowHeight * 20)) & 0x7FFF;
                        height |= 0x8000;
                    }
                    rw.u16(int(row)).u16(maxCol >= 0 ? minCol : 0).u16(maxCol >= 0 ? maxCol + 1 : 0).u16(height).u16(0).u16(0).u16(flags).u16(xf);
                    putRecord(&rows, 0x0208, rw.bytes());
                    cellBytes += rowCellBytes;
                    if (maxCol >= 0) {
                        if (firstRow < 0) firstRow = row;
                        lastRow = row;
                        firstCol = std::min<qint64>(firstCol, minCol);
                        lastCol = std::max<qint64>(lastCol, maxCol);
                    }
                }
                out += rows;
                out += cellBytes;
            }
        }
        {
            W d;
            d.u32(firstRow >= 0 ? quint32(firstRow) : 0).u32(lastRow >= 0 ? quint32(lastRow + 1) : 0).u16(lastCol >= 0 ? int(firstCol) : 0).u16(lastCol >= 0 ? int(lastCol + 1) : 0).u16(0);
            out.replace(dimensionsAt + 4, int(d.size()), d.bytes());
        }
        // the view
        {
            const SheetView &v = sheet.view;
            int g = 0x20 | 0x80;
            if (v.showGridLines) g |= 0x02;
            if (v.showHeadings) g |= 0x04;
            if (v.frozen) g |= 0x08 | 0x100;
            if (v.showZeros) g |= 0x10;
            if (v.rightToLeft) g |= 0x40;
            if (index == writtenIndex(info.activeTab) || v.tabSelected) g |= 0x200;
            if (index == writtenIndex(info.activeTab)) g |= 0x400;
            if (v.kind == SheetView::Kind::PageBreaks) g |= 0x800;
            const int zoom = std::max(10, std::min(v.normalZoom > 0 ? v.normalZoom : v.zoom, 400));
            put(&out, 0x023E, W().u16(g).u16(int(std::min<quint32>(v.topRow, 65535))).u16(int(std::min<quint32>(v.leftCol, 255))).u32(0x40).u16(0).u16(zoom == 100 ? 0 : zoom).u32(0));
            if (zoom != 100) put(&out, 0x00A0, W().u16(zoom).u16(100));
            if (v.frozen && (v.frozenRows > 0 || v.frozenCols > 0)) {
                const int rows = int(std::min<quint32>(v.frozenRows, 65535)), cols = int(std::min<quint32>(v.frozenCols, 255));
                const int top = int(std::min<quint32>(v.paneRow ? v.paneRow : v.frozenRows, 65535)), left = int(std::min<quint32>(v.paneCol ? v.paneCol : v.frozenCols, 255));
                const int active = rows > 0 && cols > 0 ? 0 : rows > 0 ? 2 : 1;
                put(&out, 0x0041, W().u16(cols).u16(rows).u16(top).u16(left).u16(active));
                const int ar = int(std::min<quint32>(v.activeRow, 65535)), ac = int(std::min<quint32>(v.activeCol, 255));
                // a selection for each pane the split makes (the format wants one for every pane: top left 3, top right 1, bottom left 2, bottom right 0), the active pane's holding the active cell and
                // each of the others the cell its pane begins at
                const int topRow0 = int(std::min<quint32>(v.topRow, 65535)), leftCol0 = int(std::min<quint32>(v.leftCol, 255));
                for (const int pane : {3, 1, 2, 0}) {
                    const bool right = pane == 1 || pane == 0, below = pane == 2 || pane == 0;
                    if ((right && cols == 0) || (below && rows == 0)) continue;
                    int r = ar, c = ac;
                    if (pane != active) {
                        r = below ? top : topRow0;
                        c = right ? left : leftCol0;
                    }
                    put(&out, 0x001D, W().u8(pane).u16(r).u16(c).u16(0).u16(1).u16(r).u16(r).u8(c).u8(c));
                }
            } else {
                const int ar = int(std::min<quint32>(v.activeRow, 65535)), ac = int(std::min<quint32>(v.activeCol, 255));
                put(&out, 0x001D, W().u8(3).u16(ar).u16(ac).u16(0).u16(1).u16(ar).u16(ar).u8(ac).u8(ac));
            }
        }
        // merged ranges
        {
            QVector<CellRange> merges;
            qint64 mergesCut = 0;
            for (const CellRange &m : sheet.merges) {
                if (!m.valid || m.row2 >= quint32(kXlsMaxRows) || m.col2 >= quint32(kXlsMaxCols) || int(merges.size()) >= kMaxXlsMerges) {
                    ++mergesCut;
                    continue;
                }
                merges.append(m);
            }
            for (qsizetype at = 0; at < merges.size(); at += 1026) {   // (a record holds 1,026 ranges at most)
                const qsizetype n = std::min<qsizetype>(1026, merges.size() - at);
                W m;
                m.u16(int(n));
                for (qsizetype k = 0; k < n; ++k) m.u16(int(merges[at + k].row1)).u16(int(merges[at + k].row2)).u16(int(merges[at + k].col1)).u16(int(merges[at + k].col2));
                put(&out, 0x00E5, m);
            }
            if (mergesCut > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 merged ranges the format cannot hold").arg(mergesCut)));
        }
        // links: one record for each range
        {
            qint64 linksCut = 0;
            int written = 0;
            for (const LinkSpec &l : sheet.links) {
                if (!l.range.valid || l.range.row2 >= quint32(kXlsMaxRows) || l.range.col2 >= quint32(kXlsMaxCols) || written >= kMaxCopyHyperlinks || (l.target.isEmpty() && l.location.isEmpty())
                    || l.target.size() > 2000 || l.location.size() > 2000) {
                    ++linksCut;
                    continue;
                }
                auto utf16z = [](W *w, const QString &t) {
                    w->u32(quint32(t.size() + 1));
                    for (const QChar c : t) w->u16(c.unicode());
                    w->u16(0);
                };
                static const char kLink[16] = {char(0xD0), char(0xC9), char(0xEA), char(0x79), char(0xF9), char(0xBA), char(0xCE), char(0x11), char(0x8C), char(0x82), 0x00, char(0xAA), 0x00, char(0x4B), char(0xA9), 0x0B};
                static const char kUrl[16] = {char(0xE0), char(0xC9), char(0xEA), char(0x79), char(0xF9), char(0xBA), char(0xCE), char(0x11), char(0x8C), char(0x82), 0x00, char(0xAA), 0x00, char(0x4B), char(0xA9), 0x0B};
                W h;
                h.u16(int(l.range.row1)).u16(int(l.range.row2)).u16(int(l.range.col1)).u16(int(l.range.col2)).raw(QByteArray(kLink, 16)).u32(2);
                const QString display = l.display.left(255);
                const quint32 flags = (l.outside() ? 0x03 : 0) | (!l.location.isEmpty() ? 0x08 : 0) | (!display.isEmpty() ? 0x14 : 0);
                h.u32(flags);
                if (!display.isEmpty()) utf16z(&h, display);
                if (l.outside()) {
                    h.raw(QByteArray(kUrl, 16)).u32(quint32((l.target.size() + 1) * 2));
                    for (const QChar c : l.target) h.u16(c.unicode());
                    h.u16(0);
                }
                if (!l.location.isEmpty()) utf16z(&h, l.location);
                put(&out, 0x01B8, h);
                ++written;
            }
            if (linksCut > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 links the format cannot hold").arg(linksCut)));
        }
        put(&out, 0x000A, W());
        if (cellsCut > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 cells past row 65,536 or column IV (the format's last)").arg(cellsCut)));
        if (cellsPastCap > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 cells past the limit on cells in a copy").arg(cellsPastCap)));
        if (textsCut > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 texts cut to 32,767 characters").arg(textsCut)));
        if (rowsCut > 0 || colsCut > 0) trim(oldsave::sheetNote(w.name, QCoreApplication::translate("Sheet", "%1 row and column sizes past the format's grid").arg(rowsCut + colsCut)));
        return true;
    }

    // How the sheet prints: the breaks, the header and footer, centering, margins, and the paper (the records that follow WSBOOL).
    void writePageSetup(QByteArray *out, const PageSetup &p)
    {
        auto breaks = [&](quint16 id, const QVector<PageBreak> &list, quint32 limit) {
            QVector<PageBreak> manual;
            for (const PageBreak &b : list)
                if (b.manual && b.before >= 1 && b.before < limit && manual.size() < 1000) manual.append(b);
            if (manual.isEmpty()) return;
            W w;
            w.u16(int(manual.size()));
            for (const PageBreak &b : manual) w.u16(int(b.before)).u16(int(b.min)).u16(int(b.max ? b.max : (id == 0x001B ? 255 : 65535)));
            putRecord(out, id, w.bytes());
        };
        breaks(0x001B, p.rowBreaks, quint32(kXlsMaxRows));
        breaks(0x001A, p.colBreaks, quint32(kXlsMaxCols));
        auto text = [&](quint16 id, const QString &s) {
            const QString t = s.left(255);
            W w;
            if (!t.isEmpty()) w.str8(t, 2);
            putRecord(out, id, w.bytes());
        };
        text(0x0014, p.headerFooter.oddHeader);
        text(0x0015, p.headerFooter.oddFooter);
        put(out, 0x0083, W().u16(p.centerHorizontally ? 1 : 0));
        put(out, 0x0084, W().u16(p.centerVertically ? 1 : 0));
        put(out, 0x0026, W().f64(std::clamp(p.margins.left, 0.0, 100.0)));
        put(out, 0x0027, W().f64(std::clamp(p.margins.right, 0.0, 100.0)));
        put(out, 0x0028, W().f64(std::clamp(p.margins.top, 0.0, 100.0)));
        put(out, 0x0029, W().f64(std::clamp(p.margins.bottom, 0.0, 100.0)));
        int grbit = 0;
        if (p.pageOrder == PageOrder::OverThenDown) grbit |= 0x0001;
        if (p.orientation == PageOrientation::Portrait) grbit |= 0x0002;
        if (p.orientation == PageOrientation::Default) grbit |= 0x0040;
        if (p.blackAndWhite) grbit |= 0x0008;
        if (p.draft) grbit |= 0x0010;
        if (p.comments != PrintComments::None) grbit |= 0x0020 | (p.comments == PrintComments::AtEnd ? 0x0200 : 0);
        if (p.useFirstPageNumber) grbit |= 0x0080;
        grbit |= (p.errors == PrintErrors::Blank ? 1 : p.errors == PrintErrors::Dash ? 2 : p.errors == PrintErrors::NA ? 3 : 0) << 10;
        W setup;
        setup.u16(std::clamp(p.paperSize, 0, 32767)).u16(std::clamp(p.scale, kMinPageScale, kMaxPageScale)).u16(std::clamp(p.firstPageNumber, 0, 32767)).u16(std::clamp(p.fitToWidth, 0, 32767))
            .u16(std::clamp(p.fitToHeight, 0, 32767)).u16(grbit).u16(std::clamp(p.horizontalDpi, 1, 32767)).u16(std::clamp(p.verticalDpi, 1, 32767)).f64(std::clamp(p.margins.header, 0.0, 100.0)).f64(std::clamp(p.margins.footer, 0.0, 100.0)).u16(1);
        put(out, 0x00A1, setup);
        const HeaderFooter &h = p.headerFooter;
        if (h.differentOddEven || h.differentFirst || !h.evenHeader.isEmpty() || !h.evenFooter.isEmpty() || !h.firstHeader.isEmpty() || !h.firstFooter.isEmpty() || !h.scaleWithDoc || !h.alignWithMargins) {
            W w;
            w.u16(0x089C).u16(0).raw(QByteArray(8, '\0')).raw(QByteArray(16, '\0'));
            w.u16((h.differentOddEven ? 1 : 0) | (h.differentFirst ? 2 : 0) | (h.scaleWithDoc ? 4 : 0) | (h.alignWithMargins ? 8 : 0));
            const QString eh = h.evenHeader.left(255), ef = h.evenFooter.left(255), fh = h.firstHeader.left(255), ff = h.firstFooter.left(255);
            w.u16(int(eh.size())).u16(int(ef.size())).u16(int(fh.size())).u16(int(ff.size()));
            for (const QString &t : {eh, ef, fh, ff}) {
                if (t.isEmpty()) continue;
                const bool wide = isWide(t);
                w.u8(wide ? 1 : 0);
                for (const QChar c : t) {
                    if (wide) w.u16(c.unicode());
                    else w.u8(c.unicode());
                }
            }
            put(out, 0x089C, w);
        }
    }

    // One row's cells, in the records of the format.
    void writeRowCells(const Sheet &sheet, const std::vector<PlacedCell> &cells, size_t from, size_t to, const std::vector<Region> &regions, const std::map<quint64, int> &regionOf, QByteArray *out,
                       int *minCol, int *maxCol, qint64 *cellsCut, qint64 *textsCut)
    {
        const SharedStrings &shared = m_book.sharedStrings();
        std::vector<int> blankXf;
        int blankStart = -1;
        auto flushBlanks = [&]() {
            if (blankXf.empty()) return;
            const quint32 row = cells[from].row;
            if (blankXf.size() == 1) {
                putRecord(out, 0x0201, W().u16(int(row)).u16(blankStart).u16(blankXf[0]).bytes());
            } else {
                W m;
                m.u16(int(row)).u16(blankStart);
                for (int x : blankXf) m.u16(x);
                m.u16(blankStart + int(blankXf.size()) - 1);
                putRecord(out, 0x00BE, m.bytes());
            }
            blankXf.clear();
            blankStart = -1;
        };
        for (size_t i = from; i < to; ++i) {
            if (stopNow()) break;
            const PlacedCell &pc = cells[i];
            if (pc.col >= quint32(kXlsMaxCols) || pc.row >= quint32(kXlsMaxRows)) {
                if (pc.cell.type != CellType::Empty || pc.cell.hasFormula()) ++*cellsCut;
                continue;
            }
            if (m_cellsWritten >= kMaxCopyCells) {
                ++*cellsCut;
                continue;
            }
            const Cell &c = pc.cell;
            const bool isRegionCell = regionOf.count((quint64(pc.row) << 16) | pc.col) != 0;
            if (c.type == CellType::Empty && !c.hasFormula() && !isRegionCell) {
                if (c.style == 0) continue;
                const int xf = xfFor(c.style);
                if (!blankXf.empty() && blankStart + int(blankXf.size()) != int(pc.col)) flushBlanks();
                if (blankXf.empty()) blankStart = int(pc.col);
                blankXf.push_back(xf);
                *minCol = std::min<int>(*minCol, int(pc.col));
                *maxCol = std::max<int>(*maxCol, int(pc.col));
                ++m_cellsWritten;
                continue;
            }
            flushBlanks();
            *minCol = std::min<int>(*minCol, int(pc.col));
            *maxCol = std::max<int>(*maxCol, int(pc.col));
            ++m_cellsWritten;
            const int xf = xfFor(c.style);
            const int row = int(pc.row), col = int(pc.col);
            // the value
            QString text;
            if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) {
                bool cut = false;
                text = oldsave::cutText(oldsave::textOf(sheet, shared, c), &cut);
                if (cut) ++*textsCut;
                m_textTotal += text.size();
                if (m_textTotal > kMaxCopyTextTotal) text.clear();
            }
            const bool formula = c.hasFormula() || isRegionCell;
            if (!formula) {
                writeValue(out, row, col, xf, c, text);
                continue;
            }
            // a formula
            QByteArray rgce, extra, after;
            bool asValue = false;
            const auto ro = regionOf.find((quint64(pc.row) << 16) | pc.col);
            if (ro != regionOf.end()) {
                const Region &r = regions[size_t(ro->second)];
                W exp;
                exp.u8(r.table ? 0x02 : 0x01).u16(int(r.range.row1)).u16(int(r.range.col1));
                rgce = exp.bytes();
                if (r.range.row1 == pc.row && r.range.col1 == pc.col) after = r.record;
            } else {
                XlsCompileContext cx = m_cx;
                cx.sheet = m_currentSheet;
                QString why;
                const QString f = sheet.formulaText(c);
                const bool compiled = !f.isEmpty() && xlsCompileFormula(f, cx, &rgce, &extra, &why);
                if (compiled && kFormulaHead + rgce.size() + extra.size() > kMaxRecord) {
                    why = QCoreApplication::translate("Sheet", "The formula is longer than the old format allows.");
                    rgce.clear();
                    extra.clear();
                }
                if (!compiled || rgce.isEmpty()) {
                    asValue = true;
                    ++m_reasons[f.isEmpty() ? QStringLiteral("no text") : why];
                }
            }
            if (asValue) {
                ++m_formulasAsValues;
                writeValue(out, row, col, xf, c, text);
                continue;
            }
            ++m_formulas;
            writeFormula(out, row, col, xf, c, text, rgce, extra, after);
        }
        flushBlanks();
    }

    int m_currentSheet = -1;

    // The ARRAY record of an array formula, or the TABLE record of a data table ("TABLE(row input,column input)", either of which may be empty); empty if the format cannot hold it.
    QByteArray regionRecord(const CellRange &range, bool table, const QString &text)
    {
        QByteArray rec;
        if (!table) {
            XlsCompileContext cx = m_cx;
            cx.sheet = m_currentSheet;
            cx.array = true;
            QByteArray rgce, extra;
            QString why;
            if (!xlsCompileFormula(text, cx, &rgce, &extra, &why)) return QByteArray();
            if (14 + rgce.size() + extra.size() > kMaxRecord) return QByteArray();
            W a;
            a.u16(int(range.row1)).u16(int(range.row2)).u8(int(range.col1)).u8(int(range.col2)).u16(0).u32(0).u16(int(rgce.size())).raw(rgce).raw(extra);
            putRecord(&rec, 0x0221, a.bytes());
            return rec;
        }
        if (!text.startsWith(QLatin1String("TABLE(")) || !text.endsWith(QLatin1Char(')'))) return QByteArray();
        const QStringList parts = text.mid(6, text.size() - 7).split(QLatin1Char(','));
        if (parts.size() != 2) return QByteArray();
        quint32 r1 = 0, c1 = 0, r2 = 0, c2 = 0;
        const bool hasRow = !parts[0].trimmed().isEmpty(), hasCol = !parts[1].trimmed().isEmpty();
        if (hasRow && !parseCellRef(QString(parts[0]).remove(QLatin1Char('$')).trimmed(), &r1, &c1)) return QByteArray();
        if (hasCol && !parseCellRef(QString(parts[1]).remove(QLatin1Char('$')).trimmed(), &r2, &c2)) return QByteArray();
        if (!hasRow && !hasCol) return QByteArray();
        if (r1 >= quint32(kXlsMaxRows) || r2 >= quint32(kXlsMaxRows) || c1 >= quint32(kXlsMaxCols) || c2 >= quint32(kXlsMaxCols)) return QByteArray();
        W t;
        t.u16(int(range.row1)).u16(int(range.row2)).u8(int(range.col1)).u8(int(range.col2));
        if (hasRow && hasCol) t.u16(0x08).u16(int(r1)).u16(int(c1)).u16(int(r2)).u16(int(c2));
        else if (hasRow) t.u16(0x04).u16(int(r1)).u16(int(c1)).u16(0).u16(0);
        else t.u16(0).u16(int(r2)).u16(int(c2)).u16(0).u16(0);
        putRecord(&rec, 0x0236, t.bytes());
        return rec;
    }

    void writeValue(QByteArray *out, int row, int col, int xf, const Cell &c, const QString &text)
    {
        switch (c.type) {
        case CellType::Empty:
            putRecord(out, 0x0201, W().u16(row).u16(col).u16(xf).bytes());
            break;
        case CellType::Number:
        case CellType::Date: {
            const double v = c.number;
            if (std::isfinite(v)) {
                const long long n = std::llround(v * 100.0);
                if (std::fabs(v) < 5.0e6 && std::fabs(double(n)) < 536870912.0 && double(n) / 100.0 == v) {
                    const bool whole = n % 100 == 0;
                    const qint32 stored = whole ? qint32(n / 100) : qint32(n);
                    putRecord(out, 0x027E, W().u16(row).u16(col).u16(xf).u32((quint32(stored) << 2) | (whole ? 2 : 3)).bytes());
                    break;
                }
            }
            // (a number that is not one is the error the other form gives it)
            if (!std::isfinite(v)) putRecord(out, 0x0205, W().u16(row).u16(col).u16(xf).u8(0x24).u8(1).bytes());
            else putRecord(out, 0x0203, W().u16(row).u16(col).u16(xf).f64(v).bytes());
            break;
        }
        case CellType::SharedString:
        case CellType::InlineString:
        case CellType::FormulaString:
            putRecord(out, 0x00FD, W().u16(row).u16(col).u16(xf).u32(quint32(sstFor(text))).bytes());
            break;
        case CellType::Bool:
            putRecord(out, 0x0205, W().u16(row).u16(col).u16(xf).u8(c.number != 0 ? 1 : 0).u8(0).bytes());
            break;
        case CellType::Error:
            putRecord(out, 0x0205, W().u16(row).u16(col).u16(xf).u8(errorCode(int(c.number))).u8(1).bytes());
            break;
        }
    }

    static int errorCode(int error)
    {
        switch (CellError(error)) {
        case CellError::Null: return 0x00;
        case CellError::Div0: return 0x07;
        case CellError::Value: return 0x0F;
        case CellError::Ref: return 0x17;
        case CellError::Name: return 0x1D;
        case CellError::Num: return 0x24;
        case CellError::NA: return 0x2A;
        default: return 0x2A;   // (the errors the old format has no number for show as #N/A)
        }
    }

    void writeFormula(QByteArray *out, int row, int col, int xf, const Cell &c, const QString &text, const QByteArray &rgce, const QByteArray &extra, const QByteArray &after)
    {
        W f;
        f.u16(row).u16(col).u16(xf);
        bool stringResult = false;
        switch (c.type) {
        case CellType::Number:
        case CellType::Date: f.f64(std::isfinite(c.number) ? c.number : 0); break;
        case CellType::Bool: f.u8(1).u8(0).u8(c.number != 0 ? 1 : 0).u8(0).u8(0).u8(0).u16(0xFFFF); break;
        case CellType::Error: f.u8(2).u8(0).u8(errorCode(int(c.number))).u8(0).u8(0).u8(0).u16(0xFFFF); break;
        case CellType::SharedString:
        case CellType::InlineString:
        case CellType::FormulaString:
            if (text.isEmpty()) {
                f.u8(3).u8(0).u8(0).u8(0).u8(0).u8(0).u16(0xFFFF);
            } else {
                f.u8(0).u8(0).u8(0).u8(0).u8(0).u8(0).u16(0xFFFF);
                stringResult = true;
            }
            break;
        case CellType::Empty: f.f64(0); break;
        }
        f.u16(c.type == CellType::Empty ? 1 : 0).u32(0).u16(int(rgce.size())).raw(rgce).raw(extra);
        putRecord(out, 0x0006, f.bytes());
        if (stringResult) {
            Continued s(out, 0x0207);
            s.string(text);
            s.finish();
        }
        out->append(after);
    }

    // ---- the records every sheet needs

    // The number, among the sheets written, of the sheet that has this number among all the book's (a sheet left out gives the next one written, or the last).
    int writtenIndex(int place) const
    {
        for (size_t i = 0; i < m_written.size(); ++i)
            if (m_written[i].place >= place) return int(i);
        return std::max(0, int(m_written.size()) - 1);
    }

    QByteArray buildGlobals(const QByteArray &names, const QVector<qsizetype> &sheetSizes)
    {
        QByteArray g;
        g += bof(0x0005);
        put(&g, 0x00E1, W().u16(0x04B0));
        put(&g, 0x00C1, W().u16(0));
        put(&g, 0x00E2, W());
        {
            // (the writer's name, as a string padded to 112 bytes)
            QByteArray data;
            data.append(char(10));
            data.append(char(0));
            data.append(char(0));
            data.append(QByteArray("JeffCrunch"));
            data.append(QByteArray(112 - int(data.size()), ' '));
            putRecord(&g, 0x005C, data);
        }
        put(&g, 0x0042, W().u16(0x04B0));
        put(&g, 0x0161, W().u16(0));
        {
            // (TABID holds the numbers of the sheets, one each)
            QByteArray data;
            for (qsizetype i = 0; i < sheetSizes.size(); ++i) {
                data.append(char((i + 1) & 0xFF));
                data.append(char(((i + 1) >> 8) & 0xFF));
            }
            putRecord(&g, 0x013D, data);
        }
        put(&g, 0x009C, W().u16(0x0E));
        put(&g, 0x0019, W().u16(m_book.info().protection.lockWindows ? 1 : 0));
        put(&g, 0x0012, W().u16(m_book.info().protection.lockStructure ? 1 : 0));
        put(&g, 0x0063, W().u16(0));
        put(&g, 0x0013, W().u16(0));
        put(&g, 0x01AF, W().u16(0));
        put(&g, 0x01BC, W().u16(0));
        put(&g, 0x0040, W().u16(0));
        put(&g, 0x008D, W().u16(0));
        const int active = writtenIndex(m_book.info().activeTab), first = writtenIndex(m_book.info().firstSheet);
        put(&g, 0x003D, W().u16(0x01E0).u16(0x010E).u16(0x2B2F).u16(0x1799).u16(0x0038).u16(active).u16(first).u16(1).u16(0x0258));
        put(&g, 0x0022, W().u16(m_book.info().dateSystem == jp::calc::DateSystem::Date1904 ? 1 : 0));
        put(&g, 0x000E, W().u16(1));
        put(&g, 0x01B7, W().u16(0));
        put(&g, 0x00DA, W().u16(0));
        for (const QByteArray &f : std::as_const(m_fontRecords)) putRecord(&g, 0x0031, f);
        for (const auto &f : std::as_const(m_formatRecords)) {
            W w;
            w.u16(f.first).str8(f.second, 2);
            putRecord(&g, 0x041E, w.bytes());
        }
        for (const QByteArray &x : std::as_const(m_xfRecords)) putRecord(&g, 0x00E0, x);
        put(&g, 0x0293, W().u16(0x8000).u8(0).u8(0xFF));
        if (!m_palette.isEmpty()) {
            W p;
            p.u16(56);
            for (int i = 0; i < 56; ++i) {
                const quint32 c = i < m_palette.size() ? m_palette[i] : Styles::defaultIndexed(8 + i);
                p.u8(int((c >> 16) & 0xFF)).u8(int((c >> 8) & 0xFF)).u8(int(c & 0xFF)).u8(0);
            }
            put(&g, 0x0092, p);
        }
        put(&g, 0x0160, W().u16(1));
        // the list of sheets: each record gives where its sheet begins, which is known once the records of the globals are all made
        const QByteArray country = W().u16(1).u16(1).bytes();
        QByteArray extern_;
        if (!m_xtis.isEmpty()) {
            putRecord(&extern_, 0x01AE, W().u16(int(m_written.size())).u16(0x0401).bytes());
            if (!m_addIns.isEmpty()) {
                putRecord(&extern_, 0x01AE, W().u16(1).u16(0x3A01).bytes());
                for (const QString &name : std::as_const(m_addIns)) putRecord(&extern_, 0x0023, W().u16(0).u32(0).str8(name, 1).u16(2).u8(0x1C).u8(0x17).bytes());
            }
            W x;
            x.u16(int(m_xtis.size()));
            for (const auto &p : std::as_const(m_xtis)) x.u16(p.supbook).u16(p.first).u16(p.last);
            putRecord(&extern_, 0x0017, x.bytes());
        }
        // the strings
        QByteArray sst;
        {
            Continued c(&sst, 0x00FC);
            W head;
            head.u32(quint32(m_sstRefs)).u32(quint32(m_sst.size()));
            c.bytes(head.bytes());
            for (const QString &s : std::as_const(m_sst)) c.string(s);
            c.finish();
        }
        QByteArray boundSheets;
        qsizetype boundSize = 0;
        for (size_t i = 0; i < m_written.size(); ++i) {
            W b;
            b.u32(0).u8(m_book.info().sheets.value(m_written[i].place).state == SheetState::Hidden ? 1 : m_book.info().sheets.value(m_written[i].place).state == SheetState::VeryHidden ? 2 : 0).u8(0);
            b.u8(int(m_written[i].name.size())).u8(isWide(m_written[i].name) ? 1 : 0);
            const bool wide = isWide(m_written[i].name);
            for (const QChar ch : m_written[i].name) {
                if (wide) b.u16(ch.unicode());
                else b.u8(ch.unicode());
            }
            boundSize += 4 + b.size();
        }
        const qsizetype globalsSize = g.size() + boundSize + 4 + country.size() + extern_.size() + names.size() + sst.size() + 4;
        qsizetype offset = globalsSize;
        for (size_t i = 0; i < m_written.size(); ++i) {
            const SheetEntry &e = m_book.info().sheets.value(m_written[i].place);
            W b;
            b.u32(quint32(offset)).u8(e.state == SheetState::Hidden ? 1 : e.state == SheetState::VeryHidden ? 2 : 0).u8(0);
            const bool wide = isWide(m_written[i].name);
            b.u8(int(m_written[i].name.size())).u8(wide ? 1 : 0);
            for (const QChar ch : m_written[i].name) {
                if (wide) b.u16(ch.unicode());
                else b.u8(ch.unicode());
            }
            putRecord(&g, 0x0085, b.bytes());
            offset += sheetSizes[qsizetype(i)];
        }
        putRecord(&g, 0x008C, country);
        g += extern_;
        g += names;
        g += sst;
        put(&g, 0x000A, W());
        return g;
    }
};

} // namespace

bool saveXls(const Workbook &book, QByteArray *out, CopyReport *report, QString *error, const std::function<bool()> &cancelled)
{
    QString message;
    XlsWriter w(book, report, cancelled);
    const bool ok = w.run(out, &message);
    if (error) *error = message;
    return ok && message.isEmpty();
}

} // namespace jp::office::sheet
