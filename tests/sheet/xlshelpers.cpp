#include "xlshelpers.h"

#include "io/cfb.h"

#include <cstring>

namespace xlstest {

W &W::f64(double v)
{
    char bytes[8];
    std::memcpy(bytes, &v, 8);
    m_b.append(bytes, 8);
    return *this;
}

W &W::str8(const QString &s, int countBytes, bool compressed)
{
    if (countBytes == 1) u8(int(s.size())); else u16(int(s.size()));
    u8(compressed ? 0 : 1);
    if (compressed) {
        m_b.append(s.toLatin1());
    } else {
        for (QChar c : s) u16(c.unicode());
    }
    return *this;
}

W &W::str5(const QString &s, int countBytes)
{
    if (countBytes == 1) u8(int(s.size())); else u16(int(s.size()));
    m_b.append(s.toLatin1());
    return *this;
}

QByteArray rec(quint16 id, const QByteArray &payload) { return W().u16(id).u16(int(payload.size())).raw(payload); }

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// tokens

QByteArray tInt(int v) { return W().u8(0x1E).u16(v); }
QByteArray tNum(double v) { return W().u8(0x1F).f64(v); }
QByteArray tStr(const QString &s, int biff)
{
    W w;
    w.u8(0x17);
    return biff >= 8 ? w.str8(s) : w.str5(s);
}
QByteArray tBool(bool v) { return W().u8(0x1D).u8(v ? 1 : 0); }
QByteArray tErr(int code) { return W().u8(0x1C).u8(code); }
QByteArray tMissing() { return W().u8(0x16); }
QByteArray tOp(int ptg) { return W().u8(ptg); }

QByteArray tRef(int row, int col, bool rowRel, bool colRel, int biff, int cls)
{
    W w;
    w.u8(cls + 0x04);
    if (biff >= 8) return w.u16(row).u16(col | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0));
    return w.u16(row | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0)).u8(col);
}

QByteArray tArea(int row1, int col1, int row2, int col2, bool rowRel, bool colRel, int biff, int cls)
{
    W w;
    w.u8(cls + 0x05);
    if (biff >= 8) return w.u16(row1).u16(row2).u16(col1 | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0)).u16(col2 | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0));
    const int flags = (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0);
    return w.u16(row1 | flags).u16(row2 | flags).u8(col1).u8(col2);
}

QByteArray tRef3d(int ixti, int row, int col, bool rowRel, bool colRel)
{
    return W().u8(0x3A).u16(ixti).u16(row).u16(col | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0));
}

QByteArray tArea3d(int ixti, int row1, int col1, int row2, int col2, bool rowRel, bool colRel)
{
    const int flags = (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0);
    return W().u8(0x3B).u16(ixti).u16(row1).u16(row2).u16(col1 | flags).u16(col2 | flags);
}

QByteArray tRefN(int dRow, int dCol, bool rowRel, bool colRel)
{
    // (a relative part is a distance, in 16 bits for a row and 8 for a column; the flags take the high bits of the column)
    return W().u8(0x2C).u16(dRow & 0xFFFF).u16((dCol & 0xFF) | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0));
}

QByteArray tAreaN(int dRow1, int dCol1, int dRow2, int dCol2, bool rowRel, bool colRel)
{
    const int flags = (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0);
    return W().u8(0x2D).u16(dRow1 & 0xFFFF).u16(dRow2 & 0xFFFF).u16((dCol1 & 0xFF) | flags).u16((dCol2 & 0xFF) | flags);
}

