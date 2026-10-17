#pragma once
// The structures of the old binary format that the reader shares: the file header, the table of pieces, the bins of properties, the
// property sets (character, paragraph, table row, section) and the sprms that change them. Internal to libs/office/doc.

#include "doc/doc.h"
#include "doc/docbytes.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>

namespace jp::office::doc {

// ---- the file header (FIB)
struct Fib {
    struct Fc { quint32 fc = 0, lcb = 0; };
    enum Slot {
        Stshf = 1, PlcffndRef = 2, PlcffndTxt = 3, PlcfandRef = 4, PlcfandTxt = 5, PlcfSed = 6, PlcfHdd = 11, PlcfBteChpx = 12, PlcfBtePapx = 13,
        SttbfFfn = 15, SttbfBkmk = 21, PlcfBkf = 22, PlcfBkl = 23, Dop = 31, Clx = 33, PlcSpaMom = 40, PlcSpaHdr = 41, PlcfendRef = 46, PlcfendTxt = 47,
        DggInfo = 50, SttbfRMark = 51, PlcftxbxTxt = 56, PlcfHdrtxbxTxt = 58, PlfLst = 73, PlfLfo = 74
    };
    bool valid = false;
    int nFib = 0;
    bool encrypted = false, whichTable1 = false, complex = false, template_ = false;
    // The characters of each story, in the order they follow one another in the text: the main text, the footnotes, the headers and
    // footers, the comments, the endnotes, the text boxes of the main text, and those of the headers.
    qint64 ccpText = 0, ccpFtn = 0, ccpHdd = 0, ccpAtn = 0, ccpEdn = 0, ccpTxbx = 0, ccpHdrTxbx = 0;
    QVector<Fc> fcs;
    Fc at(int slot) const { return slot >= 0 && slot < fcs.size() ? fcs[slot] : Fc(); }
    qint64 total() const { return ccpText + ccpFtn + ccpHdd + ccpAtn + ccpEdn + ccpTxbx + ccpHdrTxbx; }
    // Where each story starts, as a character position.
    qint64 footnoteStart() const { return ccpText; }
    qint64 headerStart() const { return ccpText + ccpFtn; }
    qint64 commentStart() const { return headerStart() + ccpHdd; }
    qint64 endnoteStart() const { return commentStart() + ccpAtn; }
    qint64 textBoxStart() const { return endnoteStart() + ccpEdn; }
    qint64 headerTextBoxStart() const { return textBoxStart() + ccpTxbx; }
};
// Reads the header of the WordDocument stream. False, with the reason, if it isn't a document this reads.
bool readFib(const Span &wd, Fib *fib, QString *why);

// A table of positions with a record for each gap (a plc): n + 1 character positions, then n records of `size` bytes.
struct Plc {
    Span data;
    int n = 0, size = 0;
    Plc() = default;
    Plc(Span d, int recordSize);
    qint64 cp(int i) const { return data.u32(qint64(i) * 4); }
    Span record(int i) const { return data.sub(qint64(n + 1) * 4 + qint64(i) * size, size); }
};

// ---- the text
struct Piece {
    qint64 cp0 = 0, cp1 = 0;   // characters [cp0, cp1)
    qint64 fc = 0;             // where its first character is in the WordDocument stream
    bool unicode = false;      // two bytes to the character (else one: the Windows Western coding)
    quint16 prm = 0;           // property changes the piece carries
};
class PieceTable {
public:
    // Reads the CLX from the table stream. The text is read from `wd`, `chars` characters of it at the most.
    bool read(const Span &clx, const Span &wd, qint64 chars, bool *truncated);
    const QVector<Piece> &pieces() const { return m_pieces; }
    // The piece holding character `cp` (null outside the text).
    const Piece *at(qint64 cp) const;
    // Where character `cp` of `piece` is in the stream.
    static qint64 fcOf(const Piece &piece, qint64 cp) { return piece.fc + (cp - piece.cp0) * (piece.unicode ? 2 : 1); }
    // The sprms a piece's property modifier carries (empty if none).
    QByteArray modifier(const Piece &piece) const;
    // All the text, indexed by character position.
    const QString &text() const { return m_text; }
    qint64 length() const { return m_text.size(); }

private:
    QVector<Piece> m_pieces;
    QVector<QByteArray> m_prcs;
    QString m_text;
};

// ---- the bins of properties: for each stretch of the stream, the sprms that apply
struct BinRun {
    quint32 fc0 = 0, fc1 = 0;
    qint32 off = -1;           // of the sprms in the stream; -1 for none
    qint32 len = 0;
    quint16 istd = 0;          // paragraphs: the style
};
class BinTable {
public:
    // Reads the bin table `plc` (from the table stream) and the pages it points to (in `wd`). `paragraphs` says which kind.
    bool read(const Span &plc, const Span &wd, bool paragraphs, qint64 *budget);
    // The run holding the stream position (null if none).
    const BinRun *find(quint32 fc) const;
    // Where the next run begins after `fc` (0xFFFFFFFF if none).
    quint32 nextStart(quint32 fc) const;
    int count() const { return int(m_runs.size()); }

private:
    QVector<BinRun> m_runs;
};

// ---- sprms
struct Sprm {
    quint16 op = 0;
    Span arg;                  // the operand, without the length bytes of the ones that have a length
    int sgc() const { return (op >> 10) & 7; }     // 1 paragraph, 2 character, 3 picture, 4 section, 5 table
};
// Calls `f(const Sprm &)` for each sprm of the group, at most kMaxSprms.
template <class F> void forEachSprm(const Span &g, F &&f)
{
    qint64 o = 0;
    int count = 0;
    while (o + 2 <= g.size() && count++ < kMaxSprms) {
        Sprm s;
        s.op = g.u16(o);
        qint64 len = 0, skip = 2;
        switch (s.op >> 13) {
        case 0: case 1: len = 1; break;
        case 2: case 4: case 5: len = 2; break;
        case 3: len = 4; break;
        case 7: len = 3; break;
        default:   // 6: a length first
            if (s.op == 0xD608) {
                len = qint64(g.u16(o + 2)) - 1;     // (the length counts itself and is two bytes)
                skip = 4;
                if (len < 0) len = 0;
            } else if (s.op == 0xC615 && g.u8(o + 2) == 255) {
                const int del = g.u8(o + 3);
                const int add = g.u8(o + 4 + 4 * qint64(del));
                len = 1 + 4 * qint64(del) + 1 + 3 * qint64(add);
                skip = 3;
            } else {
                len = g.u8(o + 2);
                skip = 3;
            }
            break;
        }
        if (o + skip + len > g.size()) break;
        s.arg = g.sub(o + skip, len);
        f(s);
        o += skip + len;
    }
}

} // namespace jp::office::doc
