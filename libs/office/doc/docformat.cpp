// The file header, the table of pieces, and the bins of properties of an old binary document.

#include "doc/docformat.h"

#include <QCoreApplication>
#include <algorithm>

namespace jp::office::doc {

bool readFib(const Span &wd, Fib *fib, QString *why)
{
    *fib = Fib();
    auto fail = [&](const QString &m) {
        if (why) *why = m;
        return false;
    };
    const QString older = QCoreApplication::translate("jp::office::doc", "This document was made by a program from before 1997, and its format can't be opened.");
    // (The 1993 and 1995 programs marked their header 0xA5DC, the 1991 one 0xA5DB: their files are older, not damaged.)
    if (wd.size() >= 2 && (wd.u16(0) == 0xA5DC || wd.u16(0) == 0xA5DB)) return fail(older);
    if (wd.size() < 68 || wd.u16(0) != 0xA5EC) return fail(QCoreApplication::translate("jp::office::doc", "The document's header is damaged."));
    fib->nFib = wd.u16(2);
    if (fib->nFib < 0x00C1) return fail(older);
    const quint16 flags = wd.u16(10);
    fib->template_ = flags & 0x0001;
    fib->complex = flags & 0x0004;
    fib->encrypted = flags & 0x0100;
    fib->whichTable1 = flags & 0x0200;
    // (Two short arrays of numbers, then the table of places.)
    qint64 o = 32;
    const int csw = wd.u16(o);
    o += 2 + qint64(csw) * 2;
    const int cslw = wd.u16(o);
    o += 2;
    if (cslw < 11 || !wd.fits(o, qint64(cslw) * 4)) return fail(QCoreApplication::translate("jp::office::doc", "The document's header is damaged."));
    fib->ccpText = wd.u32(o + 12);
    fib->ccpFtn = wd.u32(o + 16);
    fib->ccpHdd = wd.u32(o + 20);
    fib->ccpAtn = wd.u32(o + 28);
    fib->ccpEdn = wd.u32(o + 32);
    fib->ccpTxbx = wd.u32(o + 36);
    fib->ccpHdrTxbx = wd.u32(o + 40);
    o += qint64(cslw) * 4;
    const int pairs = std::min<int>(wd.u16(o), 256);
    o += 2;
    for (int i = 0; i < pairs && wd.fits(o, 8); ++i, o += 8) fib->fcs.push_back({wd.u32(o), wd.u32(o + 4)});
    // (A count that can't be right is cut to the most the reader takes; open() cuts the stories to the text the file has.)
    for (qint64 *cc : {&fib->ccpText, &fib->ccpFtn, &fib->ccpHdd, &fib->ccpAtn, &fib->ccpEdn, &fib->ccpTxbx, &fib->ccpHdrTxbx})
        *cc = std::clamp<qint64>(*cc, 0, kMaxChars);
    fib->valid = true;
    return true;
}

Plc::Plc(Span d, int recordSize) : data(d), size(recordSize)
{
    if (d.size() >= 4 && recordSize >= 0) n = int(std::min<qint64>((d.size() - 4) / (4 + recordSize), 10000000));
}

// ---- the text

namespace {

// The characters 0x80 to 0x9F of the Windows Western coding.
const ushort kWest[32] = {0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
                          0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};

} // namespace

bool PieceTable::read(const Span &clx, const Span &wd, qint64 chars, bool *truncated)
{
    m_pieces.clear();
    m_prcs.clear();
    m_text.clear();
    Span plc;
    qint64 o = 0;
    while (o < clx.size()) {
        const quint8 kind = clx.u8(o);
        if (kind == 1) {
            const qint64 cb = clx.u16(o + 1);
            if (!clx.fits(o + 3, cb)) break;
            if (m_prcs.size() < 65536) m_prcs.push_back(clx.bytes(o + 3, cb));
            o += 3 + cb;
        } else if (kind == 2) {
            const qint64 lcb = clx.u32(o + 1);
            plc = clx.sub(o + 5, lcb);
            break;
        } else {
            break;
        }
    }
    if (plc.size() < 4) return false;
    Plc pcd(plc, 8);
    const qint64 want = std::min(chars, kMaxChars);
    m_text.reserve(qsizetype(std::min<qint64>(want, 1 << 24)));
    bool cut = false;
    for (int i = 0; i < pcd.n; ++i) {
        if (i >= kMaxPieces) { cut = true; break; }
        Piece p;
        p.cp0 = pcd.cp(i);
        p.cp1 = pcd.cp(i + 1);
        if (p.cp0 != m_text.size() || p.cp1 <= p.cp0) { cut = true; break; }   // (the pieces follow one another, from the first character)
        const Span r = pcd.record(i);
        const quint32 raw = r.u32(2);
        p.unicode = !(raw & 0x40000000);
        p.fc = p.unicode ? qint64(raw & 0x3FFFFFFF) : qint64(raw & 0x3FFFFFFF) / 2;
        p.prm = r.u16(6);
        const qint64 whole = p.cp1 - p.cp0;
        qint64 len = whole;
        if (m_text.size() + len > want) { len = want - m_text.size(); cut = true; }
        const qint64 per = p.unicode ? 2 : 1;
        if (len > 0 && !wd.fits(p.fc, len * per)) {
            len = p.fc >= wd.size() ? 0 : (wd.size() - p.fc) / per;
            cut = true;
        }
        if (len <= 0) break;
        p.cp1 = p.cp0 + len;
        const uchar *d = wd.data() + p.fc;
        if (p.unicode) {
            m_text.resize(qsizetype(p.cp1));
            QChar *dst = m_text.data() + p.cp0;
            for (qint64 k = 0; k < len; ++k) dst[k] = QChar(ushort(d[2 * k] | (d[2 * k + 1] << 8)));
        } else {
            m_text.resize(qsizetype(p.cp1));
            QChar *dst = m_text.data() + p.cp0;
            for (qint64 k = 0; k < len; ++k) dst[k] = QChar(d[k] >= 0x80 && d[k] < 0xA0 ? kWest[d[k] - 0x80] : ushort(d[k]));
        }
        m_pieces.push_back(p);
        if (len < whole) break;
    }
    if (truncated) *truncated = cut;
    return !m_pieces.isEmpty();
}

const Piece *PieceTable::at(qint64 cp) const
{
    auto it = std::upper_bound(m_pieces.begin(), m_pieces.end(), cp, [](qint64 v, const Piece &p) { return v < p.cp0; });
    if (it == m_pieces.begin()) return nullptr;
    --it;
    return cp < it->cp1 ? &*it : nullptr;
}

QByteArray PieceTable::modifier(const Piece &piece) const
{
    if (!piece.prm) return QByteArray();
    if (piece.prm & 1) {
        const int i = piece.prm >> 1;
        return i < m_prcs.size() ? m_prcs[i] : QByteArray();
    }
    // One sprm with a one-byte operand: the number of it in the old numbering, and the operand.
    const int isprm = (piece.prm >> 1) & 0x7F;
    const quint8 val = quint8(piece.prm >> 8);
    quint16 op = 0;
    switch (isprm) {
    case 5: op = 0x2403; break;     // alignment
    case 7: op = 0x2405; break;     // keep lines together
    case 8: op = 0x2406; break;     // keep with next
    case 9: op = 0x2407; break;     // page break before
    case 13: op = 0x240C; break;    // no line numbers
    case 23: op = 0x2416; break;    // in a table
    case 24: op = 0x2417; break;    // ends a table row
    case 85: op = 0x0835; break;    // bold
    case 86: op = 0x0836; break;    // italic
    case 87: op = 0x0837; break;    // strike
    case 88: op = 0x0838; break;    // outline
    case 89: op = 0x0839; break;    // shadow
    case 90: op = 0x083A; break;    // small capitals
    case 91: op = 0x083B; break;    // capitals
    case 92: op = 0x083C; break;    // hidden
    default: return QByteArray();
    }
    QByteArray g(3, 0);
    g[0] = char(op & 0xFF);
    g[1] = char(op >> 8);
    g[2] = char(val);
    return g;
}

// ---- the bins

bool BinTable::read(const Span &plc, const Span &wd, bool paragraphs, qint64 *budget)
{
    m_runs.clear();
    Plc bte(plc, 4);
    bool sorted = true;
    for (int i = 0; i < bte.n; ++i) {
        const qint64 page = qint64(bte.record(i).u32(0) & 0x3FFFFF) * 512;
        if (!wd.fits(page, 512)) continue;
        const int crun = wd.u8(page + 511);
        const int per = paragraphs ? 13 : 1;
        if (crun == 0 || 4 * (crun + 1) + per * crun > 511) continue;
        const qint64 tableAt = page + 4 * qint64(crun + 1);
        for (int r = 0; r < crun; ++r) {
            if (budget && (*budget)-- <= 0) return !m_runs.isEmpty();
            BinRun run;
            run.fc0 = wd.u32(page + 4 * qint64(r));
            run.fc1 = wd.u32(page + 4 * qint64(r) + 4);
            if (run.fc1 <= run.fc0) continue;
            const int b = wd.u8(tableAt + qint64(per) * r);
            if (b != 0) {
                const qint64 at = page + qint64(b) * 2;
                qint64 off = 0, len = 0;
                if (paragraphs) {
                    int cb = wd.u8(at);
                    if (cb == 0) {
                        cb = wd.u8(at + 1);
                        off = at + 2;
                        len = qint64(cb) * 2;
                    } else {
                        off = at + 1;
                        len = qint64(cb) * 2 - 1;
                    }
                    if (len >= 2 && off + len <= page + 512) {
                        run.istd = wd.u16(off);
                        run.off = qint32(off + 2);
                        run.len = qint32(len - 2);
                    }
                } else {
                    off = at + 1;
                    len = wd.u8(at);
                    if (len > 0 && off + len <= page + 512) {
                        run.off = qint32(off);
                        run.len = qint32(len);
                    }
                }
            }
            if (!m_runs.isEmpty() && run.fc0 < m_runs.last().fc0) sorted = false;
            m_runs.push_back(run);
        }
    }
    if (!sorted) std::stable_sort(m_runs.begin(), m_runs.end(), [](const BinRun &a, const BinRun &b) { return a.fc0 < b.fc0; });
    return !m_runs.isEmpty();
}

const BinRun *BinTable::find(quint32 fc) const
{
    auto it = std::upper_bound(m_runs.begin(), m_runs.end(), fc, [](quint32 v, const BinRun &r) { return v < r.fc0; });
    if (it == m_runs.begin()) return nullptr;
    --it;
    return fc < it->fc1 ? &*it : nullptr;
}

quint32 BinTable::nextStart(quint32 fc) const
{
    auto it = std::upper_bound(m_runs.begin(), m_runs.end(), fc, [](quint32 v, const BinRun &r) { return v < r.fc0; });
    return it == m_runs.end() ? 0xFFFFFFFFu : it->fc0;
}

} // namespace jp::office::doc