QByteArray tFunc(int index, int cls) { return W().u8(cls + 0x01).u16(index); }
QByteArray tFuncVar(int index, int argc, int cls) { return W().u8(cls + 0x02).u8(argc).u16(index); }
QByteArray tName(int index, int biff, int cls)
{
    W w;
    w.u8(cls + 0x03).u16(index);
    return biff >= 8 ? w.u16(0) : w.raw(QByteArray(12, '\0'));
}
QByteArray tNameX(int ixti, int index) { return W().u8(0x39).u16(ixti).u16(index).u16(0); }
QByteArray tRef3d5(int iw, int firstSheet, int lastSheet, int row, int col, bool rowRel, bool colRel)
{
    return W().u8(0x3A).u16(iw).raw(QByteArray(8, '\0')).u16(firstSheet).u16(lastSheet).u16(row | (colRel ? 0x4000 : 0) | (rowRel ? 0x8000 : 0)).u8(col);
}
QByteArray tNameX5(int iw, int index) { return W().u8(0x39).u16(iw).raw(QByteArray(8, '\0')).u16(index).raw(QByteArray(12, '\0')); }
QByteArray tAttrSum() { return W().u8(0x19).u8(0x10).u16(0); }
QByteArray tAttrIf(int jump) { return W().u8(0x19).u8(0x02).u16(jump); }
QByteArray tAttrGoto(int jump) { return W().u8(0x19).u8(0x08).u16(jump); }
QByteArray tAttrSpace(int kind, int count) { return W().u8(0x19).u8(0x40).u8(kind).u8(count); }
QByteArray tAttrChoose(const QList<int> &offsets)
{
    W w;
    w.u8(0x19).u8(0x04).u16(int(offsets.size()) - 1);
    for (int o : offsets) w.u16(o);
    return w;
}
QByteArray tMemFunc(int cce) { return W().u8(0x29).u16(cce); }
QByteArray tArray() { return W().u8(0x60).raw(QByteArray(7, '\0')); }

