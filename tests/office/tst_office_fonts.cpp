// Tests of libs/office: reading, inspecting, and embedding fonts.
#include "officehelpers.h"

namespace {

// A bundled font's file, read whole.
QByteArray bundledFontBytes(const QString &family, bool bold = false, bool italic = false)
{
    QFile f(jp::bundledFontFile(family, bold, italic).path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// Where a table's record is in the font's directory (the 16 bytes of tag, checksum, offset, and length), -1 if it has none.
int tableRecord(const QByteArray &font, const char *tag)
{
    for (int i = 0, n = qFromBigEndian<quint16>(font.constData() + 4); i < n; ++i)
        if (font.mid(12 + 16 * i, 4) == tag) return 12 + 16 * i;
    return -1;
}

quint32 beAt(const QByteArray &b, int at) { return qFromBigEndian<quint32>(b.constData() + at); }
void setBe32(QByteArray &b, int at, quint32 v) { qToBigEndian<quint32>(v, b.data() + at); }
void setBe16(QByteArray &b, int at, quint16 v) { qToBigEndian<quint16>(v, b.data() + at); }
quint32 leAt(const QByteArray &b, int at) { return qFromLittleEndian<quint32>(b.constData() + at); }

void appendU16Be(QByteArray *out, quint16 v)
{
    out->append(char(v >> 8));
    out->append(char(v & 0xff));
}

// ----- A writer of MicroType Express, for the tests: the program only reads it, so this makes the data the reader is shown.

struct MtxBits {
    QByteArray bytes;
    int used = 0;   // bits of the last byte
    void put(int bit)
    {
        if (used == 0) bytes.append('\0');
        if (bit) bytes[bytes.size() - 1] = char(quint8(bytes[bytes.size() - 1]) | (0x80 >> used));
        used = (used + 1) % 8;
    }
    void put(quint32 v, int n)
    {
        for (int i = n - 1; i >= 0; --i) put(int((v >> i) & 1));
    }
};

// The adaptive Huffman tree as the reader keeps it, writing symbols.
class MtxTree {
public:
    explicit MtxTree(int range) : m_range(range)
    {
        const int nodes = 2 * range;
        up.assign(nodes, 0), left.assign(nodes, 0), right.assign(nodes, 0), code.assign(nodes, 0), weight.assign(nodes, 0), where.assign(range, 0);
        for (int i = 2; i < nodes; ++i) up[i] = i / 2, weight[i] = 1;
        for (int i = 1; i < range; ++i) left[i] = 2 * i, right[i] = 2 * i + 1;
        for (int i = 0; i < range; ++i) code[i] = -1, code[range + i] = i, left[range + i] = right[range + i] = -1, where[i] = range + i;
        for (int i = range - 1; i > 0; --i) weight[i] = weight[2 * i] + weight[2 * i + 1];
        if (range >= 0x101 && range <= 0x1ff) {
            const int seen[4][2] = {{256, 1}, {257, 1}, {range - 3, 12}, {range - 2, 6}};
            for (const auto &each : seen)
                for (int i = 0; i < each[1]; ++i) update(where[each[0]]);
        } else {
            for (int pass = 0; pass < 2; ++pass)
                for (int s = 0; s < range; ++s) update(where[s]);
        }
    }
    void write(int symbol, MtxBits *bits)
    {
        std::vector<int> path;
        for (int n = where[symbol]; n != 1; n = up[n]) path.push_back(right[up[n]] == n ? 1 : 0);
        for (auto it = path.rbegin(); it != path.rend(); ++it) bits->put(*it);
        update(where[symbol]);
    }

private:
    void update(int node)
    {
        for (; node != 1; node = up[node]) {
            const long long w = weight[node];
            int first = node - 1;
            if (weight[first] == w) {
                do {
                    --first;
                } while (weight[first] == w);
                ++first;
                if (first > 1) {
                    std::swap(left[node], left[first]), std::swap(right[node], right[first]), std::swap(code[node], code[first]), std::swap(weight[node], weight[first]);
                    for (const int n : {node, first}) {
                        if (code[n] < 0) up[left[n]] = n, up[right[n]] = n;
                        else where[code[n]] = n;
                    }
                    node = first;
                }
            }
            weight[node] = w + 1;
        }
        ++weight[1];
    }
    int m_range;
    std::vector<int> up, left, right, code, where;
    std::vector<long long> weight;
};

// One LZCOMP block of `data`, with copies (of at least 3 bytes) and the one-byte copies from 2, 4, and 6 back. With `runLength` the data is
// first run-length coded (the least used byte value is the marker). With `quick` the search for copies is cut short, for blocks of millions of bytes.
QByteArray mtxCompress(QByteArray data, bool runLength = false, bool quick = false)
{
    if (runLength) {
        int counts[256] = {};
        for (char c : data) ++counts[quint8(c)];
        const int marker = int(std::min_element(counts, counts + 256) - counts);
        QByteArray coded(1, char(marker));
        for (qsizetype i = 0; i < data.size();) {
            qsizetype run = 1;
            while (i + run < data.size() && data[i + run] == data[i] && run < 255) ++run;
            if (run >= 4 || quint8(data[i]) == marker) {
                if (quint8(data[i]) == marker && run == 1) coded.append(char(marker)).append('\0');
                else coded.append(char(marker)).append(char(run)).append(data[i]);
                i += run;
            } else {
                coded.append(data[i++]);
            }
        }
        data = coded;
    }
    MtxBits out;
    out.put(runLength ? 1 : 0);
    out.put(quint32(data.size()), 24);
    if (data.isEmpty()) return out.bytes;
    int ranges = 1;
    if (data.size() > 8)
        for (ranges = 2; data.size() > (qint64(1) << (3 * ranges)); ++ranges) {}
    const int dup2 = 256 + 8 * ranges;
    MtxTree symbols(dup2 + 3), lengths(8), distances(8);
    QByteArray window(0x1c00, '\0');
    {
        int k = 0;
        for (int row = 0; row < 32; ++row)
            for (int column = 0; column < 96; ++column) window[k++] = char(row), window[k++] = char(column);
        for (int b = 0; b < 256; ++b)
            for (int j = 0; j < 4; ++j) window[k++] = char(b);
    }
    window.append(data);
    const int prefix = 0x1c00;
    for (qsizetype at = 0; at < data.size();) {
        const qsizetype remaining = data.size() - at;
        qsizetype bestLength = 0, bestBack = 0;
        // `quick`: only the copy from a multiple of 270 back, a few thousand bytes into the data, is looked for (enough for data that repeats
        // with a period that divides 270 after a short head, and a copy may be as long as what is made so far), so millions of bytes go at once.
        const qsizetype quickBack = at >= 4000 ? std::min<qsizetype>((at - 3000) / 270 * 270, qsizetype(1) << 22) : 0;
        for (qsizetype back = quick ? quickBack : 1; back <= (quick ? quickBack : std::min<qsizetype>(900, prefix + at)) && back > 0; ++back) {
            if (window[prefix + at - back] != window[prefix + at]) continue;
            qsizetype n = 1;
            while (n < std::min<qsizetype>({back, remaining, quick ? (qsizetype(1) << 23) : 270}) && window[prefix + at - back + n] == window[prefix + at + n]) ++n;
            const qsizetype distance = back - n + 1;
            if (n >= 3 && n > bestLength && distance - 1 < (qint64(1) << (3 * ranges)) && n - 2 - (distance > 0x1ff ? 1 : 0) >= 0) bestLength = n, bestBack = back;
        }
        if (bestLength >= 3) {
            const qint64 distance = bestBack - bestLength + 1;
            qint64 coded = bestLength - 2 - (distance > 0x1ff ? 1 : 0);
            std::vector<int> chunks;
            for (qint64 v = coded; ; v >>= 2) {
                chunks.insert(chunks.begin(), int(v & 3));
                if (v < 4) break;
            }
            std::vector<int> digits;
            for (qint64 v = distance - 1; ; v >>= 3) {
                digits.insert(digits.begin(), int(v & 7));
                if (v < 8) break;
            }
            const int first = chunks[0] | (chunks.size() > 1 ? 4 : 0);
            symbols.write(256 + 8 * int(digits.size() - 1) + first, &out);
            for (size_t i = 1; i < chunks.size(); ++i) lengths.write(chunks[i] | (i + 1 < chunks.size() ? 4 : 0), &out);
            for (int d : digits) distances.write(d, &out);
            at += bestLength;
            continue;
        }
        int symbol = quint8(window[prefix + at]);
        for (int back : {2, 4, 6})
            if (at + prefix >= back && window[prefix + at - back] == window[prefix + at]) {
                symbol = dup2 + back / 2 - 1;
                break;
            }
        symbols.write(symbol, &out);
        ++at;
    }
    return out.bytes;
}

// A glyph's program split into the values of its first pushes and what comes after them.
void mtxSplitProgram(const QByteArray &program, std::vector<int> *values, QByteArray *rest)
{
    qsizetype i = 0;
    while (i < program.size()) {
        const quint8 op = quint8(program[i]);
        int n = 0;
        bool words = false;
        qsizetype at = i + 1;
        if (op == 0x40 || op == 0x41) {
            if (at >= program.size()) break;
            n = quint8(program[at++]);
            words = op == 0x41;
        } else if (op >= 0xB0 && op <= 0xB7) {
            n = op - 0xB0 + 1;
        } else if (op >= 0xB8 && op <= 0xBF) {
            n = op - 0xB8 + 1;
            words = true;
        } else {
            break;
        }
        if (at + (words ? 2 : 1) * n > program.size()) break;
        for (int k = 0; k < n; ++k) {
            if (words) values->push_back(qint16(quint8(program[at]) << 8 | quint8(program[at + 1]))), at += 2;
            else values->push_back(quint8(program[at++]));
        }
        i = at;
    }
    *rest = program.mid(i);
}

void mtxPut255(QByteArray *out, int v)
{
    if (v < 253) out->append(char(v));
    else if (v < 506) out->append(char(255)).append(char(v - 253));
    else if (v < 759) out->append(char(254)).append(char(v - 506));
    else out->append(char(253)).append(char(v >> 8)).append(char(v & 255));
}

void mtxPutValue(QByteArray *out, int v)
{
    auto magnitude = [&](int m) {
        if (m < 250) out->append(char(m));
        else if (m < 500) out->append(char(255)).append(char(m - 250));
        else out->append(char(254)).append(char(m - 500));
    };
    if (v >= -754 && v <= 754) {
        if (v < 0) out->append(char(250));
        magnitude(std::abs(v));
    } else {
        out->append(char(253)).append(char(v >> 8)).append(char(v & 255));
    }
}

// The values written with the hop codes where the pattern comes (A, x, A, y, A and A, x, A, y, A, z, A).
void mtxPutValues(QByteArray *out, const std::vector<int> &v)
{
    for (size_t i = 0; i < v.size();) {
        if (i + 6 < v.size() && v[i + 2] == v[i] && v[i + 4] == v[i] && v[i + 6] == v[i]) {
            mtxPutValue(out, v[i]), mtxPutValue(out, v[i + 1]);
            out->append(char(252));
            mtxPutValue(out, v[i + 3]), mtxPutValue(out, v[i + 5]);
            i += 7;
        } else if (i + 4 < v.size() && v[i + 2] == v[i] && v[i + 4] == v[i]) {
            mtxPutValue(out, v[i]), mtxPutValue(out, v[i + 1]);
            out->append(char(251));
            mtxPutValue(out, v[i + 3]);
            i += 5;
        } else {
            mtxPutValue(out, v[i++]);
        }
    }
}

// One change of a point as a flag byte and the bytes after it (the inverse of the reader's table).
void mtxPutTriplet(int dx, int dy, bool onCurve, QByteArray *flags, QByteArray *data)
{
    const int ax = std::abs(dx), ay = std::abs(dy), xs = dx > 0 ? 1 : 0, ys = dy > 0 ? 1 : 0;
    int index;
    QByteArray bytes;
    if (dx == 0 && ay < 1280) {
        index = ((ay >> 8) << 1) + ys;
        bytes.append(char(ay & 255));
    } else if (dy == 0 && ax < 1280) {
        index = 10 + ((ax >> 8) << 1) + xs;
        bytes.append(char(ax & 255));
    } else if (ax >= 1 && ax <= 64 && ay >= 1 && ay <= 64) {
        index = 20 + (((ax - 1) & 0x30) | (((ay - 1) & 0x30) >> 2)) + xs + 2 * ys;
        bytes.append(char((((ax - 1) & 15) << 4) | ((ay - 1) & 15)));
    } else if (ax >= 1 && ax <= 768 && ay >= 1 && ay <= 768) {
        index = 84 + 12 * ((ax - 1) >> 8) + 4 * ((ay - 1) >> 8) + xs + 2 * ys;
        bytes.append(char((ax - 1) & 255)).append(char((ay - 1) & 255));
    } else if (ax < 4096 && ay < 4096) {
        index = 120 + xs + 2 * ys;
        bytes.append(char(ax >> 4)).append(char(((ax & 15) << 4) | (ay >> 8))).append(char(ay & 255));
    } else {
        index = 124 + xs + 2 * ys;
        bytes.append(char(ax >> 8)).append(char(ax & 255)).append(char(ay >> 8)).append(char(ay & 255));
    }
    flags->append(char(index | (onCurve ? 0 : 0x80)));
    data->append(bytes);
}

// A TrueType font's tables, as MicroType Express holds them: the glyphs translated, the control values packed, the glyph index left out.
QByteArray mtxCompressFont(const QByteArray &font, bool runLengthOnTables = false, int extraGlyphsClaimed = 0)
{
    auto u16 = [&](qsizetype at) { return quint16(quint8(font[at]) << 8 | quint8(font[at + 1])); };
    auto u32 = [&](qsizetype at) { return quint32(u16(at)) << 16 | u16(at + 2); };
    std::map<QByteArray, QByteArray> tables;
    for (int i = 0, n = u16(4); i < n; ++i) tables[font.mid(12 + 16 * i, 4)] = font.mid(int(u32(12 + 16 * i + 8)), int(u32(12 + 16 * i + 12)));
    const QByteArray head = tables["head"], loca = tables["loca"], glyf = tables["glyf"];
    const int glyphs = qFromBigEndian<quint16>(tables["maxp"].constData() + 4);
    const bool shortLoca = qFromBigEndian<qint16>(head.constData() + 50) == 0;
    auto offsetOf = [&](int g) { return shortLoca ? 2 * qFromBigEndian<quint16>(loca.constData() + 2 * g) : int(qFromBigEndian<quint32>(loca.constData() + 4 * g)); };
    QByteArray outlines, pushes, code;
    for (int g = 0; g < glyphs; ++g) {
        const QByteArray glyph = glyf.mid(offsetOf(g), offsetOf(g + 1) - offsetOf(g));
        if (glyph.isEmpty()) {
            outlines.append(char(0)).append(char(0));
            continue;
        }
        const int contours = qFromBigEndian<qint16>(glyph.constData());
        if (contours < 0) {
            outlines.append(char(0xFF)).append(char(0xFF)).append(glyph.mid(2, 8));
            qsizetype at = 10;
            int flags;
            do {
                flags = qFromBigEndian<quint16>(glyph.constData() + at);
                const qsizetype size = 4 + ((flags & 1) ? 4 : 2) + ((flags & 8) ? 2 : (flags & 0x40) ? 4 : (flags & 0x80) ? 8 : 0);
                outlines.append(glyph.mid(at, size));
                at += size;
            } while (flags & 0x20);
            if (flags & 0x100) {
                const int length = qFromBigEndian<quint16>(glyph.constData() + at);
                std::vector<int> values;
                QByteArray rest;
                mtxSplitProgram(glyph.mid(at + 2, length), &values, &rest);
                mtxPut255(&outlines, int(values.size()));
                mtxPut255(&outlines, int(rest.size()));
                mtxPutValues(&pushes, values);
                code.append(rest);
            }
            continue;
        }
        const char *p = glyph.constData();
        std::vector<int> ends;
        for (int c = 0; c < contours; ++c) ends.push_back(qFromBigEndian<quint16>(p + 10 + 2 * c));
        qsizetype at = 10 + 2 * contours;
        const int instructionLength = qFromBigEndian<quint16>(p + at);
        const QByteArray program = glyph.mid(at + 2, instructionLength);
        at += 2 + instructionLength;
        const int points = ends.back() + 1;
        QByteArray flags;
        for (int i = 0; i < points; ++i) {
            const quint8 f = quint8(glyph[at++]);
            flags.append(char(f));
            if (f & 8) {
                const int repeat = quint8(glyph[at++]);
                for (int k = 0; k < repeat; ++k) flags.append(char(f));
                i += repeat;
            }
        }
        std::vector<int> xs(points), ys(points);
        int v = 0;
        for (int i = 0; i < points; ++i) {
            const quint8 f = quint8(flags[i]);
            if (f & 2) v += (f & 16) ? quint8(glyph[at]) : -int(quint8(glyph[at])), ++at;
            else if (!(f & 16)) v += qFromBigEndian<qint16>(p + at), at += 2;
            xs[i] = v;
        }
        v = 0;
        for (int i = 0; i < points; ++i) {
            const quint8 f = quint8(flags[i]);
            if (f & 4) v += (f & 32) ? quint8(glyph[at]) : -int(quint8(glyph[at])), ++at;
            else if (!(f & 32)) v += qFromBigEndian<qint16>(p + at), at += 2;
            ys[i] = v;
        }
        const int xMin = *std::min_element(xs.begin(), xs.end()), yMin = *std::min_element(ys.begin(), ys.end());
        const int xMax = *std::max_element(xs.begin(), xs.end()), yMax = *std::max_element(ys.begin(), ys.end());
        const bool boxIsExtremes = qFromBigEndian<qint16>(p + 2) == xMin && qFromBigEndian<qint16>(p + 4) == yMin && qFromBigEndian<qint16>(p + 6) == xMax && qFromBigEndian<qint16>(p + 8) == yMax;
        if (boxIsExtremes) {
            appendU16Be(&outlines, quint16(contours));
        } else {
            appendU16Be(&outlines, 0x7FFF);
            appendU16Be(&outlines, quint16(contours));
            outlines.append(glyph.mid(2, 8));
        }
        mtxPut255(&outlines, ends[0]);
        for (int c = 1; c < contours; ++c) mtxPut255(&outlines, ends[c] - ends[c - 1]);
        QByteArray flagBytes, coordinates;
        int px = 0, py = 0;
        for (int i = 0; i < points; ++i) {
            mtxPutTriplet(xs[i] - px, ys[i] - py, quint8(flags[i]) & 1, &flagBytes, &coordinates);
            px = xs[i];
            py = ys[i];
        }
        outlines.append(flagBytes).append(coordinates);
        std::vector<int> values;
        QByteArray rest;
        mtxSplitProgram(program, &values, &rest);
        mtxPut255(&outlines, int(values.size()));
        mtxPut255(&outlines, int(rest.size()));
        mtxPutValues(&pushes, values);
        code.append(rest);
    }
    QByteArray cvt = tables["cvt "];
    if (!cvt.isEmpty()) {
        QByteArray packed;
        appendU16Be(&packed, quint16(cvt.size() / 2));
        int previous = 0;
        for (int i = 0; i + 1 < cvt.size(); i += 2) {
            const int value = qFromBigEndian<qint16>(cvt.constData() + i);
            const int d = value - previous;
            previous = value;
            if (d >= 0 && d < 238) packed.append(char(d));
            else if (d < 0 && -d <= 238 * 9 - 1) packed.append(char(239 + (-d) / 238)).append(char((-d) % 238));
            else if (d >= 238 && d <= 238 * 9 - 1) packed.append(char(247 + d / 238)).append(char(d % 238));
            else packed.append(char(238)).append(char(qint16(d) >> 8)).append(char(qint16(d) & 255));
        }
        tables["cvt "] = packed;
    }
    tables["glyf"] = outlines;
    tables["loca"] = QByteArray();
    if (extraGlyphsClaimed) qToBigEndian<quint16>(quint16(glyphs + extraGlyphsClaimed), tables["maxp"].data() + 4);
    tables.erase("hdmx");
    tables.erase("VDMX");
    QByteArray first;
    appendU16Be(&first, 1), appendU16Be(&first, 0);
    appendU16Be(&first, quint16(tables.size()));
    appendU16Be(&first, 0), appendU16Be(&first, 0), appendU16Be(&first, 0);
    qsizetype offset = 12 + 16 * qsizetype(tables.size());
    QByteArray body;
    for (const auto &table : tables) {
        first.append(table.first).append(QByteArray(4, '\0'));
        appendU16Be(&first, quint16(table.second.isEmpty() ? 0 : offset >> 16)), appendU16Be(&first, quint16(table.second.isEmpty() ? 0 : offset & 0xFFFF));
        appendU16Be(&first, quint16(table.second.size() >> 16)), appendU16Be(&first, quint16(table.second.size() & 0xFFFF));
        body.append(table.second);
        if (!table.second.isEmpty()) offset += table.second.size();
    }
    const QByteArray b1 = mtxCompress(first + body, runLengthOnTables), b2 = mtxCompress(pushes), b3 = mtxCompress(code);
    QByteArray out;
    out.append(char(3)).append(char(0)).append(char(0xAE)).append(char(0xE0));
    const qint64 second = 10 + b1.size(), third = second + b2.size();
    for (qint64 v : {second, third}) out.append(char(v >> 16)).append(char(v >> 8)).append(char(v));
    return out + b1 + b2 + b3;
}

// An EOT around MicroType Express data, flagged as the program writes it.
QByteArray eotWithMtx(const QByteArray &font, const QByteArray &mtx, quint32 flags = 5)
{
    QByteArray eot = makeEot(font);
    eot.truncate(eot.size() - font.size());
    eot.append(mtx);
    qToLittleEndian<quint32>(quint32(eot.size()), eot.data());
    qToLittleEndian<quint32>(quint32(mtx.size()), eot.data() + 4);
    qToLittleEndian<quint32>(flags, eot.data() + 12);
    return eot;
}


// A TrueType glyph as the tests compare it: what it draws and what its program is, not how the bytes are written (flags may repeat or not, a push
// may be a byte or a word run).
struct ParsedGlyph {
    int contours = 0;
    QByteArray box, components;
    std::vector<int> ends, pushed, points;   // points: x, y, on-curve, for each
    QByteArray rest;
    bool operator==(const ParsedGlyph &o) const { return contours == o.contours && box == o.box && components == o.components && ends == o.ends && pushed == o.pushed && points == o.points && rest == o.rest; }
};

ParsedGlyph parseGlyph(const QByteArray &glyph)
{
    ParsedGlyph g;
    if (glyph.isEmpty()) return g;
    const char *p = glyph.constData();
    g.contours = qFromBigEndian<qint16>(p);
    g.box = glyph.mid(2, 8);
    if (g.contours < 0) {
        qsizetype at = 10;
        int flags;
        do {
            flags = qFromBigEndian<quint16>(p + at);
            const qsizetype size = 4 + ((flags & 1) ? 4 : 2) + ((flags & 8) ? 2 : (flags & 0x40) ? 4 : (flags & 0x80) ? 8 : 0);
            g.components += glyph.mid(at, size);
            at += size;
        } while (flags & 0x20);
        if (flags & 0x100) mtxSplitProgram(glyph.mid(at + 2, qFromBigEndian<quint16>(p + at)), &g.pushed, &g.rest);
        return g;
    }
    for (int c = 0; c < g.contours; ++c) g.ends.push_back(qFromBigEndian<quint16>(p + 10 + 2 * c));
    qsizetype at = 10 + 2 * g.contours;
    const int length = qFromBigEndian<quint16>(p + at);
    mtxSplitProgram(glyph.mid(at + 2, length), &g.pushed, &g.rest);
    at += 2 + length;
    const int points = g.ends.back() + 1;
    QByteArray flags;
    for (int i = 0; i < points; ++i) {
        const quint8 f = quint8(glyph[at++]);
        flags.append(char(f));
        if (f & 8)
            for (int k = quint8(glyph[at++]); k > 0; --k, ++i) flags.append(char(f));
    }
    std::vector<int> xs(points), ys(points);
    int v = 0;
    for (int i = 0; i < points; ++i) {
        const quint8 f = quint8(flags[i]);
        if (f & 2) v += (f & 16) ? quint8(glyph[at]) : -int(quint8(glyph[at])), ++at;
        else if (!(f & 16)) v += qFromBigEndian<qint16>(p + at), at += 2;
        xs[i] = v;
    }
    v = 0;
    for (int i = 0; i < points; ++i) {
        const quint8 f = quint8(flags[i]);
        if (f & 4) v += (f & 32) ? quint8(glyph[at]) : -int(quint8(glyph[at])), ++at;
        else if (!(f & 32)) v += qFromBigEndian<qint16>(p + at), at += 2;
        ys[i] = v;
        g.points.insert(g.points.end(), {xs[i], ys[i], quint8(flags[i]) & 1});
    }
    return g;
}

// A font's tables by name, and each glyph's bytes.
std::map<QByteArray, QByteArray> fontTables(const QByteArray &font)
{
    std::map<QByteArray, QByteArray> out;
    for (int i = 0, n = qFromBigEndian<quint16>(font.constData() + 4); i < n; ++i)
        out[font.mid(12 + 16 * i, 4)] = font.mid(int(qFromBigEndian<quint32>(font.constData() + 12 + 16 * i + 8)), int(qFromBigEndian<quint32>(font.constData() + 12 + 16 * i + 12)));
    return out;
}

QVector<QByteArray> fontGlyphs(const QByteArray &font)
{
    const auto tables = fontTables(font);
    const QByteArray &head = tables.at("head"), &loca = tables.at("loca"), &glyf = tables.at("glyf");
    const int n = qFromBigEndian<quint16>(tables.at("maxp").constData() + 4);
    const bool shortLoca = qFromBigEndian<qint16>(head.constData() + 50) == 0;
    QVector<QByteArray> out;
    for (int g = 0; g < n; ++g) {
        const int from = shortLoca ? 2 * qFromBigEndian<quint16>(loca.constData() + 2 * g) : int(qFromBigEndian<quint32>(loca.constData() + 4 * g));
        const int to = shortLoca ? 2 * qFromBigEndian<quint16>(loca.constData() + 2 * g + 2) : int(qFromBigEndian<quint32>(loca.constData() + 4 * g + 4));
        out.append(glyf.mid(from, to - from));
    }
    return out;
}

// The font with one glyph's box a little wider than its points (so the box has to be written out, and is not made from the points).
QByteArray fontWithWiderBox(QByteArray font, int glyph)
{
    const auto tables = fontTables(font);
    int glyfAt = 0;
    for (int i = 0, n = qFromBigEndian<quint16>(font.constData() + 4); i < n; ++i)
        if (font.mid(12 + 16 * i, 4) == "glyf") glyfAt = int(qFromBigEndian<quint32>(font.constData() + 12 + 16 * i + 8));
    const QByteArray &loca = tables.at("loca");
    const bool shortLoca = qFromBigEndian<qint16>(tables.at("head").constData() + 50) == 0;
    const int at = glyfAt + (shortLoca ? 2 * qFromBigEndian<quint16>(loca.constData() + 2 * glyph) : int(qFromBigEndian<quint32>(loca.constData() + 4 * glyph)));
    char *xMax = font.data() + at + 6;
    qToBigEndian<qint16>(qint16(qFromBigEndian<qint16>(xMax) + 5), xMax);
    return font;
}

// Where a glyph's data is in a font file (an absolute offset) and how long it is.
QPair<int, int> glyphPlace(const QByteArray &font, int glyph)
{
    const auto tables = fontTables(font);
    int glyfAt = 0;
    for (int i = 0, n = qFromBigEndian<quint16>(font.constData() + 4); i < n; ++i)
        if (font.mid(12 + 16 * i, 4) == "glyf") glyfAt = int(qFromBigEndian<quint32>(font.constData() + 12 + 16 * i + 8));
    const QByteArray &loca = tables.at("loca");
    const bool shortLoca = qFromBigEndian<qint16>(tables.at("head").constData() + 50) == 0;
    auto offset = [&](int g) { return shortLoca ? 2 * qFromBigEndian<quint16>(loca.constData() + 2 * g) : int(qFromBigEndian<quint32>(loca.constData() + 4 * g)); };
    return {glyfAt + offset(glyph), offset(glyph + 1) - offset(glyph)};
}

// The font with a composite glyph of these parts written over a glyph's data (which has room for it), or with the glyph's bytes made over.
QByteArray fontWithComposite(QByteArray font, int glyph, const QVector<int> &parts)
{
    QByteArray record;
    appendU16Be(&record, 0xFFFF);
    record.append(QByteArray(8, '\0'));
    for (int i = 0; i < parts.size(); ++i) {
        appendU16Be(&record, quint16(0x0002 | (i + 1 < parts.size() ? 0x0020 : 0)));
        appendU16Be(&record, quint16(parts[i]));
        record.append(QByteArray(2, '\0'));
    }
    const QPair<int, int> place = glyphPlace(font, glyph);
    if (record.size() > place.second) return QByteArray();
    font.replace(place.first, record.size(), record);
    return font;
}

// The font with maxp's maxComponentDepth set (0 says nothing).
QByteArray fontWithComponentDepth(QByteArray font, int depth)
{
    int maxpAt = 0;
    for (int i = 0, n = qFromBigEndian<quint16>(font.constData() + 4); i < n; ++i)
        if (font.mid(12 + 16 * i, 4) == "maxp") maxpAt = int(qFromBigEndian<quint32>(font.constData() + 12 + 16 * i + 8));
    qToBigEndian<quint16>(quint16(depth), font.data() + maxpAt + 30);
    return font;
}

quint32 wordSum(const QByteArray &data)
{
    quint32 sum = 0;
    for (qsizetype i = 0; i < data.size(); i += 4) {
        quint32 word = 0;
        for (qsizetype k = 0; k < 4; ++k) word = word << 8 | (i + k < data.size() ? quint8(data[i + k]) : 0);
        sum += word;
    }
    return sum;
}

// The most memory this process has had in use, in KB, and a way to start counting again (Linux only; 0 elsewhere, where a test of it is skipped).
qint64 peakMemoryKb()
{
#ifdef Q_OS_LINUX
    QFile f(QStringLiteral("/proc/self/status"));
    if (f.open(QIODevice::ReadOnly))
        for (const QByteArray &line : f.readAll().split('\n'))
            if (line.startsWith("VmHWM:")) return line.mid(6).trimmed().split(' ').value(0).toLongLong();
#endif
    return 0;
}

void resetPeakMemory()
{
#ifdef Q_OS_LINUX
    QFile f(QStringLiteral("/proc/self/clear_refs"));
    if (f.open(QIODevice::WriteOnly)) f.write("5");
#endif
}

} // namespace

class OfficeFontTests : public QObject {
    Q_OBJECT


private Q_SLOTS:

    // ----- Fonts in a file: the font file's description, and the EOT around it -----

    void fontInspectionReadsWhatAFontSays()
    {
        FontFileInfo info;
        QString why;
        QVERIFY2(jp::inspectFont(bundledFontBytes(QStringLiteral("Lato")), &info, &why), qPrintable(why));
        QCOMPARE(info.family, QStringLiteral("Lato"));
        QCOMPARE(info.subfamily, QStringLiteral("Regular"));
        QCOMPARE(int(info.weight), 400);
        QVERIFY(!info.bold && !info.italic && !info.outlinesAreCff);
        QVERIFY(jp::fontAllowsEmbedding(info.fsType));
        QCOMPARE(int(info.pitchFamily()), 0x22);   // sans serif, variable pitch: what the program writes for it
        QVERIFY(info.glyphs > 100);
        QVERIFY(info.unicodeRange[0] & 1);

        QVERIFY2(jp::inspectFont(bundledFontBytes(QStringLiteral("Lato"), true, true), &info, &why), qPrintable(why));
        QVERIFY(info.bold && info.italic);
        QCOMPARE(info.subfamily, QStringLiteral("Bold Italic"));
        QCOMPARE(int(info.weight), 700);

        // OpenType with PostScript outlines, and a license that says "editable".
        QVERIFY2(jp::inspectFont(bundledFontBytes(QStringLiteral("TeX Gyre Bonum")), &info, &why), qPrintable(why));
        QVERIFY(info.outlinesAreCff);
        QCOMPARE(int(info.fsType), 0x000c);
        QVERIFY(jp::fontAllowsEmbedding(info.fsType));

        // A family of a style of its own is the family under its full name; the typographic family only groups.
        QVERIFY2(jp::inspectFont(bundledFontBytes(QStringLiteral("Selawik Light")), &info, &why), qPrintable(why));
        QCOMPARE(info.family, QStringLiteral("Selawik Light"));
        QVERIFY(info.typographicFamily.isEmpty());
        QVERIFY2(jp::inspectFont(bundledFontBytes(QStringLiteral("TeX Gyre Schola")), &info, &why), qPrintable(why));
        QCOMPARE(info.typographicFamily, QStringLiteral("TeX Gyre Schola"));

        QVERIFY2(jp::inspectFont(bundledFontBytes(QStringLiteral("Cousine")), &info, &why), qPrintable(why));
        QVERIFY(info.fixedPitch);
        QCOMPARE(int(info.pitchFamily() & 3), 1);
    }

    void fontLicensesSayWhetherAFontMayTravel()
    {
        QVERIFY(jp::fontAllowsEmbedding(0));         // installable
        QVERIFY(jp::fontAllowsEmbedding(0x0008));    // editable
        QVERIFY(jp::fontAllowsEmbedding(0x000c));    // preview and print, and editable: the freest applies
        QVERIFY(jp::fontAllowsEmbedding(0x000a));
        QVERIFY(jp::fontAllowsEmbedding(0x0108));    // editable, and may not be subset (all of it is embedded)
        QVERIFY(!jp::fontAllowsEmbedding(0x0002));   // restricted
        QVERIFY(!jp::fontAllowsEmbedding(0x0004));   // preview and print
        QVERIFY(!jp::fontAllowsEmbedding(0x0006));
        QVERIFY(!jp::fontAllowsEmbedding(0x0200));   // bitmaps only
        QVERIFY(!jp::fontAllowsEmbedding(0x0208));
    }

    void fontInspectionRefusesDamagedFonts()
    {
        const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        QVERIFY(lato.size() > 1000);
        QString why;
        QVERIFY(jp::inspectFont(lato, nullptr, &why));
        auto refused = [&](const QByteArray &font, const char *what) {
            why.clear();
            QVERIFY2(!jp::inspectFont(font, nullptr, &why), what);
            QVERIFY2(!why.isEmpty(), what);
        };
        refused(QByteArray(), "empty");
        refused(QByteArray(100, 'x'), "not a font");
        refused(lato.left(lato.size() / 2), "cut off: the tables run past the end");
        refused(lato.left(200), "cut off in the first tables");
        QByteArray collection = lato;
        collection.replace(0, 4, "ttcf");
        refused(collection, "a collection");
        QByteArray woff = lato;
        woff.replace(0, 4, "wOFF");
        refused(woff, "a web font");

        QByteArray bad = lato;
        setBe16(bad, 4, 0xFFFF);
        refused(bad, "65,535 tables");
        bad = lato;
        setBe16(bad, 4, 0);
        refused(bad, "no tables");

        bad = lato;
        const int head = tableRecord(bad, "head");
        QVERIFY(head > 0);
        setBe32(bad, int(beAt(bad, head + 8)) + 12, 0x12345678);
        refused(bad, "head magic");
        bad = lato;
        setBe32(bad, head + 12, 8);
        refused(bad, "head table too short");

        bad = lato;
        setBe32(bad, tableRecord(bad, "glyf") + 12, 0x7FFFFFFF);
        refused(bad, "a table longer than the file");
        bad = lato;
        setBe32(bad, tableRecord(bad, "glyf") + 8, 4);
        refused(bad, "a table inside the directory");

        bad = lato;
        setBe32(bad, tableRecord(bad, "hmtx") + 12, 6);
        refused(bad, "hmtx too short for the glyphs");
        bad = lato;
        setBe32(bad, tableRecord(bad, "loca") + 12, 10);
        refused(bad, "loca too short for the glyphs");

        // The last glyph starts past the end of the glyph data.
        bad = lato;
        const int loca = tableRecord(bad, "loca"), glyphs = int(qFromBigEndian<quint16>(bad.constData() + int(beAt(bad, tableRecord(bad, "maxp") + 8)) + 4));
        const bool longLoca = qFromBigEndian<qint16>(bad.constData() + int(beAt(bad, head + 8)) + 50) == 1;
        if (longLoca) setBe32(bad, int(beAt(bad, loca + 8)) + 4 * glyphs, 0x7FFFFFF0);
        else setBe16(bad, int(beAt(bad, loca + 8)) + 2 * glyphs, 0xFFF0);
        refused(bad, "a glyph outside the glyph data");

        // Tables meant for a program to run over many values.
        bad = lato;
        bad.replace(tableRecord(bad, "gasp"), 4, "fvar");
        refused(bad, "a font with variations");

        // Two tables of one name.
        bad = lato;
        bad.replace(tableRecord(bad, "gasp"), 4, "head");
        refused(bad, "a table twice");

        // Letters that can't be in a table's name.
        bad = lato;
        bad[tableRecord(bad, "gasp")] = char(7);
        refused(bad, "a control character in a table's name");

        // More than a document may hold, refused before anything is looked at.
        QByteArray huge = lato;
        huge.append(QByteArray(jp::kMaxEmbeddedFontBytes, '\0'));
        refused(huge, "larger than 32 MB");
    }

    void bundledFontsAreFoundByFamilyAndStyle()
    {
        const jp::BundledFontFile regular = jp::bundledFontFile(QStringLiteral("lato"), false, false);
        QVERIFY(regular.path.endsWith(QLatin1String("Lato-Regular.ttf")));
        QVERIFY(jp::bundledFontFile(QStringLiteral("Lato"), true, true).path.endsWith(QLatin1String("Lato-BoldItalic.ttf")));
        QVERIFY(jp::bundledFontFile(QStringLiteral("Fraunces"), true, false).path.endsWith(QLatin1String("Fraunces-Bold.ttf")));
        // A family without the style has none; a family of one style has that one.
        QVERIFY(jp::bundledFontFile(QStringLiteral("Oswald"), false, true).path.isEmpty());
        QVERIFY(jp::bundledFontFile(QStringLiteral("Oswald"), true, false).path.endsWith(QLatin1String("Oswald-Bold.ttf")));
        QVERIFY(jp::bundledFontFile(QStringLiteral("Pacifico"), false, false).path.endsWith(QLatin1String("Pacifico-Regular.ttf")));
        // The names of the fonts people already have are not the bundled ones' (Carlito stands in for Calibri, but is not it).
        QVERIFY(jp::bundledFontFile(QStringLiteral("Calibri"), false, false).path.isEmpty());
        QVERIFY(jp::bundledFontFile(QStringLiteral("Arial"), false, false).path.isEmpty());
        QVERIFY(jp::bundledFontFile(QStringLiteral("Carlito"), false, false).path.endsWith(QLatin1String("Carlito-Regular.ttf")));
        QVERIFY(jp::isBundledFontFamily(QStringLiteral("Source Serif 4")));
        QVERIFY(!jp::isBundledFontFamily(QStringLiteral("Calibri")));
        // A font that is a style of its own is found by its full name, and a typographic family by its own.
        QVERIFY(jp::bundledFontFile(QStringLiteral("Selawik Light"), false, false).path.endsWith(QLatin1String("Selawik-Light.ttf")));
        QVERIFY(jp::bundledFontFile(QStringLiteral("TeX Gyre Schola"), true, false).path.endsWith(QLatin1String("TeXGyreSchola-Bold.otf")));
    }

    void embeddedOpenTypeWrapsAFontInTheProgramsHeader()
    {
        const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        const QByteArray eot = makeEot(lato);
        QVERIFY(!eot.isEmpty());
        QCOMPARE(int(leAt(eot, 0)), int(eot.size()));        // EOTSize
        QCOMPARE(int(leAt(eot, 4)), int(lato.size()));       // FontDataSize
        QCOMPARE(leAt(eot, 8), 0x00020002u);                 // the version the program writes
        QCOMPARE(leAt(eot, 12), 0u);                         // not compressed, not scrambled, not a subset
        QCOMPARE(eot.mid(16, 10).toHex(), QByteArray("020f0502020204030203"));   // PANOSE: Lato's, as the program has it
        QCOMPARE(int(uchar(eot[27])), 0);                    // upright
        QCOMPARE(int(leAt(eot, 28)), 400);                   // weight
        QCOMPARE(int(qFromLittleEndian<quint16>(eot.constData() + 34)), 0x504C);   // the magic number
        QCOMPARE(eot.right(lato.size()), lato);              // the font is the last thing in it
        // The family name, in UTF-16, follows the 82-byte header and its size.
        const int nameSize = qFromLittleEndian<quint16>(eot.constData() + 82);
        QCOMPARE(nameSize, 10);   // as the program has it, the name's zero character counted
        QCOMPARE(QString::fromUtf16(reinterpret_cast<const char16_t *>(eot.constData() + 84), 4), QStringLiteral("Lato"));
        // The program refuses a header whose checksum of the (empty) root string is any other than this, and then draws its text in a stand-in.
        const int start = eot.size() - lato.size();
        QCOMPARE(leAt(eot, start - 20), 0x50475342u);
        QCOMPARE(int(leAt(eot, start - 16)), 1252);
        QCOMPARE(int(leAt(eot, start - 4)), 0);   // no private-character font

        const QByteArray italic = makeEot(bundledFontBytes(QStringLiteral("Lato"), true, true));
        QCOMPARE(int(uchar(italic[27])), 1);
        QCOMPARE(int(leAt(italic, 28)), 700);
        QVERIFY(makeEot(QByteArray("not a font")).isEmpty());
    }

    void embeddedFontsAreReadFromEotAndFromPlainFontData()
    {
        const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        FontFileInfo info;
        QString why;
        QByteArray font;
        QVERIFY2(readEmbeddedFont(makeEot(lato), &font, &info, &why), qPrintable(why));
        QCOMPARE(font, lato);
        QCOMPARE(info.family, QStringLiteral("Lato"));
        // Plain font data in the part, as other programs write it.
        font.clear();
        QVERIFY2(readEmbeddedFont(lato, &font, &info, &why), qPrintable(why));
        QCOMPARE(font, lato);
        QVERIFY2(readEmbeddedFont(bundledFontBytes(QStringLiteral("TeX Gyre Bonum")), &font, nullptr, &why), qPrintable(why));

        // Scrambled: every byte of the font XOR 0x50, flag 0x10000000.
        QByteArray scrambled = makeEot(lato);
        const int start = scrambled.size() - lato.size();
        for (int i = start; i < scrambled.size(); ++i) scrambled[i] = char(scrambled[i] ^ 0x50);
        qToLittleEndian<quint32>(leAt(scrambled, 12) | 0x10000000u, scrambled.data() + 12);
        font.clear();
        QVERIFY2(readEmbeddedFont(scrambled, &font, nullptr, &why), qPrintable(why));
        QCOMPARE(font, lato);

        // The flag for a subset says only that some characters were left out; bytes after the EOT's own size are not read.
        QByteArray subset = makeEot(lato);
        qToLittleEndian<quint32>(1u, subset.data() + 12);
        subset.append("trailing bytes");
        font.clear();
        QVERIFY2(readEmbeddedFont(subset, &font, nullptr, &why), qPrintable(why));
        QCOMPARE(font, lato);

        // The older forms of the header (version 2.1) are read by where the font ends, as they all end.
        QByteArray older = makeEot(lato);
        qToLittleEndian<quint32>(0x00020001u, older.data() + 8);
        font.clear();
        QVERIFY2(readEmbeddedFont(older, &font, nullptr, &why), qPrintable(why));
        QCOMPARE(font, lato);
    }

    void embeddedFontsThatAreDamagedOrHostileAreRefused()
    {
        const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        const QByteArray eot = makeEot(lato);
        QByteArray font;
        QString why;
        auto refused = [&](const QByteArray &data, const char *what) {
            font = "left over";
            why.clear();
            QVERIFY2(!readEmbeddedFont(data, &font, nullptr, &why), what);
            QVERIFY2(!why.isEmpty(), what);
            QVERIFY2(font.isEmpty(), what);
        };
        refused(QByteArray(), "empty");
        refused(QByteArray(40, '\0'), "a header that isn't");
        refused(eot.left(eot.size() - 100), "cut off: the EOT says it is longer");
        refused(eot.left(90), "cut off in the header");

        QByteArray bad = eot;
        qToLittleEndian<quint16>(0x1234, bad.data() + 34);
        refused(bad, "wrong magic number");
        bad = eot;
        qToLittleEndian<quint32>(0x00030000u, bad.data() + 8);
        refused(bad, "a version that doesn't exist");
        bad = eot;
        qToLittleEndian<quint32>(quint32(eot.size()) + 1, bad.data() + 4);
        refused(bad, "the font larger than the EOT");
        bad = eot;
        qToLittleEndian<quint32>(60, bad.data() + 4);
        refused(bad, "the font starting inside the header");
        bad = eot;
        qToLittleEndian<quint32>(0xFFFFFFF0u, bad.data() + 4);
        qToLittleEndian<quint32>(0xFFFFFFFFu, bad.data());
        refused(bad, "sizes that overflow");

        // A font past the limit is refused whatever the file claims.
        QByteArray big = eot.left(eot.size() - lato.size());
        big.append(lato).append(QByteArray(jp::kMaxEmbeddedFontBytes, '\0'));
        qToLittleEndian<quint32>(quint32(big.size()), big.data());
        qToLittleEndian<quint32>(quint32(lato.size() + jp::kMaxEmbeddedFontBytes), big.data() + 4);
        refused(big, "a font larger than 32 MB");

        // The font in it is held to the same checks as any font.
        QByteArray damaged = lato;
        setBe32(damaged, tableRecord(damaged, "glyf") + 12, 0x7FFFFFFF);
        refused(makeEot(lato.left(100)), "a font cut off");
        QByteArray wrapped = eot.left(eot.size() - lato.size());
        wrapped.append(damaged);
        refused(wrapped, "a damaged font inside");
        refused(damaged, "a damaged font on its own");
    }

    // MicroType Express, the form the presentation program embeds fonts in. A font translated and compressed as the program does (here by a
    // writer made for these tests, from a bundled font) is read back as the font it was: the same tables, the same glyphs (what they draw and
    // their programs; the bytes are written as the reader writes them), sizes that agree, and checksums that are right.
    void microTypeExpressFontsAreReadBack()
    {
        for (const QString &family : {QStringLiteral("Lato"), QStringLiteral("Fraunces"), QStringLiteral("Open Sans")}) {
            const QByteArray font = bundledFontBytes(family);
            QVERIFY(!font.isEmpty());
            auto original = fontTables(font);
            // Hinted fonts have a push burst and control values; Fraunces has composite glyphs.
            for (bool runLength : {false, true}) {
                const QByteArray eot = eotWithMtx(font, mtxCompressFont(font, runLength));
                QByteArray read;
                FontFileInfo info;
                QString why;
                QVERIFY2(readEmbeddedFont(eot, &read, &info, &why), qPrintable(family + QLatin1String(": ") + why));
                QCOMPARE(info.family, family);
                QVERIFY2(read.size() > font.size() / 4, qPrintable(family));
                const auto tables = fontTables(read);
                for (const auto &table : original) {
                    if (table.first == "glyf" || table.first == "loca" || table.first == "head" || table.first == "hdmx" || table.first == "VDMX" || table.first == "DSIG") continue;
                    QVERIFY2(tables.count(table.first) && tables.at(table.first) == table.second, qPrintable(family + QLatin1Char(' ') + QString::fromLatin1(table.first)));
                }
                QVERIFY(!tables.count("hdmx") && !tables.count("VDMX"));
                // The glyphs.
                const QVector<QByteArray> was = fontGlyphs(font), now = fontGlyphs(read);
                QCOMPARE(now.size(), was.size());
                int composites = 0, withPrograms = 0;
                for (int g = 0; g < was.size(); ++g) {
                    QVERIFY2(parseGlyph(was[g]) == parseGlyph(now[g]), qPrintable(family + QLatin1String(" glyph ") + QString::number(g)));
                    QVERIFY(now[g].size() % 2 == 0);
                    if (!was[g].isEmpty() && qFromBigEndian<qint16>(was[g].constData()) < 0) ++composites;
                    if (!parseGlyph(was[g]).pushed.empty()) ++withPrograms;
                }
                if (family == QLatin1String("Fraunces")) QVERIFY(composites > 0);
                if (family == QLatin1String("Lato")) QVERIFY(withPrograms > 0);
                // The header: the sizes, the index format the glyph data fits, and the checksums.
                const QByteArray &head = tables.at("head");
                QCOMPARE(int(qFromBigEndian<qint16>(head.constData() + 50)), int(tables.at("glyf").size() <= 0x1FFFE ? 0 : 1));
                QCOMPARE(wordSum(read), 0xB1B0AFBAu);
                for (int i = 0, n = qFromBigEndian<quint16>(read.constData() + 4); i < n; ++i) {
                    const char *record = read.constData() + 12 + 16 * i;
                    QByteArray data = read.mid(int(qFromBigEndian<quint32>(record + 8)), int(qFromBigEndian<quint32>(record + 12)));
                    if (QByteArray(record, 4) == "head") data.replace(8, 4, QByteArray(4, '\0'));
                    QCOMPARE(qFromBigEndian<quint32>(record + 4), wordSum(data));
                    QCOMPARE(int(qFromBigEndian<quint32>(record + 8)) % 4, 0);
                }
            }
        }
        // A box that is not the extremes of the points is written out with the glyph, and kept.
        {
            const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
            const QVector<QByteArray> glyphs = fontGlyphs(lato);
            int target = -1;
            for (int g = 0; g < glyphs.size() && target < 0; ++g)
                if (!glyphs[g].isEmpty() && qFromBigEndian<qint16>(glyphs[g].constData()) > 0) target = g;
            QVERIFY(target >= 0);
            const QByteArray wider = fontWithWiderBox(lato, target);
            QVERIFY(fontGlyphs(wider)[target] != glyphs[target]);
            QByteArray read;
            QString why;
            QVERIFY2(readEmbeddedFont(eotWithMtx(wider, mtxCompressFont(wider)), &read, nullptr, &why), qPrintable(why));
            const QVector<QByteArray> now = fontGlyphs(read);
            QCOMPARE(parseGlyph(now[target]).box, parseGlyph(fontGlyphs(wider)[target]).box);
            QVERIFY(parseGlyph(now[target]).box != parseGlyph(glyphs[target]).box);
            QVERIFY(parseGlyph(now[target]) == parseGlyph(fontGlyphs(wider)[target]));
        }
        // Scrambled as well as compressed (the scrambling is undone first).
        const QByteArray font = bundledFontBytes(QStringLiteral("Lato"));
        QByteArray eot = eotWithMtx(font, mtxCompressFont(font), 0x10000005);
        const int start = eot.size() - int(leAt(eot, 4));
        for (int i = start; i < eot.size(); ++i) eot[i] = char(eot[i] ^ 0x50);
        QByteArray read;
        QString why;
        QVERIFY2(readEmbeddedFont(eot, &read, nullptr, &why), qPrintable(why));
        QCOMPARE(int(fontGlyphs(read).size()), int(fontGlyphs(font).size()));
    }

    // Damaged or hostile MicroType Express is refused, never read past its end or into more than a font may be.
    void microTypeExpressThatIsDamagedOrHostileIsRefused()
    {
        const QByteArray font = bundledFontBytes(QStringLiteral("Lato"));
        const QByteArray mtx = mtxCompressFont(font);
        QByteArray read;
        QString why;
        QVERIFY2(readEmbeddedFont(eotWithMtx(font, mtx), &read, nullptr, &why), qPrintable(why));
        auto refused = [&](const QByteArray &data, const char *what) {
            read = "left over";
            why.clear();
            QVERIFY2(!readEmbeddedFont(eotWithMtx(font, data), &read, nullptr, &why), what);
            QVERIFY2(!why.isEmpty(), what);
            QVERIFY2(read.isEmpty(), what);
        };
        refused(QByteArray(), "nothing");
        refused(mtx.left(9), "less than the header");
        refused(mtx.left(mtx.size() / 2), "cut in half");
        refused(mtx.left(mtx.size() - 40), "cut short");
        QByteArray bad = mtx;
        for (char version : {char(0), char(1), char(2), char(4), char(255)}) {
            bad[0] = version;
            refused(bad, "a version that is not read");
        }
        bad = mtx;
        bad[4] = char(0x7f);
        refused(bad, "the second block starting past the end");
        bad = mtx;
        bad[7] = char(0x00), bad[8] = char(0x00), bad[9] = char(0x20);
        refused(bad, "the third block starting inside the header");
        bad = mtx;
        bad[4] = bad[5] = bad[6] = char(0);
        refused(bad, "the second block starting at the start");

        // Every block's length is a number in the data: a block that says 16 MB but holds a few bytes runs out of them.
        bad = mtx;
        bad[10] = char(0x7f), bad[11] = char(0xff), bad[12] = char(0xff);
        refused(bad, "a block that says it is larger than it is");

        // A glyph count the glyph data doesn't make: five more glyphs than it holds.
        refused(mtxCompressFont(font, false, 5), "more glyphs than the glyph data holds");

        // A run-length block that grows past what a font may be: marker, 255, byte, over and over (1 MB of it is 90 MB).
        QByteArray runs;
        runs.append(char(7));
        for (int i = 0; i < 350000; ++i) runs.append(char(7)).append(char(255)).append('a');
        QByteArray block = mtxCompress(runs);
        block[0] = char(quint8(block[0]) | 0x80);   // the flag that says the data is run-length coded
        QByteArray all = mtx.left(4);   // the version and the copy limit
        const qint64 secondRuns = 10 + block.size();
        for (qint64 v : {secondRuns, secondRuns + 4}) all.append(char(v >> 16)).append(char(v >> 8)).append(char(v));
        all.append(block).append(mtxCompress(QByteArray())).append(mtxCompress(QByteArray()));
        refused(all, "run-length data that expands to more than a font may be");

        // A copy that reaches back before the start of what has been made: 5 distance symbols is 32,768 back, past the 7 KB the copies may reach.
        MtxBits bits;
        bits.put(0);
        bits.put(5000, 24);
        MtxTree symbols(256 + 8 * 5 + 3), lengths(8), distances(8);
        symbols.write(256 + 8 * 4, &bits);
        for (int i = 0; i < 5; ++i) distances.write(7, &bits);
        QByteArray crafted = bits.bytes;
        crafted.append(QByteArray(64, '\0'));
        QByteArray header;
        header.append(char(3)).append(char(0)).append(char(0)).append(char(0));
        const qint64 secondAt = 10 + crafted.size();
        for (qint64 v : {secondAt, secondAt + 4}) header.append(char(v >> 16)).append(char(v >> 8)).append(char(v));
        refused(header + crafted + mtxCompress(QByteArray()) + mtxCompress(QByteArray()), "a copy from before the start");

        // Corrupt data: a byte changed anywhere is read as a font or refused, in a moment, and what is read is a sound font.
        QElapsedTimer timer;
        timer.start();
        QRandomGenerator rng(20261012);
        int refusedCount = 0, readCount = 0;
        for (int i = 0; i < 300; ++i) {
            QByteArray damaged = mtx;
            damaged[int(rng.bounded(quint32(mtx.size())))] = char(rng.bounded(256u));
            if (readEmbeddedFont(eotWithMtx(font, damaged), &read, nullptr, &why)) {
                ++readCount;
                QVERIFY(jp::inspectFont(read));
            } else {
                ++refusedCount;
                QVERIFY(!why.isEmpty());
            }
        }
        QVERIFY2(timer.elapsed() < 30000, qPrintable(QString::number(timer.elapsed())));
        QVERIFY(refusedCount > 50);
    }

    // What a block decompresses to is counted against the cap before anything is copied: a tiny file whose first block is as large as a block
    // can be (16 MB) and whose directory points a hundred tables at all of it is refused as damaged at once, not after a hundred copies.
    void microTypeExpressTablesThatShareABlockAreRefusedBeforeTheyAreCopied()
    {
        const int tables = 100, headerEnd = 12 + 16 * tables, tableLength = 16000000;
        QByteArray directory;
        directory.append(char(0)).append(char(1)).append(char(0)).append(char(0));
        appendU16Be(&directory, quint16(tables));
        appendU16Be(&directory, 0), appendU16Be(&directory, 0), appendU16Be(&directory, 0);
        const QByteArray names[] = {"head", "maxp", "glyf"};
        for (int i = 0; i < tables; ++i) {
            directory.append(i < 3 ? names[i] : QByteArray("T") + QByteArray::number(1000 + i));
            directory.append(QByteArray(4, '\0'));
            for (quint32 v : {quint32(headerEnd), quint32(tableLength)}) appendU16Be(&directory, quint16(v >> 16)), appendU16Be(&directory, quint16(v & 0xFFFF));
        }
        QByteArray block = directory;
        block.append(QByteArray(16700000 - headerEnd, '\0'));
        const QByteArray first = mtxCompress(block, false, true);
        QVERIFY2(first.size() < 3000, qPrintable(QString::number(first.size())));
        QByteArray mtx;
        mtx.append(char(3)).append(char(0)).append(char(0)).append(char(0));
        const qint64 second = 10 + first.size(), third = second + 4;
        for (qint64 v : {second, third}) mtx.append(char(v >> 16)).append(char(v >> 8)).append(char(v));
        mtx += first + mtxCompress(QByteArray()) + mtxCompress(QByteArray());
        const QByteArray font = bundledFontBytes(QStringLiteral("Lato"));
        const QByteArray eot = eotWithMtx(font, mtx);
        QVERIFY2(eot.size() < 4000, qPrintable(QString::number(eot.size())));   // the whole file: a few thousand bytes
        QByteArray read;
        QString why;
        resetPeakMemory();
        const qint64 before = peakMemoryKb();
        QElapsedTimer timer;
        timer.start();
        QVERIFY(!readEmbeddedFont(eot, &read, nullptr, &why));
        const qint64 ms = timer.elapsed();
        QVERIFY2(!why.isEmpty() && read.isEmpty(), qPrintable(why));
        QCOMPARE(why, QStringLiteral("The embedded font is damaged."));   // the directory's tables overlap, which no copy is made to find out
        QVERIFY2(ms < 3000, qPrintable(QString::number(ms)));
        if (before > 0) QVERIFY2(peakMemoryKb() - before < 120 * 1024, qPrintable(QString::number(peakMemoryKb() - before) + QLatin1String(" KB more at the peak")));
        // Tables that overlap, or reach past the data, are refused whatever their size.
        for (int i = 0; i < 2; ++i) {
            QByteArray small = directory.left(12 + 16 * 3);
            qToBigEndian<quint16>(3, small.data() + 4);
            const qint64 at = 12 + 16 * 3;
            for (int t = 0; t < 3; ++t) {
                qToBigEndian<quint32>(quint32(at + (i == 0 ? 0 : 8 * t)), small.data() + 12 + 16 * t + 8);   // all at the same place; or each 8 bytes on from the one before
                qToBigEndian<quint32>(i == 0 ? 16u : 40u, small.data() + 12 + 16 * t + 12);                   // 16 bytes each; or reaching past the end
            }
            small.append(QByteArray(32, '\0'));
            QByteArray m2;
            m2.append(char(3)).append(char(0)).append(char(0)).append(char(0));
            const QByteArray b1 = mtxCompress(small);
            const qint64 s2 = 10 + b1.size();
            for (qint64 v : {s2, s2 + 4}) m2.append(char(v >> 16)).append(char(v >> 8)).append(char(v));
            m2 += b1 + mtxCompress(QByteArray()) + mtxCompress(QByteArray());
            QVERIFY2(!readEmbeddedFont(eotWithMtx(font, m2), &read, nullptr, &why), qPrintable(QString::number(i)));
            QVERIFY(!why.isEmpty());
        }
    }

    // The three blocks together are held to a budget too: each under the limit, and all three over what a font may take once made, are refused.
    void microTypeExpressBlocksAreHeldToOneBudget()
    {
        // Run-length data: marker, 255, byte, over and over (117,000 times is 30 MB once expanded).
        QByteArray runs;
        runs.append(char(7));
        for (int i = 0; i < 117000; ++i) runs.append(char(7)).append(char(255)).append('a');
        QByteArray block = mtxCompress(runs, false, true);
        block[0] = char(quint8(block[0]) | 0x80);
        QVERIFY2(block.size() < 20000, qPrintable(QString::number(block.size())));
        QByteArray mtx;
        mtx.append(char(3)).append(char(0)).append(char(0)).append(char(0));
        const qint64 second = 10 + block.size(), third = second + block.size();
        for (qint64 v : {second, third}) mtx.append(char(v >> 16)).append(char(v >> 8)).append(char(v));
        mtx += block + block + block;
        const QByteArray font = bundledFontBytes(QStringLiteral("Lato"));
        QByteArray read;
        QString why;
        resetPeakMemory();
        const qint64 before = peakMemoryKb();
        QElapsedTimer timer;
        timer.start();
        QVERIFY(!readEmbeddedFont(eotWithMtx(font, mtx), &read, nullptr, &why));
        QVERIFY2(!why.isEmpty() && read.isEmpty(), qPrintable(why));
        QCOMPARE(why, QStringLiteral("The embedded font is larger than 32 MB."));   // the budget, not what the data turned out to be
        QVERIFY2(timer.elapsed() < 10000, qPrintable(QString::number(timer.elapsed())));
        if (before > 0) QVERIFY2(peakMemoryKb() - before < 160 * 1024, qPrintable(QString::number(peakMemoryKb() - before) + QLatin1String(" KB more at the peak")));
    }

    // Composite glyphs are checked before a font reaches the font engine: a part that is no glyph of the font, a glyph inside itself (straight
    // or by way of others), parts nested deeper than 16 levels or than maxp says, and more than 256 parts, are all refused.
    void fontInspectionRefusesCompositeGlyphsThatCannotBeDrawnSafely()
    {
        const QByteArray tinos = bundledFontBytes(QStringLiteral("Tinos"));
        QVERIFY(!tinos.isEmpty());
        QString why;
        QVERIFY2(jp::inspectFont(tinos, nullptr, &why), qPrintable(why));
        const int glyphs = fontGlyphs(tinos).size();
        // Glyphs with room for a composite of a few parts, simple ones (not composite), and one with room for 257.
        QVector<int> room, big;
        const QVector<QByteArray> all = fontGlyphs(tinos);
        for (int g = 1; g < glyphs; ++g)
            if (all[g].size() >= 120 && qFromBigEndian<qint16>(all[g].constData()) > 0) {
                room << g;
                if (all[g].size() >= 1700) big << g;
            }
        QVERIFY(room.size() >= 30 && !big.isEmpty());
        auto refused = [&](const QByteArray &font, const char *what) {
            QVERIFY2(!font.isEmpty(), what);
            why.clear();
            QVERIFY2(!jp::inspectFont(font, nullptr, &why), what);
            QVERIFY2(!why.isEmpty(), what);
        };
        auto accepted = [&](const QByteArray &font, const char *what) {
            QVERIFY2(!font.isEmpty(), what);
            QVERIFY2(jp::inspectFont(font, nullptr, &why), qPrintable(QLatin1String(what) + QLatin1String(": ") + why));
        };
        // Parts the font has, and one it doesn't.
        accepted(fontWithComposite(tinos, room[0], {room[1], room[2]}), "a composite of glyphs the font has");
        refused(fontWithComposite(tinos, room[0], {room[1], glyphs}), "a part past the last glyph");
        refused(fontWithComposite(tinos, room[0], {65535}), "a part far past the last glyph");
        // A glyph inside itself, and two glyphs inside each other.
        refused(fontWithComposite(tinos, room[0], {room[0]}), "a glyph that is its own part");
        refused(fontWithComposite(fontWithComposite(tinos, room[0], {room[1]}), room[1], {room[0]}), "two glyphs that are parts of each other");
        // Depth: a chain of composites, each the part of the one before (maxp saying nothing, then what is true, then too little).
        auto chain = [&](int levels) {
            QByteArray font = fontWithComponentDepth(tinos, 0);
            for (int i = 0; i < levels; ++i) font = fontWithComposite(font, room[i], {room[i + 1]});
            return font;
        };
        accepted(chain(16), "composites nested 16 deep");
        refused(chain(17), "composites nested 17 deep");
        refused(fontWithComponentDepth(chain(2), 1), "nested 2 deep where maxp says 1");
        accepted(fontWithComponentDepth(chain(2), 2), "nested 2 deep where maxp says 2");
        accepted(fontWithComponentDepth(chain(1), 1), "nested 1 deep where maxp says 1");
        // How many parts: 256 are the most.
        accepted(fontWithComposite(fontWithComponentDepth(tinos, 0), big[0], QVector<int>(256, room[1])), "256 parts");
        refused(fontWithComposite(fontWithComponentDepth(tinos, 0), big[0], QVector<int>(257, room[1])), "257 parts");
    }

    // Glyph data and the index to it are held to what a font engine can walk: entries in order and inside the table, contours in order, a
    // program that fits.
    void fontInspectionRefusesGlyphDataThatIsOutOfOrderOrCutShort()
    {
        const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        QString why;
        auto refused = [&](const QByteArray &font, const char *what) {
            QVERIFY2(!font.isEmpty(), what);
            why.clear();
            QVERIFY2(!jp::inspectFont(font, nullptr, &why), what);
            QVERIFY2(!why.isEmpty(), what);
        };
        const QVector<QByteArray> glyphs = fontGlyphs(lato);
        int simple = -1;
        for (int g = 1; g < glyphs.size() && simple < 0; ++g)
            if (glyphs[g].size() > 60 && qFromBigEndian<qint16>(glyphs[g].constData()) > 1) simple = g;
        QVERIFY(simple > 0);
        const int at = glyphPlace(lato, simple).first;
        // Contours' last points out of order.
        QByteArray bad = lato;
        qToBigEndian<quint16>(5, bad.data() + at + 10);
        qToBigEndian<quint16>(2, bad.data() + at + 12);
        refused(bad, "contours that go backward");
        // A program longer than the glyph.
        bad = lato;
        const int contours = qFromBigEndian<qint16>(lato.constData() + at);
        qToBigEndian<quint16>(60000, bad.data() + at + 10 + 2 * contours);
        refused(bad, "a program longer than its glyph");
        // More contours than the glyph's data can hold the ends of.
        bad = lato;
        qToBigEndian<qint16>(qint16(glyphPlace(lato, simple).second), bad.data() + at);
        refused(bad, "more contours than fit");
        // The glyph index going back.
        const auto tables = fontTables(lato);
        int locaAt = 0;
        for (int i = 0, n = qFromBigEndian<quint16>(lato.constData() + 4); i < n; ++i)
            if (lato.mid(12 + 16 * i, 4) == "loca") locaAt = int(qFromBigEndian<quint32>(lato.constData() + 12 + 16 * i + 8));
        QVERIFY(qFromBigEndian<qint16>(tables.at("head").constData() + 50) == 0);   // Lato's index is the short one
        bad = lato;
        qToBigEndian<quint16>(0, bad.data() + locaAt + 2 * (simple + 1));
        refused(bad, "an entry that goes back");
    }

    // The character map must name only glyphs the font has, with each subtable inside the table.
    void fontInspectionRefusesACharacterMapThatNamesMissingGlyphs()
    {
        const QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        QString why;
        QVERIFY2(jp::inspectFont(lato, nullptr, &why), qPrintable(why));
        const auto tables = fontTables(lato);
        int cmapAt = 0;
        for (int i = 0, n = qFromBigEndian<quint16>(lato.constData() + 4); i < n; ++i)
            if (lato.mid(12 + 16 * i, 4) == "cmap") cmapAt = int(qFromBigEndian<quint32>(lato.constData() + 12 + 16 * i + 8));
        // Find the format 4 subtable.
        const int subtables = qFromBigEndian<quint16>(lato.constData() + cmapAt + 2);
        int at4 = -1;
        for (int i = 0; i < subtables; ++i) {
            const int offset = int(qFromBigEndian<quint32>(lato.constData() + cmapAt + 4 + 8 * i + 4));
            if (qFromBigEndian<quint16>(lato.constData() + cmapAt + offset) == 4) at4 = cmapAt + offset;
        }
        QVERIFY(at4 > 0);
        const int segments = qFromBigEndian<quint16>(lato.constData() + at4 + 6) / 2;
        QVERIFY(segments > 2);
        const int deltas = at4 + 14 + 2 * segments + 2 + 2 * segments;
        // A segment whose change takes its characters past the last glyph.
        QByteArray bad = lato;
        qToBigEndian<quint16>(30000, bad.data() + deltas);
        QVERIFY(!jp::inspectFont(bad, nullptr, &why));
        QVERIFY(!why.isEmpty());
        // A table whose length says it reaches past the cmap table.
        bad = lato;
        qToBigEndian<quint16>(65535, bad.data() + at4 + 2);
        QVERIFY(!jp::inspectFont(bad, nullptr, &why));
        // A count of segments the table can't hold.
        bad = lato;
        qToBigEndian<quint16>(60000, bad.data() + at4 + 6);
        QVERIFY(!jp::inspectFont(bad, nullptr, &why));
        // Characters out of order.
        bad = lato;
        const int starts = at4 + 14 + 2 * segments + 2;
        qToBigEndian<quint16>(qFromBigEndian<quint16>(bad.constData() + starts + 2 * (segments - 2)) + 100, bad.data() + starts);
        QVERIFY(!jp::inspectFont(bad, nullptr, &why));
    }

    // Tables that share bytes are refused (a font written out again would be larger than it was read), and every bundled font passes.
    void fontInspectionRefusesTablesThatShareBytesAndPassesTheBundledFonts()
    {
        QByteArray lato = bundledFontBytes(QStringLiteral("Lato"));
        // gasp's record points at hhea's data.
        const int gasp = tableRecord(lato, "gasp"), hhea = tableRecord(lato, "hhea");
        QVERIFY(gasp > 0 && hhea > 0);
        QString why;
        QByteArray bad = lato;
        setBe32(bad, gasp + 8, beAt(bad, hhea + 8));
        setBe32(bad, gasp + 12, 8);
        QVERIFY(!jp::inspectFont(bad, nullptr, &why));
        QVERIFY(!why.isEmpty());
        for (const QString &dir : jp::bundledFontDirs()) {
            const QDir d(dir);
            if (!d.exists()) continue;
            int n = 0;
            for (const QString &name : d.entryList({QStringLiteral("*.ttf"), QStringLiteral("*.otf")}, QDir::Files)) {
                QFile f(d.filePath(name));
                QVERIFY(f.open(QIODevice::ReadOnly));
                QVERIFY2(jp::inspectFont(f.readAll(), nullptr, &why), qPrintable(name + QLatin1String(": ") + why));
                ++n;
            }
            if (n) break;
        }
    }

    // A font is made over under a family name of its own (for adding it to the font database so that it is only its document's): the names
    // change, nothing else does, and the font is sound.
    void fontsAreMadeOverUnderAFamilyNameOfTheirOwn()
    {
        const QString family = QStringLiteral("JO-embedded-1234abcd-Test Family");
        struct Case {
            QString family;
            bool bold, italic;
        };
        for (const Case &c : {Case{QStringLiteral("Lato"), false, false}, Case{QStringLiteral("Lato"), true, true}, Case{QStringLiteral("Selawik Light"), false, false},
                              Case{QStringLiteral("TeX Gyre Schola"), true, false}, Case{QStringLiteral("Open Sans"), false, true}}) {
            const QByteArray font = bundledFontBytes(c.family, c.bold, c.italic);
            QVERIFY2(!font.isEmpty(), qPrintable(c.family));
            const QByteArray made = jp::fontWithFamilyName(font, family);
            QVERIFY2(!made.isEmpty(), qPrintable(c.family));
            FontFileInfo was, now;
            QString why;
            QVERIFY2(jp::inspectFont(font, &was, &why), qPrintable(why));
            QVERIFY2(jp::inspectFont(made, &now, &why), qPrintable(c.family + QLatin1String(": ") + why));
            QCOMPARE(now.family, family);
            QVERIFY2(now.fullName.startsWith(family), qPrintable(now.fullName));
            QVERIFY(now.typographicFamily.isEmpty() || now.typographicFamily == family);   // the same name as the family is no other family
            QCOMPARE(now.subfamily, was.subfamily);
            QCOMPARE(now.bold, was.bold);
            QCOMPARE(now.italic, was.italic);
            QCOMPARE(int(now.weight), int(was.weight));
            QCOMPARE(now.glyphs, was.glyphs);
            QCOMPARE(int(now.fsType), int(was.fsType));
            // Every table but the names (and the header's checksum adjustment) is as it was; the checksums are right.
            const auto before = fontTables(font), after = fontTables(made);
            QCOMPARE(after.size(), before.size());
            for (const auto &table : before) {
                if (table.first == "name" || table.first == "head") continue;
                QVERIFY2(after.count(table.first) && after.at(table.first) == table.second, qPrintable(c.family + QLatin1Char(' ') + QString::fromLatin1(table.first)));
            }
            QCOMPARE(after.at("head").left(8), before.at("head").left(8));
            QCOMPARE(after.at("head").mid(12), before.at("head").mid(12));
            QCOMPARE(wordSum(made), 0xB1B0AFBAu);
            for (int i = 0, n = qFromBigEndian<quint16>(made.constData() + 4); i < n; ++i) {
                const char *record = made.constData() + 12 + 16 * i;
                QByteArray data = made.mid(int(qFromBigEndian<quint32>(record + 8)), int(qFromBigEndian<quint32>(record + 12)));
                if (QByteArray(record, 4) == "head") data.replace(8, 4, QByteArray(4, '\0'));
                QCOMPARE(qFromBigEndian<quint32>(record + 4), wordSum(data));
            }
            // The font database takes it as that family, and no other.
            const int id = QFontDatabase::addApplicationFontFromData(made);
            QVERIFY2(id >= 0, qPrintable(c.family));
            QCOMPARE(QFontDatabase::applicationFontFamilies(id), QStringList{family});
            QVERIFY(QFontDatabase::removeApplicationFont(id));
        }
        // A font that is not one is not made over; nor is one to be called nothing.
        QVERIFY(jp::fontWithFamilyName(QByteArray("not a font"), family).isEmpty());
        QVERIFY(jp::fontWithFamilyName(bundledFontBytes(QStringLiteral("Lato")), QString()).isEmpty());
    }
};

JP_TEST_CLASS(OfficeFontTests)
#include "tst_office_fonts.moc"