QByteArray arrayValues(const QList<QVariantList> &rows, int biff)
{
    W w;
    const int cols = rows.isEmpty() ? 1 : int(rows[0].size());
    if (biff >= 8) w.u8(cols - 1).u16(int(rows.size()) - 1); else w.u8(cols).u16(int(rows.size()));
    for (const QVariantList &row : rows) {
        for (const QVariant &v : row) {
            if (v.typeId() == QMetaType::Double || v.typeId() == QMetaType::Int) {
                w.u8(0x01).f64(v.toDouble());
            } else if (v.typeId() == QMetaType::Bool) {
                w.u8(0x04).u8(v.toBool() ? 1 : 0).raw(QByteArray(7, '\0'));
            } else if (v.typeId() == QMetaType::QString) {
                w.u8(0x02);
                if (biff >= 8) w.str8(v.toString(), 2); else w.str5(v.toString(), 1);
            } else if (v.typeId() == QMetaType::UInt) {
                w.u8(0x10).u8(int(v.toUInt())).raw(QByteArray(7, '\0'));
            } else {
                w.u8(0x00).raw(QByteArray(8, '\0'));
            }
        }
    }
    return w;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// cells

QByteArray number(int row, int col, double v, int xf) { return rec(0x0203, W().u16(row).u16(col).u16(xf).f64(v)); }
QByteArray rk(int row, int col, quint32 value, int xf) { return rec(0x027E, W().u16(row).u16(col).u16(xf).u32(value)); }
QByteArray mulrk(int row, int colFirst, const QList<quint32> &values, int xf)
{
    W w;
    w.u16(row).u16(colFirst);
    for (quint32 v : values) w.u16(xf).u32(v);
    w.u16(colFirst + int(values.size()) - 1);
    return rec(0x00BD, w);
}
QByteArray labelSst(int row, int col, int index, int xf) { return rec(0x00FD, W().u16(row).u16(col).u16(xf).u32(quint32(index))); }
QByteArray label(int row, int col, const QString &s, int xf, int biff)
{
    W w;
    w.u16(row).u16(col).u16(xf);
    return rec(0x0204, biff >= 8 ? w.str8(s, 2) : w.str5(s, 2));
}
QByteArray blank(int row, int col, int xf) { return rec(0x0201, W().u16(row).u16(col).u16(xf)); }
QByteArray mulblank(int row, int colFirst, int count, int xf)
{
    W w;
    w.u16(row).u16(colFirst);
    for (int i = 0; i < count; ++i) w.u16(xf);
    w.u16(colFirst + count - 1);
    return rec(0x00BE, w);
}
QByteArray boolErr(int row, int col, int value, bool isError, int xf) { return rec(0x0205, W().u16(row).u16(col).u16(xf).u8(value).u8(isError ? 1 : 0)); }

QByteArray formulaNumber(int row, int col, const QByteArray &rgce, double result, int xf, const QByteArray &extra, int flags)
{
    return rec(0x0006, W().u16(row).u16(col).u16(xf).f64(result).u16(flags).u32(0).u16(int(rgce.size())).raw(rgce).raw(extra));
}

QByteArray formulaSpecial(int row, int col, const QByteArray &rgce, int kind, int value, int xf, const QByteArray &extra)
{
    W w;
    w.u16(row).u16(col).u16(xf).u8(kind).u8(0).u8(value).u8(0).u8(0).u8(0).u16(0xFFFF).u16(0).u32(0).u16(int(rgce.size())).raw(rgce).raw(extra);
    return rec(0x0006, w);
}

QByteArray stringRec(const QString &s, int biff)
{
    W w;
    return rec(0x0207, biff >= 8 ? w.str8(s, 2) : w.str5(s, 2));
}

QByteArray shrfmla(int row1, int row2, int col1, int col2, const QByteArray &rgce, const QByteArray &extra)
{
    return rec(0x04BC, W().u16(row1).u16(row2).u8(col1).u8(col2).u8(0).u8(row2 - row1 + 1).u16(int(rgce.size())).raw(rgce).raw(extra));
}

QByteArray arrayRec(int row1, int row2, int col1, int col2, const QByteArray &rgce, const QByteArray &extra)
{
    return rec(0x0221, W().u16(row1).u16(row2).u8(col1).u8(col2).u16(0).u32(0).u16(int(rgce.size())).raw(rgce).raw(extra));
}

QByteArray tableRec(int row1, int row2, int col1, int col2, int flags, int rowIn, int colIn, int rowIn2, int colIn2)
{
    return rec(0x0236, W().u16(row1).u16(row2).u8(col1).u8(col2).u16(flags).u16(rowIn).u16(colIn).u16(rowIn2).u16(colIn2));
}

QByteArray formulaExp(int row, int col, int expRow, int expCol, double result, int xf, bool table)
{
    return formulaNumber(row, col, W().u8(table ? 0x02 : 0x01).u16(expRow).u16(expCol), result, xf);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// sheet records

QByteArray dimensions(int rows, int cols, int biff)
{
    if (biff >= 8) return rec(0x0200, W().u32(0).u32(quint32(rows)).u16(0).u16(cols).u16(0));
    return rec(0x0200, W().u16(0).u16(rows).u16(0).u16(cols).u16(0));
}
QByteArray colinfo(int first, int last, int width256, int xf, int flags) { return rec(0x007D, W().u16(first).u16(last).u16(width256).u16(xf).u16(flags).u16(0)); }
QByteArray rowRec(int row, int heightTwips, int flags, int xf)
{
    // row, first and last column, height, two unused words, the flags (bit 8 must be set), and the format
    return rec(0x0208, W().u16(row).u16(0).u16(0).u16(heightTwips).u16(0).u16(0).u16(flags | 0x100).u16(xf));
}
QByteArray mergedCells(const QList<QList<int>> &ranges)
{
    W w;
    w.u16(int(ranges.size()));
    for (const QList<int> &r : ranges) w.u16(r[0]).u16(r[1]).u16(r[2]).u16(r[3]);
    return rec(0x00E5, w);
}
QByteArray window2(int flags, int top, int left, int scale)
{
    return rec(0x023E, W().u16(flags).u16(top).u16(left).u32(0x40).u16(0).u16(scale).u32(0));
}
QByteArray pane(int x, int y, int top, int left, int active) { return rec(0x0041, W().u16(x).u16(y).u16(top).u16(left).u8(active)); }
QByteArray scl(int num, int denom) { return rec(0x00A0, W().u16(num).u16(denom)); }
QByteArray selection(int pane, int row, int col) { return rec(0x001D, W().u8(pane).u16(row).u16(col).u16(0).u16(1).u16(row).u16(row).u8(col).u8(col)); }
QByteArray defColWidth(int w) { return rec(0x0055, W().u16(w)); }
QByteArray defaultRowHeight(int flags, int twips) { return rec(0x0225, W().u16(flags).u16(twips)); }

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// globals

QByteArray font(const QString &name, double points, int weight, bool italic, bool strike, int underline, int color, int escapement, int biff)
{
    W w;
    w.u16(int(points * 20)).u16((italic ? 2 : 0) | (strike ? 8 : 0)).u16(color).u16(weight).u16(escapement).u8(underline).u8(0).u8(0).u8(0);
    return rec(0x0031, biff >= 8 ? w.str8(name) : w.str5(name));
}

QByteArray xf(const XfSpec &s)
{
    W w;
    w.u16(s.font).u16(s.format).u16(s.style ? 4 : 0);
    w.u8((s.halign & 7) | (s.wrap ? 8 : 0) | (s.valign << 4));
    w.u8(s.rotation);
    w.u8((s.indent & 15) | (s.shrink ? 16 : 0));
    w.u8(0);
    w.u16(s.borders[0] | (s.borders[1] << 4) | (s.borders[2] << 8) | (s.borders[3] << 12));
    w.u16(s.borderColors[0] | (s.borderColors[1] << 7));
    w.u32(quint32(s.borderColors[2]) | (quint32(s.borderColors[3]) << 7) | (quint32(s.pattern) << 26));
    w.u16(s.fore | (s.back << 7));
    return rec(0x00E0, w);
}

QByteArray xf5(const XfSpec &s)
{
    W w;
    w.u16(s.font).u16(s.format).u16(s.style ? 4 : 0);
    w.u8((s.halign & 7) | (s.wrap ? 8 : 0) | (s.valign << 4));
    w.u8(0);
    // the fill and the bottom border: the colors of the fill, its pattern, then the bottom border's style and color
    w.u32(quint32(s.fore & 0x7F) | (quint32(s.back & 0x7F) << 7) | (quint32(s.pattern & 0x3F) << 16) | (quint32(s.borders[3] & 7) << 22) | (quint32(s.borderColors[3] & 0x7F) << 25));
    // the other borders: the styles of top, left, and right, then their colors
    w.u32(quint32(s.borders[2] & 7) | (quint32(s.borders[0] & 7) << 3) | (quint32(s.borders[1] & 7) << 6) | (quint32(s.borderColors[2] & 0x7F) << 9)
          | (quint32(s.borderColors[0] & 0x7F) << 16) | (quint32(s.borderColors[1] & 0x7F) << 23));
    return rec(0x00E0, w);
}

QByteArray format(int id, const QString &code, int biff)
{
    W w;
    w.u16(id);
    return rec(0x041E, biff >= 8 ? w.str8(code, 2) : w.str5(code, 1));
}

QByteArray palette(const QList<quint32> &colors)
{
    W w;
    w.u16(int(colors.size()));
    for (quint32 c : colors) w.u8(int((c >> 16) & 0xFF)).u8(int((c >> 8) & 0xFF)).u8(int(c & 0xFF)).u8(0);
    return rec(0x0092, w);
}

QByteArray boundSheet(quint32 offset, const QString &name, int state, int type, int biff)
{
    W w;
    w.u32(offset);
    if (biff >= 8) return rec(0x0085, w.u8(state).u8(type).str8(name));
    return rec(0x0085, w.u16(state | (type << 8)).str5(name));
}

QByteArray sst(const QStringList &strings, int splitAt, const QList<int> &richRuns)
{
    // the table, whole, with the places where each string's characters begin and end (for cutting it)
    QByteArray body;
    for (int i = 0; i < strings.size(); ++i) {
        const QString &s = strings[i];
        bool wide = false;
        for (QChar c : s)
            if (c.unicode() > 0xFF) wide = true;
        const int runs = i < richRuns.size() ? richRuns[i] : 0;
        W w;
        w.u16(int(s.size())).u8((wide ? 1 : 0) | (runs ? 8 : 0));
        if (runs) w.u16(runs);
        body += w;
        if (wide) {
            for (QChar c : s) body += W().u16(c.unicode()).bytes();
        } else {
            body += s.toLatin1();
        }
        for (int r = 0; r < runs; ++r) body += W().u16(r).u16(0).bytes();
    }
    const QByteArray head = W().u32(quint32(strings.size())).u32(quint32(strings.size()));
    if (splitAt <= 0) return rec(0x00FC, head + body);
    // Cut: the first piece is `splitAt` bytes of the table; each of the pieces after it is a CONTINUE record. A cut inside the characters of a string puts the byte
    // that says how the rest is written first in the next piece.
    QByteArray all = head + body;
    QByteArray out;
    qsizetype pos = 0;
    bool first = true;
    // where the character data of each string lies, to know whether a cut is inside one
    struct Span {
        qsizetype begin, end;
        bool wide;
    };
    QList<Span> spans;
    qsizetype at = head.size();
    for (int i = 0; i < strings.size(); ++i) {
        const QString &s = strings[i];
        bool wide = false;
        for (QChar c : s)
            if (c.unicode() > 0xFF) wide = true;
        const int runs = i < richRuns.size() ? richRuns[i] : 0;
        at += 3 + (runs ? 2 : 0);
        const qsizetype chars = wide ? s.size() * 2 : s.size();
        spans.append({at, at + chars, wide});
        at += chars + runs * 4;
    }
    while (pos < all.size()) {
        qsizetype end = std::min<qsizetype>(all.size(), pos + splitAt);
        // (a character of two bytes is not cut in the middle)
        for (const Span &sp : spans)
            if (sp.wide && end > sp.begin && end < sp.end && (end - sp.begin) % 2 == 1 && end - 1 > pos) --end;
        QByteArray piece = all.mid(pos, end - pos);
        QByteArray prefix;
        if (!first) {
            // a piece that begins inside a string's characters begins with the flag byte
            for (const Span &sp : spans)
                if (pos >= sp.begin && pos < sp.end) prefix = QByteArray(1, char(sp.wide ? 1 : 0));
        }
        out += rec(first ? 0x00FC : 0x003C, prefix + piece);
        first = false;
        pos = end;
    }
    return out;
}

QByteArray supbookInternal(int sheets) { return rec(0x01AE, W().u16(sheets).u16(0x0401)); }
QByteArray supbookExternal(const QString &url, const QStringList &sheets)
{
    W w;
    w.u16(int(sheets.size())).u16(int(url.size())).u8(0).raw(url.toLatin1());
    for (const QString &s : sheets) w.str8(s, 2);
    return rec(0x01AE, w);
}
QByteArray supbookAddIn() { return rec(0x01AE, W().u16(1).u16(0x3A01)); }
QByteArray externName(const QString &name) { return rec(0x0023, W().u16(0).u32(0).str8(name).u16(2).u8(0x1C).u8(0x17)); }
QByteArray externSheet(const QList<QList<int>> &xti)
{
    W w;
    w.u16(int(xti.size()));
    for (const QList<int> &x : xti) w.u16(x[0]).u16(x[1]).u16(x[2]);
    return rec(0x0017, w);
}
QByteArray externSheet5(const QByteArray &encodedName) { return rec(0x0017, W().u8(int(encodedName.size())).raw(encodedName)); }

QByteArray externCount5(int n) { return rec(0x0016, W().u16(n)); }
QByteArray externName5(const QString &name) { return rec(0x0023, W().u16(0).u32(0).str5(name)); }

QByteArray name(const QString &text, const QByteArray &rgce, int sheet, int builtin, bool hidden, const QByteArray &extra, int biff)
{
    W w;
    w.u16((hidden ? 1 : 0) | (builtin >= 0 ? 0x20 : 0)).u8(0);
    const QString chars = builtin >= 0 ? QString(QChar(builtin)) : text;
    w.u8(int(chars.size())).u16(int(rgce.size())).u16(0).u16(sheet).u8(0).u8(0).u8(0).u8(0);
    if (biff >= 8) w.u8(0).raw(chars.toLatin1()); else w.raw(chars.toLatin1());
    return rec(0x0018, w.raw(rgce).raw(extra));
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the workbook

Book &Book::sheet(const QString &name, const QByteArray &records, int state, int type)
{
    Sheet s;
    s.name = name;
    s.records = records;
    s.state = state;
    s.type = type;
    m_sheets.append(s);
    return *this;
}

QByteArray Book::stream() const
{
    auto bof = [this](int type) {
        W w;
        if (m_biff >= 8) w.u16(0x0600).u16(type).u16(0x0DBB).u16(0x07CC).u32(0x41).u32(6);
        else w.u16(0x0500).u16(type).u16(0x1234).u16(0x07CD);
        return rec(0x0809, w);
    };
    QByteArray g = bof(0x0005);
    g += rec(0x0042, W().u16(m_codepage > 0 ? m_codepage : m_biff >= 8 ? 1200 : 1252));
    if (m_defaults) {
        for (int i = 0; i < 5; ++i) g += font(QStringLiteral("Arial"), 10, 400, false, false, 0, 0x7FFF, 0, m_biff);
        XfSpec style;
        style.style = true;
        for (int i = 0; i < 16; ++i) g += m_biff >= 8 ? xf(i < 15 ? style : XfSpec()) : xf5(i < 15 ? style : XfSpec());
    }
    g += m_globals;
    // the records of the sheets in the globals have a length that does not depend on where the sheets are, so the places can be worked out first
    qsizetype boundLength = 0;
    for (const Sheet &s : m_sheets) boundLength += boundSheet(0, s.name, s.state, s.type, m_biff).size();
    const qsizetype endLength = rec(0x000A).size();
    qsizetype place = g.size() + boundLength + m_after.size() + endLength;
    QByteArray bounds, bodies;
    for (const Sheet &s : m_sheets) {
        bounds += boundSheet(quint32(place + bodies.size()), s.name, s.state, s.type, m_biff);
        bodies += bof(s.type == 2 ? 0x20 : s.type == 1 ? 0x40 : 0x10) + s.records + rec(0x000A);
    }
    return g + bounds + m_after + rec(0x000A) + bodies;
}

QByteArray Book::file(const QStringList &extraStreams) const
{
    jp::cfb::File f;
    jp::cfb::Entry root;
    root.type = jp::cfb::Entry::Root;
    root.name = QStringLiteral("Root Entry");
    f.entries << root;
    jp::cfb::Entry wb;
    wb.type = jp::cfb::Entry::Stream;
    wb.name = m_biff >= 8 ? QStringLiteral("Workbook") : QStringLiteral("Book");
    wb.data = stream();
    f.entries << wb;
    f.entries[0].children << 1;
    for (const QString &path : extraStreams) {
        int parent = 0;
        const QStringList parts = path.split(QLatin1Char('/'));
        for (int i = 0; i < parts.size(); ++i) {
            jp::cfb::Entry e;
            e.name = parts[i];
            e.type = i + 1 < parts.size() ? jp::cfb::Entry::Storage : jp::cfb::Entry::Stream;
            if (e.type == jp::cfb::Entry::Stream) e.data = QByteArray(16, 'x');
            f.entries << e;
            const int index = int(f.entries.size()) - 1;
            f.entries[parent].children << index;
            parent = index;
        }
    }
    return jp::cfb::write(f);
}

bool openBook(const Book &book, Workbook *out, QString *why, const Limits &limits)
{
    QString message;
    const bool ok = out->openBinary(book.file(), &message, limits);
    if (why) *why = message;
    return ok;
}

QString shown(const Workbook &book, const Sheet &sheet, const QString &ref) { return show(sheet, book.sharedStrings(), ref); }

} // namespace xlstest
